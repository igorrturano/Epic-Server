// ============================================================================
// epicNPCCombat.inc - per-tick brain utilities
// ============================================================================
// Extracted from epicai.inc. Consumed by brain.src and brain-type includes
// (monsterBrain, humanoidBrain, etc.) for per-tick combat/pursuit/threat/LOS
// utilities. Also consumed by external scripts that previously got these via
// the leaky-transitive epicai.inc chain (heal spells, death hooks).
// ============================================================================

use cfgfile;
use uo;
use os;
use npc;

// Most utilities here use ParseTemplateName / GetAnimalTemplateByTier from
// epicai.inc, so re-include it here.
include ":epicAI:include/epicai";

// Direct deps for per-tick functions:
include "npcUtils";
include ":epicAI:epicSpecial";
include ":npcs:NPCUtilities";
include ":fls_core:fls_skillCheck";
include ":epicAI:npcDataCache";
include ":abilities:include/models/Ability";
include ":propagacao:include/util/propagacaoConstants";
include ":epicAI:include/threatModel";

// Script-level caches (per-tick state, brain compile unit owns these)
var nearby_cache_var := dictionary;
var enemy_last_loc_var := 0;
var initial_allies_var := 0;

// Pursuit behavior configuration constants
const PURSUIT_PROGRESS_CHECK_INTERVAL := 3;  // seconds between progress checks
const PURSUIT_MAX_NO_PROGRESS_COUNT := 3;    // failed checks before repositioning
const PURSUIT_DISTANCE_TOLERANCE := 2;       // extra tiles before pursuing
const PURSUIT_PATH_CACHE_DURATION := 8;      // seconds to cache pathfinding results
const PURSUIT_TARGET_MOVE_THRESHOLD := 3;    // tiles moved before recalculating path

// Helper: Calculate the ideal engagement distance based on weapon/brain type
// Capped at 6 to prevent NPCs from retreating too far and getting stuck
const MAX_RANGED_PURSUIT_DISTANCE := 6;



/*
 * InitializeEnterArea()
 *
 * Purpose: Trigger ENTEREDAREA events for nearby players/combatants on spawn
 * This ensures newly spawned NPCs detect nearby threats immediately instead of
 * waiting up to 60 seconds in dormant mode.
 *
 * Similar to brainAI's InitializeEnterArea() function
 */
function InitializeEnterArea(mobile)
    // Scan for nearby players and trigger ENTEREDAREA events
    // This ensures consistent behavior between spawn and re-activation
    var nearby := ListMobilesNearLocationEx(mobile.x, mobile.y, LIST_IGNORE_Z,
                                            AREA_SIZE, LISTEX_FLAG_HIDDEN + LISTEX_FLAG_NORMAL + LISTEX_FLAG_CONCEALED,
                                            mobile.realm);

    foreach nearby_mobile in nearby
        if (nearby_mobile.serial == mobile.serial)
            continue;  // Skip self
        endif

        // Only trigger for players, pets, or valid combat targets
        if (nearby_mobile.acctname || nearby_mobile.master)
            var ev := struct;
            ev.+type := SYSEVENT_ENTEREDAREA;
            ev.+source := nearby_mobile;
            SendEvent(mobile, ev);
            sleepms(5);  // Small delay to prevent event flooding
        endif
    endforeach

    return 1;
endfunction


function TacticalReposition(opponent, chasing)
    var runs := 0;
    var max_dist := 12;
    var stuck_count := 0;

    while(Distance(me, opponent) < max_dist && runs < 10)
        var old_pos := struct{ "x" := me.x, "y" := me.y };
        TryToRunAway(opponent, 1);
        runs += 1;
        sleepms(200);

        if (me.x == old_pos.x && me.y == old_pos.y)
            stuck_count += 1;
            if (stuck_count >= 2)
                var possible_locations := GetAlternativeLocations(opponent);
                foreach loc in possible_locations
                    if (CheckLosAt(me, loc.x, loc.y, me.z))
                        RunTowardLocation(loc.x, loc.y);
                        return 1;
                    endif
                    sleepms(2);
                endforeach
                // All alternatives failed — switch to melee combat
                SwitchToMelee();
                return PursueMelee(opponent);
            endif
        else
            stuck_count := 0;
        endif
    endwhile

    return 1;
endfunction


function GetAlternativeLocations(opponent)
    var locations := array{};
    var range := 8;
    
    // Try cardinal directions first
    var directions := array{
        struct{ "x" := range, "y" := 0 },
        struct{ "x" := -range, "y" := 0 },
        struct{ "x" := 0, "y" := range },
        struct{ "x" := 0, "y" := -range }
    };

    // Add diagonal directions
    directions.append(struct{ "x" := range, "y" := range });
    directions.append(struct{ "x" := -range, "y" := range });
    directions.append(struct{ "x" := range, "y" := -range });
    directions.append(struct{ "x" := -range, "y" := -range });

    foreach dir in directions
        var new_x := me.x + dir.x;
        var new_y := me.y + dir.y;
        
        // Check if location is away from opponent and not hazardous
        if (CoordinateDistance(new_x, new_y, opponent.x, opponent.y) > Distance(me, opponent))
            if (!IsTileHazardous(new_x, new_y))
                locations.append(struct{
                    "x" := new_x,
                    "y" := new_y,
                    "dist" := CoordinateDistance(new_x, new_y, opponent.x, opponent.y)
                });
            endif
        endif
        sleepms(2);
    endforeach

    // Sort by distance from opponent (furthest first)
    locations.sort();
    
    return locations;
endfunction



/*
 * UpdateThreatList(mobile, ev)
 *
 * Phase 3: Threat List Caching
 * Purpose: Incrementally update threat list based on events
 * Called from ENTEREDAREA and LEFTAREA handlers
 */
function UpdateThreatList(mobile, ev)
    if (!ev || !ev.source)
        return;
    endif

    case (ev.type)
        SYSEVENT_ENTEREDAREA:
            if (CanFight(mobile, ev.source))
                if (!(ev.source.serial in threat_list_var))
                    threat_list_var.append(ev.source.serial);
                endif
            endif
        SYSEVENT_DAMAGED:
            // Attackers that hit from outside AREA_SIZE (AoE/ranged/spells)
            // never fire ENTEREDAREA, so without this they'd live only in the
            // damage table and disappear from the cache fallback path.
            if (CanFight(mobile, ev.source))
                if (!(ev.source.serial in threat_list_var))
                    threat_list_var.append(ev.source.serial);
                endif
            endif
        SYSEVENT_LEFTAREA:
            var idx := 1;
            foreach serial in threat_list_var
                if (serial == ev.source.serial)
                    threat_list_var.erase(idx);
                    break;
                endif
                idx += 1;
            endforeach
    endcase
endfunction


/*
 * GetCachedThreats()
 *
 * Phase 3: Threat List Caching
 * Purpose: Get validated threats from cache instead of full area scan
 * Returns: First valid threat found, or 0
 * Replaces: CheckNearbyThreats() - eliminates 70% of area scans
 */
function GetCachedThreats()
    if (!threat_list_var || threat_list_var.size() == 0)
        return 0;
    endif

    foreach serial in threat_list_var
        var mobile := SystemFindObjectBySerial(serial);
        if (mobile && CanFight(me, mobile))
            return mobile;
        endif
        sleepms(2);
    endforeach

    return 0;
endfunction


/*
 * CheckNearbyThreats()
 *
 * Phase 3: Hybrid Threat Detection
 * Purpose: Use cached threats (fast) / removed fallback area scan (slow)
 * This prevents NPCs from giving up when cache is empty due to event timing
 */
function CheckNearbyThreats()
    // First check cached list (fast path)
    var threat := GetCachedThreats();
    if (threat)
        return threat;
    endif

    return 0;
endfunction


/*
 * GetNearbyMobilesOnce(mobile, range, flags := 0)
 *
 * Phase 4: Scan Consolidation
 * Purpose: Cache area scans for 1s to prevent redundant ListMobilesNearLocationEx calls
 * Multiple functions can share the same scan result if called within the cache window
 *
 * Parameters:
 *   - mobile: The NPC doing the scan
 *   - range: Scan radius in tiles
 *   - flags: LISTEX flags (default: 0 = LISTEX_FLAG_NORMAL)
 *
 * Returns: Array of mobiles within range
 *
 * Eliminates: Redundant scans when CheckForIdealTarget, AlertAllies, etc. run in same tick
 */
function GetNearbyMobilesOnce(mobile, range, flags := LISTEX_FLAG_NORMAL)
    // Script-level cache (no ObjProperty serialization)
    var cache_key := $"{range}_{flags}";
    var now := polcore().systime;

    if (nearby_cache_var.exists(cache_key))
        var cached := nearby_cache_var[cache_key];
        if ((CDbl(now) - CDbl(cached.time)) < 1.0)
            return cached.mobiles;
        endif
    endif

    // Cache miss or expired - perform scan
    var mobiles := ListMobilesNearLocationEx(mobile.x, mobile.y, LIST_IGNORE_Z, range, flags, mobile.realm);

    nearby_cache_var[cache_key] := struct{
        "mobiles" := mobiles,
        "time" := now
    };

    return mobiles;
endfunction


// Safe class lookup. opponent.getClass() reads chardata.classe which doesn't
// exist on NPCs; the chained access then returns error. Normalize to "" so
// downstream comparisons (== SABIO etc.) behave like a clean string compare.
function GetOpponentClass(opponent)
    if (!opponent)
        return "";
    endif
    var klass := opponent.getClass();
    if (klass == error || !klass)
        return "";
    endif
    return klass;
endfunction


