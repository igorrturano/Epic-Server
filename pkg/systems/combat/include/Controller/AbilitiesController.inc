/***********************************
 *
 * habilidades.inc
 *
 * Created: 29/12/2025 08:51AM
 *
 * Responsável por:
 * Gerenciar a aplicar os cálculos das habilidades de combate.
 *
 **********************************/

include ":abilities:models/Ability";

function ExecuteCombatAbilitiesBeforeHit(byRef attackInfo)
	var attackerAbilites := attackInfo.GetAttacker().GetActiveAbilites()[ABILITY_ACTIVATION::BEFORE_HIT];
	var defenderAbilities := array{};
	var exec := 0;


	if (TryToExecuteHab_AtaqueInfalivel(attackInfo)) exec++; endif

	if (exec > 0)
		return true;
	endif

	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_HAB, Message($"Nenhuma habilidade ativada antes do hit.", Color(SYSTEM_COLOR::FAIL)));
	return false;
endfunction

function ExecuteCombatAbilitiesBeforeDamage(byRef attackInfo)
	var exec := 0;

	if (TryToExecuteHab_Martir(attackInfo)) exec++; endif
	if (TryToExecuteHab_MantoRothen(attackInfo)) exec++; endif
	if (TryToExecuteHab_KaniNoSutairu(attackInfo)) exec++; endif
	if (TryToExecuteHab_Exoesqueleto(attackInfo)) exec++; endif
	if (TryToExecuteHab_EspecialistaEscudos(attackInfo)) exec++; endif
	if (TryToExecuteHab_Furia(attackInfo)) exec++; endif
	if (TryToExecuteHab_TaticasArcanas(attackInfo)) exec++; endif
	if (TryToExecuteHab_ResilienciaBarbara(attackInfo)) exec++; endif
	if (TryToExecuteHab_VinculoElemental(attackInfo)) exec++; endif
	if (TryToExecuteHab_Backstab(attackInfo)) exec++; endif
	if (TryToExecuteHab_Charge(attackInfo)) exec++; endif

	if (exec > 0)
		return true;
	endif

	DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_HAB, Message($"Nenhuma habilidade ativada antes de receber o dano.", Color(SYSTEM_COLOR::FAIL)));

	return false;
endfunction

/**
 * CheckAttackerHabsOnHit(attacker, defender, hitchance, rolldice)
 *
 * Purpose
 * Função responsável por verificar e aplicar as habilidades do atacante ao atingir um defensor.
 *
 * Parameters
 * attacker: Entidade atacante (objeto ou identificador do atacante).
 * defender: Entidade defensora (objeto ou identificador do defensor).
 * attackInfo: Struct contendo as informações necessárias que as habs precisam tratar.
 *
 * Return
 * Retorna 1 após a execução das verificações e aplicações das habilidades.
 *
 */
function ExecuteCombatAbilitiesOnHit(byRef attackInfo)
	var exec := 0;

	// if (TryToExecuteHab_ArmaduraArcana(attackInfo)) exec++; endif
	// if (TryToExecuteHab_Ainalaith(attackInfo)) exec++; endif
	// if (TryToExecuteHab_DeathDance(attackInfo)) exec++; endif
	// if (TryToExecuteHab_DisparoMistico(attackInfo)) exec++; endif
	// if (TryToExecuteHab_DesviarProjetil(attackInfo)) exec++; endif
	// if (TryToExecuteHab_DesviarDisparos(attackInfo)) exec++; endif
	// if (TryToExecuteHab_FerocidadeEnsanguentada(attackInfo)) exec++; endif
	// if (TryToExecuteHab_Evasao(attackInfo)) exec++; endif
	// if (TryToExecuteHab_ContraAtaque(attackInfo)) exec++; endif
	// if (TryToExecuteHab_Rebuke(attackInfo)) exec++; endif

	if (exec > 0)
		return true;
	endif

	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_HAB, Message($"Nenhuma habilidade ativada no hit.", Color(SYSTEM_COLOR::FAIL)));

	return false;
endfunction

function ExecuteCombatAbilitiesAfterDamage(byRef attackInfo)
	var exec := 0;

	if (TryToExecuteHab_RecuperacaoSalvadora(attackInfo)) exec++; endif
	// if (TryToExecuteHab_Vinganca(attackInfo)) exec++; endif
	// if (TryToExecuteHab_ChainOfPain(attackInfo)) exec++; endif

	if (exec > 0)
		return true;
	endif

	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_HAB, Message($"Nenhuma habilidade ativada após causar o dano.", Color(SYSTEM_COLOR::FAIL)));
	DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_HAB, Message($"Nenhuma habilidade ativada após receber o dano.", Color(SYSTEM_COLOR::FAIL)));
	return false;
endfunction

function ExecuteCombatAbilitiesOnMiss(byRef attackInfo)
	var exec := 0;

	if (TryToExecuteHab_MestreDuelista(attackInfo)) exec++; endif

	if (exec > 0)
		return true;
	endif

	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_HAB, Message($"Nenhuma habilidade ativada no miss.", Color(SYSTEM_COLOR::FAIL)));
	return false;
endfunction
