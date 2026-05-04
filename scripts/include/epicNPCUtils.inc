use util;

include ":attributes:attributes";
include ":equipsys:Controller/WeaponController";
include ":combat:Model/Damage";

function customizeNpc(mob, set_tier := "0")
	var npc_elem := NPC_GetNPCConfig(mob);
	var classe := npc_elem.Classe;
	var tier := set_tier == "0" ? CInt(mob.getTier()) : CInt(set_tier);
	SetObjProperty(mob, "NpcTier", tier);
	EraseObjProperty(mob, "#npc_cache");
	var tactics := RandomIntMinMax(tier * 20, tier * 30);
	var skill_combat;
	var skill_support;
	 
	var mob_skill := 0;
	if (tier == 1)
		mob_skill += RandomIntMinMax(20, 40+1);
	elseif (tier == 2)
		mob_skill += RandomIntMinMax(40, 60+1);
	elseif (tier == 3)
		mob_skill += RandomIntMinMax(60, 80+1);
	elseif (tier == 4)
		mob_skill += RandomIntMinMax(80, 100+1);
	elseif (tier == 5)
		mob_skill += RandomIntMinMax(100, 120+1);
	else
		mob_skill += RandomIntMinMax(0, 20+1);
	endif

	if (classe == SABIO)
		AP_SetTrueSkill(mob, MAGERY, mob_skill);
		AP_SetTrueSkill(mob, ARCANA, mob_skill);
		AP_SetTrueSkill(mob, THEOLOGY, mob_skill);
		skill_combat := MAGERY;
		skill_support := ARCANA;
	elseif(classe == LADINO || mob.isSpider())
		AP_SetTrueSkill(mob, SNEAK, mob_skill);
		AP_SetTrueSkill(mob, AWARENESS, mob_skill);
		AP_SetTrueSkill(mob, PICKPOCKETING, mob_skill);
	else
		skill_combat := WRESTLING;
		skill_support := TACTICS;
		AP_SetTrueSkill(mob, SURVIVAL, mob_skill);
	endif

	AP_SetTrueSkill(mob, WRESTLING, mob_skill);
	AP_SetTrueSkill(mob, TACTICS, tactics);

    var calculate_level_hp := Cint(AP_GetSkill(mob, skill_combat) + AP_GetSkill(mob, skill_support)) - 80;
	var npc_level := (calculate_level_hp / 6);

	var hp := 0;
	if (classe == SABIO)
		hp := 15;
	elseif (classe == LADINO)
		hp := 20;
	elseif (classe == GUERREIRO)
		hp := 25;
	endif

	var hitsmod := GetObjProperty(mob, "hitsmod");
	if (!hitsmod)
		hitsmod := 0;
	endif
    if (npc_level >= 1 )
        var new_hp := 0;
        var multiplier := Cint(npc_level);
        if (classe == GUERREIRO)
            new_hp := 10 * multiplier;
        elseif (classe == LADINO)
            new_hp := 8 * multiplier;
        elseif (classe == SABIO)
            new_hp := 6 * multiplier;
		else
			new_hp := 0;
        endif
        hp += Cint(new_hp);
    endif

	SetObjProperty(mob, "hitsmod", hitsmod + hp);
	SetObjProperty(mob, "anchor", array{ mob.x, mob.y, mob.z+2 });
	RecalcVitals(mob);
    HealDamage(mob, hitsmod + hp);
endfunction

function ReturnCharDataPlayerNear(who)
    var nearby := ListMobilesNearLocationEx(who.x, who.y, LIST_IGNORE_Z, 30, LISTEX_FLAG_NORMAL + LISTEX_FLAG_HIDDEN + LISTEX_FLAG_CONCEALED, who.realm);
    foreach mobile in nearby
        if (mobile.acctname)
			return mobile.acctname;
        endif
    endforeach
    return 0;
endfunction

function HasPlayerNear(who)
    // OPTIMIZATION: Use 1-second cached scan to reduce expensive 30-tile scans
    var cache_key := "#hasnear_30";
    var cached := GetObjProperty(who, cache_key);
    var now := polcore().systime;

    if (cached && (CDbl(now) - CDbl(cached.time)) < 1.0)
        return cached.result;
    endif

    // Cache miss - perform scan
    var nearby := ListMobilesNearLocationEx(who.x, who.y, LIST_IGNORE_Z, 30, LISTEX_FLAG_NORMAL + LISTEX_FLAG_HIDDEN + LISTEX_FLAG_CONCEALED, who.realm);
    var has_player := 0;
    foreach mobile in nearby
        if (mobile.acctname)
            has_player := 1;
            break;
        endif
    endforeach

    SetObjProperty(who, cache_key, struct{"result" := has_player, "time" := now});
    return has_player;
