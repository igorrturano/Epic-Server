//2
include ":gumps:gumps";
include ":attributes:attributes";
include ":gumps:gumps_ex";
include ":gumps:requestGump";
include "include/say";
include ":tn:tngumps";
include "include/client";
include ":charactercreation:habilidades";
include ":timedscripts:timedScripts";

// lore_items.inc

const LORE_ITEMS_COUNT := 100;

function GetLoreItemInfo(item_id)
    case (item_id)
        0xEEC4: return GetRingOfDrusiliaInfo();
        // Adicione mais casos para outros itens aqui
        default: return 0;
    endcase
endfunction

function GetRingOfDrusiliaInfo()
    var info := struct;
    info.+name := "Anel de Casamento de Drusilia";
    info.+image := 0x108A;  // Gráfico do anel
    info.+flavor_image := 9260;  // ID da imagem de flavor
    info.+main_text := "Na imponente capital imperial de Aigla, o casamento de Drusilia foi um evento memorável...";
    info.+profession_text := "Um joalheiro notaria os detalhes intrincados da gravação no anel...";
    info.+archaeology_text := "Um arqueólogo perceberia que o estilo do anel remonta à Era de Ouro de Aigla...";
    info.+required_profession := "Joalheiro";
    info.+required_skill := "Arqueologia";
    info.+required_skill_value := 80.0;
    return info;
endfunction

// Adicione funções semelhantes para os outros 99 itens