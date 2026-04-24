// O parâmetro combatInfo deve ser sempre um struct com as informações do que está rolando no combate. Mesmo que contenha apenas 1 variável.
// Feito com struct pra evitar refatoramento e um monte de parametros.
include ":achievements:models/Achievement";
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
	// if (attacker.npctemplate) return; endif // Só player ganha Achievement!!! NPC metido!!!

	// AchievementCombat_Advance_NPCTemplate(attacker, defender, combatInfo);
	// AchievementCombat_Advance_NPCCategory(attacker, defender, combatInfo);

	// AchievementCombat_Advance_Weapon(attacker, defender, combatInfo);
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
	// var achiev;
	// var key := defender.npctemplate;
	// if (!key) // Se não tem template, não é um npc válido.
	// 	DebugMsg(attacker, DEBUG_ACHIEVEMENT, Message($"Defensor não é um NPC válido ({key}). Ignorando avanço de achievement para TEMPLATE."));
	// 	return;
	// endif

	// AchievementAdvance(ACHIEVEMENT_COMBAT, attacker, key, ACHIEVEMENT_COMBAT_CATEGORY::KILLS, combatInfo.kills, defender.GetParsedTemplate());
	// AchievementAdvance(ACHIEVEMENT_COMBAT, attacker, key, ACHIEVEMENT_COMBAT_CATEGORY::HITS, combatInfo.hits, defender.GetParsedTemplate());
	// AchievementAdvance(ACHIEVEMENT_COMBAT, attacker, key, ACHIEVEMENT_COMBAT_CATEGORY::DAMAGE_DONE, combatInfo.damage, defender.GetParsedTemplate());
endfunction

function AchievementCombat_Advance_NPCCategory(attacker, defender, combatInfo)
	// var achiev;
	// var key := defender.GetEnvironment() ?: NPC_GetNPCConfig(defender.npctemplate).Environment;
	// if (!key) // Se ainda não tem categoria, não é um npc válido.
	// 	DebugMsg(attacker, DEBUG_ACHIEVEMENT, Message($"Defensor não é um NPC válido ({key}). Ignorando avanço de achievement para CATEGORIA."));
	// 	return;
	// endif

	// AchievementAdvance(ACHIEVEMENT_COMBAT, attacker, key, ACHIEVEMENT_COMBAT_CATEGORY::KILLS, combatInfo.kills, key);
	// AchievementAdvance(ACHIEVEMENT_COMBAT, attacker, key, ACHIEVEMENT_COMBAT_CATEGORY::HITS, combatInfo.hits, key);
	// AchievementAdvance(ACHIEVEMENT_COMBAT, attacker, key, ACHIEVEMENT_COMBAT_CATEGORY::DAMAGE_DONE, combatInfo.damage, key);
endfunction

function AchievementCombat_Advance_Weapon(attacker, defender, combatInfo)
	// var achiev;
	// var key := CStr(Hex(attacker.weapon.objtype));
	// if (!key) // Se não tem arma, não tem achievement de arma. DUUUUUUUUUH.
	// 	DebugMsg(attacker, DEBUG_ACHIEVEMENT, Message("Arma empunhada não é uma arma válida. Ignorando avanço de achievement."));
	// 	return;
	// endif

	// AchievementAdvance(ACHIEVEMENT_COMBAT, attacker, key, ACHIEVEMENT_COMBAT_CATEGORY::KILLS, combatInfo.kills, attacker.weapon.desc);
	// AchievementAdvance(ACHIEVEMENT_COMBAT, attacker, key, ACHIEVEMENT_COMBAT_CATEGORY::HITS, combatInfo.hits, attacker.weapon.desc);
	// AchievementAdvance(ACHIEVEMENT_COMBAT, attacker, key, ACHIEVEMENT_COMBAT_CATEGORY::DAMAGE_DONE, combatInfo.damage, attacker.weapon.desc);
endfunction

function AchievementAdvance(type, who, key, name, value, refName := false)
	value := CInt(value);
	if (value < 1) return; endif

	var achiev := who.GetAchievement(type, key, name);

	achiev.IncrementValue(value);

	if (refName) achiev.SetRefName(refName); endif

	DebugMsg(who, DEBUG_ACHIEVEMENT, Message($"Ganho de achievement para {achiev.GetName()} ({type}): {value}"));
	DebugMsg(who, DEBUG_ACHIEVEMENT, Message($"Progresso atual de {achiev.GetName()}: {achiev.ToString()}"));

	who.SetAchievement(type, who, achiev);
endfunction