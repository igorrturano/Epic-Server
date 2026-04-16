/*
 * Ability Dispatch Table
 *
 * Purpose: Replace sequential if(me.canX()) chains with pre-built arrays.
 * Tables are built ONCE at brain activation from cached NPC data.
 * Per-tick cost: iterate 1-4 entries (this NPC's actual abilities) vs 12+ checks.
 *
 * Each ability entry has: id (dispatch key), phase (combat/enter),
 * range_min/range_max, cooldown_key.
 */

// Build combat ability table from NPC cache - called once at brain activation
function BuildCombatAbilities(who)
    var cache := NPC_GetCachedData(who);
    var ab := cache.abilities;
    var table := array{};

    // Ranged abilities (distance 2-6)
    if (ab.canSpitAcid)
        table.append(struct{ "id" := "AcidSpray", "range_min" := 2, "range_max" := 6 });
    endif
    if (ab.CanShootWeb)
        table.append(struct{ "id" := "WebShot", "range_min" := 2, "range_max" := 6 });
    endif
    if (ab.canThrow)
        table.append(struct{ "id" := "ThrowSpecial", "range_min" := 2, "range_max" := 8 });
    endif

    // Melee abilities (distance 0-1)
    if (ab.canBite)
        table.append(struct{ "id" := "Bite", "range_min" := 0, "range_max" := 1 });
    endif
    if (ab.CanTongueGrab)
        table.append(struct{ "id" := "TongueGrab", "range_min" := 0, "range_max" := 5 });
    endif
    if (ab.CanPoisonBite)
        table.append(struct{ "id" := "PoisonBite", "range_min" := 0, "range_max" := 1 });
    endif
    if (ab.CanCharm)
        table.append(struct{ "id" := "CharmOpponent", "range_min" := 0, "range_max" := 8 });
    endif

    // Passive/aura abilities (any range)
    if (ab.isMyrkonid)
        table.append(struct{ "id" := "SporeSpecial", "range_min" := 0, "range_max" := 8 });
    endif
    if (ab.hasHiveMind)
        table.append(struct{ "id" := "HiveMind", "range_min" := 0, "range_max" := 99 });
    endif
    if (ab.canDoConfusion)
        table.append(struct{ "id" := "doConfusion", "range_min" := 0, "range_max" := 4 });
    endif
    if (ab.CanClimbWall)
        table.append(struct{ "id" := "WallClimb", "range_min" := 0, "range_max" := 8 });
    endif
    if (ab.canPetrify)
        table.append(struct{ "id" := "petrify", "range_min" := 0, "range_max" := 4 });
    endif

    // Frighten (dragons, etc.) - checked via method since not all templates are in cache
    if (who.CanFrighten())
        table.append(struct{ "id" := "frightenOpponent", "range_min" := 0, "range_max" := 6 });
    endif

    // Dragon abilities — ordered by priority: breath (far) > buffet (mid) > swipe (close)
    if (ab.isDragon)
        table.append(struct{ "id" := "BreathWeapon", "range_min" := 3, "range_max" := 8 });
        table.append(struct{ "id" := "WingBuffet", "range_min" := 1, "range_max" := 4 });
        table.append(struct{ "id" := "TailSwipe", "range_min" := 0, "range_max" := 2 });
    endif

    // Undead abilities
    if (ab.isUndead)
        table.append(struct{ "id" := "FearAura", "range_min" := 0, "range_max" := 4 });
        table.append(struct{ "id" := "UndeadImpedirCura", "range_min" := 0, "range_max" := 1 });
        table.append(struct{ "id" := "MaldicaoMetamorfose", "range_min" := 0, "range_max" := 4 });
    endif

    // Demon abilities
    if (ab.isDemon)
        table.append(struct{ "id" := "BanishSummon", "range_min" := 0, "range_max" := 6 });
        table.append(struct{ "id" := "CurseHeal", "range_min" := 0, "range_max" := 6 });
        table.append(struct{ "id" := "InfernalBurst", "range_min" := 0, "range_max" := 3 });
    endif

    // Beast abilities
    if (ab.isBeast)
        table.append(struct{ "id" := "PackTactics", "range_min" := 0, "range_max" := 4 });
        table.append(struct{ "id" := "Enrage", "range_min" := 0, "range_max" := 99 });
    endif

    // Gazer abilities
    if (ab.isGazer)
        table.append(struct{ "id" := "GazerRay", "range_min" := 0, "range_max" := 8 });
    endif

    // NPCRace-based abilities (ab.npc_race is a lowercase trimmed string)
    _BuildRaceAbilities(ab.npc_race, table);

    // Caster behaviors - these manage their own cooldowns internally
    if (ab.hasMageConfig)
        table.append(struct{ "id" := "MageBehavior", "range_min" := 0, "range_max" := 99 });
    endif

    return table;
