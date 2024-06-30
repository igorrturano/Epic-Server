
use uo;
use cfgfile;

include ":gumps:gumps";
include "include/arrays";
include ":fls_core:fls_chargumps_ex";
include ":attributes:attributes";
include ":gumps:yesNo";


function HairColor(who, byref chardata)


	var chargump := GFCreateGump();
	GFClosable(chargump, 0);
	GFDisposable(chargump, 0);

	var input;

	var new_look := struct;
	new_look.+SkinColor := who.color;
	new_look.+HairStyle := GetEquipmentByLayer(who, LAYER_HAIR).graphic;
	new_look.+HairColor := GetEquipmentByLayer(who, LAYER_HAIR).color;
	new_look.+BeardStyle := GetEquipmentByLayer(who, LAYER_BEARD).graphic;
	new_look.+BeardColor := GetEquipmentByLayer(who, LAYER_BEARD).color;
	new_look.+Gender := who.gender;
	var myraca := GetObjProperty(who, "raca");

	repeat

		if (!who.connected)
			break;
		endif

		GFResizePic(chargump, 0, 25, 3600, 300, 480);
		GFResizePic(chargump, 15, 40, 0x13BE, 270, 450);
		GFTextMid(chargump, 15, 45, 300, 1153, "Criacao do Personagem");
		GFTextLine(chargump, 50, 75, 1153, "Escolher Cor da Pele");
		GFTextLine(chargump, 50, 105, 1153, "Escolher Cabelo");
		GFTextLine(chargump, 50, 135, 1153, "Escolher Cabelo Facial");
		GFTextLine(chargump, 50, 165, 1153, "Escolher Cor do Cabelo");

		GFAddButton(chargump, 30, 75, 2117, 2118, 1, 2);
		GFAddButton(chargump, 30, 105, 2117, 2118, 1, 3);
		GFAddButton(chargump, 30, 135, 2117, 2118, 1, 4);
		GFAddButton(chargump, 30, 165, 2117, 2118, 1, 5);

		GFAddButton(chargump, 230, 450, 4502, 4502, 1, 1 );
		GFTextLine(chargump, 190, 460, 1153, "Proximo");

		input := GFSendGump(who, chargump);
		input := input[0];

		if (input == 2)
			FLSkinGump(who, new_look);
		elseif (input == 3)
			FLHairGump(who, new_look);
		elseif (input == 4)
			if ( (myraca == "anao") || (myraca == "humano") )
				FLBeardGump(who, new_look);
			else
				SendSysMessage(who, "Sua raca nao usa barba.");
			endif
		elseif (input == 5)
			FLHairDye(who, new_look);
		endif

	until (input <= 1);

	var raca := cstr(GetObjProperty(who, "raca"));

	//se nao escolheu uma cor, seta a primeira.
	if (who.color == new_look.SkinColor)//se a cor for a cor q veio da criacao
		var cfg := ReadConfigFile(":fls_core:config/racas");
		var elem := cfg[cstr(raca)];
		var cores := GetConfigStringArray(elem, "SkinColor");
		who.color := cint(cores[2]);
		who.truecolor := cint(cores[2]);
	else
		who.color := new_look.SkinColor;
		who.truecolor := new_look.SkinColor;
	endif

	//o mesmo pra cor de cabelo
	if ( (GetEquipmentByLayer(who, LAYER_HAIR).color) == new_look.HairColor)
		var cfg := ReadConfigFile(":fls_core:config/racas");
		var elem := cfg[cstr(raca)];
		var cores := GetConfigStringArray(elem, "HairColor");
		new_look.HairColor := cint(cores[2]);
	endif

	//se for algo diferente de anao ou humano destroi a barba
	if ( (raca == "elfo") || (raca == "hobbit") || (raca == "drow") || (raca == "orc") )
		var barba := GetEquipmentByLayer(who, LAYER_BEARD);
		DestroyItem(barba);
	endif

	//cria e equipa o cabelo
	var cabelo := GetEquipmentByLayer(who, LAYER_HAIR);
	DestroyItem(cabelo);
	cabelo := CreateItemAtLocation(who.x, who.y, who.z, new_look.HairStyle, 1, who.realm);
	cabelo.color := new_look.HairColor;
	EquipItem(who, cabelo);

	//se for humano equipa a barba escolhida
	if ( (raca == "humano") || (raca == "anao") )
		var barba := GetEquipmentByLayer(who, LAYER_BEARD);
		DestroyItem(barba);
		barba := CreateItemAtLocation(who.x, who.y, who.z, new_look.BeardStyle,1, who.realm);
		barba.color := new_look.HairColor;
		EquipItem(who, barba);
	endif

	//se for anao equipa sempre a long beard
	//if ( raca == "anao" )
	//	var barba := GetEquipmentByLayer(who, LAYER_BEARD);
	//	DestroyItem(barba);
	//	barba := CreateItemAtLocation(who.x, who.y, who.z, 8268, 1, who.realm);
	//	barba.color := new_look.HairColor;
	//	EquipItem(who, barba);
	//endif

	//se for mulher tira a barba
	if ( who.graphic == 401 || who.graphic == 1829 || who.graphic == 1831 || who.graphic == 1833 || who.graphic == 1835 )
		var barba := GetEquipmentByLayer(who, LAYER_BEARD);
		DestroyItem(barba);
	endif

	var continua :=	YesNo(who, "Voce ainda pode voltar e refazer a aparencia do seu personagem caso nao tenha gostado de algo." , "Sim, quero refazer.", "Nao, quero desse jeito.");
	if (continua)
		HairColor(who, chardata);
	endif

