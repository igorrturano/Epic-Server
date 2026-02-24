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
include ":abilities:AbilityRegistryBase";
include ":abilities:controllers/AbilityMonitor";

/**
 * ATTACKER-CENTRIC: Executes BEFORE_DAMAGE abilities for attacker only.
 * Called after hit confirmed, before damage calculation.
 */
function ExecuteCombatAbilitiesBeforeDamage(byRef attackInfo)
	_ExecuteCombatAbilities(attackInfo.GetAttacker(), ABILITY_ACTIVATION::BEFORE_DAMAGE, attackInfo);
endfunction

/**
 * ATTACKER-CENTRIC: Executes ON_HIT abilities for attacker only.
 * Called after successful hit is confirmed.
 */
function ExecuteCombatAbilitiesOnHit(byRef attackInfo)
	_ExecuteCombatAbilities(attackInfo.GetAttacker(), ABILITY_ACTIVATION::ON_HIT, attackInfo);
endfunction

/**
 * ATTACKER-CENTRIC: Executes AFTER_DAMAGE abilities for attacker only.
 * Called after damage has been applied.
 */
function ExecuteCombatAbilitiesAfterDamage(byRef attackInfo)
	_ExecuteCombatAbilities(attackInfo.GetAttacker(), ABILITY_ACTIVATION::AFTER_DAMAGE, attackInfo);
endfunction

/**
 * ATTACKER-CENTRIC: Executes ON_CRITICAL abilities for attacker only.
 * Called on critical hit.
 */
function ExecuteCombatAbilitiesOnCritical(byRef attackInfo)
	_ExecuteCombatAbilities(attackInfo.GetAttacker(), ABILITY_ACTIVATION::ON_CRITICAL, attackInfo);
endfunction

/**
 * ATTACKER-CENTRIC: Executes ON_KILL abilities for attacker only.
 * Called when defender is killed.
 */
function ExecuteCombatAbilitiesOnKill(byRef attackInfo)
	_ExecuteCombatAbilities(attackInfo.GetAttacker(), ABILITY_ACTIVATION::ON_KILL, attackInfo);
endfunction

/**
 * DEFENDER-CENTRIC: Executes BEFORE_HIT abilities for defender only.
 * Called before hit roll - defensive abilities.
 */
function ExecuteCombatAbilitiesBeforeHit(byRef attackInfo)
	_ExecuteCombatAbilities(attackInfo.GetDefender(), ABILITY_ACTIVATION::BEFORE_HIT, attackInfo);
endfunction

/**
 * DEFENDER-CENTRIC: Executes ON_MISS abilities for defender only.
 * Called when attack misses - defender reactions.
 */
function ExecuteCombatAbilitiesOnMiss(byRef attackInfo)
	_ExecuteCombatAbilities(attackInfo.GetDefender(), ABILITY_ACTIVATION::ON_MISS, attackInfo);
endfunction

/**
 * DEFENDER-CENTRIC: Executes ON_PARRY abilities for defender only.
 * Called when defender successfully parries - activated abilities that trigger on parry.
 */
function ExecuteCombatAbilitiesOnParry(byRef attackInfo)
	_ExecuteCombatAbilities(attackInfo.GetDefender(), ABILITY_ACTIVATION::ON_PARRY, attackInfo);
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
			var ability := CreateAbility(abilityId);
			if (ability.errortext)
				continue;
			endif

			var result := ability.ExecuteOn(attackInfo);
			abilitiesExecuted++;

			// Single-use abilities: deactivate after first successful execution
			if (result && ability.IsSingleUse() && serializedMonitor.Pid)
				var monitorProcess := GetProcess(serializedMonitor.Pid);
				if (monitorProcess)
					monitorProcess.SendEvent(ABILITY_EVENT_ID::COMPLETE);
				endif
			endif
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
 * DEFENDER-CENTRIC: Executes passive abilities for defender on parry.
 */
function ExecutePassiveAbilitiesOnParry(byRef attackInfo)
	_ExecutePassiveAbilities(attackInfo.GetDefender(), ABILITY_ACTIVATION::ON_PARRY, attackInfo);
endfunction

/**
 * ATTACKER-CENTRIC: Executes passive abilities for attacker on kill.
 */
function ExecutePassiveAbilitiesOnKill(byRef attackInfo)
	_ExecutePassiveAbilities(attackInfo.GetAttacker(), ABILITY_ACTIVATION::ON_KILL, attackInfo);
endfunction

/**
 * ATTACKER-CENTRIC: Executes passive abilities for attacker after damage.
 */
function ExecutePassiveAbilitiesAfterDamage(byRef attackInfo)
	_ExecutePassiveAbilities(attackInfo.GetAttacker(), ABILITY_ACTIVATION::AFTER_DAMAGE, attackInfo);
endfunction

/**
 * ATTACKER-CENTRIC: Executes passive abilities for attacker before damage.
 */
function ExecutePassiveAbilitiesAttackerBeforeDamage(byRef attackInfo)
	_ExecutePassiveAbilities(attackInfo.GetAttacker(), ABILITY_ACTIVATION::BEFORE_DAMAGE, attackInfo);
endfunction

/**
 * DEFENDER-CENTRIC: Executes passive abilities for defender before damage.
 * Used for defender-side passives like damage reduction and crit immunity.
 */
function ExecutePassiveAbilitiesDefenderBeforeDamage(byRef attackInfo)
	_ExecutePassiveAbilities(attackInfo.GetDefender(), ABILITY_ACTIVATION::BEFORE_DAMAGE, attackInfo);
endfunction

/**
 * Executes passive abilities via class hooks.
 * Checks all abilities the mobile HAS (not just activated).
 *
 * @param mobile MobileWrapper - The mobile whose passive abilities to check
 * @param passiveMoment ABILITY_ACTIVATION enum - When the passive triggers
 * @param attackInfo AttackInfo - Combat context passed to ability hooks
 * @return Integer - Number of passive abilities executed
 */
function _ExecutePassiveAbilities(byRef mobile, passiveMoment, byRef attackInfo)
	var abilities := mobile.GetAbilities();

	if (!abilities)
		return 0;
	endif

	var abilitiesExecuted := 0;

	// myhabs stores { level => ability_id }
	foreach level in (abilities.keys())
		var abilityId := CInt(abilities[level]);

		if (!ABILITY_CLASSES.exists(abilityId))
			continue;
		endif

		var ability := CreateAbility(abilityId);

		// Call appropriate OnPassive* hook based on moment
		var triggered := 0;
		case (passiveMoment)
			ABILITY_ACTIVATION::ON_PARRY:
				triggered := ability.OnPassiveOnParry(attackInfo);
			ABILITY_ACTIVATION::ON_KILL:
				triggered := ability.OnPassiveOnKill(attackInfo);
			ABILITY_ACTIVATION::BEFORE_DAMAGE:
				triggered := ability.OnPassiveBeforeDamage(attackInfo);
			ABILITY_ACTIVATION::AFTER_DAMAGE:
				triggered := ability.OnPassiveAfterDamage(attackInfo);
		endcase

		if (triggered)
			abilitiesExecuted++;
		endif
	endforeach

	return abilitiesExecuted;
endfunction
