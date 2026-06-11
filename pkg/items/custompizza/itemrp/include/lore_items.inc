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

const LORE_ITEMS_COUNT := 150;

function GetLoreItemInfo(item_id)

    var loreItemsConfig := ReadConfigFile(":custompizza:itemrp/lore_items");
    var elem := FindConfigElem(loreItemsConfig, item_id);

    var info := struct;
    if(elem != error)
        info.+name := elem.name;
        info.+image := elem.image;
        info.+flavor_image := elem.flavor_image;
        info.+flavor_url := "";
        info.+culture_name := elem.culture_name;
        info.+main_text := elem.main_text;
        info.+skill_text := elem.skill_text;
        info.+archaeology_text := elem.archaeology_text;
        info.+required_skill := elem.required_skill;
        info.+required_skill_value := elem.required_skill_value;
        info.+required_habilidade := elem.required_habilidade;
    else
        return error{"errortext":="item de lore não encontrado"};
    endif

    return info;
endfunction

// Prefere lore_data persistido no item (criado via .criaritemlore) antes de
// cair no catálogo curado em lore_items.cfg. Permite que itens individuais
// tenham lore customizado sem desviar outros itens do mesmo objtype.
function GetLoreItemInfoForItem(item)
    var custom := GetObjProperty(item, "lore_data");
    if (custom && custom != error)
        var info := struct;
        info.+name := custom.name;
        info.+image := custom.image;
        info.+flavor_image := 0;
        info.+flavor_url := custom.flavor_url;
        info.+culture_name := custom.culture_name;
        info.+main_text := custom.main_text;
        info.+skill_text := custom.skill_text;
        info.+archaeology_text := custom.archaeology_text;
        info.+required_skill := custom.required_skill;
        info.+required_skill_value := custom.required_skill_value;
        info.+required_habilidade := "Arqueologo";
        return info;
    endif
    return GetLoreItemInfo(item.objtype);
endfunction