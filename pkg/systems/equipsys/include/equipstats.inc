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
	var stats := struct{};
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

function setWeaponMagicStats(weapon, structDamages)
	var stats := getPropBase(weapon);

	if (structDamages.?FireDamage)
		stats.+FireDamage := structDamages.FireDamage;
	endif
	if (structDamages.?ColdDamage)
		stats.+ColdDamage := structDamages.ColdDamage;
	endif
	if (structDamages.?PoisonDamage)
		stats.+PoisonDamage := structDamages.PoisonDamage;
	endif
	if (structDamages.?EnergyDamage)
		stats.+EnergyDamage := structDamages.EnergyDamage;
	endif

	setPropBase(weapon, stats);
endfunction

function getArmorStats(armor)
	var stats := struct{};

	stats := addResistsPhysical(stats, armor);
	stats := addResistsElemental(stats, armor);
	stats := addPenalties(stats, armor);
	if (IsShield(armor))
		stats := addShieldMods(stats, armor);
	endif
	// TODO: Calcular HP, durabilidade, Str required.

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

function addResistsPhysical(stats, armor)
	var protection := getType(armor);
	var physical := array{}; // {SlashResist, PiercingResist, BluntResist};

	// TODO: Balancear os valores aqui, não é pra ser todos iguais.
	case(Upper(protection))
		//TIER 1
		ARMOR_LOWLOW:
			physical := {0,0,0};
			break;
		ARMOR_LOWMID:
			physical := {1,1,1};
			break;
		ARMOR_LOWHIGH:
			physical := {2,2,2};
			break;

		//TIER 2
		ARMOR_MIDLOW:
			physical := {3,3,3};
			break;
		ARMOR_MIDMID:
			physical := {4,4,4};
			break;
		ARMOR_MIDHIGH:
			physical := {5,5,5};
			break;

		//TIER 3
		ARMOR_HIGHLOW:
			physical := {6,6,6};
			break;
		ARMOR_HIGHMID:
			physical := {7,7,7};
			break;
		ARMOR_HIGHHIGH:
			physical := {8,8,8};
			break;

		// Escudos também entram aqui porque atualmente eles não têm defesa. Se mudar, só adicionar as tags dos escudos aqui.
		default: 
			physical := {0,0,0};
	endcase

	stats.+SlashResist := physical[1];
	stats.+PiercingResist := physical[2];
	stats.+BluntResist := physical[3];

	stats := addResistBySlot(stats, armor);

	return stats;
endfunction

function addResistBySlot(stats, armor)
	var resists := array{}; // {SlashResist, PiercingResist, BluntResist}
	var slot := getSlot(armor);

	case(Upper(slot))
		HEAD: resists := {2,2,2}; break;
		NECK: resists := {1,1,1}; break;
		BODY: resists := {4,4,4}; break;
		ARMS: resists := {2,2,2}; break;
		HANDS: resists := {1,1,1}; break;
		LEGS: resists := {3,3,3}; break;
		FEET: resists := {2,2,2}; break;
		SHIELD: resists := {0,0,0}; break;
	endcase

	stats.SlashResist += resists[1];
	stats.PiercingResist += resists[2];
	stats.BluntResist += resists[3];
	
	return stats;
endfunction

function addResistsElemental(stats, armor)
	var prop := getPropBase(armor);

	if (prop.FireResist)
		stats.+FireResist := CInt(prop.FireResist);
	endif
	if (prop.ColdResist)
		stats.+ColdResist := CInt(prop.ColdResist);
	endif
	if (prop.PoisonResist)
		stats.+PoisonResist := CInt(prop.PoisonResist);
	endif
	if (prop.EnergyResist)
		stats.+EnergyResist := CInt(prop.EnergyResist);
	endif

	return stats;
endfunction

function addPenalties(stats, armor)
	var slot := getSlot(armor);
	var penalties := array{}; // {DexPen, SneakPen, MAPen, MageryPen, AwarenessPen}
	
	case(Upper(slot))
		HEAD: penalties := {0,0,0,1,0}; break;
		NECK: penalties := {0,0,0,1,0}; break;
		BODY: penalties := {0,0,0,12,0}; break;
		ARMS: penalties := {0,0,2,6,0}; break;
		HANDS: penalties := {0,0,4,3,0}; break;
		LEGS: penalties := {0,0,0,6,0}; break;
		FEET: penalties := {0,0,0,0,0}; break;
		SHIELD: penalties := {0,1,10,5,0}; break;
	endcase
	
	stats.+DexPen := penalties[1];
	stats.+SneakPen := penalties[2];
	stats.+MAPen := penalties[3];
	stats.+MageryPen := penalties[4];
	stats.+AwarenessPen := penalties[5];

	stats := addPenaltiesByProtection(stats, armor);

	return stats;
endfunction

function addPenaltiesByProtection(stats, armor)
	var protection := getType(armor);
	var penalties := array{}; // {DexPen, SneakPen, MAPen, MageryPen, AwarenessPen}

	case(Upper(protection))
		//TIER 1
		ARMOR_LOWLOW:
			penalties := {0,0,0,0,0};
			break;
		ARMOR_LOWMID:
			penalties := {0,0,0,0,0};
			break;
		ARMOR_LOWHIGH:
			penalties := {0,0,0,0,0};
			break;

		//TIER 2
		ARMOR_MIDLOW:
			penalties := {0,0,0,0,0};
			break;
		ARMOR_MIDMID:
			penalties := {0,0,0,0,0};
			break;
		ARMOR_MIDHIGH:
			penalties := {0,0,0,0,0};
			break;

		//TIER 3
		ARMOR_HIGHLOW:
			penalties := {0,0,0,0,0};
			break;
		ARMOR_HIGHMID:
			penalties := {0,0,0,0,0};
			break;
		ARMOR_HIGHHIGH:
			penalties := {0,0,0,0,0};
			break;


		// ESCUDOS
		SHIELD_BUCKLER:
			penalties := {0,0,0,0,0};
			break;
		SHIELD_SMALL:
			penalties := {0,0,0,0,0};
			break;
		SHIELD_MEDIUM:
			penalties := {0,0,0,0,0};
			break;
		SHIELD_LARGE:
			penalties := {0,0,0,0,0};
			break;
		SHIELD_TOWER:
			penalties := {0,0,0,0,0};
			break;

		default: 
			penalties := {0,0,0,0,0};
	endcase

	return stats;
endfunction

function addShieldMods(stats, shield)
	var protection := getType(shield);
	var mods := array{}; // {ParryDelay, ParryBonus}

	// TODO: Rever esses valores com o Igor, comparando com o .cfg (tá muito irregular lá)
	case(Upper(protection))
		SHIELD_BUCKLER: mods := {-1,0}; break;
		SHIELD_SMALL: mods := {4,8}; break;
		SHIELD_MEDIUM: mods := {5,10}; break;
		SHIELD_LARGE: mods := {6,12}; break;
		SHIELD_TOWER: mods := {7,14}; break;
	endcase

	stats.+ParryDelay := mods[1];
	stats.+BonusDelay := mods[2];

	return stats;
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