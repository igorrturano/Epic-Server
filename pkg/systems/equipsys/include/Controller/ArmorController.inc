use uo;
use math;

include ":equipsys:utils/consts";
include ":combat:Model/ArmorResist";
include ":combat:Model/CombatPenalty";

function getResistPhysicalByProtection(tier)
	var resists := array{};
	
	// TODO: Balancear os valores aqui, não é pra ser todos iguais.
	case(Upper(tier))
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

	return ArmorResist(resists[1], resists[2], resists[3]);
endfunction

function getResistPhysicalBySlot(slot)
	var resists := array{};
	
	case(Upper(slot[1])) // {SlashResist, PiercingResist, BluntResist}
			HEAD: resists := {2,2,2}; break;
			NECK: resists := {1,1,1}; break;
			BODY: resists := {4,4,4}; break;
			ARMS: resists := {2,2,2}; break;
			HANDS: resists := {1,1,1}; break;
			LEGS: resists := {3,3,3}; break;
			FEET: resists := {2,2,2}; break;

			default: resists := {0,0,0}; break;
		endcase

	return ArmorResist(resists[1], resists[2], resists[3]);
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

function getPenaltiesBySlot(slot)
	var penalties := array{};
	case(Upper(slot[1]))
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

	return dictionary{
		DEXTERITY -> CombatPenalty(DEXTERITY, penalties[1]),
		SNEAK -> CombatPenalty(SNEAK, penalties[2]),
		MECHANICS -> CombatPenalty(MECHANICS, penalties[3]),
		MAGERY -> CombatPenalty(MAGERY, penalties[4]),
		AWARENESS -> CombatPenalty(AWARENESS, penalties[5])
	};
endfunction

function getPenaltiesByProtection(protection)
	var penalties := array{};
	case(Upper(protection)) // {DexPen, SneakPen, MAPen, MageryPen, AwarenessPen}
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
		DEXTERITY -> CombatPenalty(DEXTERITY, penalties[1]),
		SNEAK -> CombatPenalty(SNEAK, penalties[2]),
		MECHANICS -> CombatPenalty(MECHANICS, penalties[3]),
		MAGERY -> CombatPenalty(MAGERY, penalties[4]),
		AWARENESS -> CombatPenalty(AWARENESS, penalties[5])
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

function getShieldModsByProtection(type)
	var mods := array{}; // {ParryDelay, ParryBonus}

	case(Upper(type))
		SHIELD_BUCKLER: mods := {-1,0}; break;
		SHIELD_SMALL: mods := {4,8}; break;
		SHIELD_MEDIUM: mods := {5,10}; break;
		SHIELD_LARGE: mods := {6,12}; break;
		SHIELD_TOWER: mods := {7,14}; break;

		default: mods := {0,0}; break;
	endcase

	return struct{
		ParryDelay := mods[1],
		ParryBonus := mods[2]
	};
endfunction
