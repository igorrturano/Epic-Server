use uo;
use os;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";


/**
 * Budōka / Artista Marcial (Martial Artist) - ABILITY_ID::MARTIAL_ARTIST (700)
 *
 * Active (OnUse): Multi-attack - Tactics/30 extra attacks (min 1, max 5).
 *
 * Passive (AttackerBeforeDamage): Tactics/20 bonus damage, but only with
 * monk weapons and without armor.
 *
 * Passive dodge (HitController, unchanged): -Tactics/5 hit chance when WRESTLING.
 */
class MartialArtist(CombatAbility)

	function MartialArtist(this)
		Super(ABILITY_ID::MARTIAL_ARTIST);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function CanActivate(this, mobile)
		var who := mobile.GetObjRef();
		if (!who.opponent)
			Message("Voce precisa estar em combate.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif
		return true;
	endfunction

	// Active: Multi-attack sequence (like RageCut but Tactics/30)
	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();

		var extraAttacks := CInt(mobile.GetSkill(TACTICS) / 30);
		if (extraAttacks < 1)
			extraAttacks := 1;
		endif
		if (extraAttacks > 5)
			extraAttacks := 5;
		endif

		Message($"*desfere {extraAttacks} golpes marciais!*").SendAbove(who);

		for i := 1 to extraAttacks
			who.attack_once();
			sleepms(500);
		endfor

		DebugMsg(
			who,
			DEBUG_HAB,
			Message($"[MARTIAL_ARTIST] Executou {extraAttacks} ataques!", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

	// Passive: Damage bonus with monk weapons and no armor
	function OnPassiveBeforeDamage(this, attackInfo)
		var attacker := attackInfo.GetAttacker();
		var who := attacker.GetObjRef();

		// Weapon Attribute == Wrestling
		if ((attacker.GetEquipBySlot(WEAPON).GetAttribute() != WRESTLING))
			return false;
		endif

		// Check no armor equipped
		var itemdesc := ReadConfigFile(":*:itemdesc");
		foreach item in (ListEquippedItems(who))
			if (item.isa(POLCLASS_ARMOR))
				var tipoProtecao := CStr(itemdesc[item.objtype].TipoProtecao);
				if (tipoProtecao["Armadura"])
					return false;
				endif
			endif
		endforeach

		var damageBonus := CInt(attacker.GetSkill(TACTICS) / 20);
		if (damageBonus <= 0)
			return false;
		endif

		attackInfo.GetDamage()
			.IncreaseSlashRawDamage(damageBonus)
			.IncreasePiercingRawDamage(damageBonus)
			.IncreaseBluntRawDamage(damageBonus);

		DebugMsg(
			who,
			DEBUG_HAB,
			Message($"[MARTIAL_ARTIST] Bonus de dano marcial: +{damageBonus}", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

endclass
