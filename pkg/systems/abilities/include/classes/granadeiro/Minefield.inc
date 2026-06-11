use uo;
use os;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":alchemy:mines";

/**
 * Campo Minado (Minefield) - ABILITY_ID::CAMPO_MINADO (2502)
 *
 * Active (ON_USE) - Detonacao Remota: instantly detonate ALL of the player's
 * planted mines, anywhere, with a 250ms stagger. CanActivate gates on having at
 * least one mine so an empty press doesn't burn the cooldown.
 *
 * Passives (checked inline via HasAbility(CAMPO_MINADO), not here):
 * - Mine cap = Science/25 [explosionPotion.src Minaterrestre]
 * - Chain reaction [exploder.src ExplosionEffect]
 * - Self-explosion resistance [exploder.src]
 * - Doubles bomb production / potion specialist bonus [alchemybook.src, alchemy.inc]
 */
class Minefield(CombatAbility)

    function Minefield(this)
        Super(ABILITY_ID::CAMPO_MINADO);
        this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
    endfunction

    function CanActivate(this, mobile)
        var who := mobile.GetObjRef();
        if (CountOwnedMines(who.serial) < 1)
            Message("Voce nao tem minas plantadas.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
            return 0;
        endif
        return 1;
    endfunction

    function ExecuteOn(this, mobile)
        var who := mobile.GetObjRef();
        if (mobile.IsNPC())
            return 0;
        endif

        Message("*aperta o detonador*").SendAbove(who);
        var mines := EnumOwnedMines(who.serial);
        foreach mine in (mines)
            if (mine && !GetObjProperty(mine, "running"))
                SetObjProperty(mine, "running", 1);
                DetonateMine(mine);
                sleepms(250);
            endif
        endforeach
        return 1;
    endfunction

endclass
