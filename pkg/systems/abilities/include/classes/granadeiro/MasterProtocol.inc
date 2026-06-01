use uo;
use os;
use polsys;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":hotbar:include/cooldown_packet";

/**
 * Protocolo Mestre (Master Protocol) - ABILITY_ID::PROTOCOLO_MESTRE (2504)
 *
 * Active (ON_USE): for 30 seconds the Granadier's explosives gain +30% radius
 * and deal no damage to allies (applied in exploder.src ExplosionEffect). On
 * activation it also resets the cooldowns of Zona de Pressao and Torreta
 * Granadeira so the player can re-stage the battlefield in the burst window.
 */
class MasterProtocol(CombatAbility)

    function MasterProtocol(this)
        Super(ABILITY_ID::PROTOCOLO_MESTRE);
        this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
    endfunction

    function ExecuteOn(this, mobile)
        var who := mobile.GetObjRef();
        if (mobile.IsNPC())
            return 0;
        endif

        SetObjProperty(who, "ProtocoloMestreActive", polcore().systime + 30);

        _ResetAbilityCooldown(mobile, who, ABILITY_ID::ZONA_DE_PRESSAO);
        _ResetAbilityCooldown(mobile, who, ABILITY_ID::TORRETA_GRANADEIRA);

        Message("*ativa o Protocolo Mestre*").SendAbove(who);
        return 1;
    endfunction

endclass

function _ResetAbilityCooldown(mobile, who, abilityId)
    var ab := Ability(abilityId);
    mobile.ClearAbilityCooldown(abilityId);
    SendCooldownPacket(who, ab.GetName(), 0, ab.GetIcon(), COOLDOWN_TYPE_ABILITY);
endfunction
