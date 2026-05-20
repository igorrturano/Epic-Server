// Internal
include ":combat:armorZones";

// External
include ":tn:cooldown";
include ":attributes:attributes";
include ":blood:blood";
include ":fls_core:packets";
include "include/epicCombatUtils";
include "include/facings";
include ":timedScripts:timedScripts";
// include ":debug:Controller/DebugController";
// include ":equipsys:utils/consts";
include "include/epicNPCUtils";
include ":combat:weaponInfo";
include ":wrappers:Objects/Model/MobileWrapper";
include ":abilities:models/Ability";
include ":charactercreation:habilidades";
/*
 * http://uo.stratics.com/content/guides/resistance.shtml
 * These could be attributes, but not every shard will want to
 * follow OSI. This gives more freedom to make custom resistances
 * and remove existing ones more easily.
 */
enum DAMAGE_TYPES
	DMG_FORCED		:= -1, // Ignores resistance
	DMG_PHYSICAL	:= "Physical",
	
	DMG_SLASH		:= "Slash",
	DMG_PIERCING	:= "Piercing",
	DMG_BLUNT		:= "Blunt",

	DMG_FIRE		:= "Fire",
	DMG_COLD		:= "Cold",
	DMG_POISON		:= "Poison",
	DMG_ENERGY		:= "Energy"
endenum

function _GetElementalSpecForDmgType(dmgType)
	case (dmgType)
		DMG_FIRE:    return ABILITY_ID::ELEMENTAL_SPEC_FIRE;
		DMG_COLD:    return ABILITY_ID::ELEMENTAL_SPEC_COLD;
		DMG_ENERGY:  return ABILITY_ID::ELEMENTAL_SPEC_ENERGY;
		DMG_POISON:  return ABILITY_ID::ELEMENTAL_SPEC_POISON;
		default:     return 0;
	endcase
endfunction

function CalculateDamage(defender, type, dmg)
	var resistMod := 0;
	var resist := 0;

	if (defender.npctemplate)
		var npctemplate := NPC_ParseTemplateName(defender.npctemplate);
		var cfg := ReadConfigFile(":*:npcdesc");

		case(type)
			DMG_PHYSICAL: resist := CInt(cfg[npctemplate.template].PhysicalResist) + CInt(defender.resist_physical_mod); break;
			DMG_FIRE: resist := CInt(cfg[npctemplate.template].FireResist) + CInt(defender.resist_fire_mod); break;
			DMG_COLD: resist := CInt(cfg[npctemplate.template].ColdResist) + CInt(defender.resist_cold_mod); break;
			DMG_ENERGY: resist := CInt(cfg[npctemplate.template].EnergyResist) + CInt(defender.resist_energy_mod); break;
			DMG_POISON: resist := CInt(cfg[npctemplate.template].PoisonResist) + CInt(defender.resist_poison_mod); break;
		endcase
	else
		case(type)
			DMG_PHYSICAL:
				resist := CInt(defender.resist_physical);
				resistMod := CInt(defender.resist_physical_mod);
				DebugMsg(defender, DEBUG_DAMAGE, $"Defesa Física (Mod): {resist}({resistMod})");
				break;

			DMG_FIRE:
				resist := CInt(defender.resist_fire);
				resistMod := CInt(defender.resist_fire_mod);
				DebugMsg(defender, DEBUG_DAMAGE, $"Defesa Fire (Mod): {resist}({resistMod})");
				break;

			DMG_COLD:
				resist := CInt(defender.resist_cold);
				resistMod := CInt(defender.resist_cold_mod);
				DebugMsg(defender, DEBUG_DAMAGE, $"Defesa Cold (Mod): {resist}({resistMod})");
				break;

			DMG_ENERGY:
				resist := CInt(defender.resist_energy);
				resistMod := CInt(defender.resist_energy_mod);
				DebugMsg(defender, DEBUG_DAMAGE, $"Defesa Energy (Mod): {resist}({resistMod})");
				break;

			DMG_POISON:
				resist := CInt(defender.resist_poison);
				resistMod := CInt(defender.resist_poison_mod);
				DebugMsg(defender, DEBUG_DAMAGE, $"Defesa Poison (Mod): {resist}({resistMod})");
				break;
		endcase
	endif

	dmg -= resist;

	if (dmg < 1)
		dmg := 1;
	endif

	return dmg;
endfunction