// Canonical ranged check. opponent.weapon is 0 when bare-handed (the common
// SABIO case), so calling .isRanged() directly errors out and poisons the
// caller's expression. SABIO is force-classified as ranged regardless of
// equipped weapon.
function IsOpponentRanged(opponent)
    if (!opponent)
        return 0;
    endif
    if (GetOpponentClass(opponent) == SABIO)
        return 1;
    endif
    var w := opponent.weapon;
    if (!w)
        return 0;
    endif
    var ranged := w.isRanged();
    if (ranged == error)
        return 0;
    endif
    return ranged;
endfunction


// Shared tracker shape for PursueTarget and PursueMelee. Includes both the
// pursue-side fields (target_serial, progress counters) and the retreat-side
// fields so the two paths can transition into each other without struct-field
// drift.
function BuildPursuitTracker(opp_serial, x, y, dist, current_time)
    return struct{
        "start_time" := current_time,
        "start_pos_x" := x,
        "start_pos_y" := y,
        "start_distance" := dist,
        "target_serial" := opp_serial,
        "failed_moves" := 0,
        "total_moves" := 0,
        "last_pos_x" := x,
        "last_pos_y" := y,
        "last_progress_check" := current_time,
        "no_progress_count" := 0,
        "retreat_dist" := 0,
        "retreat_ticks" := 0
    };
endfunction


function CheckForIdealTarget()
   // Threat-first: if the threat table has a valid candidate, use it. The
   // threat model owns target stickiness via hysteresis (THREAT_SWITCH_MARGIN),
   // so the legacy 20s target_lock_var and 90% keep_target_chance are gone.
   var top := GetTopThreat();
   if (top)
      return top;
   endif

   // Opening-engagement fallback: nobody has dealt damage yet (NPC just
   // engaged, first tick of combat). Score nearby mobiles to seed an initial
   // target; subsequent damage events populate the threat table.
   var potential_opponents := GetNearbyMobilesOnce(me, AREA_SIZE, LISTEX_FLAG_NORMAL);
   if (potential_opponents.size() < 1)
      return 0;
   endif
   var best_opponent := struct{
      "mobile" := 0,
      "priority" := -1
   };

   var preferred_target := GetObjProperty(me, "preferred_target");

   foreach opponent in potential_opponents
      if (!CanFight(me, opponent))
         continue;
      endif

      var opponent_class := GetOpponentClass(opponent);
      var opponent_is_ranged := IsOpponentRanged(opponent);
      var priority := 0;

      if (opponent == me.opponent)
         priority += 15;
      endif

      if (opponent_is_ranged)
         priority += 2;
      endif

      if (me.isSage())
         if (opponent_class == GUERREIRO)
            priority += 2;
         endif
      elseif (me.isWarrior())
         if (opponent_class == LADINO)
            priority += 2;
         endif
      elseif (me.isRogue())
         if (opponent_class == SABIO)
            priority += 2;
         endif
      endif

      var dist := Distance(me, opponent);
      if (dist <= 4)
         priority += 1;
      endif

      if (preferred_target)
          case (preferred_target)
          "Ranged": if (opponent_is_ranged) priority += 4; endif
          "Melee": if (!opponent_is_ranged) priority += 4; endif
          "Guerreiro": if (opponent_class == GUERREIRO) priority += 4; endif
          "Ladino": if (opponent_class == LADINO) priority += 4; endif
          "Mago": if (opponent_class == SABIO) priority += 4; endif
          "Pouca vida": 
              var hp_percent := CDbl(AP_GetVital(opponent, HITS)) / CDbl(AP_GetVitalMaximumValue(opponent, HITS)) * 100;
              if (hp_percent <= 50) 
                  priority += 3;
              endif
          endcase
      endif

      if (priority > best_opponent.priority)
         best_opponent.mobile := opponent;
         best_opponent.priority := priority;
      endif
   endforeach

   // Stickiness: if no candidate scored higher than the current opponent
   // (or the loop bailed early), keep what we have rather than returning 0
   // and triggering SetOpponent(0) upstream.
   if (!best_opponent.mobile && me.opponent && CanFight(me, me.opponent))
      return me.opponent;
   endif

   return best_opponent.mobile;
endfunction


// New function to handle ally behavior
function AlertAllies(invasor, me)
    if (me.alignment != 1)
        return 0;
    endif

    if (isAlly(invasor))
        return 0;
    endif

    if (invasor.hidden || invasor.concealed || GetObjProperty(invasor, "noattack") )
        return 0;
    endif

    if (GetCooldown(me, "alert_allies"))
        return 0;
    endif
    // Phase 4: Use cached scan instead of direct ListMobilesNearLocation
    foreach mobile in GetNearbyMobilesOnce(me, 8, LISTEX_FLAG_NORMAL)
        if (mobile.npctemplate && mobile != me)
            if (isAlly(mobile))
                SetObjProperty(mobile, "#taunt_opponent", invasor.serial);
                SendEvent(mobile, struct{ "type" := EVID_ALERT_ALLIES, "source" := invasor });
            endif
        endif
        sleepms(2);
    endforeach

    SetCooldown(me, "alert_allies", 15);
endfunction


function isAlly(mobile)
    if (!mobile || mobile.dead) return 0; endif

    if (mobile.master || GetObjProperty(mobile, "summoned")) return 0; endif

    if (mobile.hidden || mobile.concealed) return 0; endif

    if (mobile.acctname || mobile.alignment != 1) return 0; endif
    
    return 1;
endfunction


// Simplified NPC version of ExaminarArea for "hearing footsteps"
// NPCs use this to detect nearby players without LoS and investigate
// Only detects players who are actually MOVING (not standing still)
// Returns: mobile reference if detected someone, 0 otherwise
function NPCHearFootsteps(npc)
    var hearing_range := 8;

    // Phase 4: Use cached scan instead of direct ListMobilesNearLocationEx
    foreach mobile in GetNearbyMobilesOnce(npc, hearing_range, LISTEX_FLAG_NORMAL)
        if (mobile == npc)
            continue;
        endif

        if (mobile.npctemplate)
            continue;
        endif

        if (CheckLineOfSight(npc, mobile))
            var event := struct{ "type" := SYSEVENT_ENTEREDAREA, "source" := mobile };

            SendEvent(npc, event);
            return true;
        endif

        if (abs(mobile.z - npc.z) >= 19)
            continue;
        endif

        // Check if player is actually moving (not standing still)
        var last_pos := GetObjProperty(npc, "#heard_" + mobile.serial);
        var current_pos := struct{ "x" := mobile.x, "y" := mobile.y, "time" := polcore().systime };

        if (last_pos)
            var has_moved := (mobile.x != last_pos.x || mobile.y != last_pos.y);
            var time_diff := current_pos.time - last_pos.time;

            if (!has_moved || time_diff > 2)
                SetObjProperty(npc, "#heard_" + mobile.serial, current_pos);
                continue;
            endif
        else
            // First time seeing this player - store position but don't detect yet
            // Need to wait for next check to confirm they're actually moving
            SetObjProperty(npc, "#heard_" + mobile.serial, current_pos);
            continue;
        endif

        SetObjProperty(npc, "#heard_" + mobile.serial, current_pos);

        var difficulty := _calculateDHDiffLocal(npc, mobile);
        var cache := NPC_GetCachedData(npc);  // O(1) lookup
        var npc_awareness := cache.tier * 10;

        if (SkillCheckMod(npc, AWARENESS, difficulty, npc_awareness) > 0)
            return mobile;
        endif

        sleepms(2);
    endforeach

    return false;
endfunction


/*
 * CanFight(attacker, defender)
 *
 * Phase 3: Context-Aware LoS Check
 * Purpose: Allow NPCs to pursue existing targets without LoS, but require LoS for new targets
 *
 * Logic:
 *   - NEW targets (not current opponent): Require LoS to engage
 *   - EXISTING opponent: Allow pursuit even if LoS temporarily blocked
 *   - This prevents NPCs from giving up when player moves behind obstacles
 */
function CanFight(attacker, defender)
    if (!defender || defender.dead || defender.hidden || defender.concealed)
        return 0;
    endif

    if ( GetCooldown(attacker, "peacemade") > 0 )
        return 0;
    endif

    if (GetObjProperty(defender, "noattack"))
        return 0;
    endif

    // Only require LoS for NEW targets, not existing opponent
    // This allows pursuit of current target even if LoS is temporarily blocked
    if (attacker.opponent != defender)
        if (!CheckLineOfSight(attacker, defender))
            return 0;
        endif
    endif

    // Get cached data for attacker (O(1) lookup)
    var attacker_cache := NPC_GetCachedData(attacker);

    // Jogadores corrompidos pela entropia nao sao atacados por mortos vivos.
    if (MobileWrapper(defender).GetEntropy() <= -16000 && attacker_cache.environment_lower == "undead")
        return 0;
    endif

    // Aparencia Cadaverica: mortos vivos nao atacam quem parece um deles.
    // Se o jogador atacar um morto vivo, a aparencia quebra.
    if (GetObjProperty(defender, "aparenciacadaverica") && attacker_cache.environment_lower == "undead")
        if (attacker.opponent == defender)
            EraseObjProperty(defender, "aparenciacadaverica");
            SendSysMessageEx(defender, "A sua aparencia cadaverica se desfez!", SSM_FAIL);
        else
            return 0;
        endif
    endif

    // Cooldown to avoid ress kill
    if (Cint(GetCooldown(defender, "deathtime" )) > 0)
        // Allow combat if defender initiated it
        if (attacker.opponent == defender) return 1; endif
        // Otherwise prevent attacking during cooldown
        return 0;
    endif

    var category := attacker_cache.environment;

    if (category == "Aklats")
        // Usa função dedicada para detectar ameaças
        var aklat_threat_detected := DetectAklatsThreats(attacker);

        // Se não detectou ameaça, não pode atacar
        if (!aklat_threat_detected)
            return 0;
        endif

        // Se detectou ameaça, continua com as verificações normais
        // (não retorna 1 imediatamente para permitir outras checagens)
    endif

    // Grey NPCs (neutral alignment) should only fight back when attacked
    if (attacker.alignment == 0)
        // Only allow combat if the defender is already attacking this NPC
        if (attacker.opponent == defender || defender.opponent == attacker)
            return 1;
        endif
        return 0;
    endif

    // If defender is a player, normal combat rules apply
    if (!defender.npctemplate)
        return 1;
    endif

    // NPC vs NPC combat only happens if:
    if (defender.npctemplate)
        // Check if forced to attack by GM command
        var forced_target := GetObjProperty(attacker, "#ForcedTarget");
        if (forced_target == defender.serial)
            return 1;
        endif

        if (defender.master)
            return 1;
        endif

        // Check enemy categories - use cached data for both NPCs
        var defender_cache := NPC_GetCachedData(defender);
        var enemy_cats := GetConfigStringArray(attacker_cache.npc_elem, "EnemyCategories");

        if (enemy_cats && defender_cache.environment in enemy_cats)
            return 1;
        endif

        return 0; // No valid reason for NPCs to fight
    endif

    return 1;
