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

include ":achievements:model/achievement";
include ":debug:main";
include "include/NPCUtil";

function AchievementAdvance(attacker, defender, combatInfo)
	var prop := attacker.GetAchievementProgress();
	var npcCategory := defender.GetCategory();

	if (!npcCategory) // Se não tem categoria, ou é corpo, ou não é um npc válido.
		npcCategory := NPC_GetNPCConfig(defender.getNPCTemplate()).Category; // Tenta ver se era corpo e tenta buscar pelo template.
		if (!npcCategory) // Se ainda não tem categoria, não é um npc válido.
			return;
		endif
	endif

	print("Tem prop?"+prop);

	if (!prop[npcCategory]) // Se ainda não tem pra categoria do npc, cria o struct.
		prop[npcCategory] := Achievement(0,0,0,0,0);

		// prop[npcCategory] := struct{
		// 	xp := 0,
		// 	level := 0,
		// 	kills := 0,
		// 	hits := 0,
		// 	damage := 0
		// };
	endif

	// prop[npcCategory].UpdateProp(); // Atualiza a prop caso algum atributo do struct ainda não exista.
	// UpdateProp(prop[npcCategory]); // Atualiza a prop caso algum atributo do struct ainda não exista.

	prop[npcCategory].SetXP(prop[npcCategory].GetXP() + CInt(combatInfo.xp));
	prop[npcCategory].SetLevel(AchievementCheckLevel(prop[npcCategory].GetXP()));
	prop[npcCategory].SetKills(prop[npcCategory].GetKills() + CInt(combatInfo.kills));
	prop[npcCategory].SetHits(prop[npcCategory].GetHits() + CInt(combatInfo.hits));
	// prop[npcCategory].SetDamage(prop[npcCategory].GetDamage() + CInt(combatInfo.damage));
	DebugMsg(attacker, DEBUG_ACHIEVEMENT, $"Ganho de achievement para a categoria: {npcCategory}");
	DebugMsg(attacker, DEBUG_ACHIEVEMENT, $"XP: {CInt(combatInfo.xp)} - Kills: {CInt(combatInfo.kills)} - Hits: {CInt(combatInfo.hits)} - Damage: {CInt(combatInfo.damage)}");
	DebugMsg(attacker, DEBUG_ACHIEVEMENT, $"Achievement (acumulado) -> XP: {prop[npcCategory].GetXP()} - Level: {prop[npcCategory].GetLevel()} - Kills: {prop[npcCategory].GetKills()} - Hits: {prop[npcCategory].GetHits()} - Damage: {prop[npcCategory].GetDamage()}");

	print($"Testando toString: {prop[npcCategory].toString()}");

	prop[npcCategory] := prop[npcCategory].toStruct(); // Já que o POL não salva class, converte para struct.
	print("Prop final -> " + prop);
	attacker.SetAchievementProgress(prop);
endfunction

// function UpdateProp(byRef prop)
// 	prop.+xp := 0;
// 	prop.+level := 0;
// 	prop.+kills := 0;
// 	prop.+hits := 0;
// 	prop.+damage := 0;
// endfunction

function AchievementCheckLevel(attacker)
	return 0;
endfunction