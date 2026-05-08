use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":attributes:attributes";

/**
 * Truque Sujo (Dirty Trick) - ABILITY_ID::TRUQUE_SUJO (1601)
 *
 * Active (ON_USE):
 * - Select an item from your backpack, then select a target
 * - Plants the item in the target's backpack via Pickpocketing check
 *
 * Passive (via HasAbility in key/use.src):
 * - Can copy keys without tools or skill check
 */
class DirtyTrick(CombatAbility)

    function DirtyTrick(this)
        Super(ABILITY_ID::TRUQUE_SUJO);
    endfunction

    function ExecuteOn(this, mobile)
        // Roleplay/Pickpocket flavor — two Target() calls, no NPC use case.
        if (mobile.IsNPC())
            return false;
        endif

        var who := mobile.GetObjRef();

        Message("Escolha um item da sua mochila para por na mochila do alvo.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
        var itm := Target(who);
        if (!itm)
            Message("Cancelado.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
            return false;
        endif

        // Validate item is in player's own backpack
        var aux := itm;
        while (aux.container)
            aux := aux.container;
        endwhile
        if (aux != who)
            Message("O item deve estar na sua mochila.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
            return false;
        endif

        Message("Escolha o alvo que vai receber seu item.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
        var targ := Target(who);
        if (!targ)
            Message("Cancelado.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
            return false;
        endif

        if (!targ.IsA(POLCLASS_MOBILE))
            Message("Alvo invalido.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
            return false;
        endif

        if (Distance(who, targ) > 1)
            Message("Voce esta muito longe do alvo.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
            return false;
        endif

        if (SkillCheck(who, PICKPOCKETING, -1) > 0)
            MoveObjectToLocation(itm, 0, 0, 0, who.realm);
            MoveItemToContainer(itm, targ.backpack);

            Message("*planta um item*").SendAbove(who);
            Message("Voce guardou o item na mochila do alvo.", Color(SYSTEM_COLOR::INFO)).SendSys(who);

            DebugMsg(
                who,
                DEBUG_HAB,
                Message("[TRUQUE_SUJO] Item plantado com sucesso!", Color(SYSTEM_COLOR::SUCCESS))
            );
        else
            Message("Voce falhou ao por item na mochila do alvo.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
        endif

        return true;
    endfunction

endclass
