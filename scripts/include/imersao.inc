/* By EvilLink
Biblioteca para imersão com Sons e/ou Emotes
Exemplo de uso: Imersao(who, "Gritar", 1, 250, 1, 0, 0) - Grita; 1 loop; 250ms cada loop; sai som; não sai emote; não sai animação relacionada; 
Exemplo2 de uso: Imersao(who, "Gritar") - Grita; sai som; sai emote; sai animação relacionada
who = alvo (player/item)
"Gritar" = Emote/Som a ser emitido
1 = EMITE som
0 = NÃO EMITE emote (se tiver)
*/

use uo;
use os;
use util;

include "include/say";

const FONTE_AMARELA := 53;

//REFERÊNCIAS
//HUMANOIDE
var EL_BOW := Dictionary{
	"anim" -> 0x20
};

var EL_GRITAR := Dictionary{
	"anim" -> "",
	"sound" -> {0x441, 0x32F},
	"emote" -> "*Gritando*"
};

var EL_GRITAR2 := Dictionary{
	"anim" -> "",
	"sound" -> {0x44B, 0x339},
	"emote" -> "*Gritando*"
};

var EL_HUH := Dictionary{
	"anim" -> "",
	"sound" -> {0x441, 0x32F},
	"emote" -> "*Huh*"
};

var EL_TOSSIR1 := Dictionary{
	"anim" -> "",
	"sound" -> {0x421, 0x312},
	"emote" -> "*Tossindo*"
};

var EL_TOSSIR2 := Dictionary{
	"anim" -> 0x441,
	"sound" -> {0x422, 0x313},
	"emote" -> "*Gritando*"
};

var EL_ARFAR := Dictionary{
	"anim" -> "",
	"sound" -> {0x42A, 0x31A},
	"emote" -> "*Arfar*"
};

var EL_GEMER := Dictionary{
	"anim" -> 0x441,
	"sound" -> {0x42C, 0x31C},
	"emote" -> "*Gemendo*"
};

var EL_CAIR := Dictionary{
	"anim" -> "", //PROCURAR
	"sound" -> {EL_GEMER["sound"][1], EL_GEMER["sound"][2]},
	"emote" -> ""
};

var EL_GOLPEAR := Dictionary{
	"anim" -> 0x441,
	"sound" -> {EL_GRITAR2["sound"][1], EL_GRITAR2["sound"][2]},
	"emote" -> "*Golpeando*"
};

var EL_SUSTO := Dictionary{
	"anim" -> 0x441,
	"sound" -> {0x434, 0x324},
	"emote" -> "*Gritando*"
};

var EL_VOMITAR := Dictionary{
	"anim" -> EL_BOW["anim"],
	"sound" -> {0x440, 0x32e},
	"emote" -> "*Vomitando*"
};

//CRAFT
var EL_METAL := Dictionary{
	"anim" -> "", //ADICIONAR
	"sound" -> 0x002b,
	"emote" -> ""
};

var EL_MADEIRA := Dictionary{
	"anim" -> "",
	"sound" -> 0x23e,
	"emote" -> ""
};

var EL_FLECHA := Dictionary{
	"anim" -> "",
	"sound" -> 0x5a,
	"emote" -> ""
};

var EL_TESOURA := Dictionary{
	"anim" -> "",
	"sound" -> 0x249,
	"emote" -> ""
};

var EL_CRAFT_MW := Dictionary{
	"anim" -> 0x0b,
	"sound" -> EL_METAL["sound"],
	"emote" -> ""
};

var EL_CRAFT_WW := Dictionary{
	"anim" -> "",
	"sound" -> EL_MADEIRA["sound"],
	"emote" -> "*Moldando*"
};

var EL_CRAFT_BC := Dictionary{
	"anim" -> "",
	"sound" -> {EL_FLECHA["sound"][1], EL_FLECHA["sound"][2]},
	"emote" -> "*Moldando*"
};

var EL_CRAFT_DW := Dictionary{
	"anim" -> "",
	"sound" -> {EL_TESOURA["sound"][1], EL_TESOURA["sound"][2]},
	"emote" -> "*Costurando*"
};

//OBJETOS
var EL_VIDRO := Dictionary{
	"anim" -> "",
	"sound" -> 0x03F,
	"emote" -> "*Quebrou*"
};