endfunction

function HasPlayerNearLocation(x, y, realm := 0)
	var mobs := ListMobilesNearLocationEx( x, y, LIST_IGNORE_Z, 15, LISTEX_FLAG_NORMAL|LISTEX_FLAG_HIDDEN|LISTEX_FLAG_PLAYERS_ONLY, realm );
	foreach mob in mobs
		if ( !mob.cmdlevel && !mob.concealed ) //se for player e priv de player
			return 1;
		endif
      sleepms(2);
	endforeach

	return 0;
endfunction

// Valores em % de reducao de dano (0-100). Aplicados multiplicativamente em
// DamageController.CalculateDefenderResist via Damage.Decrease*PercentDamage.
// Ver design_combate.md / design_npcs.md para o redesign completo.
enum RESISTS_TIERS
	RESIST_TIER0 := 0,
	RESIST_TIER1 := 20,
	RESIST_TIER2 := 40,
	RESIST_TIER3 := 60,
	RESIST_TIER4 := 80,
	RESIST_TIER5 := 100,
endenum

enum TIERS
	T0 := "T0",
	T1 := "T1",
	T2 := "T2",
	T3 := "T3",
	T4 := "T4",
	T5 := "T5",
endenum

enum ELEMDAMAGES
	DAMAGE_NONE := "DAMAGE_NONE",
	DAMAGE_LOW := "DAMAGE_LOW",
	DAMAGE_MID := "DAMAGE_MID",
	DAMAGE_HIGH := "DAMAGE_HIGH",
	DAMAGE_BRUTAL := "DAMAGE_BRUTAL",
	DAMAGE_DEVASTATING := "DAMAGE_DEVASTATING"
endenum

function GetResistValueByTier(tier)
	var resist := 0;

	if (Upper(tier) == Upper(T0))
		resist := RESIST_TIER0;
	elseif (Upper(tier) == Upper(T1))
		resist := RESIST_TIER1;
	elseif (Upper(tier) == Upper(T2))
		resist := RESIST_TIER2;
	elseif (Upper(tier) == Upper(T3))
		resist := RESIST_TIER3;
	elseif (Upper(tier) == Upper(T4))
		resist := RESIST_TIER4;
	elseif (Upper(tier) == Upper(T5))
		resist := RESIST_TIER5;
	endif

	return CInt(resist);
endfunction

/**
 * getNPCDamages(npc)
 *
 * Purpose
 * Passar todos os valores dos AttackDamage do NPC em um único struct.
 *
 * Parameters
 * npc: MobRef do NPC.
 *
 * Return struct
 * Retorna um struct com todos os AttackDamage do NPC.
 *
 */
function getNPCDamages(npc)
	var elem := NPC_GetNPCConfig(npc);
	var elemFisDamages := GetConfigString(elem, "EpicAttackDamage");
	var elemElemDamages := GetConfigString(elem, "EpicElemDamage");

	if (elemFisDamages == error || elemElemDamages == error)
		return error{"errortext" := "NPC doesnt have the new attack model!"};
	endif

	elemFisDamages := SplitWords(elemFisDamages);
	elemElemDamages := SplitWords(elemElemDamages);
	var damages := struct;

	damages.+SlashDamage := elemFisDamages[1];
	damages.+PiercingDamage := elemFisDamages[2];
	damages.+BluntDamage := elemFisDamages[3];

	damages.+FireDamage := elemElemDamages[1];
	damages.+ColdDamage := elemElemDamages[2];
	damages.+PoisonDamage := elemElemDamages[3];
	damages.+EnergyDamage := elemElemDamages[4];

	return damages;
endfunction

/**
 * getNPCMainDamage(npc)
 *
 * Purpose
 * Passar o valor do dado da arma usada pelo NPC. Se NPC não for sprite que usa arma, passar dado genérico do dano.
 *
 * Parameters
 * npc: MobRef do NPC.
 *
 * Return struct
 * Retorna o valor do dado do dano e o tipo do dano.
 *
 */
