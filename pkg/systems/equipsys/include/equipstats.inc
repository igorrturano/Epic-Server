///////////////////////////////////////////////////////////////////////////////
///// Criado para substituir algumas funções do .cfg, que são limitadoras /////
///////////////////////////////////////////////////////////////////////////////

use os;
use cfgfile;
use math;

include "utils";
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
	stats := addCombatAttributes(stats, weapon);

	stats := addDurability(stats, weapon);
	stats := addWeight(stats, weapon);
	stats := addSTRRequired(stats, weapon);

	// TODO: Função para setar HP na prop
	// TODO: Função para setar PESO na prop
	// TODO: Função para setar STRRequired na prop

	return stats;
endfunction

function addDamagePhysical(stats, weapon)
	var elem := getItemCfgElem(weapon);
	var elemMainDamage := SplitWords(elem.EpicMainDamage);
	var elemOffDamage := SplitWords(elem.EpicOffDamage);

	stats.+SlashMainDamage := getDamageByTier(elemMainDamage[1]) ? getDamageByTier(elemMainDamage[1]) : 0;
	stats.+PiercingMainDamage := getDamageByTier(elemMainDamage[2]) ? getDamageByTier(elemMainDamage[2]) : 0;
	stats.+BluntMainDamage := getDamageByTier(elemMainDamage[3]) ? getDamageByTier(elemMainDamage[3]) : 0;

	if (weapon.isDualWield())
		stats.+SlashOffDamage := getDamageByTier(elemOffDamage[1]) ? getDamageByTier(elemOffDamage[1]) : 0;
		stats.+PiercingOffDamage := getDamageByTier(elemOffDamage[2]) ? getDamageByTier(elemOffDamage[2]) : 0;
		stats.+BluntOffDamage := getDamageByTier(elemOffDamage[3]) ? getDamageByTier(elemOffDamage[3]) : 0;
	endif

	return stats;
endfunction

function addDamageElemental(stats, weapon)
	// Danos físicos são dados, danos elementais são puros
	stats.+FireDamage := getDamageElemental("FIRE", weapon);
	stats.+ColdDamage := getDamageElemental("COLD", weapon);
	stats.+PoisonDamage := getDamageElemental("POISON", weapon);
	stats.+EnergyDamage := getDamageElemental("ENERGY", weapon);

	return stats;
endfunction

function getDamageElemental(DMGTYPE, weapon)
	case(DMGTYPE)
		"FIRE":
			return CInt(weapon.getAttrBase().FireDamage);

		"COLD":
			return CInt(weapon.getAttrBase().ColdDamage);

		"POISON":
			return CInt(weapon.getAttrBase().PoisonDamage);

		"ENERGY":
			return CInt(weapon.getAttrBase().EnergyDamage);

		default:
			return 0;	
	endcase
endfunction

function addCombatAttributes(stats, weapon)
	var elem := getItemCfgElem(weapon);

	stats.+AttackType := elem.TipoAtaque;
	stats.+AttackSpeed := getSpeedByTier(elem.EpicSpeed);
	stats.+CriticalChance := getCriticalChanceByTier(elem.EpicCriticalChance);

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

		// THROWING
		THROWING_LOW: return "1d4";
		THROWING_MID: return "1d6";
		THROWING_HIGH: return "1d8";
			
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
	if (armor.isShield())
		stats := addShieldMods(stats, armor);
	endif

	// TODO: Função para setar HP na prop
	// TODO: Função para setar PESO na prop
	// TODO: Função para setar STRRequired na prop

	stats := addDurability(stats, armor);
	stats := addWeight(stats, armor);
	stats := addSTRRequired(stats, armor);

	return stats;
endfunction

function addResistsPhysical(stats, armor)
	stats.+SlashResist := 0;
	stats.+PiercingResist := 0;
	stats.+BluntResist := 0;

	if (armor.isShield()) return stats; endif // Escudo não aplica resistências
	
	stats := addResistPhysicalBy("MATERIAL", stats, armor);
	stats := addResistPhysicalBy("SLOT", stats, armor);
	stats := addResistPhysicalBy("PROTECTION", stats, armor);

	return stats;
endfunction

function addResistPhysicalBy(type, stats, armor)
	var resists := dictionary{};
	
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

	stats.SlashResist += CInt(resists["SlashResist"]);
	stats.PiercingResist += CInt(resists["PiercingResist"]);
	stats.BluntResist += CInt(resists["BluntResist"]);

	return stats;
endfunction