/////INÍCIO IMERSÃO/////
function Imersao(who, tipoimersao, qtdloops:=1, tempoloop:=0, saisom:=1, saiemote:=1, saianim:=1, saiefeito:=1, user:=0) //EXEMPLO DE USO NO INÍCIO DO ARQUIVO
	var whosound, whoemote, whoanim, whoefeito, noexist, erro;
	tipoimersao := Lower(tipoimersao);
	
	case (tipoimersao)
		//HUMANOIDE
		"gritar":
			whoemote := EL_GRITAR["emote"];
			whoanim := EL_GRITAR["anim"];
			case (who.graphic)
				400:
					whosound := EL_GRITAR["sound"][1];
				401:
					whosound := EL_GRITAR["sound"][2];
				1828:
					whosound := EL_GRITAR["sound"][1];
				1829:
					whosound := EL_GRITAR["sound"][2];
				1830:
					whosound := EL_GRITAR["sound"][1];
				1831:
					whosound := EL_GRITAR["sound"][2];
				1832:
					whosound := EL_GRITAR["sound"][1];
				1833:
					whosound := EL_GRITAR["sound"][2];
				1834:
					whosound := EL_GRITAR["sound"][1];
				1835:
					whosound := EL_GRITAR["sound"][2];
																

				default: erro := " HUMANOIDE";
			endcase
			break;

		"huh":
			whoemote := EL_HUH["emote"];
			whoanim := EL_HUH["anim"];
			case (who.graphic)
				400:
					whosound := EL_HUH["sound"][1];
				401:
					whosound := EL_HUH["sound"][2];
				1828:
					whosound := EL_HUH["sound"][1];
				1829:
					whosound := EL_HUH["sound"][2];
				1830:
					whosound := EL_HUH["sound"][1];
				1831:
					whosound := EL_HUH["sound"][2];
				1832:
					whosound := EL_HUH["sound"][1];
				1833:
					whosound := EL_HUH["sound"][2];
				1834:
					whosound := EL_HUH["sound"][1];
				1835:
					whosound := EL_HUH["sound"][2];

				default: erro := " HUMANOIDE";
			endcase
			break;

		"tossir1":
			whoemote := EL_TOSSIR1["emote"];
			case (who.graphic)
				400:
					whosound := EL_TOSSIR1["sound"][1];
				401:
					whosound := EL_TOSSIR1["sound"][2];
				1828:
					whosound := EL_TOSSIR1["sound"][1];
				1829:
					whosound := EL_TOSSIR1["sound"][2];
				1830:
					whosound := EL_TOSSIR1["sound"][1];
				1831:
					whosound := EL_TOSSIR1["sound"][2];
				1832:
					whosound := EL_TOSSIR1["sound"][1];
				1833:
					whosound := EL_TOSSIR1["sound"][2];
				1834:
					whosound := EL_TOSSIR1["sound"][1];
				1835:
					whosound := EL_TOSSIR1["sound"][2];
				default: erro := " HUMANOIDE";
			endcase
			break;

		"tossir2":
			whoemote := EL_TOSSIR2["emote"];
			case (who.graphic)
				400:
					whosound := EL_TOSSIR2["sound"][1];
				401:
					whosound := EL_TOSSIR2["sound"][2];
				1828:
					whosound := EL_TOSSIR2["sound"][1];
				1829:
					whosound := EL_TOSSIR2["sound"][2];
				1830:
					whosound := EL_TOSSIR2["sound"][1];
				1831:
					whosound := EL_TOSSIR2["sound"][2];
				1832:
					whosound := EL_TOSSIR2["sound"][1];
				1833:
					whosound := EL_TOSSIR2["sound"][2];
				1834:
					whosound := EL_TOSSIR2["sound"][1];
				1835:
					whosound := EL_TOSSIR2["sound"][2];					
				default: erro := " HUMANOIDE";
			endcase
			break;

		"arfar":
			whoemote := EL_ARFAR["emote"];
			case (who.graphic)
				400:
					whosound := EL_ARFAR["sound"][1];
				401:
					whosound := EL_ARFAR["sound"][2];
				1828:
					whosound := EL_ARFAR["sound"][1];
				1829:
					whosound := EL_ARFAR["sound"][2];
				1830:
					whosound := EL_ARFAR["sound"][1];
				1831:
					whosound := EL_ARFAR["sound"][2];
				1832:
					whosound := EL_ARFAR["sound"][1];
				1833:
					whosound := EL_ARFAR["sound"][2];
				1834:
					whosound := EL_ARFAR["sound"][1];
				1835:
					whosound := EL_ARFAR["sound"][2];


				default: erro := " HUMANOIDE";
			endcase
			break;

		"gemer":
			whoemote := EL_GEMER["emote"];
			case (who.graphic)
				400:
					whosound := EL_GEMER["sound"][1];
				401:
					whosound := EL_GEMER["sound"][2];
				1828:
					whosound := EL_GEMER["sound"][1];
				1829:
					whosound := EL_GEMER["sound"][2];
				1830:
					whosound := EL_GEMER["sound"][1];
				1831:
					whosound := EL_GEMER["sound"][2];
				1832:
					whosound := EL_GEMER["sound"][1];
				1833:
					whosound := EL_GEMER["sound"][2];
				1834:
					whosound := EL_GEMER["sound"][1];
				1835:
					whosound := EL_GEMER["sound"][2];

				default: erro := " HUMANOIDE";
			endcase
			break;
		
		"cair":
			whoemote := EL_CAIR["emote"];
			whoanim := EL_CAIR["anim"];
			case (who.graphic)
				400:
					//whosound := EL_CAIR["sound"][1];
				401:
					//whosound := EL_CAIR["sound"][2];
				1828:
					//whosound := EL_CAIR["sound"][1];
				1829:
					//whosound := EL_CAIR["sound"][2];
				1830:
					//whosound := EL_CAIR["sound"][1];
				1831:
					//whosound := EL_CAIR["sound"][2];
				1832:
					//whosound := EL_CAIR["sound"][1];
				1833:
					//whosound := EL_CAIR["sound"][2];
				1834:
					//whosound := EL_CAIR["sound"][1];
				1835:
					//whosound := EL_CAIR["sound"][2];					
				default: erro := " HUMANOIDE";
			endcase
			break;

		"cair2":
			whoemote := EL_CAIR["emote"];
			whoanim := EL_CAIR["anim"];
			case (who.graphic)
				400:
					//whosound := EL_CAIR["sound"][1];
				401:
					//whosound := EL_CAIR["sound"][2];
				1828:
					//whosound := EL_CAIR["sound"][1];
				1829:
					//whosound := EL_CAIR["sound"][2];
				1830:
					//whosound := EL_CAIR["sound"][1];
				1831:
					//whosound := EL_CAIR["sound"][2];
				1832:
					//whosound := EL_CAIR["sound"][1];
				1833:
					//whosound := EL_CAIR["sound"][2];
				1834:
					//whosound := EL_CAIR["sound"][1];
				1835:
					//whosound := EL_CAIR["sound"][2];

				default: erro := " HUMANOIDE";
			endcase
			break;

		"golpear":
			whoemote := EL_GOLPEAR["emote"];
			whoanim := EL_GOLPEAR["anim"];
			case (who.graphic)
				400:
					whosound := EL_GOLPEAR["sound"][1];
				401:
					whosound := EL_GOLPEAR["sound"][2];
				1828:
					whosound := EL_GOLPEAR["sound"][1];
				1829:
					whosound := EL_GOLPEAR["sound"][2];
				1830:
					whosound := EL_GOLPEAR["sound"][1];
				1831:
					whosound := EL_GOLPEAR["sound"][2];
				1832:
					whosound := EL_GOLPEAR["sound"][1];
				1833:
					whosound := EL_GOLPEAR["sound"][2];
				1834:
					whosound := EL_GOLPEAR["sound"][1];
				1835:
					whosound := EL_GOLPEAR["sound"][2];
				default: erro := " HUMANOIDE";
			endcase
			break;
		
		"susto":
			whoemote := EL_SUSTO["emote"];
			whoanim := EL_SUSTO["anim"];
			case (who.graphic)
				400:
					whosound := EL_SUSTO["sound"][1];
				401:
					whosound := EL_SUSTO["sound"][2];
				1828:
					whosound := EL_SUSTO["sound"][1];
				1829:
					whosound := EL_SUSTO["sound"][2];
				1830:
					whosound := EL_SUSTO["sound"][1];
				1831:
					whosound := EL_SUSTO["sound"][2];
				1832:
					whosound := EL_SUSTO["sound"][1];
				1833:
					whosound := EL_SUSTO["sound"][2];
				1834:
					whosound := EL_SUSTO["sound"][1];
				1835:
					whosound := EL_SUSTO["sound"][2];
				default: erro := " HUMANOIDE";
			endcase
			break;
		
		"vomitar":
			whoemote := EL_VOMITAR["emote"];
			whoanim := EL_VOMITAR["anim"];
			case (who.graphic)
				400:
					whosound := EL_VOMITAR["sound"][1];
				401:
					whosound := EL_VOMITAR["sound"][2];
				1828:
					whosound := EL_VOMITAR["sound"][1];
				1829:
					whosound := EL_VOMITAR["sound"][2];
				1830:
					whosound := EL_VOMITAR["sound"][1];
				1831:
					whosound := EL_VOMITAR["sound"][2];
				1832:
					whosound := EL_VOMITAR["sound"][1];
				1833:
					whosound := EL_VOMITAR["sound"][2];
				1834:
					whosound := EL_VOMITAR["sound"][1];
				1835:
					whosound := EL_VOMITAR["sound"][2];
				default: whosound := EL_VOMITAR["sound"][1];
			endcase

			whoefeito := 1;
			break;
		
		//PROFISSÕES
		"mw":
			//whoemote := ;
			whoanim := EL_CRAFT_MW["anim"];
			whosound := EL_CRAFT_MW["sound"];
			break;

		"ww":
			//whoemote := ;
			//whoanim := EL_CRAFT_WW["anim"];
			whosound := EL_CRAFT_WW["sound"];
			break;

		"dw":
			//whoemote := ;
			//whoanim := EL_CRAFT_DW["anim"];
			whosound := EL_CRAFT_DW["sound"];
			break;

		//OBJETOS
		"explodir":
			break;
		
		"quebrar":
			break;
		
		"metal":
			whosound := EL_METAL["sound"];
			break;

		"vidro":
			whoemote := EL_VIDRO["emote"];
			whosound := EL_VIDRO["sound"];
			whoefeito := 1;
			break;

		default: noexist := 1;
	endcase

	if (noexist)
		SendSysMessageEx(user, "Acao inexistente no banco de dados.", SSM_FAIL);
		return;
	endif

	if (user.cmdlevel > 1) //ERROS
		if (erro) SendSysMessageEx(user, "GM: Essa imersao deve ser usada apenas em{}!".Format(erro), SSM_FAIL); return; endif

		if ((saisom == 1) && (!whosound)) erro := " SOM,"; endif
		if ((saiemote == 1) && (!whoemote)) erro := " EMOTE,"; endif
		if ((saianim == 1) && (!whoanim)) erro := " ANIM,"; endif
		if ((saiefeito == 1) && (!whoefeito)) erro := " EFEITO,"; endif

		if (erro) SendSysMessageEx(user, "GM: Executando imersao sem{}!".Format(erro), FONTE_AMARELA); endif
	endif

	var i;
	for (i:=0; i<qtdloops; i++) //EXECUTAR IMERSÃO
		if (saisom == 1) PlaySoundEffect(who, whosound); endif
		if (saiemote == 1) PrintText(who, whoemote); endif
		if (saianim == 1) PerformAction(who, whoanim); endif

		if (saiefeito == 1)
			case (tipoimersao) //AÇÕES COM EFEITOS
				"vomitar":
					Vomitar(who);
					break;
				
				"explodir":
					//efeito de explosão em who
					break;
				
				"vidro":
					if (who.account) erro := " ITEM";
					else
						DestroyItem(who);
					endif
					break;
			endcase
		endif

		sleepms(tempoloop);
	endfor
endfunction

///////////////
// EFEITOS
function Vomitar(who)
	sleepms(200);
	var vomito := CreateItemAtLocation(who.x, who.y, who.z, 0x0f3c, 1);
	vomito.name := CStr("Vômito de "+who.name);

	//random color
	var cor := RandomInt(4);
	case (cor)
		0:	vomito.color := 1422;
		1:	vomito.color := 540;
		2:	vomito.color := 555;
		3:	vomito.color := 970;
	endcase;
	vomito.decayat := ReadGameClock() + 10;
endfunction