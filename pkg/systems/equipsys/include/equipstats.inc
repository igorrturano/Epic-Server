///////////////////////////////////////////////////////////////////////////////
///// Criado para substituir algumas funções do .cfg, que são limitadoras /////
///////////////////////////////////////////////////////////////////////////////

use os;
use cfgfile;

include "utils";
include "props";
include ":attributes:attributeConstants";

/**
 * getWeaponStats(weapon)
 *
 * Purpose
 * Passar todos os valores da arma em um único struct.
 *
 * Parameters
 * weapon: ItemRef da arma.
 *
 * Return struct
 * Retorna um struct com todos os valores base da arma.
 *
 */
function getWeaponStats(weapon)
	// TODO: Sistema de tier para somar os ganhos pra cada tier
	// TODO: Como agora há um range de Speed e CriticalChance, setar o valor mínimo e um mod baseado no range.
	var elem := getItemCfgElem(weapon);
	var prop := getPropBase(weapon);
	var stats := struct;
	var elemMainDamage := SplitWords(elem.EpicMainDamage);
	var elemOffDamage := SplitWords(elem.EpicOffDamage);

	stats.+SlashMainDamage := !getDamageByTier(elemMainDamage[1]).errortext ? getDamageByTier(elemMainDamage[1]): 0;
	stats.+PiercingMainDamage := !getDamageByTier(elemMainDamage[2]).errortext ? getDamageByTier(elemMainDamage[2]): 0;
	stats.+BluntMainDamage := !getDamageByTier(elemMainDamage[3]).errortext ? getDamageByTier(elemMainDamage[3]): 0;
	
	if (Upper(elem.Attribute) == DUALWIELD.Upper())
		stats.+SlashOffDamage := !getDamageByTier(elemOffDamage[1]).errortext ? getDamageByTier(elemOffDamage[1]): 0;
		stats.+PiercingOffDamage := !getDamageByTier(elemOffDamage[2]).errortext ? getDamageByTier(elemOffDamage[2]): 0;
		stats.+BluntOffDamage := !getDamageByTier(elemOffDamage[3]).errortext ? getDamageByTier(elemOffDamage[3]): 0;
	endif

	stats.+AttackType := elem.TipoAtaque;
	stats.+AttackSpeed := getSpeedByTier(elem.EpicSpeedTier) + CInt(prop.AttackSpeedMod);
	stats.+CriticalChance := getCriticalChanceByTier(elem.CriticalChance) + CInt(prop.CriticalChanceMod);

	return stats;
endfunction

/**
 * setWeaponStats(weapon)
 *
 * Purpose
 * Define os valores na arma, setando as devidas props e os atributos no item.
 *
 * Parameters
 * weapon: ItemRef da arma.
 *
 */
function setWeaponStats(weapon)
	var stats := getWeaponStats(weapon);
	setPropWeapon(weapon);

	// TODO: Setar na arma o que não vai em Prop
	weapon.speed_mod := stats.AttackSpeed != error ? (CInt(stats.AttackSpeed) - 24) : 0;
endfunction

/**
 * setArmorStats(armor)
 *
 * Purpose
 * Define os valores na armadura, setando as devidas props e os atributos no item.
 *
 * Parameters
 * weapon: ItemRef da armadura.
 *
 */
function setArmorStats(armor)
	var stats := getArmorStats(armor);
	setPropArmor(armor);

	// TODO: Setar na armadura o que não vai em CProp
endfunction

/**
 * getWeaponSpeed(tier)
 *
 * Purpose
 * Passar os valores máximos da arma baseado em seu Grupo de Velocidade.
 *
 * Parameters
 * tier: Grupo de Velocidade da arma (o Speed no .cfg)
 *
 * Return struct
 *
 */
function getSpeedByTier(tier)
	case (tier.Upper())
		MELEE_LOW: return 24; // min = 24 + 3 random
		MELEE_MID: return 30; // min = 30 + 3 random
		MELEE_HIGH: return 36; // min = 36 + 3 random

		RANGED_LOW: return 24; // min = 24 + 3 random
		RANGED_MID: return 30; // min = 30 + 3 random
		RANGED_HIGH: return 36; // min = 36 + 3 random
		
		default: return error{ "errortext" := "Value not found for this Weapon Speed Tier!"};
	endcase
endfunction

/**
 * getWeaponDamage(tier)
 *
 * Purpose
 * Verificar o valor do dado do dano da arma baseado no seu Grupo de Dano.
 *
 * Parameters
 * tier: Grupo de dano da arma (o Damage no .cfg)
 *
 * Return dado
 *
 */
function getDamageByTier(tier)
	case (tier.Upper())
		// NO DAMAGE
		DAMAGE_NONE: return 0;
		
		// ONE HANDED
		ONEHANDED_LOW: return "1d4";
		ONEHANDED_MID: return "1d6";
		ONEHANDED_HIGH: return "1d8";
			
		// TWO HANDED
		TWOHANDED_LOW: return "1d8";
		TWOHANDED_MID: return "1d10";
		TWOHANDED_HIGH: return "1d12";

		// BOW
		BOW_LOW: return "1d4";
		BOW_MID: return "1d6";
		BOW_HIGH: return "1d8";

		// CROSSBOW
		CROSSBOW_LOW: return "1d6";
		CROSSBOW_MID: return "1d8";
		CROSSBOW_HIGH: return "1d10";
			
		default: return error{ "errortext" := "Value not found for this Weapon Damage Tier!"};
	endcase
