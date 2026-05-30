use uo;

include ":crafting:/include/caracteristicasFabricacao";
include ":crafting:/include/fabricacaoUtil";
include ":crafting:/include/util/fabricacaoConstants";
include ":wrappers:Objects/Model/WeaponWrapper";
include ":attributes:vitals";
include ":attributes:regen";

include "IModificator";

// Tracks max-mana / mana-regen bonuses sourced from a STAFF/WAND/SPELLBOOK
// crafted with manaMaxima / manaRegen characteristics. Mirrors the
// DamageModFabricacao / ResistModFabricacao pattern (one class per stat
// channel that reads PegaValorFabricacao on the equipped item) but writes
// to native vital fields instead of a Damage/Resist struct.
//
// Apply: bumps `#manamod` (additive max-mana mod, 1 unit = +1 displayed
// mana) and `AP_SetVitalRegenRateMod(mob, "mana", current + regenBonus)`.
// Per-item applied delta is stamped on a CProp on the mobile so the
// unequip path can reverse the exact value, even if cfgs change while
// equipped.
class ManaModFabricacao(IModificator)
	var _itemOrTemplate;
	var _maxBonus := 0;
	var _regenBonus := 0;

	function ManaModFabricacao(this, itemOrTemplate)
		this._itemOrTemplate := itemOrTemplate;
		this._maxBonus := PegaValorMaterialPrincipalFabricacao(itemOrTemplate, CARACTERISTICA_FABRICACAO_MANA_MAXIMA);
		this._regenBonus := PegaValorMaterialPrincipalFabricacao(itemOrTemplate, CARACTERISTICA_FABRICACAO_MANA_REGEN);
	endfunction

	function GetSource(this)
		return MOD_SOURCES::CRAFTING;
	endfunction

	function GetMod(this)
		return struct{
			manaMax := this._maxBonus,
			manaRegen := this._regenBonus
		};
	endfunction

	function SetMod(this, mod)
		this._maxBonus := CInt(mod.manaMax);
		this._regenBonus := CInt(mod.manaRegen);
	endfunction

	function HasBonus(this)
		return (this._maxBonus != 0 || this._regenBonus != 0);
	endfunction

	// Stamps the applied delta on the mobile keyed by the item's serial so
	// RemoveFrom can roll back the exact amount even if the source item's
	// values change while equipped.
	function ApplyOn(this, mobile)
		if (!this.HasBonus())
			return 1;
		endif

		var serial := this._itemOrTemplate.serial;
		if (!serial)
			return 0;
		endif

		var key := $"#manaModFab_{serial}";
		if (GetObjProperty(mobile, key) != error)
			// Already applied (e.g. resync after relogin) — skip to avoid stacking.
			return 1;
		endif

		var applied := struct{
			manaMax := this._maxBonus,
			manaRegen := this._regenBonus
		};

		if (this._maxBonus != 0)
			var current := CInt(GetObjProperty(mobile, "#manamod"));
			SetObjProperty(mobile, "#manamod", current + this._maxBonus);
			RecalcVitals(mobile);
		endif

		if (this._regenBonus != 0)
			var current := AP_GetVitalRegenRateMod(mobile, "mana");
			AP_SetVitalRegenRateMod(mobile, "mana", current + this._regenBonus);
		endif

		SetObjProperty(mobile, key, applied);
		return 1;
	endfunction

	function RemoveFrom(this, mobile)
		var serial := this._itemOrTemplate.serial;
		if (!serial)
			return 0;
		endif

		var key := $"#manaModFab_{serial}";
		var applied := GetObjProperty(mobile, key);
		if (applied == error)
			return 1;
		endif

		if (CInt(applied.manaMax) != 0)
			var current := CInt(GetObjProperty(mobile, "#manamod"));
			SetObjProperty(mobile, "#manamod", current - CInt(applied.manaMax));
			RecalcVitals(mobile);
		endif

		if (CInt(applied.manaRegen) != 0)
			var current := AP_GetVitalRegenRateMod(mobile, "mana");
			AP_SetVitalRegenRateMod(mobile, "mana", current - CInt(applied.manaRegen));
		endif

		EraseObjProperty(mobile, key);
		return 1;
	endfunction
endclass
