// ============================================================================
// Hotbar Execution Helpers - Lightweight data helpers for Execute* functions
// ============================================================================
// Extracted from hotbar_data.inc to keep the main handler's data segment small.
// Contains ONLY the functions needed to resolve action IDs during execution.
// ============================================================================

use uo;
use os;
use cfgfile;

include ":hotbar:include/icon_manager";
include ":timedScripts:timedScripts";

// ============================================================================
// GetCombatHotkeys - Get player's combat hotkeys (ability + commands)
// ============================================================================
function GetCombatHotkeys(who)
    var chardata := GetObjProperty(who, "chardata");
    if (!chardata)
        return array{};
    endif

    var combat_hotkeys := array{};
    var special_name := "";
    var special_class := "";
    var special_hue := 0;
    var special_index := 0;

    case (chardata.classe)
        "Guerreiro":
            special_name := "Ataque Extra";
            special_class := "Guerreiro";
            special_hue := 33;
            special_index := 1;
        "Ladino":
            special_name := "Evasao";
            special_class := "Ladino";
            special_hue := 63;
            special_index := 2;
        "Sabio":
            if (chardata.wisdom_path == "Engenheiro")
                special_name := "Recarga Rapida";
                special_class := "Engenheiro";
                special_hue := 2101;
                special_index := 3;
            elseif (chardata.magia == "arcane_archer")
                special_name := "Velocidade Arcana";
                special_class := "arcane_archer";
                special_hue := 16;
                special_index := 5;
            else
                special_name := "Conjuracao Rapida";
                special_class := "Conjurador";
                special_hue := 88;
                special_index := 4;
            endif
    endcase

    var level := CInt(GetObjProperty(who, "Level"));
    if (!level) level := 1; endif
    var final_cooldown := 180 - (Floor(level / 4) * 30);
    if (final_cooldown < 30) final_cooldown := 30; endif

    var remaining_cooldown := CInt(TS_GetCooldown(who, "special_ability"));
    if (!remaining_cooldown || remaining_cooldown < 0) remaining_cooldown := 0; endif

    if (special_name != "")
        var special_desc := "";
        case (special_index)
            1: special_desc := "Executa um ataque instantâneo adicional";
            2: special_desc := "Fica invisível por 0.5s e avanca 4 tiles";
            3: special_desc := "Adiciona 1 bala instantaneamente na arma de fogo";
            4: special_desc := "Permite conjurar a próxima magia instantaneamente";
            5: special_desc := "Aumenta a velocidade de movimento por 4 segundos";
            default: special_desc := "Habilidade especial de " + special_class;
        endcase

        var icon_data := GetSpecialIcon(special_name);

        combat_hotkeys.append(struct{
            "name" := special_name,
            "classe" := special_class,
            "desc" := special_desc,
            "iconid" := icon_data.iconid,
            "hue" := icon_data.hue,
            "index" := special_index,
            "delay" := final_cooldown,
            "remaining" := remaining_cooldown,
            "command_type" := 0,
            "script_path" := ":specialAbilities:specialAbilities"
        });
    endif

    var golpe_icon := GetCommandIcon("golpe");
    if (!golpe_icon.errortext)
        combat_hotkeys.append(struct{
            "name" := ".golpe",
            "classe" := special_class,
            "desc" := "Golpe especial de arma",
            "iconid" := golpe_icon.iconid,
            "hue" := golpe_icon.hue,
            "index" := 10,
            "delay" := 0,
            "remaining" := 0,
            "command_type" := 1,
            "script_path" := ":equipsys:commands/player/golpe"
        });
    endif

    var combate_icon := GetCommandIcon("combate");
    if (!combate_icon.errortext)
        combat_hotkeys.append(struct{
            "name" := ".combate",
            "classe" := special_class,
            "desc" := "Mudar postura de combate",
            "iconid" := combate_icon.iconid,
            "hue" := combate_icon.hue,
            "index" := 11,
            "delay" := 0,
            "remaining" := 0,
            "command_type" := 1,
            "script_path" := ":charactercreation:commands/player/combate"
        });
    endif

    return combat_hotkeys;
endfunction

// ============================================================================
// GetCombatHotkeyByIndex - Get specific combat hotkey by index
// ============================================================================
function GetCombatHotkeyByIndex(who, actionId)
    var hotkeys := GetCombatHotkeys(who);

    foreach hotkey in (hotkeys)
        if (hotkey.index == actionId)
            return hotkey;
        endif
    endforeach

    return error{"errortext" := "Combat hotkey not found"};
endfunction