function getResistPhysicalByProtection(armor)
	var resists := array{};
	// TODO: Balancear os valores aqui, não é pra ser todos iguais.
	case(Upper(armor.getType())) // {SlashResist, PiercingResist, BluntResist}
		//TIER 1
		ARMOR_LOWLOW: resists := {0,0,0}; break;
		ARMOR_LOWMID: resists := {1,1,1}; break;
		ARMOR_LOWHIGH: resists := {2,2,2}; break;

		//TIER 2
		ARMOR_MIDLOW: resists := {3,3,3}; break;
		ARMOR_MIDMID: resists := {4,4,4}; break;
		ARMOR_MIDHIGH: resists := {5,5,5}; break;

		//TIER 3
		ARMOR_HIGHLOW: resists := {6,6,6}; break;
		ARMOR_HIGHMID: resists := {7,7,7}; break;
		ARMOR_HIGHHIGH: resists := {8,8,8}; break;

		// Escudos também entram aqui porque atualmente eles não têm defesa. Se mudar, só adicionar as tags dos escudos aqui.
		default: resists := {0,0,0}; break;
	endcase

	return dictionary{
		"SlashResist" -> resists[1],
		"PiercingResist" -> resists[2],
		"BluntResist" -> resists[3]
	};
endfunction

function getResistPhysicalBySlot(armor)
	var resists := array{};
	if (armor.getType() && Upper(armor.getType()) != ARMOR_LOWLOW) // Só adiciona penalidade de Slot se não for roupa de pano
		case(Upper(armor.getSlot())) // {SlashResist, PiercingResist, BluntResist}
			HEAD: resists := {2,2,2}; break;
			NECK: resists := {1,1,1}; break;
			BODY: resists := {4,4,4}; break;
			ARMS: resists := {2,2,2}; break;
			HANDS: resists := {1,1,1}; break;
			LEGS: resists := {3,3,3}; break;
			FEET: resists := {2,2,2}; break;
			
			default: resists := {0,0,0}; break;
		endcase
	endif

	return dictionary{
		"SlashResist" -> resists[1],
		"PiercingResist" -> resists[2],
		"BluntResist" -> resists[3]
	};
endfunction

function getResistPhysicalByMaterial(armor)
	// TODO: Pensar em transformar isso daqui em uma função que retorne TODAS as infos do material em um struct: Danos (Físicos e Elementais), Resists (Físicos e Elementais), mod de peso, mod de HP, etc.
	var resists := array{};
	// TODO: Falta couros, tecidos, ossos, madeira e demais materiais de defesa.
	case (armor.getMaterialID()) // {SlashResist, PiercingResist, BluntResist}
		// TECIDOS
		0xf125: // Lã
		0xdf9: // Algodão
		0x1a94: // Seda
		0x1a9C: // Linho
		0xdfa: // Malva
		0xdfb: // Sisal
		0xdfc: // Denim
		0xdfd: // Rami
		0xdf6: // Flamel
		0xdf7: // Rajal

		// PELES
		0xee55: // Cinza
		0xee56: // Marrom
		0xee57: // Bege
		0xee58: // Polar

		// COUROS
		0xee45: // Curtido
		0xef24: // Peludo
		"COURO EFERVECIDO": // Efervecido
		0xef26: // Polar
		0xef20: // Escamoso 
		0xed29: // Draconiano
		0xef27: // Gigante
		"COURO DAS PROFUNDEZAS": resists := {0,0,0}; break; // Profundezas

		// METAIS
		0x6991: resists := {-2,-3,-4}; break;// Estanho
		0x6465: resists := {0,0,0}; break; // Cobre

		0x6994: resists := {1,1,1}; break; // Bronze
		0x62ff: resists := {1,1,1}; break; // Ferro
		0x6309: resists := {2,2,2}; break; // Ferro Temperado

		0x6995: // Aço
		0x630b: // Fulmetium
		0x630c: // Venetium
		0x630d: // Ignium
		0x6460: // Frigutium
			resists := {3,3,3}; break;
		
		0x6315: // Illyra
			resists := {4,4,4}; break;

		0x6311: // Mithril
			resists := {5,5,5}; break;

		// OSSOS
		"OSSO":
		"OSSO CINZA":
		"OSSO VERMELHO":
		"OSSO PRETO":
		"OSSO DE LICH":
		"OSSO REPITILIANO":
		"OSSO DE GIGANTE":
		"OSSO DE TERATHAN":
		"OSSO DEMONIACO":

		default: resists := {0,0,0}; break;
	endcase

	return dictionary{
		"SlashResist" -> resists[1],
		"PiercingResist" -> resists[2],
		"BluntResist" -> resists[3]
	};
endfunction

function addResistsElemental(stats, armor)
	stats.+FireResist := 0;
	stats.+ColdResist := 0;
	stats.+PoisonResist := 0;
	stats.+EnergyResist := 0;

	if (armor.isShield()) return stats; endif // Escudo não aplica resistências

	stats := addResistElementalByArmor(stats, armor);

	return stats;