endfunction

// Build enter-area ability table - called once at brain activation
function BuildEnterAbilities(who)
    var cache := NPC_GetCachedData(who);
    var ab := cache.abilities;
    var table := array{};

    if (ab.canCharge)
        table.append("Charge");
    endif
    if (ab.hasTunnelTactics)
        table.append("TunnelTactics");
    endif
    if (ab.CanAmbush)
        table.append("Ambush");
    endif

    return table;
endfunction

// Execute combat abilities from pre-built table. Returns 1 if any ability fired.
function TickCombatAbilities(who, opponent, ability_table)
    var dist := Distance(who, opponent);

    foreach ab in ability_table
        if (dist >= ab.range_min && dist <= ab.range_max)
            var cd := CInt(ab.cd);
            var cd_key := ab.cd_key;
            if (!cd_key) cd_key := ab.id; endif
            if (ExecuteCombatAbility(ab.id, who, opponent, cd, cd_key))
                return 1;
            endif
        endif
    endforeach

    return 0;
endfunction

// Dispatch to actual ability function. Returns 1 if ability executed.
function ExecuteCombatAbility(id, who, opponent, cd := 0, cd_key := "")
    case (id)
        "AcidSpray":          return AcidSpray(who, opponent);
        "WebShot":            return WebShot(who, opponent);
        "ThrowSpecial":       return ThrowSpecial(who, opponent);
        "Bite":               return Bite(who, opponent);
        "TongueGrab":         return TongueGrab(who, opponent);
        "PoisonBite":         return PoisonBite(who, opponent);
        "CharmOpponent":      return CharmOpponent(who, opponent);
        "SporeSpecial":       return SporeSpecial(who, opponent);
        "HiveMind":           return HiveMind(who);
        "doConfusion":        return doConfusion(opponent);
        "WallClimb":          return WallClimb(who);
        "petrify":            return petrify(who, opponent);
        "frightenOpponent":   return frightenOpponent(who, opponent);
        "MageBehavior":       return MageBehavior(who, opponent);
        // Dragon (external scripts — shared cooldown spaces abilities 8s apart)
        "BreathWeapon":       return _StartDragonAbility(":epicAI:abilities/BreathWeapon", who, opponent, "breath_weapon", 25);
        "TailSwipe":          return _StartDragonAbility(":epicAI:abilities/TailSwipe", who, opponent, "tail_swipe", 18);
        "WingBuffet":         return _StartDragonAbility(":epicAI:abilities/WingBuffet", who, opponent, "wing_buffet", 20);
        // Undead
        "FearAura":           return _StartFearAura(who, opponent);
        "UndeadImpedirCura":  return _StartAbilityScript(":epicAI:abilities/UndeadImpedirCura", who, opponent, "impedir_cura", 30);
        "MaldicaoMetamorfose": return _StartAbilityScript(":epicAI:abilities/MaldicaoMetamorfose", who, opponent, "maldic_metamorfose", 45);
        // Demon
        "BanishSummon":       return _StartAbilityScript(":epicAI:abilities/BanishSummon", who, opponent, "banish_summon", 30);
        "CurseHeal":          return _StartAbilityScript(":epicAI:abilities/CurseHeal", who, opponent, "curse_heal", 25);
        "InfernalBurst":      return _StartAbilityScript(":epicAI:abilities/InfernalBurst", who, opponent, "infernal_burst", 20);
        // Beast
        "PackTactics":        return _StartAbilityScript(":epicAI:abilities/PackTactics", who, opponent, "pack_tactics", 5);
        "Enrage":             return _StartAbilityScript(":epicAI:abilities/Enrage", who, opponent, "enrage", 15);
        // Gazer
        "GazerRay":           return _StartAbilityScript(":epicAI:abilities/GazerRay", who, opponent, "gazer_ray", 8);
        // Race-based abilities: DiveAttack/FrostAura need HP checks
        "DiveAttack":         return _StartDiveAttack(who, opponent);
        "FrostAura":          return _StartFrostAura(who, opponent);
        // All other race abilities use convention: script path = ":epicAI:abilities/" + id
        default:              return _StartRaceAbility(id, who, opponent, cd, cd_key);
    endcase
    return 0;
