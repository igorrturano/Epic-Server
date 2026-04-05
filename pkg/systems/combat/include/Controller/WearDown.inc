include ":wrappers:Objects/Model/ArmorWrapper";
include ":wrappers:Objects/Model/WeaponWrapper";

include ":combat:Exceptions/InvalidEquipmentException";

function WearDownEquip(equip, weapon) // equip EquipWrapper
	var wearChance := 0.0;

	if (equip is @ArmorWrapper)
		if (equip.IsShield())
			wearChance := 15.0;
		else
			wearChance := 30.0;
		endif
	elseif (equip is @WeaponWrapper)
		wearChance := 20.0;
	else
		return InvalidEquipmentException("The equipment must be an ArmorWrapper or WeaponWrapper!");
	endif


endfunction

function BreakEquip(equip)
	var hp := equip.GetHP();
	
	if ( armorhp <= 0 )
		SendSysMessage(defender, armor.desc+" foi destruida.");
		SetObjProperty(armor, "broken", 1);
		SetObjProperty(armor, "realdesc", armor.desc);
		SetName(armor, armor.desc+" [danificado]");
		MoveItemToContainer(armor, defender.backpack);
		//DestroyItem(armor);
	elseif ( armor.hp > armor.maxhp )
		armor.hp := armor.maxhp;
	else
		armor.hp := armorhp;
	endif
	var nome := armor.name;
	SetName(armor, nome + " -");
	sleepms(30);
	SetName(armor, nome);
endfunction