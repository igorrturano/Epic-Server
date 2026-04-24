use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":attributes:attributes";
include ":tn:cooldown";
include "include/sounds";
include "include/epicConstants";

/**
 * Fluencia (Fluency) - ABILITY_ID::FLUENCIA (2301)
 *
 * Active (ON_USE):
 * - Target enemy, drain their elemental resist based on weapon element (arcana*0.3)
 * - Grant self that resist amount
 * - Set #bomcomeco for guaranteed next hit (+50 in HitController)
 * - Duration = arcana/10
 * - Requires enchanted weapon (fire/ice/energy color)
 */
class Fluency(CombatAbility)

    function Fluency(this)
        Super(ABILITY_ID::FLUENCIA);
        this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
    endfunction

    function ExecuteOn(this, mobile)
        var who := mobile.GetObjRef();

        var targ := Target(who, TGTOPT_CHECK_LOS + TGTOPT_HARMFUL);
        if (mobile.IsNPC())
            targ := GetRandomNearbyHostile(who);
        endif

        if (!targ)
            Message("Cancelado.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
            return 0;
        endif

        if (targ.isa(POLCLASS_ITEM))
            Message("Alvo invalido.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
            return 0;
        endif

        var elemental := GetObjProperty(who.weapon, "#elementalweapon");
        if (elemental == error)
            Message("Sua arma nao esta encantada!", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
            return 0;
        endif

        _GrantBonus(who, targ);
        Message("Voce drenou parte da armadura do alvo.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
        Message("Suas defesas sao drenadas!", Color(SYSTEM_COLOR::FAIL)).SendSys(targ);
        mobile.GetObjRef().SetProp("#bomcomeco", 1);

        var duration := CInt(mobile.GetSkill(ARCANA) / 10);
        SetCooldown(who, "armor_drain", duration);

        DebugMsg(
            who,
            DEBUG_HAB,
            Message($"[FLUENCIA] Drain ativado por {duration}s", Color(SYSTEM_COLOR::SUCCESS))
        );

        sleep(duration);

        _RevokeBonus(who, targ);
        Message("O efeito se dissipa.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
        Message("Suas defesas normalizam.", Color(SYSTEM_COLOR::INFO)).SendSys(targ);

        return 1;
    endfunction

endclass

function _GrantBonus(who, targ)
    var weapon := who.weapon;
    var elemental := GetObjProperty(weapon, "#elementalweapon");
    var effect_color := 0;
    var armor_drain := CInt(AP_GetSkill(who, ARCANA) * 0.3);

    if (elemental.type == "Fire")
        effect_color := COLOR_ELEMENTAL_FIRE;
        targ.resist_fire_mod := targ.resist_fire_mod - armor_drain;
        who.resist_fire_mod := who.resist_fire_mod + armor_drain;
    elseif (elemental.type == "Cold")
        effect_color := COLOR_ELEMENTAL_ICE;
        targ.resist_cold_mod := targ.resist_cold_mod - armor_drain;
        who.resist_cold_mod := who.resist_cold_mod + armor_drain;
    elseif (elemental.type == "Energy")
        effect_color := COLOR_ELEMENTAL_ENERGY;
        targ.resist_energy_mod := targ.resist_energy_mod - armor_drain;
        who.resist_energy_mod := who.resist_energy_mod + armor_drain;
    endif

    PlaySoundEffect(who, 0x5CF);
    PlayObjectCenteredEffectHuefx(targ, 0x6db6, 10, 20, effect_color, 0);
endfunction

function _RevokeBonus(who, targ)
    var weapon := who.weapon;
    var elemental := GetObjProperty(weapon, "#elementalweapon");
    var armor_drain := CInt(AP_GetSkill(who, ARCANA) * 0.3);

    if (elemental.type == "Fire")
        targ.resist_fire_mod := targ.resist_fire_mod + armor_drain;
        who.resist_fire_mod := who.resist_fire_mod - armor_drain;
    elseif (elemental.type == "Cold")
        targ.resist_cold_mod := targ.resist_cold_mod + armor_drain;
        who.resist_cold_mod := who.resist_cold_mod - armor_drain;
    elseif (elemental.type == "Energy")
        targ.resist_energy_mod := targ.resist_energy_mod + armor_drain;
        who.resist_energy_mod := who.resist_energy_mod - armor_drain;
    endif
endfunction
