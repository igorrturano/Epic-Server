// ============================================================================
// racegump.inc
// Gump de seleção de raça e gênero para NPCs de facção.
// ============================================================================

use uo;
use os;
use cfgfile;

// ===== RACE BUTTON CONSTANTS =====
const FACCAO_BTN_HUMANO_M   := 1;
const FACCAO_BTN_HUMANO_F   := 2;
const FACCAO_BTN_ANAO_M     := 3;
const FACCAO_BTN_ANAO_F     := 4;
const FACCAO_BTN_ELFO_M     := 5;
const FACCAO_BTN_ELFO_F     := 6;
const FACCAO_BTN_DROW_M     := 7;
const FACCAO_BTN_DROW_F     := 8;
const FACCAO_BTN_ORC_M      := 9;
const FACCAO_BTN_ORC_F      := 10;
const FACCAO_BTN_PEQUILHO_M := 11;
const FACCAO_BTN_PEQUILHO_F := 12;

function FaccaoRaceGump(who)
    var gump := GFCreateGump();
    GFClosable(gump, 1);
    GFResizePic(gump, 0, 0, 9270, 270, 250);
    GFHTMLArea(gump, 10, 8, 250, 24, "<BASEFONT COLOR=#FFCC00><B>Escolha a Raca e o Genero</B>", 0, 0);
    
    var racas := array{"Humano", "Anao", "Elfo", "Drow", "Orc", "Pequilho"};
    var y := 40;
    var btn := 1;
    
    foreach raca in racas
        GFHTMLArea(gump, 12, y, 130, 22, "<BASEFONT COLOR=#FFFFFF>" + raca, 0, 0);
        GFAddButton(gump, 150, y + 2, 5605, 5605, GF_CLOSE_BTN, btn);
        GFHTMLArea(gump, 165, y, 35, 22, "<BASEFONT COLOR=#88CCFF><B>M</B>", 0, 0);
        GFAddButton(gump, 205, y + 2, 5605, 5605, GF_CLOSE_BTN, btn + 1);
        GFHTMLArea(gump, 220, y, 35, 22, "<BASEFONT COLOR=#FF88CC><B>F</B>", 0, 0);
        y += 30;
        btn += 2;
        sleepms(2);
    endforeach
    
    var input := GFSendGump(who, gump);
    if (!input || input[0] == 0)
        return 0;
    endif
    return input[0];
endfunction

function GetRaceNameList(btn_value)
    case (btn_value)
        FACCAO_BTN_HUMANO_M:   return "Human_Male";
        FACCAO_BTN_HUMANO_F:   return "Human_Female";
        FACCAO_BTN_ANAO_M:     return "Dwarf_Male";
        FACCAO_BTN_ANAO_F:     return "Dwarf_Female";
        FACCAO_BTN_ELFO_M:     return "Elf_Male";
        FACCAO_BTN_ELFO_F:     return "Elf_Female";
        FACCAO_BTN_DROW_M:     return "Drow_Male";
        FACCAO_BTN_DROW_F:     return "Drow_Female";
        FACCAO_BTN_ORC_M:      return "Orc_Male";
        FACCAO_BTN_ORC_F:      return "Orc_Female";
        FACCAO_BTN_PEQUILHO_M: return "Pequilho_Male";
        FACCAO_BTN_PEQUILHO_F: return "Pequilho_Female";
    endcase
    return 0;
endfunction

function GetRaceGraphic(btn_value)
    case (btn_value)
        FACCAO_BTN_HUMANO_M:   return 400;
        FACCAO_BTN_HUMANO_F:   return 401;
        FACCAO_BTN_ANAO_M:     return 1828;
        FACCAO_BTN_ANAO_F:     return 1829;
        FACCAO_BTN_ELFO_M:     return 1830;
        FACCAO_BTN_ELFO_F:     return 1831;
        FACCAO_BTN_DROW_M:     return 1830;
        FACCAO_BTN_DROW_F:     return 1831;
        FACCAO_BTN_ORC_M:      return 1832;
        FACCAO_BTN_ORC_F:      return 1833;
        FACCAO_BTN_PEQUILHO_M: return 1834;
        FACCAO_BTN_PEQUILHO_F: return 1835;
    endcase
    return 0;
