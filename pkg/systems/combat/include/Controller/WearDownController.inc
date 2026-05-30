include ":wrappers:Objects/Model/ArmorWrapper";
include ":wrappers:Objects/Model/WeaponWrapper";
include ":diceroll:model/DiceRoll";
include ":message:model/Message";
include ":runecarving:runeCarving";
include ":combat:settings";

include ":combat:Exceptions/InvalidEquipmentException";

/**
 * Aplica o desgaste em um equipamento baseado em chance de dano.
 * 
 * Esta função realiza o processo completo de desgaste de um equipamento,
 * determinando se o equipamento será danificado por meio de um teste de dado
 * e, consequentemente, causará dano ao equipamento.
 * 
 * @param equip O equipamento (ArmorWrapper ou WeaponWrapper) a sofrer desgaste.
 * @param weapon (Opcional) A arma utilizada para calcular dano adicional ao armamento.
 *                          Se false, o dano não considera a arma. Padrão: false.
 * 
 * @return void. Lança InvalidEquipmentException se os parâmetros forem inválidos.
 * 
 * @throws InvalidEquipmentException Se equip não for uma instância de EquipWrapper.
 * @throws InvalidEquipmentException Se weapon não for false e nem uma instância de WeaponWrapper.
 * 
 * @example
 * WearDownEquip(myWeapon);
 * WearDownEquip(defensorArmor, attackerWeapon);
 */
function WearDownEquip(equip, weapon := false, wearMlt := 1) // equip EquipWrapper, weapon WeaponWrapper
	if (!(equip is @EquipWrapper))
		return InvalidEquipmentException("The equipment must be an instance of ArmorWrapper or WeaponWrapper!");
	endif

	if (weapon != false && !(weapon is @WeaponWrapper))
		return InvalidEquipmentException("The weapon must be an instance of WeaponWrapper!");
	endif

	// Cargas rúnicas se gastam a cada golpe, independente da rolagem de desgaste.
	if (equip is @ArmorWrapper)
		DamageRuneCharges(equip);
	endif

	var wearChance := CalculateWearChance(equip);

	if (Dice("1d100").Roll().GetResult() > wearChance)
		return;
	endif

	DamageEquipment(equip, weapon, wearMlt);
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
		wearChance := 2.0;
	endif

	if (wearChance < 5.0)
		wearChance := 5.0;
	endif

	return wearChance;
endfunction

function DamageEquipment(equip, weapon, wearMlt := 1)
	if (equip is @WeaponWrapper && equip.GetCategory() == WEAPON_CATEGORY::UNARMED)
		return; // Arma natural, não desgasta
	endif

	var damage := CalculateWearDamage(equip, weapon) * wearMlt;
	var owner := equip.GetOwner();

	Message($"{equip.GetName()} levou {damage} de dano.").SetColor(Color(SYSTEM_COLOR::BAD)).SendSys(owner.GetObjRef());

	ApplyDamageOnEquip(equip, damage);
endfunction

function CalculateWearDamage(equip, weapon := false)
	var wearRate := 0;

	if (equip is @WeaponWrapper)
		wearRate := CInt(CS_GetSettingsCfgElem("Weapons").WearRate);
	elseif (equip is @ArmorWrapper)
		wearRate := equip.IsShield() ? CInt(CS_GetSettingsCfgElem("Parry").WearRate)
									 : CInt(CS_GetSettingsCfgElem("Armor").WearRate);
	else
		return 0;
	endif

	// Armas com a propriedade "ArmorDmg" desgastam armaduras/escudos mais rápido.
	var bonus := 0;
	if (equip is @ArmorWrapper && weapon is @WeaponWrapper && CInt(weapon.GetProperty("ArmorDmg")) == 1)
		bonus := Dice("1d4").Roll().GetResult();
	endif

	return wearRate + bonus;
endfunction

function DamageRuneCharges(equip)
	var rune := equip.GetProperty("priestrune");
	if (!rune || rune is @Exception)
		return; // Equipamento sem runa.
	endif

	var charges := CInt(equip.GetProperty("runecharges"));
	if (charges <= 0)
		return; // Runa já esgotada.
	endif

	charges -= 1;
	equip.SetProperty("runecharges", charges);

	if (charges <= 0)
		removeRuneBonus(equip.GetObjRef(), rune);
		Message($"A benção divina da {rune} em {equip.GetName()} se esvaiu.").SetColor(Color(SYSTEM_COLOR::BAD)).SendSys(equip.GetOwner().GetObjRef());
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
	equip.SetProperty("realdesc", equip.GetObjRef().desc);
	equip._SetName(equip.GetObjRef().desc + " [danificado]");
	equip.MoveToContainer(equip.GetOwner().GetBackpack());
	
	// Gambiarra antiga, deixei aqui porque não sei se vai quebrar o refresh
	var nome := equip.GetObjRef().name;
	SetName(equip, nome + " -");
	sleepms(30);
	SetName(equip, nome);

	return true;
endfunction