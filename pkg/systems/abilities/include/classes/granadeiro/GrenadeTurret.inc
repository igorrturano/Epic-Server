use uo;
use os;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":attributes:attributes";

/**
 * Torreta Granadeira (Grenade Turret) - ABILITY_ID::TORRETA_GRANADEIRA (2503)
 *
 * Active (ON_USE): deploy a stationary turret item (0x9F93) at the caster's feet
 * with HP = Science*2 (via the destroyable system, so players can attack it). It
 * lobs a basic Explosivo at the nearest hostile every 3s for 30s. Only one per
 * Granadier (gated in CanActivate so a duplicate press burns nothing). The
 * behavior loop lives in the standalone :alchemy:turretBrain script.
 */
class GrenadeTurret(CombatAbility)

    function GrenadeTurret(this)
        Super(ABILITY_ID::TORRETA_GRANADEIRA);
        this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
    endfunction

    function CanActivate(this, mobile)
        var who := mobile.GetObjRef();
        var existing := GetObjProperty(who, "activeTurret");
        if (existing && SystemFindObjectBySerial(existing))
            Message("Voce ja tem uma torreta ativa.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
            return 0;
        endif
        return 1;
    endfunction

    function ExecuteOn(this, mobile)
        var who := mobile.GetObjRef();
        if (mobile.IsNPC())
            return 0;
        endif

        var science := CInt(mobile.GetSkill(SCIENCE));

        var turret := CreateItemAtLocation(who.x, who.y, who.z, 0x9F93, 1, who.realm);
        if (!turret)
            Message("Nao foi possivel montar a torreta aqui.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
            return 0;
        endif
        turret.movable := 0;
        turret.name := "Torreta Granadeira";
        SetObjProperty(turret, "CreatedBy", who.serial);
        SetObjProperty(turret, "owner", who.serial);
        SetObjProperty(who, "activeTurret", turret.serial);

        // HP (PROPNAME_DURABILIDADE) + no-respawn flag are set in turretBrain,
        // which already includes epicPropNames (keeps those consts out of the
        // AbilityExecutorSabio data segment).
        start_script(":alchemy:turretBrain", { turret, who, science });
        Message("*monta uma torreta granadeira*").SendAbove(who);
        return 1;
    endfunction

endclass
