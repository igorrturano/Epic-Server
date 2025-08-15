// O parâmetro combatInfo deve ser sempre um struct com as informações do que está rolando no combate. Mesmo que contenha apenas 1 variável.
// Feito com struct pra evitar refatoramento e um monte de parametros.
include ":achievements:model/Achievement";
// include ":achievements:model/AchievementCombat";
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
	if (attacker.npctemplate) return; endif // Só player ganha Achievement!!! NPC metido!!!

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
	var key := defender.npctemplate;
	if (!key) // Se não tem template, não é um npc válido.
		DebugMsg(attacker, DEBUG_ACHIEVEMENT, $"Defensor não é um NPC válido ({key}). Ignorando avanço de achievement para TEMPLATE.");
		return;
	endif

	AchievementAdvance(ACHIEVEMENT_COMBAT, attacker, key, ACHIEVEMENT_NAME_KILLS, combatInfo.kills);
	AchievementAdvance(ACHIEVEMENT_COMBAT, attacker, key, ACHIEVEMENT_NAME_HITS, combatInfo.hits);
	AchievementAdvance(ACHIEVEMENT_COMBAT, attacker, key, ACHIEVEMENT_NAME_DAMAGE_DONE, combatInfo.damage);

	AchievementSetRefName(ACHIEVEMENT_COMBAT, attacker, key, ACHIEVEMENT_NAME_KILLS, defender.GetParsedTemplate());
	AchievementSetRefName(ACHIEVEMENT_COMBAT, attacker, key, ACHIEVEMENT_NAME_HITS, defender.GetParsedTemplate());
	AchievementSetRefName(ACHIEVEMENT_COMBAT, attacker, key, ACHIEVEMENT_NAME_DAMAGE_DONE, defender.GetParsedTemplate());
endfunction

function AchievementCombat_Advance_NPCCategory(attacker, defender, combatInfo)
	var achiev;
	var key := defender.GetCategory() ?: NPC_GetNPCConfig(defender.npctemplate).Category;
	if (!key) // Se ainda não tem categoria, não é um npc válido.
		DebugMsg(attacker, DEBUG_ACHIEVEMENT, $"Defensor não é um NPC válido ({key}). Ignorando avanço de achievement para CATEGORIA.");
		return;
	endif

	AchievementAdvance(ACHIEVEMENT_COMBAT, attacker, key, ACHIEVEMENT_NAME_KILLS, combatInfo.kills);
	AchievementAdvance(ACHIEVEMENT_COMBAT, attacker, key, ACHIEVEMENT_NAME_HITS, combatInfo.hits);
	AchievementAdvance(ACHIEVEMENT_COMBAT, attacker, key, ACHIEVEMENT_NAME_DAMAGE_DONE, combatInfo.damage);

	AchievementSetRefName(ACHIEVEMENT_COMBAT, attacker, key, ACHIEVEMENT_NAME_KILLS, key);
	AchievementSetRefName(ACHIEVEMENT_COMBAT, attacker, key, ACHIEVEMENT_NAME_HITS, key);
	AchievementSetRefName(ACHIEVEMENT_COMBAT, attacker, key, ACHIEVEMENT_NAME_DAMAGE_DONE, key);
endfunction

function AchievementCombat_Advance_Weapon(attacker, defender, combatInfo)
	var achiev;
	var key := CStr(Hex(attacker.weapon.objtype));
	if (!key) // Se não tem arma, não tem achievement de arma. DUUUUUUUUUH. E
		DebugMsg(attacker, DEBUG_ACHIEVEMENT, "Arma empunhada não é uma arma válida. Ignorando avanço de achievement.");
		return;
	endif

	AchievementAdvance(ACHIEVEMENT_COMBAT, attacker, key, ACHIEVEMENT_NAME_KILLS, combatInfo.kills);
	AchievementAdvance(ACHIEVEMENT_COMBAT, attacker, key, ACHIEVEMENT_NAME_HITS, combatInfo.hits);
	AchievementAdvance(ACHIEVEMENT_COMBAT, attacker, key, ACHIEVEMENT_NAME_DAMAGE_DONE, combatInfo.damage);

	// TODO: Pensar em uma solução melhor para atualizar o nome junto com o avanço do achievement
	AchievementSetRefName(ACHIEVEMENT_COMBAT, attacker, key, ACHIEVEMENT_NAME_KILLS, attacker.weapon.desc);
	AchievementSetRefName(ACHIEVEMENT_COMBAT, attacker, key, ACHIEVEMENT_NAME_HITS, attacker.weapon.desc);
	AchievementSetRefName(ACHIEVEMENT_COMBAT, attacker, key, ACHIEVEMENT_NAME_DAMAGE_DONE, attacker.weapon.desc);
endfunction

function AchievementAdvance(type, who, key, name, value)
	value := CInt(value);
	if (value < 1) return; endif

	var achiev := who.GetAchievement(type, who, key, name);

	achiev.IncrementValue(value);

	DebugMsg(who, DEBUG_ACHIEVEMENT, $"Ganho de achievement para {achiev.GetName()} ({type}): {value}");
	DebugMsg(who, DEBUG_ACHIEVEMENT, $"Progresso de achievement para {achiev.GetName()}: {achiev.ToString()}");

	who.SetAchievement(type, who, achiev);
endfunction

function AchievementSetRefName(type, who, key, name, refName)
	var achiev := who.GetAchievement(type, who, key, name);
	achiev.SetRefName(refName);
	who.SetAchievement(type, who, achiev);
endfunction