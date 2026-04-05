include ":wrappers:Objects/Model/ArmorWrapper";
include ":wrappers:Objects/Model/WeaponWrapper";
include ":diceroll:model/DiceRoll";
include ":message:model/Message";
include ":runecarving:runeCarving";

include ":combat:Exceptions/InvalidEquipmentException";

function WearDownEquip(equip, weapon) // equip EquipWrapper, weapon WeaponWrapper
	if (!(equip is @EquipWrapper))
		return InvalidEquipmentException("The equipment must be an instance of ArmorWrapper or WeaponWrapper!");
	endif

	if (!(weapon is @WeaponWrapper))
		return InvalidEquipmentException("The weapon must be an instance of WeaponWrapper!");
	endif
	
	var wearChance := CalculateWearChance(equip);

	if (DiceRoll("1d100").Roll().GetResult() > wearChance)
		return;
	endif
	
	DamageEquipment(equip, weapon);
endfunction

function CalculateWearChance(equip)
	var wearChance := 0.0;
	
	if (equip is @ArmorWrapper)
		if (equip.IsShield())
			wearChance := 80.0;
		else
			wearChance := 30.0;
		endif

		if (equip.GetProperty("encerada"))
			wearChance -= 15.0;
		endif
	elseif (equip is @WeaponWrapper)
		wearChance := 20.0;
	endif

	if (wearChance < 5.0)
		wearChance := 5.0;
	endif

	return wearChance;
endfunction

function DamageEquipment(equip, weapon)
	var damage := 0;
	var extraDamage := 0;
	var owner := equip.GetOwner();
	
	if (equip is @ArmorWrapper)
		if (CInt(equip.GetProperty("priestrune") &&
			CInt(equip.GetProperty("runecharges")) > 0))
			DamageRuneCharges(equip);
			return;
		endif

		if (weapon.GetProperty("ArmorDmg") == 1)
			extraDamage := DiceRoll("1d4").Roll().GetResult();
		endif
	elseif (equip is @WeaponWrapper)
		if (equip.GetCategory() == WEAPON_CATEGORY::UNARMED)
			return; // Arma natural, não desgasta
		endif
	endif

	if (extraDamage > 0)
		Message($"{equip.GetName()} foi bem danificada!").SetColor(Color(SYSTEM_COLOR::BAD)).SendSys(owner);
	else
		Message($"{equip.GetName()} foi danificada.").SetColor(Color(SYSTEM_COLOR::BAD)).SendSys(owner);
	endif

	damage += extraDamage;
	ApplyDamageOnEquip(equip, damage);
endfunction

function DamageRuneCharges(equip)
	var rune := equip.GetProperty("priestrune");
	var charges := CInt(equip.GetProperty("runecharges"));

	if (charges > 0)
		charges -= 1;
		equip.SetProperty("runecharges", charges);
	endif

	if (charges <= 0)
		removeRuneBonus(equip, rune);
		Message($"A benção divina da {rune.name} em {equip.GetName()} se esvaiu.").SetColor(Color(SYSTEM_COLOR::BAD)).SendSys(equip.GetOwner());
	endif
endfunction

function ApplyDamageOnEquip(equip, damage)
	equip.SetHP(equip.GetHP() - damage);
	
	if (equip.GetHP() <= 0)
		BreakEquip(equip);
	endif

endfunction

/**
 * Marca um equipamento como danificado e o move para a mochila do proprietário.
 * 
 * @param equip O EquipWrapper a ser danificado.
 * 
 * @return void
 */
function BreakEquip(equip)
	// TODO: Revisar se isso aqui precisar ser assim mesmo
	// TODO: Herdado do weardown_legacy...
	
	equip.SetProperty("broken", 1);
	equip.SetProperty("realdesc", equip.desc);
	equip._SetName(equip.desc + " [danificado]");
	equip.MoveToContainer(equip.GetOwner().GetBackpack());
	
	// Gambiarra antiga, deixei aqui porque não sei se vai quebrar o refresh
	var nome := equip.GetObjRef().name;
	SetName(equip, nome + " -");
	sleepms(30);
	SetName(equip, nome);

	return true;
endfunction