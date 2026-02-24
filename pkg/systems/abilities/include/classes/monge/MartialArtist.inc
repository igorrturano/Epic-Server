use uo;
use os;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":IO:CFGFile";

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

		var monkWeapons := array{ 0x0df0, 0x13f9, 0x0e8a, 0x0e81, 0x1F020, 0x27AD, 0x27AE, 0x27A7 };
		if (!(attacker.GetEquipBySlot(WEAPON).GetObjType() in monkWeapons))
			return false;
		endif

		// Check no armor equipped
		var armorTypes := array{ "ArmaduraPesada", "ArmaduraMedia", "ArmaduraLeve" };
		var itemdesc := CFGFile("*", "itemdesc");
		foreach item in (ListEquippedItems(who))
			var itemCfg := itemdesc.SetElementName(item.objtype);
			if (itemCfg.GetPropertyValue("TipoProtecao") in armorTypes)
				return false;
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