function getNPCMainDamage(npc, type)
	var damage := struct;

	if (type.Upper() == DAMAGE_TYPE::SLASH.Upper())
		damage.+AttackType := DAMAGE_TYPE::SLASH;
	elseif (type.Upper() == DAMAGE_TYPE::PIERCING.Upper())
		damage.+AttackType := DAMAGE_TYPE::PIERCING;
	elseif (type.Upper() == DAMAGE_TYPE::BLUNT.Upper())
		damage.+AttackType := DAMAGE_TYPE::BLUNT;
	else
		damage.+AttackType := error{"errortext" := "No valid attack type!"};
	endif

	if (!npc.weapon || npc.weapon == error)
		// TODO: Buscar o type dentro do npcTemplate.
		var npcDamages := getNPCDamages(npc);
		var damageTier;

		if (type.Upper() == DAMAGE_TYPE::SLASH.Upper())
			damageTier := npcDamages.SlashDamage;
		elseif (type.Upper() == DAMAGE_TYPE::PIERCING.Upper())
			damageTier := npcDamages.PiercingDamage;
		elseif (type.Upper() == DAMAGE_TYPE::BLUNT.Upper())
			damageTier := npcDamages.BluntDamage;
		else
			damageTier := error{"errortext" := "No valid damage tier!"};
		endif

		damage.+dice := getDamageByTier(damageTier);
	else
		if (!npc.weapon.getAttrBase()) npc.weapon.setAttrBase(0); endif
		damage.+dice := npc.weapon.getMainDamageDice();
	endif

	return damage;
endfunction

/**
 * getNPCResists(npc)
 *
 * Purpose
 * Passar todos os valores das resistências do NPC em um único struct.
 *
 * Parameters
 * npc: MobRef do NPC.
 *
 * Return struct
 * Retorna um struct com todos as resistências do NPC.
 *
 */
function getNPCResists(npc)
	var elem := NPC_GetNPCConfig(npc);
	var elemFiscResists := GetConfigString(elem, "EpicFisResists");
	var elemElemResists := GetConfigString(elem, "EpicElemResists");

	if (elemFiscResists == error || elemElemResists == error)
		return error{"errortext" := "NPC doesnt have the new resists defined!"};
	endif

	elemFiscResists := SplitWords(elemFiscResists);
	elemElemResists := SplitWords(elemElemResists);
	var stats := struct;

	foreach resist in elemFiscResists
	case (resist.Upper())
			T0: resist := RESIST_TIER0; break;
			T1: resist := RESIST_TIER1; break;
			T2: resist := RESIST_TIER2; break;
			T3: resist := RESIST_TIER3; break;
			T4: resist := RESIST_TIER4; break;
			T5: resist := RESIST_TIER5; break;
			default: resist := 0; break;
		endcase
	endforeach
	foreach resist in elemElemResists
	case (resist.Upper())
			T0: resist := RESIST_TIER0; break;
			T1: resist := RESIST_TIER1; break;
			T2: resist := RESIST_TIER2; break;
			T3: resist := RESIST_TIER3; break;
			T4: resist := RESIST_TIER4; break;
			T5: resist := RESIST_TIER5; break;
			default: resist := 0; break;
		endcase
	endforeach

	stats.+SlashResist := elemFiscResists[1];
	stats.+PiercingResist := elemFiscResists[2];
	stats.+BluntResist := elemFiscResists[3];

	stats.+FireResist := elemElemResists[1];
	stats.+ColdResist := elemElemResists[2];
	stats.+PoisonResist := elemElemResists[3];
	stats.+EnergyResist := elemElemResists[4];

	return stats;
endfunction

function getNPCElemDamages(npc)
	var elem := NPC_GetNPCConfig(npc);
	var elemElementalDamages := GetConfigString(elem, "EpicElemDamage");

	if (elemElementalDamages == error)
		return error{"errortext" := "NPC doesnt have the new elemental damages defined!"};
	endif

	elemElementalDamages := SplitWords(elemElementalDamages);
	var damages := struct;

	foreach damage in elemElementalDamages
		case (damage.Upper())
			DAMAGE_LOW: damage := getElementalDamageByTier(DAMAGE_LOW); break;
			DAMAGE_MID: damage := getElementalDamageByTier(DAMAGE_MID); break;
			DAMAGE_HIGH: damage := getElementalDamageByTier(DAMAGE_HIGH); break;
			DAMAGE_BRUTAL: damage := getElementalDamageByTier(DAMAGE_BRUTAL); break;
			DAMAGE_DEVASTATING: damage := getElementalDamageByTier(DAMAGE_DEVASTATING); break;
		endcase
	endforeach

	damages.+FireDamage := elemElementalDamages[1];
	damages.+ColdDamage := elemElementalDamages[2];
	damages.+PoisonDamage := elemElementalDamages[3];
	damages.+EnergyDamage := elemElementalDamages[4];

	return damages;
endfunction

function getElementalDamageByTier(tier)
	case (tier.Upper())
		DAMAGE_LOW:
			return 1;
		DAMAGE_MID:
			return 3;
		DAMAGE_HIGH:
			return 5;
		DAMAGE_BRUTAL:
			return 8;
		DAMAGE_DEVASTATING:
			return 12;

		default: return error{ "errortext" := "Value not found for this Elemental Tier!"};
	endcase