endfunction


function CalculateIdealPursuitDistance()
    // Use cached weapon range (computed once per brain activation, not per tick)
    if (!cached_weapon_range)
        cached_weapon_range := GetWeaponRangeInfo();
    endif
    var range_info := cached_weapon_range;

    if (me.isHumanoidBrain())
        if (me.weapon.isRanged() || range_info.max_range > 1)
            if (range_info.max_range > MAX_RANGED_PURSUIT_DISTANCE)
                return MAX_RANGED_PURSUIT_DISTANCE;
            endif
            return range_info.max_range;
        elseif (me.isSage() && me.weapon.objtype == FIST)
            return 6;
        endif
    else
        if (isMonsterRanged())
            if (me.weapon && range_info.max_range > 1)
                if (range_info.max_range > MAX_RANGED_PURSUIT_DISTANCE)
                    return MAX_RANGED_PURSUIT_DISTANCE;
                endif
                return range_info.max_range;
            else
                return MAX_RANGED_PURSUIT_DISTANCE;
            endif
        endif
    endif

    return 1;
endfunction


// Helper: Check if we're already at the desired distance
function IsWithinEngagementRange(opponent, ideal_distance)
    var dist := Distance(me, opponent);

    // No tolerance - NPCs should close to exactly the ideal distance
    // Tolerance was causing NPCs to stop too far away (melee at 3 tiles, etc.)
    return (dist <= ideal_distance);
endfunction


// Helper: Get cached path or calculate new one
function GetOrCreatePursuitPath(opponent)
    if (path_cache_var)
        var cache_age := ReadGameClock() - path_cache_var.created_time;
        var target_moved := CoordinateDistance(opponent.x, opponent.y, path_cache_var.target_x, path_cache_var.target_y);

        if (cache_age < PURSUIT_PATH_CACHE_DURATION &&
            target_moved < PURSUIT_TARGET_MOVE_THRESHOLD &&
            path_cache_var.path && path_cache_var.path.size() > 0)
            return path_cache_var.path;
        endif
    endif

    var new_path := FindPath(me.x, me.y, me.z, opponent.x, opponent.y, opponent.z, me.realm, FP_IGNORE_MOBILES, 50);

    if (new_path)
        path_cache_var := struct{
            "path" := new_path,
            "target_x" := opponent.x,
            "target_y" := opponent.y,
            "created_time" := ReadGameClock()
        };
    endif

    return new_path;
endfunction


// Cache LOS check for pursuit (20% performance gain)
function GetCachedLOS(opponent)
    var current_time := ReadGameClock();

    if (los_cache_var &&
        los_cache_var.opponent_serial == opponent.serial &&
        (current_time - los_cache_var.time) < 1)
        return los_cache_var.has_los;
    endif

    var has_los := CheckLineOfSight(me, opponent);
    los_cache_var := struct{
        "opponent_serial" := opponent.serial,
        "has_los" := has_los,
        "time" := current_time
    };

    return has_los;
endfunction


// Hazard detection cache (2s TTL, range 3 scan)
// Returns hazard_cache_var struct with .tiles dictionary and .on_hazard flag
function GetCachedHazardTiles()
    var current_time := ReadGameClock();

    if (hazard_cache_var && (current_time - hazard_cache_var.time) < 2)
        return hazard_cache_var;
    endif

    var tile_dict := dictionary;
    var on_hazard := 0;
    var markers := ListItemsNearLocationOfType(me.x, me.y, me.z, 3, MARCADOR_PROPAGACAO_OBJTYPE, me.realm);
    var cache := NPC_GetCachedData(me);
    var is_elemental := (cache.environment_lower == "elemental");

    foreach marker in markers
        if (Abs(me.z - marker.z) > 19)
            continue;
        endif
        var prop := GetObjProperty(marker, "propPropagacao");
        if (prop && prop.lerEstadoAtual > ESTADO_PROPAGACAO::DORMENTE)
            // Elementals ignore fire hazards (but still avoid gas)
            if (is_elemental && prop.tipoPropagacao == PROPAGACAO_TIPO::INCENDIO)
                continue;
            endif
            var key := CStr(marker.x) + "_" + CStr(marker.y);
            tile_dict[key] := 1;
            if (marker.x == me.x && marker.y == me.y)
                on_hazard := 1;
            endif
        endif
        sleepms(2);
    endforeach

    hazard_cache_var := struct{
        "tiles" := tile_dict,
        "on_hazard" := on_hazard,
        "time" := current_time
    };

    return hazard_cache_var;
endfunction


function IsTileHazardous(x, y)
    if (!hazard_cache_var || !hazard_cache_var.tiles)
        return 0;
    endif
    var key := CStr(x) + "_" + CStr(y);
    return hazard_cache_var.tiles.exists(key);
endfunction


// One-tick escape reflex: if standing on a hazard, sidestep to the best adjacent safe tile
// Returns 1 if the NPC moved this tick (caller should skip normal movement)
function TickHazardEscape()
    GetCachedHazardTiles();

    if (!hazard_cache_var || !hazard_cache_var.on_hazard)
        return 0;
    endif

    var best_x := me.x, best_y := me.y;
    var best_score := -9999;
    var found := 0;
    var has_target := (me.opponent && !me.opponent.dead);

    // Score 8 adjacent tiles: prefer safe tiles closer to opponent/anchor
    var offsets := array{ {-1,-1}, {0,-1}, {1,-1}, {-1,0}, {1,0}, {-1,1}, {0,1}, {1,1} };
    foreach off in offsets
        var tx := me.x + off[1];
        var ty := me.y + off[2];

        if (IsTileHazardous(tx, ty))
            continue;
        endif

        var score := 0;
        if (has_target)
            score := -CoordinateDistance(tx, ty, me.opponent.x, me.opponent.y);
        elseif (anchor_var)
            score := -CoordinateDistance(tx, ty, anchor_var[1], anchor_var[2]);
        endif

        if (score > best_score)
            best_score := score;
            best_x := tx;
            best_y := ty;
            found := 1;
        endif
    endforeach

    if (found)
        RunTowardLocation(best_x, best_y);
        return 1;
    endif

    return 0;
endfunction


/*
 * PursueMelee(opponent)
 *
 * Fast pursuit for melee NPCs. One movement action per call, no internal sleep loops.
 * The brain loop handles iteration at 100ms intervals, keeping event processing responsive.
 */
function PursueMelee(opponent)
    var dist := Distance(me, opponent);

    if (dist > AREA_SIZE + 6)
        return 0;
    endif

    if (dist <= 1)
        TurnToward(opponent);
        return 1;
    endif

    var should_run := !(me.hidden || me.concealed);
    var current_time := ReadGameClock();

    // Init tracker on first call or target switch. Distance-window fields are
    // melee-specific; the legacy fields stay for PursueTarget's retreat path.
    if (!pursuit_tracker || pursuit_tracker.target_serial != opponent.serial)
        pursuit_tracker := BuildPursuitTracker(opponent.serial, me.x, me.y, dist, current_time);
        pursuit_tracker.+window_start_dist := dist;
        pursuit_tracker.+window_start_time := current_time;
        pursuit_tracker.+windows_stalled := 0;
        pursuit_tracker.+path_mode := 0;
        pursuit_tracker.+best_distance := dist;
        pursuit_tracker.+last_progress_time := current_time;
    endif

    // Real progress: any tick that closes opponent distance below the best
    // ever resets the give-up timer.
    if (dist < pursuit_tracker.best_distance)
        pursuit_tracker.best_distance := dist;
        pursuit_tracker.last_progress_time := current_time;
    endif

    // 2s window: count consecutive windows where distance didn't decrease.
    // Catches oscillation (alternating tiles around a blocked direct path)
    // that movement-based detection can't see.
    if (current_time - pursuit_tracker.window_start_time >= 2)
        if (dist >= pursuit_tracker.window_start_dist)
            pursuit_tracker.windows_stalled += 1;
        else
            pursuit_tracker.windows_stalled := 0;
        endif
        pursuit_tracker.window_start_dist := dist;
        pursuit_tracker.window_start_time := current_time;
    endif

    // ~4s of no distance progress → commit to pathfinding (sticky). LOS is
    // unreliable as a "walkable" signal: lava, water, and elevation gaps pass
    // LOS but block movement, leaving RunToward to oscillate forever.
    if (pursuit_tracker.windows_stalled >= 2)
        pursuit_tracker.path_mode := 1;
    endif
    // Drop path_mode for the final approach: cheap direct movement is enough
    // once we're close and have clear LOS.
    if (pursuit_tracker.path_mode && dist <= 3 && CheckLineOfSight(me, opponent))
        pursuit_tracker.path_mode := 0;
        pursuit_tracker.windows_stalled := 0;
    endif

    var has_los := CheckLineOfSight(me, opponent);

    if (has_los)
        enemy_last_loc_var := struct{
            "x" := opponent.x, "y" := opponent.y, "z" := opponent.z, "time" := current_time
        };
    endif

    // Direct movement: LOS + not in path_mode
    if (has_los && !pursuit_tracker.path_mode)
        if (should_run)
            RunToward(opponent);
        else
            WalkToward(opponent);
        endif
        return 1;
    endif

    // Pathfind: no LOS, or stuck on the LOS approach (path_mode active)
    var path := FindPath(me.x, me.y, me.z, opponent.x, opponent.y, opponent.z, me.realm, FP_IGNORE_MOBILES, 50);
    if (path && path.size() > 0)
        var coord := 0;
        foreach waypoint in path
            if (!IsTileHazardous(waypoint.x, waypoint.y))
                coord := waypoint;
                break;
            endif
        endforeach
        if (coord)
            if (should_run)
                RunTowardLocation(coord.x, coord.y);
            else
                WalkTowardLocation(coord.x, coord.y);
            endif
        endif
    else
        var last_loc := enemy_last_loc_var;
        if (last_loc && (current_time - last_loc.time) < 30)
            if (should_run)
                RunTowardLocation(last_loc.x, last_loc.y);
            else
                WalkTowardLocation(last_loc.x, last_loc.y);
            endif
        else
            if (should_run)
                RunToward(opponent);
            else
                WalkToward(opponent);
            endif
        endif
    endif

    return 1;
