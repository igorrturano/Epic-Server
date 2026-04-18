use cfgfile;

include ":combat:Controller/DamageController_legacy";
include ":tn:tngumps";

//funcao que pergunta por fechadura, caso o item seja lockable

 function AddItemDescription(who, item)
	var repeating := 1;
	var str;
	SendSysMessage(who, "Utilize apenas 3 linhas do gump.");
	while (repeating)
		str := SmallRequestGump(who, "Descreva o item", 1);
		if (len(str) < 84)
			break;
		endif
		SendSysMessage(who, "Voce excedeu o numero maximo de caracteres! Utilize apenas 3 linhas do gump");
	endwhile
	SetObjProperty(item, "description", str);
endfunction
