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
        info.+culture_name := elem.culture_name;
        info.+main_text := elem.main_text;
        info.+skill_text := elem.skill_text;
        info.+archaeology_text := elem.archaeology_text;
        info.+required_skill := elem.required_skill;
        info.+required_skill_value := elem.required_skill_value;
        info.+required_habilidade := elem.required_habilidade;
        Print(item_id + " - " + elem + " - " + info);
    else
        return error{"errortext":="item de lore não encontrado"};
    endif
     
    return info;
endfunction