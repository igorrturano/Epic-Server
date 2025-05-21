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

function MasteryAdvance(attacker, combatInfo)
	var prop := attacker.GetMasteryProp();

	prop[attacker.weapon.objtype].level := MasteryCheckLevel(attacker);
	prop[attacker.weapon.objtype].xp += CInt(combatInfo.xp);
	prop[attacker.weapon.objtype].hits += CInt(combatInfo.hits);
	DebugMsg(attacker, DEBUG_MASTERY, $"Maestria ganho para a arma {attacker.weapon.desc} ({Hex(attacker.weapon.objtype)})");
	DebugMsg(attacker, DEBUG_MASTERY, $"XP: {CInt(combatInfo.xp)} - Hits: {CInt(combatInfo.hits)}");
	DebugMsg(attacker, DEBUG_MASTERY, $"Maestria (resultado) -> XP: {prop[attacker.weapon.objtype].xp} - Hits: {prop[attacker.weapon.objtype].hits}");

	attacker.SetMasteryProp(prop);
endfunction

function MasteryCheckLevel(attacker)
	return 0;
endfunction