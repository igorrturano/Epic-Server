/***********************************
 *
 * AbilitiesController.inc
 *
 * Created: 29/12/2025 08:51AM
 *
 * Responsável por:
 * Gerenciar e executar habilidades de combate nos momentos corretos.
 *
 * Princípio: Combat code triggers abilities, AbilityMonitor tracks state.
 *
 **********************************/

include ":debug:Controller/DebugController";
include ":abilities:models/Ability";
include ":abilities:controllers/AbilityMonitor";

/**
 * Executes BEFORE_HIT abilities for both attacker and defender.
 * Called after hit roll, before determining if attack hits.
 */
function ExecuteCombatAbilitiesBeforeHit(byRef attackInfo)
	_ExecuteCombatAbilities(attackInfo.GetAttacker(), ABILITY_ACTIVATION::BEFORE_HIT, attackInfo);
	_ExecuteCombatAbilities(attackInfo.GetDefender(), ABILITY_ACTIVATION::BEFORE_HIT, attackInfo);
endfunction

/**
 * Executes BEFORE_DAMAGE abilities for both attacker and defender.
 * Called after hit confirmed, before damage calculation.
 */
function ExecuteCombatAbilitiesBeforeDamage(byRef attackInfo)
	_ExecuteCombatAbilities(attackInfo.GetAttacker(), ABILITY_ACTIVATION::BEFORE_DAMAGE, attackInfo);
	_ExecuteCombatAbilities(attackInfo.GetDefender(), ABILITY_ACTIVATION::BEFORE_DAMAGE, attackInfo);
endfunction

/**
 * Executes ON_HIT abilities for both attacker and defender.
 * Called after successful hit is confirmed.
 */
function ExecuteCombatAbilitiesOnHit(byRef attackInfo)
	_ExecuteCombatAbilities(attackInfo.GetAttacker(), ABILITY_ACTIVATION::ON_HIT, attackInfo);
	_ExecuteCombatAbilities(attackInfo.GetDefender(), ABILITY_ACTIVATION::ON_HIT, attackInfo);
endfunction

/**
 * Executes AFTER_DAMAGE abilities for both attacker and defender.
 * Called after damage has been applied.
 */
function ExecuteCombatAbilitiesAfterDamage(byRef attackInfo)
	_ExecuteCombatAbilities(attackInfo.GetAttacker(), ABILITY_ACTIVATION::AFTER_DAMAGE, attackInfo);
	_ExecuteCombatAbilities(attackInfo.GetDefender(), ABILITY_ACTIVATION::AFTER_DAMAGE, attackInfo);
endfunction

/**
 * Executes ON_MISS abilities for both attacker and defender.
 * Called when attack misses.
 */
function ExecuteCombatAbilitiesOnMiss(byRef attackInfo)
	_ExecuteCombatAbilities(attackInfo.GetAttacker(), ABILITY_ACTIVATION::ON_MISS, attackInfo);
	_ExecuteCombatAbilities(attackInfo.GetDefender(), ABILITY_ACTIVATION::ON_MISS, attackInfo);
endfunction

/**
 * Executes ON_CRITICAL abilities for both attacker and defender.
 * Called on critical hit.
 */
function ExecuteCombatAbilitiesOnCritical(byRef attackInfo)
	_ExecuteCombatAbilities(attackInfo.GetAttacker(), ABILITY_ACTIVATION::ON_CRITICAL, attackInfo);
	_ExecuteCombatAbilities(attackInfo.GetDefender(), ABILITY_ACTIVATION::ON_CRITICAL, attackInfo);
endfunction

/**
 * Executes ON_KILL abilities for both attacker and defender.
 * Called when defender is killed.
 */
function ExecuteCombatAbilitiesOnKill(byRef attackInfo)
	_ExecuteCombatAbilities(attackInfo.GetAttacker(), ABILITY_ACTIVATION::ON_KILL, attackInfo);
	_ExecuteCombatAbilities(attackInfo.GetDefender(), ABILITY_ACTIVATION::ON_KILL, attackInfo);
endfunction

/**
 * Internal: Executes all abilities for a specific mobile at a specific moment.
 *
 * @param mobile MobileWrapper - The mobile whose abilities to execute
 * @param activationMoment ABILITY_ACTIVATION enum - When the ability triggers
 * @param attackInfo AttackInfo - Combat context passed to ability scripts
 * @return Integer - Number of abilities executed
 */
function _ExecuteCombatAbilities(byRef mobile, activationMoment, byRef attackInfo)
	var abilitiesList := mobile.GetActiveAbilities()[activationMoment];

	if (!abilitiesList)
		return 0;
	endif

	var abilitiesExecuted := 0;

	foreach abilityKey in (abilitiesList.keys())
		var serializedMonitor := abilitiesList[abilityKey];
		var abilityId := serializedMonitor.AbilityId;

		if (abilityId)
			var ability := Ability(abilityId);
			ability.ExecuteOn(attackInfo);

			// Notify AbilityMonitor to consume charge
			var monitor := mobile.GetAbilityMonitor(ability);
			if (monitor is @AbilityMonitor)
				monitor._GetProcess().SendEvent(ABILITY_EVENT_ID::USED);
			endif

			abilitiesExecuted++;
		endif
	endforeach

	return abilitiesExecuted;
endfunction

function DebugExecutedAbilities(byRef attackInfo)
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_HAB, Message("=== Habilidades executadas neste ataque (ATACANTE) ===", Color(GENERAL_COLOR::PURPLE)));
	DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_HAB, Message("=== Habilidades executadas neste ataque (DEFENSOR) ===", Color(GENERAL_COLOR::PURPLE)));
endfunction
