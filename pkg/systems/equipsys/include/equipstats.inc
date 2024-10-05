///////////////////////////////////////////////////////////////////////////////
///// Criado para substituir algumas funções do .cfg, que são limitadoras /////
///////////////////////////////////////////////////////////////////////////////

use os;
use cfgfile;

include "utils";
include "props";
include ":combat:combat";

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
	var stats := struct;
	var statsMod := struct;
	var elemMainDamage := SplitWords(elem.EpicMainDamage);
	var elemOffDamage := SplitWords(elem.EpicOffDamage);

	stats.+SlashMainDamage := !getDamageByTier(elemMainDamage[1]).errortext ? getDamageByTier(elemMainDamage[1]) : 0;
	stats.+PiercingMainDamage := !getDamageByTier(elemMainDamage[2]).errortext ? getDamageByTier(elemMainDamage[2]) : 0;
	stats.+BluntMainDamage := !getDamageByTier(elemMainDamage[3]).errortext ? getDamageByTier(elemMainDamage[3]) : 0;
	if (Upper(elem.Attribute) == "DUAL_WIELD")
		stats.+DualWield := 1;
		stats.+SlashOffDamage := !getDamageByTier(elemOffDamage[1]).errortext ? getDamageByTier(elemOffDamage[1]) : 0;
		stats.+PiercingOffDamage := !getDamageByTier(elemOffDamage[2]).errortext ? getDamageByTier(elemOffDamage[2]) : 0;
		stats.+BluntOffDamage := !getDamageByTier(elemOffDamage[3]).errortext ? getDamageByTier(elemOffDamage[3]) : 0;
	endif
	stats.+AttackType := elem.TipoAtaque;
	stats.+AttackSpeed := getSpeedByTier(elem.EpicSpeed);
	stats.+CriticalChange := getCriticalChanceByTier(elem.CriticalChance);

	return stats;
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
		
		default: return error{ "errortext" := "Damage value not found for this Weapon Speed Tier!"};
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
		// ONE HANDED
		ONEHANDED_LOW: return ONEHANDED_LOW_DAMAGE;
		ONEHANDED_MID: return ONEHANDED_MID_DAMAGE;
		ONEHANDED_HIGH: return ONEHANDED_HIGH_DAMAGE;
			
		// TWO HANDED
		TWOHANDED_LOW: return TWOHANDED_LOW_DAMAGE;
		TWOHANDED_MID: return TWOHANDED_MID_DAMAGE;
		TWOHANDED_HIGH: return TWOHANDED_HIGH_DAMAGE;

		// BOW
		BOW_LOW: return BOW_LOW_DAMAGE;
		BOW_MID: return BOW_MID_DAMAGE;
		BOW_HIGH: return BOW_HIGH_DAMAGE;

		// CROSSBOW
		CROSSBOW_LOW: return CROSSBOW_LOW_DAMAGE;
		CROSSBOW_MID: return CROSSBOW_MID_DAMAGE;
		CROSSBOW_HIGH: return CROSSBOW_HIGH_DAMAGE;
			
		default: return error{ "errortext" := "Damage value not found for this Weapon Damage Tier!"};
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
		
		default: return error{ "errortext" := "Damage value not found for this Weapon Critical Chance Tier!"};
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

    case(slot) // {SlashResist, PiercingResist, BluntResist, DexPenalty}
        HEAD: stats := {2,2,2}; break;
        NECK: stats := {1,1,1}; break;
        BODY: stats := {4,4,4}; break;
        ARMS: stats := {2,2,2}; break;
        HANDS: stats := {1,1,1}; break;
        LEGS: stats := {3,3,3}; break;
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

	if (prop.Upper() == SLASHDAMAGE.Upper())
		dice := prop.SlashMainDamage;
	elseif (prop.Upper() == PIERCINGDAMAGE.Upper())
		dice := prop.PiercingMainDamage;
	elseif (prop.Upper() == BLUNTDAMAGE.Upper())
		dice := prop.BluntMainDamage;
	else
		dice := error{"errortext" := "No dice damage!"};
	endif
	
	return dice;
endfunction

function getWeaponOffDamageDice(weapon)
	var prop := getPropBase(weapon);
	var dice;
	
	if (prop.Upper() == SLASHDAMAGE.Upper())
		dice := prop.SlashOffDamage;
	elseif (prop.Upper() == PIERCINGDAMAGE.Upper())
		dice := prop.PiercingOffDamage;
	elseif (prop.Upper() == BLUNTDAMAGE.Upper())
		dice := prop.BluntOffDamage;
	else
		dice := error{"errortext" := "No dice damage!"};
	endif
	
	return dice;
endfunction