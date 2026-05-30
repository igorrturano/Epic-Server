use uo;
use os;

include ":abilities:models/CombatAbility";
include ":message:model/Message";

/**
 * Truque Sujo (Dirty Trick) - ABILITY_ID::TRUQUE_SUJO (1601)
 *
 * Active (ON_USE) — single Target with two modes:
 *   - Target is a Mobile  -> "Plantar Objeto": pick an item from your pack,
 *                            it goes into the target's pack, returns to you in 300s.
 *   - Target is an Item   -> "Marcar Item": the item (must be in your pack)
 *                            returns to your pack from wherever it is in 300s.
 *
 * Passive (via HasAbility in key/use.src):
 * - Can copy keys without tools or skill check
 */
class DirtyTrick(CombatAbility)

    function DirtyTrick(this)
        Super(ABILITY_ID::TRUQUE_SUJO);
    endfunction

    function ExecuteOn(this, mobile)
        if (mobile.IsNPC())
            return false;
        endif

        var who := mobile.GetObjRef();

        Message("Escolha um alvo: um mobile para plantar um item, ou um item da sua mochila para marcar.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
        var tgt := Target(who);
        if (!tgt)
            Message("Cancelado.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
            return false;
        endif

        var itm;

        if (tgt.IsA(POLCLASS_MOBILE))
            // Plantar Objeto
            if (Distance(who, tgt) > 1)
                Message("Voce esta muito longe do alvo.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
                return false;
            endif

            Message("Escolha um item da sua mochila para plantar.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
            itm := Target(who);
            if (!itm || !itm.IsA(POLCLASS_ITEM))
                Message("Item invalido.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
                return false;
            endif

            var aux := itm;
            while (aux.container)
                aux := aux.container;
            endwhile
            if (aux != who)
                Message("O item deve estar na sua mochila.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
                return false;
            endif

            MoveObjectToLocation(itm, 0, 0, 0, who.realm);
            MoveItemToContainer(itm, tgt.backpack);
            Message("*planta um item*").SendAbove(who);
            Message("Item plantado. Retornara em 300 segundos.", Color(SYSTEM_COLOR::INFO)).SendSys(who);

        elseif (tgt.IsA(POLCLASS_ITEM))
            // Marcar Item
            itm := tgt;

            var aux := itm;
            while (aux.container)
                aux := aux.container;
            endwhile
            if (aux != who)
                Message("O item deve estar na sua mochila.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
                return false;
            endif

            Message("*marca um item*").SendAbove(who);
            Message("Item marcado. Retornara em 300 segundos.", Color(SYSTEM_COLOR::INFO)).SendSys(who);

        else
            Message("Alvo invalido.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
            return false;
        endif

        sleep(300);

        if (itm && who.backpack)
            MoveObjectToLocation(itm, 0, 0, 0, who.realm);
            MoveItemToContainer(itm, who.backpack);
            Message("Seu item marcado retornou.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
        endif

        return true;
    endfunction

endclass
