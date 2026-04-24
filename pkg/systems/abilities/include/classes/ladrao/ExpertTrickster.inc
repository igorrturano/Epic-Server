use uo;

include ":abilities:models/CombatAbility";
include ":timedScripts:timedScripts";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":attributes:attributes";

/**
 * Trapaceiro Experiente (Expert Trickster) - ABILITY_ID::TRAPACEIRO_EXPERIENTE (1603)
 *
 * Active (BEFORE_DAMAGE, SingleUse):
 * - On next hit, steals the defender's weapon to attacker's backpack
 * - Requires passing a Pickpocketing vs Tactics check
 *
 * Passive (via HasAbility in players.src):
 * - Reduces unconscious time on death
 */
class ExpertTrickster(CombatAbility)

    function ExpertTrickster(this)
        Super(ABILITY_ID::TRAPACEIRO_EXPERIENTE);
        this.ActivationMoment := ABILITY_ACTIVATION::BEFORE_DAMAGE;
        this.SingleUse := true;
    endfunction

    function GetDurationFor(this, mobile)
        var duration := CInt(mobile.GetSkill(PICKPOCKETING) / 5);
        if (duration < 2)
            duration := 2;
        endif
        return duration;
    endfunction

    function OnBeforeDamage(this, attackInfo)
        var attacker := attackInfo.GetAttacker();
        var defender := attackInfo.GetDefender();

        var defenderWeapon := defender.GetEquipBySlot(WEAPON);
        if (!defenderWeapon || !defenderWeapon.GetObjRef())
            DebugMsg(
                attacker.GetObjRef(),
                DEBUG_HAB,
                Message("[TRAPACEIRO_EXPERIENTE] Alvo sem arma!", Color(SYSTEM_COLOR::FAIL))
            );
            return false;
        endif

        // Don't steal unarmed combat (fists)
        if (defenderWeapon.GetCategory() == WEAPON_CATEGORY::UNARMED)
            return false;
        endif

        // Pickpocketing check vs defender's Tactics
        var difficulty := CInt(defender.GetSkill(TACTICS));
        if (SkillCheck(attacker.GetObjRef(), PICKPOCKETING, difficulty) <= 0)
            Message("*tenta roubar a arma, mas falha!*").SendAbove(attacker.GetObjRef());

            DebugMsg(
                attacker.GetObjRef(),
                DEBUG_HAB,
                Message("[TRAPACEIRO_EXPERIENTE] Falha no teste de Pickpocketing!", Color(SYSTEM_COLOR::FAIL))
            );
            return false;
        endif

        // Steal weapon to attacker's backpack
        if (MoveItemToContainer(defenderWeapon.GetObjRef(), attacker.GetObjRef().backpack))
            var disarmDuration := CInt(attacker.GetSkill(PICKPOCKETING) / 5);
            if (disarmDuration < 2)
                disarmDuration := 2;
            endif

            TS_StartTimer(defender.GetObjRef(), "desarmar", disarmDuration, 0, attacker.GetObjRef());

            Message("*rouba a arma do oponente!*").SendAbove(attacker.GetObjRef());
            Message("*sua arma foi roubada!*", Color(SYSTEM_COLOR::FAIL)).SendAbove(defender.GetObjRef());

            DebugMsg(
                attacker.GetObjRef(),
                DEBUG_HAB,
                Message($"[TRAPACEIRO_EXPERIENTE] Arma roubada! Desarme: {disarmDuration}s", Color(SYSTEM_COLOR::SUCCESS))
            );

            return true;
        endif

        return false;
    endfunction

endclass
