use uo;
use math;

include ":equipsys:utils/consts";

function getResistPhysicalByProtection(armor)
	var resists := array{};
	
	resists := getResistPhysicalByProtectionType(armor.getType());

	return dictionary{
		"SlashResist" -> resists[1],
		"PiercingResist" -> resists[2],
		"BluntResist" -> resists[3]
	};
endfunction

function getResistPhysicalByProtectionType(type)
	// TODO: Balancear os valores aqui, não é pra ser todos iguais.
	case(Upper(type)) // {SlashResist, PiercingResist, BluntResist}
		//TIER 1
		ARMOR_LOWLOW: return {0,0,0}; break;
		ARMOR_LOWMID: return {1,1,1}; break;
		ARMOR_LOWHIGH: return {2,2,2}; break;

		//TIER 2
		ARMOR_MIDLOW: return {3,3,3}; break;
		ARMOR_MIDMID: return {4,4,4}; break;
		ARMOR_MIDHIGH: return {5,5,5}; break;

		//TIER 3
		ARMOR_HIGHLOW: return {6,6,6}; break;
		ARMOR_HIGHMID: return {7,7,7}; break;
		ARMOR_HIGHHIGH: return {8,8,8}; break;

		// Escudos também entram aqui porque atualmente eles não têm defesa. Se mudar, só adicionar as tags dos escudos aqui.
		default: return {0,0,0}; break;
	endcase
endfunction

function getResistPhysicalBySlotArmor(armor)
	var resists := array{};
	if (armor.getType() && Upper(armor.getType()) != ARMOR_LOWLOW) // Só adiciona penalidade de Slot se não for roupa de pano
		resists := getResistPhysicalBySlot(armor.getSlot());
	endif

	return dictionary{
		"SlashResist" -> resists[1],
		"PiercingResist" -> resists[2],
		"BluntResist" -> resists[3]
	};
endfunction

function getResistPhysicalBySlot(slot)
	case(Upper(slot)) // {SlashResist, PiercingResist, BluntResist}
			HEAD: return {2,2,2}; break;
			NECK: return {1,1,1}; break;
			BODY: return {4,4,4}; break;
			ARMS: return {2,2,2}; break;
			HANDS: return {1,1,1}; break;
			LEGS: return {3,3,3}; break;
			FEET: return {2,2,2}; break;
			
			default: return {0,0,0}; break;
		endcase
endfunction

function getResistPhysicalByMaterial(armor)
	// TODO: Pensar em transformar isso daqui em uma função que retorne TODAS as infos do material em um struct: Danos (Físicos e Elementais), Resists (Físicos e Elementais), mod de peso, mod de HP, etc.
	var resists := array{};
	
	resists := getResistPhysicalByMaterialID(armor.getMaterialID());

	return dictionary{
		"SlashResist" -> resists[1],
		"PiercingResist" -> resists[2],
		"BluntResist" -> resists[3]
	};
endfunction

function getResistPhysicalByMaterialID(ID)
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
		"COURO DAS PROFUNDEZAS": return {0,0,0}; break; // Profundezas

		// METAIS
		0x6991: return {-2,-3,-4}; break;// Estanho
		0x6465: return {0,0,0}; break; // Cobre

		0x6994: return {1,1,1}; break; // Bronze
		0x62ff: return {1,1,1}; break; // Ferro
		0x6309: return {2,2,2}; break; // Ferro Temperado

		0x6995: // Aço
		0x630b: // Fulmetium
		0x630c: // Venetium
		0x630d: // Ignium
		0x6460: // Frigutium
			return {3,3,3}; break;
		
		0x6315: // Illyra
			return {4,4,4}; break;

		0x6311: // Mithril
			return {5,5,5}; break;

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

		default: return {0,0,0}; break;
	endcase
endfunction

function getPenaltiesBySlot(slot)
	var penalties := array{};
	case(Upper(slot))
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

	return struct{
		DexPen := penalties[1],
		SneakPen := penalties[2],
		MAPen := penalties[3],
		MageryPen := penalties[4],
		AwarenessPen := penalties[5]
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

	return struct{
		DexPen := penalties[1],
		SneakPen := penalties[2],
		MAPen := penalties[3],
		MageryPen := penalties[4],
		AwarenessPen := penalties[5]
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
			var resists := getResistPhysicalByProtection(item);
			maxhp := Max(array{resists["SlashResist"], resists["PiercingResist"], resists["BluntResist"]});
		endif
	endif

	return maxhp;
endfunction

function getDurabilityModByMaterial(item)
	var modifier := 0;

	case (item.getMaterialID())
		0x6309: modifier := 1; break; // Ferro

		default: modifier := 0.5;
	endcase

	return CDbl(modifier);
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