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
        0xEEC4: return GetMarcaGadoDuwInfo();
        // Adicione mais casos para outros itens aqui
        default: return 0;
    endcase
endfunction

function GetMarcaGadoDuwInfo()
    var info := struct;
    info.+name := "Marca de Gado de Duw";
    info.+image := 0xF8E4;  // Gráfico do item
    info.+flavor_image := 18004;  // ID da imagem de flavor
    info.+culture_name := "Björske";  // Nome da cultura
    info.+main_text := "Durante um rigoroso inverno no norte, a tribo Björske lutava para manter seu rebanho de gado. Bjorn, o líder da aldeia, pegou o ferro de marcar recém-forjado e, sob a lua cheia, marcou o touro mais forte com o símbolo sagrado de Duw, seu deus do sol. A cerimônia de marcação garantiu que o touro lideraria o rebanho com segurança pelas traiçoeiras florestas cobertas de neve. Semanas depois, durante uma tempestade, o touro, agora infundido com proteção divina, guiou os aldeões e seu rebanho por um desfiladeiro perigoso, salvando-os de uma matilha de lobos famintos.";
    info.+skill_text := "O ferro sagrado, gravado com o símbolo de Duw, acreditava-se conceder orientação divina ao gado, protegendo a tribo durante as migrações. Tornou-se uma ferramenta reverenciada entre os Björske";
    info.+archaeology_text := "O poder desse ferro ligava a sobrevivência do gado Björske à proteção de seus deuses.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 80.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

// Adicione funções semelhantes para os outros itens de lore