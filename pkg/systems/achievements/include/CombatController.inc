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
	AchievementCombat_Advance_NPCTemplate(attacker, defender, combatInfo);
	AchievementCombat_Advance_NPCCategory(attacker, defender, combatInfo);

	AchievementCombat_Advance_Weapon(attacker, defender, combatInfo);
endfunction

function AchievementCombat_Advance_NPCTemplate(attacker, defender, combatInfo)
	var achiev;
	var key := defender.getNPCTemplate();
	if (!key) return; endif // Se não tem template, não é um npc válido.

	achiev := attacker.GetAchievementProgress(ACHIEVEMENT_COMBAT, key) ?: AchievementCombat(key);

	achiev.SetName(NPC_ParseTemplateName(key).template);

	achiev.AddXP(AchievementXPGain());
	achiev.AddData(ACHIEVDATA_KILLS, combatInfo.kills);
	achiev.AddData(ACHIEVDATA_HITS, combatInfo.hits);
	achiev.AddData(ACHIEVDATA_DAMAGE_DONE, combatInfo.damage);

	DebugMsg(attacker, DEBUG_ACHIEVEMENT, $"Ganho de achievement para a categoria: {achiev.GetName()} ({achiev.GetType()})");
	DebugMsg(attacker, DEBUG_ACHIEVEMENT, $"XP: {CInt(combatInfo.xp)} - Kills: {CInt(combatInfo.kills)} - Hits: {CInt(combatInfo.hits)} - Damage: {CInt(combatInfo.damage)}");
	DebugMsg(attacker, DEBUG_ACHIEVEMENT, $"Achievement (progresso atual) -> XP: {achiev.GetXP()} - Level: {achiev.GetLevel()} - Kills: {achiev.GetData(ACHIEVDATA_KILLS)} - Hits: {achiev.GetData(ACHIEVDATA_HITS)} - Damage: {achiev.GetData(ACHIEVDATA_DAMAGE_DONE)}");

	attacker.SetAchievementProgress(achiev);
endfunction

function AchievementCombat_Advance_NPCCategory(attacker, defender, combatInfo)
	var achiev;
	var key := defender.GetCategory() ?: NPC_GetNPCConfig(defender.getNPCTemplate()).Category;
	if (!key) return; endif // Se ainda não tem categoria, não é um npc válido.

	achiev := attacker.GetAchievementProgress(ACHIEVEMENT_COMBAT, key) ?: AchievementCombat(key);

	achiev.AddXP(AchievementXPGain());

	achiev.AddData(ACHIEVDATA_KILLS, combatInfo.kills);
	achiev.AddData(ACHIEVDATA_HITS, combatInfo.hits);
	achiev.AddData(ACHIEVDATA_DAMAGE_DONE, combatInfo.damage);
	
	DebugMsg(attacker, DEBUG_ACHIEVEMENT, $"Ganho de achievement para a categoria: {achiev.GetName()} ({achiev.GetType()})");
	DebugMsg(attacker, DEBUG_ACHIEVEMENT, $"XP: {CInt(combatInfo.xp)} - Kills: {CInt(combatInfo.kills)} - Hits: {CInt(combatInfo.hits)} - Damage: {CInt(combatInfo.damage)}");
	DebugMsg(attacker, DEBUG_ACHIEVEMENT, $"Achievement (progresso atual) -> XP: {achiev.GetXP()} - Level: {achiev.GetLevel()} - Kills: {achiev.GetData(ACHIEVDATA_KILLS)} - Hits: {achiev.GetData(ACHIEVDATA_HITS)} - Damage: {achiev.GetData(ACHIEVDATA_DAMAGE_DONE)}");

	attacker.SetAchievementProgress(achiev);
endfunction

function AchievementCombat_Advance_Weapon(attacker, defender, combatInfo)

endfunction

function AchievementXPGain()
	return 0;
endfunction

function AchievementCheckLevel()
	return 0;
endfunction

