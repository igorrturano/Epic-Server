include "IHitInfo";

include "Damage";
include "CombatAbility";

class HitInfo(IHitInfo)
	var _attacker; // MobRef
	var _defender; // MobRef
	var _armorHit; // ItemRef
	var _slotHit := ""; // string

	var _weapon; // ItemRef
	var _damage := Damage();

	var _abilitiesToExecute := dictionary{};

	function HitInfo(this, attacker, defender, weapon, armorHit, slotHit)
		this._attacker := attacker;
		this._defender := defender;
		this._weapon := weapon;
		this._armorHit := armorHit;
		this._slotHit := slotHit;

		this._damage := Damage();
		this._abilitiesToExecute := dictionary{};
	endfunction

	function AddAbility(this, combatAbility)
		if (!this._abilitiesToExecute[combatAbility.GetMomentum()]) this._abilitiesToExecute[combatAbility.GetMomentum()] := array{}; endif

		this._abilitiesToExecute[combatAbility.GetMomentum()].append(combatAbility);

		return this;
	endfunction
endclass