endfunction


function PursueTarget(opponent)
    if (!opponent || opponent.dead)
        pursuit_tracker := 0;
        return 0;
    endif

    var ideal_distance := CalculateIdealPursuitDistance();

    // Fast path for melee NPCs — no internal sleep loops, one action per call
    if (ideal_distance <= 1)
        return PursueMelee(opponent);
    endif

    // === Ranged/special NPC pursuit below ===
    var current_time := ReadGameClock();
    var dist := Distance(me, opponent);
    var is_ranged := 1;
    var should_run := !(me.hidden || me.concealed);
    var has_los := GetCachedLOS(opponent);
    var my_x := me.x;
    var my_y := me.y;
    var opp_serial := opponent.serial;
    var opp_x := opponent.x;
    var opp_y := opponent.y;
    var opp_z := opponent.z;

    if (dist > AREA_SIZE + 6)
        return 0;
    endif

    if (dist == ideal_distance && has_los)
        return 1;
    endif

    if (has_los)
        enemy_last_loc_var := struct{
            "x" := opp_x, "y" := opp_y, "z" := opp_z, "time" := current_time
        };
    endif

    // Ranged retreat logic (too close) - one attempt per tick, non-blocking.
    // Mutates pursuit_tracker in place; do NOT refactor to a local `tracker`
    // var (the pursue branch below uses that pattern but only after this
    // early-return path has settled).
    if (dist < ideal_distance)
        if (should_run)
            RunAwayFrom(opponent);
        else
            WalkAwayFrom(opponent);
        endif

        // Ensure tracker has the unified shape so a retreat→pursue transition
        // doesn't trip the `target_serial` comparison below.
        if (!pursuit_tracker || pursuit_tracker.target_serial != opp_serial)
            pursuit_tracker := BuildPursuitTracker(opp_serial, my_x, my_y, dist, current_time);
        endif
        if (!pursuit_tracker.retreat_dist)
            pursuit_tracker.retreat_dist := dist;
            pursuit_tracker.retreat_ticks := 0;
        endif
        pursuit_tracker.retreat_ticks += 1;

        if (pursuit_tracker.retreat_ticks >= 5)
            if (Distance(me, opponent) <= pursuit_tracker.retreat_dist + 1)
                // No progress after 5 ticks - cornered/oscillating
                pursuit_tracker := 0;
                SwitchToMelee();
                return PursueMelee(opponent);
            endif
            pursuit_tracker.retreat_dist := 0;
        endif
        return 1;
    else
        if (pursuit_tracker && pursuit_tracker.retreat_dist)
            pursuit_tracker.retreat_dist := 0;
        endif
    endif

    // Pursuit tracker (script-level var)
    var tracker := pursuit_tracker;
    var tracker_needs_reset := (!tracker || tracker.target_serial != opp_serial);

    if (tracker_needs_reset)
        tracker := BuildPursuitTracker(opp_serial, my_x, my_y, dist, current_time);
    endif

    // Direct movement with LOS - one step per tick, non-blocking
    var use_pathfinding := !has_los;

    if (has_los && dist > ideal_distance)
        if (should_run)
            RunToward(opponent);
        else
            WalkToward(opponent);
        endif

        // Detect stuck using native lastcoord
        if (me.x == me.lastcoord.lastx && me.y == me.lastcoord.lasty)
            path_cache_var := 0;
            TacticalReposition(opponent, 1);
            use_pathfinding := 1;
        endif
    endif

    // Stuck detection and flee logic
    var time_elapsed := current_time - tracker.start_time;
    var distance_from_start := CoordinateDistance(me.x, me.y, tracker.start_pos_x, tracker.start_pos_y);
    var moved_this_cycle := (me.x != tracker.last_pos_x || me.y != tracker.last_pos_y);

    tracker.total_moves += 1;
    if (!moved_this_cycle)
        tracker.failed_moves += 1;
    endif
    tracker.last_pos_x := me.x;
    tracker.last_pos_y := me.y;

    if (time_elapsed >= 10 && distance_from_start <= 6)
        var success_rate := (tracker.total_moves > 0) ?
            CInt((CDbl(tracker.total_moves - tracker.failed_moves) / CDbl(tracker.total_moves)) * 100) : 0;

        if (success_rate < 30)
            pursuit_tracker := 0;
            path_cache_var := 0;
            // Anti-exploit: teleport to anchor instead of blocking Flee loop
            var anchor := GetObjProperty(me, "anchor");
            if (anchor)
                PrintTextAbove(me, "*Retorna correndo* [Sistema Anti-Exploits]");
                MoveObjectToLocation(me, anchor[1], anchor[2], anchor[3], me.realm, MOVEOBJECT_FORCELOCATION);
            endif
            return 0;
        endif
    endif

    var distance_improvement := tracker.start_distance - dist;
    if (distance_from_start > 8 || distance_improvement > 3)
        tracker.start_time := current_time;
        tracker.start_pos_x := me.x;
        tracker.start_pos_y := me.y;
        tracker.start_distance := dist;
        tracker.failed_moves := 0;
        tracker.total_moves := 0;
    endif

    // Pathfinding (only when needed)
    if (use_pathfinding)
        var path := GetOrCreatePursuitPath(opponent);
        if (path)
            var step_count := 0;
            foreach coord in path
                dist := Distance(me, opponent);
                if (dist <= ideal_distance)
                    break;
                endif

                // Skip hazardous path steps — invalidate cache and break
                if (IsTileHazardous(coord.x, coord.y))
                    path_cache_var := 0;
                    break;
                endif

                step_count += 1;
                if (step_count % 3 == 0 && CheckLineOfSight(me, opponent))
                    SetOpponent(opponent);
                    TurnToward(opponent);
                    los_cache_var := 0;
                    break;
                endif

                if (should_run)
                    RunTowardLocation(coord.x, coord.y);
                else
                    WalkTowardLocation(coord.x, coord.y);
                endif
                sleepms(100);
            endforeach
        else
            var last_loc := enemy_last_loc_var;
            if (last_loc && (current_time - last_loc.time) < 30)
                var last_path := FindPath(me.x, me.y, me.z, last_loc.x, last_loc.y, last_loc.z, me.realm, FP_IGNORE_MOBILES, 50);
                if (last_path)
                    foreach coord in last_path
                        if (CheckLineOfSight(me, opponent))
                            SendEvent(me, struct{ "type" := SYSEVENT_ENGAGED, "source" := opponent });
                            break;
                        endif
                        if (IsTileHazardous(coord.x, coord.y))
                            break;
                        endif
                        if (should_run)
                            RunTowardLocation(coord.x, coord.y);
                        else
                            WalkTowardLocation(coord.x, coord.y);
                        endif
                        sleepms(100);
                    endforeach
                endif
            endif

            if (Abs(me.z - opp_z) >= 19)
                var attempts := 0;
                while (attempts < 5 && !CheckLineOfSight(me, opponent))
                    if (should_run)
                        RunToward(opponent);
                    else
                        WalkToward(opponent);
                    endif
                    sleepms(200);
                    attempts += 1;
                endwhile
            endif

            if (me.hasTunnelTactics())
                TunnelTactics(me, opponent);
            endif
        endif
    endif

    // Progress check (interval-based)
    if (current_time >= tracker.last_progress_check + PURSUIT_PROGRESS_CHECK_INTERVAL)
        var current_dist := Distance(me, opponent);
        if (!moved_this_cycle || current_dist >= dist)
            tracker.no_progress_count += 1;
        else
            tracker.no_progress_count := 0;
        endif
        tracker.last_progress_check := current_time;

        if (tracker.no_progress_count >= PURSUIT_MAX_NO_PROGRESS_COUNT)
            EraseObjProperty(me, "path");
            enemy_last_loc_var := 0;
            path_cache_var := 0;
            TacticalReposition(opponent, 1);
            tracker.no_progress_count := 0;
        endif
    endif

    // Store tracker in script-level var
    pursuit_tracker := tracker;
    return 1;
