use util; //funcoes de gerar aleatorio

//includes para gumps. Nao sei se todos sao usados
include ":gumps:gumps";
include ":gumps:requestGump";
include ":gumps:yesNo";
include ":charactercreation:habilidades"; //Contem as funcoes de testar habilidades
include ":attributes:attributes";//AP_GetSkill()
include ":timedScripts:timedScripts"; //TS_StartTimer()
include "include/say"; //Precisei incluir porque o arquivo resists.inc usa mas nao tem declarado
include ":charactercreation:resists"; //Contem as funcoes de testar resistencias

CONST BARD_ENCERRA := 999;
CONST BARD_TOTAL_LENDARIAS := 22; //NUMERO DE PARTITURAS LENDARI

//Constantes para tipos de alvos TODO: Achar as referencias corretas

CONST ALVO_BUFF_TODOS := 20;
CONST ALVO_BUFF_PARTY := 30;
CONST ALVO_BUFF_NO_PARTY := 40;
CONST ALVO_BUFF_PLAYER := 50;
CONST ALVO_BUFF_NO_PLAYER := 60;
CONST ALVO_BUFF_EVIL := 70;

CONST ALVO_DEBUFF_TODOS := 200;
CONST ALVO_DEBUFF_PARTY := 300;
CONST ALVO_DEBUFF_NO_PARTY := 400;
CONST ALVO_DEBUFF_PLAYER := 500;
CONST ALVO_DEBUFF_NO_PLAYER := 600;
CONST ALVO_DEBUFF_EVIL := 700;

//Constantes para partitura em branco

CONST TIPO_PARTITURA := 2;
CONST ITEM_INSTRUMENTO := 1;
CONST NOTA_VAZIA := array;
CONST LETRA_VAZIA := array;
CONST TEMPO_PADRAO := array;

//ID dos efeitos

CONST BARD_COMUM := 1;
CONST BARD_CALMA := 2;
CONST BARD_TRISTE := 3;
CONST BARD_ACAO := 4;

CONST BARD_STR_UP := 5;
CONST BARD_STR_DOWN := 6;
CONST BARD_DEX_UP := 7;
CONST BARD_DEX_DOWN := 8;
CONST BARD_INT_UP := 9;
CONST BARD_INT_DOWN := 10;
CONST BARD_STR_DEX_UP := 11;
CONST BARD_STR_DEX_DOWN := 12;
CONST BARD_STR_DEX_INT_UP := 13;
CONST BARD_STR_DEX_INT_DOWN := 14;

CONST BARD_INSPIRAR_COMPETENCIA := 15;
CONST BARD_INSPIRAR_HEROISMO := 16;
CONST BARD_INSPIRAR_CORAGEM := 17;

CONST BARD_FASCINAR := 18;
CONST BARD_ALUCINACAO := 19;
CONST BARD_DANCA_CONTAGIANTE := 20;

CONST BARD_BLADE_DANCE := 21;

CONST BARD_HAMELIN := 22;
CONST BARD_SNAKE_CHARM := 23;
CONST BARD_FIREDANCE := 24;
CONST BARD_EMUDECER := 25;
CONST BARD_BLOOM := 26;

CONST BARD_INCITAR := 27;
CONST BARD_ACALMAR := 28;

CONST BARD_COPIA := 29;
CONST BARD_FANTASMAS := 30;

//Configuracoes do texto
CONST BARD_FONT := 3;
CONST BARD_FONT_COLOR := 191; //um tipo de azul bebe
CONST BARD_FONT_COLOR_LENDARIA := 43; //achar a cor dourada, alaranjada ou outra chamativa
CONST BARD_FONT_COLOR_COMUM := 93; //achar uma cor proxima ao azul
CONST BARD_FONT_COLOR_BAD := 38; //um tipo de vermelho
CONST BARD_FONT_COLOR_GOOD := 568; //um tipo de verde
CONST BARD_FONT_COLOR_ITEM := 28; //um tipo de rosa
CONST BARD_FONT_COLOR_TARGET := 300; //um tipo de azul claro
CONST BARD_FONT_COLOR_BASICA := 186; //um azul claro esverdeado
CONST BARD_FONT_COLOR_NEUTRA := 275; //Verde claro

//Primeira oitava
CONST NOTA_C := 11;      //C
CONST NOTA_CS := 12;     //Cs
CONST NOTA_D := 13;      //D
CONST NOTA_DS := 14;     //Ds
CONST NOTA_E := 15;      //E
CONST NOTA_F := 16;      //F
CONST NOTA_FS := 17;     //Fs
CONST NOTA_G := 18;      //G
CONST NOTA_GS := 19;     //Gs
CONST NOTA_A := 20;     //A
CONST NOTA_ASS := 21;   //Ass
CONST NOTA_B := 22;     //B

//Pausa
CONST NOTA_P := 23;     //P

//Letra
CONST NOTA_L := 24;     //L

//Segunda oitava
CONST NOTA_C2 := 25;    //C2
CONST NOTA_CS2 := 26;   //Cs2
CONST NOTA_D2 := 27;    //D2
CONST NOTA_DS2 := 28;   //Ds2
CONST NOTA_E2 := 29;    //E2
CONST NOTA_F2 := 30;    //F2
CONST NOTA_FS2 := 31;   //Fs2
CONST NOTA_G2 := 32;    //G2
CONST NOTA_GS2 := 33;   //Gs2
CONST NOTA_A2 := 34;    //A2
CONST NOTA_ASS2 := 35;  //Ass2
CONST NOTA_B2 := 36;    //B2

//terceira oitava
CONST NOTA_C3 := 37;    //C3

//Botao COMPOR/TOCAR
CONST NOTA_TOCAR:= 50;

//Botao de limpar todas as notas
CONST NOTA_LIMPAR_TUDO:= 51;

//Botao de limpar apenas a ultima das notas
CONST NOTA_LIMPAR_ULTIMO:= 52;

//Botao de compor partitura
CONST NOTA_COMPOR_PARTITURA:= 53;

function preenchePartituraMagica(byref item, mag:=0) //mag é um parametro a ser utilizado se desejar criar uma partitura magica especifica
	//verifica se criacao personalizada ou aleatoria
	var magica;
	if (!mag)
		magica := (Randomint(BARD_TOTAL_LENDARIAS) + 5); // Aleatoria entre as partituras lendarias, pulando as 4 musicas básicas
	else
		magica := mag;
	endif

	var notas := array;
	var notas2 := array;
	var letra := array;
	var compositor;
	var tempo := array;
	var desc;

	//define os dados que a serem inseridos na partitura
	case (magica)
		BARD_STR_UP:
			desc := "Hino de Zharkia";
			compositor := "Serge Hivythorn";
			notas := {"G", "|", "L", "|", "F", "D#", "|", "G", "|", "F", "|", "|", "D#", "D", "|", "|", "L", "|", "G", "|", "|", "G#", "A#", "|", "G#", "G", "F", "|", "G", "G#", "G", "|", "|", "L", "|", "F", "|", "G", "|", "D#", "|", "C", "L", "|", "D", "|", "D#", "|", "D", "|", "|", "L", "|", "F", "|", "G", "|", "D#", "|", "F", "|", "D", "|", "D#", "|", "L", "C"};
			foreach nota in notas
				tempo[_nota_iter] := 450;
			endforeach
			letra := { "Vamos vamos vamos lutar!" , "Com forca e Coragem no Coracao!", "Todo dia Memmar vamos defender", "Pois lutaremos como Ahenson, Deus-Rei de Zanshir", "Para que um dia Zharkia veja o raiar", "De um dia sem sangue no Chao!" };
			break;
		BARD_STR_DOWN:
			desc := "Grito de Cha'ke";
			compositor := "Serveror Centopedia";
			notas := {"C", "C", "C", "C", "C", "C", "C", "C", "C", "C", "C", "C", "C", "C", "C", "C", "C", "C", "C", "C", "C", "C", "C", "C", "C", "C", "C", "C", "C", "C", "C", "C"};
			foreach nota in notas
				tempo[_nota_iter] := 250;
			endforeach
			break;
		BARD_DEX_UP:
			desc := "Vento das Brumas";
			compositor := "O Desconhecido";
			notas := {"B", "C", "B", "C", "B", "C", "B", "C", "G#", "C", "G#", "C", "G#", "C", "G#", "C", "F", "C", "F", "C", "F", "C", "F", "C", "G", "C", "G", "C", "G", "C", "G", "C", "G#", "G", "F", "G", "G#", "G", "F", "D#", "D", "F", "D", "D#", "D#"};
			foreach nota in notas
				tempo[_nota_iter] := 300;
			endforeach
			break;
		BARD_DEX_DOWN:  //Roberta Criou
			desc := "Primavera verdejante"; //The Four Seasons - Spring: A. Vivaldi
			compositor := "Vivaldini Primaveril";
			notas := {"C","E","E","E","D","C","G","G","F","E","E","E","D","C","G","G","F","E","F","G","F","E","D","C","E","E","E","D","C","G","G","F","E","E","E","D","C","G","G","F","E","F","G","F","E","D","C","G","F","E","F","G","A","G","C","G","F","E","F","G","A","G","C","A","G","F","E","D","C","D","C","C","C","G","F","E","F","G","A","G","C","G","F","E","F","G","A","G","C","A","G","F","E","D","C","D","C","C"};
			tempo := {400,400,400,400,220,220,600,220,220,400,400,400,220,220,600,220,220,400,220,220,400,400,600,400,400,400,400,220,220,600,220,220,400,400,400,220,220,600,220,220,400,220,220,400,400,600,400,400,220,220,400,400,400,600,400,400,220,220,400,400,400,600,400,400,600,400,400,220,220,280,220,600,400,400,220,220,400,400,400,600,400,400,220,220,400,400,400,600,400,400,600,400,400,220,220,280,400,600};
			break;
		BARD_INT_UP:
			desc := "A Irmandade";
			compositor := "Uiara Ully";
			notas := {"C", "C", "C#", "C#", "C", "C", "D#", "C", "C", "C#", "C#", "C", "C", "F#", "|", "|", "F", "F", "F#", "F#", "D#", "D#", "F", "|", "|", "D#", "F#", "G", "C#", "C", "|", "|", "|"};
			foreach nota in notas
				tempo[_nota_iter] := 450;
			endforeach
			break;
		BARD_INT_DOWN:  //Roberta Criou
			desc := "Pedregulho Pedroso"; //The Flinstones: Hanna, Barbera, Curtin
			compositor := "Fred Rubble";
			notas := {"G","C","C2","A","G","C","G","F","E","E","F","G","C","D","E","G","C","C2","A","G","C","G","F","E","E","F","G","C","D","C","B","E","C2","B","B","A","A","B","A","A","D","B","A","A","G","G","A","G","G","C","C2","A","G","C","G","F","E","E","F","G","C","D","E","F","G","C","D","E","F","G","C2","D2","C2"};
			tempo := {500,700,500,200,500,700,500,200,200,200,200,200,500,500,750,500,700,500,200,500,700,500,200,200,200,200,200,500,500,750,500,700,500,200,200,200,200,200,600,500,700,500,200,200,200,200,200,600,500,700,500,200,500,700,500,200,200,200,200,200,500,700,200,200,200,500,700,200,200,200,500,500,650};
			break;
		BARD_STR_DEX_UP:
			desc := "Aurora de Tor'Amril";
			compositor := "Katarina Lamart";
			notas := {"E", "|", "|", "E", "|", "B", "A", "|", "|", "G", "|", "F#", "G", "F#", "|", "D", "|", "|", "|", "|", "E", "|", "|", "E", "|", "B", "|", "A", "|", "|", "G", "|", "F#", "G", "F#", "|", "G"};
			foreach nota in notas
				tempo[_nota_iter] := 300;
			endforeach
			break;
		BARD_INSPIRAR_HEROISMO:
			desc := "Herois do Cerco"; //Contribuição de CJ com base em cancoes militares ATHOSIADO
			compositor := "Cleudson, o menestrel";
			notas := {"L","D","G","A","A#","D","A#","F#","G","G","L","F","D#","D","C","D#","F","G","G","D","L","D","F#","G","A","C","A","A#","G","A","A","L","G","F#","D#","D#","D","C","D#","G","D","L","D","G","A","A#","D","A#","A","D","F#","G","G","L","G","C","D","D#","D","C","G","D#","C","C","L","C","D#","F","G","D#","C","G","A","A#","A#","L","A#","A","G","D","F#","A","A#","A","G","G","L","D","G","F","D","D","A","C","A#","D","F","A","G","G","D#","L","D#","A","G","C","C","D","C","G","G","A#","A","G","F","L","D","G","F","D","D","C","A#","F","F","D#","D","D","C","G","L","G","A","G","F","F","F#","D","A#","A","A","G","F","A#","L","D","G","A","A#","D","A#","A","D","F","G","G","L","F","D#","D","C","D#","F","G","G","D","L","D","F#","G","A","C","A","A#","G","A","A","L","G","A#","D#","D#","D","C","D#","G","D"};//"G","A","A#","A","G","D","F","D#","F","D#","D","A","C","A#","A","G","A#","A","G","A","G","G","A","A#","A","G","D","F","D#","D","A","D","D","F","|","G","G","G","A#","A","G","A","A","A","C","A#","A","A#","A#","A#","D","C","A#","C","C","A#","A","G","G","G","A#","A","G","A","A","A","C","A#","A","A#","A#","A#","D","C","A#","C","C","A#","A","G","D","D","C#","C#",
			letra := {"O cerco jah durava","tantos sois que perdi a conta quando no ceu despontava","Nossas mulheres e criancas raptadas","subjugadas, violadas e desesperadas","Esse foi o Cerco a Cidadela","um periodo de pouca ou nenhuma cautela","Quem o diga nosso querido Arauto"," que em uma nobre acao para muitos","acabou, pelo orc General, facilmente enganado","Coube a Emister nossa suma santidade","a tarefa de livrar dos orcs nossa cidade","Balian, Cailan, Lucca, Benjamin, Gaiznom e Cantrell","eram os nobres guerreiros que ele acompanhava","A cada machadada ou espada atravessada","o sangue verde dos orcs jorrava","e assim o fogo na cidade se apagava"};
			foreach nota in notas
				tempo[_nota_iter] := 370;
			endforeach
			break;
		BARD_STR_DEX_INT_UP:
			desc := "Mangas Verdes"; //Silverleaf criou: MUSICA CALMA E INSPIRADORA
			compositor :="Rhul Kumak";
			notas := {"A","C2","D2","E2","F2","E2","D2","B","G","A","B","C2","A","A","G#","A","B","G#","E","A","C2","D2","E2","F2","E2","D2","B","G","A","B","C2","A","A","G#","A","B","G#","E"};
			tempo := {500,1000,500,750,450,500,1000,500,750,450,500,1000,500,750,360,500,1000,500,1000};
			//PartituraDebug(notas,tempo);
			break;
		BARD_STR_DEX_INT_DOWN:
			desc := "Vai Nahtang"; //Fred - Winchester criou com base em Vai Malandra
			compositor :="Elbert Casabrava";
			notas := {"E","D","C#","A","A","E","D","C#","C#","D","E","E","D","D","C#","C#","A","A","E","D","C#","A","A"};
			foreach nota in notas
				tempo[_nota_iter] := 500;
			endforeach
			//PartituraDebug(notas,tempo);
			break;
		BARD_INSPIRAR_COMPETENCIA: //SilverLeaf criou
			desc := "Mara Querida"; //Partita : Bach
			compositor := "Bachino Musir";
			notas := {"E2","A2","G#2","A2","C3","A2","E2","A","E2","A2","G#2","A2","C3","A2","E2","A","C2","E2","F2","G#","F2","E2","D2","C2","E2","G#2","A2","E2","D2","C2","B2","A"};
			tempo := {700,600,600,600,500,450,450,700,600,700,500,500,600,500,500,700,450,500,500,500,450,450,450,450,500,500,700,500,600,500,500,700};
			//PartituraDebug(notas,tempo);
			break;
		BARD_STR_DEX_DOWN: //Roberta Criou
			desc := "A Marcha Imperial"; //The Imperial March: John Williams
			compositor := "Cajuzinho Doce";
			notas := {"G","G","G","D#","A#","G","D#","A#","G","D2","D2","D2","D#2","A#","F#","D#","A#","G","G2","G","G","G2","F#2","F2","E2","D#2","E2","G#","C#2","C2","B","A#","A","A#","D#","F#","D#","F#","A#","G","A#","D2","G2","G","G","G2","F#2","F2","E2","D#2","E2","G#","C#2","C2","B","A#","A","A#","D#","F#","D#","A#","G","D#","A#","G"};
			tempo := {600,600,600,300,200,600,300,200,700,600,600,600,300,200,600,300,200,700,600,300,200,600,300,200,200,200,300,300,600,300,200,200,200,300,300,600,300,200,600,300,200,700,600,300,200,600,300,200,200,200,300,300,600,300,200,200,200,300,300,600,300,200,600,300,200,700};
			letra := {};
			//PartituraDebug(notas,tempo);
			break;
		BARD_INSPIRAR_CORAGEM: //essa partitura tá ruim
			desc := "Terra Querida"; //The Shire: Howard Shore
			compositor := "Anna del mar";
			notas := {"D","E","F#","A","F#","E","D","|","D","F#","A","B","D","C#2","A","F#","G","F#","E"};
			tempo := {750,750,750,750,750,750,750,750,750,750,450,750,750,750,450,750,750,750,750};
			break;
		BARD_FASCINAR:
			desc := "Ardiloso Marach";
			compositor := "Ully Nerfertari";
			notas := {"C", "G#", "|", "|", "C", "G", "|", "|", "D#", "A#", "|", "|", "C#", "G#", "|", "|", "C", "G", "|", "|", "F", "G", "|", "|", "D", "F", "|", "|", "D#", "C"};
			foreach nota in notas
				tempo[_nota_iter] := 550;
			endforeach
			break;
		BARD_ALUCINACAO:
			desc := "Hino do Exercito Zharkes"; //Contribuição de CJ com base em cancoes militares
			compositor := "Silvar, o Sabio";
			notas := {"C","D","E","F","E","D","C","A#","A","A","A#","C","G","A","A#","E","F","D","C","A","F","D","C","L","C","C","D","F","E","C","A","F","L","A","C","A","G","F","L","A","C","A","A#","G","L","C","C","D","F","E","C","A#","G","L","C","F","E","A","C","L","C","C","D","F","E","C","A","F","L","A","C","F","D","D","L","D","D","F","E","G","F","C","A","L","C","E","D","A#","G","L","A","A#","E","F","F","L","C","F","A","G","A#","A","G","F","L","A","A#","C","D","F","E","D","C","L","A","A#","F","E","D","A","A","L","A","A#","C","C","A#","A","D","D","L","G","A","A#","A#","E","E","E"};
			letra := {"Nos somos da Zhakia a guarda, fieis soldados","Por ela amados","Nas cores de nossa farda, rebrilha a gloria","Nossa vitoria","Em nosso Deus-Rei se encerra","Toda a esperanca","Que o povo Alcanca","Quando travarmos uma guerra","Rebrilha a gloria","Nossa a vitoria","A paz queremos com fervor","A guerra so nos causa dor","Porem se a Zharkia amada", "For um dia ultrajada","Lutaremos sem temor"};
			foreach nota in notas
				tempo[_nota_iter] := 250;
			endforeach
			break;
		BARD_DANCA_CONTAGIANTE:
			desc := "O Poderoso Chefao";  //Contribuicao da Roberta
			compositor := "Jooo do Pe de Alcachofra";
			notas := {"G","C2","D#2","D2","C2","D#2","C2","D2","C2","G#","A#","G","G","C2","D#2","D2","C2","D#2","C2","D2","C2","G","F#","F","F","G#","B","D2","F","G#","B","C2","C","D#","A#","G#","G","A#","G#","G#","G","G","A#","C","C2","C2","B","A#","D2","C2","G#","G","G","A#","G","F","F","G#","F#","G","G","C2","D#2","D2","C2","D#2","C2","D2","C2","G#","A#","G","G","C2","D#2","D2","C2","D#2","C2","D2","C2","G","F#","F","F","G#","B","D2","F","G#","B","C2","C","D#2","A#","G#","G","A#","G#","G#","G","G","B","C2"};
			tempo := {370,370,370,370,370,370,370,370,370,370,370,600,370,370,370,370,370,370,370,370,370,370,370,600,370,370,370,600,370,370,370,600,370,370,370,370,370,370,370,370,370,370,370,600,370,370,370,600,370,370,370,600,370,370,370,600,370,370,370,600,370,370,370,370,370,370,370,370,370,370,370,600,370,370,370,370,370,370,370,370,370,370,370,600,370,370,370,600,370,370,370,600,370,370,370,370,370,370,370,370,370,370,370,800};
