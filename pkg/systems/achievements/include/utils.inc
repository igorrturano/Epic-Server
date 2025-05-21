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
include ":commands:debugsystem";

function AchievementAdvance(attacker, defender, combatInfo)
	var prop := GetAchievementProp(attacker);

	SetAchievementProp(attacker, prop);
endfunction

function GetAchievementProp(attacker)
	var prop := attacker.GetProp("Achievement");

	if (!prop) 
		prop := dictionary;
	endif

	return prop;
endfunction

function SetAchievementProp(attacker, prop)
	attacker.SetProp("Achievement", prop);
endfunction

function AchievementCheckLevel(attacker)
	return 0;
endfunction