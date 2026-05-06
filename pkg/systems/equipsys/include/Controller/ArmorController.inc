use uo;
use math;

include ":equipsys:utils/consts";
include ":combat:Model/ArmorResist";
include ":combat:Model/ShieldAttributes";
include ":combat:Model/AttributeModifier";

// Valores em % de reducao de dano por tier de TipoProtecao (steps de 2%).
// Floor por tipo de armadura (Light 10 / Medium 15 / Heavy 20) e cap por tipo
// (50/60/70) sao aplicados em ArmorWrapper. Ver design_combate.md.
function GetResistPhysicalByProtection(tier)
	if (Upper(tier) == Upper(ARMOR_LOWLOW))
		return ArmorResist(0, 0, 0);
	elseif (Upper(tier) == Upper(ARMOR_LOWMID))
		return ArmorResist(2, 2, 2);
	elseif (Upper(tier) == Upper(ARMOR_LOWHIGH))
		return ArmorResist(4, 4, 4);
	elseif (Upper(tier) == Upper(ARMOR_MIDLOW))
		return ArmorResist(6, 6, 6);
	elseif (Upper(tier) == Upper(ARMOR_MIDMID))
		return ArmorResist(8, 8, 8);
	elseif (Upper(tier) == Upper(ARMOR_MIDHIGH))
		return ArmorResist(10, 10, 10);
	elseif (Upper(tier) == Upper(ARMOR_HIGHLOW))
		return ArmorResist(12, 12, 12);
	elseif (Upper(tier) == Upper(ARMOR_HIGHMID))
		return ArmorResist(14, 14, 14);
	elseif (Upper(tier) == Upper(ARMOR_HIGHHIGH))
		return ArmorResist(16, 16, 16);
	endif

	// Escudos também entram aqui porque atualmente eles não têm defesa. Se mudar, só adicionar as tags dos escudos nos IFs.
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
