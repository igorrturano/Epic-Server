///////////////////////////////////////////////////////////////////////////////
///// Criado para substituir algumas funções do .cfg, que são limitadoras /////
///////////////////////////////////////////////////////////////////////////////

use os;
use cfgfile;

include "utils";
include "props";
include ":attributes:attributeConstants";

/////////////
// WEAPONS //
/////////////
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
	var stats := struct{};

	stats := addDamagePhysical(stats, weapon);
	stats := addDamageElemental(stats, weapon);

	return stats;
endfunction

function addDamagePhysical(stats, weapon)
	var elem := getItemCfgElem(weapon);
	var elemMainDamage := SplitWords(elem.EpicMainDamage);
	var elemOffDamage := SplitWords(elem.EpicOffDamage);
	var prop := getPropBase(weapon);

	stats.+SlashMainDamage := !getDamageByTier(elemMainDamage[1]).errortext ? getDamageByTier(elemMainDamage[1]) : 0;
	stats.+PiercingMainDamage := !getDamageByTier(elemMainDamage[2]).errortext ? getDamageByTier(elemMainDamage[2]) : 0;
	stats.+BluntMainDamage := !getDamageByTier(elemMainDamage[3]).errortext ? getDamageByTier(elemMainDamage[3]) : 0;

	if (Upper(elem.Attribute) == DUALWIELD.Upper())
		stats.+SlashOffDamage := !getDamageByTier(elemOffDamage[1]).errortext ? getDamageByTier(elemOffDamage[1]) : 0;
		stats.+PiercingOffDamage := !getDamageByTier(elemOffDamage[2]).errortext ? getDamageByTier(elemOffDamage[2]) : 0;
		stats.+BluntOffDamage := !getDamageByTier(elemOffDamage[3]).errortext ? getDamageByTier(elemOffDamage[3]) : 0;
	endif

	stats.+AttackType := elem.TipoAtaque;
	stats.+AttackSpeed := getSpeedByTier(elem.EpicSpeedTier);
	stats.+CriticalChance := getCriticalChanceByTier(elem.CriticalChance);

	return stats;
endfunction

function addDamageElemental(stats, weapon)
	// Danos físicos são dados, danos elementais são puros
	stats.+FireDamage := 0;
	stats.+ColdDamage := 0;
	stats.+PoisonDamage := 0;
	stats.+EnergyDamage := 0;

	stats := addDamageElementalByWeapon(stats, weapon);

	return stats;
endfunction

function addDamageElementalByWeapon(stats, weapon)
	var prop := getPropBase(weapon);

	stats.FireDamage += CInt(prop.FireDamage);
	stats.ColdDamage += CInt(prop.ColdDamage);
	stats.PoisonDamage += CInt(prop.PoisonDamage);
	stats.EnergyDamage += CInt(prop.EnergyDamage);

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



///////////
// ARMOR //
///////////
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

function addResistsPhysical(stats, armor)
	stats.+SlashResist := 0;
	stats.+PiercingResist := 0;
	stats.+BluntResist := 0;
	
	stats := addResistPhysicalBy("MATERIAL", stats, armor);
	stats := addResistPhysicalBy("SLOT", stats, armor);
	stats := addResistPhysicalBy("PROTECTION", stats, armor);

	return stats;
endfunction

function addResistPhysicalBy(type, stats, armor)
	var resists := array{};
	
	case(Upper(type))
		"MATERIAL":
			resists := getResistPhysicalByMaterial(armor);
			break;

		"SLOT":
			resists := getResistPhysicalBySlot(armor);
			break;

		"PROTECTION":
			resists := getResistPhysicalByProtection(armor);
			break;
	endcase

	stats.SlashResist += CInt(resists[1]);
	stats.PiercingResist += CInt(resists[2]);
	stats.BluntResist += CInt(resists[3]);

	return stats;
endfunction

function getResistPhysicalByProtection(armor)
	// TODO: Balancear os valores aqui, não é pra ser todos iguais.
	case(Upper(armor.getType())) // {SlashResist, PiercingResist, BluntResist}
		//TIER 1
		ARMOR_LOWLOW: return {0,0,0};
		ARMOR_LOWMID: return {1,1,1};
		ARMOR_LOWHIGH: return {2,2,2};

		//TIER 2
		ARMOR_MIDLOW: return {3,3,3};
		ARMOR_MIDMID: return {4,4,4};
		ARMOR_MIDHIGH: return {5,5,5};

		//TIER 3
		ARMOR_HIGHLOW: return {6,6,6};
		ARMOR_HIGHMID: return {7,7,7};
		ARMOR_HIGHHIGH: return {8,8,8};

		// Escudos também entram aqui porque atualmente eles não têm defesa. Se mudar, só adicionar as tags dos escudos aqui.
		default: return {0,0,0};
	endcase
endfunction

