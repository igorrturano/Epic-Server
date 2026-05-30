include ":abilities:models/CombatAbility";

/**
 * Consagracao Divina (Divine Consecration) - ABILITY_ID::CONSAGRACAO_DIVINA (400)
 *
 * Passive (BeforeDamage - as ATTACKER):
 * - When attacking Undead NPCs, adds Theology/20 bonus divine damage
 *   to all damage types (blunt, pierce, slash)
 */
class DivineConsecration(CombatAbility)

    function DivineConsecration(this)
        Super(ABILITY_ID::CONSAGRACAO_DIVINA);
    endfunction

    function OnPassiveBeforeDamage(this, attackInfo)
        var defender := attackInfo.GetDefender();

        if (!defender.IsNPC())
            return false;
        endif

        if (Lower(defender.Environment) != "undead")
            return false;
        endif

        var attacker := attackInfo.GetAttacker();
        var theology := attacker.GetSkill(THEOLOGY);
        var bonusDamage := CInt(theology / 20);

        if (bonusDamage > 0)
            attackInfo.GetDamage()
                .IncreaseBluntRawDamage(bonusDamage)
                .IncreasePiercingRawDamage(bonusDamage)
                .IncreaseSlashRawDamage(bonusDamage);

            DebugMsg(
                attacker.GetObjRef(),
                DEBUG_HAB,
                Message($"[CONSAGRACAO] Dano vs Undead: +{bonusDamage}", Color(SYSTEM_COLOR::SUCCESS))
            );
        endif

        return true;
    endfunction

endclass
