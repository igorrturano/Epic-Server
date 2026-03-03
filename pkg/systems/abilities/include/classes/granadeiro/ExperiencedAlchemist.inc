use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";

/**
 * Experienced Alchemist (Alquimista Experiente) - ABILITY_ID::ALQUIMISTA_EXPERIENTE (2500)
 *
 * Active (OnUse): Sets "extratoinstantaneo" property so next potion skips full process.
 *
 * Passive (no hook, checked inline via HasAbility):
 * - Reduces reagent costs by 50% [alchemybook.src, dyebook.src]
 */
class ExperiencedAlchemist(CombatAbility)

	function ExperiencedAlchemist(this)
		Super(ABILITY_ID::ALQUIMISTA_EXPERIENTE);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();

		SetObjProperty(who, "extratoinstantaneo", 1);
		Message("Faca um extrato.", Color(SYSTEM_COLOR::INFO)).SendSys(who);

		DebugMsg(
			who,
			DEBUG_HAB,
			Message("[ALQUIMISTA_EXPERIENTE] Extrato instantaneo ativado", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

endclass
