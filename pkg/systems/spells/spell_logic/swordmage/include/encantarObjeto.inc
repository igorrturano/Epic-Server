use uo;

include "include/client";
include "include/say";

function setBonus(who, item, bonus)
	var material := item.getMaterial();
	bonus := CInt(bonus); // POL BUGADO DO CARALHO!!!!!!!!!!
	
	if (item.isA(POLCLASS_WEAPON))
		case (material)
			0x630d: // Ignium
				item.damage_fire_mod := bonus;
				break;
			0x6303: // Venetita
				item.damage_poison_mod := bonus;
				break;
			0x6460: // Frigutium
				item.damage_cold_mod := bonus;
				break;
			0x630b: // Fulmium
				item.damage_energy_mod := bonus;
				break;

			0x6315: // Illyra
				bonus := bonus * 0.6;
				item.damage_fire_mod := bonus;
				item.damage_cold_mod := bonus;
				item.damage_poison_mod := bonus;
				item.damage_energy_mod := bonus;
				break;

			0x6311: // Mithril
				item.damage_fire_mod := bonus;
				item.damage_cold_mod := bonus;
				item.damage_poison_mod := bonus;
				item.damage_energy_mod := bonus;
				break;
		endcase
	elseif (item.isA(POLCLASS_ARMOR))
		
		case (material)
			0x630d: // Ignium
				item.resist_fire_mod := bonus;
				break;
			0x6303: // Venetita
				item.resist_poison_mod := bonus;
				break;
			0x6460: // Frigutium
				item.resist_cold_mod := bonus;
				break;
			0x630b: // Fulmium
				item.resist_energy_mod := bonus;
				break;
			0xef20: // Escamoso
				item.resist_poison_mod := bonus;
				break;
			0xed29: // Draconiano
				item.resist_fire_mod := bonus;
				break;
			0xef27: //Gigante
				item.resist_energy_mod := bonus;
				break;
			0xef26: //Polar
				item.resist_cold_mod := bonus;
				break;
		endcase
	endif

	SendSysMessageEx(who, "Você encantou o objeto.", SSM_INFO);
	PlayStationaryEffect(who.x, who.y, who.z, FX_SPARK_EFFECT, 2, 2);
	PlaySoundEffect(who, SFX_SPELL_HEAL);
endfunction