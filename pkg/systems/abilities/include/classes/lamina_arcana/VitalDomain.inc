use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":attributes:attributes";
include "include/client";
include "include/sounds";
include "include/utils";

/**
 * Dominio Vital (Vital Domain) - ABILITY_ID::DOMINIO_VITAL (2302)
 *
 * Active (ON_USE):
 * - Push/pull mechanic based on distance
 * - If target at 1 tile: push away (arcana/10 tiles)
 * - If target far: pull to self
 *
 * Passive (via HasAbility checks):
 * - Boosts enchant object bonus [encantarobjeto.src]
 */
class VitalDomain(CombatAbility)

    function VitalDomain(this)
        Super(ABILITY_ID::DOMINIO_VITAL);
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

        var dist := Distance(who, targ);
        var total_distance := CInt(mobile.GetSkill(ARCANA) / 10);

        PlayObjectCenteredEffectEx(targ, 0xB468, 10, 20, 0, 1);

        if (dist == 1)
            PushBack(targ, total_distance, 300);
            Message("Voce empurrou o alvo para longe.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
        else
            MoveObjectToLocation(targ, who.x, who.y, who.z, who.realm);
            Message("Voce puxou o alvo para perto.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
        endif

        PlayObjectCenteredEffectEx(targ, 0xB468, 10, 20, 0, 1);

        DebugMsg(
            who,
            DEBUG_HAB,
            Message($"[DOMINIO_VITAL] Dist: {dist}, Push/Pull: {total_distance}", Color(SYSTEM_COLOR::SUCCESS))
        );

        return 1;
    endfunction

endclass
