use uo;

// Identidade canonica de "pet domado" — substitui os testes espalhados de
// `script == ":ghaia:tamed"`. Mantido num include leve (so builtins core)
// para que consumidores sensiveis (abilities, happyCounter) possam usa-lo
// sem puxar o peso de taming.inc.
//
// Drop-in fiel: enquanto o pet roda o brain legado retorna igual ao teste
// antigo (inclui summons em :ghaia:tamed, tratados ate a Phase 5). Apos
// migracao para :epicAI:brain identifica pelo dono, excluindo summons.
// O script do brain passa a ser detalhe de implementacao.
function IsTamedPet(creature)
	if (!creature)
		return 0;
	endif
	if (creature.script[":ghaia:tamed"])
		return 1;
	endif
	if (creature.script[":epicAI:brain"])
		return creature.master
			&& GetObjProperty(creature, "owner")
			&& !GetObjProperty(creature, "summoned");
	endif
	return 0;
endfunction
