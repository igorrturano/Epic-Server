use uo;
use os;

include ":abilities:models/CombatAbility";
include ":attributes:attributes";
include ":tn:cooldown";
include ":fls_core:packets";
include ":timedScripts:timedScripts";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include "include/epicConstants";
include "include/epicNPCUtils";

/**
 * Channeled Shot (Disparo Canalizado) - ABILITY_ID::DISPARO_CANALIZADO (2402)
 *
 * Active (OnUse): Requires active arcane_bow. Fires a channeled elemental attack
 * based on arrow type:
 * - Fire → runs fireball spell
 * - Cold → freeze target (doubled damage if already frozen)
 * - Energy → teleport to target
 */
class ChanneledShot(CombatAbility)

	function ChanneledShot(this)
		Super(ABILITY_ID::DISPARO_CANALIZADO);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function CanActivate(this, mobile)
		var who := mobile.GetObjRef();

		if (!GetCooldown(who, "arcane_bow"))
			Message("Voce precisa estar com seu arco elemental conjurado!", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif

		return true;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();
		var weapon := mobile.GetEquipBySlot(WEAPON);

		if (!weapon)
			return false;
		endif

		var weaponRef := weapon.GetObjRef();
		PrintTextAbove(who, "*Disparo Canalizando*");

		var isFireArrow := GetObjProperty(weaponRef, "fire_arrow");
		var isColdArrow := GetObjProperty(weaponRef, "cold_arrow");
		var isEnergyArrow := GetObjProperty(weaponRef, "energy_arrow");
		var mlore := CInt(mobile.GetSkill(ARCANA) / 5);

		var ammo_color := 0;
		var sound;
		if (isFireArrow)
			ammo_color := COLOR_ELEMENTAL_FIRE;
			sound := 0x5cb;
		elseif (isEnergyArrow)
			ammo_color := COLOR_ELEMENTAL_ENERGY;
			sound := 0x666;
		elseif (isColdArrow)
			ammo_color := COLOR_ELEMENTAL_ICE;
			sound := 0x5c8;
		endif

		if (isFireArrow || isColdArrow)
			var targ := Target(who);
			if (who.npctemplate)
				targ := GetRandomNearbyHostile(who);
			endif
			if (!targ)
				Message("Voce precisa selecionar um alvo.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
				return false;
			endif

			if (!CheckLineOfSight(who, targ))
				Message("Voce nao tem linha de visao para o alvo.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
				return false;
			endif

			if (isColdArrow)
				if (targ.frozen)
					mlore *= 2;
					targ.frozen := 0;
				else
					TS_StartTimer(targ, "congelamento", CInt(mlore / 2));
				endif
				start_script(":combat:weaponHitScripts/IceHit", array{ who, targ, 0, 0, 0, mlore });
			else
				Run_Script(":spells:spell_logic/mage/fireball", { who, targ });
			endif
			PlaySoundEffect(who, sound);
			PlayMovingEffectXYZHued(who, targ, 0xf42, 10, 10, 0, ammo_color);

		elseif (isEnergyArrow)
			Run_Script(":spells:spell_logic/mage/teleport", { who });
		endif

		DebugMsg(
			who,
			DEBUG_HAB,
			Message("[DISPARO_CANALIZADO] Disparo canalizado executado", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

endclass