endfunction

function NameClassRace(who, byref chardata)

	var chargump := GFCreateGump();
	var input;
	var first_name, last_name;
	first_name := "";
	last_name := "";

	GFClosable(chargump, 0);
	GFDisposable(chargump, 0);
	GFResizePic(chargump, 0, 25, 3600, 350, 480);
	GFResizePic(chargump, 15, 40, 0x13BE, 320, 450);

	GFTextMid(chargump, 20, 50, 300, 1153, "Criacao do Personagem");

	GFTextLine(chargump, 80, 75, 1153, "Defina o Nome do seu Personagem");
	GFTextLine(chargump, 100, 95, 1153, "Primeiro Nome:");
	GFGumpPic(chargump, 100, 115, 0x988);
	first_name := GFTextEntry(chargump, 105, 117, 150, 25, 1153, first_name, 99);
	GFTextLine(chargump, 100, 135, 1153, "Ultimo Nome:");
	GFGumpPic(chargump, 100, 155, 0x988);
	last_name := GFTextEntry(chargump, 105, 157, 150, 25, 1153, last_name, 98);

	GFTextLine(chargump, 30, 195, 1153, "Escolha a Classe");
	GFTextLine(chargump, 60, 215, 1153, "Combatente");
	GFTextLine(chargump, 60, 235, 1153, "Ladino");
	GFTextLine(chargump, 60, 255, 1153, "Sabio");
	GFTextLine(chargump, 60, 275, 1153, "Artifice");

	GFTextLine(chargump, 30, 315, 1153, "Escolha a Raca");
	GFTextLine(chargump, 60, 335, 1153, "Humano");
	GFTextLine(chargump, 60, 355, 1153, "Elfo");
	GFTextLine(chargump, 60, 375, 1153, "Anao");
	GFTextLine(chargump, 60, 395, 1153, "Hobbit");
	GFTextLine(chargump, 60, 415, 1153, "Drow");
	GFTextLine(chargump, 60, 435, 1153, "Orc");

	GFSetRadioGroup(chargump, 1);
	GFRadioButton(chargump, 30, 215, 210, 211, 1, 1);
	GFRadioButton(chargump, 30, 235, 210, 211, 0, 2);
	GFRadioButton(chargump, 30, 255, 210, 211, 0, 3);
	GFRadioButton(chargump, 30, 275, 210, 211, 0, 4);

	GFSetRadioGroup(chargump, 2);
	GFRadioButton(chargump, 30, 335, 210, 211, 1, 10);
	GFRadioButton(chargump, 30, 355, 210, 211, 0, 11);
	GFRadioButton(chargump, 30, 375, 210, 211, 0, 12);
	GFRadioButton(chargump, 30, 395, 210, 211, 0, 13);
	GFRadioButton(chargump, 30, 415, 210, 211, 0, 14);
	//GFRadioButton(chargump, 30, 435, 210, 211, 0, 15);

	//GFTextLine(chargump, 190, 195, 1153, "Escolha a Tendencia");
	//GFTextLine(chargump, 220, 215, 1153, "Leal e Bom");
	//GFTextLine(chargump, 220, 235, 1153, "Neutro e Bom");
	//GFTextLine(chargump, 220, 255, 1153, "Caotico e Bom");
	//GFTextLine(chargump, 220, 275, 1153, "Leal e Neutro");
	//GFTextLine(chargump, 220, 295, 1153, "Neutro");
	//GFTextLine(chargump, 220, 315, 1153, "Caotico e Neutro");
	//GFTextLine(chargump, 220, 335, 1153, "Leal e Mau");
	//GFTextLine(chargump, 220, 355, 1153, "Neutro e Mau");

	//GFSetRadioGroup(chargump, 3);
	//GFRadioButton(chargump, 190, 215, 210, 211, 1, 31);
	//GFRadioButton(chargump, 190, 235, 210, 211, 0, 32);
	//GFRadioButton(chargump, 190, 255, 210, 211, 0, 33);
	//GFRadioButton(chargump, 190, 275, 210, 211, 0, 34);
	//GFRadioButton(chargump, 190, 295, 210, 211, 0, 35);
	//GFRadioButton(chargump, 190, 315, 210, 211, 0, 36);
	//GFRadioButton(chargump, 190, 335, 210, 211, 0, 37);
	//GFRadioButton(chargump, 190, 355, 210, 211, 0, 38);

	GFAddButton(chargump, 280, 450, 4502, 4502, 1, 20 );
	GFTextLine(chargump, 240, 460, 1153, "Proximo");

	input := GFSendGump(who, chargump);

	if (input[0] == 20)
		first_name := GFExtractData( input, 99);
		last_name := GFExtractData( input, 98);
		if ( len(first_name) < 3)
			SendSysMessage(who, "Seu primeiro nome deve ter mais de 3 letras.");
			NameClassRace(who,  chardata);
		else
			if ( len(last_name) < 3 )
				last_name := "";
			endif
			chardata.+FirstName := first_name;
			chardata.+LastName := last_name;
			who.name := first_name;
			if (last_name)
				who.name := who.name + " " + last_name;
			endif
			var nameinfo := struct;
			nameinfo.+FirstName := first_name;
			nameinfo.+LastName := last_name;
			SetObjProperty(who, "NameInfo", nameinfo);
			foreach key in (input.keys)
				if (key == 1)
					SendSysMessage(who, "Sua classe sera combatente!");
					SetObjProperty(who, "classe", "combatente");
				elseif (key == 2)
					SendSysMessage(who, "Sua classe sera ladino!");
					SetObjProperty(who, "classe", "ladino");
				elseif (key == 3)
					SendSysMessage(who, "Sua classe sera sabio!");
					SetObjProperty(who, "classe", "sabio");
				elseif (key == 4)
					SendSysMessage(who, "Sua classe sera artifice!");
					SetObjProperty(who, "classe", "artifice");
				elseif (key == 10)
					SendSysMessage(who, "Sua raca sera humana!");
					SetObjProperty(who, "raca", "humano");
				elseif (key == 11)
					SendSysMessage(who, "Sua raca sera elfica!");
					SetObjProperty(who, "raca", "elfo");

				elseif (key == 12)
					SendSysMessage(who, "Sua raca sera ana!");
					SetObjProperty(who, "raca", "anao");
				elseif (key == 13)
					SendSysMessage(who, "Sua raca sera hobbit!");
					SetObjProperty(who, "raca", "hobbit");
				elseif (key == 14)
					SendSysMessage(who, "Sua raca sera drow!");
					SetObjProperty(who, "raca", "drow");
				elseif (key == 15)
					SendSysMessage(who, "Sua raca sera orc!");
					SetObjProperty(who, "raca", "orc");

				elseif (key == 31)
					SendSysMessage(who, "Sua tendencia sera Leal e Bom!");
					SetObjProperty(who, "tendencia", "Leal e Bom");
				elseif (key == 32)
					SendSysMessage(who, "Sua tendencia sera Neutro e Bom!");
					SetObjProperty(who, "tendencia", "Neutro e Bom");
				elseif (key == 33)
					SendSysMessage(who, "Sua tendencia sera Caotico e Bom!");
					SetObjProperty(who, "tendencia", "Caotico e Bom");
				elseif (key == 34)
					SendSysMessage(who, "Sua tendencia sera Leal e Neutro!");
					SetObjProperty(who, "tendencia", "Leal e Neutro");
				elseif (key == 35)
					SendSysMessage(who, "Sua tendencia sera Neutro!");
					SetObjProperty(who, "tendencia", "Neutro");
				elseif (key == 36)
					SendSysMessage(who, "Sua tendencia sera Caotico e Neutro!");
					SetObjProperty(who, "tendencia", "Caotico e Neutro");
				elseif (key == 37)
					SendSysMessage(who, "Sua tendencia sera Leal e Mau!");
					SetObjProperty(who, "tendencia", "Leal e Mau");
				elseif (key == 38)
					SendSysMessage(who, "Sua tendencia sera Neutro e Mau!");
					SetObjProperty(who, "tendencia", "Neutro e Mau");

				endif


			endforeach
		endif

	endif


