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

// Returns the nickname `observer` has stored for `targ`, or "" if none.
// Routes between `conhecidos` (real identity, keyed by serial) and
// `conhecidos_disfarce` (disguise identity, keyed by FakeID) so that a name
// given to a disguise does not leak to the real identity when the disguise is removed.
function GetKnownNickname(observer, targ)
	var isDisguised := (GetObjProperty(targ, "disfarce") != error);
	if (isDisguised && !_PodeVerDisfarce(observer))
		var conhecidos_disfarce := GetObjProperty(observer, "conhecidos_disfarce");
		if (conhecidos_disfarce == error || !conhecidos_disfarce)
			return "";
		endif
		var fakeId := GetObjProperty(targ, "FakeID");
		if (fakeId == error)
			return "";
		endif
		var disguiseKey := Cstr(fakeId);
		if (conhecidos_disfarce.Exists(disguiseKey))
			return conhecidos_disfarce[disguiseKey];
		endif
		return "";
	endif

	var conhecidos := GetObjProperty(observer, "conhecidos");
	if (conhecidos == error || !conhecidos)
		return "";
	endif
	var realKey := Cstr(targ.serial);
	if (conhecidos.Exists(realKey))
		return conhecidos[realKey];
	endif
	return "";
endfunction