endfunction

function getPropNPCAttackType(npc)
	var prop := npc.GetProp("atributos");
	if (!prop) prop := struct; endif
    return prop;
endfunction

function setPropNPCAttackType(npc, type)
	var prop := npc.GetProp("atributos");
	if (prop == error)
		prop := struct;
		prop.+AttackType := type;
	else
		if (!prop.?AttackType)
			prop.+AttackType := type;
		else
			prop.AttackType := type;
		endif
	endif
	npc.SetProp("atributos", prop);
endfunction

function controlNPC(who, targ)
	if ( targ.IsA(POLCLASS_NPC) )
		targ.SetMaster(who);
				
		SetObjProperty(targ, "owner", who.serial);
		SetObjProperty(targ, "name", targ.name);

		if ( !GetObjProperty(targ, "script") )
			SetObjproperty(targ, "script", targ.Script);
		endif
		targ.script := ":ghaia:tamed";
		RestartScript(targ);
		
		Run_Script_To_Completion(":death:npcdeath", {targ});
	endif
endfunction

function isHurt(mobile)
    if (AP_GetVital(mobile, HITS) / AP_GetVitalMaximumValue(mobile, HITS) < 0.70)
        return 1;
    endif
    return 0;
endfunction

function StealCriticalFailure(mobile, targ)
	if (mobile.hidden)
		mobile.hidden := 0;
	endif
	SendSysMessageEx(mobile, "Voce chamou muita atencao!", SSM_FAIL);
	if (TS_GetTimerInfo(targ, "sleep"))
		SendSysMessageEx(mobile, "Voce acordou o alvo!", SSM_FAIL);
		TS_LowerDuration(targ, "sleep", -1);
	endif
	var range := 25 - CInt(AP_GetSkill(mobile, PICKPOCKETING)/5);

	foreach player in ( ListMobilesInLineOfSight(targ, range) )
		if ( player.npctemplate );
		elseif ( player == mobile );
		elseif ( player == targ );
		elseif ( !IsBehind(player, targ.x, targ.y) )
			SendSysMessageEx(player, "Voce nota "+mobile.name+" tentando roubar de "+targ.name+".", SSM_INFO);
		endif
		SleepMS(2);
	endforeach

	return 1;
endfunction

function getNpcResist()
   // Get NPC configuration for spell difficulty
   var tier := me.getTier();
   if (!tier)
      tier := 1;
   endif
   
   // Calculate spell difficulty based on tier
   var diff := RESIST_EASY;
   case (tier)
      1: diff := RESIST_EASY;
      2: diff := RESIST_MEDIUM;
      3: diff := RESIST_HARD;
      4: diff := RESIST_VERY_HARD;
      5: diff := RESIST_EXTREMELY_HARD;
   endcase

    return diff;
endfunction

function GetRandomNearbyCoords(item, range := 2)
    var coords := array{};
    for i := -range to range
        for j := -range to range
            if (i == 0 && j == 0) continue; endif // Skip item's exact position
            
            var x := item.x + i;
            var y := item.y + j;
            var z := GetWorldHeight(x, y, item.realm);
            
            // Check if location is walkable
            if (CheckLosAt(item, x, y, z))
                coords.append(struct{
                    "x" := x,
                    "y" := y, 
                    "z" := z
                });
            endif
            sleepms(2);
        endfor
    endfor
    
    if (coords.size() > 0)
        var random_pos := coords[RandomInt(coords.size())+1];
        return random_pos;
    endif
    
    // Fallback to item position if no valid spots found
    return struct{
        "x" := item.x,
        "y" := item.y,
        "z" := item.z
    };
endfunction

function AlertNearbySpiders(mobile)
    foreach spider in ListMobilesNearLocation(mobile.x, mobile.y, mobile.z, 22)
        if (spider.isSpider())
            SendEvent(spider, struct{ "type" := EVID_ALERT_ALLIES, "source" := mobile });
        endif
        sleepms(2);
    endforeach
endfunction

function GetRandomNearbyHostile(who)
	var opponents := ListHostiles(who, 8);
	var chosen_opponent := opponents[RandomIntMinMax(1, opponents.size())];
	if (!chosen_opponent)
		return who.opponent;
	endif

	return chosen_opponent;
endfunction

function getNPCState(npc)
    var hp_percent := AP_GetVital(npc, HITS) / AP_GetVitalMaximumValue(npc, HITS) * 100;

    if (hp_percent <= 25)
        return "dying";
    elseif (hp_percent <= 50)
        return "hurt";
    elseif (hp_percent <= 75)
        return "injured";
    else
        return "healthy";
    endif
endfunction