endfunction

function attributesGump(who, byref chardata)

	var stats := 100;
	var str, int, dex, vit;
	str := 0;
	int := 0;
	vit := 0;
	dex := 0;
	var repeatGump := 0;

	//zera todas as skills e stats
	foreach attrib_name in AP_ListAttributesByType()
		AP_SetTrueSkill(who, attrib_name, 0);
		sleepms(10);
	endforeach

	while ( (repeatGump != 1) )
		if (!who.connected)
			break;
		endif
		repeatGump := GumpAttr(who, chardata, stats, str, int, vit, dex);
	endwhile


endfunction

function GumpAttr(who, byref chardata, byref stats, byref str, byref int, byref vit, byref dex)

	var chargump := GFCreateGump();
	var input;

	var classCfg := ReadConfigFile(":fls_core:config/classes");
	var elem := FindConfigElem(classCfg, cstr(getobjproperty(who, "classe")) );
	var baseInt := Cint(GetConfigInt(elem, "int"));
	var baseStr := Cint(GetConfigInt(elem, "str"));
	var baseDex := Cint(GetConfigInt(elem, "dex"));
	var baseVit := Cint(GetConfigInt(elem, "vit"));

	var raceCfg := ReadConfigFile(":fls_core:config/racas");
	var elem2 := FindConfigElem(raceCfg, cstr(getobjproperty(who, "raca")) );
	var modInt := Cint(GetConfigInt(elem2, "intmod"));
	var modStr := Cint(GetConfigInt(elem2, "strmod"));
	var modDex := Cint(GetConfigInt(elem2, "dexmod"));
	var modVit := Cint(GetConfigInt(elem2, "vitmod"));

	GFClosable(chargump, 0);
	GFDisposable(chargump, 0);
	GFResizePic(chargump, 0, 25, 3600, 300, 480);
	GFResizePic(chargump, 15, 40, 0x13BE, 270, 450);
	GFTextMid(chargump, 15, 50, 300, 1153, "Defina os Atributos");
	GFTextMid(chargump, 15, 70, 300, 1153, "Voce tem " + stats + " pontos.");
	GFTextLine(chargump, 230, 90, 1153,  "+   - ");
	GFTextLine(chargump, 50, 120, 1153,  "Forca: ");
	GFTextLine(chargump, 50, 150, 1153, "Destreza: ");
	GFTextLine(chargump, 50, 180, 1153, "Vitalidade: ");
	GFTextLine(chargump, 50, 210, 1153, "Inteligencia: ");

	GFTextLine(chargump, 120, 120, 1153,  baseStr + " + " + modStr + " + " + str);
	GFTextLine(chargump, 120, 150, 1153, baseDex +  " + " + modDex + " + " + dex);
	GFTextLine(chargump, 120, 180, 1153, baseVit +  " + " + modVit + " + " + vit);
	GFTextLine(chargump, 120, 210, 1153, baseInt +  " + " + modInt + " + " + int);

	GFAddButton(chargump, 230, 120, 9760, 9761, 1, 10); //+str
	GFAddButton(chargump, 230, 150, 9760, 9761, 1, 11); //+dex
	GFAddButton(chargump, 230, 180, 9760, 9761, 1, 12); //+vit
	GFAddButton(chargump, 230, 210, 9760, 9761, 1, 13); //+int

	GFAddButton(chargump, 260, 120, 9764, 9765, 1, 20); //-str
	GFAddButton(chargump, 260, 150, 9764, 9765, 1, 21); //-dex
	GFAddButton(chargump, 260, 180, 9764, 9765, 1, 22); //-vit
	GFAddButton(chargump, 260, 210, 9764, 9765, 1, 23); //-int


	GFAddButton(chargump, 230, 450, 4502, 4502, 1, 1 );
	GFTextLine(chargump, 190, 460, 1153, "Proximo");

	input := GFSendGump(who, chargump);
	input := input[0];

	if (input == 10)
		if (stats >= 5)
			if (str < 50)
				str := str +5;
				stats := stats - 5;
			else
				SendSysMessage(who, "Voce so pode colocar 50 pontos em um attributo.");
			endif
		endif
	elseif (input == 11)
		if (stats >= 5)
			if (dex < 50)
				dex := dex + 5;
				stats := stats - 5;
			else
				SendSysMessage(who, "Voce so pode colocar 50 pontos em um attributo.");
			endif
		endif
	elseif (input == 12)
		if (stats >= 5)
			if (vit < 50)
				vit := vit + 5;
				stats := stats - 5;
			else
				SendSysMessage(who, "Voce so pode colocar 50 pontos em um attributo.");
			endif
		endif
	elseif (input == 13)
		if (stats >= 5)
			if (int < 50)
				int := int+5;
				stats := stats - 5;
			else
				SendSysMessage(who, "Voce so pode colocar 50 pontos em um attributo.");
			endif
		endif
	elseif (input == 20)
		if (str > 0)
			str := str - 5;
			stats := stats + 5;
		endif
	elseif (input == 21)
		if (dex > 0)
			dex := dex - 5;
			stats := stats + 5;
		endif
	elseif (input == 22)
		if (vit > 0)
			vit := vit - 5;
			stats := stats + 5;
		endif
	elseif (input == 23)
		if (int >0)
			int := int -5;
			stats := stats + 5;
		endif
	elseif (input == 1)
		chardata.+Str := baseStr + modStr + str;
		chardata.+Dex := baseDex + modDex + dex;
		chardata.+Int := baseInt + modInt + int;
		chardata.+Vit := baseVit + modVit + vit;
		//Setando os atributos
		AP_SetTrueStat(who, STRENGTH, chardata.Str);
		AP_SetTrueStat(who, DEXTERITY, chardata.Dex);
		AP_SetTrueStat(who, INTELLIGENCE, chardata.Int);
		SetObjProperty(who, "Vitality", chardata.Vit);
		RecalcVitals(who);
		return 1;
	endif

	return 0;

