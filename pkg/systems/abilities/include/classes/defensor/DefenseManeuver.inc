use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":tn:cooldown";
include ":attributes:attributes";

/**
 * Manobra de Defesa (Defense Maneuver) - ABILITY_ID::MANOBRA_DE_DEFESA (301)
 *
 * OnUse (posture-based):
 * - Offensive: Guarantees next parry (#tempparrybonus = 100, resets parry delay)
 * - Defensive: Parry bonus for 20 seconds ("defesapersistente" cooldown)
 *
 * Inline effects:
 *   - ParryController: reads #tempparrybonus for guaranteed parry
 *   - HitController: checks "defesapersistente" for hit_chance reduction
 */
class DefenseManeuver(CombatAbility)

    function DefenseManeuver(this)
        Super(ABILITY_ID::MANOBRA_DE_DEFESA);
        this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
    endfunction

    function CanActivate(this, mobile)
        var postura := mobile.GetObjRef().GetProp("Postura");
        if (!postura || (postura != "Ofensiva" && postura != "Defensiva"))
            Message("Defina sua postura primeiro (.postura ofensiva ou .postura defensiva).", Color(SYSTEM_COLOR::FAIL)).SendSys(mobile.GetObjRef());
            return false;
        endif
        return true;
    endfunction

    function ExecuteOn(this, mobile)
        var who := mobile.GetObjRef();
        var postura := who.GetProp("Postura");

        if (postura == "Ofensiva")
            mobile.SetProperty("#tempparrybonus", 100);
            SetCooldown(who, "parrydelay", 0);
            Message("Voce defendera o proximo golpe.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
            DebugMsg(who, DEBUG_HAB, Message("[MANOBRA_DE_DEFESA] Parry garantido!", Color(SYSTEM_COLOR::SUCCESS)));
        elseif (postura == "Defensiva")
            SetCooldown(who, "defesapersistente", 20);
            Message("Firme, escudeiro! Sua defesa persistira.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
            DebugMsg(who, DEBUG_HAB, Message("[MANOBRA_DE_DEFESA] Defesa persistente 20s", Color(SYSTEM_COLOR::SUCCESS)));
        endif

        return true;
    endfunction

endclass