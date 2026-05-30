// O combatInfo deve ser sempre um struct com as informações do que está rolando no combate. Mesmo que contenha apenas 1 variável.
// Foi feito com struct pra evitar refatoramento e um monte de parametros.
include ":mastery:model/MasteryCombat";
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
 * attacker: Objeto do jogador/personagem que está atacando
 * combatInfo: Struct contendo informações do combate (xp, hits, kills, damage, etc).
 *
 * Return value
 * Nenhum (void)
 */
function MasteryCombat_Advance(attacker, combatInfo)
	return; // Desabilitando pra pararem de perguntar...
	
	var key := Hex(attacker.weapon.objtype); // Chave para o índice.
	var mastery := attacker.GetMastery(key) ?: MasteryCombat(key);

	if (HasReachedMasteriesLimit(attacker, mastery))
		DebugMsg(attacker, DEBUG_MASTERY, Message("Limite de maestrias atingido. Para avançar, remova uma maestria existente.", Color(SYSTEM_COLOR::FAIL)));
		return;
	endif

	var earnedXP := CalcXPEarned(combatInfo);
	mastery._SetName(attacker.weapon.desc);
	mastery.AddXP(earnedXP);
	mastery.AddKills(combatInfo.kills);
	mastery.AddHits(combatInfo.hits);
	mastery.AddDamage(combatInfo.damage);
	DebugMsg(attacker, DEBUG_MASTERY, Message($"Ganho de maestria para a arma: {mastery.GetName()} ({mastery.GetKey()})"));
	DebugMsg(attacker, DEBUG_MASTERY, Message($"XP: {CInt(earnedXP)} - Hits: {CInt(combatInfo.hits)} - Kills: {CInt(combatInfo.kills)} - Damage: {CInt(combatInfo.damage)}"));
	DebugMsg(attacker, DEBUG_MASTERY, Message($"Maestria (acumulado) -> XP: {mastery.GetXP()} - Level: {mastery.GetLevel()} - Hits: {mastery.GetHits()} - Kills: {mastery.GetKills()} - Damage: {mastery.GetDamage()}"));

	attacker.SetMastery(mastery);
endfunction

/**
 * HasReachedMasteriesLimit(attacker, mastery)
 * 
 * Esta função determina se um jogador pode adicionar uma nova maestria baseado
 * no limite máximo configurado (MAX_MASTERIES). Se o jogador já possui o número
 * máximo de maestrias, só poderá adicionar se já possuir a maestria específica.
 * 
 * @param attacker O objeto jogador/atacante que está tentando adicionar a maestria
 * @param mastery O objeto maestria que está sendo verificado
 * @return true se o limite foi atingido e a maestria não pode ser adicionada,
 *         false se a maestria pode ser adicionada ou se ainda há espaço disponível
 */
function HasReachedMasteriesLimit(attacker, mastery)
	var propMastery := attacker.GetMasteryProp();
	if (propMastery.size() >= MAX_MASTERIES)
		if (mastery.GetKey() in propMastery.keys())
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
function CalcXPEarned(combatInfo)
	// TODO: Implementar lógica de cálculo de XP mais complexa, se necessário.
	return combatInfo.damage;
endfunction

function MasteryCheckLevel(xp)
	return 0;
endfunction