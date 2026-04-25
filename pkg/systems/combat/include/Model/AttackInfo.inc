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
	var _moment := ATTACK_MOMENT::COMBATING;
	var _attacker; // MobileWrapper object
	var _defender; // MobileWrapper object
	
	var _bodyRegion := ""; // Região do corpo a ser atingida
	var _direction := ATTACK_DIRECTION::FRONT;
	var _damage; // Damage object
	var _damageHand := DAMAGE_HAND::MAINHAND;
	var _attackType := DAMAGE_TYPE::SLASH; // enum da postura que o atacante está usando no ataque
	var _weapon; // WeaponWrapper object
	var _ammo; // Ammo object
	var _hitChance := 100;
	var _hitChanceMod := 0;
	var _hitChanceResult := 0; // Resultado da rolagem do dado de hit chance
	var _criticalHitChance := 0;
	var _criticalHitChanceResult := 0; // Resultado da rolagem do dado de dano crítico
	var _parryEquip; // EquipWrapper(ArmorWrapper/WeaponWrapper) do item usado para parry
	var _parryChance := 0;
	var _parryChanceMod := 0;
	var _parryChanceResult := 0; // Resultado da rolagem do dado de parry chance

	var _ignoreArmor := false; // Flag for abilities that bypass armor (encontrarbrecha, marcadacaca)
	var _counterAttack := false; // Flag for abilities that trigger counter-attack (crosscut)
	var _criticalHitChanceMod := 0; // Modifier for critical hit chance (for future combat refactor)
	var _attackerAbilities := dictionary{}; // Dictionary of attacker's abilities (myhabs)
	var _defenderAbilities := dictionary{}; // Dictionary of defender's abilities (myhabs)

	// Spell-origin fields — populated when damage comes from a spell via SC_DealDamage.
	// Spells default to skipping parry, crit, and body-region rolls since those are
	// combat-specific mechanics; individual spells can opt into them if needed.
	var _isSpell := false;
	var _spellId := 0;
	var _damageType := ""; // e.g. DMG_FIRE / DMG_COLD / DMG_ENERGY
	var _skipParry := false;
	var _skipCrit := false;
	var _skipBodyRegion := false;

	function AttackInfo(this, attacker, defender, moment := ATTACK_MOMENT::COMBATING) // attacker: MobileWrapper, defender: MobileWrapper
		this._moment := moment;
		this._attacker := attacker;
		this._defender := defender;

		this._bodyRegion := ValueNotSetException();
		this._direction := ATTACK_DIRECTION::FRONT;
		this._damage := Damage();
		this._damageHand := DAMAGE_HAND::MAINHAND;
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
	endfunction

	/**
	 * Obtém o momento do ataque.
	 *
	 * @return ATTACK_MOMENT Momento atual do ataque (ENGAGING ou COMBATING)
	 */
	function GetMoment(this)
		return this._moment;
	endfunction
	
	/**
	 * Obtém o atacante.
	 *
	 * @return MobileWrapper Objeto que representa o atacante
	 */
	function GetAttacker(this)
		return this._attacker;
	endfunction

	/**
	 * Obtém o defensor.
	 *
	 * @return MobileWrapper Objeto que representa o defensor
	 */
	function GetDefender(this)
		return this._defender;
	endfunction

	/**
	 * Obtém a região do corpo alvo do ataque.
	 *
	 * @return String Região do corpo a ser atingida
	 */
	function GetBodyRegion(this)
		return this._bodyRegion;
	endfunction

	/**
	 * Obtém a direção do ataque.
	 *
	 * @return ATTACK_DIRECTION Direção do ataque (FRONT, BACK, SIDE)
	 */
	function GetDirection(this)
		return this._direction;
	endfunction

	/**
	 * Obtém o objeto de dano.
	 *
	 * @return Damage Objeto contendo informações sobre o dano do ataque
	 */
	function GetDamage(this)
		return this._damage;
	endfunction

	/**
	 * Obtém a mão usada para o ataque.
	 *
	 * @return DAMAGE_HAND Mão do ataque (MAINHAND ou OFFHAND)
	 */
	function GetDamageHand(this)
		return this._damageHand;
	endfunction

	/**
	 * Obtém o tipo de ataque.
	 *
	 * @return DAMAGE_TYPE Tipo de ataque baseado na postura
	 */
	function GetAttackType(this)
		return this._attackType;
	endfunction

	/**
	 * Obtém a arma usada no ataque.
	 *
	 * @return WeaponWrapper Objeto representando a arma equipada
	 */
	function GetWeapon(this)
		return this._weapon;
	endfunction
	
	/**
	 * Define a arma utilizada no ataque.
	 *
	 * @param weapon WeaponWrapper Objeto que representa a arma equipada pelo atacante
	 * @return AttackInfo Retorna a própria instância para encadeamento
	 * @throws InvalidFormatException Se o parâmetro não for um objeto WeaponWrapper
	 */
	function SetWeapon(this, weapon)
		if (!(weapon is @WeaponWrapper))
			return InvalidFormatException("Parameter must be a WeaponWrapper object!");
		endif
		
		this._weapon := weapon;

		return this;
	endfunction

	/**
	 * Obtém a munição usada no ataque.
	 *
	 * @return Ammo Objeto representando a munição
	 */
	function GetAmmo(this)
		return this._ammo;
	endfunction

	/**
	 * Obtém a chance base de acerto.
	 *
	 * @return Integer Chance de acerto (0-100)
	 */
	function GetHitChance(this)
		return this._hitChance;
	endfunction

	/**
	 * Obtém o modificador da chance de acerto.
	 *
	 * @return Integer Modificador aplicado à chance de acerto
	 */
	function GetHitChanceMod(this)
		return this._hitChanceMod;
	endfunction

	/**
	 * Obtém o resultado da rolagem de chance de acerto.
	 *
	 * @return Integer Valor rolado para determinar acerto (0-100)
	 */
	function GetHitChanceResult(this)
		return this._hitChanceResult;
	endfunction

	/**
	 * Obtém a chance base de acerto crítico.
	 *
	 * @return Integer Chance de acerto crítico (0-100)
	 */
	function GetCriticalHitChance(this)
		return this._criticalHitChance;
	endfunction

	/**
	 * Obtém o resultado da rolagem de acerto crítico.
	 *
	 * @return Integer Valor rolado para determinar crítico (0-100)
	 */
	function GetCriticalHitChanceResult(this)
		return this._criticalHitChanceResult;
	endfunction
	
	/**
	 * Obtém o equipamento usado para aparar.
	 *
	 * @return EquipWrapper Item usado na defesa (ArmorWrapper ou WeaponWrapper)
	 */
	function GetParryEquip(this)
		return this._parryEquip;
	endfunction

	/**
	 * Obtém a chance base de aparar.
	 *
	 * @return Integer Chance de aparar o ataque (0-100)
	 */
	function GetParryChance(this)
		return this._parryChance;
	endfunction

	/**
	 * Obtém o modificador da chance de aparar.
	 *
	 * @return Integer Modificador aplicado à chance de aparar
	 */
	function GetParryChanceMod(this)
		return this._parryChanceMod;
	endfunction

	/**
	 * Obtém o resultado da rolagem de chance de aparar.
	 *
	 * @return Integer Valor rolado para determinar aparar (0-100)
	 */
	function GetParryChanceResult(this)
		return this._parryChanceResult;
	endfunction

	/**
	 * Verifica se o ataque ignora armadura.
	 *
	 * @return Boolean true se ignora armadura, false caso contrário
	 */
	function IgnoresArmor(this)
		return this._ignoreArmor;
	endfunction

	/**
	 * Verifica se o defensor deve contra-atacar após receber dano.
	 *
	 * @return Boolean true se deve contra-atacar, false caso contrário
	 */
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

	/**
	 * Obtém o modificador da chance de acerto crítico.
	 *
	 * @return Integer Modificador aplicado à chance de crítico
	 */
	function GetCriticalHitChanceMod(this)
		return this._criticalHitChanceMod;
	endfunction

	/**
	 * Obtém as habilidades do atacante.
	 *
	 * @return Dictionary Dicionário de habilidades do atacante
	 */
	function GetAttackerAbilities(this)
		return this._attackerAbilities;
	endfunction

	/**
	 * Obtém as habilidades do defensor.
	 *
	 * @return Dictionary Dicionário de habilidades do defensor
	 */
	function GetDefenderAbilities(this)
		return this._defenderAbilities;
	endfunction

	/**
	 * Define o momento do ataque.
	 *
	 * @param moment ATTACK_MOMENT Momento do ataque (ENGAGING ou COMBATING)
	 * @return AttackInfo Retorna a própria instância para encadeamento
	 */
	function SetMoment(this, moment)
		this._moment := moment;

		return this;
	endfunction

	/**
	 * Define o atacante.
	 *
	 * @param attacker MobileWrapper Objeto que representa o atacante
	 * @return AttackInfo Retorna a própria instância para encadeamento
	 * @throws Error Se o parâmetro não for um MobileWrapper
	 */
	function SetAttacker(this, attacker)
		if (!(attacker is @MobileWrapper)) return InvalidFormatException("Parameter must be a MobileWrapper object!"); endif
		
		this._attacker := attacker;

		return this;
	endfunction

	/**
	 * Define o defensor.
	 *
	 * @param defender MobileWrapper Objeto que representa o defensor
	 * @return AttackInfo Retorna a própria instância para encadeamento
	 * @throws Error Se o parâmetro não for um MobileWrapper
	 */
	function SetDefender(this, defender)
		if (!(defender is @MobileWrapper)) return InvalidFormatException("Parameter must be a MobileWrapper object!"); endif

		this._defender := defender;

		return this;
	endfunction

	/**
	 * Define a região do corpo alvo do ataque.
	 *
	 * @param bodyRegion String Região do corpo (deve ser uma zona de armadura válida)
	 * @return AttackInfo Retorna a própria instância para encadeamento
	 * @throws Error Se o parâmetro não for uma zona de armadura válida
	 */
	function SetBodyRegion(this, bodyRegion)
		if (!(bodyRegion in CS_GetArmorZones())) return InvalidFormatException("Parameter must be a valid armor zone!"); endif

		this._bodyRegion := bodyRegion;

		return this;
	endfunction

	/**
	 * Define a direção do ataque.
	 *
	 * @param direction ATTACK_DIRECTION Direção do ataque
	 * @return AttackInfo Retorna a própria instância para encadeamento
	 */
	function SetDirection(this, direction)
		this._direction := direction;

		return this;
	endfunction

	/**
	 * Define o objeto de dano.
	 *
	 * @param damage Damage Objeto contendo informações sobre o dano
	 * @return AttackInfo Retorna a própria instância para encadeamento
	 * @throws Error Se o parâmetro não for um objeto Damage
	 */
	function SetDamage(this, damage)
		if (!(damage is @Damage)) return error{errortext := "Parameter must be a Damage class!"}; endif

		this._damage := damage;

		return this;
	endfunction

	/**
	 * Define a mão usada para o ataque.
	 *
	 * @param damageHand DAMAGE_HAND Mão do ataque (MAINHAND ou OFFHAND)
	 * @return AttackInfo Retorna a própria instância para encadeamento
	 */
	function SetDamageHand(this, damageHand)
		this._damageHand := damageHand;

		return this;
	endfunction

	/**
	 * Define o tipo de ataque.
	 *
	 * @param attackType DAMAGE_TYPE Tipo de ataque baseado na postura
	 * @return AttackInfo Retorna a própria instância para encadeamento
	 */
	function SetAttackType(this, attackType)
		this._attackType := attackType;

		return this;
	endfunction

	/**
	 * Define a munição usada no ataque.
	 *
	 * @param ammo Ammo Objeto representando a munição
	 * @return AttackInfo Retorna a própria instância para encadeamento
	 * @throws InvalidFormatException Se o parâmetro não for um objeto Ammo
	 */
	function SetAmmo(this, ammo)
		if (!(ammo is @Ammo)) return InvalidFormatException("Parameter must be an Ammo object!"); endif
		
		this._ammo := ammo;

		return this;
	endfunction

	/**
	 * Define a chance base de acerto.
	 *
	 * @param value Integer Chance de acerto (0-100)
	 * @return AttackInfo Retorna a própria instância para encadeamento
	 */
	function SetHitChance(this, value)
		this._hitChance := CInt(value);

		return this;
	endfunction

	/**
	 * Define o modificador da chance de acerto.
	 *
	 * @param value Integer Modificador a ser aplicado
	 * @return AttackInfo Retorna a própria instância para encadeamento
	 */
	function SetHitChanceMod(this, value)
		this._hitChanceMod := CInt(value);

		return this;
	endfunction

	/**
	 * Define a chance base de acerto crítico.
	 *
	 * @param value Integer Chance de crítico (0-100)
	 * @return AttackInfo Retorna a própria instância para encadeamento
	 */
	function SetCriticalHitChance(this, value)
		this._criticalHitChance := CInt(value);

		return this;
	endfunction

	/**
	 * Define o modificador da chance de acerto crítico.
	 *
	 * @param value Integer Modificador a ser aplicado
	 * @return AttackInfo Retorna a própria instância para encadeamento
	 */
	function SetCriticalHitChanceMod(this, value)
		this._criticalHitChanceMod := CInt(value);

		return this;
	endfunction

	/**
	 * Define o equipamento usado para aparar.
	 *
	 * @param equip EquipWrapper Item de defesa (ArmorWrapper ou WeaponWrapper)
	 * @return AttackInfo Retorna a própria instância para encadeamento
	 * @throws InvalidFormatException Se não for ArmorWrapper ou WeaponWrapper
	 */
	function SetParryEquip(this, equip)
		if (!(equip is @ArmorWrapper) && !(equip is @WeaponWrapper))
			return InvalidFormatException("Parameter must be an ArmorWrapper or WeaponWrapper object!");
		endif

		this._parryEquip := equip;

		return this;
	endfunction

	/**
	 * Define a chance base de aparar.
	 *
	 * @param parryChance Integer Chance de aparar (0-100)
	 * @return AttackInfo Retorna a própria instância para encadeamento
	 */
	function SetParryChance(this, parryChance)
		this._parryChance := CInt(parryChance);

		return this;
	endfunction

	/**
	 * Define a penalidade da chance de aparar.
	 *
	 * @param penalty Integer Penalidade a ser aplicada
	 * @return AttackInfo Retorna a própria instância para encadeamento
	 */
	function SetParryChancePenalty(this, penalty)
		this._parryChancePenalty := CInt(penalty);

		return this;
	endfunction

	/**
	 * Define o modificador da chance de aparar.
	 *
	 * @param value Integer Modificador a ser aplicado (use 100 for guaranteed parry)
	 * @return AttackInfo Retorna a própria instância para encadeamento
	 */
	function SetParryChanceMod(this, value)
		this._parryChanceMod := CInt(value);

		return this;
	endfunction

	/**
	 * Define se o ataque ignora armadura.
	 *
	 * @param value Boolean true para ignorar armadura
	 * @return AttackInfo Retorna a própria instância para encadeamento
	 */
	function SetIgnoreArmor(this, value := true)
		this._ignoreArmor := value;

		return this;
	endfunction

	/**
	 * Define se o defensor deve contra-atacar após receber dano.
	 *
	 * @param value Boolean true para contra-atacar
	 * @return AttackInfo Retorna a própria instância para encadeamento
	 */
	function SetCounterAttack(this, value := true)
		this._counterAttack := value;

		return this;
	endfunction

	// Mark this AttackInfo as originating from a spell. Auto-enables skipParry/Crit/BodyRegion
	// since those rolls are combat-specific; callers can re-enable them individually if desired.
	function SetSpellOrigin(this, spellId, damageType := "")
		this._isSpell := true;
		this._spellId := CInt(spellId);
		this._damageType := damageType;
		this._skipParry := true;
		this._skipCrit := true;
		this._skipBodyRegion := true;
		return this;
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

	/**
	 * Rola um dado para determinar se o ataque acerta.
	 *
	 * Gera um número aleatório entre 0 e 100 e armazena em _hitChanceResult.
	 *
	 * @return AttackInfo Retorna a própria instância para encadeamento
	 * @throws Exception Se a chance de acerto não foi definida
	 */
	function RollHitChance(this)
		if (this._hitChance is @Exception) return this._hitChance; endif
		
		this._hitChanceResult := RandomInt(100 + 1);
		return this;
	endfunction

	/**
	 * Rola um dado para determinar se o ataque é crítico.
	 *
	 * Gera um número aleatório entre 0 e 100 e armazena em _criticalHitChanceResult.
	 * Se o resultado for menor ou igual à chance de crítico, marca o dano como crítico.
	 *
	 * @return AttackInfo Retorna a própria instância para encadeamento
	 * @throws Exception Se a chance de crítico não foi definida
	 */
	function RollCriticalHitChance(this)
		if (this._criticalHitChance is @Exception) return this._criticalHitChance; endif

		this._criticalHitChanceResult := RandomInt(100 + 1);
		if (this._criticalHitChanceResult <= this._criticalHitChance)
			this._damage.SetCritical(true);
		endif
		return this;
	endfunction

	/**
	 * Rola um dado para determinar se o ataque é aparado.
	 *
	 * Gera um número aleatório entre 0 e 100 e armazena em _parryChanceResult.
	 *
	 * @return AttackInfo Retorna a própria instância para encadeamento
	 * @throws Exception Se a chance de aparar não foi definida
	 */
	function RollParryChance(this)
		if (this._parryChance is @Exception) return this._parryChance; endif
		
		this._parryChanceResult := RandomInt(100 + 1);
		return this;
	endfunction

	/**
	 * Verifica se o ataque foi bem-sucedido.
	 *
	 * Compara o resultado da rolagem com a chance de acerto.
	 *
	 * @return Boolean true se o ataque acertou, false caso contrário
	 * @throws Exception Se a chance ou resultado não foram definidos
	 */
	function Succeeded(this)
		if (this._hitChance is @Exception) return this._hitChance; endif
		if (this._hitChanceResult is @Exception) return this._hitChanceResult; endif

		return CInt(this._hitChanceResult) <= CInt(this._hitChance) + CInt(this._hitChanceMod);
	endfunction

	/**
	 * Verifica se o ataque foi aparado.
	 *
	 * Compara o resultado da rolagem com a chance de aparar.
	 *
	 * @return Boolean true se o ataque foi aparado, false caso contrário
	 * @throws Exception Se a chance ou resultado de aparar não foram definidos
	 */
	function Parried(this)
		if (this._parryChance is @Exception) return this._parryChance; endif
		if (this._parryChanceResult is @Exception) return this._parryChanceResult; endif

		return CInt(this._parryChanceResult) <= CInt(this._parryChance) + CInt(this._parryChanceMod);
	endfunction

	/**
	 * Executa habilidades programadas para um determinado momento.
	 *
	 * @param momentum COMBATABILITY_MOMENT Momento de execução das habilidades
	 * @deprecated TODO: Mover para combat::CombatController
	 */
	function ExecuteAbilitesOnMomentum(this, momentum)
		if (this._abilitiesToExecute[momentum])
			foreach ability in (this._abilitiesToExecute[momentum])
				ability.ExecuteOn(this._attacker, this._defender);
			endforeach
		endif
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
