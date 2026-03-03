use uo;
use os;

include ":abilities:models/CombatAbility";
include ":fls_core:packets";
include ":fls_magery:basicMagery";
include ":attributes:attributes";
include ":tn:cooldown";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include "include/epicConstants";

/**
 * Elemental Bow (Gwaenedh) - ABILITY_ID::GWAENEDH (2400)
 *
 * Active (OnUse): Conjures a temporary elemental bow with 3-color cycling animation.
 * Duration = arcana/10 * 60 * 3 seconds. Bow model depends on Tirothan ability.
 * Sets "fire_arrow" property on created bow.
 */
class ElementalBow(CombatAbility)

	function ElementalBow(this)
		Super(ABILITY_ID::GWAENEDH);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function CanActivate(this, mobile)
		var who := mobile.GetObjRef();

		if (!EquipmentCheck(who))
			Message("Suas maos precisam estar livres para gesticular os componentes gestuais necessarios para a magia.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif

		if (GetCooldown(who, "arcane_bow"))
			Message("Voce ja possui um arco elemental!", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif

		return true;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();

		who.frozen := 1;
		var color;
		var sound;
		var i := 1;
		while (i <= 3)
			PerformAction(who, HUMAN_ANIM::CAST_DIR);
			PrintTextAbove(who, "*Conjurando*");
			if (i == 1)
				sound := 0x5cb;
				color := COLOR_ELEMENTAL_FIRE;
			elseif (i == 2)
				sound := 0x5c8;
				color := COLOR_ELEMENTAL_ICE;
			elseif (i == 3)
				sound := 0x666;
				color := COLOR_ELEMENTAL_ENERGY;
			endif
			PlaySoundEffect(who, sound);
			PlayStationaryEffectEx(who.x, who.y, who.z, who.realm, 0xB462, 8, 40, color);
			sleep(1);
			i++;
		endwhile
		PlaySoundEffect(who, 0x245);
		sleep(2);
		who.frozen := 0;

		var the_bow := 0xFA6D;
		if (mobile.HasAbility(ABILITY_ID::TIROTHAN))
			the_bow := 0xF997;
		endif

		var arcane_bow := CreateItemInBackpack(who, the_bow, 1);
		EquipItem(who, arcane_bow);
		var duration := CInt(mobile.GetSkill(ARCANA) / 10 * 60 * 3);
		SetCooldown(who, "arcane_bow", duration);
		SetObjProperty(arcane_bow, "fire_arrow", 1);

		DebugMsg(
			who,
			DEBUG_HAB,
			Message($"[GWAENEDH] Arco elemental conjurado por {duration}s", Color(SYSTEM_COLOR::SUCCESS))
		);

		sleep(duration);
		if (arcane_bow)
			DestroyItem(arcane_bow);
			Message("Seu arco se dissipou!", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		endif

		return true;
	endfunction

endclass