endfunction

function addResistElementalByArmor(stats, armor)
	var attr := armor.getAttrBase();

	stats.FireResist += CInt(attr.FireResist);
	stats.ColdResist += CInt(attr.ColdResist);
	stats.PoisonResist += CInt(attr.PoisonResist);
	stats.EnergyResist += CInt(attr.EnergyResist);

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
	var penalties := dictionary{};
	
	case(Upper(type))
		"SLOT":
			penalties := getPenaltiesBySlot(armor);
			break;
		"PROTECTION":
			penalties := getPenaltiesByProtection(armor);
			break;
	endcase

	stats.DexPen += CInt(penalties["DexPen"]);
	stats.SneakPen += CInt(penalties["SneakPen"]);
	stats.MAPen += CInt(penalties["MAPen"]);
	stats.MageryPen += CInt(penalties["MageryPen"]);
	stats.AwarenessPen += CInt(penalties["AwarenessPen"]);

	return stats;
endfunction

function getPenaltiesBySlot(armor)
	var penalties := array{};
	if (armor.getType() && Upper(armor.getType()) != ARMOR_LOWLOW) // Só adiciona penalidade de Slot se não for roupa de pano
		case(Upper(armor.getSlot())) // {DexPen, SneakPen, MAPen, MageryPen, AwarenessPen}
			HEAD: penalties := {1,0,0,1,0}; break;
			NECK: penalties := {1,0,0,1,0}; break;
			BODY: penalties := {4,0,0,12,0}; break;
			ARMS: penalties := {2,0,2,6,0}; break;
			HANDS: penalties := {1,0,4,3,0}; break;
			LEGS: penalties := {2,0,0,6,0}; break;
			FEET: penalties := {2,0,0,0,0}; break;
			SHIELD: penalties := {0,1,10,5,0}; break;

			default: penalties := {0,0,0,0,0}; break;
		endcase
	endif

	return dictionary{
		"DexPen" -> penalties[1],
		"SneakPen" -> penalties[2],
		"MAPen" -> penalties[3],
		"MageryPen" -> penalties[4],
		"AwarenessPen" -> penalties[5]
	};
endfunction

function getPenaltiesByProtection(armor)
	var penalties := array{};
	case(Upper(armor.getType())) // {DexPen, SneakPen, MAPen, MageryPen, AwarenessPen}
		//TIER 1
		ARMOR_LOWLOW: penalties := {0,0,0,0,0}; break;
		ARMOR_LOWMID: penalties := {0,0,0,0,0}; break;
		ARMOR_LOWHIGH: penalties := {0,0,0,0,0}; break;

		//TIER 2
		ARMOR_MIDLOW: penalties := {0,0,0,2,0}; break;
		ARMOR_MIDMID: penalties := {1,0,0,4,0}; break;
		ARMOR_MIDHIGH: penalties := {2,0,0,5,0}; break;

		//TIER 3
		ARMOR_HIGHLOW: penalties := {3,0,0,8,0}; break;
		ARMOR_HIGHMID: penalties := {4,0,0,12,0}; break;
		ARMOR_HIGHHIGH: penalties := {5,0,0,24,0}; break;

		// ESCUDOS
		SHIELD_BUCKLER: penalties := {0,0,0,0,0}; break;
		SHIELD_SMALL: penalties := {4,0,0,0,0}; break;
		SHIELD_MEDIUM: penalties := {8,0,0,6,0}; break;
		SHIELD_LARGE: penalties := {12,0,0,12,0}; break;
		SHIELD_TOWER: penalties := {16,0,0,24,0}; break;

		default: penalties := {0,0,0,0,0}; break;
	endcase

	return dictionary{
		"DexPen" -> penalties[1],
		"SneakPen" -> penalties[2],
		"MAPen" -> penalties[3],
		"MageryPen" -> penalties[4],
		"AwarenessPen" -> penalties[5]
	};
endfunction

function addShieldMods(stats, shield)
	stats.+ParryDelay := 0;
	stats.+BonusDelay := 0;

	stats := addShieldModsBy("PROTECTION", stats, shield);

	return stats;
endfunction

function addShieldModsBy(type, stats, shield)
	var mods := dictionary{};

	case(Upper(type))
		"PROTECTION":
			mods := getShieldModsByProtection(shield);
			break;
	endcase

	stats.ParryDelay += CInt(mods["ParryDelay"]);
	stats.ParryBonus += CInt(mods["ParryBonus"]);

	return stats;
endfunction

