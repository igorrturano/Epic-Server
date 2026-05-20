include ":timedScripts:timedScripts";
include ":abilities:models/CombatAbility";

/**
 * Golpe Divino (Divine Smite) - ABILITY_ID::GOLPE_DIVINO (401)
 *
 * Active (BeforeDamage - as ATTACKER, SingleUse):
 * - Adds divine damage (Theology/10) to all damage types
 * - Applies religion-specific effect to target:
 *   VENERATIO: Mana drain + "corpofechado" cooldown
 *   BOKK_BHARAZ: "tombar" + "lentidao" timers
 *   HALLEUAD: "blinded" timer
 *   ELUITHORTH: "darkness" + "blinded", remove nightvision
 *   KAI: Ignore armor
 * - SingleUse: auto-deactivates after first hit (handled by AbilitiesController)
 */
class DivineSmite(CombatAbility)

    function DivineSmite(this)
        Super(ABILITY_ID::GOLPE_DIVINO);
        this.ActivationMoment := ABILITY_ACTIVATION::BEFORE_DAMAGE;
        this.SingleUse := true;
    endfunction

    function GetDurationFor(this, mobile)
        var duration := CInt(mobile.GetSkill(THEOLOGY) / 10);
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

        // Visual effect on caster
        Message("*Canaliza energia divina*").SendEmote(who);
        PlayObjectCenteredEffect(who, 0x376A, 1, 32);

        // Apply religion-specific effects
        case (religiao)
            VENERATIO:
                PlayObjectCenteredEffect(targ, 0xAFED, 7, 16);
                PlaySoundEffect(targ, 0x358);
                var maxMana := defender.GetManaMax();
                var currentMana := defender.GetCurrentMana();
                var manaDrain := CInt((maxMana - currentMana) * 1.2);
                manaDrain := manaDrain + attacker.GetWill();
                defender.ConsumeMana(manaDrain);
                SetCooldown(who, "corpofechado", theology_bonus);

            BOKK_BHARAZ:
                PlayStationaryEffect(targ.x, targ.y, targ.z, 0x6C9B, 0, 10);
                PlaySoundEffect(targ, 0x221);
                TS_StartTimer(targ, "tombar", theology_bonus);
                TS_StartTimer(targ, "lentidao", theology_bonus);

            HALLEUAD:
                PlaySoundEffect(targ, 0x1F8);
                TS_StartTimer(targ, "blinded", theology_bonus, 0, who);
                Message("*cegueira*").SendAbove(targ);

            ELUITHORTH:
                PlayStationaryEffect(targ.x, targ.y, targ.z, 0x3789, 0x0A, 0x1E);
                PlaySoundEffect(targ, 0x231);
                SetCooldown(targ, "nightvision", 0);
                SetCooldown(targ, "darkness", theology_bonus);
                TS_StartTimer(targ, "blinded", theology_bonus, 0, who);

            KAI:
                PlayObjectCenteredEffect(targ, 0x37B9, 0x0A, 0x1E);
                PlaySoundEffect(targ, 0x51D);
                attackInfo.SetIgnoreArmor();
        endcase

        // Add divine damage to all damage types
        attackInfo.GetDamage()
            .IncreaseFireRawDamage(theology_bonus);


        DebugMsg(
            who,
            DEBUG_HAB,
            Message($"[GOLPE_DIVINO] Dano divino: +{theology_bonus}, Religiao: {religiao}", Color(SYSTEM_COLOR::SUCCESS))
        );

        return true;
    endfunction

endclass
