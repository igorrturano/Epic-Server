use uo;

include "system/exception/FormatException/InvalidFormatException";
include ":combat:Exceptions/_all";

include "IAttackInfo";

include "Damage";
// include "CombatAbility";
include ":wrappers:Objects/Model/MobileWrapper";
include ":wrappers:Objects/Model/ArmorWrapper";
include ":wrappers:Objects/Model/WeaponWrapper";
include ":combat:Model/Ammo";
include ":diceroll:model/Dice";
include ":message:model/Message";
include "include/packets";

/**
 * Classe que representa informações detalhadas sobre um ataque no sistema de combate.
 * 
 * Esta classe encapsula todos os dados relacionados a um ataque, incluindo informações sobre
 * o atacante, defensor, tipo de dano, chances de acerto, crítico e aparar, além de gerenciar
 * habilidades de combate que podem ser executadas durante o ataque.
 *
 * @implements IAttackInfo Interface base para informações de ataque
 */
class AttackInfo(IAttackInfo)
	// All fields are initialized in the constructor, not via declaration defaults.
	// AttackInfo crosses a Start_Script boundary (combatHook -> mainHitScript) where
	// declaration-only defaults deserialize as uninitialized. See combat-pipeline.md Stage 4.
	var _moment;
	var _attacker;       // MobileWrapper
	var _defender;       // MobileWrapper
	var _bodyRegion;     // hit zone
	var _direction;
	var _damage;         // Damage object
	var _damageHand;
	var _attackType;     // attacker's posture (slash/piercing/blunt)
	var _weapon;         // WeaponWrapper — set by combatHook
	var _ammo;           // Ammo — set for ranged
	var _hitChance;
	var _hitChanceMod;
	var _hitChanceResult;
	var _criticalHitChance;
	var _criticalHitChanceResult;
	var _parryEquip;     // EquipWrapper used to parry — set in ParryController
	var _parryChance;
	var _parryChanceMod;
	var _parryChanceResult;
	var _ignoreArmor;    // ability bypasses armor (encontrarbrecha, marcadacaca)
	var _counterAttack;  // ability triggers counter-attack (crosscut)
	var _criticalHitChanceMod;
	var _attackerAbilities;
	var _defenderAbilities;

	// Spell-origin flags — set when damage comes from a spell (SC_DealDamage).
	var _isSpell;
	var _spellId;
	var _damageType;     // DAMAGE_TYPE::FIRE / COLD / ENERGY / ...
	var _skipParry;
	var _skipCrit;
	var _skipBodyRegion;
	var _appliesArmorWear; // true: a swing wears the armor it hits. Spells/env set false (resist still applies).
	var _appliesByChannel; // true: apply each stamped damage channel as-is (spells). false: collapse to attacker posture (swings).

	function AttackInfo(this, attacker, defender, moment := ATTACK_MOMENT::COMBATING) // attacker: MobileWrapper, defender: MobileWrapper
		this._moment := moment;
		this._attacker := attacker;
		this._defender := defender;

		this._bodyRegion := ValueNotSetException();
		this._direction := ATTACK_DIRECTION::FRONT;
		this._damage := Damage();
		this._damageHand := DAMAGE_HAND::MAINHAND;
		this._attackType := DAMAGE_TYPE::SLASH;
		this._hitChance := ValueNotSetException();
		this._hitChanceMod := ValueNotSetException();
		this._hitChanceResult := ValueNotSetException();
		this._parryChance := ValueNotSetException();
		this._parryChanceMod := ValueNotSetException();
		this._parryChanceResult := ValueNotSetException();
		this._criticalHitChance := ValueNotSetException();
		this._criticalHitChanceResult := ValueNotSetException();
		this._criticalHitChanceMod := 0;
		this._attackerAbilities := attacker.GetAbilities() ?: dictionary{};
		this._defenderAbilities := defender.GetAbilities() ?: dictionary{};

		this._isSpell := false;
		this._spellId := 0;
		this._damageType := "";
		this._skipParry := false;
		this._skipCrit := false;
		this._skipBodyRegion := false;
		this._ignoreArmor := false;
		this._counterAttack := false;
		this._appliesArmorWear := true;
		this._appliesByChannel := false;
		// _weapon, _ammo, _parryEquip stay uninit — set by their setters later.
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

	function GetWeapon(this)
		return this._weapon;
	endfunction
	
	function SetWeapon(this, weapon)
		if (!(weapon is @WeaponWrapper))
			return InvalidFormatException("Parameter must be a WeaponWrapper object!");
		endif
		
		this._weapon := weapon;

		return this;
	endfunction

	function GetAmmo(this)
		return this._ammo;
	endfunction

	function GetHitChance(this)
		return this._hitChance;
	endfunction

	function GetHitChanceMod(this)
		return this._hitChanceMod;
	endfunction

	function GetHitChanceResult(this)
		return this._hitChanceResult;
	endfunction

	function GetCriticalHitChance(this)
		return this._criticalHitChance;
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

	function GetParryChanceMod(this)
		return this._parryChanceMod;
	endfunction

	function GetParryChanceResult(this)
		return this._parryChanceResult;
	endfunction

	function IgnoresArmor(this)
		return this._ignoreArmor;
	endfunction

	function ShouldCounterAttack(this)
		return this._counterAttack;
	endfunction

	function IsSpell(this)
		return this._isSpell;
	endfunction

	function GetSpellId(this)
		return this._spellId;
	endfunction

	function GetDamageType(this)
		return this._damageType;
	endfunction

	function ShouldSkipParry(this)
		return this._skipParry;
	endfunction

	function ShouldSkipCrit(this)
		return this._skipCrit;
	endfunction

	function ShouldSkipBodyRegion(this)
		return this._skipBodyRegion;
	endfunction

	function GetCriticalHitChanceMod(this)
		return this._criticalHitChanceMod;
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
		if (!(attacker is @MobileWrapper)) return InvalidFormatException("Parameter must be a MobileWrapper object!"); endif
		
		this._attacker := attacker;

		return this;
	endfunction

	function SetDefender(this, defender)
		if (!(defender is @MobileWrapper)) return InvalidFormatException("Parameter must be a MobileWrapper object!"); endif

		this._defender := defender;

		return this;
	endfunction

	function SetBodyRegion(this, bodyRegion)
		if (!(bodyRegion in CS_GetArmorZones())) return InvalidFormatException("Parameter must be a valid armor zone!"); endif

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

	function SetAttackType(this, attackType)
		this._attackType := attackType;

		return this;
	endfunction

	function SetAmmo(this, ammo)
		if (!(ammo is @Ammo)) return InvalidFormatException("Parameter must be an Ammo object!"); endif
		
		this._ammo := ammo;

		return this;
	endfunction

	function SetHitChance(this, value)
		this._hitChance := CInt(value);

		return this;
	endfunction

	function SetHitChanceMod(this, value)
		this._hitChanceMod := CInt(value);

		return this;
	endfunction

	function SetCriticalHitChance(this, value)
		this._criticalHitChance := CInt(value);

		return this;
	endfunction

	function SetCriticalHitChanceMod(this, value)
		this._criticalHitChanceMod := CInt(value);

		return this;
	endfunction

	function SetParryEquip(this, equip)
		if (!(equip is @ArmorWrapper) && !(equip is @WeaponWrapper))
			return InvalidFormatException("Parameter must be an ArmorWrapper or WeaponWrapper object!");
		endif

		this._parryEquip := equip;

		return this;
	endfunction

	function SetParryChance(this, parryChance)
		this._parryChance := CInt(parryChance);

		return this;
	endfunction

	function SetParryChanceMod(this, value)
		this._parryChanceMod := CInt(value);

		return this;
	endfunction

	function SetIgnoreArmor(this, value := true)
		this._ignoreArmor := value;

		return this;
	endfunction

	function SetCounterAttack(this, value := true)
		this._counterAttack := value;

		return this;
	endfunction

	// Mark this AttackInfo as spell-originated. Spells skip the combat-specific
	// parry/crit/body-region rolls, don't wear armor, and apply damage by channel.
	function SetSpellOrigin(this, spellId, damageType := "")
		this._isSpell := true;
		this._spellId := CInt(spellId);
		this._damageType := damageType;
		this._skipParry := true;
		this._skipCrit := true;
		this._skipBodyRegion := true;
		this._appliesArmorWear := false;
		this._appliesByChannel := true;
		return this;
	endfunction

	// true: apply each stamped damage channel as-is (spells/DealDamage).
	// false: collapse physical damage to the attacker's posture (swings).
	function SetAppliesByChannel(this, value := true)
		this._appliesByChannel := value;
		return this;
	endfunction

	function AppliesByChannel(this)
		return this._appliesByChannel;
	endfunction

	function SetSkipParry(this, value := true)
		this._skipParry := value;
		return this;
	endfunction

	function SetSkipCrit(this, value := true)
		this._skipCrit := value;
		return this;
	endfunction

	function SetSkipBodyRegion(this, value := true)
		this._skipBodyRegion := value;
		return this;
	endfunction

	function SetAppliesArmorWear(this, value := true)
		this._appliesArmorWear := value;
		return this;
	endfunction

	function GetAppliesArmorWear(this)
		return this._appliesArmorWear;
	endfunction

	function RollHitChance(this)
		if (this._hitChance is @Exception) return this._hitChance; endif
		
		this._hitChanceResult := RandomInt(100 + 1);
		return this;
	endfunction

	function RollCriticalHitChance(this)
		if (this._criticalHitChance is @Exception) return this._criticalHitChance; endif

		this._criticalHitChanceResult := RandomInt(100 + 1);
		if (this._criticalHitChanceResult <= this._criticalHitChance)
			this._damage.SetCritical(true);
		endif
		return this;
	endfunction

	function RollParryChance(this)
		if (this._parryChance is @Exception) return this._parryChance; endif
		
		this._parryChanceResult := RandomInt(100 + 1);
		return this;
	endfunction

	function Succeeded(this)
		if (this._hitChance is @Exception) return this._hitChance; endif
		if (this._hitChanceResult is @Exception) return this._hitChanceResult; endif

		return CInt(this._hitChanceResult) <= CInt(this._hitChance) + CInt(this._hitChanceMod);
	endfunction

	function Parried(this)
		if (this._parryChance is @Exception) return this._parryChance; endif
		if (this._parryChanceResult is @Exception) return this._parryChanceResult; endif

		return CInt(this._parryChanceResult) <= CInt(this._parryChance) + CInt(this._parryChanceMod);
	endfunction

	/**
	 * Substituicao (Kawarimi, spell 1100) — reação de fuga do defensor.
	 *
	 * Disparada na janela before-hit. NÃO depende da habilidade NINJA_PATH:
	 * o cast foi desacoplado da habilidade (o Shinobi conjura desde a criação
	 * mas só ganha a habilidade ao subir de nível), então o efeito chaveia
	 * apenas em #substituicao_active, espelhando como o combate lê #sanctuary.
	 *
	 * @return Boolean true se o ataque foi substituído (forçado a errar)
	 */
	function ResolveSubstituicao(this)
		var defRef := this._defender.GetObjRef();

		if (!defRef.GetProp("#substituicao_active"))
			return false;
		endif

		// Consome a flag: encerra o wait-loop da magia e impede re-trigger.
		defRef.EraseProp("#substituicao_active");

		// Deixa uma tora no lugar do ninja e some.
		var decoy := CreateItemAtLocation(defRef.x, defRef.y, defRef.z, 0x1BDD, 1, defRef.realm);
		if (decoy)
			decoy.name := "Tora";
			decoy.decayat := ReadGameClock() + 15;
		endif

		defRef.hidden := 1;

		PlaySoundEffect(defRef, 0x228);
		PlayObjectCenteredEffect(defRef, 0x3728, 10, 10);

		Message("*Substituicao!*").SendEmote(defRef);
		Message($"Voce substituiu seu corpo por uma tora!", Color(SYSTEM_COLOR::SUCCESS)).SendSys(defRef);

		// Força este ataque a errar.
		this._hitChanceMod := -1000;

		return true;
	endfunction

	function ToString(this)
		return $"Detalhes do AttackInfo: " +
			$"Momento: {this._moment}, " +
			$"Atacante: {this._attacker}, " +
			$"Defensor: {this._defender}, " +
			$"Região do Corpo: {this._bodyRegion}, " +
			$"Direção: {this._direction}, " +
			$"Dano: {this._damage}, " +
			$"Mão do Dano: {this._damageHand}, " +
			$"Tipo de Ataque: {this._attackType}, " +
			$"Ref. Item Munição: {this._ammo.itemRef}, " +
			$"Quantidade Munição: {this._ammo.amount}, " +
			$"Chance de Acerto: {this._hitChance}, " +
			$"Mod Chance de Acerto: {this._hitChanceMod}, " +
			$"Resultado Chance de Acerto: {this._hitChanceResult}, " +
			$"Chance de Acerto Crítico: {this._criticalHitChance}, " +
			$"Mod Chance de Acerto Crítico: {this._criticalHitChanceMod}, " +
			$"Resultado Chance de Acerto Crítico: {this._criticalHitChanceResult}, " +
			$"Equipamento de Aparar: {this._parryEquip}, " +
			$"Chance de Aparar: {this._parryChance}, " +
			$"Mod Chance de Aparar: {this._parryChanceMod}, " +
			$"Resultado Chance de Aparar: {this._parryChanceResult}, " +
			$"Habilidades do Atacante: {this._attackerAbilities}, " +
			$"Habilidades do Defensor: {this._defenderAbilities}";
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
