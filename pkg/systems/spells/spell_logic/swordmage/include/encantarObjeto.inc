use uo;

include "include/client";
include ":objWrapper:Model/WeaponWrapper";
include ":objWrapper:Model/ArmorWrapper";
include "include/epicMaterials";

function setBonus(who, item, bonus)
	if (item.IsA(POLCLASS_ARMOR))
		item := ArmorWrapper(item);
	elseif (item.IsA(POLCLASS_WEAPON))
		item := WeaponWrapper(item);
	else
		return;
	endif

	var material := item.getMaterialID();
	bonus := CInt(bonus); // POL BUGADO DO CARALHO!!!!!!!!!!
	
	if (item is @WeaponWrapper)
		var damageMod := item.GetDamageMod();
		if (material == MATERIAL_OBJTYPE::IGNIUM)
			damageMod.IncreaseFireRawDamage(bonus);
		elseif (material == MATERIAL_OBJTYPE::VENETIUM)
			damageMod.IncreasePoisonRawDamage(bonus);
		elseif (material == MATERIAL_OBJTYPE::FRIGUTIUM)
			damageMod.IncreaseColdRawDamage(bonus);
		elseif (material == MATERIAL_OBJTYPE::FULMIUM)
			damageMod.IncreaseEnergyRawDamage(bonus);
		elseif (material == MATERIAL_OBJTYPE::ILLYRA)
			bonus := bonus * 0.6;
			damageMod.IncreaseFireRawDamage(bonus);
			damageMod.IncreaseColdRawDamage(bonus);
			damageMod.IncreasePoisonRawDamage(bonus);
			damageMod.IncreaseEnergyRawDamage(bonus);
		elseif (material == MATERIAL_OBJTYPE::MITHRIL)
			damageMod.IncreaseFireRawDamage(bonus);
			damageMod.IncreaseColdRawDamage(bonus);
			damageMod.IncreasePoisonRawDamage(bonus);
			damageMod.IncreaseEnergyRawDamage(bonus);
		endif
		item.SetDamageMod(damageMod);
	elseif (item is @ArmorWrapper)
		var resistMod := item.GetResistsMod();
		if (material == MATERIAL_OBJTYPE::IGNIUM)
			resistMod.IncreaseFireResist(bonus);
		elseif (material == MATERIAL_OBJTYPE::VENETIUM)
			resistMod.IncreasePoisonResist(bonus);
		elseif (material == MATERIAL_OBJTYPE::FRIGUTIUM)
			resistMod.IncreaseColdResist(bonus);
		elseif (material == MATERIAL_OBJTYPE::FULMIUM)
			resistMod.IncreaseEnergyResist(bonus);
		elseif (material == MATERIAL_OBJTYPE::ESCAMOSO)
			resistMod.IncreasePoisonResist(bonus);
		elseif (material == MATERIAL_OBJTYPE::DRACONIANO)
			resistMod.IncreaseFireResist(bonus);
		elseif (material == MATERIAL_OBJTYPE::GIGANTE)
			resistMod.IncreaseEnergyResist(bonus);
		elseif (material == MATERIAL_OBJTYPE::POLAR)
			resistMod.IncreaseColdResist(bonus);
		endif
		item.SetResistsMod(resistMod);
	endif

	Message("Você encantou o objeto.", FONTCOLOR_SUCCESS).SendSys(who.GetObjRef());
	PlayStationaryEffect(who.GetPosition().x, who.GetPosition().y, who.GetPosition().z, FX_SPARK_EFFECT, 2, 2);
	who.PlaySound(SFX_SPELL_HEAL);
endfunction