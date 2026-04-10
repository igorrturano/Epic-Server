use cfgfile;

include ":combat:Controller/DamageController_legacy";
include ":tn:tngumps";

const  EXCEP := 1;
const  NORMAL := 2;
const  POOR := 3;
const STR_ERRORCHANCE := "ERROR 666";
const STR_NULLCHANCE := "nula";
const STR_VERYLOWCHANCE := "muito baixa";
const STR_LOWCHANCE := "baixa";
const STR_INTERMEDIATECHANCE := "media";
const STR_GOODCHANCE := "alta";
const STR_VERYHIGHCHANCE := "muito alta";

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
