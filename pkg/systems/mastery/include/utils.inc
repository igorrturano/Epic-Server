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

const MASTERY_PROPNAME := "MasterySystem";

function MasteryAdvance(attacker, combatInfo)
	var prop := GetMasteryProp(attacker);
	var weapon := attacker.weapon;

	prop[weapon.objtype].level := MasteryCheckLevel(attacker);
	prop[weapon.objtype].xp += CInt(combatInfo.xp);
	prop[weapon.objtype].hits += CInt(combatInfo.hits);

	SetMasteryProp(attacker, prop);
endfunction

function GetMasteryProp(attacker)
	var prop := attacker.GetProp(MASTERY_PROPNAME);

	if (!prop)
		prop := dictionary;
		prop[attacker.weapon.objtype] := struct{
			xp := 0,
			level := 0,
			hits := 0
		};
	endif

	return prop;
endfunction

function SetMasteryProp(attacker, prop)
	attacker.SetProp(MASTERY_PROPNAME, prop);
endfunction

function MasteryCheckLevel(attacker)

endfunction