function ApplyDamageEX(defender, dmg, type, source := 0)
	var totaldmg := cint(CalculateDamage(defender, type, dmg ));

	ApplyRawDamage( defender, totaldmg );

	if (source)
		SetLastDamageInfo(defender, totaldmg, type, source);
		SetScriptController(source);
	endif

	return totaldmg;
endfunction


/*
 * SetLastDamageInfo(mobile, amount, type, source)
 *
 * Purpose
 *
 * Parameters
 * mobile:
 * amount:
 * type:
 * source:
 *
 * Return value
 *
 */
function SetLastDamageInfo(mobile, amount, type, source, spell := "")
	var last_damage := struct;
	last_damage.+serial	:= source.serial;
	last_damage.+time	:= polcore().systime;
	last_damage.+amount	:= amount;
	last_damage.+type	:= type;

	if ( spell )
		last_damage.+spell := spell;
	endif

////AUTOPSIA
var life := AP_GetVital(mobile, "HITS");
if (amount >= life)
var autopsy := struct;
	if ( source.weapon )// adicionado para o forensicevaluation detectar a arma do crime
		//last_damage.+weapon := source.weapon;
		autopsy.+serial_weapon := source.weapon.serial; 
		autopsy.+objtype := source.weapon.objtype;
	endif
	
    // Descobrir mais Dano
    var slashDamage := getNPCMainDamage(mobile, SLASHDAMAGE);
    var piercingDamage := getNPCMainDamage(mobile, PIERCINGDAMAGE);
    var bluntDamage := getNPCMainDamage(mobile, BLUNTDAMAGE);
    // Compara os valores dos dados (assumindo que dice tem um valor numérico)
    var maxDamage := slashDamage.dice;
    var maxType := SLASHDAMAGE;  
    if (piercingDamage.dice > maxDamage)
        maxDamage := piercingDamage.dice;
        maxType := PIERCINGDAMAGE;
    endif
    if (bluntDamage.dice > maxDamage)
        maxDamage := bluntDamage.dice;
        maxType := BLUNTDAMAGE;
    endif
    autopsy.+dano_principal := maxType;

	var stat := struct;
	stat.+STR := AP_GetStat(mobile, "STRENGTH");
	stat.+DEX := AP_GetStat(mobile, "DEXTERITY");
	stat.+INT := AP_GetStat(mobile, "INTELLIGENCE");
	
	var vital := struct;
	vital.+HIT := AP_GetVitalMaximumValue(mobile, "HITS");
	vital.+STA := AP_GetVitalMaximumValue(mobile, "STAMINA");
	vital.+MAN := AP_GetVitalMaximumValue(mobile, "MANA");
	//Descobir as resits
	autopsy.+resists := getNPCResists(mobile);
	autopsy.+elemdamage := getNPCElemDamages(mobile);
	autopsy.+stats := stat;
	autopsy.+vitals := vital;
	
	var npc_elem := NPC_GetNPCConfig(mobile);
	var classe := npc_elem.Classe;
	var skills_combat := struct;
	
	if (classe == SABIO)
		skills_combat.+magery := AP_GetTrueSkill(mobile, MAGERY);
		skills_combat.+arcana := AP_GetTrueSkill(mobile, ARCANA);
	elseif(classe == LADINO || mobile.isSpider())
		skills_combat.+sneak := EffectiveTrueSkill(mobile, "Stealth");
		skills_combat.+awareness := EffectiveTrueSkill(mobile, "Perception");
		skills_combat.+pickpocketing := EffectiveTrueSkill(mobile, "Theft");
	else
		skills_combat.+wrestling := AP_GetTrueSkill(mobile, WRESTLING);
		skills_combat.+tactics := AP_GetTrueSkill(mobile, TACTICS);
	endif
	autopsy.+skills := skills_combat;
	autopsy.+taxonomy := npc_elem.Taxonomy;
	autopsy.+rastros := npc_elem.rastro;

	SetObjProperty(mobile, "Autopsy", autopsy);
endif	
//FIM AUTOPSIA

	if ( last_damage.acctname )
		last_damage.+acctname := source.acctname;
	endif
	if ( source.name )
		last_damage.+name := source.name;
	else
		last_damage.+name := source.desc;
	endif

	SetObjProperty(mobile, "LastDamage", last_damage);

	return last_damage;
endfunction

