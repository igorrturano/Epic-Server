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


/**
 * Marca um equipamento como danificado e o move para a mochila do proprietário.
 * 
 * @param equip O EquipWrapper a ser danificado.
 * @param owner O MobileWrapper dono do equipamento.
 * 
 * @return void
 */
function BreakEquip(equip, owner)
	equip.SetProperty("broken", 1);
	equip.SetProperty("realdesc", equip.desc);
	equip._SetName(equip.desc+" [danificado]");
	MoveItemToContainer(equip, owner.GetBackpack().GetObjRef());
	
	var nome := equip.name;
	SetName(equip, nome + " -");
	sleepms(30);
	SetName(equip, nome);
endfunction