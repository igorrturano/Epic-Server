use uo;

include ":gumps:gumps";
include ":gumps:gumps_ex";

function ComprarDanoAgravado(who)

	var classPoints := cint(GetObjProperty(who, "classpoints"));
	var points := Cint(AP_GetSkill(who, BEGGING))/10 - classPoints;

	var chargump := GFCreateGump();
	GFClosable(chargump, 1);
	GFDisposable(chargump, 0);

	var input;	

	GFResizePic(chargump, 0, 25, 3600, 300, 380);
	GFResizePic(chargump, 15, 40, 0x13BE, 270, 350);
	GFTextMid(chargump, 15, 45, 300, 55, "Habilidade Dano Agravado");
	GFTextLine(chargump, 50, 75, 600, "Espadas" );
	GFTextLine(chargump, 50, 90, 600, "Macas" );	
	GFTextLine(chargump, 50, 105, 600, "Esgrima" );	
	GFTextLine(chargump, 50, 120, 600, "Armas de haste" );	
	GFTextLine(chargump, 50, 135, 600, "Bastoes" );	
	GFTextLine(chargump, 50, 150, 600, "Arqueria" );	
	GFTextLine(chargump, 50, 165, 600, "Machados" );
	GFTextLine(chargump, 50, 180, 600, "Luta desarmada" );
	GFAddButton(chargump, 20, 109, 2103, 2104, 1, 1 );
	GFAddButton(chargump, 20, 124, 2103, 2104, 1, 2 );
	GFAddButton(chargump, 20, 139, 2103, 2104, 1, 3 );
	GFAddButton(chargump, 20, 154, 2103, 2104, 1, 4 );
	GFAddButton(chargump, 20, 169, 2103, 2104, 1, 5 );
	GFAddButton(chargump, 20, 184, 2103, 2104, 1, 6 );
	GFAddButton(chargump, 20, 199, 2103, 2104, 1, 7 );
	GFAddButton(chargump, 20, 214, 2103, 2104, 1, 8 );

	input := GFSendGump(who, chargump);
	input := input[0];
	var nvl := 0;

	if (input == 1)
		nvl := Cint(GetObjProperty(who, "danoagravadoespadas"));
		SetObjProperty(who, "danoagravadoespadas", (nvl + 1));
	elseif (input == 2)
		nvl := Cint(GetObjProperty(who, "danoagravadomacas"));
		SetObjProperty(who, "danoagravadomacas", (nvl + 1));
	elseif (input == 3)
		nvl := Cint(GetObjProperty(who, "danoagravadoesgrima"));
		SetObjProperty(who, "danoagravadoesgrima", (nvl + 1));
	elseif (input == 4)
		nvl := Cint(GetObjProperty(who, "danoagravadoarmasdehaste"));
		SetObjProperty(who, "danoagravadoarmasdehaste", (nvl + 1));
	elseif (input == 5)
		nvl := Cint(GetObjProperty(who, "danoagravadobastoes"));
		SetObjProperty(who, "danoagravadobastoes", (nvl + 1));
	elseif (input == 6)
		nvl := Cint(GetObjProperty(who, "danoagravadoarqueria"));
		SetObjProperty(who, "danoagravadoarqueria", (nvl + 1));
	elseif (input == 7)
		nvl := Cint(GetObjProperty(who, "danoagravadomachados"));
		SetObjProperty(who, "danoagravadomachados", (nvl + 1));
	elseif (input == 8)
		nvl := Cint(GetObjProperty(who, "danoagravadolutadesarmada"));
		SetObjProperty(who, "danoagravadolutadesarmada", (nvl + 1));
	endif
		

endfunction