endfunction


function EnableMainEvents()
    DisableEvents(SYSEVENT_OPPONENT_MOVED + SYSEVENT_DISENGAGED);
    EnableEvents(SYSEVENT_ENGAGED + SYSEVENT_DAMAGED + SYSEVENT_ENTEREDAREA, AREA_SIZE);
    EnableEvents(SYSEVENT_DISENGAGED + SYSEVENT_LEFTAREA, AREA_SIZE);
endfunction


function DisableMainEvents()
    DisableEvents(SYSEVENT_ENGAGED + SYSEVENT_DAMAGED + SYSEVENT_ENTEREDAREA);
    DisableEvents(SYSEVENT_OPPONENT_MOVED + SYSEVENT_DISENGAGED);
endfunction


function HandleIdleState()
    DisableMainEvents();
    EnableEvents(SYSEVENT_ENTEREDAREA + SYSEVENT_ENGAGED + SYSEVENT_DAMAGED, AREA_SIZE);

    // Return to spawn point if we have one
    var spawn_serial := GetObjProperty(me, "spawnpoint");
    var spawn_point := 0;
    if (spawn_serial)
        spawn_point := SystemFindObjectBySerial(spawn_serial);
    endif

    var anchor := GetObjProperty(me, "anchor");
    var home_x, home_y;

    if (spawn_point)
        home_x := spawn_point.x;
        home_y := spawn_point.y;
    elseif (anchor)
        home_x := anchor[1];
        home_y := anchor[2];
    else
        return;
    endif

    // OPTIMIZATION: Check cached threats FIRST (O(1) property access, fast path)
    if (CheckNearbyThreats())
        return; // Found threat, stay alert
    endif

    // OPTIMIZATION: Only check HasPlayerNear if no cached threats (now cached itself)
    // if (!HasPlayerNear(me))
        var attempts := 0;
        // DA pra simplificar, remover esse Coordinate Distance e so fazer ele ir voltando pra casa. SE a distancia for maior que 30, apenas retorna anti exploit
        if (CoordinateDistance(me.x, me.y, home_x, home_y) > 5 && attempts < 20)
            if (CheckNearbyThreats())
                return; // Abort returning home if threats appear
            endif
            WalkTowardLocation(home_x, home_y);
            sleepms(100);
            attempts += 1;
        endif

        // If still not home after 20 attempts, just teleport
        if (CoordinateDistance(me.x, me.y, home_x, home_y) > 5 && attempts >= 20)
            PrintTextAbove(me, "*Retorna correndo* [Sistema Anti-Exploits]");
            sleep(3);
            MoveObjectToLocation(me, home_x, home_y, anchor[3], me.realm, MOVEOBJECT_FORCELOCATION);
        endif
    // endif

    return 1;
endfunction



function MaintainRange(opponent)
    var sleepdelay := 275 - me.run_speed;
    if (sleepdelay < 0)
        sleepdelay := 50;
    endif

    var dist := Distance(me, opponent);
    var z_diff := Abs(me.z - opponent.z);

    var range_info := GetWeaponRangeInfo();
    
    if (me.isHumanoidBrain())
        return MaintainHumanoidDistance(opponent, dist, sleepdelay, range_info);
    endif

    return MaintainSimpleDistance(opponent, dist, sleepdelay);
endfunction


function MaintainHumanoidDistance(opponent, dist, sleepdelay, range_info)
    if (isFireWeapon(me.weapon))
        var balas := CInt(GetObjProperty(me.weapon, "balas"));
        if (balas <= 0)
            UseItem(me.weapon, me);
        endif
    endif

    if (GetObjProperty(me, "metamorfose"))
        // Force melee behavior
        if (dist > 1)
            RunToward(opponent);
            sleepms(sleepdelay);
        else
            TurnToward(opponent);
            return StayNearAllies(4, sleepdelay, opponent);
        endif
        return 0;
    endif

    if (me.weapon.isRanged() || range_info.max_range > 1)
        // Check if we can actually move to ideal range
        var ideal_range := range_info.max_range;
        var can_move_back := CanMoveInDirection(me, opponent, 1); // 1 means moving away
        var can_move_forward := CanMoveInDirection(me, opponent, 0); // 0 means moving toward

        if (dist < ideal_range)
            if (can_move_back)
                TryToRunAway(opponent);
                sleepms(sleepdelay);
            else
                // Cornered — switch to melee instead of standing still getting hit
                SwitchToMelee();
                TurnToward(opponent);
                return 1;
            endif
        elseif (dist > ideal_range)
            if (can_move_forward)
                if (GetObjProperty(me, "metamorfose"))
                    RunToward(opponent);
                    sleepms(sleepdelay);
                else
                    RunToward(opponent);
                    sleepms(sleepdelay);
                endif
            else
                // If we can't move forward, just maintain current position
                TurnToward(opponent);
                return StayNearAllies(4, sleepdelay, opponent);
            endif
        else
            TurnToward(opponent);
            return StayNearAllies(4, sleepdelay, opponent);
        endif
    elseif (me.isSage() && me.weapon.objtype == FIST)
        if (dist < 6)
            TryToRunAway(opponent);
            sleepms(sleepdelay);
        elseif (dist > 6)
            RunToward(opponent);
            sleepms(sleepdelay);
        else
            TurnToward(opponent);
            return StayNearAllies(4, sleepdelay, opponent);
        endif
    else
        if (dist > range_info.max_range)
            RunToward(opponent);
            sleepms(sleepdelay);
        else
            TurnToward(opponent);
            return StayNearAllies(4, sleepdelay, opponent);
        endif
    endif

    return 0;
endfunction


function CanMoveInDirection(me, opponent, moving_away := 0)
    var test_distance := 3; // Check 3 tiles ahead
    var angle := GetFacing(me.x, me.y, opponent.x, opponent.y);
    var test_coords;
    
    if (moving_away)
        // Reverse the angle when moving away
        angle := (angle + 4) % 8;
    endif
    
    case (angle)
        0: test_coords := struct{ "x" := me.x, "y" := me.y - test_distance }; // North
        1: test_coords := struct{ "x" := me.x + test_distance, "y" := me.y - test_distance }; // Northeast
        2: test_coords := struct{ "x" := me.x + test_distance, "y" := me.y }; // East
        3: test_coords := struct{ "x" := me.x + test_distance, "y" := me.y + test_distance }; // Southeast
        4: test_coords := struct{ "x" := me.x, "y" := me.y + test_distance }; // South
        5: test_coords := struct{ "x" := me.x - test_distance, "y" := me.y + test_distance }; // Southwest
        6: test_coords := struct{ "x" := me.x - test_distance, "y" := me.y }; // West
        7: test_coords := struct{ "x" := me.x - test_distance, "y" := me.y - test_distance }; // Northwest
    endcase
    
    // Check if we can actually move there
    return CheckLosAt(me, test_coords.x, test_coords.y, me.z);
endfunction


function MaintainSimpleDistance(opponent, dist, sleepdelay)  
    if (isMonsterRanged())
        // Simple ranged monster logic
        if (dist < 4)
            TryToRunAway(opponent);
            sleepms(sleepdelay);
        elseif (dist > 10)
            RunToward(opponent);
            sleepms(sleepdelay);
        else
            TurnToward(opponent);
            return 1;
        endif
    else
        // Simple melee monster logic
        if (dist > 1)
            RunToward(opponent);
            sleepms(sleepdelay);
        else
            TurnToward(opponent);
            return 1;
        endif
    endif

    return 0;
endfunction


function DoThiefCombatBehavior(opponent)
    if (GetCooldown(me, "thief_behavior"))
        return 0;
    endif

    if (Distance(me, opponent) < 6)
        return 0;
    endif

    SetCooldown(me, "thief_behavior", 30);

    // 50% chance to steal, 50% chance to sneak attack
    if (RandomInt(100) < 50)
        DoThiefSteal(opponent);
    else
        DoThiefSneakAttack(opponent);
    endif
endfunction


function ShamanBehavior(shaman)
    if (GetCooldown(shaman, "spell_cast"))
        return 0;
    endif
    var opponent := shaman.opponent;
    if (!opponent)
        return 0;
    endif
    var spells := GetAvailableShamanSpells(shaman);
    if (!spells.size())
        return 0;
    endif
    var result := ChooseShamanSpell(shaman, opponent, spells);
    if (!result)
        return 0;
    endif
    return NPC_CastSpell(shaman, result);
endfunction


function CheckForWoundedAllies(who)
	var allies := NPC_GetNearbyAllies(who);
	var wounded_allies := array{};

	// Check allies' health (skip undead/demons — healing damages them)
	foreach ally in allies
		if (isHurt(ally))
			var cache := NPC_GetCachedData(ally);
			if (cache.environment_lower != "undead" && cache.environment_lower != "demons")
				wounded_allies.append(ally);
			endif
		endif
	endforeach

	return wounded_allies;
endfunction


function FindMostWoundedAlly(wounded_allies)
    // Initialize our best candidate with a full 100% health
    var most_wounded := struct{
        "mobile" := 0,
        "hp_percent" := 100.0
    };

    foreach ally in wounded_allies
        // Calculate current HP percentage for each ally
        var hp_percent := (CDbl(AP_GetVital(ally, HITS)) / CDbl(AP_GetVitalMaximumValue(ally, HITS))) * 100;
        // Update if this ally has a lower (worse) percentage
        if (hp_percent < most_wounded.hp_percent)
            most_wounded.mobile := ally;
            most_wounded.hp_percent := hp_percent;
        endif
        sleepms(2);
    endforeach

    return most_wounded.mobile;
