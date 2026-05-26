// tree_pieces.inc — Fallen log pieces: spawn, track, and damage.
// Include path: ":ultimalive:include/tree_pieces"

use uo;
use basic;
use math;

include ":attributes:attributes";
include ":crafting:toolWear";
include "include/say";

const UL_TREE_PIECES_DF   := ":ultimalive:tree_pieces";
const UL_PIECE_GRAPHIC_1  := 0x0CF5;
const UL_PIECE_GRAPHIC_2  := 0x0CF6;
const UL_PIECE_GRAPHIC_3  := 0x0CF7;
const UL_PIECE_HP_INITIAL := 35;

function _UL_GetTreePiecesDF()
    var df := DFOpenDataFile(UL_TREE_PIECES_DF, DF_CREATE);
    if (!df)
        return error{"errortext" := "UL: failed to open tree_pieces datafile"};
    endif
    return df;
endfunction

function _UL_TreePieceKey(realm, x, y)
    return CStr(realm) + " " + CStr(CInt(x)) + " " + CStr(CInt(y));
endfunction

function _UL_DirStep(direction, base_x, base_y, steps)
    var tx := CInt(base_x);
    var ty := CInt(base_y);
    var s  := CInt(steps);
    case (direction)
        "LESTE": tx := tx + s;
        "OESTE": tx := tx - s;
        "NORTE": ty := ty - s;
        "SUL":   ty := ty + s;
    endcase
    var pos := struct{};
    pos.+x  := tx;
    pos.+y  := ty;
    return pos;
endfunction

function _UL_PieceGraphic(piece_index)
    case (piece_index)
        1: return UL_PIECE_GRAPHIC_1;
        2: return UL_PIECE_GRAPHIC_2;
        3: return UL_PIECE_GRAPHIC_3;
    endcase
    return UL_PIECE_GRAPHIC_1;
endfunction

function _UL_SpawnPiece(realm, x, y, z, graphic, meta, parent_x, parent_y, piece_index)
    var item := CreateItemAtLocation(CInt(x), CInt(y), CInt(z), graphic, 1, realm);
    if (item.errortext)
        return 0;
    endif
    item.movable := 0;
    item.name    := "Pedaco de tronco de " + meta.type_name;
    SetObjProperty(item, "tree_piece",  1);
    SetObjProperty(item, "log_type",    CInt(meta.log_type));
    SetObjProperty(item, "parent_x",    CInt(parent_x));
    SetObjProperty(item, "parent_y",    CInt(parent_y));
    SetObjProperty(item, "piece_index", CInt(piece_index));

    var df := _UL_GetTreePiecesDF();
    if (df.errortext)
        return 0;
    endif
    var key := _UL_TreePieceKey(realm, x, y);
    var el  := df.FindElement(key);
    if (!el)
        el := df.CreateElement(key);
    endif
    el.SetProp("Serial",     item.serial);
    el.SetProp("PieceIndex", CInt(piece_index));
    el.SetProp("HP",         UL_PIECE_HP_INITIAL);
    el.SetProp("LogType",    CInt(meta.log_type));
    el.SetProp("ParentX",    CInt(parent_x));
    el.SetProp("ParentY",    CInt(parent_y));
    return 1;
endfunction

// Tenta spawnar 3 pedacos em linha na 'direction'. Retorna 1 em sucesso.
function _UL_TryDirection(direction, realm, stump_x, stump_y, stump_z, meta)
    var p1 := _UL_DirStep(direction, stump_x, stump_y, 1);
    var p2 := _UL_DirStep(direction, stump_x, stump_y, 2);
    var p3 := _UL_DirStep(direction, stump_x, stump_y, 3);

    if (CanWalk("L", CInt(stump_x), CInt(stump_y), CInt(stump_z), p1.x, p1.y, realm).errortext)
        return 0;
    endif
    var z1 := GetWorldHeight(p1.x, p1.y, realm);
    if (CanWalk("L", p1.x, p1.y, z1, p2.x, p2.y, realm).errortext)
        return 0;
    endif
    var z2 := GetWorldHeight(p2.x, p2.y, realm);
    if (CanWalk("L", p2.x, p2.y, z2, p3.x, p3.y, realm).errortext)
        return 0;
    endif
    var z3 := GetWorldHeight(p3.x, p3.y, realm);

    _UL_SpawnPiece(realm, p1.x, p1.y, z1, _UL_PieceGraphic(1), meta, stump_x, stump_y, 1);
    _UL_SpawnPiece(realm, p2.x, p2.y, z2, _UL_PieceGraphic(2), meta, stump_x, stump_y, 2);
    _UL_SpawnPiece(realm, p3.x, p3.y, z3, _UL_PieceGraphic(3), meta, stump_x, stump_y, 3);
    return 1;
endfunction

