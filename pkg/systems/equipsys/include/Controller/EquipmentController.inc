///////////////////////////////////////////////////////////////////////////////
///// Criado para substituir algumas funções do .cfg, que são limitadoras /////
///////////////////////////////////////////////////////////////////////////////

use os;
use cfgfile;
use math;

include ":equipsys:utils/consts";
include ":equipsys:ViewModel/EquipWrapperWeapon";
include ":equipsys:ViewModel/EquipWrapperArmor";
include ":equipsys:Controller/ArmorController";
include ":equipsys:Controller/WeaponController";

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
	var wrapped := EquipWrapperWeapon(weapon);
	var mainDamage := wrapped.getMainDamage();
	var offDamage := wrapped.getOffDamage();

	stats.+SlashMainDamage := mainDamage.SlashDamage;
	stats.+PiercingMainDamage := mainDamage.PiercingDamage;
	stats.+BluntMainDamage := mainDamage.BluntDamage;

	if (weapon.isDualWield())
		stats.+SlashOffDamage := offDamage.SlashDamage;
		stats.+PiercingOffDamage := offDamage.PiercingDamage;
		stats.+BluntOffDamage := offDamage.BluntDamage;
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

function addCombatAttributes(stats, weapon)
	var elem := weapon.getCfgElem();

	stats.+AttackType := elem.TipoAtaque;
	stats.+AttackSpeed := getSpeedByTier(elem.EpicSpeed);
	stats.+CriticalChance := getCriticalChanceByTier(elem.EpicCriticalChance);

	return stats;
endfunction

function getArmorStats(armor)
	var stats := struct{};

	stats := addResistsPhysical(stats, armor);
	stats := addResistsElemental(stats, armor);
	stats := addPenalties(stats, armor);
	if (armor.isShield())
		stats := addShieldMods(stats, armor);
	endif

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
	var resists := struct{};
	
	case(Upper(type))
		"MATERIAL":
			resists := getResistPhysicalByMaterial(armor.getMaterialID());
			break;

		"SLOT":
			resists := (armor.getType() && Upper(armor.getType()) != ARMOR_LOWLOW) ? getResistPhysicalBySlot(armor.getSlot()) : getResistPhysicalBySlot(0);
			break;

		"PROTECTION":
			resists := getResistPhysicalByProtection(armor.getType());
			break;
	endcase

	stats.SlashResist += CInt(resists.SlashResist);
	stats.PiercingResist += CInt(resists.PiercingResist);
	stats.BluntResist += CInt(resists.BluntResist);

	return stats;
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
	var penalties;

	case(Upper(type))
		"SLOT":
			penalties := (armor.getType() && Upper(armor.getType()) != ARMOR_LOWLOW) ? GetModifiersBySlot(armor.getSlot) : GetModifiersBySlot(0);
			break;
		"PROTECTION":
			penalties := GetModifiersByProtection(armor.getType());
			break;
	endcase

	stats.DexPen += CInt(penalties.DexPen);
	stats.SneakPen += CInt(penalties.SneakPen);
	stats.MAPen += CInt(penalties.MAPen);
	stats.MageryPen += CInt(penalties.MageryPen);
	stats.AwarenessPen += CInt(penalties.AwarenessPen);

	return stats;
endfunction

function addShieldMods(stats, shield)
	stats.+ParryDelay := 0;
	stats.+BonusDelay := 0;

	stats := addShieldModsBy("PROTECTION", stats, shield);

	return stats;
endfunction

function addShieldModsBy(type, stats, shield)
	var mods := struct{};

	case(Upper(type))
		"PROTECTION":
			mods := getShieldModsByProtection(shield.getType());
			break;
	endcase

	stats.ParryDelay += CInt(mods.ParryDelay);
	stats.ParryBonus += CInt(mods.ParryBonus);

	return stats;
endfunction

function addDurability(stats, item)
	stats.+MaxHP := 0;

	stats.MaxHP += getDurabilityByType(item);
	if (item.isA(POLCLASS_ARMOR))
		stats.MaxHP += getDurabilityBySlot(item) * 2;
	endif

	stats.MaxHP := stats.MaxHP * getDurabilityModByMaterial(item.getMaterialID());

	return stats;
endfunction

function getDurabilityBySlot(item)
	return Max(getResistPhysicalBySlot(item.getSlot()));
endfunction

function addWeight(stats, item)
	stats.+Weight := 0;

	stats.Weight += getWeightByType(item);

	stats.Weight := CInt(stats.Weight * getWeightModByMaterial(item.getMaterialID()));
	if (item.weight == 1)
		item.weight_multiplier_mod := stats.Weight;
	endif
	
	return stats;
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
			maxResist += CInt(getDurabilityBySlot(item.getSlot()));
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