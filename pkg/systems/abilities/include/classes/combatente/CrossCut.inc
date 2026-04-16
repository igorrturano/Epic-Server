use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":hotbar:custombuff";

/**
 * Contragolpe Cruzado (Cross Cut) - ABILITY_ID::CONTRAGOLPE_CRUZADO (202)
 *
 * Active (BeforeHit - as DEFENDER):
 * - First hit: Guarantees parry (+100 mod via SetParryChanceMod)
 * - All hits: Counter-attack flag - FOR TT/5 SECONDS
 */
class CrossCut(CombatAbility)

    function CrossCut(this)
        Super(ABILITY_ID::CONTRAGOLPE_CRUZADO);
        this.ActivationMoment := ABILITY_ACTIVATION::BEFORE_HIT;
    endfunction

    function GetDurationFor(this, mobile)
        var duration := CInt(mobile.GetSkill(TACTICS) / 5);
        if (duration < 1)
            duration := 1;
        endif
        return duration;
    endfunction

    // On activation: flag for guaranteed first parry
    function OnActivate(this, mobile)
        mobile.GetObjRef().SetProp("#contragolpe_first_parry", true);

        var duration := this.GetDurationFor(mobile);
        SendAbilityBuff(mobile.GetObjRef(), ABILITY_ID::CONTRAGOLPE_CRUZADO, duration, "Defesa garantida + contra-ataques");
    endfunction

    // Every hit while active: counter-attack + guaranteed parry on first hit
    function OnBeforeHit(this, attackInfo)
        var defender := attackInfo.GetDefender();

        // FIRST HIT: Guaranteed parry (one time per activation)
        if (defender.GetObjRef().GetProp("#contragolpe_first_parry"))
            defender.GetObjRef().EraseProp("#contragolpe_first_parry");
            attackInfo.SetParryChanceMod(100);

            Message("*prepara o contragolpe cruzado!*").SendAbove(defender.GetObjRef());

            DebugMsg(
                defender.GetObjRef(),
                DEBUG_HAB,
                Message("[CONTRAGOLPE_CRUZADO] Defesa garantida!", Color(SYSTEM_COLOR::SUCCESS))
            );
        endif

        // EVERY HIT: Enable counter-attack
        attackInfo.SetCounterAttack(true);

        DebugMsg(
            defender.GetObjRef(),
            DEBUG_HAB,
            Message("[CONTRAGOLPE_CRUZADO] Contra-ataque ativado!", Color(SYSTEM_COLOR::SUCCESS))
        );

        return true;
    endfunction

endclass
