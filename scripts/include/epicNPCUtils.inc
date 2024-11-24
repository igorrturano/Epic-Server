use util;

include ":attributes:attributes";
include ":equipsys:equipstats";
include ":equipsys:props";
include ":taming:taming";

function customizeNpc(mob, set_tier := "0")
	var npc_elem := NPC_GetNPCConfig(mob);
	var classe := npc_elem.Classe;
	var tier := set_tier == "0" ? CInt(npc_elem.Tier) : CInt(set_tier);
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
		AP_SetTrueSkill(mob, MAGICLORE, mob_skill);
		skill_combat := MAGERY;
		skill_support := MAGICLORE;
	else
		skill_combat := WRESTLING;
		skill_support := TACTICS;
	endif

	AP_SetTrueSkill(mob, WRESTLING, mob_skill);
	AP_SetTrueSkill(mob, TACTICS, tactics);

    var calculate_level_hp := Cint(AP_GetSkill(mob, skill_combat) + AP_GetSkill(mob, skill_support)) - 80;

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
    if ((calculate_level_hp / 6) >= 1 )
        var new_hp := 0;
        var multiplier := Cint((calculate_level_hp / 6));
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
    var nearby := ListMobilesNearLocationEx(who.x, who.y, LIST_IGNORE_Z, 30, LISTEX_FLAG_NORMAL + LISTEX_FLAG_HIDDEN + LISTEX_FLAG_CONCEALED, who.realm);
    foreach mobile in nearby
        if (mobile.acctname)
            return 1;
        endif
    endforeach
    return 0;
endfunction

function HasPlayerNearLocation(x, y, realm := 0)
	var mobs := ListMobilesNearLocationEx( x, y, LIST_IGNORE_Z, 15, LISTEX_FLAG_NORMAL|LISTEX_FLAG_HIDDEN|LISTEX_FLAG_PLAYERS_ONLY );
	foreach mob in mobs
		if ( !mob.cmdlevel && !mob.concealed ) //se for player e priv de player
			return 1;
		endif
      sleepms(2);
	endforeach

	return 0;
endfunction

enum RESISTS_TIERS
	RESIST_TIER0 := 0,
	RESIST_TIER1 := 1,
	RESIST_TIER2 := 3,
	RESIST_TIER3 := 6,
	RESIST_TIER4 := 12,
	RESIST_TIER5 := 16,
endenum

enum TIERS
	T0 := "T0",
	T1 := "T1",
	T2 := "T2",
	T3 := "T3",
	T4 := "T4",
	T5 := "T5",
endenum

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

	if (type.Upper() == SLASHDAMAGE.Upper())
		damage.+AttackType := SLASHDAMAGE;
	elseif (type.Upper() == PIERCINGDAMAGE.Upper())
		damage.+AttackType := PIERCINGDAMAGE;
	elseif (type.Upper() == BLUNTDAMAGE.Upper())
		damage.+AttackType := BLUNTDAMAGE;
	else
		damage.+AttackType := error{"errortext" := "No valid attack type!"};
	endif

	if (!npc.weapon || npc.weapon == error)
		// TODO: Buscar o type dentro do npcTemplate.
		var npcDamages := getNPCDamages(npc);
		var damageTier;

		if (type.Upper() == SLASHDAMAGE.Upper())
			damageTier := npcDamages.SlashDamage;
		elseif (type.Upper() == PIERCINGDAMAGE.Upper())
			damageTier := npcDamages.PiercingDamage;
		elseif (type.Upper() == BLUNTDAMAGE.Upper())
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

function controlNPC(who)
	var targ := target(who);
	
	if ( targ.IsA(POLCLASS_NPC) )
		targ.SetMaster(who);
				
		SetObjProperty(targ, "owner", who.serial);
		SetObjProperty(targ, "name", targ.name);
		
		if ( !GetObjProperty(targ, "script") )
			SetObjproperty(targ, "script", targ.script);
		endif
			
		targ.script := ":ghaia:tamed";
		RestartScript(targ);
		
		Run_Script_To_Completion(":death:npcdeath", {targ});
	endif
endfunction

function endControlNPC(who)
	ErasePet(who.master, who.serial);
	who.SetMaster(0);

	var orig_script := GetObjProperty(who, "script");

	if ( !orig_script )

		PrintTextAbove(who, "*doesnt have an original script to use now that taming is done?!*");
		SysLog(who.name+" doesnt have an original script to use now that taming is done?!");
		orig_script := ":brainai:brain";

	elseif ( orig_script["tamed"] )

		PrintTextAbove(who, "*my original script was tamed.src ?!*");
		SysLog(who.name+" my original script was tamed.src ?!");
		orig_script := ":brainai:brain";

	endif

	who.script := orig_script;
	SetName(who, GetObjProperty(who, "name"));
	EraseObjProperty(who, "owner");
	EraseObjProperty(who, "name");
	EraseObjproperty(who, "script");
	EraseObjProperty(who, "happiness");
	RestartScript(who);

	ErasePetMods(who);
endfunction
