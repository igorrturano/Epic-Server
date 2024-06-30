
//se tiver passado o tempo retorna 0, se nao retorna o tempo que falta
function getCooldown(who, hability)

	hability := "#"+hability;

	var now := ReadGameClock();
	
	var future := Cint(GetObjProperty(who, hability));
	
	if (now >= future)
		EraseObjProperty(who, hability);
		return 0;
	else
		return (future-now);
	endif

endfunction
	
//verifica se ta em cooldown ainda, se tiber retorna o tempo que falta, se nao tiver seta e retorna 0
function setCooldown(who, hability, time)
	
	hability := "#" + hability;

	var cooldown := GetCoolDown(who, hability);
	
	if (cint(cooldown))
		return cooldown;
	else
		SetObjProperty(who, hability, Cint(ReadGameClock()+time) );
		return 0;
	endif
	
endfunction