endfunction


function AskForHealing(me)
    if (!isHurt(me))
        return 0;
    endif

    foreach mobile in GetNearbyMobilesOnce(me, 8, LISTEX_FLAG_NORMAL)
        if (mobile.npctemplate && mobile != me)
            if (mobile.isSage())
                SendEvent(mobile, struct{ "type" := EVID_BUDDYATTACKED, "source" := me });
            endif
        endif
    endforeach
endfunction


function shouldKeepPursuing(opponent, dist)
    if (!opponent)
        return 0;
    endif

    // Don't chase too far
    if (dist > AREA_SIZE + 6)
        return 0;
    endif

    // If we're in melee range and can see opponent, stop pursuing (only for melee)
    // Ranged creatures need to keep pursuing to retreat away
    var ideal_distance := CalculateIdealPursuitDistance();
    if (dist <= 1 && ideal_distance == 1 && CheckLineOfSight(me, opponent))
        return 0;
    endif

    // Prevent flee spam by adding a cooldown
    if (GetCooldown(me, "flee_check"))
        return 1; // Continue pursuing if on cooldown
    endif
    var npcState := getNPCState(me);
    var cd := 15;
    
    if (npcState == "dying")
        cd := 5;
    endif

    SetCooldown(me, "flee_check", cd);
    var allies := NPC_GetNearbyAllies(me);
    if (allies.size() > 0)
        if (!GetCooldown(me, "allies"))
            initial_allies_var := allies.size();
            SetCooldown(me, "allies", 15);
        endif

        var initial_allies := initial_allies_var;
        var already_fighting := GetObjProperty(me, "already_fighting");
        
        // Only consider fleeing if we've lost allies
        if (initial_allies && allies.size() <= CInt(initial_allies/2))
            // Don't flee if already engaged unless heavily outnumbered
            if (!already_fighting || ListHostiles(me, 3).size() >= allies.size() * 2)
                // Higher chance to flee when heavily damaged
                var flee_chance := (npcState == "dying") ? 40 : 15;
                
                if (RandomInt(100) < flee_chance)
                    PrintTextAbove(me, "*"+me.name+" recua*");
                    SM_TransitionTo("FLEEING", struct{ "target" := opponent, "steps" := 0 });
                    return 0;
                endif
            endif
        endif
    else
        // If alone, don't pursue new targets but stay if already fighting
        if (!GetObjProperty(me, "already_fighting"))
            return 0;
        endif
    endif

    return 1;  // Continue pursuing if none of the above conditions are met
endfunction


function LootCorpse(corpse)
    if (!CorpseHasLoot(corpse))
        return 0;
    endif

    if (GetCooldown(me, "loot_cooldown") > 0)
        return 0;
    endif

    // Approach the corpse, but check for threats while approaching
    while (Distance(me, corpse) > 1)
        // Check for nearby hostiles - combat takes priority over looting
        // Phase 4: Use cached scan instead of direct ListMobilesNearLocationEx
        foreach mobile in GetNearbyMobilesOnce(me, AREA_SIZE, LISTEX_FLAG_NORMAL)
            if (CanFight(me, mobile))
                // Hostile nearby - abort looting and engage
                return 0;
            endif
            sleepms(20);
        endforeach

        RunToward(corpse);
        sleepms(100);

        // Break if corpse disappeared or too far
        if (!corpse || Distance(me, corpse) > 20)
            return 0;
        endif
    endwhile

    // Calculate max items to loot based on intelligence
    var max_loot := 1/* CInt(AP_GetStat(me, INTELLIGENCE)/20) */;
    if (max_loot < 1)
        max_loot := 1;
    endif
    var looted := 0;

    // Ensure NPC has a backpack
    if (!me.backpack)
        var backpack := CreateItemAtLocation(me.x, me.y, me.z, "Backpack", 1);
        if (!EquipItem(me, backpack))
            return 0;
        endif
    endif

    // Loot items, but check for threats between each item
    while (looted < max_loot)
        // Check for nearby hostiles before looting each item
        // Phase 4: Use cached scan instead of direct ListMobilesNearLocationEx
        foreach mobile in GetNearbyMobilesOnce(me, AREA_SIZE, LISTEX_FLAG_NORMAL)
            if (CanFight(me, mobile))
                // Hostile nearby - abort looting and let the brain handle combat
                return (looted > 0);
            endif
            sleepms(2);
        endforeach

        PrintTextAbove(me, "*revira o corpo*");
        if (!LootSomething(me, corpse))
            break;
        endif
        looted += 1;
        sleepms(1500);
    endwhile

    return (looted > 0);
endfunction


function CorpseHasLoot(corpse)
    if (!corpse || corpse.IsA(POLCLASS_CORPSE) == error)
        return 0;
    endif

    foreach item in EnumerateItemsInContainer(corpse)
        if (item.movable)
            return 1;
        endif
        sleepms(2);
    endforeach

    return 0;
endfunction


function LootSomething(npc, corpse)
    // Define priority items by objtype (coins)
    var coins := array{0xBA63, 0xBA64, 0xBA65}; // copper, silver, gold
    var resources := array{0x1bf2, 0x1bd7};     // ingots, wood

    if (GetCooldown(npc, "already_looted") > 0)
        return 0;
    endif

    // First try to loot coins
    var priority_loot := FindPriorityLoot(corpse, coins);
    if (priority_loot)
        return LootItem(npc, priority_loot);
    endif

    // Then try resources
    var resource_loot := FindPriorityLoot(corpse, resources);
    if (resource_loot)
        return LootItem(npc, resource_loot);
    endif

    // Then look for equipment by class
    foreach item in EnumerateItemsInContainer(corpse)
        if (!item.movable || item.newbie)
            continue;
        endif

        if (CInt(GetObjProperty(item, "#EquippedOn")) == corpse.serial && item.IsA(POLCLASS_ARMOR))
            continue;
        endif

        if (item.IsA(POLCLASS_WEAPON) || item.IsShield() || item.IsA(POLCLASS_ARMOR))
            return LootItem(npc, item);
        endif

        // Check containers
        if (item.IsA(POLCLASS_CONTAINER))
            var container_item := FindEquipmentInContainer(item);
            if (container_item)
                return LootItem(npc, container_item);
            endif
        endif
        
        // Adicionei pra meter o bicho em alguma coisa pelo menos (bandages, tesoura)
        return LootItem(npc, item);
        sleepms(2);
    endforeach

    return 1;
endfunction


function FindEquipmentInContainer(container)
    foreach item in EnumerateItemsInContainer(container)
        if (!item.movable)
            continue;
        endif

        if (item.IsA(POLCLASS_WEAPON) || item.IsShield() || item.IsA(POLCLASS_ARMOR))
            return item;
        endif

        if (item.IsA(POLCLASS_CONTAINER))
            var container_item := FindEquipmentInContainer(item);
            if (container_item)
                return container_item;
            endif
        endif
        
        sleepms(2);
    endforeach
    
    return 0;
endfunction


function LootItem(npc, item)
    if (!item)
        return 0;
    endif

    if (!npc.backpack)
        var backpack := CreateItemAtLocation(npc.x, npc.y, npc.z, "Backpack", 1);
        EquipItem(npc, backpack);
    endif
    SetObjProperty(item, "LootedBy", npc.serial);

    PrintTextAbove(npc, "*pega " + item.desc + "*");
    SetCooldown(npc, "already_looted", 300);

    return MoveItemToContainer(item, npc.backpack);
endfunction


function FindPriorityLoot(corpse, graphics_array)
    foreach item in EnumerateItemsInContainer(corpse)
        if (!item.movable)
            continue;
        endif

        if (item.objtype in graphics_array || item.graphic in graphics_array)
            return item;
        endif

        // Check containers for these items
        if (item.IsA(POLCLASS_CONTAINER))
            var container_item := FindPriorityLoot(item, graphics_array);
            if (container_item)
                return container_item;
            endif
        endif
        
        sleepms(2);
    endforeach
    
    return 0;
endfunction


function ClearCombatState()
    SetWarMode(0);
    SetOpponent(0);
    // Intentionally NOT clearing threat_table_var: keeps damage history of other
    // attackers so ShouldReevaluateTarget can auto-pick the next-best target on
    // the following tick. DecayThreat self-evicts stale entries (~10s half-life).
endfunction


/*
 * SwitchToMelee()
 *
 * When a ranged NPC is stuck retreating (cornered, no escape path),
 * switch to a melee weapon and fight close range instead of running forever.
 * Resets cached_weapon_range so CalculateIdealPursuitDistance returns 1 (melee).
 * Returns 1 if switched, 0 if no melee weapon available (NPC uses fists).
 */
function SwitchToMelee()
    if (brain_type != "humanoid")
        return 0;
    endif

    var current_weapon := me.weapon;
    if (current_weapon && current_weapon.isRanged())
        if (me.backpack)
            MoveItemToContainer(current_weapon, me.backpack);
        endif
    endif

    var sword := FindObjtypeInContainer(me.backpack, 0x0F60);
    if (!sword)
        sword := CreateItemAtLocation(me.x, me.y, me.z, 0x0F60, 1, me.realm);
    endif
    if (sword)
        EquipItem(me, sword);
    endif

    cached_weapon_range := 0;
    PrintTextAbove(me, "*avanca para o combate*");
    return 1;
endfunction



