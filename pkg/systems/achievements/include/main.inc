// TODO: Controlar uma PROP no player (struct) com os dados do achievement:
	// struct achievement
		// dict npc.category
			// .level
			// .xp
			// .kills
			// (ver o que mais pode ser interessante adicionar aqui para ter controle total do achievement)

// TODO: Essa função deve ser chamada no npcdeath()

// O combatInfo deve ser sempre um struct com as informações do que está rolando no combate. Mesmo que contenha apenas 1 variável.
// Foi feito com struct pra evitar refatoramento e um monte de parametros.
use cfgfile;

include ":achievements:model/Achievement";
include ":debug:main";
include "include/NPCUtil";

function AchievementAdvance(attacker, defender, combatInfo)
	var achiev;
	var npcCategory := defender.GetCategory();

	if (!npcCategory) // Se não tem categoria, ou é corpo, ou não é um npc válido.
		npcCategory := NPC_GetNPCConfig(defender.getNPCTemplate()).Category; // Tenta ver se era corpo e tenta buscar pelo template.
		if (!npcCategory) // Se ainda não tem categoria, não é um npc válido.
			return;
		endif
	endif

	achiev := attacker.GetAchievementProgress(npcCategory);
	achiev.SetXP(achiev.GetXP() + CInt(combatInfo.xp));
	achiev.SetKills(achiev.GetKills() + CInt(combatInfo.kills));
	achiev.SetHits(achiev.GetHits() + CInt(combatInfo.hits));
	achiev.SetDamage(achiev.GetDamage() + CInt(combatInfo.damage));
	DebugMsg(attacker, DEBUG_ACHIEVEMENT, $"Ganho de achievement para a categoria: {achiev.GetName()}");
	DebugMsg(attacker, DEBUG_ACHIEVEMENT, $"XP: {CInt(combatInfo.xp)} - Kills: {CInt(combatInfo.kills)} - Hits: {CInt(combatInfo.hits)} - Damage: {CInt(combatInfo.damage)}");
	DebugMsg(attacker, DEBUG_ACHIEVEMENT, $"Achievement (acumulado) -> XP: {achiev.GetXP()} - Level: {achiev.GetLevel()} - Kills: {achiev.GetKills()} - Hits: {achiev.GetHits()} - Damage: {achiev.GetDamage()}");

	attacker.SetAchievementProgress(achiev);
endfunction

function AchievementCheckLevel(attacker)
	return 0;
endfunction