endfunction

function GetRaceGender(btn_value)
    return (btn_value % 2 == 0) ? 1 : 0;
endfunction

function _PickRandom(arr)
    if (!arr || arr.size() == 0)
        return 0;
    endif
    return arr[RandomInt(arr.size()) + 1];
endfunction

function GetRaceSkinColor(btn_value)
    var config_group := "";
    
    // Map button to epic gump config group
    if (btn_value == FACCAO_BTN_HUMANO_M || btn_value == FACCAO_BTN_HUMANO_F)
        config_group := "list";  // Default human colors
    elseif (btn_value == FACCAO_BTN_ANAO_M || btn_value == FACCAO_BTN_ANAO_F)
        config_group := "negros"; // Dwarves use dark skin tones
    elseif (btn_value == FACCAO_BTN_ELFO_M || btn_value == FACCAO_BTN_ELFO_F)
        config_group := "elfo";   // Epic gump elf colors
    elseif (btn_value == FACCAO_BTN_DROW_M || btn_value == FACCAO_BTN_DROW_F)
        config_group := "drow";   // Epic gump drow colors
    elseif (btn_value == FACCAO_BTN_ORC_M || btn_value == FACCAO_BTN_ORC_F)
        config_group := "orc";    // Epic gump orc colors
    elseif (btn_value == FACCAO_BTN_PEQUILHO_M || btn_value == FACCAO_BTN_PEQUILHO_F)
        config_group := "brancos"; // Halflings use lighter skin
    endif
    
    // Read from Epic Gump config (DRY principle)
    var cfg := ReadConfigFile(":fls_core:config/skincolors");
    if (!cfg || !config_group)
        return CInt(_PickRandom(GetConfigStringArray(cfg["list"], "color")));
    endif
    
    var colors := GetConfigStringArray(cfg[config_group], "color");
    if (!colors || colors.size() == 0)
        colors := GetConfigStringArray(cfg["list"], "color"); // Fallback
    endif
    
    return CInt(_PickRandom(colors));
endfunction

function GetRaceHairColor(btn_value)
    if (btn_value == FACCAO_BTN_DROW_M || btn_value == FACCAO_BTN_DROW_F)
        return _PickRandom(array{1301, 1302, 1303, 1310, 1311});
    elseif (btn_value == FACCAO_BTN_ANAO_M || btn_value == FACCAO_BTN_ANAO_F)
        return _PickRandom(array{1133, 1147, 1148, 1149, 1890});
    elseif (btn_value == FACCAO_BTN_ORC_M || btn_value == FACCAO_BTN_ORC_F)
        return _PickRandom(array{1145, 1146, 1147});
    else
        return _PickRandom(array{1103, 1104, 1105, 1106, 1107, 1108, 1109});
    endif
endfunction

function _IsMaleBtn(btn_value)
    return (btn_value % 2 == 1) ? 1 : 0;
endfunction

function _CreateAndEquip(npc, graphic, color)
    var item := CreateItemAtLocation(npc.x, npc.y, npc.z, graphic, 1);
    if (item)
        item.color := color;
        if (!EquipItem(npc, item))
            DestroyItem(item);
        endif
    endif
endfunction

function _RemoveHairBeard(npc)
    var hair := GetEquipmentByLayer(npc, 0x0b);
    if (hair) DestroyItem(hair); endif
    var beard := GetEquipmentByLayer(npc, 0x10);
    if (beard) DestroyItem(beard); endif
endfunction

function _GetHairStylesByRace(btn_value)
    // Lê todos os estilos de cabelo do Epic Gump
    // Todos os estilos estão disponíveis para todas as raças em [todos]
    var cfg := ReadConfigFile(":fls_core:config/hairtypes");
    if (!cfg)
        return array{}; // Fallback se config não existir
    endif
    
    var hair_entries := GetConfigStringArray(cfg["todos"], "Style");
    var hair_graphics := array{};
    
    // Parse: Style Name Graphic GumpGraphic
    // Extrai o segundo campo (Graphic ID)
    foreach entry in hair_entries
        var parts := SplitWords(entry);
        if (parts.size() >= 2)
            var graphic := CInt(parts[2]);
            hair_graphics.append(graphic);
        endif
        sleepms(1);
    endforeach
    
    return hair_graphics.size() > 0 ? hair_graphics : array{};
