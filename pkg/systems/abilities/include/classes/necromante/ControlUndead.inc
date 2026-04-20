use uo;
use cfgfile;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":attributes:attributes";
include ":timedscripts:timedScripts";
include ":brainai:npcUtil";

/**
 * Controlar Mortos (Control Undead) - ABILITY_ID::CONTROLAR_MORTOS (2003)
 *
 * Active (ON_USE):
 * - Target undead NPC only
 * - Duration: 120 + (1.8 * ARCANA) seconds
 * - Applies "controlarmortovivo" timed script (sets NPC master, changes AI script)
 */
class ControlUndead(CombatAbility)

    function ControlUndead(this)
        Super(ABILITY_ID::CONTROLAR_MORTOS);
    endfunction

    function ExecuteOn(this, mobile)
        var who := mobile.GetObjRef();

        if (who.npctemplate)
            return false;
        endif

        var cfg := ReadConfigFile(":*:npcdesc");

        Message("Escolha o morto para controlar.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
        var targ := Target(who);

        if (!targ)
            return false;
        endif

        if (!targ.npctemplate)
            Message("Este nao e um alvo valido.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
            return false;
        endif

        var npctemplate := NPC_ParseTemplateName(targ.npctemplate);
        if (cfg[npctemplate.template].Environment != "Undead")
            Message("Este nao e um alvo valido.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
            return false;
        endif

        var duration := 120 + 180 * mobile.GetSkill(ARCANA) / 100;
        TS_StartTimer(targ, "controlarmortovivo", CInt(duration), 1, who);

        Message("*controlar mortos*").SendAbove(who);
        DebugMsg(
            who,
            DEBUG_HAB,
            Message($"[CONTROLAR_MORTOS] Duracao: {CInt(duration)}s", Color(SYSTEM_COLOR::SUCCESS))
        );
        return true;
    endfunction

endclass
