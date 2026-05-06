use uo;

include "include/client";
include ":wrappers:Objects/Model/WeaponWrapper";
include ":wrappers:Objects/Model/ArmorWrapper";
include "include/epicMaterials";

// Helper function to detect if item is ammunition
function IsAmmunition(item)
	var objtype := item.objtype;
	// Check against known ammo types: arrows, bolts, war arrows, war bolts, fireweapon ammo
	return objtype in array{0xf3f, 0x1bfb, 0x9088, 0x9098, 0x9090};
endfunction

// bonusFlat: usado por weapon/ammo (dano elemental adicional, escala legacy).
// bonusPct:  usado por armor (% de reducao adicional, escala nova do redesign).
// Decoupled em encantarobjeto.src — ver design_magia.md.
function setBonus(who, item, bonusFlat, bonusPct)
	var isAmmo := IsAmmunition(item);
	var bonus := bonusFlat; // alias usado pelas paths de weapon/ammo abaixo

	if (item.IsA(POLCLASS_ARMOR))
		item := ArmorWrapper(item);
	elseif (item.IsA(POLCLASS_WEAPON))
		item := WeaponWrapper(item);
	elseif (!isAmmo)
		return;
	endif

	// For ammunition, get material from damage component (arrowhead) in fabricado CProp
	var material;
	var materialName := "";  // Used for ammo (material name matching)

	if (isAmmo)
		var fabricado := item.GetItem().GetProperty(item, PROPNAME_FABRICADO);
		if (!fabricado || !fabricado.componentes)
			SendSysMessage(who, "Esta munição não possui componentes para ser encantada.");
			return;
		endif

		// Find the "dano" component (arrowhead) and extract its material NAME
		var danoComp := fabricado.componentes["dano"];
		if (!danoComp || !danoComp.material || !danoComp.material.nome)
			SendSysMessage(who, "Esta munição não possui material na ponta para ser encantada.");
			return;
		endif

		// Match material by NAME instead of hex code (due to data inconsistency)
		materialName := Lower(danoComp.material.nome);
	else
		material := item.getMaterialID();
	endif

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
		// Armor usa bonusPct (escala % do redesign). ILLYRA e MITHRIL adicionados
		// para paridade com o weapon branch — eram omissoes do codigo anterior, mas
		// o design (design_magia.md) e o test plan ("DV mage encanta mithril → 70%
		// em todos 4 elementos") os requerem.
		if (material == MATERIAL_OBJTYPE::IGNIUM)
			resistMod.IncreaseFireResist(bonusPct);
		elseif (material == MATERIAL_OBJTYPE::VENETIUM)
			resistMod.IncreasePoisonResist(bonusPct);
		elseif (material == MATERIAL_OBJTYPE::FRIGUTIUM)
			resistMod.IncreaseColdResist(bonusPct);
		elseif (material == MATERIAL_OBJTYPE::FULMIUM)
			resistMod.IncreaseEnergyResist(bonusPct);
		elseif (material == MATERIAL_OBJTYPE::ESCAMOSO)
			resistMod.IncreasePoisonResist(bonusPct);
		elseif (material == MATERIAL_OBJTYPE::DRACONIANO)
			resistMod.IncreaseFireResist(bonusPct);
		elseif (material == MATERIAL_OBJTYPE::GIGANTE)
			resistMod.IncreaseEnergyResist(bonusPct);
		elseif (material == MATERIAL_OBJTYPE::POLAR)
			resistMod.IncreaseColdResist(bonusPct);
		elseif (material == MATERIAL_OBJTYPE::ILLYRA)
			var illyraPct := CInt(bonusPct * 0.6);
			resistMod.IncreaseFireResist(illyraPct);
			resistMod.IncreaseColdResist(illyraPct);
			resistMod.IncreasePoisonResist(illyraPct);
			resistMod.IncreaseEnergyResist(illyraPct);
		elseif (material == MATERIAL_OBJTYPE::MITHRIL)
			resistMod.IncreaseFireResist(bonusPct);
			resistMod.IncreaseColdResist(bonusPct);
			resistMod.IncreasePoisonResist(bonusPct);
			resistMod.IncreaseEnergyResist(bonusPct);
		endif
		item.SetResistsMod(resistMod);
	elseif (isAmmo)
		// Ammunition enchantment - store elemental damage in AmmoDamage CProp
		var ammoDamage := struct{
			fire := 0,
			cold := 0,
			poison := 0,
			energy := 0
		};

		// Apply elemental damage based on material NAME (not hex code)
		if (materialName == "ignium")
			ammoDamage.fire := bonus;
		elseif (materialName == "venetium")
			ammoDamage.poison := bonus;
		elseif (materialName == "frigutium")
			ammoDamage.cold := bonus;
		elseif (materialName == "fulmium")
			ammoDamage.energy := bonus;
		elseif (materialName == "illyra")
			var illyraBonus := CInt(bonus * 0.6);
			ammoDamage.fire := illyraBonus;
			ammoDamage.cold := illyraBonus;
			ammoDamage.poison := illyraBonus;
			ammoDamage.energy := illyraBonus;
		elseif (materialName == "mithril")
			ammoDamage.fire := bonus;
			ammoDamage.cold := bonus;
			ammoDamage.poison := bonus;
			ammoDamage.energy := bonus;
		endif

		if (item.GetObjRef().amount >= 10)
			Message("Você não pode encantar pilhas de munição com mais de 10 unidades.", Color(SYSTEM_COLOR::FAIL)).SendSys(who.GetObjRef());
			return;
		endif
		SetObjProperty(item, "AmmoDamage", ammoDamage);
	endif

	Message("Você encantou o objeto.", Color(SYSTEM_COLOR::SUCCESS)).SendSys(who.GetObjRef());
	PlayStationaryEffect(who.GetPosition().GetX(), who.GetPosition().GetY(), who.GetPosition().GetZ(), FX_SPARK_EFFECT, 2, 2);
	who.PlaySound(SFX_SPELL_HEAL);
endfunction