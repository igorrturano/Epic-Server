use uo;
include ":attributes:skills";

function VerificaDisfarce(alvo, quem := 0)
	var ID := cstr(alvo.serial); //Salva o serial do personagem
	if (GetObjProperty(alvo, "disfarce"))
		if (!quem || !_PodeVerDisfarce(quem))
			ID := cstr(GetObjProperty(alvo, "FakeID")); //Substitui o serial pelo falso quando o personagem observado está disfarçado
		endif
	endif
	return ID;
endfunction

// Returns 1 if `quem` can see through disguises (TrueSight active or AWARENESS >= 70)
function _PodeVerDisfarce(quem)
	var now := ReadGameClock();
	if (CInt(GetObjProperty(quem, "#truesight_active")) > now)
		return 1;
	endif
	if (AP_GetSkill(quem, AWARENESS) >= 70)
		return 1;
	endif
	return 0;
endfunction

