use uo;

include "include/client";
include ":wrappers:Objects/Model/WeaponWrapper";
include ":wrappers:Objects/Model/ArmorWrapper";
include ":equipsys:packets/UpdateEquipmentRequest";
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

	// Le o material ANTES de embrulhar — getMaterialID e' uma method-dispatch
	// no methods.src do UObject (chama item.getProp("material")). Se chamada em
	// um wrapper class, retorna uninit silenciosamente e o spell vira no-op.
	var material;
	var materialName := "";  // Used for ammo (material name matching)
	if (!isAmmo)
		material := item.getMaterialID();
	endif

	if (item.IsA(POLCLASS_ARMOR))
		item := ArmorWrapper(item);
	elseif (item.IsA(POLCLASS_WEAPON))
		item := WeaponWrapper(item);
	elseif (!isAmmo)
		return;
	endif

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
	endif

	bonus := CInt(bonus);

	if (item is @WeaponWrapper)
		// Encantar = elemental flat damage por material. Persistido na CProp
		// EncantarDamage e relido via DamageModEncantar a cada construcao de
		// WeaponWrapper. Overwrite semantics: cada cast substitui o struct.
		var ench := struct{ fire := 0, cold := 0, poison := 0, energy := 0 };
		if (material == MATERIAL_OBJTYPE::IGNIUM)
			ench.fire := bonus;
		elseif (material == MATERIAL_OBJTYPE::VENETIUM)
			ench.poison := bonus;
		elseif (material == MATERIAL_OBJTYPE::FRIGUTIUM)
			ench.cold := bonus;
		elseif (material == MATERIAL_OBJTYPE::FULMIUM)
			ench.energy := bonus;
		elseif (material == MATERIAL_OBJTYPE::ILLYRA)
			var illyra := CInt(bonus * 0.6);
			ench.fire := illyra;
			ench.cold := illyra;
			ench.poison := illyra;
			ench.energy := illyra;
		elseif (material == MATERIAL_OBJTYPE::MITHRIL)
			ench.fire := bonus;
			ench.cold := bonus;
			ench.poison := bonus;
			ench.energy := bonus;
		endif
		item.SetProperty("EncantarDamage", ench);
	elseif (item is @ArmorWrapper)
		// Armor usa bonusPct (escala % do redesign). ILLYRA e MITHRIL adicionados
		// para paridade com o weapon branch — eram omissoes do codigo anterior, mas
		// o design (design_magia.md) e o test plan ("DV mage encanta mithril → 70%
		// em todos 4 elementos") os requerem.
		var ench := struct{ fire := 0, cold := 0, poison := 0, energy := 0 };
		if (material == MATERIAL_OBJTYPE::IGNIUM)
			ench.fire := bonusPct;
		elseif (material == MATERIAL_OBJTYPE::VENETIUM)
			ench.poison := bonusPct;
		elseif (material == MATERIAL_OBJTYPE::FRIGUTIUM)
			ench.cold := bonusPct;
		elseif (material == MATERIAL_OBJTYPE::FULMIUM)
			ench.energy := bonusPct;
		elseif (material == MATERIAL_OBJTYPE::ESCAMOSO)
			ench.poison := bonusPct;
		elseif (material == MATERIAL_OBJTYPE::DRACONIANO)
			ench.fire := bonusPct;
		elseif (material == MATERIAL_OBJTYPE::GIGANTE)
			ench.energy := bonusPct;
		elseif (material == MATERIAL_OBJTYPE::POLAR)
			ench.cold := bonusPct;
		elseif (material == MATERIAL_OBJTYPE::ILLYRA)
			var illyraPct := CInt(bonusPct * 0.6);
			ench.fire := illyraPct;
			ench.cold := illyraPct;
			ench.poison := illyraPct;
			ench.energy := illyraPct;
		elseif (material == MATERIAL_OBJTYPE::MITHRIL)
			ench.fire := bonusPct;
			ench.cold := bonusPct;
			ench.poison := bonusPct;
			ench.energy := bonusPct;
		endif
		item.SetProperty("EncantarResist", ench);
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

	// Se o alvo esta equipado no caster, dispara o packet 0xBF:0x63 para
	// atualizar o StatusGumpEpic na hora — sem isso o jogador so veria o
	// bonus elemental apos desequipar e reequipar a peca.
	if (item is @WeaponWrapper || item is @ArmorWrapper)
		var ref := item.GetObjRef();
		if (ref.container && ref.container.serial == who.GetObjRef().serial)
			SendUpdateEquipmentRequest(who, ref, 1, ref.layer);
		endif
	endif

	Message("Você encantou o objeto.", Color(SYSTEM_COLOR::SUCCESS)).SendSys(who.GetObjRef());
	PlayStationaryEffect(who.GetPosition().GetX(), who.GetPosition().GetY(), who.GetPosition().GetZ(), FX_SPARK_EFFECT, 2, 2);
	who.PlaySound(SFX_SPELL_HEAL);
endfunction