use uo;

// Identidade canonica de "pet domado". Mantido num include leve (so
// builtins core) para que consumidores sensiveis (abilities,
// happyCounter) possam usa-lo sem puxar o peso de taming.inc. Token
// minusculo e exato — match de substring eScript e case-sensitive.
function IsTamedPet(creature)
	if (!creature)
		return 0;
	endif
	if (creature.script[":epicai:tamedBrain"])
		return creature.master
			&& GetObjProperty(creature, "owner")
			&& !GetObjProperty(creature, "summoned");
	endif
	return 0;
endfunction
