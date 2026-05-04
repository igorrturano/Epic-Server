use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":attributes:attributes";

/**
 * Sacrificio da Carne (Flesh Sacrifice) - ABILITY_ID::SACRIFICIO_DA_CARNE (2000)
 *
 * Active (ON_USE):
 * - Self-bleed via MakeBleed()
 * - Restore mana: ARCANA * 2
 * - Self-damage: missing HP amount
 *
 * Passive (via HasAbility in 8 necromancer spell files):
 * - When bloodied (low HP), spell effects are 1.5x stronger
 */
class FleshSacrifice(CombatAbility)

    function FleshSacrifice(this)
        Super(ABILITY_ID::SACRIFICIO_DA_CARNE);
    endfunction

    function ExecuteOn(this, mobile)
        var who := mobile.GetObjRef();
        MakeBleed(who);

        var mana := AP_GetVital(who, MANA);
        var bonus := CInt(mobile.GetSkill(ARCANA) * 2);
        AP_SetVital(who, MANA, mana + bonus);

        var dmg := AP_GetVital(who, HITS) - GetHP(who);
        DamageFLS(who, dmg);

        RecalcVitals(who);

        Message("*sacrificio da carne*").SendAbove(who);
        DebugMsg(
            who,
            DEBUG_HAB,
            Message($"[SACRIFICIO_DA_CARNE] Mana +{bonus}", Color(SYSTEM_COLOR::SUCCESS))
        );
        return true;
    endfunction

endclass
