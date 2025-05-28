// O combatInfo deve ser sempre um struct com as informações do que está rolando no combate. Mesmo que contenha apenas 1 variável.
// Foi feito com struct pra evitar refatoramento e um monte de parametros.
include ":mastery:model/MasteryCombat";
include ":debug:main";

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
	if (HasReachedMasteriesLimit(attacker))
		DebugMsg(attacker, DEBUG_MASTERY, "Limite de maestrias atingido. Para avançar, remova uma maestria existente.");
		return;
	endif
	
	var key := Hex(attacker.weapon.objtype); // Chave para o índice.
	var mastery := attacker.GetMastery(key) ?: MasteryCombat(key);

	mastery._SetName(attacker.weapon.desc);
	mastery.AddXP(combatInfo.xp);
	mastery.AddKills(combatInfo.kills);
	mastery.AddHits(combatInfo.hits);
	mastery.AddDamage(combatInfo.damage);
	DebugMsg(attacker, DEBUG_MASTERY, $"Ganho de maestria para a arma: {mastery.GetName()} ({mastery.GetKey()})");
	DebugMsg(attacker, DEBUG_MASTERY, $"XP: {CInt(combatInfo.xp)} - Hits: {CInt(combatInfo.hits)} - Kills: {CInt(combatInfo.kills)} - Damage: {CInt(combatInfo.damage)}");
	DebugMsg(attacker, DEBUG_MASTERY, $"Maestria (acumulado) -> XP: {mastery.GetXP()} - Level: {mastery.GetLevel()} - Hits: {mastery.GetHits()} - Kills: {mastery.GetKills()} - Damage: {mastery.GetDamage()}");

	attacker.SetMastery(mastery);
endfunction

function HasReachedMasteriesLimit(attacker)
	var masteriesNames := attacker.GetMasteryProp();
	if (masteriesNames.size() >= 3)
		return true;
	endif
	return false;
endfunction

function MasteryCheckLevel(xp)
	return 0;
endfunction