function GetWeaponRangeInfo()
    // Get the actual equipped item in hand
    var weapon := GetEquipmentByLayer(me, LAYER_HAND1);
    if (!weapon)
        weapon := GetEquipmentByLayer(me, LAYER_HAND2);
    endif

    if (!weapon)
        return struct{ "min_range" := 1, "max_range" := 1 };
    endif

    var itemdesc := ReadConfigFile(":*:itemdesc");
    var wpn_elem := FindConfigElem(itemdesc, weapon.objtype);
    var min_range := 1;
    var max_range := 1;

    if (wpn_elem && wpn_elem != error)
        min_range := CInt(GetConfigInt(wpn_elem, "MinRange")) ? CInt(GetConfigInt(wpn_elem, "MinRange")) : 1;
        max_range := CInt(GetConfigInt(wpn_elem, "MaxRange")) ? CInt(GetConfigInt(wpn_elem, "MaxRange")) : 1;
    endif

    return struct{
        "min_range" := min_range,
        "max_range" := max_range
    };
endfunction


function StayNearAllies(max_distance, sleepdelay, opponent)
    var allies := NPC_GetNearbyAllies(me);
    if (allies.size() > 0)
        var closest_ally := allies[1];
        var ally_dist := Distance(me, closest_ally);
        
        if (ally_dist > max_distance)
            RunToward(closest_ally);
            sleepms(sleepdelay);
            return 0;
        endif
    endif
    
    TurnToward(opponent);
    return 1;
endfunction


function isMonsterRanged()
    if (GetObjProperty(me, "#force_ranged"))
        return 1;
    endif

    var cache := NPC_GetCachedData(me);
    return cache.is_ranged || me.isSage();
endfunction


function TryToRunAway(opponent, chasing := 0)
    if (!runaway_progress_var)
        runaway_progress_var := struct{
            "last_pos" := struct{ "x" := me.x, "y" := me.y },
            "last_check_time" := ReadGameClock(),
            "no_progress_count" := 0
        };
    endif

    if (!chasing)
        RunAwayFrom(opponent);
    endif

    if (ReadGameClock() >= runaway_progress_var.last_check_time + 1)
        var current_dist := CoordinateDistance(me.x, me.y, runaway_progress_var.last_pos.x, runaway_progress_var.last_pos.y);
        if (current_dist <= 3)
            runaway_progress_var.no_progress_count += 1;
        else
            runaway_progress_var.no_progress_count := 0;
        endif

        if (runaway_progress_var.no_progress_count >= 1)
            var alternative_locations := GetAlternativeLocations(opponent);
            if (alternative_locations.size() > 0)
                RunTowardLocation(alternative_locations[1].x, alternative_locations[1].y);
            endif
            runaway_progress_var.no_progress_count := 0;
        endif

        runaway_progress_var.last_pos.x := me.x;
        runaway_progress_var.last_pos.y := me.y;
        runaway_progress_var.last_check_time := ReadGameClock();
    endif
endfunction


/*
 * CheckHealthTeleport(mobile)
 * Checks if mobile should teleport based on current health percentage
 * Returns: 1 if teleported, 0 if not
 */
function CheckHealthTeleport(mobile)
    var teleport_locations := GetObjProperty(mobile, "teleport_locations");
    
    if (!teleport_locations || teleport_locations.size() < 1)
        return 0;
    endif
    
    var max_hp := AP_GetVitalMaximumValue(mobile, HITS);
    var current_hp_percent := CInt((CDbl(AP_GetVital(mobile, HITS)) / CDbl(max_hp)) * 100);
    
    // Find best matching location (highest hp_percent threshold that's <= current_hp_percent)
    var best_location := 0;
    var best_index := -1;
    var i := 1;  // Start from 1 since POL arrays are 1-indexed
    
    foreach location in teleport_locations
        if (current_hp_percent <= location.hp_percent)
            if (!best_location || location.hp_percent > best_location.hp_percent)
                best_location := location;
                best_index := i;
            endif
        endif
        i := i + 1;
    endforeach
    
    if (best_index >= 0)
        // Teleport the mobile to the best location
        if (MoveObjectToLocation(mobile, best_location.x, best_location.y, best_location.z, best_location.realm, MOVEOBJECT_FORCELOCATION))
            PlaySoundEffect(mobile, 0x1FE);
            PlayObjectCenteredEffect(mobile, 0x3728, 7, 0x10);
            
            // Remove this location from the array so it won't be used again
            teleport_locations.erase(best_index);
            SetObjProperty(mobile, "teleport_locations", teleport_locations);
            return 1;
        else
            return 0;
        endif
    endif
    
    return 0;
endfunction


function ShouldFlee(me)
    // should flee when life is below X%
    var flee_health := CInt(GetObjProperty(me, "flee_health"));
    if (flee_health)
        var max_hp := AP_GetVitalMaximumValue(me, HITS);
        var current_hp_percent := CInt((CDbl(AP_GetVital(me, HITS)) / CDbl(max_hp)) * 100);
        if (current_hp_percent <= flee_health)
            return 1;
        endif
    endif
     // if mob is neutral alignment, alaways flee
    if (GetAlignment(me) == ALIGN_NEUTRAL)
        return 1;
    endif

    return 0;
endfunction


function startPatrolling(byref me, patrol_points, current_patrol_point)
    var patrol_direction := GetObjProperty(me, "#patrol_direction");
    if (patrol_direction == error)
        patrol_direction := 1; // 1 = forward, -1 = backward
        SetObjProperty(me, "#patrol_direction", patrol_direction);
    endif
    
    var dest := patrol_points[current_patrol_point];
    
    // Check if we've reached the destination (within 1 tile)
    if (CoordinateDistance(me.x, me.y, dest.x, dest.y) <= 1)
        // Determine next patrol point based on direction
        if (patrol_direction == 1)
            current_patrol_point += 1;
            // If reached the end, reverse direction
            if (current_patrol_point > patrol_points.size())
                current_patrol_point := patrol_points.size() - 1;
                patrol_direction := -1;
                SetObjProperty(me, "#patrol_direction", patrol_direction);
            endif
        else // patrol_direction == -1
            current_patrol_point -= 1;
            // If reached the beginning, reverse direction
            if (current_patrol_point < 1)
                current_patrol_point := 2;
                patrol_direction := 1;
                SetObjProperty(me, "#patrol_direction", patrol_direction);
            endif
        endif
        SetObjProperty(me, "#current_patrol_point", current_patrol_point);
    endif

    // Walk toward destination
    WalkTowardLocation(dest.x, dest.y);
endfunction


// Função específica para detecção dos Aklats
function DetectAklatsThreats(npc)
    // Verifica se já tem oponente ativo
    if (npc.opponent)
		return 1;
    endif
    
   // Verifica se foi alertado por som (taunt_opponent)
    var taunt_opponent := GetObjProperty(npc, "#taunt_opponent");
    if (taunt_opponent && !npc.warmode)
        PrintTextAbove(npc, "*Alertado pelo som!*");
        return 1;
    endif
    
    // Detecção de pegadas alinhadas
    var pegadas := {0x1E03, 0x1E04, 0x1E05, 0x1E06}; 
    var footsteps := array;
    
    // Coleta todas as pegadas na área
    foreach objtype in pegadas
        foreach pegada in ListItemsNearLocationOfType(npc.x, npc.y, npc.z, 10, objtype, npc.realm)
            footsteps.append(pegada);
        endforeach
        sleepms(2);
    endforeach
    
    // Verifica se há pegadas suficientes e alinhadas
    if (footsteps.size() >= 3 && CheckAlinhadas(footsteps))
        PrintTextAbove(npc, "*Alertado pelas vibrações!*");
        return 1;
    endif
    
    return 0;
endfunction


//FUNCÕES PARA AKLATS E PEGADAS PARA ACIONAR O ATAQUE
// Função para verificar se 3 pegadas estão alinhadas (em linha reta)
function CheckAlinhadas(footsteps)
    if (footsteps.size() < 3)
        return 0; // Não há pegadas suficientes
    endif
    
    // Pega as 3 pegadas mais recentes (assumindo que são as últimas na lista)
    var step1 := footsteps[footsteps.size() - 3];
    var step2 := footsteps[footsteps.size() - 2];
    var step3 := footsteps[footsteps.size() - 1];
    
    // Verifica se os 3 pontos estão alinhados
    return ArePointsAligned(step1, step2, step3);
endfunction


// Função para verificar se 3 pontos estão em linha reta
function ArePointsAligned(p1, p2, p3)
    // Calcula os vetores entre os pontos
    var dx1 := p2.x - p1.x;
    var dy1 := p2.y - p1.y;
    var dx2 := p3.x - p2.x;
    var dy2 := p3.y - p2.y;
    
    // Se os vetores são proporcionais, os pontos estão alinhados
    // Usando produto cruzado: se dx1*dy2 - dy1*dx2 == 0, então estão alinhados
    var crossProduct := (dx1 * dy2) - (dy1 * dx2);
    
    // Considera alinhado se o produto cruzado for 0 (ou muito próximo de 0)
    // Valor absoluto manual já que POL pode não ter abs()
    if (crossProduct < 0)
        crossProduct := -crossProduct;
    endif
    
    return (crossProduct <= 1); // Tolerância de 1 para compensar arredondamentos
endfunction


