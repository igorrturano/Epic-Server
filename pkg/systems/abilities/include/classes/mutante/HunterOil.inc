use uo;

include ":abilities:models/CombatAbility";
include ":tn:tngumps";
include ":message:model/Message";
include ":debug:Controller/DebugController";

/**
 * Hunter Oil (Óleo do Caçador) - ABILITY_ID::OLEO_DO_CACADOR (901)
 *
 * Active (ON_USE): Toggle. Shows gump to select oil type
 * (Dragon/EvilHuman/GiantAnimal/Giants/Goblin/Undead/Underdark).
 * Sets "Óleo" property. Second use removes it.
 *
 * Passive effects (checked inline elsewhere):
 * - HitController:284 — SCIENCE/5 hit chance bonus (already V3)
 * - DamageController_legacy:558 — SCIENCE/20 damage vs matching NPC category
 */
class HunterOil(CombatAbility)

	function HunterOil(this)
		Super(ABILITY_ID::OLEO_DO_CACADOR);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();
		var oleo := GetObjProperty(who, "Óleo");

		if (oleo == error)
			var opts := array{ "Dragon", "EvilHuman", "GiantAnimal", "Giants", "Goblin", "Undead", "Underdark" };
			var inimigo;
			if (mobile.IsNPC())
				inimigo := opts[RandomInt(opts.size()) + 1];
			else
				inimigo := RadioGump(who, 200, 260, "Escolha o Óleo", opts);
				if (!inimigo)
					return false;
				endif
			endif

			SetObjProperty(who, "Óleo", CStr(inimigo));
			Message($"Voce escolheu Oleo de {inimigo}", Color(SYSTEM_COLOR::INFO)).SendSys(who);
			DebugMsg(
				who,
				DEBUG_HAB,
				Message($"[OLEO_DO_CACADOR] Oleo: {inimigo}", Color(SYSTEM_COLOR::SUCCESS))
			);
		else
			EraseObjProperty(who, "Óleo");
			Message("Voce removeu o Oleo.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
		endif

		return true;
	endfunction

endclass
