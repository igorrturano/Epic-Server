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

/***********************************
 * PASSIVE ABILITIES
 * These trigger when player HAS ability (not activated)
 * Discovered via passivoMomento/passivoScript config fields
 **********************************/

/**
 * Executes passive abilities for defender on parry.
 * Called when defender successfully parries an attack.
 */
function ExecutePassiveAbilitiesOnParry(byRef attackInfo)
	_ExecutePassiveAbilities(attackInfo.GetDefender(), ABILITY_ACTIVATION::ON_PARRY, attackInfo);
endfunction

/**
 * Executes passive abilities for attacker on kill.
 * Called when attacker kills the defender.
 */
function ExecutePassiveAbilitiesOnKill(byRef attackInfo)
	_ExecutePassiveAbilities(attackInfo.GetAttacker(), ABILITY_ACTIVATION::ON_KILL, attackInfo);
endfunction

/**
 * Internal: Discovers and executes passive abilities.
 * Checks all abilities the mobile HAS (not just activated).
 *
 * @param mobile MobileWrapper - The mobile whose passive abilities to check
 * @param passiveMoment ABILITY_ACTIVATION enum - When the passive triggers
 * @param attackInfo AttackInfo - Combat context passed to ability scripts
 * @return Integer - Number of passive abilities executed
 */
function _ExecutePassiveAbilities(byRef mobile, passiveMoment, byRef attackInfo)
	var abilities := mobile.GetAbilities();

	print($"[PASSIVE DEBUG] Checking passives for {mobile.GetName()}, moment={passiveMoment}");
	print($"[PASSIVE DEBUG] Abilities: {abilities}");

	if (!abilities)
		print("[PASSIVE DEBUG] No abilities found!");
		return 0;
	endif

	var abilitiesExecuted := 0;

	// myhabs stores { level => ability_id }, so we iterate keys (levels) and get values (ability IDs)
	foreach level in (abilities.keys())
		var abilityId := abilities[level];
		var ability := Ability(CInt(abilityId));

		print($"[PASSIVE DEBUG] Checking ability {abilityId}: passiveMoment={ability.GetPassiveMoment()}, passiveScript={ability.GetPassiveScript()}");

		if (ability.GetPassiveMoment() == passiveMoment && ability.GetPassiveScript())
			var scriptPath := $"{ABILITY_PKG}scripts/Passiva/{passiveMoment}/{ability.GetPassiveScript()}";
			print($"[PASSIVE DEBUG] Running script: {scriptPath}");
			Run_Script_To_Completion(scriptPath, attackInfo);
			abilitiesExecuted++;
		endif
	endforeach

	print($"[PASSIVE DEBUG] Executed {abilitiesExecuted} passive abilities");
	return abilitiesExecuted;
endfunction