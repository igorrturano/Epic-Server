include "Ability";
include ":combat:Model/AttackInfo";
include ":wrappers:Objects/Model/MobileWrapper";

class CombatAbility(Ability)

    var SingleUse := false;
    var IsPool := false;
    // IsDefenderSide is declared on Ability base (default 0). Defender-side
    // combat abilities set this.IsDefenderSide := 1 in their constructor.

    function CombatAbility(this, abilityEnum)
        Super(abilityEnum);
        // Default for CombatAbility - subclasses override in their constructor
        this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
    endfunction

    function IsSingleUse(this)
        return this.SingleUse;
    endfunction

    function IsPoolAbility(this)
        return this.IsPool;
    endfunction

    // Override: Cleanup when ability deactivates (by any reason: expiry, abort, move, complete)
    // @param mobile MobileWrapper - who had the ability
    function OnDeactivate(this, mobile)
        // To be overridden by subclasses
    endfunction

    //===========================================
    // COMBAT MOMENT HOOKS (override as needed)
    // All receive AttackInfo with GetAttacker()/GetDefender()
    //===========================================

    // DEFENDER: Before hit roll
    function OnBeforeHit(this, attackInfo)
        // To be overridden by subclasses
        return false;
    endfunction

    // Defender: Attack missed
    function OnMiss(this, attackInfo)
        // To be overridden by subclasses
        return false;
    endfunction

    // Attacker: Attack Hit
    function OnHit(this, attackInfo)
        // To be overridden by subclasses
        return false;
    endfunction

    // Attacker: After hit, before damage calculation
    function OnBeforeDamage(this, attackInfo)
        // To be overridden by subclasses
        return false;
    endfunction

    // Attacker: After damage applied
    function OnAfterDamage(this, attackInfo)
        // To be overridden by subclasses
        return false;
    endfunction

    // Attacker: Critical hit
    function OnCritical(this, attackInfo)
        // To be overridden by subclasses
        return false;
    endfunction

    // Attacker: Target died
    function OnKill(this, attackInfo)
        // To be overridden by subclasses
        return false;
    endfunction

    // Defender:  parry
    function OnParry(this, attackInfo)
        // To be overridden by subclasses
        return false;
    endfunction

    //===========================================
    // PASSIVE HOOKS (triggered by HAVING ability, not activating)
    //===========================================

    function OnPassiveBeforeDamage(this, attackInfo)
        return false;
    endfunction

    function OnPassiveOnKill(this, attackInfo)
        return false;
    endfunction

    function OnPassiveOnParry(this, attackInfo)
        return false;
    endfunction

    function OnPassiveAfterDamage(this, attackInfo)
        return false;
    endfunction

    function OnPassiveOnMiss(this, attackInfo)
        return false;
    endfunction

    function OnPassiveBeforeHitAttacker(this, attackInfo)
        return false;
    endfunction

    function OnPassiveBeforeHitDefender(this, attackInfo)
        return false;
    endfunction

    function OnPassiveOnEquip(this, equip)
        return false;
    endfunction

    function OnPassiveOnUnequip(this, equip)
        return false;
    endfunction

    //===========================================
    // EXECUTION ROUTER (called by AbilitiesController)
    //===========================================

    function ExecuteOn(this, attackInfo)
        var moment := this.ActivationMoment;

        case (moment)
            ABILITY_ACTIVATION::BEFORE_HIT:
                return this.OnBeforeHit(attackInfo);
            ABILITY_ACTIVATION::ON_MISS:
                return this.OnMiss(attackInfo);
            ABILITY_ACTIVATION::BEFORE_DAMAGE:
                return this.OnBeforeDamage(attackInfo);
            ABILITY_ACTIVATION::ON_HIT:
                return this.OnHit(attackInfo);
            ABILITY_ACTIVATION::ON_CRITICAL:
                return this.OnCritical(attackInfo);
            ABILITY_ACTIVATION::AFTER_DAMAGE:
                return this.OnAfterDamage(attackInfo);
            ABILITY_ACTIVATION::ON_KILL:
                return this.OnKill(attackInfo);
            ABILITY_ACTIVATION::ON_PARRY:
                return this.OnParry(attackInfo);
        endcase

        return false;
    endfunction

endclass