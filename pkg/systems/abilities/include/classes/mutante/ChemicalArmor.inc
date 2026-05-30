use uo;

include ":abilities:models/CombatAbility";
include ":attributes:attributes";

/**
 * Blindagem Quimica (Chemical Armor) - ABILITY_ID::BLINDAGEM_QUIMICA (901)
 *
 * Passive, defender-side. Two effects:
 *   - Vicio cap 100 -> 150: GetViciopotMaximumValue hook in
 *     :attributes:hooks/vitalInit (gated on HasAbility).
 *   - Last stand: while at or below 10% HP, incoming damage is halved. Runs at
 *     BEFORE_DAMAGE (pre-resist); halving pre-resist is identical to halving the
 *     final value since resists are multiplicative.
 */
const BLINDAGEM_HP_THRESHOLD := 10;   // percent of max HP

class ChemicalArmor(CombatAbility)

	function ChemicalArmor(this)
		Super(ABILITY_ID::BLINDAGEM_QUIMICA);
		this.IsDefenderSide := 1;
	endfunction

	function OnPassiveBeforeDamage(this, attackInfo)
		var who := attackInfo.GetDefender().GetObjRef();
		var maxHp := AP_GetVitalMaximumValue(who, "hits");
		if (maxHp <= 0 || AP_GetVital(who, "hits") * 100 > maxHp * BLINDAGEM_HP_THRESHOLD)
			return false;
		endif

		attackInfo.GetDamage()
			.DecreaseSlashPercentDamage(50)
			.DecreasePiercingPercentDamage(50)
			.DecreaseBluntPercentDamage(50)
			.DecreaseFirePercentDamage(50)
			.DecreaseColdPercentDamage(50)
			.DecreasePoisonPercentDamage(50)
			.DecreaseEnergyPercentDamage(50);
		return true;
	endfunction

endclass
