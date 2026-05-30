use uo;
use os;

include ":attributes:attributes";
include ":attributes:vitals";

// Applies/removes the `equip` ObjProperty modifier dict on a mobile.
// Stamps the applied delta on `#equipMods_<serial>` so RemoveFrom can
// subtract exactly what ApplyOn added, decoupled from later mutations
// to the source dict or `magic` flag.

const EQUIPMOD_STAMP_PREFIX := "#equipMods_";

class EquipModStamper()
    var _item;

    function EquipModStamper(this, item)
        this._item := item;
    endfunction

    function _StampKey(this)
        return EQUIPMOD_STAMP_PREFIX + CStr(this._item.serial);
    endfunction

    // magic == 1 → unidentified, skip (Arcana/Theology flip to 2).
    // No magic property → crafted gear, always apply.
    function _ShouldApply(this)
        var magic_state := GetObjProperty(this._item, "magic");
        if (magic_state == error)
            return 1;
        endif
        return (CInt(magic_state) == 2);
    endfunction

    function _ComputeDelta(this)
        var mods := GetObjProperty(this._item, "equip");
        var delta := struct{
            strmod    := 0,
            dexmod    := 0,
            intmod    := 0,
            hitsmod   := 0,
            manamod   := 0,
            stammod   := 0,
            resists   := struct{
                ColdResistMod     := 0,
                FireResistMod     := 0,
                PhysicalResistMod := 0,
                EnergyResistMod   := 0,
                PoisonResistMod   := 0
            },
            setprop   := struct{ name := "", value := 0 },
            skillmods := dictionary{}
        };
        if (!mods)
            return delta;
        endif
        if (mods.size() < 1)
            return delta;
        endif

        foreach key in (mods.keys())
            case (key)
                "strmod":  delta.strmod  := CInt(mods[key]);
                "dexmod":  delta.dexmod  := CInt(mods[key]);
                "intmod":  delta.intmod  := CInt(mods[key]);
                "hitsmod": delta.hitsmod := CInt(mods[key]);
                "manamod": delta.manamod := CInt(mods[key]);
                "stammod": delta.stammod := CInt(mods[key]);
                "ColdResistMod":
                    delta.resists.+ColdResistMod := CInt(mods[key]);
                "FireResistMod":
                    delta.resists.+FireResistMod := CInt(mods[key]);
                "PhysicalResistMod":
                    delta.resists.+PhysicalResistMod := CInt(mods[key]);
                "EnergyResistMod":
                    delta.resists.+EnergyResistMod := CInt(mods[key]);
                "PoisonResistMod":
                    delta.resists.+PoisonResistMod := CInt(mods[key]);
                "setprop":
                    delta.setprop.+name  := CStr(GetObjProperty(this._item, "propname"));
                    delta.setprop.+value := GetObjProperty(this._item, "propvalue");
                "sacred":
                "cursed":
                default:
                    delta.skillmods[key] := CInt(mods[key]);
            endcase
        endforeach

        return delta;
    endfunction

    function ApplyOn(this, mobile)
        if (!this._item || !this._item.serial)
            return 0;
        endif
        if (!mobile || mobile == error)
            return 0;
        endif

        var key := this._StampKey();
        if (GetObjProperty(mobile, key) != error)
            return 1;
        endif

        if (!this._ShouldApply())
            return 1;
        endif

        var delta := this._ComputeDelta();

        if (delta.strmod != 0)
            AP_ModifyStatMod(mobile, STRENGTH, delta.strmod);
        endif
        if (delta.dexmod != 0)
            AP_ModifyStatMod(mobile, DEXTERITY, delta.dexmod);
        endif
        if (delta.intmod != 0)
            AP_ModifyStatMod(mobile, INTELLIGENCE, delta.intmod);
        endif

        var vitals_touched := 0;
        if (delta.hitsmod != 0)
            SetObjProperty(mobile, "#hitsmod",
                CInt(GetObjProperty(mobile, "#hitsmod")) + delta.hitsmod);
            vitals_touched := 1;
        endif
        if (delta.manamod != 0)
            SetObjProperty(mobile, "#manamod",
                CInt(GetObjProperty(mobile, "#manamod")) + delta.manamod);
            vitals_touched := 1;
        endif
        if (delta.stammod != 0)
            SetObjProperty(mobile, "#stammod",
                CInt(GetObjProperty(mobile, "#stammod")) + delta.stammod);
            vitals_touched := 1;
        endif
        if (delta.resists.PhysicalResistMod
         || delta.resists.ColdResistMod
         || delta.resists.FireResistMod
         || delta.resists.EnergyResistMod
         || delta.resists.PoisonResistMod)
            vitals_touched := 1;
        endif
        if (vitals_touched)
            RecalcVitals(mobile);
        endif

        if (delta.setprop.name != "")
            SetObjProperty(mobile, "#" + delta.setprop.name, delta.setprop.value);
        endif

        foreach skill in (delta.skillmods.keys())
            var val := CInt(delta.skillmods[skill]);
            if (val != 0)
                AP_ModifySkillMod(mobile, skill, val);
            endif
        endforeach

        SetObjProperty(mobile, key, delta);
        return 1;
    endfunction

    function RemoveFrom(this, mobile)
        if (!this._item || !this._item.serial)
            return 0;
        endif
        if (!mobile || mobile == error)
            return 0;
        endif

        var key := this._StampKey();
        var delta := GetObjProperty(mobile, key);
        if (delta == error)
            return 1;
        endif

        if (CInt(delta.strmod) != 0)
            AP_ModifyStatMod(mobile, STRENGTH, -CInt(delta.strmod));
        endif
        if (CInt(delta.dexmod) != 0)
            AP_ModifyStatMod(mobile, DEXTERITY, -CInt(delta.dexmod));
        endif
        if (CInt(delta.intmod) != 0)
            AP_ModifyStatMod(mobile, INTELLIGENCE, -CInt(delta.intmod));
        endif

        var vitals_touched := 0;
        if (CInt(delta.hitsmod) != 0)
            var new_hits := CInt(GetObjProperty(mobile, "#hitsmod")) - CInt(delta.hitsmod);
            if (new_hits <= 0)
                EraseObjProperty(mobile, "#hitsmod");
            else
                SetObjProperty(mobile, "#hitsmod", new_hits);
            endif
            vitals_touched := 1;
        endif
        if (CInt(delta.manamod) != 0)
            SetObjProperty(mobile, "#manamod",
                CInt(GetObjProperty(mobile, "#manamod")) - CInt(delta.manamod));
            vitals_touched := 1;
        endif
        if (CInt(delta.stammod) != 0)
            SetObjProperty(mobile, "#stammod",
                CInt(GetObjProperty(mobile, "#stammod")) - CInt(delta.stammod));
            vitals_touched := 1;
        endif
        if (delta.resists.PhysicalResistMod
         || delta.resists.ColdResistMod
         || delta.resists.FireResistMod
         || delta.resists.EnergyResistMod
         || delta.resists.PoisonResistMod)
            vitals_touched := 1;
        endif
        if (vitals_touched)
            RecalcVitals(mobile);
        endif

        if (delta.setprop.name != "")
            EraseObjProperty(mobile, "#" + delta.setprop.name);
        endif

        foreach skill in (delta.skillmods.keys())
            var val := CInt(delta.skillmods[skill]);
            if (val != 0)
                AP_ModifySkillMod(mobile, skill, -val);
            endif
        endforeach

        EraseObjProperty(mobile, key);
        return 1;
    endfunction
endclass
