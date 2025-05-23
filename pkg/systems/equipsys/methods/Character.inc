include ":equipsys:utils";
include ":combat:armorZones";

print("INSTALLING: Methods for Character -> Equipment System... OK!");

exported function getMobileDefenses(who)
	var armorSlot := struct{
		HEAD := CS_GetEquipmentInArmorZone(who, HEAD)[1],
		NECK := CS_GetEquipmentInArmorZone(who, NECK)[1],
		BODY := CS_GetEquipmentInArmorZone(who, BODY)[1],
		ARMS := CS_GetEquipmentInArmorZone(who, ARMS)[1],
		LEGS := CS_GetEquipmentInArmorZone(who, LEGS)[1],
		FEET := CS_GetEquipmentInArmorZone(who, FEET)[1],
		HANDS := CS_GetEquipmentInArmorZone(who, HANDS)[1],
		SHIELD := CS_GetEquipmentInArmorZone(who, SHIELD)[1]
	};

	return struct{
		HEAD := struct{
			SLASHDAMAGE := CInt(armorSlot.HEAD.getAttrBase().SlashResist),
			PIERCINGDAMAGE := CInt(armorSlot.HEAD.getAttrBase().PiercingResist),
			BLUNTDAMAGE := CInt(armorSlot.HEAD.getAttrBase().BluntResist)
		},
		NECK := struct{
			SLASHDAMAGE := CInt(armorSlot.NECK.getAttrBase().SlashResist),
			PIERCINGDAMAGE := CInt(armorSlot.NECK.getAttrBase().PiercingResist),
			BLUNTDAMAGE := CInt(armorSlot.NECK.getAttrBase().BluntResist)
		},
		BODY := struct{
			SLASHDAMAGE := CInt(armorSlot.BODY.getAttrBase().SlashResist),
			PIERCINGDAMAGE := CInt(armorSlot.BODY.getAttrBase().PiercingResist),
			BLUNTDAMAGE := CInt(armorSlot.BODY.getAttrBase().BluntResist)
		},
		ARMS := struct{
			SLASHDAMAGE := CInt(armorSlot.ARMS.getAttrBase().SlashResist),
			PIERCINGDAMAGE := CInt(armorSlot.ARMS.getAttrBase().PiercingResist),
			BLUNTDAMAGE := CInt(armorSlot.ARMS.getAttrBase().BluntResist)
		},
		LEGS := struct{
			SLASHDAMAGE := CInt(armorSlot.LEGS.getAttrBase().SlashResist),
			PIERCINGDAMAGE := CInt(armorSlot.LEGS.getAttrBase().PiercingResist),
			BLUNTDAMAGE := CInt(armorSlot.LEGS.getAttrBase().BluntResist)
		},
		FEET := struct{
			SLASHDAMAGE := CInt(armorSlot.FEET.getAttrBase().SlashResist),
			PIERCINGDAMAGE := CInt(armorSlot.FEET.getAttrBase().PiercingResist),
			BLUNTDAMAGE := CInt(armorSlot.FEET.getAttrBase().BluntResist)
		},
		HANDS := struct{
			SLASHDAMAGE := CInt(armorSlot.HANDS.getAttrBase().SlashResist),
			PIERCINGDAMAGE := CInt(armorSlot.HANDS.getAttrBase().PiercingResist),
			BLUNTDAMAGE := CInt(armorSlot.HANDS.getAttrBase().BluntResist)
		},
		SHIELD := struct{
			SLASHDAMAGE := CInt(armorSlot.SHIELD.getAttrBase().SlashResist),
			PIERCINGDAMAGE := CInt(armorSlot.SHIELD.getAttrBase().PiercingResist),
			BLUNTDAMAGE := CInt(armorSlot.SHIELD.getAttrBase().BluntResist)
		}
	};
endfunction