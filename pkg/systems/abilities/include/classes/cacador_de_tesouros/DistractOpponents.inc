use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":combat:Controller/FXController";
include ":combat:Model/AttackInfo";
include ":timedScripts:timedScripts";
include ":wrappers:Objects/Model/MobileWrapper";

/**
 * Distrair Oponentes (Distract Opponents) - ABILITY_ID::DISTRAIR_OPONENTES (1101)
 *
 * OnUse: Paralyzes all nearby non-party enemies for 5 seconds with attack animations.
 */
class DistractOpponents(CombatAbility)

	function DistractOpponents(this)
		Super(ABILITY_ID::DISTRAIR_OPONENTES);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();

		Message("*distraindo os inimigos*").SendAbove(who);

		var nearby := mobile.GetNearbyMobiles(1);
		foreach mob in nearby
			if (mob != who && (mob.npctemplate || !(mob in who.party.members)))
				PlayAttackFX(AttackInfo(mobile, MobileWrapper(mob)));
				sleepms(250);
				who.facing := who.facing + 1;
				TS_StartTimer(mob, "paralysis", 5);
			endif
		endforeach

		return true;
	endfunction

endclass