function getShieldModsByProtection(shield)
	var mods := array{}; // {ParryDelay, ParryBonus}

	// TODO: Rever esses valores com o Igor, comparando com o .cfg (tá muito irregular lá)
	case(Upper(shield.getType()))
		SHIELD_BUCKLER: mods := {-1,0}; break;
		SHIELD_SMALL: mods := {4,8}; break;
		SHIELD_MEDIUM: mods := {5,10}; break;
		SHIELD_LARGE: mods := {6,12}; break;
		SHIELD_TOWER: mods := {7,14}; break;

		default: mods := {0,0}; break;
	endcase

	return dictionary{
		"ParryDelay" -> mods[1],
		"ParryBonus" -> mods[2]
	};
endfunction

function addDurability(stats, item)
	stats.+MaxHP := 0;

	stats.MaxHP += getDurabilityByType(item);
	if (item.isA(POLCLASS_ARMOR))
		stats.MaxHP += getDurabilityBySlot(item);
	endif

	stats.MaxHP := stats.MaxHP * getDurabilityModByMaterial(item); // Multiplica pelo MOD do material

	return stats;
endfunction

function getDurabilityByType(item)
	var maxhp := 0;
	
	if(item.isA(POLCLASS_WEAPON))
		case (item.getType())
			SLASHDAMAGE: maxhp := 45; break;
			PIERCINGDAMAGE: maxhp := 60; break;
			BLUNTDAMAGE: maxhp := 75; break;

			default: maxhp := 0; break;
		endcase
	elseif (item.isA(POLCLASS_ARMOR))
		if (item.isShield())
			case (item.getType().Upper())
				SHIELD_BUCKLER: maxhp := 40; break;
				SHIELD_SMALL: maxhp := 60; break;
				SHIELD_MEDIUM: maxhp := 80; break;
				SHIELD_LARGE: maxhp := 100; break;
				SHIELD_TOWER: maxhp := 120; break;

				default: maxhp := 0; break;
			endcase
		else
			var resists := getResistPhysicalByProtection(item);
			maxhp := Max(array{resists["SlashResist"], resists["PiercingResist"], resists["BluntResist"]});
		endif
	endif

	return maxhp;
endfunction

function getDurabilityBySlot(item)
	return Max(getResistPhysicalBySlot(item));
endfunction

function getDurabilityModByMaterial(item)
	var modifier := 0;

	case (item.getMaterialID())
		0x6309: modifier := 1; break; // Ferro

		default: modifier := 0.5;
	endcase

	return CDbl(modifier);
endfunction

function addWeight(stats, item)
	stats.+Weight := 0;

	stats.Weight += getWeightByType(item);

	stats.Weight := CInt(stats.Weight * getWeightModByMaterial(item));
	if (item.weight == 1)
		item.weight_multiplier_mod := stats.Weight;
	endif
	
	return stats;
endfunction

function getWeightModByMaterial(item)
var modifier := 0;

	case (item.getMaterialID())
		0x6994: modifier := 1.3; break; // Bronze
		0x6309: modifier := 1.1; break; // Ferro
		0x6995: modifier := 0.9; break; // Aço
        0x6315: modifier := 0.8; break; // Illyra
        0x6311: modifier := 0.7; break; // Mithril

		default: modifier := 1;
	endcase

	return CDbl(modifier);
endfunction

function getWeightByType(item)
	var weight;

	if(item.isA(POLCLASS_WEAPON))
		var maxMainDamage := item.getMainDamageMax();
		maxMainDamage := Max(array{maxMainDamage.SlashDamage, maxMainDamage.PiercingDamage, maxMainDamage.BluntDamage});

		var maxOffDamage := item.getOffDamageMax();
		maxOffDamage := Max(array{maxOffDamage.SlashDamage, maxOffDamage.PiercingDamage, maxOffDamage.BluntDamage});

		weight := CInt(maxMainDamage + maxOffDamage);
	elseif (item.isA(POLCLASS_ARMOR))
		if (item.isShield())
			weight := item.getAttrBase().DexPen;
		else
			// Poderia ter sido feito direto no weight. Deixar assim pra caso precise de algum MOD futuramente.
			var armorAttr := item.getAttrBase();
			var maxResist := Max(array{armorAttr.SlashResist, armorAttr.PiercingResist, armorAttr.BluntResist});
			maxResist += CInt(getDurabilityBySlot(item));
			weight := maxResist;
		endif
	endif
	return weight;
endfunction

function addSTRRequired(stats, item)
	stats.+STRRequired := 0;

	// stats.STRRequired += getSTRRequiredByType(item);

	var weight := getWeightByType(item);
	stats.STRRequired := CInt(weight * 7.5);
	return stats;
endfunction