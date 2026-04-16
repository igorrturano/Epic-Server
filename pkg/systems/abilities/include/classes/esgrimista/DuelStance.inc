use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":attributes:attributes";
include ":timedScripts:timedScripts";
include ":hotbar:custombuff";

/**
 * Postura de Duelo (Duel Stance) - ABILITY_ID::POSTURA_DE_DUELO (1501)
 *
 * Active (ON_USE):
 * - Stamina regen over time (recuperacaostamina timed script)
 *
 * Passive (in HitController::CalculateAbilityHitModifier, based on LAYER_HAND2):
 * - Buckler (0x1B73): Uses TACTICS instead of PARRY (in ParryController)
 * - Empty off-hand: -tactics/10 enemy hit chance (evasion)
 * - Dual Wield: +tactics/10 hit chance
 * - Pistol (0xc5fc): Enables pistol3 usage (gated in fireweapons.src)
 */
class DuelStance(CombatAbility)

    function DuelStance(this)
        Super(ABILITY_ID::POSTURA_DE_DUELO);
    endfunction

    function ExecuteOn(this, mobile)
        var who := mobile.GetObjRef();
        var duration := CInt(mobile.GetSkill(TACTICS) / 5);
        if (duration < 5)
            duration := 5;
        endif

        TS_StartTimer(who, "recuperacaostamina", duration);
        SendAbilityBuff(who, ABILITY_ID::POSTURA_DE_DUELO, duration, "Recuperacao de stamina");
        Message("*postura de duelo*").SendAbove(who);

        DebugMsg(
            who,
            DEBUG_HAB,
            Message($"[POSTURA_DE_DUELO] Recuperacao stamina por {duration}s", Color(SYSTEM_COLOR::SUCCESS))
        );

        return true;
    endfunction

endclass
