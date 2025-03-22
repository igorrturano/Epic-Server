
include "include/say";
include ":attributes:vitals";

function CheckCanWork(who, startingHealth := 0, initialPosStruct := 0)

	startingHealth := !startingHealth ? AP_GetVital(who, "Hits") : startingHealth;
	initialPosStruct := !initialPosStruct ? struct {x := who.x, y := who.y, z := who.z, realm := who.realm} : initialPosStruct ;

	return CheckIsAlive(who) &&
		CheckIsVisible(who) &&
		CheckIsInPeace(who) &&
		CheckIsFree(who) &&
		CheckIsHealthy(who, startingHealth) &&
		CheckHasMoved(who, initialPosStruct);
endFunction

function CheckIsAlive(who)
	if(who.dead)
		SendSysMessageEx(who,"Você foi paralizado.",SSM_FAIL);
		return false;
	endif
	return true;
endFunction

function CheckIsVisible(who)
	if(who.concealed)
		SendSysMessageEx(who,"Você não pode estar escondido.",SSM_FAIL);
		return false;
	endif
	return true;
endFunction

function CheckIsInPeace(who)
	if(who.warmode)
		SendSysMessageEx(who,"Você não pode estar em combate.",SSM_FAIL);
		return false;
	endif
	return true;
endFunction

function CheckIsFree(who)
	if(who.frozen)
		SendSysMessageEx(who,"Você não pode se mover.",SSM_FAIL);
		return false;
	endif
	return true;
endFunction

function CheckIsHealthy(who, startingHealth)
	if(startingHealth !=  AP_GetVital(who, "Hits"))
		SendSysMessageEx(who,"Você foi ferido.",SSM_FAIL);
		return false;
	endif
	return true;
endFunction

function CheckHasMoved(who, initialPosStruct)
	if(initialPosStruct.x != who.x || initialPosStruct.y != who.y|| initialPosStruct.z != who.z || initialPosStruct.realm != who.realm )
		SendSysMessageEx(who,"Você se moveu.",SSM_FAIL);
		return false;
	endif
	return true;
endFunction