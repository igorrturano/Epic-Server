use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":timedScripts:timedScripts";
include ":attributes:attributes";

/**
 * Valdruk - ABILITY_ID::VALDRUK (1802)
 *
 * Active (ON_USE):
 * - Conceals the player in shadows for ARCANA/5 seconds
 *
 * Passive (via HasAbility in spell files):
 * - Enhances illusionist/mage/necromancer spells (duration, range, effects)
 */
class Valdruk(CombatAbility)

    function Valdruk(this)
        Super(ABILITY_ID::VALDRUK);
    endfunction

    function ExecuteOn(this, mobile)
        var who := mobile.GetObjRef();
        var duration := CInt(mobile.GetSkill(ARCANA) / 5);

        PlayObjectCenteredEffectEx(who, 0x54CE, 10, 20, 1420, 1);
        PlaySoundEffect(who, 0x204);

        TS_StartTimer(who, "conceal", duration);

        Message("*desaparece nas sombras*").SendAbove(who);

        DebugMsg(
            who,
            DEBUG_HAB,
            Message($"[VALDRUK] Concealment ativado por {duration}s", Color(SYSTEM_COLOR::SUCCESS))
        );

        return true;
    endfunction

endclass
