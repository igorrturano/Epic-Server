use uo;
use os;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
;
include "include/client";
include "include/epicConstants";

/**
 * Enhanced Infusion (Infusão Aprimorada) - ABILITY_ID::PROTOCOLO_MESTRE (2504)
 *
 * Active (ON_USE): Creates a Philosopher's Stone (Pedra Filosofal) item
 * that provides death protection, following the Lich phylactery pattern.
 *
 * - Requires hands free (somatic components)
 * - Only one Philosopher's Stone allowed at a time
 * - 3-phase casting animation
 * - Creates stone item (0x0240) in backpack
 * - Death protection via #phylactery cooldown (checked in players.src death script)
 */
class EnhancedInfusion(CombatAbility)

	function EnhancedInfusion(this)
		Super(ABILITY_ID::PROTOCOLO_MESTRE);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function CanActivate(this, mobile)
		var who := mobile.GetObjRef();

		if (!EquipmentCheck(who))
			Message("Suas maos precisam estar livres para gesticular os componentes gestuais necessarios para a magia.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif

		if (GetObjProperty(who, "phylactery"))
			Message("Voce ja possui uma Pedra Filosofal!", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif

		return true;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();

		who.frozen := 1;
		var sounds := array{ 583, 584, SFX_SPELL_WEAKEN };
		var i := 1;
		while (i <= 3)
			PerformAction(who, HUMAN_ANIM::CAST_DIR);
			Message("*Conjurando*").SendAbove(who);
			PlaySoundEffect(who, sounds[i]);
			PlayStationaryEffectHuefx(who.x, who.y, who.z, 0x548F, 8, 40, 0, 0);
			sleep(3);
			i++;
		endwhile

		PlaySoundEffect(who, 0x245);
		sleep(2);
		who.frozen := 0;

		var stone := CreateItemInBackpack(who, 0x0240, 1);
		stone.decayAt := 0;
		SetObjProperty(who, "phylactery", who.serial);

		Message("*cria uma Pedra Filosofal*").SendAbove(who);
		DebugMsg(
			who,
			DEBUG_HAB,
			Message("[PROTOCOLO_MESTRE] Pedra Filosofal criada", Color(SYSTEM_COLOR::SUCCESS))
		);
		return true;
	endfunction

endclass