endfunction

function skillsgump(who, byref chardata)

	var skillspoints := 25;
	var bonus := Cint(AP_GetStat(who, INTELLIGENCE));
	skillspoints := skillspoints + Cint(bonus/20);
	if (GetObjProperty(who, "raca") == "humano")
		skillspoints := skillspoints + 1;
	endif
	var repeatGump := 0;

	chardata.+Skills := dictionary;
	chardata.+Primarias := 0;

	while ( (repeatGump != 1) )
		if (!who.connected)
			break;
		endif
		repeatGump := GumpSkills(who, chardata, skillspoints);
	endwhile

	//agora vamos setar as que o char comprou
	var value;
	var skillsmap := chardata.Skills;
	var skillsArray := skillsmap.keys();
	foreach skill in skillsArray
		value := 0;
		if ( chardata.Skills[skill] == 3)
			value := 40;
		elseif (chardata.Skills[skill] == 2)
			value := 20;
		else
			value := 0;
		endif
		AP_SetTrueSkill(who, GetTrueSkillName(skill), value);
	endforeach

	chardata.Skills["classskill"] := 3;
endfunction

function GumpSkills(who, byref chardata, byref skillspoints)

	var chargump := GFCreateGump();
	var input;
	var cfg := ReadConfigFile(":fls_core:config/classes");
	var elem := FindConfigElem(cfg, cstr(GetObjProperty(who, "classe")) );
	var primarias    := GetConfigStringArray(elem, "P");
	var secundarias  := GetConfigStringArray(elem, "S");
	var terciarias   := GetConfigStringArray(elem, "T");
	var quaternarias := GetConfigStringArray(elem, "Q");

	GFClosable(chargump, 0);
	GFDisposable(chargump, 0);
	GFResizePic(chargump, 0, 25, 3600, 600, 480);
	GFResizePic(chargump, 15, 40, 0x13BE, 570, 450);
	GFTextMid(chargump, 15, 45, 600, 1153, "Defina suas Skills");
	GFTextMid(chargump, 15, 60, 600, 1153, "Voce tem " + skillspoints + " pontos.");

	GFAddButton(chargump, 510, 450, 4502, 4502, 1, 99 );
	GFTextLine(chargump, 450, 460, 1153, "Proximo");

	GFTextLine(chargump, 30, 460, 1153, "Custo dos pontos: ");
	GFTextLine(chargump, 140, 460, 88, "Azul: 1 ");
	GFTextLine(chargump, 190, 460, 80, "Verde: 2 ");
	GFTextLine(chargump, 250, 460, 52, "Amarelo: 3 ");
	GFTextLine(chargump, 320, 460, 38, "Vermelho: 4 ");

	var x := 50;
	var y := 75;
	var x2 := 150;
	var x3 := 30;
	foreach skill in primarias
		GFTextLine(chargump, x, y, 88, skill); //nome da skill
		var value := chardata.Skills[skill];
		if (value == error)
			value := "";
		endif
		//valor da skill (P, S ou T)
		GFTextLine(chargump, x2, y, 88, GetSkillDescValue(value));
		GFAddButton(chargump, x3, y+4, 2103, 2104, 1, FL_GetSkillID(skill) );
		y := y + 15;
		if (y >= 450)
			y := 75;
			x := 350;
			x2 := 450;
			x3 := 330;
		endif
	endforeach

	foreach skill in secundarias
		GFTextLine(chargump, x, y, 80, skill);
		var value := chardata.Skills[skill];
		if (value == error)
			value := "";
		endif
		GFTextLine(chargump, x2, y, 80, GetSkillDescValue(value));
		GFAddButton(chargump, x3, y+4, 2103, 2104, 1, FL_GetSkillID(skill) );
		y := y + 15;
		if (y >= 450)
			y := 75;
			x := 350;
			x2 := 450;
			x3 := 330;
		endif
	endforeach

	foreach skill in terciarias
		GFTextLine(chargump, x, y, 52, skill);
		var value := chardata.Skills[skill];
		if (value == error)
			value := "";
		endif
		GFTextLine(chargump, x2, y, 52, GetSkillDescValue(value));
		GFAddButton(chargump, x3, y+4, 2103, 2104, 1, FL_GetSkillID(skill) );
		y := y + 15;
		if (y >= 450)
			y := 75;
			x := 350;
			x2 := 450;
			x3 := 330;
		endif
	endforeach

	foreach skill in quaternarias
		GFTextLine(chargump, x, y, 38, skill);
		var value := chardata.Skills[skill];
		if (value == error)
			value := "";
		endif
		GFTextLine(chargump, x2, y, 38, GetSkillDescValue(value));
		GFAddButton(chargump, x3, y+4, 2103, 2104, 1, FL_GetSkillID(skill) );
		y := y + 15;
		if (y >= 450)
			y := 75;
			x := 350;
			x2 := 450;
			x3 := 330;
		endif
	endforeach


	input := GFSendGump(who, chargump);
	input := input[0];

	if ( input != 99)
		var custo := GetSkillPrice(input, primarias, secundarias, terciarias, quaternarias);
		var skillname := FL_GetSkillName(input);
		if (skillspoints >= custo) //se tem pontos pra comprar
			if (chardata.Skills[skillname] == 3)//se ja for primaria
				SendSysMessage(who, "Voce ja tem essa skill primaria!");
			elseif (chardata.Skills[skillname] == 2)//se for secundaria
				if (chardata.Primarias <3)
					chardata.Skills[skillname] := chardata.Skills[skillname]+1;
					chardata.Primarias := chardata.Primarias + 1;
					skillspoints := skillspoints - custo;
				else
					SendSysMessage(who, "Voce ja tem 3 skills primarias!");
				endif
			else//se nao tiver ou for terciaria
				chardata.Skills[skillname] := Cint(chardata.Skills[skillname]) +1;
				skillspoints := skillspoints - custo;
			endif
		else
			SendSysMessage(who, "Voce nao tem pontos para comprar esta skill.");
		endif
	else
		return 1;
	endif



