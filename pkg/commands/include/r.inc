use uo;

function VerificaDisfarce(alvo)
	var ID := cstr(alvo.serial); //Salva o serial do personagem
	if(GetObjProperty(alvo, "disfarce"))
		ID := cstr(GetObjProperty(alvo, "FakeID")); //Substitui o serial pelo falso quando o personagem observado está disfarçado
	endif
	return ID;
endfunction

