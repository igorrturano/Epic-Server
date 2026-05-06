use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":hotbar:custombuff";

/**
 * Movimentacao Graciosa (Graceful Movement) - ABILITY_ID::MOVIMENTACAO_GRACIOSA (1500)
 *
 * Active (SingleUse BEFORE_HIT - as DEFENDER):
 * - Guarantees a dodge: forces the next incoming hit to miss
 * - Displays "*Esquiva!*" above defender
 *
 * Passive (permanent via HasAbility in HitController:268-274):
 * - +tactics/10 to hit chance (attacker)
 * - -tactics/10 to hit chance (defender evasion)
 */
class GracefulMovement(CombatAbility)

    function GracefulMovement(this)
        Super(ABILITY_ID::MOVIMENTACAO_GRACIOSA);
        this.ActivationMoment := ABILITY_ACTIVATION::BEFORE_HIT;
        this.SingleUse := true;
    endfunction

    function GetDurationFor(this, mobile)
        var duration := CInt(mobile.GetSkill(TACTICS) / 5);
        if (duration < 2)
            duration := 2;
        endif
        return duration;
    endfunction

    function OnActivate(this, mobile)
        var who := mobile.GetObjRef();
        var duration := this.GetDurationFor(mobile);
        Message("*prepara uma esquiva graciosa*").SendAbove(who);
        SendAbilityBuff(who, ABILITY_ID::MOVIMENTACAO_GRACIOSA, duration, "Esquiva garantida");
    endfunction

    function OnBeforeHit(this, attackInfo)
        var defender := attackInfo.GetDefender();

        // Only trigger if the attack would actually hit
        var wouldHit := CInt(attackInfo.GetHitChanceResult()) <= CInt(attackInfo.GetHitChance()) + CInt(attackInfo.GetHitChanceMod());
        if (!wouldHit)
            DebugMsg(
                defender.GetObjRef(),
                DEBUG_HAB,
                Message("[MOVIMENTACAO_GRACIOSA] Habilidade não consumida porque o alvo errou.", Color(SYSTEM_COLOR::INFO))
            );
            return false;
        endif

        attackInfo.SetHitChanceMod(-100);

        Message("*Esquiva!*").SendAbove(defender.GetObjRef());
        Message("Você se esquivou de um ataque que teria te acertado!", Color(SYSTEM_COLOR::SUCCESS)).SendSys(defender.GetObjRef());

        DebugMsg(
            defender.GetObjRef(),
            DEBUG_HAB,
            Message("[MOVIMENTACAO_GRACIOSA] Esquiva garantida!", Color(SYSTEM_COLOR::SUCCESS))
        );

        return true;
    endfunction

endclass
