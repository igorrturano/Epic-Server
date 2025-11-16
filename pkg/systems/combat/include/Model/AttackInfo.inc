use uo;

include "IAttackInfo";

include "Damage";
include "CombatAbility";
include ":objWrapper:Model/MobileWrapper";
include ":objWrapper:Model/ArmorWrapper";
include ":objWrapper:Model/WeaponWrapper";
include ":diceroll:model/Dice";
include ":message:model/Message";
include "include/packets";

class AttackInfo(IAttackInfo)
	var _moment := ATTACK_MOMENT::COMBATING;
	var _attacker; // MobileWrapper object
	var _defender; // MobileWrapper object
	
	var _bodyRegion := ""; // Região do corpo a ser atingida
	var _direction := ATTACK_DIRECTION::FRONT;
	var _damage; // Damage object
	var _damageHand := DAMAGE_HAND::MAINHAND;
	var _attackType := DAMAGE_TYPE::SLASH; // enum da postura que o atacante está usando no ataque
	var _ammo := struct{
		itemRef := 0x0,
		amount := 0
	};
	var _hitChance := 100;
	var _hitChancePenalty := 0;
	var _hitChanceResult := 0; // Resultado da rolagem do dado de hit chance
	var _criticalHitChance := 0;
	var _criticalHitChancePenalty := 0;
	var _criticalHitChanceResult := 0; // Resultado da rolagem do dado de dano crítico
	var _parryEquip; // EquipWrapper(ArmorWrapper/WeaponWrapper) do item usado para parry
	var _parryChance := 0;
	var _parryChancePenalty := 0;
	var _parryChanceResult := 0; // Resultado da rolagem do dado de parry chance

	var _attackerAbilities := dictionary{}; // O dictionary é indexado por COMBATABILITY_MOMENT e contém arrays de CombatAbility()
	var _defenderAbilities := dictionary{}; // O dictionary é indexado por COMBATABILITY_MOMENT e contém arrays de CombatAbility()

	function AttackInfo(this, attacker, defender, moment := ATTACK_MOMENT::COMBATING) // attacker: MobileWrapper, defender: MobileWrapper
		this._moment := moment;
		this._attacker := attacker;
		this._defender := defender;
		
		this._bodyRegion := error{errortext := "Body region not set yet!"};
		this._direction := ATTACK_DIRECTION::FRONT;
		this._damage := Damage();
		this._damageHand := DAMAGE_HAND::MAINHAND;
		this._hitChance := error{errortext := "Hit chance not set yet!"};
		this._hitChancePenalty := 0;
		this._hitChanceResult := error{errortext := "Hit chance not rolled yet!"};
		this._parryChance := error{errortext := "Parry chance not set yet!"};
		this._parryChancePenalty := 0;
		this._parryChanceResult := error{errortext := "Parry chance not rolled yet!"};

		this._attackerAbilities := AttackInfo::_attackerAbilities;
		this._defenderAbilities := AttackInfo::_defenderAbilities;
	endfunction

	function GetMoment(this)
		return this._moment;
	endfunction
	
	function GetAttacker(this)
		return this._attacker;
	endfunction

	function GetDefender(this)
		return this._defender;
	endfunction

	function GetBodyRegion(this)
		return this._bodyRegion;
	endfunction

	function GetDirection(this)
		return this._direction;
	endfunction

	function GetDamage(this)
		return this._damage;
	endfunction

	function GetDamageHand(this)
		return this._damageHand;
	endfunction

	function GetAttackType(this)
		return this._attackType;
	endfunction

	function GetAmmoObjRef(this)
		return this._ammo.itemRef;
	endfunction

	function GetAmmoAmount(this)
		return this._ammo.amount;
	endfunction

	function GetHitChance(this)
		return this._hitChance;
	endfunction

	function GetHitChancePenalty(this)
		return this._hitChancePenalty;
	endfunction

	function GetHitChanceResult(this)
		return this._hitChanceResult;
	endfunction

	function GetCriticalHitChance(this)
		return this._criticalHitChance;
	endfunction

	function GetCriticalHitChancePenalty(this)
		return this._criticalHitChancePenalty;
	endfunction
	
	function GetCriticalHitChanceResult(this)
		return this._criticalHitChanceResult;
	endfunction
	
	function GetParryEquip(this)
		return this._parryEquip;
	endfunction

	function GetParryChance(this)
		return this._parryChance;
	endfunction

	function GetParryChancePenalty(this)
		return this._parryChancePenalty;
	endfunction

	function GetParryChanceResult(this)
		return this._parryChanceResult;
	endfunction

	function GetAttackerAbilities(this)
		return this._attackerAbilities;
	endfunction

	function GetDefenderAbilities(this)
		return this._defenderAbilities;
	endfunction

	function SetMoment(this, moment)
		this._moment := moment;

		return this;
	endfunction

	function SetAttacker(this, attacker)
		if (!(attacker is @MobileWrapper)) return error{errortext := "Parameter must be a MobileWrapper object!"}; endif
		
		this._attacker := attacker;

		return this;
	endfunction

	function SetDefender(this, defender)
		if (!(defender is @MobileWrapper)) return error{errortext := "Parameter must be a MobileWrapper object!"}; endif

		this._defender := defender;

		return this;
	endfunction

	function SetBodyRegion(this, bodyRegion)
		if (!(bodyRegion in CS_GetArmorZones())) return error{errortext := "Parameter must be a valid armor zone!"}; endif

		this._bodyRegion := bodyRegion;

		return this;
	endfunction

	function SetDirection(this, direction)
		this._direction := direction;

		return this;
	endfunction

	function SetDamage(this, damage)
		if (!(damage is @Damage)) return error{errortext := "Parameter must be a Damage class!"}; endif

		this._damage := damage;

		return this;
	endfunction

	function SetDamageHand(this, damageHand)
		this._damageHand := damageHand;

		return this;
	endfunction

	function SetAttackType(this, attackType) // attackType: enum DAMAGE_TYPE
		this._attackType := attackType;

		return this;
	endfunction

	function SetAmmo(this, ammoItemRef, amount)
		if (amount < 1) return error{errortext := "Amount used must be at least 1!"}; endif

		this._ammo.itemRef := ammoItemRef;
		this._ammo.amount := amount;

		return this;
	endfunction

	function SetHitChance(this, hitChance)
		this._hitChance := hitChance;

		return this;
	endfunction

	function SetHitChancePenalty(this, penalty)
		this._hitChancePenalty := penalty;

		return this;
	endfunction

	function SetCriticalHitChance(this, critChance)
		this._criticalHitChance := critChance;

		return this;
	endfunction

	function SetCriticalHitChancePenalty(this, penalty)
		this._criticalHitChancePenalty := penalty;

		return this;
	endfunction

	function SetParryEquip(this, equip)
		if (!(equip is @ArmorWrapper) && !(equip is @WeaponWrapper))
			return error{errortext := "Parameter must be an ArmorWrapper or WeaponWrapper object!"};
		endif

		this._parryEquip := equip;

		return this;
	endfunction

	function SetParryChance(this, parryChance)
		this._parryChance := parryChance;

		return this;
	endfunction

	function SetParryChancePenalty(this, penalty)
		this._parryChancePenalty := penalty;

		return this;
	endfunction

	function AddAttackerAbility(this, combatAbility)
		if (!(combatAbility is @CombatAbility)) return error{errortext := "Parameter must be a CombatAbility object!"}; endif
		if (!this._attackerAbilities[combatAbility.GetMomentum()]) this._attackerAbilities[combatAbility.GetMomentum()] := array{}; endif

		this._attackerAbilities[combatAbility.GetMomentum()].append(combatAbility);

		return this;
	endfunction

	function AddDefenderAbility(this, combatAbility)
		if (!(combatAbility is @CombatAbility)) return error{errortext := "Parameter must be a CombatAbility object!"}; endif
		if (!this._defenderAbilities[combatAbility.GetMomentum()]) this._defenderAbilities[combatAbility.GetMomentum()] := array{}; endif

		this._defenderAbilities[combatAbility.GetMomentum()].append(combatAbility);

		return this;
	endfunction

	function RollHitChance(this)
		if (!this._hitChance) return error{errortext := "Hit chance not set!"}; endif
		
		this._hitChanceResult := Dice("1d" + this._hitChance).Roll().GetResult();
		return this;
	endfunction

	function RollParryChance(this)
		if (!this._parryChance) return error{errortext := "Parry chance not set!"}; endif
		
		this._parryChanceResult := Dice("1d" + this._parryChance).Roll().GetResult();
		return this;
	endfunction

	function Succeeded(this)
		if (!this._hitChance) return error{errortext := "Hitchance not set!"}; endif
		if (!this._hitChanceResult) return error{errortext := "Hitchance Dice not rolled yet!"}; endif

		return this._hitChanceResult <= this._hitChance;
	endfunction

	function Parried(this)
		if (!this._parryChance) return error{errortext := "Parry chance not set!"}; endif
		if (!this._parryChanceResult) return error{errortext := "Parry chance Dice not rolled yet!"}; endif

		return this._parryChanceResult <= this._parryChance;
	endfunction

	function ExecuteAbilitesOnMomentum(this, momentum) // TODO: Jogar para combat::CombatController
		if (this._abilitiesToExecute[momentum])
			foreach ability in (this._abilitiesToExecute[momentum])
				ability.ExecuteOn(this._attacker, this._defender);
			endforeach
		endif
	endfunction

	function ToString(this)
		// TODO: Implementar uma representação em string do AttackInfo
	endfunction
endclass

enum class ATTACK_MOMENT
	ENGAGING,
	COMBATING
endenum

enum class DAMAGE_HAND
	MAINHAND,
	OFFHAND
endenum

enum class ATTACK_DIRECTION
	FRONT,
	BACK,
	SIDE
endenum

const OFFHAND_DELAY := 500; // em milissegundos
