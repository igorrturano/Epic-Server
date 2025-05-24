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
include ":debug:main";


function MasteryAdvance(attacker, combatInfo)
	var prop := attacker.GetMasteryProp();

	if (!prop[attacker.weapon.objtype]) // Se ainda não tem pra arma em punho, cria o struct.
		prop[attacker.weapon.objtype] := struct{
			xp := 0,
			level := 0,
			hits := 0,
			kills := 0
		};
	endif

	UpdateProp(prop[attacker.weapon.objtype]); // Atualiza a prop caso algum atributo do struct ainda não exista.

	prop[attacker.weapon.objtype].xp += CInt(combatInfo.xp);
	prop[attacker.weapon.objtype].level := MasteryCheckLevel(prop[attacker.weapon.objtype].xp);
	prop[attacker.weapon.objtype].hits += CInt(combatInfo.hits);
	prop[attacker.weapon.objtype].kills += CInt(combatInfo.kill);
	DebugMsg(attacker, DEBUG_MASTERY, $"Ganho de maestria para a arma: {attacker.weapon.desc} ({Hex(attacker.weapon.objtype)})");
	DebugMsg(attacker, DEBUG_MASTERY, $"XP: {CInt(combatInfo.xp)} - Hits: {CInt(combatInfo.hits)} - Kills: {CInt(combatInfo.kills)}");
	DebugMsg(attacker, DEBUG_MASTERY, $"Maestria (acumulado) -> XP: {prop[attacker.weapon.objtype].xp} - Level: {prop[attacker.weapon.objtype].level} - Hits: {prop[attacker.weapon.objtype].hits} - Kills: {prop[attacker.weapon.objtype].kills}");

	attacker.SetMasteryProp(prop);
endfunction

function UpdateProp(byRef prop)
	prop.+xp := 0;
	prop.+level := 0;
	prop.+hits := 0;
	prop.+kills := 0;
endfunction

function MasteryCheckLevel(xp)
	return 0;
endfunction