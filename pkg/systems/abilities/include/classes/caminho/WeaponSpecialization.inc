use uo;

include ":abilities:models/CombatAbility";
include ":debug:Controller/DebugController";
include ":message:model/Message";

/**
 * Weapon Specialization (Pool Ability)
 * Each purchase grants +1 hit and +1 damage with the matching weapon category.
 */
class WeaponSpecBase(CombatAbility)

	var TargetCategory := uninit;

	function WeaponSpecBase(this, abilityEnum)
		Super(abilityEnum);
		this.IsPool := true;
	endfunction

	function _MatchesWeapon(this, weapon)
		if (!weapon || weapon is @Exception)
			return false;
		endif
		return weapon.GetCategory() == this.TargetCategory;
	endfunction

	function OnPassiveBeforeHitAttacker(this, attackInfo)
		var attacker := attackInfo.GetAttacker();
		var weapon := attacker.GetEquipBySlot(WEAPON);
		if (!this._MatchesWeapon(weapon))
			return false;
		endif

		var count := attacker.GetPoolAbilityCount(this.GetId());
		if (count <= 0)
			return false;
		endif

		attackInfo.SetHitChanceMod(CInt(attackInfo.GetHitChanceMod()) + count);

		DebugMsg(attacker.GetObjRef(), DEBUG_HAB,
			Message($"[WEAPON_SPEC] +{count} acerto", Color(SYSTEM_COLOR::SUCCESS)));
		return true;
	endfunction

	function OnPassiveBeforeDamage(this, attackInfo)
		var attacker := attackInfo.GetAttacker();
		var weapon := attacker.GetEquipBySlot(WEAPON);
		if (!this._MatchesWeapon(weapon))
			return false;
		endif

		var count := attacker.GetPoolAbilityCount(this.GetId());
		if (count <= 0)
			return false;
		endif

		attackInfo.GetDamage()
			.IncreaseSlashBaseDamage(count)
			.IncreasePiercingBaseDamage(count)
			.IncreaseBluntBaseDamage(count);

		DebugMsg(attacker.GetObjRef(), DEBUG_HAB,
			Message($"[WEAPON_SPEC] +{count} dano", Color(SYSTEM_COLOR::SUCCESS)));
		return true;
	endfunction

endclass

// === Weapon Specialization Subclasses ===

class WeaponSpecDagger(WeaponSpecBase)
	function WeaponSpecDagger(this)
		Super(ABILITY_ID::WEAPON_SPEC_DAGGER);
		this.TargetCategory := WEAPON_CATEGORY::DAGGER;
	endfunction
endclass

class WeaponSpecSword(WeaponSpecBase)
	function WeaponSpecSword(this)
		Super(ABILITY_ID::WEAPON_SPEC_SWORD);
		this.TargetCategory := WEAPON_CATEGORY::SWORD;
	endfunction
endclass

class WeaponSpecGreatsword(WeaponSpecBase)
	function WeaponSpecGreatsword(this)
		Super(ABILITY_ID::WEAPON_SPEC_GREATSWORD);
		this.TargetCategory := WEAPON_CATEGORY::GREATSWORD;
	endfunction
endclass

class WeaponSpecAxe(WeaponSpecBase)
	function WeaponSpecAxe(this)
		Super(ABILITY_ID::WEAPON_SPEC_AXE);
		this.TargetCategory := WEAPON_CATEGORY::AXE;
	endfunction
endclass

class WeaponSpecSpear(WeaponSpecBase)
	function WeaponSpecSpear(this)
		Super(ABILITY_ID::WEAPON_SPEC_SPEAR);
		this.TargetCategory := WEAPON_CATEGORY::SPEAR;
	endfunction
endclass

class WeaponSpecMace(WeaponSpecBase)
	function WeaponSpecMace(this)
		Super(ABILITY_ID::WEAPON_SPEC_MACE);
		this.TargetCategory := WEAPON_CATEGORY::MACE;
	endfunction
endclass

class WeaponSpecStaff(WeaponSpecBase)
	function WeaponSpecStaff(this)
		Super(ABILITY_ID::WEAPON_SPEC_STAFF);
		this.TargetCategory := WEAPON_CATEGORY::STAFF;
	endfunction
endclass

class WeaponSpecThrowing(WeaponSpecBase)
	function WeaponSpecThrowing(this)
		Super(ABILITY_ID::WEAPON_SPEC_THROWING);
		this.TargetCategory := WEAPON_CATEGORY::THROWING;
	endfunction
endclass

class WeaponSpecPolearm(WeaponSpecBase)
	function WeaponSpecPolearm(this)
		Super(ABILITY_ID::WEAPON_SPEC_POLEARM);
		this.TargetCategory := WEAPON_CATEGORY::POLEARM;
	endfunction
endclass

class WeaponSpecWhip(WeaponSpecBase)
	function WeaponSpecWhip(this)
		Super(ABILITY_ID::WEAPON_SPEC_WHIP);
		this.TargetCategory := WEAPON_CATEGORY::WHIP;
	endfunction
endclass

class WeaponSpecBow(WeaponSpecBase)
	function WeaponSpecBow(this)
		Super(ABILITY_ID::WEAPON_SPEC_BOW);
		this.TargetCategory := WEAPON_CATEGORY::BOW;
	endfunction
endclass

class WeaponSpecCrossbow(WeaponSpecBase)
	function WeaponSpecCrossbow(this)
		Super(ABILITY_ID::WEAPON_SPEC_CROSSBOW);
		this.TargetCategory := WEAPON_CATEGORY::CROSSBOW;
	endfunction
endclass

class WeaponSpecMagicBow(WeaponSpecBase)
	function WeaponSpecMagicBow(this)
		Super(ABILITY_ID::WEAPON_SPEC_MAGICBOW);
		this.TargetCategory := WEAPON_CATEGORY::MAGICBOW;
	endfunction
endclass

class WeaponSpecFirearm(WeaponSpecBase)
	function WeaponSpecFirearm(this)
		Super(ABILITY_ID::WEAPON_SPEC_FIREARM);
	endfunction

	function _MatchesWeapon(this, weapon)
		if (!weapon || weapon is @Exception)
			return false;
		endif
		var cat := weapon.GetCategory();
		return cat == WEAPON_CATEGORY::PISTOL ||
		       cat == WEAPON_CATEGORY::MUSKET ||
		       cat == WEAPON_CATEGORY::CARABINE ||
		       cat == WEAPON_CATEGORY::HANDCANNON;
	endfunction
endclass
