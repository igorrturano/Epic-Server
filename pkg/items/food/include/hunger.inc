use uo;
use os;
use util;

include ":attributes:attributes";
include ":charactercreation:resists";
include "include/say";

function MSGFome(fome, sendmsg:=1, sendcolor:=0)
	var msg;

	case (fome)
		5: msg := "Voce esta faminto."; break;
		4: msg := "Voce esta com muita fome"; break;
		3: msg := "Voce esta com fome."; break;
		2: msg := "Voce esta satisfeito."; break;
		1: msg := "Voce esta cheio."; break;
		0: msg := "Voce esta muito bem alimentado."; break;
   	endcase

	return msg;
endfunction

function MSGFomeColor(fome)
	var msgcolor;

	if (fome >= FomeGetLimiar()) return SSM_FAIL;
	else return SSM_INFO;
	endif
endfunction

function FomeGetLimiar()
    return 3; //Para referências. A partir desse valor aplicar debuff!
endfunction

function FomeCalcDebuff(fome)
	var mod := 0;
	var formula := CInt((fome - mod) * 10);
	
	var result := formula;

	return result;
endfunction

function FomeApplyDebuff(who, valor)
	AP_ModifyStatMod(who, STRENGTH, valor);
	AP_ModifyStatMod(who, INTELLIGENCE, valor);
	AP_ModifyStatMod(who, DEXTERITY, valor);
endfunction

function ModStatFome(who, fome)
	var limiar := 3;
	var valdebuff := FomeCalcDebuff(fome);
	var debuff := GetObjProperty(who, "#fome");
	var debufffome := Dictionary;
	var resist := GetResist(who, FORTITUDE);
	var dado := RandomFloat(100);
	
	if (fome >= limiar)
		if (resist >= dado)
			SendSysMessageEx(who, "Voce resistiu aos efeitos da fome.", SSM_INFO);
		else
			SendSysMessageEx(who, "Voce sucumbiu aos efeitos da fome.", SSM_FAIL);
			if (!debuff)
				debufffome["#nivel"] := fome;
				debufffome["valdebuff"] := valdebuff;

				FomeApplyDebuff(who, -(valdebuff));
				who.SetProp("#fome", debufffome);
			else
				var diff := debuff."valdebuff" - valdebuff;
				if (diff != 0)
					debufffome["#nivel"] := fome;
					debufffome["valdebuff"] := valdebuff;

					FomeApplyDebuff(who, diff);
					who.SetProp("#fome", debufffome);
				endif
			endif
		endif
	else
		if (debuff)
			FomeApplyDebuff(who, debuff."valdebuff");
			who.EraseProp("#fome");
		endif
	endif
endfunction