// Essa função deve ser chamada no npcdeath()
// O combatInfo deve ser sempre um struct com as informações do que está rolando no combate. Mesmo que contenha apenas 1 variável.
// Foi feito com struct pra evitar refatoramento e um monte de parametros.
use cfgfile;

include ":achievements:model/Achievement";
include ":achievements:model/Combat";
include ":debug:main";
include "include/NPCUtil";

function AchievementCombat_Advance(attacker, defender, combatInfo)
	var achiev;
	var achievName := defender.GetCategory();

	if (!achievName) // Se não tem categoria, ou é corpo, ou não é um npc válido.
		achievName := NPC_GetNPCConfig(defender.getNPCTemplate()).Category; // Tenta ver se era corpo e tenta buscar pelo template.
		if (!achievName) // Se ainda não tem categoria, não é um npc válido.
			return;
		endif
	endif

	achiev := attacker.GetAchievementProgress(achievName);

	if (!achiev)
		achiev := Combat(achievName, 0, 0, 0, 0, 0); // Cria um novo achievement de combate.
	endif

	Advance(achiev, combatInfo);
	DebugMsg(attacker, DEBUG_ACHIEVEMENT, $"Ganho de achievement para a categoria: {achiev.GetName()} ({achiev.GetType()})");
	DebugMsg(attacker, DEBUG_ACHIEVEMENT, $"XP: {CInt(combatInfo.xp)} - Kills: {CInt(combatInfo.kills)} - Hits: {CInt(combatInfo.hits)} - Damage: {CInt(combatInfo.damage)}");
	DebugMsg(attacker, DEBUG_ACHIEVEMENT, $"Achievement (acumulado) -> XP: {achiev.GetXP()} - Level: {achiev.GetLevel()} - Kills: {achiev.GetKills()} - Hits: {achiev.GetHits()} - Damage: {achiev.GetDamage()}");

	attacker.SetAchievementProgress(achiev);
endfunction

function Advance(byRef achiev, combatInfo)
	achiev.AddXP(combatInfo.xp);
	achiev.AddKills(combatInfo.kills);
	achiev.AddHits(combatInfo.hits);
	achiev.AddDamage(combatInfo.damage);
endfunction

function CheckLevel()
	return 0;
endfunction

