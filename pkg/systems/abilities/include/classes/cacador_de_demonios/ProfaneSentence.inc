use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":tn:cooldown";
include ":attributes:attributes";
include ":timedscripts:timedScripts";

/**
 * Sentenca Profana (Profane Sentence) - ABILITY_ID::SENTENCA_PROFANA (1901)
 *
 * Active (BEFORE_DAMAGE, SingleUse):
 * - On next hit, applies religion-specific effects + forced divine damage (THEOLOGY/10)
 * - VENERATIO: silence
 * - BOKK_BHARAZ: slowness + visual projectile
 * - HALLEUAD: blind
 * - ELUITHORTH: darkness
 * - KAI: bleeding strike
 */
class ProfaneSentence(CombatAbility)

    function ProfaneSentence(this)
        Super(ABILITY_ID::SENTENCA_PROFANA);
        this.ActivationMoment := ABILITY_ACTIVATION::BEFORE_DAMAGE;
        this.SingleUse := true;
    endfunction

    function GetDurationFor(this, mobile)
        var duration := CInt(mobile.GetSkill(THEOLOGY) / 5);
        if (duration < 2)
            duration := 2;
        endif
        return duration;
    endfunction

    function OnBeforeDamage(this, attackInfo)
        var attacker := attackInfo.GetAttacker();
        var defender := attackInfo.GetDefender();
        var who := attacker.GetObjRef();
        var targ := defender.GetObjRef();

        var religiao := attacker.GetReligion();
        var theology_bonus := CInt(attacker.GetSkill(THEOLOGY) / 2);

        if (theology_bonus < 1)
            theology_bonus := 1;
        endif

        // Visual activation
        PlayObjectCenteredEffect(who, 0xDCE5, 1, 32);
        Message("*Invoca poder profano*").SendAbove(who);

        // Religion-specific effects on target
        case (religiao)
            VENERATIO:
                PlayObjectCenteredEffectHuefx(targ, 0x54b3, 10, 20, 1254, 1);
                PlaySoundEffect(targ, 0x1FB);
                SetCooldown(targ, "silence", theology_bonus);

            BOKK_BHARAZ:
                TS_StartTimer(targ, "lentidao", theology_bonus);
                PlayMovingEffectEx(who, targ, 0x36E4, 7, 0, 2598);
                PlaySoundEffect(targ, 0x208);

            HALLEUAD:
                TS_StartTimer(targ, "blinded", 15, 0, who);

            ELUITHORTH:
                SetCooldown(targ, "darkness", theology_bonus);

            KAI:
                TS_StartTimer(targ, "bleedingstrike", 30, 1, who, 2);
        endcase

        // Base forced divine damage (all religions) — rides on the current swing
        attackInfo.GetDamage()
            .IncreaseEnergyRawDamage(theology_bonus);

        DebugMsg(
            who,
            DEBUG_HAB,
            Message($"[SENTENCA_PROFANA] Religiao: {religiao}, Dano: {theology_bonus}", Color(SYSTEM_COLOR::SUCCESS))
        );

        return true;
    endfunction

endclass
