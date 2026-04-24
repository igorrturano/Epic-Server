use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":attributes:attributes";
include ":timedScripts:timedScripts";
include ":spells:summon";

/**
 * Invocacao do Caos (Chaos Invocation) - ABILITY_ID::INVOCACAO_DO_CAOS (1002)
 *
 * OnUse: Rolls 1d10 and summons a random creature, from a spiderling to an ankheg.
 * The summoned creature fights alongside the caster for a duration based on ARCANA.
 */
class ChaosInvocation(CombatAbility)

	function ChaosInvocation(this)
		Super(ABILITY_ID::INVOCACAO_DO_CAOS);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();
		var arcana := CInt(mobile.GetSkill(ARCANA));

		var template := ":epicai:sheep";
		var dice_roll := RandomDiceRoll("1d10");

		Message($"Rolou um {dice_roll}!", Color(SYSTEM_COLOR::INFO)).SendSys(who);

		case (dice_roll)
			1:  template := ":epicai:spiderling";
			2:  template := ":epicai:giantrat";
			3:  template := ":epicai:gianttoad";
			4:  template := ":epicai:DreadGiantSpider";
			5:  template := ":epicai:HugeRedSpider";
			6:  template := ":epicai:umberhulk";
			7:  template := ":epicai:reptoide3";
			8:  template := ":epicai:SuccubusVermelha";
			9:  template := ":epicai:SkullSpider";
			10: template := ":epicai:ankheg";
		endcase

		// Cast animation
		who.frozen := 1;
		var i := 0;
		while (i < 4)
			PerformAction(who, HUMAN_ANIM::CAST_DIR);
			sleep(1);
			i++;
		endwhile
		who.frozen := 0;

		PlaySoundEffect(who, SFX_SPELL_SUMMON_ELEMENTAL);

		var mount := summon(template, who, who);

		var duracao := 120 + 180 * arcana / 100;
		TS_StartTimer(mount, "summon", duracao);

		if (dice_roll == 9)
			PlayObjectCenteredEffect(mount, 0x6D1D, 10, 20);
			sleep(1);
			Message("Teu chamado ressoou pelos planos, invocador ousado. Estou aqui para servir.").SendAbove(mount);
			duracao := arcana;
		elseif (dice_roll == 1)
			Message("Beeh").SendAbove(mount);
		endif

		SetObjProperty(mount, "anchor", array{ who.x, who.y, who.z + 2 });

		Message("Sua invocação caótica está pronta para servir!", Color(SYSTEM_COLOR::SUCCESS)).SendSys(who);
		return true;
	endfunction

endclass