function getResistPhysicalBySlot(armor)
	if (armor.getType() && Upper(armor.getType()) != ARMOR_LOWLOW) // Só adiciona penalidade de Slot se não for roupa de pano
		case(Upper(armor.getSlot())) // {SlashResist, PiercingResist, BluntResist}
			HEAD: return {2,2,2};
			NECK: return {1,1,1};
			BODY: return {4,4,4};
			ARMS: return {2,2,2};
			HANDS: return {1,1,1};
			LEGS: return {3,3,3};
			FEET: return {2,2,2};
			
			default: return {0,0,0};
		endcase
	endif

	return {0,0,0};
endfunction

function getResistPhysicalByMaterial(armor)
	case (Upper(armor.getMaterialName())) // {SlashResist, PiercingResist, BluntResist}
		"COBRE": return {0,0,0};

		"BRONZE": return {1,1,1};
		"FERRO": return {1,1,1};
		"FERROTEMPERADO": return {2,2,2};

		"ACO": "FULMETIUM": "VENETIUM": "IGNIUM": "FRIGUTIUM": return {3,3,3};
		
		"ILLYRA": return {4,4,4};
		"MITHRIL": return {5,5,5};

		default: return {0,0,0};
	endcase
endfunction

function addResistsElemental(stats, armor)
	stats.+FireResist := 0;
	stats.+ColdResist := 0;
	stats.+PoisonResist := 0;
	stats.+EnergyResist := 0;

	stats := addResistElementalByArmor(stats, armor);

	return stats;
endfunction

function addResistElementalByArmor(stats, armor)
	var prop := getPropBase(armor);

	stats.FireResist += CInt(prop.FireResist);
	stats.ColdResist += CInt(prop.ColdResist);
	stats.PoisonResist += CInt(prop.PoisonResist);
	stats.EnergyResist += CInt(prop.EnergyResist);

	return stats;
endfunction

function addPenalties(stats, armor)
	stats.+DexPen := 0;
	stats.+SneakPen := 0;
	stats.+MAPen := 0;
	stats.+MageryPen := 0;
	stats.+AwarenessPen := 0;
	
	stats := addPenaltiesBy("SLOT", stats, armor);
	stats := addPenaltiesBy("PROTECTION", stats, armor);

	return stats;
endfunction

function addPenaltiesBy(type, stats, armor)
	var penalties := array{};
	
	case(Upper(type))
		"SLOT":
			penalties := getPenaltiesBySlot(armor);
			break;
		"PROTECTION":
			penalties := getPenaltiesByProtection(armor);
			break;
	endcase

	stats.DexPen += CInt(penalties[1]);
	stats.SneakPen += CInt(penalties[2]);
	stats.MAPen += CInt(penalties[3]);
	stats.MageryPen += CInt(penalties[4]);
	stats.AwarenessPen += CInt(penalties[5]);

	return stats;
endfunction

function getPenaltiesBySlot(armor)
	if (armor.getType() && Upper(armor.getType()) != ARMOR_LOWLOW) // Só adiciona penalidade de Slot se não for roupa de pano
		case(Upper(armor.getSlot())) // {DexPen, SneakPen, MAPen, MageryPen, AwarenessPen}
			HEAD: return {1,0,0,1,0};
			NECK: return {1,0,0,1,0};
			BODY: return {4,0,0,12,0};
			ARMS: return {2,0,2,6,0};
			HANDS: return {1,0,4,3,0};
			LEGS: return {2,0,0,6,0};
			FEET: return {2,0,0,0,0};
			SHIELD: return {0,1,10,5,0};

			default: return {0,0,0,0,0};
		endcase
	endif

	return {0,0,0,0,0};
endfunction

function getPenaltiesByProtection(armor)
	case(Upper(armor.getType())) // {DexPen, SneakPen, MAPen, MageryPen, AwarenessPen}
		//TIER 1
		ARMOR_LOWLOW: return {0,0,0,0,0};
		ARMOR_LOWMID: return {0,0,0,0,0};
		ARMOR_LOWHIGH: return {0,0,0,0,0};

		//TIER 2
		ARMOR_MIDLOW: return {0,0,0,2,0};
		ARMOR_MIDMID: return {1,0,0,4,0};
		ARMOR_MIDHIGH: return {2,0,0,5,0};

		//TIER 3
		ARMOR_HIGHLOW: return {3,0,0,8,0};
		ARMOR_HIGHMID: return {4,0,0,12,0};
		ARMOR_HIGHHIGH: return {5,0,0,24,0};

		// ESCUDOS
		SHIELD_BUCKLER: return {0,0,0,0,0};
		SHIELD_SMALL: return {4,0,0,0,0};
		SHIELD_MEDIUM: return {8,0,0,6,0};
		SHIELD_LARGE: return {12,0,0,12,0};
		SHIELD_TOWER: return {16,0,0,24,0};

		default: return {0,0,0,0,0};
	endcase
endfunction

function addShieldMods(stats, shield)
	var protection := shield.getType();
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