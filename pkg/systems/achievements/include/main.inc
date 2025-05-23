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

include ":debug:main";
include "include/NPCUtil";

function AchievementAdvance(attacker, defender, combatInfo)
	var prop := attacker.GetAchievementProp();
	var npcCategory := defender.GetCategory();

	if (!npcCategory) // Se não tem categoria, ou é corpo, ou não é um npc válido.
		npcCategory := NPC_GetNPCConfig(defender.getNPCTemplate()).Category; // Tenta ver se era corpo e tenta buscar pelo template.
		if (!npcCategory) // Se ainda não tem categoria, não é um npc válido.
			return;
		endif
	endif

	if (!prop[npcCategory]) // Se ainda não tem pra categoria do npc, cria o struct.
		prop[npcCategory] := struct{
			xp := 0,
			level := 0,
			kills := 0,
			hits := 0
		};
	endif

	prop[npcCategory].xp += CInt(combatInfo.xp);
	prop[npcCategory].level := AchievementCheckLevel(prop[npcCategory].xp);
	prop[npcCategory].kills += CInt(combatInfo.kills);
	prop[npcCategory].hits += CInt(combatInfo.hits);
	DebugMsg(attacker, DEBUG_ACHIEVEMENT, $"Ganho de achievement para a categoria: {npcCategory}");
	DebugMsg(attacker, DEBUG_ACHIEVEMENT, $"XP: {CInt(combatInfo.xp)} - Kills: {CInt(combatInfo.kills)} - Hits: {CInt(combatInfo.hits)}");
	DebugMsg(attacker, DEBUG_ACHIEVEMENT, $"Achievement (acumulado) -> XP: {prop[npcCategory].xp} - Level: {prop[npcCategory].level} - Kills: {prop[npcCategory].kills} - Hits: {prop[npcCategory].hits}");

	attacker.SetAchievementProp(prop);
endfunction

function AchievementCheckLevel(attacker)
	return 0;
endfunction