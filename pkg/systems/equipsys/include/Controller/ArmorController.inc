use uo;
use math;

include ":equipsys:utils/consts";
include ":combat:Model/ArmorResist";
include ":combat:Model/ShieldAttributes";
include ":combat:Model/AttributeModifier";

function GetResistPhysicalByProtection(tier)
	// TODO: Balancear os valores aqui, não é pra ser todos iguais.
	if (Upper(tier) == Upper(ARMOR_LOWLOW))
		return ArmorResist(0, 0, 0);
	elseif (Upper(tier) == Upper(ARMOR_LOWMID))
		return ArmorResist(1, 1, 1);
	elseif (Upper(tier) == Upper(ARMOR_LOWHIGH))
		return ArmorResist(2, 2, 2);
	elseif (Upper(tier) == Upper(ARMOR_MIDLOW))
		return ArmorResist(3, 3, 3);
	elseif (Upper(tier) == Upper(ARMOR_MIDMID))
		return ArmorResist(4, 4, 4);
	elseif (Upper(tier) == Upper(ARMOR_MIDHIGH))
		return ArmorResist(5, 5, 5);
	elseif (Upper(tier) == Upper(ARMOR_HIGHLOW))
		return ArmorResist(6, 6, 6);
	elseif (Upper(tier) == Upper(ARMOR_HIGHMID))
		return ArmorResist(7, 7, 7);
	elseif (Upper(tier) == Upper(ARMOR_HIGHHIGH))
		return ArmorResist(8, 8, 8);
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

function getResistPhysicalByMaterial(ID)
	var resists := array{};

	// TODO: Falta couros, tecidos, ossos, madeira e demais materiais de defesa.
	case (ID) // {SlashResist, PiercingResist, BluntResist}
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

	return ArmorResist(resists[1], resists[2], resists[3]);
endfunction

function GetModifiersBySlot(slot)
	var modifiers := array{};
	case(Upper(slot))
		HEAD: modifiers := {-1,0,0,-1,0}; break;
		NECK: modifiers := {-1,0,0,-1,0}; break;
		BODY: modifiers := {-4,0,0,-12,0}; break;
		ARMS: modifiers := {-2,0,-2,-6,0}; break;
		HANDS: modifiers := {-1,0,-4,-3,0}; break;
		LEGS: modifiers := {-2,0,0,-6,0}; break;
		FEET: modifiers := {-2,0,0,0,0}; break;
		SHIELD: modifiers := {0,-1,-10,-5,0}; break;

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
	case(Upper(protection)) // {Dexterity, Sneak, Mechanics, Magery, Awareness}
		//TIER 1
		ARMOR_LOWLOW: modifiers := {0,0,0,0,0}; break;
		ARMOR_LOWMID: modifiers := {0,0,0,0,0}; break;
		ARMOR_LOWHIGH: modifiers := {0,0,0,0,0}; break;

		//TIER 2
		ARMOR_MIDLOW: modifiers := {0,0,0,-2,0}; break;
		ARMOR_MIDMID: modifiers := {-1,0,0,-4,0}; break;
		ARMOR_MIDHIGH: modifiers := {-2,0,0,-5,0}; break;

		//TIER 3
		ARMOR_HIGHLOW: modifiers := {-3,0,0,-8,0}; break;
		ARMOR_HIGHMID: modifiers := {-4,0,0,-12,0}; break;
		ARMOR_HIGHHIGH: modifiers := {-5,0,0,-24,0}; break;

		// ESCUDOS
		SHIELD_BUCKLER: modifiers := {0,0,0,0,0}; break;
		SHIELD_SMALL: modifiers := {-4,0,0,0,0}; break;
		SHIELD_MEDIUM: modifiers := {-8,0,0,-6,0}; break;
		SHIELD_LARGE: modifiers := {-12,0,0,-12,0}; break;
		SHIELD_TOWER: modifiers := {-16,0,0,-24,0}; break;

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
			var resists := getResistPhysicalByProtection(item.getType());
			maxhp := Max(array{resists.SlashResist, resists.PiercingResist, resists.BluntResist});
		endif
	endif

	return maxhp;
endfunction

function getDurabilityModByMaterial(material)
	var modifier := 0;

	case (material)
		0x6309: modifier := 1; break; // Ferro

		default: modifier := 0.5;
	endcase

	return CDbl(modifier);
endfunction

function getWeightModByMaterial(material)
	var modifier := 0;

	case (material)
		0x6994: modifier := 1.3; break; // Bronze
		0x6309: modifier := 1.1; break; // Ferro
		0x6995: modifier := 0.9; break; // Aço
		0x6315: modifier := 0.8; break; // Illyra
		0x6311: modifier := 0.7; break; // Mithril

		default: modifier := 1;
	endcase

	return CDbl(modifier);
endfunction

function getShieldModsByProtection(tier)
	if (Upper(tier) == Upper(SHIELD_BUCKLER))
		return ShieldResist(-1, 0);
	elseif (Upper(tier) == Upper(SHIELD_SMALL))
		return ShieldResist(4, 8);
	elseif (Upper(tier) == Upper(SHIELD_MEDIUM))
		return ShieldResist(5, 10);
	elseif (Upper(tier) == Upper(SHIELD_LARGE))
		return ShieldResist(6, 12);
	elseif (Upper(tier) == Upper(SHIELD_TOWER))
		return ShieldResist(7, 14);
	endif

	return error{ errortext := "Value not found for this Shield Protection Tier!"};
endfunction