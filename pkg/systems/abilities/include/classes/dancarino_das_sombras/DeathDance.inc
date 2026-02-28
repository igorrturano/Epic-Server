use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":tn:cooldown";
include ":attributes:attributes";

/**
 * Danca da Morte (Death Dance) - ABILITY_ID::DANCA_DA_MORTE (1803)
 *
 * Active (ON_USE):
 * - Sets "deathdance" cooldown for ARCANA/5 seconds
 *
 * Passive (AFTER_DAMAGE):
 * - While cooldown active, each hit teleports attacker behind the defender
 * - combatHook.src:300 already has facing bypass for "deathdance" cooldown
 * - moveToBehind() from epicCombatUtils.inc (transitive include via combatHook.src)
 */
class DeathDance(CombatAbility)

    function DeathDance(this)
        Super(ABILITY_ID::DANCA_DA_MORTE);
        this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
    endfunction

    function ExecuteOn(this, mobile)
        var who := mobile.GetObjRef();
        var duration := CInt(mobile.GetSkill(ARCANA) / 5);

        SetCooldown(who, "deathdance", duration);

        PlayObjectCenteredEffect(who, 0x5433, 7, 16);
        PlaySoundEffect(who, 0x526);

        Message("*danca das sombras*").SendAbove(who);

        DebugMsg(
            who,
            DEBUG_HAB,
            Message($"[DANCA_DA_MORTE] Ativada por {duration}s", Color(SYSTEM_COLOR::SUCCESS))
        );

        return true;
    endfunction

    function OnPassiveAfterDamage(this, attackInfo)
        var attacker := attackInfo.GetAttacker();
        var defender := attackInfo.GetDefender();

        if (!GetCooldown(attacker.GetObjRef(), "deathdance"))
            return false;
        endif

        moveToBehind(attacker.GetObjRef(), defender.GetObjRef());

        DebugMsg(
            attacker.GetObjRef(),
            DEBUG_HAB,
            Message("[DANCA_DA_MORTE] Moveu para as costas do oponente", Color(SYSTEM_COLOR::SUCCESS))
        );

        return true;
    endfunction

endclass