endfunction

// Execute enter-area abilities from pre-built table
function TickEnterAbilities(who, source, enter_table)
    foreach ab in enter_table
        case (ab)
            "Charge":        StartCharge(who, source);
            "TunnelTactics": TunnelTactics(who, source);
            "Ambush":        HandleAmbush(who, source);
        endcase
    endforeach
endfunction

// Helper: start an ability as external script with cooldown check
function _StartAbilityScript(script_path, who, opponent, cooldown_name, cooldown_time)
	if (GetCooldown(who, cooldown_name) > 0)
		return 0;
	endif

	SetCooldown(who, cooldown_name, cooldown_time);
	var result := start_script(script_path, array{ who, opponent });
	if (!result || result.errortext)
		return 0;
	endif

	return 1;
endfunction

// Dragon abilities use a shared cooldown so they space out naturally
function _StartDragonAbility(script_path, who, opponent, cooldown_name, cooldown_time)
	// Shared cooldown prevents all dragon abilities for 8 seconds after any fires
	if (GetCooldown(who, "dragon_ability") > 0)
		return 0;
	endif

	if (GetCooldown(who, cooldown_name) > 0)
		return 0;
	endif

	SetCooldown(who, cooldown_name, cooldown_time);
	SetCooldown(who, "dragon_ability", 8);

	var result := start_script(script_path, array{ who, opponent });
	if (!result || result.errortext)
		return 0;
	endif

	return 1;
endfunction

// FearAura needs HP check before starting script
function _StartFearAura(who, opponent)
	if (GetCooldown(who, "fear_aura") > 0 || !opponent)
		return 0;
	endif

	var cur_hp := GetVital(who, HITS) / 100;
	var max_hp := GetVitalMaximumValue(who, HITS) / 100;
	if (cur_hp > max_hp / 2)
		return 0;
	endif

	SetCooldown(who, "fear_aura", 30);
	start_script(":epicAI:abilities/FearAura", array{ who, opponent });
	return 1;
endfunction

// DiveAttack only fires when HP > 50%
function _StartDiveAttack(who, opponent)
	if (GetCooldown(who, "dive_attack") > 0 || !opponent)
		return 0;
	endif

	var cur_hp := GetVital(who, HITS) / 100;
	var max_hp := GetVitalMaximumValue(who, HITS) / 100;
	if (cur_hp < max_hp / 2)
		return 0;
	endif

	SetCooldown(who, "dive_attack", 15);
	start_script(":epicAI:abilities/DiveAttack", array{ who, opponent });
	return 1;
endfunction