/*
 * GetLastDamageInfo(mobile)
 *
 * Purpose
 *
 * Parameters
 * mobile:
 *
 * Return value
 *
 */
function GetLastDamageInfo(mobile)
	return GetObjProperty(mobile, "LastDamage");
endfunction

// Barreira de Mana (spell 114): redirects incoming damage to mana at shield.ratio:1
// (default 2:1, 1:1 when caster has Arcana primary).
// Returns the damage that should still be applied to HP after absorption.
function _ApplyManaShield(defender, dmg)
	if (dmg <= 0)
		return dmg;
	endif

	var shield := GetObjProperty(defender, "#manashield");
	if (!shield || shield is @Exception)
		return dmg;
	endif

	if (shield.endtime == error || shield.endtime <= ReadGameClock())
		EraseObjProperty(defender, "#manashield");
		return dmg;
	endif

	var current_mana := AP_GetVital(defender, MANA);
	if (current_mana <= 0)
		EraseObjProperty(defender, "#manashield");
		return dmg;
	endif

	var ratio := CInt(shield.ratio);
	if (ratio < 1)
		ratio := 2;
	endif

	var absorbable := Cint(current_mana / ratio);
	var absorbed := dmg;
	if (absorbed > absorbable)
		absorbed := absorbable;
	endif
	if (absorbed <= 0)
		return dmg;
	endif

	AP_ConsumeVital(defender, MANA, absorbed * ratio);

	if (AP_GetVital(defender, MANA) <= 0)
		EraseObjProperty(defender, "#manashield");
		PrintTextAbove(defender, "*barreira arcana quebra*");
	endif

	return dmg - absorbed;
endfunction

function DamageFLS(defender, dmg, type := DMG_FORCED, source := 0, spell := "")
	var totaldmg;
	var attackInfo := struct{};

	// Caminho: Elemental Specialization bonus (+2 per purchase)
	if (source && type != DMG_FORCED)
		var elemSpecId := _GetElementalSpecForDmgType(type);
		if (elemSpecId)
			var mob := MobileWrapper(source);
			var count := mob.GetPoolAbilityCount(elemSpecId);
			if (count > 0)
				dmg := dmg + (count * 2);
			endif
		endif
	endif

	if (type == DMG_FORCED)
		totaldmg := CInt(dmg);
	else
		totaldmg := CInt(CalculateDamage(defender, type, dmg));
	endif

	if (type == DMG_PHYSICAL || type == DMG_SLASH || type == DMG_PIERCING || type == DMG_BLUNT)
		totaldmg := _ApplyManaShield(defender, totaldmg);
	endif

	if (source)
		SetScriptController(source);
	endif

	// Mesmo sendo uma função VANILLA. Talvez esses CheckHab_ precisem ser trocados por CheckDefenderHabsBeforeDamage() e CheckDefenderHabsAfterDamage()
	attackInfo.+damage := dmg;
	attackInfo.+type := type;
	
	// Checa as habilidades antes do dano
	// CheckHab_Martir(0, defender, attackInfo);
	// CheckHab_MantoRothen(0, defender, attackInfo);

	// Aplica o dano
	DisplayDamage(defender, totaldmg, source);
	ApplyRawDamage(defender, totaldmg);
	SetLastDamageInfo(defender, totaldmg, type, source, spell);

	if (totaldmg > 0)
		var vingancaCur := GetObjProperty(defender, "#vinganca");
		if (vingancaCur != error)
			SetObjProperty(defender, "#vinganca", CInt(vingancaCur) + CInt(totaldmg));
		endif
	endif

//FRAJOLAS Adicionado para o discordwebhook detectar dano de trap
if (totaldmg > 0)
    var lastHit := dictionary;

    if (source) // dano normal (npc/player)
        lastHit["attacker"] := source.serial;
    else // trap ou dano sem source
        lastHit["attacker"] := 0;
        lastHit["source"] := "trap";
    endif

    lastHit["amount"] := totaldmg;
    lastHit["time"] := polcore().systime;
    lastHit["type"] := type;

    SetObjProperty(defender, "lastHit", lastHit);
endif
//end FRAJOLAS

	// Checa as habilidades depois do dano
	// CheckHab_RecuperacaoSalvadora(0, defender, attackInfo);
	// CheckHab_Vinganca(0, defender, attackInfo);

	if (defender.frozen)
		defender.frozen := 0;
	endif

	return totaldmg;
endfunction
