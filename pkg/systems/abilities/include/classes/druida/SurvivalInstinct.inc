use uo;
use os;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":attributes:attributes";
include ":timedscripts:timedScripts";
include "include/sounds";

/**
 * Instinto de Sobrevivencia (Survival Instinct) - ABILITY_ID::INSTINTO_DE_SOBREVIVENCIA (2102)
 *
 * Active (ON_USE):
 * - AoE heal all non-hostile mobs in range
 * - Range = Theology/10
 * - Healing = Theology/5 + 6d8
 *
 * Passive (via HasAbility checks in other files):
 * - Boosts polymorph stats (assumirforma.src, polymorph.src, formaprimordial.src)
 * - Uses Theology instead of Tactics for combat when polymorphed (DamageController.inc)
 */
class SurvivalInstinct(CombatAbility)

    function SurvivalInstinct(this)
        Super(ABILITY_ID::INSTINTO_DE_SOBREVIVENCIA);
        this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
    endfunction

    function ExecuteOn(this, mobile)
        var who := mobile.GetObjRef();

        var range := CInt(AP_GetSkill(who, THEOLOGY) / 10);
        var points := CInt(AP_GetSkill(who, THEOLOGY) / 5) + RandomDiceRoll("6d8");
        var hostiles := mobile.GetNearbyHostiles(range);
        var nearby := mobile.GetNearbyMobiles(range);

        foreach mob in nearby
            if (!(mob in hostiles))
                if (mobile.IsNPC() && mob.acctname)
                    continue;
                endif
                PlayObjectCenteredEffect(mob, FX_BLUE_SPARKLE_SWIRL, 7, 16);
                PlaySoundEffect(mob, SFX_HEAL);
                ApplyEpicHealing(mob, points);
            endif
        endforeach

        Message($"Voce curou {points} pontos de vida.").SendSys(who);

        DebugMsg(
            who,
            DEBUG_HAB,
            Message($"[INSTINTO_DE_SOBREVIVENCIA] Cura AoE: {points} HP, range {range}", Color(SYSTEM_COLOR::SUCCESS))
        );

        return true;
    endfunction

endclass
