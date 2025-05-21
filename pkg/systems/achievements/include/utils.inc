// TODO: Controlar uma PROP no player (struct) com os dados do mastery:
	// struct masterysystem
		// dict weapon_id
			// .xp
			// .level
			// .hits
			// (ver o que mais pode ser interessante adicionar aqui para ter controle total do mastery)

// TODO: Essa função deve ser chamada no EpicAttack()

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