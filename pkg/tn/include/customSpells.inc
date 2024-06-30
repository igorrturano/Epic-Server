use uo;

function EncantoTeia(who, targ, spellinfo)

	targ := targ[1];

	PlaySoundEffect(targ, 0x493);
	TS_StartTimer(targ, "paralysis", CINT(AP_GetSkill(who, MAGERY)/10)+30, cint(spellinfo.power)*10, who);
	
	var web := CreateItemAtLocation( targ.x, targ.y, targ.z, 0xee4, 1, targ.realm);
	MoveObjectToLocation(web, web.x, web.y, targ.z-20, web.realm, MOVEOBJECT_FORCELOCATION);
	var web2 := CreateItemAtLocation( targ.x, targ.y, targ.z, 0xee6, 1, targ.realm);
	MoveObjectToLocation(web2, web2.x, web2.y, targ.z-20, web2.realm, MOVEOBJECT_FORCELOCATION);
	
	if (!web)
		SendSysMessage(who, " " + web);
	endif
	
	
	var i := 0;
	for ( i := 0; i < 20; i := i + 1)
		MoveObjectToLocation(web , web.x, web.y, web.z+1, web.realm, MOVEOBJECT_FORCELOCATION);
		MoveObjectToLocation(web2 , web2.x, web2.y, web2.z+1, web2.realm, MOVEOBJECT_FORCELOCATION);
		sleepms(200);
		//SendSysMessage(who, "ah ");
	endfor

endfunction