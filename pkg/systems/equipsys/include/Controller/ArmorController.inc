use uo;
use math;

include ":equipsys:utils/consts";
include ":combat:Model/ArmorResist";
include ":combat:Model/ShieldAttributes";
include ":combat:Model/AttributeModifier";

// ArmorResist(slash, pierce, blunt) — valores finais por peça, sem floor.
// Light  → strong vs BLUNT,  mid vs PIERCE, weak vs SLASH
// Medium → strong vs SLASH,  mid vs BLUNT,  weak vs PIERCE
// Heavy  → strong vs PIERCE, mid vs SLASH,  weak vs BLUNT
// Cap por tipo (50/60/70) em ArmorWrapper._GetTypeCap.
function GetResistPhysicalByProtection(tier)
	if (Upper(tier) == Upper(ARMOR_LOWLOW))
		return ArmorResist(4, 6, 10);
	elseif (Upper(tier) == Upper(ARMOR_LOWMID))
		return ArmorResist(6, 9, 12);
	elseif (Upper(tier) == Upper(ARMOR_LOWHIGH))
		return ArmorResist(7, 11, 14);
	elseif (Upper(tier) == Upper(ARMOR_MIDLOW))
		return ArmorResist(21, 11, 16);
	elseif (Upper(tier) == Upper(ARMOR_MIDMID))
		return ArmorResist(23, 12, 17);
	elseif (Upper(tier) == Upper(ARMOR_MIDHIGH))
		return ArmorResist(25, 13, 19);
	elseif (Upper(tier) == Upper(ARMOR_HIGHLOW))
		return ArmorResist(24, 32, 16);
	elseif (Upper(tier) == Upper(ARMOR_HIGHMID))
		return ArmorResist(26, 34, 17);
	elseif (Upper(tier) == Upper(ARMOR_HIGHHIGH))
		return ArmorResist(27, 36, 18);
	endif

	// Shields: sem resist físico aqui (sistema separado).
	return ArmorResist(0, 0, 0);
endfunction

function GetResistPhysicalBySlot(slot)
	if (Upper(slot) == Upper(HEAD))
		return ArmorResist(2, 2, 2);
	elseif (Upper(slot) == Upper(NECK))
		return ArmorResist(1, 1, 1);
	elseif (Upper(slot) == Upper(BODY))
		return ArmorResist(4, 4, 4);
	elseif (Upper(slot) == Upper(ARMS))
		return ArmorResist(2, 2, 2);
	elseif (Upper(slot) == Upper(HANDS))
		return ArmorResist(1, 1, 1);
	elseif (Upper(slot) == Upper(LEGS))
		return ArmorResist(3, 3, 3);
	elseif (Upper(slot) == Upper(FEET))
		return ArmorResist(2, 2, 2);
	endif
		
	return ArmorResist(0, 0, 0);
endfunction

function GetModifiersBySlot(slot)
	var modifiers := array{};
	case(slot)
		HEAD: modifiers := {-1,-1,0,-1,0}; break;
		NECK: modifiers := {-1,-1,0,-1,0}; break;
		BODY: modifiers := {-4,-12,0,-12,0}; break;
		ARMS: modifiers := {-2,-6,-2,-6,0}; break;
		HANDS: modifiers := {-1,-3,-4,-3,0}; break;
		LEGS: modifiers := {-2,-6,0,-6,0}; break;
		FEET: modifiers := {-2,0,0,0,0}; break;
		SHIELD: modifiers := {0,-5,-10,-5,0}; break;

		default: modifiers := {0,0,0,0,0}; break;
	endcase

	return dictionary{
		DEXTERITY -> AttributeModifier(DEXTERITY, modifiers[1]),
		SNEAK -> AttributeModifier(SNEAK, modifiers[2]),
		MECHANICS -> AttributeModifier(MECHANICS, modifiers[3]),
		MAGERY -> AttributeModifier(MAGERY, modifiers[4]),
		AWARENESS -> AttributeModifier(AWARENESS, modifiers[5])
	};
endfunction

function GetModifiersByProtection(protection)
	var modifiers := array{};
	case(protection) // {Dexterity, Sneak, Mechanics, Magery, Awareness}
		//TIER 1
		ARMOR_LOWLOW: modifiers := {0,0,0,0,0}; break;
		ARMOR_LOWMID: modifiers := {0,0,0,0,0}; break;
		ARMOR_LOWHIGH: modifiers := {0,0,0,0,0}; break;

		//TIER 2
		ARMOR_MIDLOW: modifiers := {0,-2,0,-2,-20}; break;
		ARMOR_MIDMID: modifiers := {-1,-4,0,-4,-25}; break;
		ARMOR_MIDHIGH: modifiers := {-2,-5,0,-5,-30}; break;

		//TIER 3
		ARMOR_HIGHLOW: modifiers := {-3,-8,0,-8,-35}; break;
		ARMOR_HIGHMID: modifiers := {-4,-12,0,-12,-40}; break;
		ARMOR_HIGHHIGH: modifiers := {-5,-24,0,-24,-45}; break;

		// ESCUDOS
		SHIELD_BUCKLER: modifiers := {0,0,0,0,0}; break;
		SHIELD_SMALL: modifiers := {-4,0,0,0,0}; break;
		SHIELD_MEDIUM: modifiers := {-8,-6,0,-6,0}; break;
		SHIELD_LARGE: modifiers := {-12,-12,0,-12,0}; break;
		SHIELD_TOWER: modifiers := {-16,-24,0,-24,0}; break;

		default: modifiers := {0,0,0,0,0}; break;
	endcase

	return dictionary{
		DEXTERITY -> AttributeModifier(DEXTERITY, modifiers[1]),
		SNEAK -> AttributeModifier(SNEAK, modifiers[2]),
		MECHANICS -> AttributeModifier(MECHANICS, modifiers[3]),
		MAGERY -> AttributeModifier(MAGERY, modifiers[4]),
		AWARENESS -> AttributeModifier(AWARENESS, modifiers[5])
	};
endfunction

function GetShieldModsByProtection(tier)
	if (Upper(tier) == Upper(SHIELD_BUCKLER))
		return ShieldAttributes(-1, 0);
	elseif (Upper(tier) == Upper(SHIELD_SMALL))
		return ShieldAttributes(4, 8);
	elseif (Upper(tier) == Upper(SHIELD_MEDIUM))
		return ShieldAttributes(5, 10);
	elseif (Upper(tier) == Upper(SHIELD_LARGE))
		return ShieldAttributes(6, 12);
	elseif (Upper(tier) == Upper(SHIELD_TOWER))
		return ShieldAttributes(7, 14);
	endif

	return error{ errortext := "Value not found for this Shield Protection Tier!"};
endfunction
