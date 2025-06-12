// O parâmetro combatInfo deve ser sempre um struct com as informações do que está rolando no combate. Mesmo que contenha apenas 1 variável.
// Feito com struct pra evitar refatoramento e um monte de parametros.
include ":achievements:model/Achievement";
include ":achievements:model/AchievementCombat";
include ":debug:main";
include "include/NPCUtil";

/**
 * AchievementCombat_Advance(attacker, defender, combatInfo)
 *
 * Avança o progresso de achievements de combate para o atacante, baseado nas informações do combate contra o defensor.
 *
 * Parameters
 * attacker: Entidade que realizou o ataque (jogador ou NPC).
 * defender: Entidade que foi atacada (jogador ou NPC).
 * combatInfo: Struct contendo informações do combate (xp, kills, hits, damage, etc).
 */
function AchievementCombat_Advance(attacker, defender, combatInfo)
	var achiev;
	var achievName := defender.GetCategory() ?: NPC_GetNPCConfig(defender.getNPCTemplate()).Category;

	// Se ainda não tem categoria, não é um npc válido.
	if (!achievName) return; endif

	achiev := attacker.GetAchievementProgress(ACHIEVEMENT_COMBAT, achievName) ?: AchievementCombat(achievName);

	achiev.AddXP(combatInfo.xp);
	achiev.AddKills(combatInfo.kills);
	achiev.AddHits(combatInfo.hits);
	achiev.AddDamage(combatInfo.damage);
	DebugMsg(attacker, DEBUG_ACHIEVEMENT, $"Ganho de achievement para a categoria: {achiev.GetName()} ({achiev.GetType()})");
	DebugMsg(attacker, DEBUG_ACHIEVEMENT, $"XP: {CInt(combatInfo.xp)} - Kills: {CInt(combatInfo.kills)} - Hits: {CInt(combatInfo.hits)} - Damage: {CInt(combatInfo.damage)}");
	DebugMsg(attacker, DEBUG_ACHIEVEMENT, $"Achievement (acumulado) -> XP: {achiev.GetXP()} - Level: {achiev.GetLevel()} - Kills: {achiev.GetKills()} - Hits: {achiev.GetHits()} - Damage: {achiev.GetDamage()}");

	attacker.SetAchievementProgress(achiev);
endfunction

function AchievementCheckLevel()
	return 0;
endfunction

