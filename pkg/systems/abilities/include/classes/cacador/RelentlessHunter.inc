include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";

/**
 * Caçador Implacável (Relentless Hunter) - ABILITY_ID::CACADOR_IMPLACAVEL (104)
 *
 * Active (BeforeDamage): Ignores all armor on attacks for Nature/5 seconds.
 * Passive effects (handled inline elsewhere):
 *   - intrinsicMods.src: +2 Fortitude bonus
 *   - attributes.inc: +25 base HP
 *   - habilidades.inc: +20 hitsmod on purchase
 */
class RelentlessHunter(CombatAbility)

	function RelentlessHunter(this)
		Super(ABILITY_ID::CACADOR_IMPLACAVEL);
		this.ActivationMoment := ABILITY_ACTIVATION::BEFORE_DAMAGE;
	endfunction

	function GetDurationFor(this, mobile)
		var duration := CInt(mobile.GetSkill(NATURE) / 5);
		if (duration < 1)
			duration := 1;
		endif
		return duration;
	endfunction

	function OnBeforeDamage(this, attackInfo)
		attackInfo.SetIgnoreArmor(true);

		Message("*caça implacavelmente sua presa, ignorando suas defesas!*")
			.SendAbove(attackInfo.GetAttacker().GetObjRef());

		DebugMsg(
			attackInfo.GetAttacker().GetObjRef(),
			DEBUG_HAB,
			Message("[CACADOR_IMPLACAVEL] Ativo - ignorando defesas!", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

endclass