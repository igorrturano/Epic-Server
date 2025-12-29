include ":equipsys:utils/consts";
include ":combat:Model/DamageTier";

function getDamageElemental(DMGTYPE, weapon)
	case(Upper(DMGTYPE))
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
	case (Upper(tier))
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
 * getDamageByTier(tier)
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
	case (Upper(tier))
		// NO DAMAGE
		DAMAGE_TIER::DAMAGE_NONE: return 0;

		DAMAGE_TIER::UNARMED_LOW: return "1d2";
		DAMAGE_TIER::UNARMED_MID: return "2d2";
		DAMAGE_TIER::UNARMED_HIGH: return "3d2";
		
		// ONE HANDED
		DAMAGE_TIER::ONEHANDED_LOW: return "1d4";
		DAMAGE_TIER::ONEHANDED_MID: return "1d6";
		DAMAGE_TIER::ONEHANDED_HIGH: return "1d8";
			
		// TWO HANDED
		DAMAGE_TIER::TWOHANDED_LOW: return "1d8";
		DAMAGE_TIER::TWOHANDED_MID: return "1d10";
		DAMAGE_TIER::TWOHANDED_HIGH: return "1d12";

		// BOW
		DAMAGE_TIER::BOW_LOW: return "1d4";
		DAMAGE_TIER::BOW_MID: return "1d6";
		DAMAGE_TIER::BOW_HIGH: return "1d8";

		// CROSSBOW
		DAMAGE_TIER::CROSSBOW_LOW: return "1d6";
		DAMAGE_TIER::CROSSBOW_MID: return "1d8";
		DAMAGE_TIER::CROSSBOW_HIGH: return "1d10";

		// THROWING
		DAMAGE_TIER::THROWING_LOW: return "1d4";
		DAMAGE_TIER::THROWING_MID: return "1d6";
		DAMAGE_TIER::THROWING_HIGH: return "1d8";

		// FIREWEAPON
		DAMAGE_TIER::FIREWEAPON_LOW: return "1d6";
		DAMAGE_TIER::FIREWEAPON_MID: return "1d8";
		DAMAGE_TIER::FIREWEAPON_HIGH: return "1d10";

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
	case(Upper(tier))
		CRITICALCHANCE_NOCHANCE: return 0;
		CRITICALCHANCE_LOW: return 1;
		CRITICALCHANCE_MID: return 3;
		CRITICALCHANCE_HIGH: return 5;
		
		default: return error{ "errortext" := "Value not found for this Weapon Critical Chance Tier!"};
	endcase
endfunction

/**
 * ApplyDamageOverrides(weapon, mobile)
 *
 * Purpose
 * Applies damage overrides to a weapon based on mobile state (metamorfose or NPC template).
 * Called when retrieving equipped weapon to ensure damage reflects current state.
 * No caching - always-current state is simpler than invalidation logic.
 *
 * Parameters
 * weapon: WeaponWrapper instance to modify
 * mobile: MobileWrapper instance to check for overrides
 *
 * Return
 * Modified weapon (for chaining)
 */
function ApplyDamageOverrides(byref weapon, byref mobile)
	// Check for metamorfose transformation first (higher priority)
	if (GetObjProperty(mobile.GetObjRef(), "metamorfose") != error)
		ApplyMetamorfoseDamage(weapon, mobile);
	elseif (mobile.IsNPC())
		ApplyNPCTemplateDamage(weapon, mobile);
	endif
	
	return weapon;
endfunction

/**
 * ApplyMetamorfoseDamage(weapon, mobile)
 *
 * Purpose
 * Applies damage from metamorfose transformation to weapon.
 * Checks for "dicedmg" property set by transformation spells.
 *
 * Parameters
 * weapon: WeaponWrapper instance to modify
 * mobile: MobileWrapper instance with metamorfose properties
 *
 * Return
 * Modified weapon
 */
function ApplyMetamorfoseDamage(byref weapon, byref mobile)
	var dicedmgProp := GetObjProperty(mobile.GetObjRef(), "dicedmg");
	if (dicedmgProp != error)
		// Use dicedmg from metamorfose transformation (array of [slash, piercing, blunt])
		weapon.SetMainDamage(Dice(dicedmgProp[1]), Dice(dicedmgProp[2]), Dice(dicedmgProp[3]));
	endif
	
	return weapon;
endfunction

/**
 * ApplyNPCTemplateDamage(weapon, mobile)
 *
 * Purpose
 * Applies damage from NPC template to weapon.
 * NPCs use template-defined damage instead of physical weapon stats.
 *
 * Parameters
 * weapon: WeaponWrapper instance to modify
 * mobile: MobileWrapper instance with NPC template
 *
 * Return
 * Modified weapon
 */
function ApplyNPCTemplateDamage(byref weapon, byref mobile)
	var template := mobile.GetTemplate();
	if (template is @Exception)
		return weapon;
	endif
	
	// Apply physical attack damage from template
	weapon.SetMainDamage(
		Dice(GetDamageByTier(SplitWords(template.EpicAttackDamage)[1])),
		Dice(GetDamageByTier(SplitWords(template.EpicAttackDamage)[2])),
		Dice(GetDamageByTier(SplitWords(template.EpicAttackDamage)[3]))
	);
	
	// Apply elemental damage modifiers from template
	weapon.SetDamageMod(Damage(
		GetDamageByTier(SplitWords(template.EpicElemDamage)[1]),
		GetDamageByTier(SplitWords(template.EpicElemDamage)[2]),
		GetDamageByTier(SplitWords(template.EpicElemDamage)[3]),
		GetDamageByTier(SplitWords(template.EpicElemDamage)[4])
	));
	
	return weapon;
endfunction