// FrostAura only fires when HP < 60%
function _StartFrostAura(who, opponent)
	if (GetCooldown(who, "frost_aura") > 0 || !opponent)
		return 0;
	endif

	var cur_hp := GetVital(who, HITS) / 100;
	var max_hp := GetVitalMaximumValue(who, HITS) / 100;
	if (cur_hp > CInt(max_hp * 0.6))
		return 0;
	endif

	SetCooldown(who, "frost_aura", 15);
	start_script(":epicAI:abilities/FrostAura", array{ who, opponent });
	return 1;
endfunction

// Generic handler for race-based abilities: derives script path from id
function _StartRaceAbility(id, who, opponent, cd, cd_key := "")
	if (cd < 1)
		cd := 15;
	endif
	if (!cd_key)
		cd_key := id;
	endif
	return _StartAbilityScript(":epicAI:abilities/" + id, who, opponent, cd_key, cd);
endfunction

// Build race-based ability table entries with cooldowns
function _BuildRaceAbilities(race, byref table)
	case (race)
		"goblin":
			table.append(struct{ "id" := "DirtyTrick", "range_min" := 0, "range_max" := 2, "cd" := 12 });
		"hobgoblin":
			table.append(struct{ "id" := "WarCry", "range_min" := 0, "range_max" := 6, "cd" := 25 });
		"orog":
		"bugbear":
		"urso":
			table.append(struct{ "id" := "Enrage", "range_min" := 0, "range_max" := 99, "cd" := 15 });
		"gremlin":
			table.append(struct{ "id" := "Sabotage", "range_min" := 0, "range_max" := 1, "cd" := 20 });
		"goblettin":
			table.append(struct{ "id" := "DoubleStrike", "range_min" := 0, "range_max" := 1, "cd" := 15 });
		"varag":
			table.append(struct{ "id" := "PackTactics", "range_min" := 0, "range_max" := 4, "cd" := 5 });
		"ratóide":
			table.append(struct{ "id" := "DirtyTrick", "range_min" := 0, "range_max" := 2, "cd" := 30, "cd_key" := "ratoide_atk" });
			table.append(struct{ "id" := "Gnaw", "range_min" := 0, "range_max" := 1, "cd" := 30, "cd_key" := "ratoide_atk" });
			table.append(struct{ "id" := "SwarmFrenzy", "range_min" := 0, "range_max" := 4, "cd" := 5 });
			table.append(struct{ "id" := "DiseaseSpread", "range_min" := 0, "range_max" := 1, "cd" := 60 });
		"escamante":
			table.append(struct{ "id" := "TailWhip", "range_min" := 0, "range_max" := 2, "cd" := 12 });
			table.append(struct{ "id" := "Camouflage", "range_min" := 0, "range_max" := 99, "cd" := 30 });
		"kuo-toa":
			table.append(struct{ "id" := "SlipperyGrasp", "range_min" := 2, "range_max" := 4, "cd" := 15 });
		"wastrilith":
			table.append(struct{ "id" := "TidalPull", "range_min" := 3, "range_max" := 6, "cd" := 15 });
		"harpia":
			table.append(struct{ "id" := "ScreechAttack", "range_min" := 2, "range_max" := 4, "cd" := 20 });
			table.append(struct{ "id" := "DiveAttack", "range_min" := 3, "range_max" := 6, "cd" := 15 });
		"wendigo":
			table.append(struct{ "id" := "FrostAura", "range_min" := 0, "range_max" := 3, "cd" := 15 });
		"troll":
		"troll das cavernas":
		"troll marinho":
		"troll do deserto":
		"troll selvagem":
			table.append(struct{ "id" := "Regeneration", "range_min" := 0, "range_max" := 99, "cd" := 8 });
			table.append(struct{ "id" := "Rend", "range_min" := 0, "range_max" := 1, "cd" := 15 });
		"sluagh":
			table.append(struct{ "id" := "LifeDrain", "range_min" := 0, "range_max" := 1, "cd" := 12 });
		"garravalha":
			table.append(struct{ "id" := "GroundSlam", "range_min" := 0, "range_max" := 3, "cd" := 18 });
	endcase
endfunction