endfunction

function introgump(who)

	var chargump := GFCreateGump();
	var input;

	GFClosable(chargump, 0);
	GFDisposable(chargump, 0);
	GFResizePic(chargump, 0, 25, 3600, 600, 480);
	GFResizePic(chargump, 15, 40, 0xA3C, 570, 450);
	//GFTextMid(chargump, 15, 35, 600, 1153, "Criacao do Personagem");

	var cfgtext := ReadConfigFile(":fls_core:config/introduction");
	var textelem := FindConfigElem( cfgtext, "intro1" );
	var lines := GetConfigString(textelem, "Line");

	GFAddButton(chargump, 510, 450, 4502, 4502, 1, 1 );
	GFTextLine(chargump, 450, 460, 1153, "Proximo");
	GFHTMLArea(chargump, 20, 65, 560, 400, lines, 0, 1);
	cfgtext := 0;
	input := GFSendGump(who, chargump);
	input := input[0];
	if (input == 1)
		return;
	endif

endfunction

function ItemPickerGump(who, byref chardata)

	var cfg := ReadConfigFile(":fls_core:config/itempicker");
	var secoes := GetConfigStringKeys( cfg );
	var chargump := GFCreateGump();
	var input;

	GFClosable(chargump, 0);
	GFDisposable(chargump, 0);
	GFResizePic(chargump, 0, 25, 3600, 600, 480);
	GFResizePic(chargump, 15, 40, 0x13BE, 570, 450);
	GFTextMid(chargump, 15, 45, 600, 1153, "Defina seus Itens Iniciais");
	GFTextMid(chargump, 15, 60, 600, 1153, "Voce tem " + chardata.Credits + " creditos.");
	GFAddButton(chargump, 510, 450, 4502, 4502, 1, 999 );
	GFTextLine(chargump, 450, 460, 1153, "Proximo");

	var x := 50;
	var y := 75;
	var page := 1;
	foreach entry in secoes
		if (entry != "DefaultAmount")
			GFTextLine(chargump, x, y, 1153, entry );
			GFAddButton(chargump, x-30, y+4, 9762, 9763, GF_PAGE_BTN, page);
			y := y + 20;
			page := page + 1;
		endif
	endforeach

	page := 1;
	var itens;
	x := 170;
	y := 75;
	var itensmap := dictionary;
	var itemcusto := dictionary;
	var i := 1;
	var elem;
	var custo;
	foreach entry in secoes
		if (entry != "DefaultAmount")
			GFPage(chargump, page);
			elem := FindConfigElem(cfg, entry);
			itens := ListConfigElemProps(elem);
			foreach item in itens
				GFTextLine(chargump, x, y, 1153, item );
				custo := GetConfigInt(elem, item);
				GFTextLine(chargump, x+100, y, 1153, ""+custo );
				itemcusto[item] := custo;
				itensmap[i] := item;
				GFAddButton(chargump, x-20, y+4, 2103, 2104, 1, i);
				i := i + 1;
				y := y + 20;
				if ( y >= 450 )
					y := 75;
					x := 320;
				endif
			endforeach
			x := 170;
			y := 75;
			page := page + 1;
		endif
	endforeach

	input := GFSendGump(who, chargump);
	input := input[0];

	if (input == 999)
		return;
	else
		var buyitem := itensmap[input];
		var custo := itemcusto[buyitem];
		var amt := 1;
		var amtelem := FindConfigElem(cfg, "DefaultAmount");
		amt := Cint(GetConfigInt(amtelem, buyitem));
		if ( (amt == 0) || (amt == error) )
			amt := 1;
		endif
		if (chardata.Credits >= custo)
			chardata.Credits := chardata.Credits - custo;
			CreateItemInBackpack( who, buyitem, amt );
			ItemPickerGump(who, chardata);
		else
			SendSysMessage(who, "Voce nao tem creditos para comprar isto.");
			ItemPickerGump(who, chardata);
		endif
	endif

endfunction
