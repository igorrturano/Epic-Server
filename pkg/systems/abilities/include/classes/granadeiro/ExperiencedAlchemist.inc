use uo;
use os;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":alchemy:alchemy";
include ":gumps:include/epicGumps";

/**
 * Experienced Alchemist (Alquimista Experiente) - ABILITY_ID::ENGENHARIA_DE_GATILHOS (2500)
 *
 * Active (OnUse):
 *   - Player: opens explosive configuration gump (menuExplosivos2).
 *   - NPC:    throws one explosive potion from backpack.
 *
 * Passive (no hook, checked inline via HasAbility):
 * - Reduces reagent costs by 50% [alchemybook.src, dyebook.src]
 */
class ExperiencedAlchemist(CombatAbility)

	function ExperiencedAlchemist(this)
		Super(ABILITY_ID::ENGENHARIA_DE_GATILHOS);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();

		if (mobile.IsNPC())
			return _NpcThrowExplosive(who);
		endif

		menuExplosivos2(who);
		DebugMsg(
			who,
			DEBUG_HAB,
			Message("[ENGENHARIA_DE_GATILHOS] modificacoes ativas", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

endclass

function _NpcThrowExplosive(who)
	var explosive_potions := array{0xbf13, 0xbf14, 0xbf15, 0xef60, 0xef61, 0xef62};
	foreach item in EnumerateItemsInContainer(who.backpack)
		if (item.objtype in explosive_potions)
			UseItem(item, who);
			return true;
		endif
	endforeach
	return false;
endfunction

// Constantes de navegação
const BOTAO_GATILHO        := 1;
const BOTAO_ELEMENTO       := 2;
const BOTAO_INVOLUCRO    := 3;
const BOTAO_CARGA   := 4;
const BOTAO_REFORCO := 5;

const MENU_GATILHO         := 1;
const MENU_ELEMENTO        := 2;
const MENU_INVOLUCRO     := 3;
const MENU_CARGA    := 4;
const MENU_REFORCO := 5;

var last_clicked_button := BOTAO_GATILHO;
var button_states       := array{ 1, 0, 0, 0, 0 };
var config_data         := struct{};

function menuExplosivos2(who)
    var saved := GetObjProperty(who, "ExplosivoConfig");
    if (saved)
        config_data := saved;
    else
        InitConfigData();
    endif

    var gump_width  := 500;
    var gump_height := 300;
    var title       := "Configuracao de Explosivo";

    var input := ShowPageGatilho(who, title, gump_width, gump_height);

    while (input[0] != 0)

        ExtractPageData(input, last_clicked_button);

        // Botão Confirmar
        if (input[0] == 100)
            SetObjProperty(who, "ExplosivoConfig", config_data);
            SendSysMessage(who, "Configuracao salva!");
            break;
        endif

        // Botão Cancelar
        if (input[0] == 101)
            SendSysMessage(who, "Cancelado.");
            break;
        endif

        var new_menu := UpdateMenuState(input[0], who);

        case (new_menu)
            MENU_GATILHO:   	input := ShowPageGatilho(who, title, gump_width, gump_height);
            MENU_ELEMENTO:  	input := ShowPageElemento(who, title, gump_width, gump_height);
            MENU_INVOLUCRO:		input := ShowPageInvolucro(who, title, gump_width, gump_height); 
            MENU_CARGA:   		input := ShowPageCarga(who, title, gump_width, gump_height);
            MENU_REFORCO: 		input := ShowPageReforco(who, title, gump_width, gump_height);
        endcase
    endwhile

    return 1;
endfunction

// ============================================================
// INICIALIZAÇÃO
// ============================================================
function InitConfigData()
    config_data.gatilho         := GATILHO_PAVIO;
    config_data.elemento        := ELEMENTO_FOGO;
    config_data.involucro 	    := INVOLUCRO_BASE;
    config_data.carga    		:= CARGA_BALANCEADA;
    config_data.reforco 		:= REFORCO_COMUM;
endfunction

// ============================================================
// EXTRAÇÃO DE DADOS POR PÁGINA
// ============================================================
function ExtractPageData(input, page)
    case (page)
        BOTAO_GATILHO:
            if (input[GATILHO_PAVIO])    config_data.gatilho := GATILHO_PAVIO;    endif
            if (input[GATILHO_IMPACTO])  config_data.gatilho := GATILHO_IMPACTO;  endif
            if (input[GATILHO_MINA])     config_data.gatilho := GATILHO_MINA;     endif
            break;

        BOTAO_ELEMENTO:
            if (input[ELEMENTO_FOGO])    config_data.elemento := ELEMENTO_FOGO;    endif
            if (input[ELEMENTO_ENERGIA]) config_data.elemento := ELEMENTO_ENERGIA; endif
            if (input[ELEMENTO_GELO])    config_data.elemento := ELEMENTO_GELO;    endif
            break;

        BOTAO_INVOLUCRO:
            if (input[INVOLUCRO_BASE])          config_data.involucro := INVOLUCRO_BASE;         endif
            if (input[INVOLUCRO_FRAGMENTACAO])  config_data.involucro := INVOLUCRO_FRAGMENTACAO; endif
            if (input[INVOLUCRO_CONCUSSAO])      config_data.involucro := INVOLUCRO_CONCUSSAO;     endif
            break;

        BOTAO_CARGA:
            if (input[CARGA_BALANCEADA])  config_data.carga := CARGA_BALANCEADA;  endif
            if (input[CARGA_CONCENTRADA]) config_data.carga := CARGA_CONCENTRADA; endif
            if (input[CARGA_DISPERSA])    config_data.carga := CARGA_DISPERSA;    endif
            break;

        BOTAO_REFORCO:
            if (input[REFORCO_COMUM])          config_data.reforco := REFORCO_COMUM;          endif
            if (input[REFORCO_CARGA_DUPLA])    config_data.reforco := REFORCO_CARGA_DUPLA;    endif
            if (input[REFORCO_POTENCIA_ELEM])  config_data.reforco := REFORCO_POTENCIA_ELEM;  endif
            break;
    endcase
endfunction

// ============================================================
// ATUALIZAÇÃO DE ESTADO
// ============================================================ else SendSysMessage(who, "Voce ainda nao aprendeu este tipo de modificacao"); 
function UpdateMenuState(input, who)
    last_clicked_button := input;
    case (input)
        BOTAO_GATILHO:         button_states := array{1,0,0,0,0}; return MENU_GATILHO;
        BOTAO_ELEMENTO:        button_states := array{0,1,0,0,0}; if(MobileWrapper(who).HasAbility(ABILITY_ID::ZONA_DE_PRESSAO))return MENU_ELEMENTO; else SendSysMessage(who, "Voce ainda nao aprendeu este tipo de modificacao"); endif
        BOTAO_INVOLUCRO:     button_states := array{0,0,1,0,0}; if(MobileWrapper(who).HasAbility(ABILITY_ID::CAMPO_MINADO)) return MENU_INVOLUCRO; else SendSysMessage(who, "Voce ainda nao aprendeu este tipo de modificacao"); endif
        BOTAO_CARGA:    button_states := array{0,0,0,1,0}; if(MobileWrapper(who).HasAbility(ABILITY_ID::TORRETA_GRANADEIRA)) return MENU_CARGA; else SendSysMessage(who, "Voce ainda nao aprendeu este tipo de modificacao"); endif
        BOTAO_REFORCO: button_states := array{0,0,0,0,1}; if(MobileWrapper(who).HasAbility(ABILITY_ID::PROTOCOLO_MESTRE)) return MENU_REFORCO; else SendSysMessage(who, "Voce ainda nao aprendeu este tipo de modificacao"); endif
    endcase
    button_states := array{1,0,0,0,0};
    last_clicked_button := BOTAO_GATILHO;
    return MENU_GATILHO;
endfunction

// ============================================================
// BARRA DE NAVEGAÇÃO (igual ao epicBOSS)
// ============================================================
function AddNavBar(byref gump, gump_width)
    var titles    := array{ " Gatilho", " Elemento", " Involucro", " Carga", " Reforco" };
    var x_pos     := array{ 30, 120, 210, 310, 400 };
    var btn_ids   := array{ BOTAO_GATILHO, BOTAO_ELEMENTO, BOTAO_INVOLUCRO,
                            BOTAO_CARGA, BOTAO_REFORCO };

    GFResizePic(gump, 10, 35, 9350, gump_width - 20, 30);

    for i := 1 to 5
        var btn_gump  := 2151;
        var txt_color := 53;
        if (btn_ids[i] == last_clicked_button)
            btn_gump  := 2153;
            txt_color := 230;
        endif
        GFTextLine(gump, x_pos[i] + 20, 42, txt_color, titles[i]);
        GFAddButton(gump, x_pos[i], 40, btn_gump, btn_gump, 1, btn_ids[i]);
    endfor
endfunction

// ============================================================
// RODAPÉ COM BOTÕES CONFIRMAR / CANCELAR
// ============================================================
function AddFooter(byref gump, gump_width, gump_height)
    GFResizePic(gump, 10, gump_height - 45, 3000, gump_width - 20, 2);
    GFResizePic(gump, gump_width / 2 - 100, gump_height - 38, 9350, 200, 30);
    GFAddButton(gump, gump_width / 2 - 90, gump_height - 30, 2128, 2129, 1, 100);
    GFAddButton(gump, gump_width / 2 + 20,  gump_height - 30, 2119, 2120, 1, 101);
endfunction

// ============================================================
// HELPER: bloco de 3 radio buttons numa página
// ============================================================
function AddRadioBlock(byref gump, label1, label2, label3, val1, val2, val3, current)
    var y_base := 90;
    var x_btn  := 40;
    var x_txt  := 70;

    GFSetRadioGroup(gump, 1);

    GFRadioButton(gump, x_btn, y_base,      210, 211, (current == val1), val1);
    GFTextLine(gump,    x_txt, y_base,      1153, label1);

    GFRadioButton(gump, x_btn, y_base + 40, 210, 211, (current == val2), val2);
    GFTextLine(gump,    x_txt, y_base + 40, 1153, label2);

    GFRadioButton(gump, x_btn, y_base + 80, 210, 211, (current == val3), val3);
    GFTextLine(gump,    x_txt, y_base + 80, 1153, label3);
endfunction

// ============================================================
// PÁGINAS
// ============================================================
function ShowPageGatilho(who, title, gump_width, gump_height)
    var gump := GFECreateGump(title, gump_width, gump_height);
    AddNavBar(gump, gump_width);
    GFTextMid(gump, 0, 72, gump_width, 1153, "Gatilho");
    AddRadioBlock(gump, "Pavio: Detona apos 3 segundos de ativacao.", "Impacto: Detona no impacto com o alvo.", "Mina: Oculta o explosivo, funcionando como uma armadilha.",
                  GATILHO_PAVIO, GATILHO_IMPACTO, GATILHO_MINA,
                  config_data.gatilho);
    AddFooter(gump, gump_width, gump_height);
    return GFSendGump(who, gump);
endfunction

function ShowPageElemento(who, title, gump_width, gump_height)
    var gump := GFECreateGump(title, gump_width, gump_height);
    AddNavBar(gump, gump_width);
    GFTextMid(gump, 0, 72, gump_width, 1153, "Elemento");
    AddRadioBlock(gump, "Fogo: o explosivo causa dano de fogo.", "Raio: o explosivo causa dano de energia.", "Gelo: o explosivo causa dano de gelo.",
                  ELEMENTO_FOGO, ELEMENTO_ENERGIA, ELEMENTO_GELO,
                  config_data.elemento);
    AddFooter(gump, gump_width, gump_height);
    return GFSendGump(who, gump);
endfunction

function ShowPageInvolucro(who, title, gump_width, gump_height)
    var gump := GFECreateGump(title, gump_width, gump_height);
    AddNavBar(gump, gump_width);
    GFTextMid(gump, 0, 72, gump_width, 1153, "Involucro");
    AddRadioBlock(gump, "Base: o explosivo causa apenas o dano base.", "Fragmentacao: Lanca fragmentos cortantes que causam sangramento.", "Concussão: Causa uma onda de choque que empurra os alvos.",
                  INVOLUCRO_BASE, INVOLUCRO_FRAGMENTACAO, INVOLUCRO_CONCUSSAO,
                  config_data.involucro);
    AddFooter(gump, gump_width, gump_height);
    return GFSendGump(who, gump);
endfunction

function ShowPageCarga(who, title, gump_width, gump_height)
    var gump := GFECreateGump(title, gump_width, gump_height);
    AddNavBar(gump, gump_width);
    GFTextMid(gump, 0, 72, gump_width, 1153, "Carga");
    AddRadioBlock(gump, "Balanceada: Equilibrio entre alcance e efetividade.", "Concentrada: Alcance reduzido, porém mais efetivo.", "Dispersa: Alcance aumentado, porém menos efetivo.",
                  CARGA_BALANCEADA, CARGA_CONCENTRADA, CARGA_DISPERSA,
                  config_data.carga);
    AddFooter(gump, gump_width, gump_height);
    return GFSendGump(who, gump);
endfunction

function ShowPageReforco(who, title, gump_width, gump_height)
    var gump := GFECreateGump(title, gump_width, gump_height);
    AddNavBar(gump, gump_width);
    GFTextMid(gump, 0, 72, gump_width, 1153, "Reforco");
    AddRadioBlock(gump, "Comum: Sem reforco, efeito base.", "Carga Dupla: Causa uma segunda explosao menor apos a detonacao.", "Potencia Elemental: Causa um efeito baseado no elemento ativo.",
                  REFORCO_COMUM, REFORCO_CARGA_DUPLA, REFORCO_POTENCIA_ELEM,
                  config_data.reforco);
    AddFooter(gump, gump_width, gump_height);
    return GFSendGump(who, gump);
endfunction