function _UL_GetPieceData(realm, x, y)
    var df := _UL_GetTreePiecesDF();
    if (df.errortext)
        return error{"errortext" := "tree_pieces DF indisponivel"};
    endif
    var key := _UL_TreePieceKey(realm, x, y);
    var el  := df.FindElement(key);
    if (!el)
        return error{"errortext" := "UL_GetPieceData: posicao nao registrada"};
    endif
    var d := struct{};
    d.+serial      := CInt(el.GetProp("Serial"));
    d.+piece_index := CInt(el.GetProp("PieceIndex"));
    d.+hp          := CInt(el.GetProp("HP"));
    d.+log_type    := CInt(el.GetProp("LogType"));
    d.+parent_x    := CInt(el.GetProp("ParentX"));
    d.+parent_y    := CInt(el.GetProp("ParentY"));
    return d;
endfunction

function _UL_DestroyPiece(realm, x, y)
    var df := _UL_GetTreePiecesDF();
    if (df.errortext)
        return;
    endif
    var key := _UL_TreePieceKey(realm, x, y);
    var el  := df.FindElement(key);
    if (!el)
        return;
    endif
    var serial := CInt(el.GetProp("Serial"));
    el.Delete();
    var item := SystemFindObjectBySerial(serial);
    if (!item.errortext)
        DestroyItem(item);
    endif
endfunction

// — API publica ——————————————————————————————————————————————

function UL_IsFallenLog(item)
    if (!item)
        return 0;
    endif
    return GetObjProperty(item, "tree_piece") == 1;
endfunction

// Chamada quando o trunk vira toco (fase 3). Spawna 3 pedacos em linha
// ou entrega madeira direta se todas as direcoes estiverem bloqueadas.
function UL_CreateFallenLogs(realm, x, y, z_trunk, meta, who)
    var primary   := "";
    var secondary := "";

    if (RandomInt(2) == 0)
        primary   := "LESTE";
        secondary := "OESTE";
    else
        primary   := "OESTE";
        secondary := "LESTE";
    endif

    if (_UL_TryDirection(primary,   realm, x, y, z_trunk, meta))
        return;
    endif
    if (_UL_TryDirection(secondary, realm, x, y, z_trunk, meta))
        return;
    endif
    if (_UL_TryDirection("NORTE",   realm, x, y, z_trunk, meta))
        return;
    endif
    if (_UL_TryDirection("SUL",     realm, x, y, z_trunk, meta))
        return;
    endif

    // Nenhuma direcao livre — 3 skill checks virtuais individuais (F2)
    PrintTextAbove(who, "*A arvore desaba sobre si mesma. Voce recolhe a madeira do chao.*");
    var per_piece := CInt(EffectiveSkill(who, "Lumberjacking") / 20);
    if (per_piece < 1)
        per_piece := 1;
    endif
    var total := 0;
    for i := 1 to 3
        if (SkillCheck(who, "Lumberjacking", 30) > 0)
            total := total + per_piece;
        endif
    endfor
    if (total > 0)
        CreateItemInBackPack(who, meta.log_type, total);
    endif
endfunction

// Retorna: 0 = pedaco nao encontrado; 1 = golpe aplicado, vivo; 2 = destruido
function UL_DamagePiece(realm, x, y, who, tool)
    var data := _UL_GetPieceData(realm, x, y);
    if (data.errortext)
        return 0;
    endif

    var skill_val := CInt(EffectiveSkill(who, "Lumberjacking"));
    var damage    := CInt(skill_val / 2);
    var new_hp    := data.hp;
    var diff      := DetermineLumberDifficulty(data.log_type);

    if (SkillCheck(who, "Lumberjacking", diff) > 0)
        new_hp := new_hp - damage;
        if (new_hp <= 0)
            _UL_DestroyPiece(realm, x, y);
            var amt := CInt(skill_val / 20);
            if (amt < 1)
                amt := 1;
            endif
            CreateItemInBackPack(who, data.log_type, amt);
            SendSysMessage(who, "Voce recolhe a madeira do chao.");
            CheckToolWear(who, tool, "Lumberjacking");
            return 2;
        endif
    else
        new_hp := new_hp - CInt(damage / 2);
        if (new_hp <= 0)
            _UL_DestroyPiece(realm, x, y);
            SendSysMessage(who, "O pedaco de madeira se fragmenta.");
            CheckToolWear(who, tool, "Lumberjacking");
            return 2;
        endif
    endif

    var df := _UL_GetTreePiecesDF();
    if (!df.errortext)
        var key := _UL_TreePieceKey(realm, x, y);
        var el  := df.FindElement(key);
        if (el)
            el.SetProp("HP", new_hp);
        endif
    endif

    CheckToolWear(who, tool, "Lumberjacking");
    return 1;
endfunction