endfunction

function _GetBeardStylesByRace(btn_value)
    // Lê estilos de barba do Epic Gump
    // Anões usam dwarwen_beard, outros usam beard padrão
    var cfg := ReadConfigFile(":fls_core:config/hairtypes");
    if (!cfg)
        return array{}; // Fallback
    endif
    
    var beard_type := (btn_value == FACCAO_BTN_ANAO_M) ? "dwarwen_beard" : "beard";
    var beard_entries := GetConfigStringArray(cfg[beard_type], "Style");
    var beard_graphics := array{};
    
    // Parse: Style Name Graphic GumpGraphic
    foreach entry in beard_entries
        var parts := SplitWords(entry);
        if (parts.size() >= 2)
            var graphic := CInt(parts[2]);
            beard_graphics.append(graphic);
        endif
        sleepms(1);
    endforeach
    
    return beard_graphics.size() > 0 ? beard_graphics : array{};
endfunction


function _ApplyRaceAppearance(npc, btn_value)
    var skin_color := GetRaceSkinColor(btn_value);
    var hair_color := GetRaceHairColor(btn_value);
    
    npc.color := skin_color;
    npc.truecolor := skin_color;
    
    _RemoveHairBeard(npc);
    
    var hair_styles := _GetHairStylesByRace(btn_value);
    if (hair_styles.size() > 0)
        _CreateAndEquip(npc, _PickRandom(hair_styles), hair_color);
    endif

    var can_have_beard := _IsMaleBtn(btn_value) &&
                          btn_value != FACCAO_BTN_ELFO_M &&
                          btn_value != FACCAO_BTN_PEQUILHO_M;
    if (can_have_beard && (btn_value == FACCAO_BTN_ANAO_M || RandomInt(2) == 1))
        var beard_styles := _GetBeardStylesByRace(btn_value);
        if (beard_styles.size() > 0)
            _CreateAndEquip(npc, _PickRandom(beard_styles), hair_color);
        endif
    endif
    return 1;
endfunction

function _SaveEquipment(npc)
    var saved := array{};
    var layer := 1;
    while (layer <= 25)
        if (layer != 0x0b && layer != 0x10) // Ignora Hair e Beard
            var item := GetEquipmentByLayer(npc, layer);
            if (item)
                saved.append(item);
            endif
        endif
        layer += 1;
    endwhile
    return saved;
endfunction

function ApplyRaceToNPC(npc, btn_value)
    // Muda corpo e cores
    npc.gender := GetRaceGender(btn_value);
    npc.graphic := GetRaceGraphic(btn_value);
    _ApplyRaceAppearance(npc, btn_value);
    
    // Veste de volta as roupas do grupo (Rogue, Rogue2, etc)
    ReequipNPC(npc);
    
    // Nome racial
    var namelist := GetRaceNameList(btn_value);
    var cfg := ReadConfigFile(":brainAI:config/names");
    if (cfg)
        var name_list := GetConfigStringArray(cfg[namelist], "Name");
        if (name_list.size() > 0)
            SetName(npc, name_list[RandomInt(name_list.size()) + 1]);
        endif
    endif
    return 1;
endfunction

function ReequipNPC(npc)
    var backpack := npc.backpack;
    if (!backpack) return 0; endif
    

    foreach item in (EnumerateItemsInContainer(backpack))
        // Só tenta vestir o que estiver solto na mochila (raiz)
        if (item.container == backpack)
            // Pula Cabelo/Barba
            if (item.layer == 0x0b || item.layer == 0x10)
                continue; 
            endif

            var res := EquipItem(npc, item);
            if (res)
                // Item foi vestido com sucesso
            else
                // Se o item for proibido para a raca, ele fica na mochila.
            endif
        endif
        sleepms(2);
    endforeach
    return 1;
endfunction