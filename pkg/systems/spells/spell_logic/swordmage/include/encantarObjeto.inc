use uo;

include ":equipsys:props";
include "include/client";
include "include/say";

function setBonus(who, item, bonus)
	var prop := item.getAttrBase();
	var material := item.getMaterialID();
	bonus := CInt(bonus); // POL BUGADO DO CARALHO!!!!!!!!!!
	
	if (item.isA(POLCLASS_WEAPON))
		case (material)
			0x630d: // Ignium
				prop.FireDamage := bonus;
				break;
			0x6303: // Venetita
				prop.PoisonDamage := bonus;
				break;
			0x6460: // Frigutium
				prop.ColdDamage := bonus;
				break;
			0x630b: // Fulmium
				prop.EnergyDamage := bonus;
				break;

			0x6315: // Illyra
				bonus := bonus * 0.6;
				prop.FireDamage := bonus;
				prop.ColdDamage := bonus;
				prop.PoisonDamage := bonus;
				prop.EnergyDamage := bonus;
				break;

			0x6311: // Mithril
				prop.FireDamage := bonus;
				prop.ColdDamage := bonus;
				prop.PoisonDamage := bonus;
				prop.EnergyDamage := bonus;
				break;
		endcase
	elseif (item.isA(POLCLASS_ARMOR))
		
		case (material)
			0x630d: // Ignium
				prop.FireResist := bonus;
				break;
			0x6303: // Venetita
				prop.PoisonResist := bonus;
				break;
			0x6460: // Frigutium
				prop.ColdResist := bonus;
				break;
			0x630b: // Fulmium
				prop.EnergyResist := bonus;
				break;
			0xef20: // Escamoso
				prop.PoisonResist := bonus;
				break;
			0xed29: // Draconiano
				prop.FireResist := bonus;
				break;
			0xef27: //Gigante
				prop.EnergyResist := bonus;
				break;
			0xef26: //Polar
				prop.ColdResist := bonus;
				break;
		endcase
	endif

	setPropBase(item, prop);
	SendSysMessageEx(who, "Você encantou o objeto.", SSM_INFO);
	PlayStationaryEffect(who.x, who.y, who.z, FX_SPARK_EFFECT, 2, 2);
	PlaySoundEffect(who, SFX_SPELL_HEAL);
endfunction