function DoppelgangerTransformation(mobile, opponent)
    if (!mobile || !opponent || !mobile.isDoppelganger())
        return 0;
    endif

    // Don't transform if already transformed
    if (GetObjProperty(mobile, "#doppelganger_transformed"))
        return 0;
    endif

    // Don't transform NPCs or already dead opponents
    if (opponent.npctemplate || opponent.dead || opponent.concealed || opponent.hidden)
        return 0;
    endif

    if (Distance(mobile, opponent) > 10)
        return 0;
    endif

    // Store original appearance and equipment for cleanup
    var original_data := struct{
        "graphic" := mobile.graphic,
        "color" := mobile.color,
        "name" := mobile.name,
        "title_race" := mobile.title_race,
        "title_suffix" := mobile.title_suffix,
        "title_prefix" := mobile.title_prefix,
        "gender" := mobile.gender,
        "trueobjtype" := mobile.trueobjtype,
        "truecolor" := mobile.truecolor
    };

    // Create safe storage area for original equipment
    var storage_pack := CreateItemAtLocation(mobile.x, mobile.y, mobile.z, 0xe75, 1, mobile.realm);
    if (!storage_pack)
        return 0;
    endif
    MoveObjectToLocation(storage_pack, 4230, 25, 1, mobile.realm, MOVEOBJECT_FORCELOCATION);

    // Store original equipment
    var original_items := ListEquippedItems(mobile);
    foreach item in original_items
        MoveItemToContainer(item, storage_pack);
        sleepms(2);
    endforeach

    // Copy opponent's appearance
    mobile.gender := opponent.gender;
    mobile.graphic := opponent.graphic;
    mobile.color := opponent.color;
    mobile.title_race := opponent.title_race;
    mobile.title_suffix := opponent.title_suffix;
    mobile.title_prefix := opponent.title_prefix;
    mobile.name := opponent.name;
    mobile.TrueColor := opponent.TrueColor;
    mobile.trueobjtype := opponent.trueobjtype;

    // Copy opponent's description if available
    var desc := GetObjProperty(opponent, "description");
    if (desc != error)
        SetObjProperty(mobile, "description", desc);
    else
        EraseObjProperty(mobile, "description");
    endif

    // Copy and equip opponent's equipment
    var copied_items := array{};
    var opponent_items := ListEquippedItems(opponent);
    foreach item in opponent_items
        var copied_item := CreateItemCopyAtLocation(mobile.x, mobile.y, mobile.z, item);
        if (copied_item)
            copied_item.color := item.color;
            copied_item.name := item.name;
            copied_item.desc := item.desc;
            EquipItem(mobile, copied_item);
            copied_items.append(copied_item.serial);
        endif
        sleepms(2);
    endforeach

    // Set combat behavior based on opponent's class
    var opponent_class := GetObjProperty(opponent, "classe");
    var combat_properties := struct{
        "original_behavior" := isMonsterRanged() ? "ranged" : "melee",
        "adopted_behavior" := "melee",
        "spell_school" := "",
        "is_ranged" := 0
    };

    // Determine combat style
    if (opponent_class == "Sabio")
        combat_properties.adopted_behavior := "mage";
        combat_properties.spell_school := GetObjProperty(opponent, "SpellSchool");
        if (combat_properties.spell_school)
            SetObjProperty(mobile, "SpellSchool", combat_properties.spell_school);
            InitSpellCache();
        endif
        combat_properties.is_ranged := 1;
    elseif (opponent.weapon && opponent.weapon.isRanged())
        combat_properties.adopted_behavior := "ranged";
        combat_properties.is_ranged := 1;
    else
        combat_properties.adopted_behavior := "melee";
        combat_properties.is_ranged := 0;
    endif

    // Store transformation data for cleanup
    var transformation_data := struct{
        "original_data" := original_data,
        "storage_pack" := storage_pack,
        "copied_items" := copied_items,
        "combat_properties" := combat_properties,
        "transformation_time" := ReadGameClock()
    };

    SetObjProperty(mobile, "#doppelganger_transformed", 1);
    SetObjProperty(mobile, "#doppelganger_data", transformation_data);

    // Set combat behavior override
    if (combat_properties.is_ranged)
        SetObjProperty(mobile, "#force_ranged", 1);
    else
        EraseObjProperty(mobile, "#force_ranged");
    endif

    return 1;
endfunction


function RestoreDoppelgangerAppearance(mobile)
    var transformation_data := GetObjProperty(mobile, "#doppelganger_data");
    if (!transformation_data || !transformation_data.original_data)
        return 0;
    endif

    var original := transformation_data.original_data;
    mobile.graphic := original.graphic;
    mobile.color := original.color;
    mobile.name := original.name;
    mobile.title_race := original.title_race;
    mobile.title_suffix := original.title_suffix;
    mobile.title_prefix := original.title_prefix;
    mobile.gender := original.gender;
    mobile.trueobjtype := original.trueobjtype;
    mobile.truecolor := original.truecolor;

    return 1;
endfunction


function CleanupDoppelgangerEquipment(mobile)
    var transformation_data := GetObjProperty(mobile, "#doppelganger_data");
    if (!transformation_data)
        return 0;
    endif

    // Destroy all copied equipment (using serials to find items)
    if (transformation_data.copied_items)
        foreach item_serial in (transformation_data.copied_items)
            var item := SystemFindObjectBySerial(item_serial);
            if (item)
                DestroyItem(item);
            endif
            sleepms(2);
        endforeach
    endif

    // Destroy the storage pack and its contents
    if (transformation_data.storage_pack)
        var storage_pack := transformation_data.storage_pack;
        if (storage_pack)
            DestroyItem(storage_pack);
        endif
    endif

    // Clear transformation properties
    EraseObjProperty(mobile, "#doppelganger_transformed");
    EraseObjProperty(mobile, "#doppelganger_data");
    EraseObjProperty(mobile, "#force_ranged");
    EraseObjProperty(mobile, "SpellSchool");

    return 1;
endfunction


function CleanupDoppelgangerTransformation(mobile)
    // This function calls both appearance restoration and equipment cleanup
    // for backward compatibility
    RestoreDoppelgangerAppearance(mobile);
    CleanupDoppelgangerEquipment(mobile);
    return 1;
endfunction


/*
 * ShouldGoDormant(last_activity_time)
 *
 * Purpose: Check if NPC should go dormant due to inactivity
 * Returns 1 if dormant (caller should return), 0 otherwise
 */
function ShouldGoDormant(last_activity_time)
    var current_time := polcore().systime;
    var time_since_activity := CInt(current_time - last_activity_time);

    if (time_since_activity >= 120)  // 2 minutes
        return 1;
    endif

    return 0;
endfunction


/*
 * CheckDarknessCreatureSurvival(mobile)
 *
 * Purpose: Kill darkness creatures (undead/Mobnoturno) if not in darkness
 * Called every AI loop iteration - O(1) cache lookup
 * Returns 1 if creature was killed (caller should return), 0 otherwise
 */
function CheckDarknessCreatureSurvival(mobile)
    var cache := NPC_GetCachedData(mobile);  // O(1) lookup
    var isDarknessCreature := GetObjProperty(mobile, "Mobnoturno") || cache.environment_lower == "undead";

    if (!isDarknessCreature)
        return 0;
    endif

    if (!InDarkness(mobile))
        SetObjProperty(mobile, "nocorpse", 1);
        ApplyRawDamage(mobile, CInt(AP_GetVitalMaximumValue(mobile, "Hits")));
        return 1;
    endif

    return 0;
endfunction

/*
 * WanderAroundAnchor(mobile)
 *
 * Purpose: Handle random wandering for idle NPCs, respecting anchor range
 * Checks if the destination is within the allowed range from the anchor point
 */
function WanderAroundAnchor(mobile)
    var anchor := anchor_var;
    if (!anchor)
        anchor := GetObjProperty(mobile, "anchor");
    endif
    var range := 6;
    if (anchor[4])
        range := anchor[4];
    endif

    // If outside range, walk back to anchor
    if (CoordinateDistance(mobile.x, mobile.y, anchor[1], anchor[2]) > range)
        WalkTowardLocation(anchor[1], anchor[2]);
        return;
    endif

    // Random wandering when no targets
    if (RandomInt(100) < 30)
        var direction := RandomInt(8);
        var dest_x := mobile.x;
        var dest_y := mobile.y;

        case (direction)
            0: dest_y -= 1;
            1: dest_x += 1; dest_y -= 1;
            2: dest_x += 1;
            3: dest_x += 1; dest_y += 1;
            4: dest_y += 1;
            5: dest_x -= 1; dest_y += 1;
            6: dest_x -= 1;
            7: dest_x -= 1; dest_y -= 1;
        endcase

        if (CoordinateDistance(dest_x, dest_y, anchor[1], anchor[2]) <= range)
            WalkTowardLocation(dest_x, dest_y);
        endif
    endif
endfunction


function _calculateDHDiffLocal(who, mobile)
    var mobile_skill := CInt(AP_GetSkill(mobile, SNEAK));

    if (GetObjProperty(mobile, "#overridehiding") != error)
        mobile_skill := CInt(GetObjProperty(mobile, "#overridehiding"));
    endif
    var dist := Distance(who, mobile);
    var difficulty;

    if (MobileWrapper(mobile).HasAbility(ABILITY_ID::SMOKE_SCREEN) && !MobileWrapper(who).HasAbility(ABILITY_ID::SEMPRE_ALERTA))
        difficulty := CInt(mobile_skill);
    else
        if (dist >= 8)
            difficulty := CInt(mobile_skill);
        elseif (dist >= 7)
            difficulty := CInt(mobile_skill * 0.95);
        elseif (dist >= 6)
            difficulty := CInt(mobile_skill * 0.9);
        elseif (dist >= 5)
            difficulty := CInt(mobile_skill * 0.85);
        elseif (dist >= 4)
            difficulty := CInt(mobile_skill * 0.8);
        elseif (dist >= 3)
            difficulty := CInt(mobile_skill * 0.7);
        elseif (dist >= 2)
            difficulty := CInt(mobile_skill * 0.6);
        elseif (dist >= 1)
            difficulty := CInt(mobile_skill * 0.5);
        else
            difficulty := CInt(mobile_skill * 0.25);
        endif
    endif

    return difficulty;
endfunction

