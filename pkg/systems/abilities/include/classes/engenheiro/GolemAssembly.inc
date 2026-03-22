use uo;
use os;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":attributes:attributes";
include "include/sounds";
include ":gumps:yesNo";

class GolemAssembly(CombatAbility)

	function GolemAssembly(this)
		Super(ABILITY_ID::GOLEM_ASSEMBLY);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	// Passive: When piloting a golem, add blunt damage based on METALWORKING
	function OnPassiveBeforeDamage(this, attackInfo)
		var attacker := attackInfo.GetAttacker();
		var who := attacker.GetObjRef();

		if (!attacker.GetProperty("pilotando"))
			return false;
		endif

		var metalworking := CInt(attacker.GetSkill(METALWORKING));
		var damageBonus := CInt(metalworking / 5);

		if (damageBonus <= 0)
			return false;
		endif

		attackInfo.GetDamage()
			.IncreaseBluntRawDamage(damageBonus);

		DebugMsg(
			who,
			DEBUG_HAB,
			Message($"[GOLEM_ASSEMBLY] Dano do Golem (Blunt): +{damageBonus}", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();

		if (mobile.GetProperty("golem_maker"))
			Message("Ter mais de um golem para uso pessoal eh um desperdicio de recursos!").SendSys(who);
			return false;
		endif

		if (who.weapon.objtype != 0x13e3)
			Message("Use um martelo de ferreiro.").SendSys(who);
			return false;
		endif

		var anvil;
		foreach item in ListItemsNearLocation(who.x, who.y, who.z, 1, who.realm)
			if (item.objtype == 0xA102 || item.objtype == 0xA108)
				anvil := item;
				break;
			endif
		endforeach
		if (!anvil)
			Message("Voce precisa estar perto de uma bigorna para criar seu golem.").SendSys(who);
			return false;
		endif

		if (!yesno(who, "Voce utilizara 100 minerios de Mithril e 50 couros curtidos. Deseja continuar?"))
			return false;
		endif

		var has_mithril := 0;
		var has_leather := 0;
		foreach item in EnumerateItemsInContainer(who.backpack)
			if (item.objtype == 0x6311 && !has_mithril)
				if (!SubtractAmount(item, 100))
					Message("Voce precisa de 100 minerios de Mithril.").SendSys(who);
					return false;
				endif
				has_mithril := 1;
			endif
			if (item.objtype == 0xee45 && !has_leather)
				if (!SubtractAmount(item, 50))
					Message("Voce precisa de 50 couros curtidos.").SendSys(who);
					return false;
				endif
				has_leather := 1;
			endif
		endforeach

		if (!has_mithril)
			Message("Voce precisa de 100 minerios de Mithril.").SendSys(who);
			return false;
		endif
		if (!has_leather)
			Message("Voce precisa de 50 couros curtidos.").SendSys(who);
			return false;
		endif

		who.frozen := 1;
		if (anvil.objtype == 0xA108)
			anvil.graphic := 0xA103;
		elseif (anvil.objtype == 0xA102)
			anvil.graphic := 0xA109;
		endif

		var i := 0;
		// while (i < 10)
		// 	PerformAction(who, 12);
		// 	Message("*Construindo*").SendAbove(who);
		// 	PlaySoundEffect(who, SFX_ANVIL);
		// 	PlayStationaryEffect(who.x, who.y, who.z, 0x6DF1, 0, 40, 0, who.realm);
		// 	sleep(4);
		// 	i++;
		// endwhile

		if (anvil.graphic == 0xA103)
			anvil.graphic := 0xA108;
		elseif (anvil.graphic == 0xA109)
			anvil.graphic := 0xA102;
		endif

		who.frozen := 0;
		PlaySoundEffect(who, SFX_SPELL_SUMMON_ELEMENTAL);

		var template := ":epicai:PowerArmor";
		var golem := CreateNpcFromTemplate(template, who.x, who.y, who.z, 0, who.realm);
		golem.frozen := 1;
		SetObjProperty(golem, "golem_maker", who.serial);
		mobile.SetProperty("golem_maker", 1);

		Message("O mecanismo esta pronto para ser operado").SendAbove(golem);
		Message("Sua arte. Esculpida em aco!").SendSys(who);

		DebugMsg(
			who,
			DEBUG_HAB,
			Message("[GOLEM_ASSEMBLY] Golem criado com sucesso!", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

endclass