endfunction

/**
 * getWeaponCriticalChance(tier)
 *
 * Purpose
 *
 * Parameters
 * tier:
 *
 * Return struct
 *
 */
function getCriticalChanceByTier(tier)
	case(tier.upper())
		CRITICALCHANCE_NOCHANCE: return 0;
		CRITICALCHANCE_LOW: return 1;
		CRITICALCHANCE_MID: return 3;
		CRITICALCHANCE_HIGH: return 5;
		
		default: return error{ "errortext" := "Value not found for this Weapon Critical Chance Tier!"};
	endcase
endfunction

function getArmorStats(armor)
	var elem := getItemCfgElem(armor);
	var stats := struct;

	// TODO: Pegar todos os atributos da armadura e retornar no stats
	stats := getResistByProtection(elem.TipoProtecao);
	// TODO: Soma dos materiais.
	// TODO: Calcular HP, durabilidade, Str required, parry delay, parry bonus.
	// stats := calcResistByTier(stats, tier);

	return stats;
endfunction

function calcResistByTier(resists, tier)
	if (!resists.?SlashResist)
		return error{"errortext" := "Must be a armor or shield"};
	endif
	tier--;
	
	resists.PhysicalResist += tier;
	resists.SlashResist += tier;
	resists.PiercingResist += tier;
	resists.BluntResist += tier;

	return resists;
endfunction

function getResistByProtection(protectionType)
	var physical := array; // {SlashResist, PiercingResist, BluntResist};

	case(Upper(protectionType))
        //TIER 1
        ARMOR_LOWLOW: physical := {0,0,0};
        ARMOR_LOWMID: physical := {1,1,1};
        ARMOR_LOWHIGH: physical := {2,2,2};

        //TIER 2
        ARMOR_MIDLOW: physical := {3,3,3};
        ARMOR_MIDMID: physical := {4,4,4};
        ARMOR_MIDHIGH: physical := {5,5,5};

        //TIER 3
        ARMOR_HIGHLOW: physical := {6,6,6};
        ARMOR_HIGHMID: physical := {7,7,7};
        ARMOR_HIGHHIGH: physical := {8,8,8};
    endcase

	var resists := struct;
	resists.+SlashResist := physical[1];
	resists.+PiercingResist := physical[2];
	resists.+BluntResist := physical[3];
	// TODO: PEGAR OS ELEMENTAIS BASEADOS NAS PROPS QUE O ITEM VAI TER QUANDO O MAGO ENCANTAR (ver onde ficam esses valores)
	resists.+FireResist := 0;
	resists.+ColdResist := 0;
	resists.+PoisonResist := 0;
	resists.+EnergyResist := 0;
	return resists;
endfunction

function getResistModBySlot(item)
	var stats := array;
	var slot := getSlot(item).upper;

    case(slot) // {SlashResist, PiercingResist, BluntResist}
        HEAD: stats := {2,2,2}; break;
        NECK: stats := {1,1,1}; break;
        BODY: stats := {4,4,4}; break;
        ARMS: stats := {2,2,2}; break;
        HANDS: stats := {1,1,1}; break;
        LEGS: stats := {3,3,3}; break;
		FEET: stats := {2,2,2}; break;
        SHIELD: stats := {4,4,4}; break;
    endcase

    var resists := struct;
	resists.+PhysicalResistMod := stats[1];
	resists.+SlashResistMod := stats[1];
	resists.+PiercingResistMod := stats[2];
	resists.+BluntResistMod := stats[3];

	resists.+FireResistMod := 0;
	resists.+ColdResistMod := 0;
	resists.+PoisonResistMod := 0;
	resists.+EnergyResistMod := 0;
	return resists;
endfunction

function getDexPenModBySlot(item)
	var slot := getSlot(item).upper();
	var dexPen;

	case(slot) // {SlashResist, PiercingResist, BluntResist, DexPenalty}
        HEAD: dexPen := 1; break;
        NECK: dexPen := 1; break;
        BODY: dexPen := 3; break;
        ARMS: dexPen := 2; break;
        HANDS: dexPen := 1; break;
        LEGS: dexPen := 2; break;
        SHIELD: dexPen := 1; break;
    endcase

	return dexPen;
endfunction

function getWeaponMainDamageDice(weapon)
	var prop := getPropBase(weapon);
	var dice;

	if (Upper(prop.AttackType) == Upper(SLASHDAMAGE))
		dice := prop.SlashMainDamage;
	elseif (Upper(prop.AttackType) == Upper(PIERCINGDAMAGE))
		dice := prop.PiercingMainDamage;
	elseif (Upper(prop.AttackType) == Upper(BLUNTDAMAGE))
		dice := prop.BluntMainDamage;
	else
		dice := error{"errortext" := "No dice damage!"};
	endif
	
	return dice;
endfunction

function getWeaponOffDamageDice(weapon)
	var prop := getPropBase(weapon);
	var dice;
	
	if (prop.AttackType.Upper() == SLASHDAMAGE.Upper())
		dice := prop.SlashOffDamage;
	elseif (prop.AttackType.Upper() == PIERCINGDAMAGE.Upper())
		dice := prop.PiercingOffDamage;
	elseif (prop.AttackType.Upper() == BLUNTDAMAGE.Upper())
		dice := prop.BluntOffDamage;
	else
		dice := error{"errortext" := "No dice damage!"};
	endif
	
	return dice;
endfunction