/*
			//CJ criou com base em topgear:
			desc := "Noites na Taiga";
			compositor := "Brottor";
			notas := {"G","G","D#","C","G","G","D#","A#","F","F","D","A#","G","C","G","G","D#","G","G","D#","A#","F","F","D","A#","G","C","G","|","D#","|","C","|","G","|","C","G","A#","C","D#","D#","D","C","B","D#","A#","C","D#","D#","D","C","B","D#","A#","D#","G","B","F","D","F","G","G#","G","F","D#","G","D#","D","E","C","G#","|","D#","D","C","D","G","A#","|","F","D#","D","E","F","C","|","G#","G","F","G","D#","D","D","D","D#","F","E","D","A#","C","G","A#","C","D#","D#","D","C","B","D#","A#","D#","G","B","F","D","F","G","G#","G","F","D#","G","D#","D","E","C","C","G","A#","C","D#","D#","D","C","B","D#","A#","D","G","B","F","D","G","G#","G","F","D#","G","D#","D","E","C"};
			foreach nota in notas
				tempo[_nota_iter] := 200;
			endforeach
			break;
*/

/*
		BARD_INCITAR:

			break;
		BARD_ACALMAR:

			break;
		BARD_COPIA:

			break;
		BARD_FANTASMAS:

			break;
*/
		BARD_BLADE_DANCE:
			desc := "Lamina da Eldrad";
			compositor := "Desconhecido";
			notas := {"C", "D", "C", "D#", "C", "D", "|", "F", "F#", "F", "A", "|", "|", "F#", "F", "|", "F#", "F", "D#", "|", "D#", "F", "D#", "C", "|", "C", "C", "C"};
			foreach nota in notas
				tempo[_nota_iter] := 350;
			endforeach
			break;
		BARD_HAMELIN:
			desc := "Cancao de Hamelin";
			compositor := "Flautista";
			notas := {"C", "D", "E", "G", "A", "G", "D", "|", "A", "G", "A", "|", "D", "G", "A", "|", "|", "|", "C", "E", "C", "|", "C", "E", "D", "|", "C", "D", "E", "G", "A", "G", "C"};
			foreach nota in notas
				tempo[_nota_iter] := 300;
			endforeach
			break;
		BARD_SNAKE_CHARM:
			desc := "Noite em Unnam";
			compositor := "Desconhecido";
			notas := {"C", "D", "D#", "|", "D#", "D", "C", "|", "C", "D", "D#", "G", "D#", "D", "C", "|", "C", "D", "D#", "|", "D#", "D", "C", "|", "C", "D", "D#", "G", "D#", "D", "C"};
			foreach nota in notas
				tempo[_nota_iter] := 350;
			endforeach
			break;
		BARD_FIREDANCE:
			desc := "Flamula de Kandis Ra";
			compositor := "Desconhecido";
			notas := {"A", "|", "B", "|", "C", "|", "D", "|", "E", "A", "E", "A", "F", "A", "F", "A", "E", "A", "E", "A", "|", "D", "|", "C", "|", "B", "|", "A", "G#", "E", "G#", "E", "B", "E", "B", "E", "A", "A", "A", "A"};
			foreach nota in notas
				tempo[_nota_iter] := 250;
			endforeach
			break;
		BARD_EMUDECER:
			desc := "Silencio de Anlar";
			compositor := "Desconhecido";
			notas := {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B", "B", "A#", "A", "G#", "G", "F#", "F", "D#", "D", "C#", "C", "C#", "C", "C#", "C", "C#"};
			foreach nota in notas
				tempo[_nota_iter] := 450;
			endforeach
			break;
		BARD_BLOOM:
			desc := "Paixao Minguante";
			compositor := "Pokiehl";
			notas := {"C", "D", "E", "|", "C", "D", "E", "|", "C", "D", "E", "F", "E", "D", "E", "|", "G", "|", "F", "E", "G", "|", "F", "E", "G", "|", "F", "E", "D", "C"};
			foreach nota in notas
				tempo[_nota_iter] := 250;
			endforeach
			break;
		default:
			desc := "Paixao Minguante";
			compositor := "Pokiehl";
			notas := {"C", "D", "E", "|", "C", "D", "E", "|", "C", "D", "E", "F", "E", "D", "E", "|", "G", "|", "F", "E", "G", "|", "F", "E", "G", "|", "F", "E", "D", "C"};
			foreach nota in notas
				tempo[_nota_iter] := 250;
			endforeach
			break;
	endcase

//Atualiza as propriedades da partitura do jeito classico
	item.name := desc;
	var propriedades := {"+STR","+INT","-STR","+DEX","++STR","--STR","++DEX","--DEX","++INT","--INT","++STR<BR>++DEX","--STR<BR>--DEX","++STR<BR>++DEX<BR>++INT","--STR<BR>--DEX<BR>--INT","++PACIFICAS","++COMBATE","++FORT<BR>++REFL<BR>++VONT","FASCINAR","ALUCINAÇÃO","CONTAGIANTE","ANIMAR ESPADA","ATRAIR RATOS","ENCANTAR COBRAS","AGITAR FOGO","EMUDECER","FLORESCER","DESABILITADO 1","DESABILITADO 2","DESABILITADO 3","DESABILITADO 4"};
	item.desc := propriedades[magica];
	SetObjProperty(item, "compositor", compositor);
	SetObjProperty(item, "notas", notas);
	SetObjProperty(item, "notas2", notas2);
	SetObjProperty(item, "tempo", tempo);
	SetObjProperty(item, "letra", letra);
	SetObjProperty(item, "magica", magica);
	return 1;
endfunction



//funcao pra tocar as musicas simples de bardo
function tocarBasico(byref who, byref item) //Habilita os efeitos magicos da musica e manda tocar o som da musica
	if (!TemHabilidade(who, "Talento Musical") && !TemHabilidade(who, "Compositor Talentoso"))
		return basicoSom(who,item,0);//Efeito de som errado
	elseif(Cint(GetObjProperty(who, "#MagicaMusic")) > ReadGameClock()) //verifica se pode voltar a tocar
		SendSysMessage(who, "Voce deve esperar um momento antes de tocar novamente.",BARD_FONT,BARD_FONT_COLOR_BAD);
		return 0;
	else
		return basicoSom(who,item); //Efeito de som certo
	endif
endfunction

//funcao para tocar as partituras
function tocarPartitura(byref who, byref instrumento, byref partitura) //Habilita os efeitos magicos da partitura e manda tocar o som da musica
	if (!TemHabilidade(who, "Talento Musical") && !TemHabilidade(who, "Compositor Talentoso"))
		SendSysMessage(who, "Voce nao entende essa partitura.",BARD_FONT,BARD_FONT_COLOR_BAD);
		return 0;
	elseif(Cint(GetObjProperty(who, "#MagicaMusic")) > ReadGameClock()) //verifica se pode voltar a tocar
		SendSysMessage(who, "Voce deve esperar um momento antes de tocar novamente.",BARD_FONT,BARD_FONT_COLOR_BAD);
		return 0;
	endif
	return partituraSom(who,instrumento,partitura);
endfunction

//funcao para tocar sons do instrumento
function basicoSom(byref who, byref item, acerto:=1)
	var success,fail;
	//cria o gump de selecao de musicas basicas
	var bardgump := GFCreateGump();

	//dimensiona o gump
	//GFResizePic(bardgump, 0, 0, 2600, 200, 200);
	GFResizePic(bardgump, 20, 20, 3500, 160, 160);

	//preenche o gump (texto)
	GFTextLine(bardgump, 50, 33, 1890, "Escolha a melodia" );
	GFTextLine(bardgump, 60, 63, 600, "Comum" );
	GFTextLine(bardgump, 60, 83, 600, "Calma" );
	GFTextLine(bardgump, 60, 103, 600, "Triste" );
	GFTextLine(bardgump, 60, 123, 600, "Acao" );

	//preenche o gump (botao)
	var comum  := GFAddButton(bardgump, 45, 68, 2103, 2104, 1, BARD_COMUM);
	var calma  := GFAddButton(bardgump, 45, 88, 2103, 2104, 1, BARD_CALMA);
	var triste := GFAddButton(bardgump, 45, 108, 2103, 2104, 1, BARD_TRISTE);
	var acao   := GFAddButton(bardgump, 45, 128, 2103, 2104, 1, BARD_ACAO);

	//exibe o gump e coleta o valor do botao clicado
	var opcao := GFSendGump(who, bardgump);

	case (opcao[0])	//identifica o som a ser tocado em caso de sucesso e em caso de falha na musica
		BARD_COMUM:
			case(item.graphic)
				0x0e9c: //Tambor (Drum)
					success := 0x039; //drum01.wav
					fail := 0x03A; //drum01_b.wav
					break;
				0x0e9d: //Pandeiro (Tambourine)
				0x0e9e: //Pandeiro (Tambourine2)
					success := 0x053; //tamborin.wav
					fail := 0x054; //tambm_b.wav
					break;
				0x2805: //Flauta (flute)
				0x2807: //Flauta (flute2)
					success := 0x5B9; //satyrpipe_use_well.wav
					fail := 0x5B8; //satyrpipe_use_poorly.wav
					break;
				0x0EB3: //Alaude (lute)
				0x0EB4: //Alaude (lute)
					success := 0x04D; //lute01.wav
					fail := 0x04E; //lute01_b.wav
					break;
				0x0EB1: //Harp Sinfonica (standingharp)
					success := 0x044; //harp01.wav
					fail := 0x045; //harp01_b.wav
					break;
				0x0EB2: //Harpa (harp)
					success := 0x046; //harp02.wav
					fail := 0x047; //harp02_b.wav
					break;
				default:
					break;
			endcase
			break;
		BARD_CALMA:
			case(item.graphic)
				0x0e9c: //Tambor (Drum)
					success := 0x2EA; //drum_peace.eav
					fail := 0x03A; //drum01_b.wav
					break;
				0x0e9d: //Pandeiro (Tambourine)
				0x0e9e: //Pandeiro (Tambourine2)
					success := 0x4B8; //tamb_peace.wav
					fail := 0x054; //tambm_b.wav
					break;
				0x2805: //Flauta (flute)
				0x2807: //Flauta (flute2)
					success := 0x58C; //satyr_special2.wav
					fail := 0x5B8; //satyrpipe_use_poorly.wav
					break;
				0x0EB3: //Alaude (lute)
				0x0EB4: //Alaude (lute)
					success := 0x419; //lute_peace.wav
					fail := 0x04E; //lute01_b.wav
					break;
				0x0EB1: //Harp Sinfonica (standingharp)
					success := 0x394; //harp_peace.wav
					fail := 0x045; //harp01_b.wav
					break;
				0x0EB2: //Harpa (harp)
					success := 0x419; //lute_peace.wav
					fail := 0x047; //harp02_b.wav
					break;
				default:
					break;
			endcase
			break;
		BARD_TRISTE:
			case(item.graphic)
				0x0e9c: //Tambor (Drum)
					success := 0x2E9; //drum_drama.wav
					fail := 0x03A; //drum01_b.wav
					break;
				0x0e9d: //Pandeiro (Tambourine)
				0x0e9e: //Pandeiro (Tambourine2)
					success := 0x4B7; //tamb_drama.wav
					fail := 0x054; //tambm_b.wav
					break;
				0x2805: //Flauta (flute)
				0x2807: //Flauta (flute2)
					success := 0x58B; //satyr_special1.wav
					fail := 0x5B8; //satyrpipe_use_poorly.wav
					break;
				0x0EB3: //Alaude (lute)
				0x0EB4: //Alaude (lute)
					success := 0x40C; //lute_drama.wav
					fail := 0x04E; //lute01_b.wav
					break;
				0x0EB1: //Harp Sinfonica (standingharp)
					success := 0x393; //harp_drama.wav
					fail := 0x045; //harp01_b.wav
					break;
				0x0EB2: //Harpa (harp)
					success := 0x40C; //lute_drama.wav
					fail := 0x047; //harp02_b.wav
					break;
				default:
					break;
			endcase
			break;
		BARD_ACAO:
			case(item.graphic)
				0x0e9c: //Tambor (Drum)
					success := 0x2E8; //drum_battle.wav
					fail := 0x03A; //drum01_b.wav
				0x0e9d: //Pandeiro (Tambourine)
				0x0e9e: //Pandeiro (Tambourine2)
					success :=  0x4B6; //tamb_battle.wav
					fail := 0x054; //tambm_b.wav
				0x2805: //Flauta (flute)
				0x2807: //Flauta (flute2)
					success := 0x03E; //flute01.wav
					fail := 0x5B8; //satyrpipe_use_poorly.wav
				0x0EB3: //Alaude (lute)
				0x0EB4: //Alaude (lute)
					success := 0x404; //lute_battle.wav
					fail := 0x04E; //lute01_b.wav
					break;
				0x0EB1: //Harp Sinfonica (standingharp)
					success := 0x392; //harp_battle.wav
					fail := 0x045; //harp01_b.wav
					break;
				0x0EB2: //Harpa (harp)
					success := 0x404; //lute_battle.wav
					fail := 0x047; //harp02_b.wav
					break;
				default:
					break;
			endcase
			break;
		default:
			return 0;
			break;
	endcase

	//var check := 1;  //so utiliza o valor 1 enquanto nao houver teste de skill associado
	var check := acerto;
	//usar quando decidir testar alguma skill. ainda nao tem o include pra ela
	//check := Skillcheck(who, nome_skill, dificuldade, pts) ;// serve pra testar os pts de uma skill contra uma dificuldade. Avaliar criar uma funcao propria no pacote

	if (check > 0)
		//executa os efeitos mágicos da musica
		musicaEfeitos(who, opcao[0], 5000, item);
		//faz o UO tocar o som do item utilizado

		PlaySoundEffect(who, success);
	else
		SendSysMessage(who, "Voce nao sabe tocar o instrumento.",BARD_FONT,BARD_FONT_COLOR_BAD);
		PlaySoundEffect(who, fail);
	endif
	return 1;
endfunction

//funcao para tocar sons da partitura
function partituraSom(byref who,byref instrumento,byref partitura)

	//extrai da partitura os valores as informações que serão utilizadas
	var notas := GetObjProperty(partitura, "notas");
	var notas2 := GetObjProperty(partitura, "notas2");
	var tempo := GetObjProperty(partitura, "tempo");
	var letra := GetObjProperty(partitura, "letra");
	var magica := GetObjProperty(partitura, "magica");

	//Prepara os apontamentos para as notas do instrumento utilizado para tocar
	//Variaveis que vao armazenas os sons que devem ser tocados dependendo do tipo de instrumento
	var c,cs,d,ds,e,f,fs,g,gs,a,ass,b,c2,cs2,d2,ds2,e2,f2,fs2,g2,gs2,a2,ass2,b2,c3;

	var todasNotas := notas; //variavel para armazenar todas as notas
	var posicao := notas.Size(); //tamanho do primeiro conjunto de notas
	//concatena notas
	foreach nota in (notas2)
		todasNotas.Append(notas2[_nota_iter]);
	endforeach

	//variável para contar o trecho da musica a ser exibido
	var L := 1;

	//Seleciona os identificadores dos sons a serem tocados dependendo do intrumento
	case(instrumento.graphic)
		0x0eb1: //Harpa Sinfonica
			c := 0x49E;
			cs := 0x4A1;
			d := 0x4A3;
			ds := 0x4A5;
			e := 0x4A7;
			f := 0x4A9;
			fs := 0x4AB;
			g := 0x4AD;
			gs := 0x4AF;
			a := 0x498;
			ass := 0x49A;
			b := 0x49C;
			c2 := 0x49F;
			cs2 := 0x4A2;
			d2 := 0x4A4;
			ds2 := 0x4A6;
			e2 := 0x4A8;
			f2 := 0x4AA;
			fs2 := 0x4AC;
			g2 := 0x4AE;
			gs2 := 0x4B0;
			a2 := 0x499;
			ass2 := 0x49B;
			b2 := 0x49D;
			c3 := 0x4A0;
			break;
		0x0eb2: //Harpa
			c := 0x3D1;
			cs := 0x3D4;
			d := 0x3D6;
			ds := 0x3D8;
			e := 0x3DA;
			f := 0x3DC;
			fs := 0x3DE;
			g := 0x3E0;
			gs := 0x3E2;
			a := 0x3CB;
			ass := 0x3CD;
			b := 0x3CF;
			c2 := 0x3D2;
			cs2 := 0x3D5;
			d2 := 0x3D7;
			ds2 := 0x3D9;
			e2 := 0x3DB;
			f2 := 0x3DD;
			fs2 := 0x3DF;
			g2 := 0x3E1;
			gs2 := 0x3E3;
			a2 := 0x3CC;
			ass2 := 0x3CE;
			b2 := 0x3D0;
			c3 := 0x3D3;
			break;
		0x0eb3: //Alaude
			c := 0x405;
			cs := 0x408;
			d := 0x40A;
			ds := 0x40D;
			e := 0x40F;
			f := 0x411;
			fs := 0x413;
			g := 0x415;
			gs := 0x417;
			a := 0x3FE;
			ass := 0x400;
			b := 0x402;
			c2 := 0x406;
			cs2 := 0x409;
			d2 := 0x40B;
			ds2 := 0x40E;
			e2 := 0x410;
			f2 := 0x412;
			fs2 := 0x414;
			g2 := 0x416;
			gs2 := 0x418;
			a2 := 0x3FF;
			ass2 := 0x401;
			b2 := 0x403;
			c3 := 0x407;
			break;
		0x0eb4: //Alaude
			c := 0x405;
			cs := 0x408;
			d := 0x40A;
			ds := 0x40D;
			e := 0x40F;
			f := 0x411;
			fs := 0x413;
			g := 0x415;
			gs := 0x417;
			a := 0x3FE;
			ass := 0x400;
			b := 0x402;
			c2 := 0x406;
			cs2 := 0x409;
			d2 := 0x40B;
			ds2 := 0x40E;
			e2 := 0x410;
			f2 := 0x412;
			fs2 := 0x414;
			g2 := 0x416;
			gs2 := 0x418;
			a2 := 0x3FF;
			ass2 := 0x401;
			b2 := 0x403;
			c3 := 0x407;
			break;
		default:
			SendSysMessage(who, "Tente usar outro instrumento.",BARD_FONT,BARD_FONT_COLOR_BAD);
			return 0;
	endcase

	var duration := 0;
	foreach delay in tempo
		duration += delay;
	endforeach

	//se for um instrumento valido ele tenta executar o efeito magico da partitura
	if(musicaEfeitos(who, magica, duration, instrumento) == BARD_ENCERRA) //executa os efeitos das partituras e trata erros de target
		SendSysMessage(who, "Cancelado.",BARD_FONT,BARD_FONT_COLOR_BAD);
		return 0;
	endif

	//Toca os efeitos sonoros da partitura
	Set_Critical(1);
	foreach delay in tempo
		case (todasNotas[_delay_iter])
			"C":
				PlaySoundEffect(who, c);
				break;
			"C#":
				PlaySoundEffect(who, cs);
				break;
			"D":
				PlaySoundEffect(who, d);
				break;
			"D#":
				PlaySoundEffect(who, ds);
				break;
			"E":
				PlaySoundEffect(who, e);
				break;
			"F":
				PlaySoundEffect(who, f);
				break;
			"F#":
				PlaySoundEffect(who, fs);
				break;
			"G":
				PlaySoundEffect(who, g);
				break;
			"G#":
				PlaySoundEffect(who, gs);
				break;
			"A":
				PlaySoundEffect(who, a);
				break;
			"A#":
				PlaySoundEffect(who, ass);
				break;
			"B":
				PlaySoundEffect(who, b);
				break;
			"C2":
				PlaySoundEffect(who, c2);
				break;
			"C#2":
				PlaySoundEffect(who, cs2);
				break;
			"D2":
				PlaySoundEffect(who, d2);
				break;
			"D#2":
				PlaySoundEffect(who, ds2);
				break;
			"E2":
				PlaySoundEffect(who, e2);
				break;
			"F2":
				PlaySoundEffect(who, f2);
				break;
			"F#2":
				PlaySoundEffect(who, fs2);
				break;
			"G2":
				PlaySoundEffect(who, g2);
				break;
			"G#2":
				PlaySoundEffect(who, gs2);
				break;
			"A2":
				PlaySoundEffect(who, a2);
				break;
			"A#2":
				PlaySoundEffect(who, ass2);
				break;
			"B2":
				PlaySoundEffect(who, b2);
				break;
			"C3":
				PlaySoundEffect(who, c3);
				break;
			"L":
				PrintTextAbove(who, letra[L],BARD_FONT,BARD_FONT_COLOR_NEUTRA);
				L += 1;
				break;
			"|":
				break;
			default:
				return 0;
				break; //so coloquei o break por estetica
		endcase
		sleepms(delay); //intervalo entre as notas
	endforeach
	Set_Critical(0);
	return 1;
endfunction

function escrevePartitura(byref who, byref partitura)
	if (TemHabilidade(who, "Talento Musical") || TemHabilidade(who, "Compositor Talentoso"))
		printtextabove(who, "*Escrevendo a partitura*",BARD_FONT,0);
		PlaySoundEffect(who, 0x24a); //Toca som de escrita
		sleepms(2000);

		//Solicita o nome da musica
		var nomedamusica := Requestgump(who,"Qual o nome da Musica?","","",0);
		var magica;

		if (TemHabilidade(who, "Compositor Talentoso")) //muitas vezes mais chance que um bardo comum de conseguir escrever uma partitura lendária
			magica := 1 + RandomInt(60);
		else
			magica := 1 + RandomInt(1000);
		endif

		Set_Critical(1); //Força o codigo a rodar sem parar da criação da partitura escrita até a exclusão da partitura em branco
		var partituraNova := CreateItemInBackPack(who, "partitura", 1); //Cria uma partitura comum na backpack de "who"
		var propriedades := {"+STR","+INT","-STR","+DEX","++STR","--STR","++DEX","--DEX","++INT","--INT","++STR<BR>++DEX","--STR<BR>--DEX","++STR<BR>++DEX<BR>++INT","--STR<BR>--DEX<BR>--INT","++PACIFICAS","++COMBATE","++FORT<BR>++REFL<BR>++VONT","FASCINAR","ALUCINAÇÃO","CONTAGIANTE","ANIMAR ESPADA","ATRAIR RATOS","ENCANTAR COBRAS","AGITAR FOGO","EMUDECER","FLORESCER","DESABILITADO 1","DESABILITADO 2","DESABILITADO 3","DESABILITADO 4"};
		partituraNova.desc := propriedades[magica];
		Setobjproperty(partituraNova, "compositor", who.name);
		Setobjproperty(partituraNova, "notas", GetObjProperty(partitura,"notas"));
		Setobjproperty(partituraNova, "notas2", GetObjProperty(partitura,"notas2"));
		Setobjproperty(partituraNova, "tempo", GetObjProperty(partitura,"tempo"));
		Setobjproperty(partituraNova, "letra", GetObjProperty(partitura,"letra"));
		partituraNova.name := nomedamusica;
		if (magica <= (BARD_TOTAL_LENDARIAS+5))
			Setobjproperty(partituraNova, "magica", magica);
		else
			Setobjproperty(partituraNova, "magica", 1 + RandomInt(4));
		endif
		SubtractAmount(partitura,1);
		Set_Critical(0); //Finaliza o bloco de execução contínua

		PlaySoundEffect(who, 0x24a); //Toca som de escrita
		sleepms(2000);
		SendSysMessage(who, "Voce terminou de escrever a partitura",BARD_FONT,BARD_FONT_COLOR);
		return 1;
	else
		SendSysMessage(who, "Voce arruinou a partitura",BARD_FONT,0);
		printtextabove(who, "*Erra ao escrever as notas*",BARD_FONT_COLOR,BARD_FONT_COLOR_BAD);
		SubtractAmount(partitura,1);
		return 0;
	endif
endfunction


function comporPartitura(byref who,byref item)

	//Extrai da "Partitura em branco" os valores que já estiverem armazenados nela
	var notas := GetObjProperty(item, "notas");
	var notas2 := GetObjProperty(item, "notas2");
	var tempo := GetObjProperty(item, "tempo");
	var letra := GetObjProperty(item, "letra");
	var magica := GetObjProperty(item, "magica");

	if (!notas)
		notas := array;
	endif
	if (!notas2)
		notas2 := array;
	endif
	if (!tempo)
		tempo := array;
	endif
	if (!letra)
		letra := array;
	endif

	//junta todas as notas da partitura numa unica lista
	foreach nota in (notas2)
		notas.Append(nota);
	endforeach

	//prepara o conteudo da partitura para ser exibido
	var partituraTxt := "";
	var l := 1;
	foreach delay in (tempo)
		if (notas[_delay_iter] == "L") // "L" é o simbolo de Letra
			partituraTxt += "LETRA: " + letra[l] + " (" + CStr(delay) + "ms). ";
			l += 1;
		elseif (notas[_delay_iter] == "|")// "|" é o simbolo de Pausa
			partituraTxt += "PAUSA: (" + CStr(delay) + "ms). ";
		else
			partituraTxt += notas[_delay_iter] + " (" + CStr(delay) + "ms). ";
		endif
	endforeach
	//botao de escrever 1533, 1534
	//botão de apagar tudo 1535,1536
	//potao de apagar ultimo 1541,1542
	//botoes de notas 1594, 1595. Antigo 1645, 1646

	//Cria o gump
	var bardgump := GFCreateGump();
	GFClosable(bardgump, 1); //O gump PODE ser fechado com botão direito do mouse
	//Dimensiona o gump
	//GFResizePic(bardgump, 30, 30, 83, 750, 290); //2600
	GFResizePic(bardgump, 50, 50, 302, 710, 550); //302. Antigo 3500
	GFTextLine(bardgump, 70, 60, BARD_FONT_COLOR_NEUTRA, "Notas:" );

	//primeira oitava (botao)
	//                              1645. 1646
	GFAddButton(bardgump, 67, 92, 1594, 1595, 1, NOTA_C);
	GFAddButton(bardgump, 112, 92, 1594, 1595, 1, NOTA_CS);
	GFAddButton(bardgump, 157, 92, 1594, 1595, 1, NOTA_D);
	GFAddButton(bardgump, 202, 92, 1594, 1595, 1, NOTA_DS);
	GFAddButton(bardgump, 247, 92, 1594, 1595, 1, NOTA_E);
	GFAddButton(bardgump, 292, 92, 1594, 1595, 1, NOTA_F);
	GFAddButton(bardgump, 337, 92, 1594, 1595, 1, NOTA_FS);
	GFAddButton(bardgump, 382, 92, 1594, 1595, 1, NOTA_G);
	GFAddButton(bardgump, 427, 92, 1594, 1595, 1, NOTA_GS);
	GFAddButton(bardgump, 472, 92, 1594, 1595, 1, NOTA_A);
	GFAddButton(bardgump, 517, 92, 1594, 1595, 1, NOTA_ASS);
	GFAddButton(bardgump, 562, 92, 1594, 1595, 1, NOTA_B);

	GFAddButton(bardgump, 652, 103, 1594, 1595, 1, NOTA_P); //Pausa
	GFAddButton(bardgump, 697, 103, 1594, 1595, 1, NOTA_L); //Letra

	//segunda oitava (botao)
	GFAddButton(bardgump, 67, 122, 1594, 1595, 1, NOTA_C2);
	GFAddButton(bardgump, 112, 122, 1594, 1595, 1, NOTA_CS2);
	GFAddButton(bardgump, 157, 122, 1594, 1595, 1, NOTA_D2);
	GFAddButton(bardgump, 202, 122, 1594, 1595, 1, NOTA_DS2);
	GFAddButton(bardgump, 247, 122, 1594, 1595, 1, NOTA_E2);
	GFAddButton(bardgump, 292, 122, 1594, 1595, 1, NOTA_F2);
	GFAddButton(bardgump, 337, 122, 1594, 1595, 1, NOTA_FS2);
	GFAddButton(bardgump, 382, 122, 1594, 1595, 1, NOTA_G2);
	GFAddButton(bardgump, 427, 122, 1594, 1595, 1, NOTA_GS2);
	GFAddButton(bardgump, 472, 122, 1594, 1595, 1, NOTA_A2);
	GFAddButton(bardgump, 517, 122, 1594, 1595, 1, NOTA_ASS2);
	GFAddButton(bardgump, 562, 122, 1594, 1595, 1, NOTA_B2);

	//terceira oitava (botao)
	GFAddButton(bardgump, 67, 152, 1594, 1595, 1, NOTA_C3);

	//primeira oitava (nome)
	GFTextLine(bardgump, 85, 95, BARD_FONT_COLOR_LENDARIA, "C" );
	GFTextLine(bardgump, 125, 95, BARD_FONT_COLOR_LENDARIA, "C#" );
	GFTextLine(bardgump, 175, 95, BARD_FONT_COLOR_LENDARIA, "D" );
	GFTextLine(bardgump, 215, 95, BARD_FONT_COLOR_LENDARIA, "D#" );
	GFTextLine(bardgump, 265, 95, BARD_FONT_COLOR_LENDARIA, "E" );
	GFTextLine(bardgump, 310, 95, BARD_FONT_COLOR_LENDARIA, "F" );
	GFTextLine(bardgump, 350, 95, BARD_FONT_COLOR_LENDARIA, "F#" );
	GFTextLine(bardgump, 400, 95, BARD_FONT_COLOR_LENDARIA, "G" );
	GFTextLine(bardgump, 440, 95, BARD_FONT_COLOR_LENDARIA, "G#" );
	GFTextLine(bardgump, 490, 95, BARD_FONT_COLOR_LENDARIA, "A" );
	GFTextLine(bardgump, 530, 95, BARD_FONT_COLOR_LENDARIA, "A#" );
	GFTextLine(bardgump, 580, 95, BARD_FONT_COLOR_LENDARIA, "B" );

	GFTextLine(bardgump, 657, 105, BARD_FONT_COLOR_LENDARIA, "Pausa" ); //Pausa
	GFTextLine(bardgump, 702, 105, BARD_FONT_COLOR_LENDARIA, "Letra" ); //Letra

	//segunda oitava (nome)
	GFTextLine(bardgump, 80, 125, BARD_FONT_COLOR_LENDARIA, "C2" );
	GFTextLine(bardgump, 120, 125, BARD_FONT_COLOR_LENDARIA, "C#2" );
	GFTextLine(bardgump, 170, 125, BARD_FONT_COLOR_LENDARIA, "D2" );
	GFTextLine(bardgump, 210, 125, BARD_FONT_COLOR_LENDARIA, "D#2" );
	GFTextLine(bardgump, 260, 125, BARD_FONT_COLOR_LENDARIA, "E2" );
	GFTextLine(bardgump, 305, 125, BARD_FONT_COLOR_LENDARIA, "F2" );
	GFTextLine(bardgump, 345, 125, BARD_FONT_COLOR_LENDARIA, "F#2" );
	GFTextLine(bardgump, 395, 125, BARD_FONT_COLOR_LENDARIA, "G2" );
	GFTextLine(bardgump, 435, 125, BARD_FONT_COLOR_LENDARIA, "G#2" );
	GFTextLine(bardgump, 485, 125, BARD_FONT_COLOR_LENDARIA, "A2" );
	GFTextLine(bardgump, 525, 125, BARD_FONT_COLOR_LENDARIA, "A#2" );
	GFTextLine(bardgump, 575, 125, BARD_FONT_COLOR_LENDARIA, "B2" );

	//terceira oitava (nome)
	GFTextLine(bardgump, 80, 155, BARD_FONT_COLOR_LENDARIA, "C3" );


	//tempo entre as notas (texto)
	GFTextLine(bardgump, 675, 60, BARD_FONT_COLOR_NEUTRA, "Tempo(ms):" );

	//tempo entre as notas (entrada) O valor base é o valor anteriormente utilizado
	var tempo_anterior := CInt(tempo[tempo.size()]);
	if (!tempo_anterior)
		tempo_anterior := 600;
	endif
	GFTextEntry(bardgump, 675, 80, 255, 35, BARD_FONT_COLOR_GOOD,tempo_anterior);

	//Campo para exibicao das combinacoes de notas
	GFTextLine(bardgump, 80, 182, BARD_FONT_COLOR_NEUTRA, "Rascunho:" );
	GFHTMLArea(bardgump, 80, 202, 580, 360, CStr(partituraTxt),1,1);

	//Apagar ultima nota da combinacao
	GFTextLine(bardgump, 673, 134, BARD_FONT_COLOR_NEUTRA, "Backspace" );
	GFAddButton(bardgump, 652, 133, 1541, 1542, 1, NOTA_LIMPAR_ULTIMO);

	//Limpar combinacao
	GFTextLine(bardgump, 673, 155, BARD_FONT_COLOR_NEUTRA, "Limpar" );
	GFAddButton(bardgump, 652, 154, 1535, 1536, 1, NOTA_LIMPAR_TUDO);

	//Salvar combinacao numa partitura
	GFTextLine(bardgump, 673, 176, BARD_FONT_COLOR_NEUTRA, "Finalizar" );
	GFAddButton(bardgump, 652, 175, 1533, 1534, 1, NOTA_COMPOR_PARTITURA);

	//Opção de tocar
	GFTextLine(bardgump, 690, 262, BARD_FONT_COLOR_NEUTRA, "Tocar" );
	GFAddButton(bardgump, 675, 202, 5553, 5554, 1, NOTA_TOCAR);


	//Exibe o gump
	var entrada := GFSendGump(who, bardgump);

	if (!entrada[0]) //verifica se o Gump não foi fechado
		return 0;
	else
		tempo_anterior := CInt(entrada[2][4,len(entrada[2])]); //verifica sem o delay é válido
	endif

	if ((tempo_anterior < 100) || (tempo_anterior > 1000)) //de 400 pra cima funciona bem pra alaude, mas pra os outros engole notas abaixo de 450 de intervalo
		SendSysMessage(who, "Intervalo invalido. Tente usar intervalos de 100 a 1000.",BARD_FONT,BARD_FONT_COLOR_BAD);
		comporPartitura(who,item);
		return 0;
	else
		case (entrada[0])
			NOTA_C:
				notas.append("C");
				tempo.append(CInt(tempo_anterior));
				SetObjProperty(item, "notas", notas);
				SetObjProperty(item, "tempo", tempo);
				comporPartitura(who,item);
				break;
			NOTA_CS:
				notas.append("C#");
				tempo.append(tempo_anterior);
				SetObjProperty(item, "notas", notas);
				SetObjProperty(item, "tempo", tempo);
				comporPartitura(who,item);
				break;
			NOTA_D:
				notas.append("D");
				tempo.append(tempo_anterior);
				SetObjProperty(item, "notas", notas);
				SetObjProperty(item, "tempo", tempo);
				comporPartitura(who,item);
				break;
			NOTA_DS:
				notas.append("D#");
				tempo.append(tempo_anterior);
				SetObjProperty(item, "notas", notas);
				SetObjProperty(item, "tempo", tempo);
				comporPartitura(who,item);
				break;
			NOTA_E:
				notas.append("E");
				tempo.append(tempo_anterior);
				SetObjProperty(item, "notas", notas);
				SetObjProperty(item, "tempo", tempo);
				comporPartitura(who,item);
				break;
			NOTA_F:
				notas.append("F");
				tempo.append(tempo_anterior);
				SetObjProperty(item, "notas", notas);
				SetObjProperty(item, "tempo", tempo);
				comporPartitura(who,item);
				break;
			NOTA_FS:
				notas.append("F#");
				tempo.append(tempo_anterior);
				SetObjProperty(item, "notas", notas);
				SetObjProperty(item, "tempo", tempo);
				comporPartitura(who,item);
				break;
			NOTA_G:
				notas.append("G");
				tempo.append(tempo_anterior);
				SetObjProperty(item, "notas", notas);
				SetObjProperty(item, "tempo", tempo);
				comporPartitura(who,item);
				break;
			NOTA_GS:
				notas.append("G#");
				tempo.append(tempo_anterior);
				SetObjProperty(item, "notas", notas);
				SetObjProperty(item, "tempo", tempo);
				comporPartitura(who,item);
				break;
			NOTA_A:
				notas.append("A");
				tempo.append(tempo_anterior);
				SetObjProperty(item, "notas", notas);
				SetObjProperty(item, "tempo", tempo);
				comporPartitura(who,item);
				break;
			NOTA_ASS:
				notas.append("A#");
				tempo.append(tempo_anterior);
				SetObjProperty(item, "notas", notas);
				SetObjProperty(item, "tempo", tempo);
				comporPartitura(who,item);
				break;
			NOTA_B:
				notas.append("B");
				tempo.append(tempo_anterior);
				SetObjProperty(item, "notas", notas);
				SetObjProperty(item, "tempo", tempo);
				comporPartitura(who,item);
				break;
			NOTA_C2:
				notas.append("C2");
				tempo.append(tempo_anterior);
				SetObjProperty(item, "notas", notas);
				SetObjProperty(item, "tempo", tempo);
				comporPartitura(who,item);
				break;
			NOTA_CS2:
				notas.append("C#2");
				tempo.append(tempo_anterior);
				SetObjProperty(item, "notas", notas);
				SetObjProperty(item, "tempo", tempo);
				comporPartitura(who,item);
				break;
			NOTA_D2:
				notas.append("D2");
				tempo.append(tempo_anterior);
				SetObjProperty(item, "notas", notas);
				SetObjProperty(item, "tempo", tempo);
				comporPartitura(who,item);
				break;
			NOTA_DS2:
				notas.append("D#2");
				tempo.append(tempo_anterior);
				SetObjProperty(item, "notas", notas);
				SetObjProperty(item, "tempo", tempo);
				comporPartitura(who,item);
				break;
			NOTA_E2:
				notas.append("E2");
				tempo.append(tempo_anterior);
				SetObjProperty(item, "notas", notas);
				SetObjProperty(item, "tempo", tempo);
				comporPartitura(who,item);
				break;
			NOTA_F2:
				notas.append("F2");
				tempo.append(tempo_anterior);
				SetObjProperty(item, "notas", notas);
				SetObjProperty(item, "tempo", tempo);
				comporPartitura(who,item);
				break;
			NOTA_FS2:
				notas.append("F#2");
				tempo.append(tempo_anterior);
				SetObjProperty(item, "notas", notas);
				SetObjProperty(item, "tempo", tempo);
				comporPartitura(who,item);
				break;
			NOTA_G2:
				notas.append("G2");
				tempo.append(tempo_anterior);
				SetObjProperty(item, "notas", notas);
				SetObjProperty(item, "tempo", tempo);
				comporPartitura(who,item);
				break;
			NOTA_GS2:
				notas.append("G#2");
				tempo.append(tempo_anterior);
				SetObjProperty(item, "notas", notas);
				SetObjProperty(item, "tempo", tempo);
				comporPartitura(who,item);
				break;
			NOTA_A2:
				notas.append("A2");
				tempo.append(tempo_anterior);
				SetObjProperty(item, "notas", notas);
				SetObjProperty(item, "tempo", tempo);
				comporPartitura(who,item);
				break;
			NOTA_ASS2:
				notas.append("A#2");
				tempo.append(tempo_anterior);
				SetObjProperty(item, "notas", notas);
				SetObjProperty(item, "tempo", tempo);
				comporPartitura(who,item);
				break;
			NOTA_B2:
				notas.append("B2");
				tempo.append(tempo_anterior);
				SetObjProperty(item, "notas", notas);
				SetObjProperty(item, "tempo", tempo);
				comporPartitura(who,item);
				break;
			NOTA_C3:
				notas.append("C3");
				tempo.append(tempo_anterior);
				SetObjProperty(item, "notas", notas);
				SetObjProperty(item, "tempo", tempo);
				comporPartitura(who,item);
				break;
			NOTA_P:
				notas.append("|");
				tempo.append(tempo_anterior);
				SetObjProperty(item, "notas", notas);
				SetObjProperty(item, "tempo", tempo);
				comporPartitura(who,item);
				break;
			NOTA_L:
				notas.append("L");
				letra.append(Requestgump(who,"Qual o verso a ser adicionado?","","",0)); //Solicita o trecho de letra a ser adicionado
				tempo.append(tempo_anterior);
				SetObjProperty(item, "notas", notas);
				SetObjProperty(item, "tempo", tempo);
				SetObjProperty(item, "letra", letra);
				comporPartitura(who,item);
				break;
			NOTA_LIMPAR_TUDO:
				var input := YesNo(who,"Tem certeza que deseja excluir tudo?");
					if(input)
						notas := array;
						notas2 := array;
						letra := array;
						tempo := array;
						SetObjProperty(item, "notas", notas);
						SetObjProperty(item, "notas2", notas);
						SetObjProperty(item, "tempo", tempo);
						SetObjProperty(item, "letra", letra);
					endif
					comporPartitura(who,item);
				break;
			NOTA_LIMPAR_ULTIMO:
				if(notas[notas.size()] == "L")
					letra.erase(letra.size());
					SetObjProperty(item, "letra", letra);
				endif
				notas.erase(notas.size());
				tempo.erase(tempo.size());
				SetObjProperty(item, "notas", notas);
				SetObjProperty(item, "tempo", tempo);
				comporPartitura(who,item);
				break;
			NOTA_COMPOR_PARTITURA:
				foreach delay in (tempo) //verifica sem tem algum delay abusivo na partitura antes de tentar escrever a partitura
					if (delay > 2000)
						SendSysMessage(who, "Intervalos maiores que 2000 entre as notas nao sao permitidos.",BARD_FONT,BARD_FONT_COLOR_BAD);
						return 0;
					endif
				endforeach
				return escrevePartitura(who,item); //Salva a musica na partitura
				break;
			NOTA_TOCAR:
				foreach delay in (tempo)  //verifica sem tem delay abusivo
					if (delay > 2000)
						SendSysMessage(who, "Intervalos maiores que 2000ms entre as notas nao sao permitidos.",BARD_FONT,BARD_FONT_COLOR_BAD);
						return 0;
					endif
				endforeach

				//Solicita a seleção do instrumento a ser usado para tocar
				var instrumento;
				var cont := 0;
				repeat
					if (cont > 0) //caso não tenha encerrado o loop, exibe erro
						SendSysmessage (who, "Não consegue tocar o rascunho com isso.", BARD_FONT, BARD_FONT_COLOR_BAD); //Exibe no canto esquerdo
					endif
					if (cont < 3) //limita o numero de tentativas máximas de escolher instrumento antes de fechar a edição de partitura
						SendSysmessage (who, "Selecione um Instrumento Musical para tocar seu rascunho.", BARD_FONT, BARD_FONT_COLOR); //Exibe no canto esquerdo
						instrumento := Target(who);
						cont += 1;
					else
						SendSysmessage (who, "Não consegue tocar o rascunho com isso. Fechando o rascunho.", BARD_FONT, BARD_FONT_COLOR_BAD); //Exibe no canto esquerdo
						return 0;
					endif
				until ((instrumento[0].graphic == 0x0eb1) || (instrumento[0].graphic == 0x0eb2) || (instrumento[0].graphic == 0x0eb3) || (instrumento[0].graphic == 0x0eb4));

				//Executa o rascunho de paretitura com o instrumento selecionado
				PrintTextAbove(who, "*Lendo rascunho de partitura*",BARD_FONT,BARD_FONT_COLOR_BAD);
				partituraSom(who,instrumento[0],item); //Executa os sons da partitura
				comporPartitura(who,item); //após tocar o rascunho, volta para a edição
				break;
			default:
				SendSysMessage(who, "Cancelada a composicao.",BARD_FONT,BARD_FONT_COLOR_BAD);
				return 0;
				break; //break mantido apenas por estética do codigo
		endcase
	endif
	return 1;
endfunction

function PartituraDebug(notas, tempo)
	if (notas.size() != tempo.size())
		print("Notas: " + notas.size() + " |Tempo: " + tempo.size());
	else
		print("Tamanho de notas e tempo da partituras está ok!");
	endif
	foreach delay in (tempo)
		if (delay < 450 || delay > 2000)
			print("Partitura possui valores de tempo fora do aceitavel!");
			return 0;
		endif
	endforeach
endfunction

function AplicaEfeito(byref who, nomedoefeito, byref duration, byref modifier, byref range, tipoAlvo, mensagemSucesso, resistencia_mod)

	var resistiu;

	//Calcula o cubo de alcance dos poderes das musicas
	var x1 := who.x - range;
	var x2 := who.x + range;
	var y1 := who.y - range;
	var y2 := who.y + range;
	var z1 := who.z - range*2;
	var z2 := who.z + range*2;
	var mobiles := ListMobilesInBox(x1, y1, z1, x2, y2, z2, who.realm);

	case (tipoAlvo)
		ALVO_BUFF_TODOS:
			foreach mobile in mobiles
				if ((mobile.connected) && (mobile != who)) //Afeta alvos logados ou NPCs. Não afeta o próprio bardo
					TS_StartTimer(mobile, nomedoefeito, duration, modifier, who, 0);
					PrintTextAbovePrivate(mobile, mensagemSucesso, mobile, BARD_FONT, BARD_FONT_COLOR_GOOD);
				endif
			endforeach
			break;
		ALVO_BUFF_PLAYER:
			foreach mobile in mobiles
				if ((mobile.connected) && (mobile != who))
					if (mobile.acctname)
						TS_StartTimer(mobile, nomedoefeito, duration, modifier, who, 0);
						PrintTextAbovePrivate(mobile, mensagemSucesso, mobile, BARD_FONT, BARD_FONT_COLOR_GOOD);
					elseif (GetObjProperty(mobile,"owner"))
						TS_StartTimer(mobile, nomedoefeito, duration, modifier, who, 0);
						PrintTextAbovePrivate(mobile, mensagemSucesso, SystemFindObjectBySerial(CInt(GetObjProperty(mobile,"owner"))), BARD_FONT, BARD_FONT_COLOR_GOOD);
					endif
				endif
			endforeach
			break;
		ALVO_BUFF_NO_PLAYER:
			foreach mobile in mobiles
				if ((mobile.connected) && (mobile != who))
					if (!mobile.acctname)
						if (!GetObjProperty(mobile,"owner"))
							TS_StartTimer(mobile, nomedoefeito, duration, modifier, who, 0);
							PrintTextAbovePrivate(mobile, mensagemSucesso, mobile, BARD_FONT, BARD_FONT_COLOR_GOOD);
						endif
					endif
				endif
			endforeach
			break;
		ALVO_BUFF_PARTY:
			if (!who.Party) //trata em caso de nao estar em party, aplicando para todos os alvos
				foreach mobile in mobiles
					if ((mobile.connected) && (mobile != who))
						TS_StartTimer(mobile, nomedoefeito, duration, modifier, who, 0);
						PrintTextAbovePrivate(mobile, mensagemSucesso, mobile, BARD_FONT, BARD_FONT_COLOR_GOOD);
					endif
				endforeach
			else //Alvos membros da mesma Party
				var dono;
				foreach mobile in mobiles
					if ((mobile.connected) && (mobile != who))
						if ((mobile.acctname) && (who.Party == mobile.Party))
							TS_StartTimer(mobile, nomedoefeito, duration, modifier, who, 0);
							PrintTextAbovePrivate(mobile, mensagemSucesso, mobile, BARD_FONT, BARD_FONT_COLOR_GOOD);
						elseif (GetObjProperty(mobile,"owner"))
							dono := SystemFindObjectBySerial(CInt(GetObjProperty(mobile,"owner")));
							if (who.Party == dono.Party)
								TS_StartTimer(mobile, nomedoefeito, duration, modifier, who, 0);
								PrintTextAbovePrivate(mobile, mensagemSucesso, dono, BARD_FONT, BARD_FONT_COLOR_GOOD);
							endif
						endif
					endif
				endforeach
			endif
			break;
		ALVO_BUFF_NO_PARTY:
			if (!who.Party) //trata em caso de nao estar em party, aplicando para todos os alvos
				foreach mobile in mobiles
					if ((mobile.connected) && (mobile != who))
						TS_StartTimer(mobile, nomedoefeito, duration, modifier, who, 0);
						PrintTextAbovePrivate(mobile, mensagemSucesso, mobile, BARD_FONT, BARD_FONT_COLOR_GOOD);
					endif
				endforeach
			else //Alvos nao membros da mesma Party
				var dono;
				foreach mobile in mobiles
					if ((mobile.connected) && (mobile != who))
						if (mobile.acctname)
							if (who.Party != mobile.Party)
								TS_StartTimer(mobile, nomedoefeito, duration, modifier, who, 0);
								PrintTextAbovePrivate(mobile, mensagemSucesso, mobile, BARD_FONT, BARD_FONT_COLOR_GOOD);
							endif
						elseif (GetObjProperty(mobile,"owner"))
							dono := SystemFindObjectBySerial(CInt(GetObjProperty(mobile,"owner")));
							if (who.Party != dono.Party)
								TS_StartTimer(mobile, nomedoefeito, duration, modifier, who, 0);
								PrintTextAbovePrivate(mobile, mensagemSucesso, dono, BARD_FONT, BARD_FONT_COLOR_GOOD);
							endif
						else
							TS_StartTimer(mobile, nomedoefeito, duration, modifier, who, 0);
							PrintTextAbovePrivate(mobile, mensagemSucesso, dono, BARD_FONT, BARD_FONT_COLOR_GOOD);
						endif
					endif
				endforeach
			endif
			break;
		ALVO_BUFF_EVIL:
			foreach mobile in mobiles
				print (mobile.name + ":" +mobile.alignment);
				if ((mobile.connected) && (mobile != who) && (mobile.alignment == 1)) //Alvo Evil
					TS_StartTimer(mobile, nomedoefeito, duration, modifier, who, 0);
					PrintTextAbovePrivate(mobile, mensagemSucesso, mobile, BARD_FONT, BARD_FONT_COLOR_GOOD);
				endif
			endforeach
			break;
		ALVO_DEBUFF_TODOS:
			foreach mobile in mobiles
				if ((mobile.connected) && (mobile != who)) //Afeta alvos logados ou NPCs. Não afeta o próprio bardo
					resistiu := CheckResist(mobile, VONTADE, 30, resistencia_mod); //Alvo passou no teste de resistencia
					if(!resistiu)
						TS_StartTimer(mobile, nomedoefeito, duration, modifier, who, 0);
						PrintTextAbovePrivate(mobile, mensagemSucesso, mobile, BARD_FONT, BARD_FONT_COLOR_BAD);
					else
						PrintTextAbovePrivate(mobile, "resistiu", who, BARD_FONT, BARD_FONT_COLOR_NEUTRA); //avisa ao bardo que o alvo resistiu
					endif
				endif
			endforeach
			break;
		ALVO_DEBUFF_PLAYER:
			foreach mobile in mobiles
				if ((mobile.connected) && (mobile != who))
					if (mobile.acctname)
						resistiu := CheckResist(mobile, VONTADE, 30, resistencia_mod); //Alvo passou no teste de resistencia
						if(!resistiu)
							TS_StartTimer(mobile, nomedoefeito, duration, modifier, who, 0);
							PrintTextAbovePrivate(mobile, mensagemSucesso, mobile, BARD_FONT, BARD_FONT_COLOR_BAD);
						else
							PrintTextAbovePrivate(mobile, "resistiu", who, BARD_FONT, BARD_FONT_COLOR_NEUTRA); //avisa ao bardo que o alvo resistiu
						endif
					elseif (GetObjProperty(mobile,"owner"))
						resistiu := CheckResist(mobile, VONTADE, 30, resistencia_mod); //Alvo passou no teste de resistencia
						if(!resistiu)
							TS_StartTimer(mobile, nomedoefeito, duration, modifier, who, 0);
							PrintTextAbovePrivate(mobile, mensagemSucesso, SystemFindObjectBySerial(CInt(GetObjProperty(mobile,"owner"))), BARD_FONT, BARD_FONT_COLOR_BAD);
						else
							PrintTextAbovePrivate(mobile, "resistiu", SystemFindObjectBySerial(CInt(GetObjProperty(mobile,"owner"))), BARD_FONT, BARD_FONT_COLOR_NEUTRA); //avisa ao bardo que o alvo resistiu
						endif
					endif
				endif
			endforeach
			break;
		ALVO_DEBUFF_NO_PLAYER:
			foreach mobile in mobiles
				if ((mobile.connected) && (mobile != who))
					if (!mobile.acctname)
						if (!GetObjProperty(mobile,"owner"))
							resistiu := CheckResist(mobile, VONTADE, 30, resistencia_mod); //Alvo passou no teste de resistencia
							if(!resistiu)
								TS_StartTimer(mobile, nomedoefeito, duration, modifier, who, 0);
								PrintTextAbovePrivate(mobile, mensagemSucesso, mobile, BARD_FONT, BARD_FONT_COLOR_BAD);
							else
								PrintTextAbovePrivate(mobile, "resistiu", who, BARD_FONT, BARD_FONT_COLOR_NEUTRA); //avisa ao bardo que o alvo resistiu
							endif
						endif
					endif
				endif
			endforeach
			break;
		ALVO_DEBUFF_PARTY:
			if (!who.Party) //trata em caso de nao estar em party, aplicando para todos os alvos
				foreach mobile in mobiles
					if ((mobile.connected) && (mobile != who))
						resistiu := CheckResist(mobile, VONTADE, 30, resistencia_mod); //Alvo passou no teste de resistencia
						if(!resistiu)
							TS_StartTimer(mobile, nomedoefeito, duration, modifier, who, 0);
							PrintTextAbovePrivate(mobile, mensagemSucesso, mobile, BARD_FONT, BARD_FONT_COLOR_BAD);
						else
							PrintTextAbovePrivate(mobile, "resistiu", who, BARD_FONT, BARD_FONT_COLOR_NEUTRA); //avisa ao bardo que o alvo resistiu
						endif
					endif
				endforeach
			else //Alvos membros da mesma Party
				var dono;
				foreach mobile in mobiles
					if ((mobile.connected) && (mobile != who))
						if (mobile.acctname)
							if (who.Party == mobile.Party)
								resistiu := CheckResist(mobile, VONTADE, 30, resistencia_mod); //Alvo passou no teste de resistencia
								if(!resistiu)
									TS_StartTimer(mobile, nomedoefeito, duration, modifier, who, 0);
									PrintTextAbovePrivate(mobile, mensagemSucesso, mobile, BARD_FONT, BARD_FONT_COLOR_BAD);
								else
									PrintTextAbovePrivate(mobile, "resistiu", who, BARD_FONT, BARD_FONT_COLOR_NEUTRA); //avisa ao bardo que o alvo resistiu
								endif
							endif
						elseif (GetObjProperty(mobile,"owner"))
							dono := SystemFindObjectBySerial(CInt(GetObjProperty(mobile,"owner")));
							if (who.Party == dono.Party)
								resistiu := CheckResist(mobile, VONTADE, 30, resistencia_mod); //Alvo passou no teste de resistencia
								if(!resistiu)
									TS_StartTimer(mobile, nomedoefeito, duration, modifier, who, 0);
									PrintTextAbovePrivate(mobile, mensagemSucesso, dono, BARD_FONT, BARD_FONT_COLOR_BAD);
								else
									PrintTextAbovePrivate(mobile, "resistiu", who, BARD_FONT, BARD_FONT_COLOR_NEUTRA); //avisa ao bardo que o alvo resistiu
								endif
							endif
						endif
					endif
				endforeach
			endif
			break;
		ALVO_DEBUFF_NO_PARTY:
			if (!who.Party) //trata em caso de nao estar em party, aplicando para todos os alvos
				foreach mobile in mobiles
					if ((mobile.connected) && (mobile != who))
						resistiu := CheckResist(mobile, VONTADE, 30, resistencia_mod); //Alvo passou no teste de resistencia
						if(!resistiu)
							TS_StartTimer(mobile, nomedoefeito, duration, modifier, who, 0);
							PrintTextAbovePrivate(mobile, mensagemSucesso, mobile, BARD_FONT, BARD_FONT_COLOR_BAD);
						else
							PrintTextAbovePrivate(mobile, "resistiu", who, BARD_FONT, BARD_FONT_COLOR_NEUTRA); //avisa ao bardo que o alvo resistiu
						endif
					endif
				endforeach
			else //Alvos nao membros da mesma Party
				var dono;
				foreach mobile in mobiles
					if ((mobile.connected) && (mobile != who))
						if (mobile.acctname)
							if (who.Party != mobile.Party)
								resistiu := CheckResist(mobile, VONTADE, 30, resistencia_mod); //Alvo passou no teste de resistencia
								if(!resistiu)
									TS_StartTimer(mobile, nomedoefeito, duration, modifier, who, 0);
									PrintTextAbovePrivate(mobile, mensagemSucesso, mobile, BARD_FONT, BARD_FONT_COLOR_BAD);
								else
									PrintTextAbovePrivate(mobile, "resistiu", who, BARD_FONT, BARD_FONT_COLOR_NEUTRA); //avisa ao bardo que o alvo resistiu
								endif
							endif
						elseif (GetObjProperty(mobile,"owner"))
							dono := SystemFindObjectBySerial(CInt(GetObjProperty(mobile,"owner")));
							if (who.Party != dono.Party)
								resistiu := CheckResist(mobile, VONTADE, 30, resistencia_mod); //Alvo passou no teste de resistencia
								if(!resistiu)
									TS_StartTimer(mobile, nomedoefeito, duration, modifier, who, 0);
									PrintTextAbovePrivate(mobile, mensagemSucesso, dono, BARD_FONT, BARD_FONT_COLOR_BAD);
								else
									PrintTextAbovePrivate(mobile, "resistiu", who, BARD_FONT, BARD_FONT_COLOR_NEUTRA); //avisa ao bardo que o alvo resistiu
								endif
							endif
						else
							resistiu := CheckResist(mobile, VONTADE, 30, resistencia_mod); //Alvo passou no teste de resistencia
							if(!resistiu)
								TS_StartTimer(mobile, nomedoefeito, duration, modifier, who, 0);
								PrintTextAbovePrivate(mobile, mensagemSucesso, dono, BARD_FONT, BARD_FONT_COLOR_BAD);
							else
								PrintTextAbovePrivate(mobile, "resistiu", who, BARD_FONT, BARD_FONT_COLOR_NEUTRA); //avisa ao bardo que o alvo resistiu
							endif
						endif
					endif
				endforeach
			endif
			break;
		ALVO_DEBUFF_EVIL:
			foreach mobile in mobiles
				print (mobile.name + ":" +mobile.alignment);
				if ((mobile.connected) && (mobile != who) && (mobile.alignment == 1)) //Alvo Evil
					resistiu := CheckResist(mobile, VONTADE, 30, resistencia_mod); //Alvo passou no teste de resistencia
					if(!resistiu)
						TS_StartTimer(mobile, nomedoefeito, duration, modifier, who, 0);
						PrintTextAbovePrivate(mobile, mensagemSucesso, mobile, BARD_FONT, BARD_FONT_COLOR_BAD);
					else
						PrintTextAbovePrivate(mobile, "resistiu", who, BARD_FONT, BARD_FONT_COLOR_NEUTRA); //avisa ao bardo que o alvo resistiu
					endif
				endif
			endforeach
			break;
		endcase
	return 1;
endfunction



function musicaEfeitos(byref who, byref magica, byref tempo, byref instrumento) //DOING

	//Verifica os tipos de alvo selecionados pelo comando .bardo para Buffs e Debuffs
	var alvos;
	if (TemHabilidade(who, "Compositor Talentoso"))
		alvos := GetObjProperty(who, "BardTargets");
	endif
	if(!alvos)	//caso nao esteja definico o tipo de algo, inicializa com o padrao
		alvos := struct{"Buffs":=ALVO_BUFF_TODOS, "Debuffs":=ALVO_DEBUFF_TODOS};
		SetObjProperty(who, "BardTargets",struct{"Buffs":=ALVO_BUFF_TODOS, "Debuffs":=ALVO_DEBUFF_TODOS});
	endif

	var MagicLore := AP_GetSkill(who, MAGICLORE);

	//Variaveis importantes para os efeitos
	var modifier := 0;
	var BuffModifier,DebuffModifier;
	var range := 0;
	var duration := tempo;
	var sound_duration := tempo/1000;
	var resistencia_mod := - CInt(MagicLore/2.0);  //Redutor nos testes de resistência inimigos
	var HarmoniaPerfeita, Vituperio;

	//Calcula o modificador base da intensidade dos efeitos
	if(MagicLore < 40)
		modifier := 5 + CInt(MagicLore/10); //limite 5-9
		range := 2 + CInt(MagicLore/10); //limite 2-6
		duration  := (tempo * (1.0 + (MagicLore/400)))/1000; //limite 1.0-1.1 vezes o tempo base
	elseif (MagicLore < 70)
		modifier := 5 + CInt(MagicLore/5); //limite 13-19
		range := 5 + CInt(MagicLore/10); //limite 9-12
		duration  := (tempo * (1.0 + (MagicLore/200)))/1000; //limite 1.2-1.35 vezes o tempo base
	elseif (MagicLore < 100)
		modifier := 5 + CInt(MagicLore/4); //limite 22-30
		range := CInt(MagicLore/5); //limite 14-20
		duration  := (tempo * (1.0 + (MagicLore/100)))/1000; //limite 1.7-2.0 vezes o tempo base
	elseif (MagicLore >= 100) //Lendário
		modifier := 5 + CInt(MagicLore/2); //limite 55-60
		range := CInt(MagicLore/5); //limite 14-20
		duration  := (tempo * (1.0 + (MagicLore/50)))/1000; //limite 3.0-3.2 vezes o tempo base
	else ///caso de MagicLore negativo
		modifier := 1; //limite 1
		range := 1; //limite 1
		duration  := CInt(tempo/2000); //limite 1.0-1.1 vezes o tempo base
	endif

	//Aplica os efeitos de várias habilidades de nível

	if (TemHabilidade(who, "Musica Persistente"))
		duration := duration * 1.5; //dobra a duração base das cancoes
	endif
	if (TemHabilidade(who, "Musica Alta"))
		range := range + 5; //aumenta a area de efeito das cancoes
	endif

	BuffModifier := modifier;  //porder base dos Buffs
	DebuffModifier := modifier; //poder base do Debuffs

	//Habilita eficacia extra nos buffs e debuffs
	if (TemHabilidade(who, "Musica Contagiante"))
		BuffModifier := modifier * 1.2;  //aumenta o porder dos Buffs
	endif

	if (TemHabilidade(who, "Musica da Alma"))
		DebuffModifier := modifier * 1.2;  //aumenta o porder dos Debuffs
		resistencia_mod := resistencia_mod - 20; //torna os Debuffs mais dificies de resistir
	endif

	if (TemHabilidade(who, "Mestre do Improviso"))
		BuffModifier := modifier * 1.2;
		DebuffModifier := modifier * 1.2;  //aumenta o porder dos Debuffs
		resistencia_mod := resistencia_mod - 40; //torna os Debuffs muito mais dificies de resistir
	endif

	//Habilita efeitos extras nos buffs OU debuffs, nunca os dois

	HarmoniaPerfeita := 0;
	Vituperio := 0;

	if (TemHabilidade(who, "Harmonia Perfeita"))
		HarmoniaPerfeita := 1;
	endif
	if (TemHabilidade(who, "Vituperio"))
		HarmoniaPerfeita := 0;
		Vituperio := 1;
	endif

	//Aplica efeitos dos materiais
	var material := GetObjProperty(instrumento, "material");

	case (material)
		0x1bd7: //madeira
			break;
		0xee66: //pinheiro
			BuffModifier := BuffModifier*1.1;
			DebuffModifier := DebuffModifier*0.9;
			break;
		0xee67: //carvalho
			BuffModifier := BuffModifier*0.9;
			DebuffModifier := DebuffModifier*1.1;
			break;
		0xee68: //nogueira
			resistencia_mod := resistencia_mod-10; //mais dificil resistir
			range := range - 1;
			duration := duration *0.9;
			break;
		0xee70: //cipreste
			resistencia_mod := resistencia_mod+10; //mais facil resistir
			range := range + 1;
			duration := duration *1.1;
			break;
		0xee69: //salgueiro
			BuffModifier := BuffModifier*1.2;
			DebuffModifier := DebuffModifier*0.8;
			resistencia_mod := resistencia_mod+10;
			break;
		0xee73: //polar
			BuffModifier := BuffModifier*0.9;
			DebuffModifier := DebuffModifier*0.9;
			range := range+3;
			duration := duration * 1.2;
			break;
		0x615b: //teixo
			BuffModifier := BuffModifier*1.3;
			DebuffModifier := DebuffModifier*0.7;
			range := range-3;
			duration := duration * 0.7;
			break;
		0xee64: //imbuia
			BuffModifier := BuffModifier*0.7;
			DebuffModifier := DebuffModifier*1.3;
			resistencia_mod := resistencia_mod-20; //mais dificil resistir
			range := range-3;
			duration := duration * 0.7;
			break;
		0xee65: //cogumelo
			BuffModifier := BuffModifier*0.3;
			DebuffModifier := DebuffModifier*1.4;
			resistencia_mod := resistencia_mod-30; //mais dificil resistir
			range := range-5;
			duration := duration * 0.5;
			break;
		0xef2a: //mogno
			BuffModifier := BuffModifier*1.4;
			DebuffModifier := DebuffModifier*0.3;
			range := range-5;
			duration := duration * 0.5;
			break;
		0xff05: //ebano
			BuffModifier := BuffModifier*1.3;
			DebuffModifier := DebuffModifier*1.3;
			resistencia_mod := resistencia_mod-40; //mais dificil resistir
			range := range-10;
			duration := duration * 0.25;
			break;
		default:
			break;
	endcase

	//Ajusta as variaveis para numeros inteiros
	BuffModifier := CInt(BuffModifier);
	DebuffModifier := CInt(DebuffModifier);
	duration := CInt(duration);

	var resistiu;

	case (magica) //criar a funcao aplicaEfeito(bardo,nomedoefeito,duracao,modifier,range,tipodealvo)
		BARD_COMUM:
			PrintTextAbove(who, "*Tocando musica encorajadora*", BARD_FONT, BARD_FONT_COLOR_COMUM);
			TS_StartTimer(who, "bard_tocando", sound_duration, 1, who, 0);
			TS_StartTimer(who, "bard_segurando_instrumento", sound_duration, 1, who, 0);
			if(HarmoniaPerfeita)
				AplicaEfeito(who,"bard_str_up",duration,CInt(BuffModifier*1.0),range,alvos.Buffs,"*bem mais forte*",resistencia_mod);
			else
				AplicaEfeito(who,"bard_str_up",duration,CInt(BuffModifier*0.5),range,alvos.Buffs,"*um pouco mais forte*",resistencia_mod);
			endif
			break;
		BARD_CALMA:
			PrintTextAbove(who, "*Tocando musica inspiradora*", BARD_FONT, BARD_FONT_COLOR_COMUM);
			TS_StartTimer(who, "bard_tocando", sound_duration, 1, who, 0);
			TS_StartTimer(who, "bard_segurando_instrumento", sound_duration, 1, who, 0);
			if(HarmoniaPerfeita)
				AplicaEfeito(who,"bard_int_up",duration,CInt(BuffModifier*1.0),range,alvos.Buffs,"*bem mais esperto*",resistencia_mod);
			else
				AplicaEfeito(who,"bard_int_up",duration,CInt(BuffModifier*0.5),range,alvos.Buffs,"*um pouco mais esperto*",resistencia_mod);
			endif
			break;
		BARD_TRISTE:
			PrintTextAbove(who, "*Tocando musica desencorajadora*", BARD_FONT, BARD_FONT_COLOR_COMUM);
			TS_StartTimer(who, "bard_tocando", sound_duration, 1, who, 0);
			TS_StartTimer(who, "bard_segurando_instrumento", sound_duration, 1, who, 0);
			if(Vituperio)
				AplicaEfeito(who,"bard_str_down",duration,CInt(DebuffModifier*1.0),range,alvos.Debuffs,"*bem mais fraco*",resistencia_mod);
			else
				AplicaEfeito(who,"bard_str_down",duration,CInt(DebuffModifier*0.5),range,alvos.Debuffs,"*um pouco mais fraco*",resistencia_mod);
			endif
			break;
		BARD_ACAO:
			PrintTextAbove(who, "*Tocando musica empolgante*", BARD_FONT, BARD_FONT_COLOR_COMUM);
			TS_StartTimer(who, "bard_tocando", sound_duration, 1, who, 0);
			TS_StartTimer(who, "bard_segurando_instrumento", sound_duration, 1, who, 0);
			if(HarmoniaPerfeita)
				AplicaEfeito(who,"bard_dex_up",duration,CInt(BuffModifier*1.0),range,alvos.Buffs,"*bem mais ágil*",resistencia_mod);
			else
				AplicaEfeito(who,"bard_dex_up",duration,CInt(BuffModifier*0.5),range,alvos.Buffs,"*um pouco mais ágil*",resistencia_mod);
			endif
			break;
		BARD_STR_UP:
			PrintTextAbove(who, "*Tocando musica encorajadora*", BARD_FONT, BARD_FONT_COLOR_COMUM);
			TS_StartTimer(who, "bard_tocando", sound_duration, 2, who, 0);
			TS_StartTimer(who, "bard_segurando_instrumento", sound_duration, 1, who, 0);
			if(HarmoniaPerfeita)
				AplicaEfeito(who,"bard_str_up",duration,CInt(BuffModifier*1.2),range,alvos.Buffs,"*muito mais forte*",resistencia_mod);
			else
				AplicaEfeito(who,"bard_str_up",duration,CInt(BuffModifier),range,alvos.Buffs,"*bem mais forte*",resistencia_mod);
			endif
			break;
		BARD_STR_DOWN:
			PrintTextAbove(who, "*Tocando musica desencorajadora*", BARD_FONT, BARD_FONT_COLOR_COMUM);
			TS_StartTimer(who, "bard_tocando", sound_duration, 2, who, 0);
			TS_StartTimer(who, "bard_segurando_instrumento", sound_duration, 1, who, 0);
			if(Vituperio)
				AplicaEfeito(who,"bard_str_down",duration,CInt(DebuffModifier*1.2),range,alvos.Debuffs,"*muito mais fraco*",resistencia_mod);
			else
				AplicaEfeito(who,"bard_str_down",duration,CInt(DebuffModifier),range,alvos.Debuffs,"*bem mais fraco*",resistencia_mod);
			endif
			break;
		BARD_DEX_UP:
			PrintTextAbove(who, "*Tocando musica empolgante*", BARD_FONT, BARD_FONT_COLOR_COMUM);
			TS_StartTimer(who, "bard_tocando", sound_duration, 2, who, 0);
			TS_StartTimer(who, "bard_segurando_instrumento", sound_duration, 1, who, 0);
			if(HarmoniaPerfeita)
				AplicaEfeito(who,"bard_dex_up",duration,CInt(BuffModifier*1.2),range,alvos.Buffs,"*muito mais ágil*",resistencia_mod);
			else
				AplicaEfeito(who,"bard_dex_up",duration,CInt(BuffModifier),range,alvos.Buffs,"*bem mais ágil*",resistencia_mod);
			endif
			break;
		BARD_DEX_DOWN:
			PrintTextAbove(who, "*Tocando musica desanimadora*", BARD_FONT, BARD_FONT_COLOR_COMUM);
			TS_StartTimer(who, "bard_tocando", sound_duration, 2, who, 0);
			TS_StartTimer(who, "bard_segurando_instrumento", sound_duration, 1, who, 0);
			if(Vituperio)
				AplicaEfeito(who,"bard_dex_down",duration,CInt(DebuffModifier*1.2),range,alvos.Debuffs,"*muito mais lento*",resistencia_mod);
			else
				AplicaEfeito(who,"bard_dex_down",duration,CInt(DebuffModifier),range,alvos.Debuffs,"*bem mais lento*",resistencia_mod);
			endif
			break;
		BARD_INT_UP:
			PrintTextAbove(who, "*Tocando musica inspiradora*", BARD_FONT, BARD_FONT_COLOR_COMUM);
			TS_StartTimer(who, "bard_tocando", sound_duration, 2, who, 0);
			TS_StartTimer(who, "bard_segurando_instrumento", sound_duration, 1, who, 0);
			if(HarmoniaPerfeita)
				AplicaEfeito(who,"bard_int_up",duration,CInt(BuffModifier*1.2),range,alvos.Buffs,"*muito mais esperto*",resistencia_mod);
			else
				AplicaEfeito(who,"bard_int_up",duration,CInt(BuffModifier),range,alvos.Buffs,"*bem mais esperto*",resistencia_mod);
			endif
			break;
		BARD_INT_DOWN:
			PrintTextAbove(who, "*Tocando musica desconcertante*", BARD_FONT, BARD_FONT_COLOR_COMUM);
			TS_StartTimer(who, "bard_tocando", sound_duration, 2, who, 0);
			TS_StartTimer(who, "bard_segurando_instrumento", sound_duration, 1, who, 0);
			if(Vituperio)
				AplicaEfeito(who,"bard_int_down",duration,CInt(DebuffModifier),range,alvos.Debuffs,"*muito mais tapado*",resistencia_mod);
			else
				AplicaEfeito(who,"bard_int_down",duration,CInt(DebuffModifier),range,alvos.Debuffs,"*bem mais tapado*",resistencia_mod);
			endif
			break;
		BARD_STR_DEX_UP:
			PrintTextAbove(who, "*Tocando musica motivadora*", BARD_FONT, BARD_FONT_COLOR_COMUM);
			TS_StartTimer(who, "bard_tocando", sound_duration, 3, who, 0);
			TS_StartTimer(who, "bard_segurando_instrumento", sound_duration, 1, who, 0);
			if(HarmoniaPerfeita)
				AplicaEfeito(who,"bard_str_up",duration,CInt(BuffModifier),range,alvos.Buffs,"*bem mais forte*",resistencia_mod);
				AplicaEfeito(who,"bard_dex_up",duration,CInt(BuffModifier),range,alvos.Buffs,"*bem mais ágil*",resistencia_mod);
			else
				AplicaEfeito(who,"bard_str_up",duration,CInt(BuffModifier*0.9),range,alvos.Buffs,"*mais forte*",resistencia_mod);
				AplicaEfeito(who,"bard_dex_up",duration,CInt(BuffModifier*0.9),range,alvos.Buffs,"*mais ágil*",resistencia_mod);
			endif
			break;
		BARD_STR_DEX_DOWN:
			PrintTextAbove(who, "*Tocando musica desmotivante*", BARD_FONT, BARD_FONT_COLOR_COMUM);
			TS_StartTimer(who, "bard_tocando", sound_duration, 3, who, 0);
			TS_StartTimer(who, "bard_segurando_instrumento", sound_duration, 1, who, 0);
			if(Vituperio)
				AplicaEfeito(who,"bard_str_down",duration,CInt(DebuffModifier),range,alvos.Debuffs,"*bem mais fraco*",resistencia_mod);
				AplicaEfeito(who,"bard_dex_down",duration,CInt(DebuffModifier),range,alvos.Debuffs,"*bem mais lento*",resistencia_mod);
			else
				AplicaEfeito(who,"bard_str_down",duration,CInt(DebuffModifier*0.9),range,alvos.Debuffs,"*mais fraco*",resistencia_mod);
				AplicaEfeito(who,"bard_dex_down",duration,CInt(DebuffModifier*0.9),range,alvos.Debuffs,"*mais lento*",resistencia_mod);
			endif
			break;
		BARD_STR_DEX_INT_UP:
			PrintTextAbove(who, "*Tocando musica motivadora*", BARD_FONT, BARD_FONT_COLOR_COMUM);
			TS_StartTimer(who, "bard_tocando", sound_duration, 4, who, 0);
			TS_StartTimer(who, "bard_segurando_instrumento", sound_duration, 1, who, 0);
			if(HarmoniaPerfeita)
				AplicaEfeito(who,"bard_str_up",duration,CInt(BuffModifier*0.9),range,alvos.Buffs,"*mais forte*",resistencia_mod);
				AplicaEfeito(who,"bard_dex_up",duration,CInt(BuffModifier*0.9),range,alvos.Buffs,"*mais ágil*",resistencia_mod);
				AplicaEfeito(who,"bard_int_up",duration,CInt(BuffModifier*0.9),range,alvos.Buffs,"*mais esperto*",resistencia_mod);
			else
				AplicaEfeito(who,"bard_str_up",duration,CInt(BuffModifier*0.8),range,alvos.Buffs,"*um tanto mais forte*",resistencia_mod);
				AplicaEfeito(who,"bard_dex_up",duration,CInt(BuffModifier*0.8),range,alvos.Buffs,"*um tanto mais ágil*",resistencia_mod);
				AplicaEfeito(who,"bard_int_up",duration,CInt(BuffModifier*0.8),range,alvos.Buffs,"*um tanto mais esperto*",resistencia_mod);
			endif
			break;
		BARD_STR_DEX_INT_DOWN:
			PrintTextAbove(who, "*Tocando musica desmotivante*", BARD_FONT, BARD_FONT_COLOR_COMUM);
			TS_StartTimer(who, "bard_tocando", sound_duration, 4, who, 0);
			TS_StartTimer(who, "bard_segurando_instrumento", sound_duration, 1, who, 0);
			if(Vituperio)
				AplicaEfeito(who,"bard_str_down",duration,CInt(DebuffModifier*0.9),range,alvos.Debuffs,"*um tanto mais fraco*",resistencia_mod);
				AplicaEfeito(who,"bard_dex_down",duration,CInt(DebuffModifier*0.9),range,alvos.Debuffs,"*um tanto mais lento*",resistencia_mod);
				AplicaEfeito(who,"bard_int_down",duration,CInt(DebuffModifier*0.9),range,alvos.Debuffs,"*um tanto mais tapado*",resistencia_mod);
			else
				AplicaEfeito(who,"bard_str_down",duration,CInt(DebuffModifier*0.8),range,alvos.Debuffs,"*um tanto mais fraco*",resistencia_mod);
				AplicaEfeito(who,"bard_dex_down",duration,CInt(DebuffModifier*0.8),range,alvos.Debuffs,"*um tanto mais lento*",resistencia_mod);
				AplicaEfeito(who,"bard_int_down",duration,CInt(DebuffModifier*0.8),range,alvos.Debuffs,"*um tanto mais tapado*",resistencia_mod);
			endif
			break;
		BARD_INSPIRAR_COMPETENCIA: //concede buffs em pericias não combativas nem magicas a UM aliado escolhido
			PrintTextAbove(who, "*Tocando musica ispiradora*", BARD_FONT, BARD_FONT_COLOR); //Exibe sobre cabeca do bardo
			TS_StartTimer(who, "bard_tocando", sound_duration, 6, who, 0);
			TS_StartTimer(who, "bard_segurando_instrumento", sound_duration, 1, who, 0);
			if(HarmoniaPerfeita)
				AplicaEfeito(who,"bard_inspirar_competencia",duration*1.2,CInt(BuffModifier),range,alvos.Buffs,"*pronto pro trabalho*",resistencia_mod);
			else
				AplicaEfeito(who,"bard_inspirar_competencia",duration,CInt(BuffModifier),range,alvos.Buffs,"*pronto pro trabalho*",resistencia_mod);
			endif
			break;
		BARD_INSPIRAR_HEROISMO: //concede bônus de combate a UM aliado escolhido
			PrintTextAbove(who, "*Tocando musica inspiradora*", BARD_FONT, BARD_FONT_COLOR); //Exibe sobre cabeca do bardo
			TS_StartTimer(who, "bard_tocando", sound_duration, 6, who, 0);
			TS_StartTimer(who, "bard_segurando_instrumento", sound_duration, 1, who, 0);
			if(HarmoniaPerfeita)
				AplicaEfeito(who,"bard_inspirar_heroismo",duration*1.2,CInt(BuffModifier),range,alvos.Buffs,"*pronto pro combate*",resistencia_mod);
			else
				AplicaEfeito(who,"bard_inspirar_heroismo",duration,CInt(BuffModifier),range,alvos.Buffs,"*pronto pro combate*",resistencia_mod);
			endif
			break;
		BARD_INSPIRAR_CORAGEM: //aumenta todas as resistências em um percentual. Preciso ver como fazer isso ainda
			PrintTextAbove(who, "*Tocando musica inspiradora*", BARD_FONT, BARD_FONT_COLOR); //Exibe sobre cabeca do bardo
			TS_StartTimer(who, "bard_tocando", sound_duration, 4, who, 0);
			TS_StartTimer(who, "bard_segurando_instrumento", sound_duration, 1, who, 0);
			if(HarmoniaPerfeita)
				AplicaEfeito(who,"bard_inspirar_coragem",duration*1.2,CInt(BuffModifier),range,alvos.Buffs,"*Cheio de si*",resistencia_mod);
			else
				AplicaEfeito(who,"bard_inspirar_coragem",duration,CInt(BuffModifier),range,alvos.Buffs,"*Cheio de si*",resistencia_mod);
			endif
			break;
		break;
		BARD_FASCINAR:
			PrintTextAbove(who, "*Tocando musica fascinante*", BARD_FONT, BARD_FONT_COLOR_COMUM);
			TS_StartTimer(who, "bard_tocando", sound_duration, 6, who, 0);
			TS_StartTimer(who, "bard_segurando_instrumento", sound_duration, 1, who, 0);
			if(Vituperio)
				AplicaEfeito(who,"bard_fascinar",duration*0.5,CInt(DebuffModifier),range,alvos.Debuffs,"*Fascinado pelo som*",resistencia_mod);
				AplicaEfeito(who,"bard_contagiante",duration*0.5,CInt(DebuffModifier),range,alvos.Debuffs,"*Dancando*",resistencia_mod);
			else
				AplicaEfeito(who,"bard_fascinar",duration*0.5,CInt(DebuffModifier),range,alvos.Debuffs,"*Fascinado*",resistencia_mod);
			endif
			break;
		BARD_ALUCINACAO:
			PrintTextAbove(who, "*Tocando musica aluciante*", BARD_FONT, BARD_FONT_COLOR_COMUM);
			TS_StartTimer(who, "bard_tocando", sound_duration, 3, who, 0);
			TS_StartTimer(who, "bard_segurando_instrumento", sound_duration, 1, who, 0);
			if(Vituperio)
				AplicaEfeito(who,"bard_alucinacao",duration,CInt(DebuffModifier),range,alvos.Debuffs,"*Alucinado*",resistencia_mod);
				AplicaEfeito(who,"bard_str_down",duration,CInt(DebuffModifier*0.5),range,alvos.Debuffs,"*um pouco mais fraco*",resistencia_mod);
				AplicaEfeito(who,"bard_dex_down",duration,CInt(DebuffModifier*0.5),range,alvos.Debuffs,"*um pouco mais lento*",resistencia_mod);
				AplicaEfeito(who,"bard_int_down",duration,CInt(DebuffModifier*0.5),range,alvos.Debuffs,"*um pouco mais tapado*",resistencia_mod);
			else
				AplicaEfeito(who,"bard_alucinacao",duration,CInt(DebuffModifier),range,alvos.Debuffs,"*Alucinado*",resistencia_mod);
			endif
			break;
		BARD_DANCA_CONTAGIANTE: //força todos os que estão na área da música a dançarem) faz até aleijado dançar, afinal, é lendaria
			PrintTextAbove(who, "*Tocando musica contagiante*", BARD_FONT, BARD_FONT_COLOR_COMUM);
			TS_StartTimer(who, "bard_tocando", sound_duration, 4, who, 0);
			TS_StartTimer(who, "bard_segurando_instrumento", sound_duration, 1, who, 0);
			if(Vituperio)
				AplicaEfeito(who,"bard_contagiante",duration,CInt(DebuffModifier),range,alvos.Debuffs,"*dança muito empolgado*",resistencia_mod);
				AplicaEfeito(who,"bard_tocando",sound_duration,2,range,alvos.Debuffs,"*dançando intensamente*",resistencia_mod);
			else
				AplicaEfeito(who,"bard_contagiante",duration,CInt(DebuffModifier),range,alvos.Debuffs,"*dança empolgado*",resistencia_mod);
			endif
			break;
		BARD_BLADE_DANCE:
			var sword, mobile, laminaAnimada;
			var ok,x,y,z;

			//Verifica se foi escolhida uma arma valida
			SendSysMessage(who, "Selecione a espada que deseja encantar", BARD_FONT, BARD_FONT_COLOR_ITEM);
			ok := 0;
			do
				sword := target(who);
				if ((sword.objtype == 5049) || (sword.objtype == 3934) || (sword.objtype == 3936)) //Verifica se o item é uma arma valida
					ok := 1;
				elseif(!sword)
					return BARD_ENCERRA;
				else
					SendSysMessage(who, "Essa nao e uma espada valida. Escolha outra", BARD_FONT, BARD_FONT_COLOR_ITEM);
				endif
			dowhile (ok == 0);

			//Verifica se foi selecionado um alvo valido
			SendSysMessage(who, "Selecione um alvo para a espada", BARD_FONT, BARD_FONT_COLOR_TARGET);
			ok := 0;
			do
				mobile := target(who);
				if ((mobile.connected) && (mobile != who))
					ok := 1;
				elseif(!mobile)
					return BARD_ENCERRA;
				else
					SendSysMessage(who, "Alvo invalido. Escolha outro", BARD_FONT, BARD_FONT_COLOR_TARGET);
				endif
			dowhile (ok == 0);

			//Cria o NPC da espada conforme o nivel do bardo
//			if (Accessible(who, sword, range))

			x := sword.x;
			y := sword.y;
			z := sword.z;
			if (!x && !y && !z)
				x := sword.container.x;
				y := sword.container.y;
				z := sword.container.z;
			elseif (sword.container == who.backpack)
				x := who.x;
				y := who.y;
				z := who.z;
			endif

			if (modifier < 10) //terciario
				laminaAnimada := createnpcfromtemplate(":brainAI:espada_animada1", x, y, z, 0, who.realm);
			elseif (modifier < 20) //secundario
				laminaAnimada := createnpcfromtemplate(":brainAI:espada_animada2", x, y, z, 0, who.realm);
			elseif (modifier < 40) //primario
				laminaAnimada := createnpcfromtemplate(":brainAI:espada_animada3", x, y, z, 0, who.realm);
			else //lendario
				laminaAnimada := createnpcfromtemplate(":brainAI:espada_animada4", x, y, z, 0, who.realm);
				laminaAnimada.setmaster(who); //Coloca a espada sob controle do bardo TODO: Verificar se isso existe mesmo
			endif

			if (!x && !y && !z)
				return 0;
			else
				MoveObjectToLocation(sword, x, y, z+1, who.realm,MOVEOBJECT_FORCELOCATION);
				EquipItem(laminaAnimada, sword);
				laminaAnimada.color := sword.color; //A arma animada tem a mesma cor da arma base

				//Aciona o efeito da lamina animada
				TS_StartTimer(who, "bard_tocando", sound_duration, 4, who, 0);
				TS_StartTimer(laminaAnimada, "bard_bladedance", duration, sword, mobile, 0);
				return 1;
			endif
			break;
		BARD_HAMELIN:
			TS_StartTimer(who, "bard_tocando", sound_duration, 4, who, 0);
			TS_StartTimer(who, "bard_segurando_instrumento", sound_duration, 1, who, 0);
			TS_StartTimer(who, "bard_hamelin", duration, modifier, who, 0);
			break;
		BARD_SNAKE_CHARM: //mais dificil de resistir por ser focada em tipo de alvo especifico
			//Encontra mobiles no cubo de alcance
			var x1 := who.x - range;
			var x2 := who.x + range;
			var y1 := who.y - range;
			var y2 := who.y + range;
			var z1 := who.z - range*2;
			var z2 := who.z + range*2;
			var mobiles := ListMobilesInBox(x1, y1, z1, x2, y2, z2, who.realm);

			//execução do efeito
			TS_StartTimer(who, "bard_tocando", sound_duration, 2, who, 0);
			TS_StartTimer(who, "bard_segurando_instrumento", sound_duration, 1, who, 0);
			foreach mobile in mobiles
				if ((mobile.graphic == 52) || (mobile.graphic == 21)) //Graficos de cobras
					resistiu := CheckResist(mobile, VONTADE, 60, resistencia_mod); //Mais difícil de resistir pela especificidade do alvo
					if(!resistiu)
						if ((mobile.connected) && (mobile != who)) //Verificar se esta funcionando nos MOB
							TS_StartTimer(mobile, "bard_fascinar", duration, DebuffModifier, who, 0);
							PrintTextAbove(mobile, "*Fascinado*", BARD_FONT, BARD_FONT_COLOR_BAD);
						elseif (mobile.connected)
							PrintTextAbove(mobile, "*Tocando musica fascinadora de cobras*", BARD_FONT, BARD_FONT_COLOR); //Exibe sobre cabeca do bardo
						endif
					else
						PrintTextAbovePrivate(mobile, "resistiu", who, BARD_FONT, BARD_FONT_COLOR_NEUTRA); //avisa ao bardo que o alvo resistiu
					endif
				endif
			endforeach
			break;
		BARD_FIREDANCE:
			//Encontra mobiles no cubo de alcance
			var x1 := who.x - range;
			var x2 := who.x + range;
			var y1 := who.y - range;
			var y2 := who.y + range;
			var z1 := who.z - range*2;
			var z2 := who.z + range*2;
			var mobiles := ListMobilesInBox(x1, y1, z1, x2, y2, z2, who.realm);

			//execução do efeito
			var fogo;
			var total_alvos := 0;
			PrintTextAbove(who, "*Tocando musica dancante*", BARD_FONT, BARD_FONT_COLOR);
			foreach item in mobiles //verifica varias fontes de fogo
				if ((item.graphic > 1116 && item.graphic < 1167) ||
					(item.graphic > 2347 && item.graphic < 2413) ||
					(item.graphic > 2526 && item.graphic < 2560) ||
					(item.graphic > 2561 && item.graphic < 2565) ||
					(item.graphic > 2566 && item.graphic < 2570) ||
					(item.graphic > 2571 && item.graphic < 2581) ||
					(item.graphic > 3554 && item.graphic < 3561) ||
					(item.graphic > 4845 && item.graphic < 4942) ||
					(item.graphic > 6217 && item.graphic < 6221) ||
					(item.graphic > 6221 && item.graphic < 6225) ||
					(item.graphic > 6226 && item.graphic < 6230) ||
					(item.graphic > 6230 && item.graphic < 6234) ||
					(item.graphic > 6521 && item.graphic < 6583) ||
					(item.graphic > 6680 && item.graphic < 6783) ||
					(item.graphic > 10748 && item.graphic < 10756) ||
					(item.graphic > 10979 && item.graphic < 10988) ||
					(item.graphic > 11069 && item.graphic < 11110) ||
					(item.graphic > 12933 && item.graphic < 12956) ||
					(item.graphic > 13370 && item.graphic < 13421) ||
					(item.graphic > 13638 && item.graphic < 13645) ||
					(item.graphic > 14000 && item.graphic < 14120) ||
					(item.graphic > 15910 && item.graphic < 15930) ||
					(item.graphic > 28006 && item.graphic < 28034) ||
					(item.graphic > 28044 && item.graphic < 28077) ||
					(item.graphic > 28190 && item.graphic < 28206) ||
					(item.graphic > 39516 && item.graphic < 39530) ||
					(item.graphic > 40755 && item.graphic < 40761) ||
					(item.graphic > 40761 && item.graphic < 40767) ||
					(item.graphic > 41718 && item.graphic < 41725) ||
					(item.graphic > 42238 && item.graphic < 42245) ||
					(item.graphic > 42288 && item.graphic < 42292) ||
					(item.graphic > 42292 && item.graphic < 42296) ||
					(item.graphic > 45011 && item.graphic < 45022) ||
					(item.graphic > 45036 && item.graphic < 45042) ||
					(item.graphic > 45022 && item.graphic < 45025) ||
					(item.graphic > 45036 && item.graphic < 45042) ||
					(item.graphic > 45042 && item.graphic < 45045) ||
					(item.graphic > 45045 && item.graphic < 45050) ||
					(item.graphic > 45050 && item.graphic < 45053) ||
					(item.graphic > 45054 && item.graphic < 45057) ||
					(item.graphic > 45057 && item.graphic < 45062) ||
					(item.graphic > 45062 && item.graphic < 45068) ||
					(item.graphic > 46300 && item.graphic < 46304) ||
					(item.graphic > 46333 && item.graphic < 46341) ||
					(item.graphic > 46341 && item.graphic < 46344) ||
					(item.graphic > 46512 && item.graphic < 46516) ||
					(item.graphic > 46516 && item.graphic < 46520) ||
					(item.graphic > 47889 && item.graphic < 47902) ||
					(item.graphic > 48024 && item.graphic < 48028) ||
					(item.graphic > 48036 && item.graphic < 48040) ||
					(item.graphic > 48377 && item.graphic < 48382) ||
					(item.graphic > 49591 && item.graphic < 49602) ||
					(item.graphic > 52360 && item.graphic < 52377) ||
					(item.graphic > 52415 && item.graphic < 52437) ||
					(item.graphic > 52449 && item.graphic < 52479) ||
					(item.graphic > 52529 && item.graphic < 52556) ||
					item.graphic == 4012 || item.graphic == 6587 ||
					item.graphic == 8435 || item.graphic == 8698 ||
					item.graphic == 9689 || item.graphic == 10268 ||
					item.graphic == 28240 || item.graphic == 42004)

					total_alvos := total_alvos +1;

					if (modifier < 7)
						//dança menor do fogo: fogo pequeno
						fogo := createitematlocation( item.x , item.y , item.z , 14732 , 1 ,  item.realm );
						TS_StartTimer(fogo, "bard_firedance", duration);
						if(item.movable)
							destroyitem(item);
						endif
					elseif (modifier < 10)
						//dança media do fogo: labareda focada no objeto
						fogo := createitematlocation( item.x, item.y, item.z , 14742 , 1 , item.realm );
						TS_StartTimer(fogo, "bard_firedance", duration);
						if(item.movable)
							destroyitem(item);
						endif
					elseif (modifier < 30)
						//dança forte do fogo: labareda no objeto e 1 tile ao redor do objeto
						var i,j;
						for (i:= -1; i<=1; i:=i+1)
							for (j:= -1; j<=1; j:=j+1)
								fogo := createitematlocation( (item.x)+i , (item.y)+j , item.z , 14742 , 1 , item.realm );
								TS_StartTimer(fogo, "bard_firedance", duration);
							endfor
						endfor
					//Aqui o item não é mais destruido
					elseif (modifier < 90)
						//dança gigante do fogo: labareda no objeto e 2 tiles ao redor do ojeto
						var i,j;
						for (i:= -2; i<=2; i:=i+1)
							for (j:= -2; j<=2; j:=j+1)
								fogo := createitematlocation( (item.x)+i , (item.y)+j , item.z , 14742 , 1 , item.realm );
								TS_StartTimer(fogo, "bard_firedance", duration);
							endfor
						endfor
						//Aqui o item não é mais destruido
					else //TODO: Colocar fogo em três tile de raio
						var i,j;
						for (i:= -2; i<=2; i:=i+1)
							for (j:= -2; j<=2; j:=j+1)
								fogo := createitematlocation( (item.x)+i , (item.y)+j , item.z , 14742 , 1 , item.realm );
								TS_StartTimer(fogo, "bard_firedance", duration);
							endfor
						endfor
						//Aqui o item não é mais destruido
					endif
				endif
			endforeach
			TS_StartTimer(who, "bard_tocando", sound_duration, total_alvos, who, 0);
			TS_StartTimer(who, "bard_segurando_instrumento", sound_duration, 1, who, 0);
		 	break;
		BARD_EMUDECER:
			PrintTextAbove(who, "*Tocando musica desencorajadora*", BARD_FONT, BARD_FONT_COLOR_COMUM);
			TS_StartTimer(who, "bard_tocando", sound_duration, 2, who, 0);
			TS_StartTimer(who, "bard_segurando_instrumento", sound_duration, 1, who, 0);
			if(Vituperio)
				AplicaEfeito(who,"bard_emudecer",duration,CInt(DebuffModifier*0.5),range,alvos.Debuffs,"*Mudo(a)*",resistencia_mod);
				AplicaEfeito(who,"bard_int_down",duration,CInt(DebuffModifier),range,alvos.Debuffs,"*bem mais tapado*",resistencia_mod);
			else
				AplicaEfeito(who,"bard_emudecer",duration,CInt(DebuffModifier*0.5),range,alvos.Debuffs,"*Mudo(a)*",resistencia_mod);
			endif
			break;
/*		BARD_INCITAR:
			break;
		BARD_ACALMAR:
			break;
		BARD_COPIA:
			break;
		BARD_FANTASMAS:
			break;*/
		BARD_BLOOM:
			TS_StartTimer(who, "bard_tocando", sound_duration, 2, who, 0);
			TS_StartTimer(who, "bard_segurando_instrumento", sound_duration, 1, who, 0);
			PrintTextAbove(who, "*A musica faz o ambiente florescer a sua volta*",BARD_FONT,BARD_FONT_COLOR_LENDARIA);
			TS_StartTimer(who, "bard_florescer", duration, range, duration, 0);
			break;
		default:
			break;
	endcase

	SkillCheck(who, MAGICLORE, AP_GetSkill(who, MAGICLORE)); //Linha para upar Magic lore sempre que tocar

	return 0;
endfunction
