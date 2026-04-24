use uo;
use os;

include ":abilities:models/CombatAbility";
include ":fls_core:packets";
include "include/epicMageryUtils";
include ":attributes:attributes";
include ":tn:cooldown";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include "include/epicConstants";

/**
 * Arcane Bow (Arco Elemental / Gwaenedh) - ABILITY_ID::ARCANE_BOW (2400)
 *
 * Active (OnUse): Conjures an elemental bow.
 * - Without ARCANE_MASTERY: 5s ritual (frozen, 3-color animation), bow expires after duration.
 * - With ARCANE_MASTERY: instant conjuration, bow is permanent (no expiry).
 * Duration = arcana/10 * 60 * 3 seconds. Enhanced bow model with ARCANE_MASTERY.
 * Sets "fire_arrow" property on created bow.
 * Requires religion Lore to use.
 */
class ElementalBow(CombatAbility)

	function ElementalBow(this)
		Super(ABILITY_ID::ARCANE_BOW);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function CanActivate(this, mobile)
		var who := mobile.GetObjRef();

		if (mobile.GetReligion() != LORE)
			Message("Voce precisa ser devoto de Lore para conjurar o arco elemental.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif

		if (!EquipmentCheck(who))
			Message("Suas maos precisam estar livres para conjurar o arco.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
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
		var instant := mobile.HasAbility(ABILITY_ID::ARCANE_MASTERY);

		if (!instant)
			who.frozen := 1;
			var color;
			var sound;
			var i := 1;
			while (i <= 3)
				PerformAction(who, HUMAN_ANIM::CAST_DIR);
				Message("*Conjurando*").SendAbove(who);
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
		else
			PlaySoundEffect(who, 0x245);
			PlayStationaryEffectEx(who.x, who.y, who.z, who.realm, 0xB462, 8, 40, COLOR_ELEMENTAL_ENERGY);
		endif

		var the_bow := instant ? 0xF997 : 0xFA6D;
		var arcane_bow := CreateItemInBackpack(who, the_bow, 1);
		EquipItem(who, arcane_bow);
		SetObjProperty(arcane_bow, "fire_arrow", 1);

		if (instant)
			// Permanent bow — cooldown set high so CanActivate blocks re-conjuration
			SetCooldown(who, "arcane_bow", 99999);
			DebugMsg(
				who,
				DEBUG_HAB,
				Message("[ARCANE_BOW] Arco elemental permanente conjurado.", Color(SYSTEM_COLOR::SUCCESS))
			);
		else
			var duration := CInt(mobile.GetSkill(ARCANA) / 10 * 60 * 3);
			SetCooldown(who, "arcane_bow", duration);
			DebugMsg(
				who,
				DEBUG_HAB,
				Message($"[ARCANE_BOW] Arco elemental conjurado por {duration}s", Color(SYSTEM_COLOR::SUCCESS))
			);
			sleep(duration);
			if (arcane_bow)
				DestroyItem(arcane_bow);
				Message("Seu arco se dissipou!", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			endif
		endif

		return true;
	endfunction

endclass
