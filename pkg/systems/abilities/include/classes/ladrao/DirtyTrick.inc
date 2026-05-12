use uo;
use os;

include ":abilities:models/CombatAbility";
include ":message:model/Message";

/**
 * Truque Sujo (Dirty Trick) - ABILITY_ID::TRUQUE_SUJO (1601)
 *
 * Active (ON_USE):
 * - Pick an item from your backpack to "mark".
 * - 300s later the item returns to your backpack from wherever it is.
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

        Message("Escolha um item da sua mochila para marcar.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
        var itm := Target(who);
        if (!itm || !itm.IsA(POLCLASS_ITEM))
            Message("Alvo invalido.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
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

        Message("*marca um item*").SendAbove(who);
        Message("Item marcado. Retornara em 300 segundos.", Color(SYSTEM_COLOR::INFO)).SendSys(who);

        sleep(300);

        if (itm && who.backpack)
            MoveObjectToLocation(itm, 0, 0, 0, who.realm);
            MoveItemToContainer(itm, who.backpack);
            Message("Seu item marcado retornou.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
        endif

        return true;
    endfunction

endclass
