use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":attributes:attributes";
include ":tn:cooldown";
include ":timedScripts:timedScripts";
include "include/sounds";
include "include/epicConstants";

/**
 * Armadura Arcana (Arcane Armor) - ABILITY_ID::ARMADURA_ARCANA (2303)
 *
 * Active (ON_USE):
 * - Grant +100 elemental resist based on weapon element
 * - Duration = arcana/10
 *
 * Passive (OnPassiveBeforeDamage - defender-on-hit):
 * - When swordmage is hit while armadura_arcana cooldown active
 * - Apply status effects to attacker if within 1 tile
 * - Fire weapon → "queimar" (burn), Ice → "congelamento" (freeze), Energy → "lentidao" (slow)
 */
class ArcaneArmor(CombatAbility)

    function ArcaneArmor(this)
        Super(ABILITY_ID::ARMADURA_ARCANA);
        this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
        this.IsDefenderSide := 1;
    endfunction

    function ExecuteOn(this, mobile)
        var who := mobile.GetObjRef();
        var duration := CInt(mobile.GetSkill(ARCANA) / 10);

        var element_type := _GrantArmorBonus(who, duration);
        if (element_type == "")
            return 0;
        endif

        Message("Sua armadura esta em comunhao com os elementos.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
        SetCooldown(who, "armadura_arcana", duration);

        DebugMsg(
            who,
            DEBUG_HAB,
            Message($"[ARMADURA_ARCANA] +100 resist, dur {duration}s", Color(SYSTEM_COLOR::SUCCESS))
        );

        sleep(duration);

        _RevokeArmorBonus(who, element_type);

        return 1;
    endfunction

    function OnPassiveBeforeDamage(this, attackInfo)
        var defender := attackInfo.GetDefender();
        var who := defender.GetObjRef();

        if (!GetCooldown(who, "armadura_arcana"))
            return false;
        endif

        var attacker := attackInfo.GetAttacker().GetObjRef();
        if (Distance(who, attacker) > 1)
            return false;
        endif

        var weapon := who.weapon;
        var elemental := GetObjProperty(weapon, "#elementalweapon");
        var status_duration := 5;

        if (elemental.type == "Fire")
            TS_StartTimer(attacker, "queimar", status_duration);
            PlaySoundEffect(attacker, 0x5cb);
            PlayObjectCenteredEffectHuefx(attacker, 0x6d77, 7, 30, COLOR_ELEMENTAL_FIRE, 0);
        elseif (elemental.type == "Cold")
            TS_StartTimer(attacker, "congelamento", status_duration);
            PlaySoundEffect(attacker, 0x5c8);
            PlayObjectCenteredEffectHuefx(attacker, 0x54f7, 30, 30, COLOR_ELEMENTAL_ICE, 0);
        elseif (elemental.type == "Energy")
            TS_StartTimer(attacker, "lentidao", status_duration);
            PlaySoundEffect(attacker, 0x666);
            PlayObjectCenteredEffectHuefx(attacker, 0xcdaf, 7, 30, COLOR_ELEMENTAL_ENERGY, 0);
        else
            return false;
        endif

        DebugMsg(
            who,
            DEBUG_HAB,
            Message($"[ARMADURA_ARCANA] Status aplicado ao atacante", Color(SYSTEM_COLOR::SUCCESS))
        );

        return true;
    endfunction

endclass

function _GrantArmorBonus(who, duration)
    var weapon := who.weapon;
    var elemental := GetObjProperty(weapon, "#elementalweapon");
    var element_type := "";

    if (elemental.type == "Fire")
        element_type := "Fire";
        who.resist_fire_mod := who.resist_fire_mod + 100;
    elseif (elemental.type == "Cold")
        element_type := "Cold";
        who.resist_cold_mod := who.resist_cold_mod + 100;
    elseif (elemental.type == "Energy")
        element_type := "Energy";
        who.resist_energy_mod := who.resist_energy_mod + 100;
    endif

    if (element_type != "")
        PlayObjectCenteredEffect(who, FX_ENERGY_GLOBE, 6, duration * 20);
    endif

    return element_type;
endfunction

function _RevokeArmorBonus(who, element_type)
    if (element_type == "Fire")
        who.resist_fire_mod := who.resist_fire_mod - 100;
    elseif (element_type == "Cold")
        who.resist_cold_mod := who.resist_cold_mod - 100;
    elseif (element_type == "Energy")
        who.resist_energy_mod := who.resist_energy_mod - 100;
    endif
endfunction
