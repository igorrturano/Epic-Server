include ":wrappers:Objects/Model/MobileWrapper";
include ":mastery:models/Mastery";
include ":debug:Controller/DebugController";

const MAX_MASTERIES := 3; // Limite máximo de maestrias que um personagem pode ter.

// TODO: Criar 3 níveis que progridem reduzindo o cooldown de ativação da habilidade
	// Cooldown de 150s.
	// Reduz -30s por nível.
// TODO: Definir critérios de progressão de maestria para cada nível.


/**
 * MasteryCombat_Advance(attacker, combatInfo)
 *
 * Avança o sistema de maestria em combate para um atacante, atualizando as estatísticas
 * de experiência, acertos, mortes e dano baseadas nas informações de combate fornecidas.
 * A função também registra mensagens de debug com as informações de progresso da maestria.
 *
 * Parameters
 * attacker: MobileWrapper do jogador/personagem que vai avancar a maestria.
 * combatInfo: Dictionary contendo informações do que vai upar (hits, kills, damage, etc).
 * 				Usar o ENUM MasteryCombatAtributes para as chaves do dicionário.
 * 				Exemplo: combatInfo[MasteryCombatAtributes::Damage] := 15;
 */
function MasteryCombat_Advance(player, advanceInfo)
	var weapon := player.GetEquipBySlot(WEAPON);
	var mastery := Mastery(weapon.GetObjRef());
	var playerMasteries := player.GetMasteries();

	if (HasReachedMasteriesLimit(mastery, playerMasteries))
		DebugMsg(player, DEBUG_MASTERY, Message("Limite de maestrias atingido. Para avançar, remova uma maestria existente.", Color(SYSTEM_COLOR::FAIL)));
		return;
	endif

	var earnedXP := CalcXPEarned(advanceInfo);
	mastery._SetName(weapon.GetName());
	mastery.AddXP(earnedXP);
	AdvanceMastery(player, playerMasteries, mastery, advanceInfo);

	DebugMsg(player, DEBUG_MASTERY, Message($"Ganho de maestria para a arma: {mastery.GetName()} ({mastery.GetRef()})"));
	DebugAdvanced(player, advanceInfo);
	DebugMsg(player, DEBUG_MASTERY, Message($"Maestria (acumulado) -> XP: {mastery.GetXP()} - Level: {mastery.GetLevel()} - Hits: {mastery.GetHits()} - Kills: {mastery.GetKills()} - Damage: {mastery.GetDamage()}"));

endfunction

function HasReachedMasteriesLimit(mastery, playerMasteries)
	if (playerMasteries.size() >= MAX_MASTERIES)
		if (mastery.GetRef() in playerMasteries.keys())
			return false;
		endif

		return true;
	endif

	return false;
endfunction

/**
 * CalcXPEarned(combatInfo)
 *
 * Calcula a quantidade de XP (experiência) ganha com base nas informações de combate.
 * 
 * @param combatInfo Objeto contendo informações do combate, incluindo o dano causado
 * @return Integer A quantidade de XP ganha, equivalente ao dano causado
 * 
 * @note Esta função atualmente retorna o valor do dano como XP 1:1
 *       ganho, mas pode ser ajustada no futuro para incluir outros fatores como
 *       acertos, mortes, etc.
 */
function CalcXPEarned(advanceInfo)
	// TODO: Implementar lógica de cálculo de XP mais complexa, se necessário.
	return CInt(advanceInfo[MasteryCombatAtributes::Damage]);
endfunction

function MasteryCheckLevel(xp)
	xp := xp; // Só pra remover o warning

	return 0;
endfunction

function AdvanceMastery(player, playerMasteries, mastery, advanceInfo)
	foreach attribute in (advanceInfo.keys())
		mastery.IncreaseAttribute(attribute, advanceInfo[attribute]);
	endforeach

	playerMasteries[mastery.GetRef()] := mastery;
	player.SetMasteries(playerMasteries);
endfunction

function DebugAdvanced(who, advanceInfo)
	foreach attribute in (advanceInfo.keys())
		DebugMsg(who.GetObjRef(), DEBUG_MASTERY, Message($"Avançou {CInt(advanceInfo[attribute])} pontos de {attribute}"));
	endforeach
endfunction
