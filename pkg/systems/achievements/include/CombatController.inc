// O parâmetro combatInfo deve ser sempre um struct com as informações do que está rolando no combate. Mesmo que contenha apenas 1 variável.
// Feito com struct pra evitar refatoramento e um monte de parametros.
include ":achievements:model/Achievement";
include ":achievements:model/AchievementCombat";
include ":debug:Controller/DebugController";
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

/**
 * AchievementCombat_Advance_NPCTemplate(attacker, defender, combatInfo)
 *
 * Avança o progresso de achievements de combate para o atacante, baseado no template do NPC defensor.
 *
 * Parameters
 * attacker: Entidade que realizou o ataque (jogador ou NPC).
 * defender: Entidade que foi atacada (jogador ou NPC).
 * combatInfo: Struct contendo informações do combate (xp, kills, hits, damage, etc).
 */
function AchievementCombat_Advance_NPCTemplate(attacker, defender, combatInfo)
	var achiev;
	var key := defender.getNPCTemplate();
	if (!key) return; endif // Se não tem template, não é um npc válido.

	achiev := attacker.GetAchievementProgress(ACHIEVEMENT_COMBAT, key) ?: AchievementCombat(attacker, key);

	achiev._SetName(NPC_ParseTemplateName(key).template);

	AchievementAdvance_XP(achiev, combatInfo);
	AchievementAdvance_Kills(achiev, combatInfo);
	AchievementAdvance_Hits(achiev, combatInfo);
	AchievementAdvance_Damage(achiev, combatInfo);

	DebugMsg(attacker, DEBUG_ACHIEVEMENT, $"Ganho de achievement para a categoria: {achiev.GetName()} ({ACHIEVEMENT_COMBAT})");
	DebugMsg(attacker, DEBUG_ACHIEVEMENT, $"XP: {CInt(0)} - Kills: {CInt(combatInfo.kills)} - Hits: {CInt(combatInfo.hits)} - Damage: {CInt(combatInfo.damage)}");
	DebugMsg(attacker, DEBUG_ACHIEVEMENT, $"Achievement (progresso atual) -> Level: {achiev.GetLevel()} - XP: {achiev.GetXP()} - Kills: {achiev.GetData(ACHIEVDATA_KILLS)} - Hits: {achiev.GetData(ACHIEVDATA_HITS)} - Damage: {achiev.GetData(ACHIEVDATA_DAMAGE_DONE)}");

	attacker.SetAchievementProgress(achiev);
endfunction

function AchievementCombat_Advance_NPCCategory(attacker, defender, combatInfo)
	var achiev;
	var key := defender.GetCategory() ?: NPC_GetNPCConfig(defender.getNPCTemplate()).Category;
	if (!key) return; endif // Se ainda não tem categoria, não é um npc válido.

	achiev := attacker.GetAchievementProgress(ACHIEVEMENT_COMBAT, key) ?: AchievementCombat(attacker, key);

	AchievementAdvance_XP(achiev, combatInfo);
	AchievementAdvance_Kills(achiev, combatInfo);
	AchievementAdvance_Hits(achiev, combatInfo);
	AchievementAdvance_Damage(achiev, combatInfo);

	DebugMsg(attacker, DEBUG_ACHIEVEMENT, $"Ganho de achievement para a categoria: {achiev.GetName()} ({achiev.GetType()})");
	DebugMsg(attacker, DEBUG_ACHIEVEMENT, $"XP: {CInt(0)} - Kills: {CInt(combatInfo.kills)} - Hits: {CInt(combatInfo.hits)} - Damage: {CInt(combatInfo.damage)}");
	DebugMsg(attacker, DEBUG_ACHIEVEMENT, $"Achievement (progresso atual) -> Level: {achiev.GetLevel()} - XP: {achiev.GetXP()} - Kills: {achiev.GetData(ACHIEVDATA_KILLS)} - Hits: {achiev.GetData(ACHIEVDATA_HITS)} - Damage: {achiev.GetData(ACHIEVDATA_DAMAGE_DONE)}");

	attacker.SetAchievementProgress(achiev);
endfunction

function AchievementCombat_Advance_Weapon(attacker, defender, combatInfo)
	var achiev;
	var key := CStr(Hex(attacker.weapon.objtype));
	if (!key) return; endif // Se não tem arma, não tem achievement de arma.

	achiev := attacker.GetAchievementProgress(ACHIEVEMENT_COMBAT, key) ?: AchievementCombat(attacker,key);

	achiev._SetName(attacker.weapon.desc);
	
	AchievementAdvance_XP(achiev, combatInfo);
	AchievementAdvance_Kills(achiev, combatInfo);
	AchievementAdvance_Hits(achiev, combatInfo);
	AchievementAdvance_Damage(achiev, combatInfo);

	DebugMsg(attacker, DEBUG_ACHIEVEMENT, $"Ganho de achievement para a categoria: {achiev.GetName()} ({achiev.GetType()})");
	DebugMsg(attacker, DEBUG_ACHIEVEMENT, $"XP: {CInt(0)} - Kills: {CInt(combatInfo.kills)} - Hits: {CInt(combatInfo.hits)} - Damage: {CInt(combatInfo.damage)}");
	DebugMsg(attacker, DEBUG_ACHIEVEMENT, $"Achievement (progresso atual) -> Level: {achiev.GetLevel()} - XP: {achiev.GetXP()} - Kills: {achiev.GetData(ACHIEVDATA_KILLS)} - Hits: {achiev.GetData(ACHIEVDATA_HITS)} - Damage: {achiev.GetData(ACHIEVDATA_DAMAGE_DONE)}");

	attacker.SetAchievementProgress(achiev);
endfunction

function AchievementAdvance_XP(byRef achiev, combatInfo)
	// TODO: Implementar lógica de ganho de XP para achievements de combate.
	achiev.AddXP(AchievementXPCalc(achiev));

	if (AchievementAdvance_Level(achiev))
		achiev.AddLevel();
	endif
endfunction

function AchievementXPCalc(achiev)
	// TODO: Implementar lógica de cálculo de XP para achievements de combate.
	return 0;
endfunction

function AchievementAdvance_Level(byRef achiev)
	// TODO: Implementar lógica de avanço de nível de achievement.
	return false;
endfunction

function AchievementAdvance_Hits(byRef achiev, combatInfo)
	achiev.AddData(ACHIEVDATA_HITS, combatInfo.hits);
endfunction

function AchievementAdvance_Kills(byRef achiev, combatInfo)
	achiev.AddData(ACHIEVDATA_KILLS, combatInfo.kills);
endfunction

function AchievementAdvance_Damage(byRef achiev, combatInfo)
	achiev.AddData(ACHIEVDATA_DAMAGE_DONE, combatInfo.damage);
endfunction