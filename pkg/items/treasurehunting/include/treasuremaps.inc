// File: :treasurehunting:include/treasuremaps.inc

use uo;
use os;
use cfgfile;
include "include/client";
include ":coords:coords";
include ":attributes:attributes";
include ":charactercreation:habilidades";

/* CONSTANTS */
enum MAP_LEVELS 
    MAP_LEVEL_1 := 1,
    MAP_LEVEL_2 := 2,
    MAP_LEVEL_3 := 3,
    MAP_LEVEL_4 := 4,
    MAP_LEVEL_5 := 5
endenum

/* Core Map Interface Functions */

/*
 * IsTreasureMap(item)
 * Verifica se um item é um mapa do tesouro válido
 */
function IsTreasureMap(item)
    if (!item)
        return 0;
    endif
    
    return (GetObjProperty(item, "chestx") != error);
endfunction


/*
 * GetMapLevel(map)
 * Retorna o nível do mapa
 */
function GetMapLevel(map)
    if (!map)
        return 0;
    endif
    
    return CInt(GetObjProperty(map, "level"));
endfunction

/*
 * GetMapCoordinates(map)
 * Retorna as coordenadas do tesouro
 */
function GetMapCoordinates(map)
    if (!map)
        return struct{"x" := 0, "y" := 0, "z" := 0};
    endif
    
    return struct{
        "x" := CInt(GetObjProperty(map, "chestx")),
        "y" := CInt(GetObjProperty(map, "chesty")),
        "z" := CInt(GetObjProperty(map, "chestz"))
    };
endfunction

/*
 * CanDecodeMap(who, map)
 * Verifica se o personagem pode decodificar o mapa
 */
function CanDecodeMap(who, map)
    if (!who || !map)
        return 0;
    endif
    
    // Verifica habilidade de Desbravador Nato
    return TemHabilidade(who, "Desbravador Nato");
endfunction

/*
 * CreateNewTreasureMap(who, level)
 * Cria um novo mapa do tesouro
 */
function CreateNewTreasureMap(who, level := 1)
    if (!who)
        return 0;
    endif
    
    var map := CreateItemInBackpack(who, GetObjTypeByName("map_nonedit"), 1);
    if (!map)
        return 0;
    endif
    
    // Define propriedades básicas
    SetObjProperty(map, "level", level);
    SetObjProperty(map, "decoder", who.serial);
    SetObjProperty(map, "CreatedOn", ReadGameClock());
    SetObjProperty(map, "CreatedBy", who.serial);
    
    // Define localização do tesouro
    if (SetTreasureLocation(map))
        var coords := GetMapCoordinates(map);
        var sextant_coords := ToSextant(coords.x, coords.y);
        SetName(map, "Mapa do Tesouro Nível " + level + " [" + sextant_coords + "]");
        return map;
    else
        DestroyItem(map);
        return 0;
    endif
endfunction

/*
 * SetTreasureLocation(map)
 * Define a localização do tesouro no mapa
 */
function SetTreasureLocation(map)
    if (!map)
        return 0;
    endif
    
    var cfg := ReadConfigFile("treasure");
    if (!cfg)
        return 0;
    endif
    
    var num_locations := GetConfigMaxIntKey(cfg);
    var chosen_loc := (RandomInt(num_locations) + 1);
    var location := FindConfigElem(cfg, chosen_loc);
    
    if (!location)
        return 0;
    endif
    
    SetObjProperty(map, "chestx", GetConfigInt(location, "X"));
    SetObjProperty(map, "chesty", GetConfigInt(location, "Y"));
    SetObjProperty(map, "chestz", GetConfigInt(location, "Z"));
    
    return 1;
endfunction

/*
 * MarkMapComplete(map, who)
 * Marca um mapa como completado
 */
function MarkMapComplete(map, who)
    if (!map || !who)
        return 0;
    endif
    
    SetObjProperty(map, "Completed", ReadGameClock());
    SetObjProperty(map, "CompletedBy", who.serial);
    SetObjProperty(map, "CompletedByName", who.name);
    SetObjProperty(map, "CompletedOn", ReadGameClock());
    
    // Mantém a compatibilidade com o sistema antigo
    SetObjProperty(map, "Solved", 1);
    
    var map_level := GetObjProperty(map, "level");
    var coords := GetMapCoordinates(map);
    var sextant_coords := ToSextant(coords.x, coords.y);
    
    SetName(map, "Mapa do Tesouro Nível " + map_level + " [" + sextant_coords + "] - Desvendado por " + who.name);
    
    return 1;
endfunction

/*
 * ValidateMapDistance(who, targ, map)
 * Verifica se o alvo está próximo o suficiente do tesouro
 */
function ValidateMapDistance(who, targ, map)
    if (!who || !targ || !map)
        return 0;
    endif
    
    var coords := GetMapCoordinates(map);
    var map_distance := CoordinateDistance(targ.x, targ.y, coords.x, coords.y);
    
    
    const MAX_DISTANCE := 20;
    const DIG_RANGE := 3;
    
    if (map_distance > MAX_DISTANCE)
        SendSysMessage(who, "Você está muito longe do local do tesouro.", SSM_FAIL);
        return 0;
    elseif (map_distance > DIG_RANGE)
        SendSysMessage(who, "Você está próximo, mas não exatamente no local correto.", SSM_FAIL);
        return 0;
    endif
    
    return 1;
endfunction


function IsMapSolved(map)
    if (!map)
        return 0;
    endif
    
    return (GetObjProperty(map, "Completed") != error || GetObjProperty(map, "Solved") != error);
endfunction

function GetMapCompletionInfo(map)
    var info := struct{
        "completed" := 0,
        "completed_on" := 0,
        "completed_by" := "Ninguém",
        "completed_by_serial" := 0
    };
    
    if (!map)
        return info;
    endif
    
    var completed := GetObjProperty(map, "Completed");
    if (completed)
        info.completed := 1;
        info.completed_on := completed;
        info.completed_by_serial := GetObjProperty(map, "CompletedBy");
        
        if (info.completed_by_serial)
            var finder := SystemFindObjectBySerial(info.completed_by_serial, SYSFIND_SEARCH_OFFLINE_MOBILES);
            info.completed_by := finder ? finder.name : GetObjProperty(map, "CompletedByName");
        endif
    endif
    
    return info;
endfunction

function ValidateTreasureMap(who, map)
    if (!map)
        return struct{"valid" := 0, "errortext" := "Mapa inválido"};
    endif
    
    // Verifica se é um mapa do tesouro
    if (!IsTreasureMap(map))
        return struct{"valid" := 0, "errortext" := "Item não é um mapa do tesouro"};
    endif
    
    // Verifica se já foi completado
    if (IsMapSolved(map))
        var info := GetMapCompletionInfo(map);
        return struct{
            "valid" := 0, 
            "errortext" := "Mapa já foi desvendado por " + info.completed_by,
            "completion_info" := info
        };
    endif
    
    // Verifica se o jogador pode usar
    if (!CanDecodeMap(who, map))
        return struct{"valid" := 0, "errortext" := "Você não tem as habilidades necessárias para decifrar este mapa"};
    endif
    
    return struct{"valid" := 1, "errortext" := ""};
endfunction