use uo;
use os;
use util;
use datafile;

// ============================================================================
// formationCore.inc — Núcleo do Sistema de Comando de Tropas
// ============================================================================
// Biblioteca PURA + I/O de pelotão (squad). NÃO move NPC, NÃO trata evento,
// NÃO abre gump. Apenas: cálculo de slots, rotação por lookup das 4 cardeais,
// e persistência de squads em DataFile (molde fog_regions, ADR-012).
//
// Reusada por: squadController.src (processo do squad) e formacao.src (comando
// do jogador). Por isso NENHUMA função depende de me/self() — tudo é parâmetro.
//
// Convenções de slot:
//   lat = lateral, + = direita da formação ; fwd = profundidade, 0 = frente,
//   negativo = retaguarda. Tudo multiplicado por Spacing. Rotação só nas 4
//   cardeais (Facing 0/2/4/6) por lookup — sem trig, evita cisalhamento.
//
//   Roster (Via A): SlotIndex é FIXO por NPC; mortes deixam buracos até um
//   Reformar, que chama AssignSlots de novo para refechar a linha.
// ============================================================================

// ── Comandos de brain (faixa nova 0xA1XX — separada do 0xA00X de pet) ────────
const FORMATION_CMD     := 0xA100;   // entra/atualiza slot na formação
const FORMATION_BREAK   := 0xA101;   // sai da formação

// ── Formações (string persistida no squad) ───────────────────────────────────
const FORM_LINHA        := "LINHA";
const FORM_COLUNA       := "COLUNA";
const FORM_QUADRADO     := "QUADRADO";
const FORM_CUNHA        := "CUNHA";

// ── Ordens ────────────────────────────────────────────────────────────────────
const ORD_MANTER        := "MANTER";
const ORD_AVANCAR       := "AVANCAR";
const ORD_RECUAR        := "RECUAR";
const ORD_REFORMAR      := "REFORMAR";
const ORD_ATACAR        := "ATACAR";
const ORD_CARGA         := "CARGA";
const ORD_ASSASSINAR    := "ASSASSINAR";

// ── Largura default do QUADRADO (colunas) ─────────────────────────────────────
const SQUAD_QUAD_WIDTH  := 4;

// ── DataFile ──────────────────────────────────────────────────────────────────
const FORMATION_DATA_FILE := ":tropas:squads";

// ============================================================================
// SEÇÃO 1 — CÁLCULO PURO (testável sem mundo)
// ============================================================================

// ── Tabela de offsets locais {lat, fwd} para N unidades ──────────────────────
// Retorna array 1-indexed; offset[i] corresponde ao SlotIndex i (1..N).
function FormationOffsets(formation, count, spacing)
    var offsets := array{};
    if (count < 1)
        return offsets;
    endif
    if (spacing < 1)
        spacing := 1;
    endif

    var i;
    if (formation == FORM_LINHA)
        // largura N, 1 fileira ; centrado
        var half := CInt((count - 1) / 2);
        for (i := 0; i < count; i := i + 1)
            offsets.append(struct{ lat := (i - half) * spacing, fwd := 0 });
        endfor

    elseif (formation == FORM_COLUNA)
        // 1 coluna, profundidade N
        for (i := 0; i < count; i := i + 1)
            offsets.append(struct{ lat := 0, fwd := (0 - i) * spacing });
        endfor

    elseif (formation == FORM_QUADRADO)
        var w := SQUAD_QUAD_WIDTH;
        var halfw := CInt((w - 1) / 2);
        for (i := 0; i < count; i := i + 1)
            var col := i % w;
            var row := CInt(i / w);
            offsets.append(struct{ lat := (col - halfw) * spacing,
                                   fwd := (0 - row) * spacing });
        endfor

    elseif (formation == FORM_CUNHA)
        // fileira r tem ate (2r+1) unidades, fwd = -r ; apice na frente (r=0).
        // Preenchimento CENTRADO em cada fileira: comeca em lat=0, depois -1,+1,
        // depois -2,+2, ... ate completar (2r+1) ou esgotar count. Garante
        // simetria mesmo quando a ultima fileira fica incompleta (corrige bug
        // anterior que varria -r..+r e gerava cunha torta com n nao-triangular).
        var r := 0;
        var placed := 0;
        while (placed < count)
            offsets.append(struct{ lat := 0, fwd := (0 - r) * spacing });
            placed := placed + 1;
            if (placed >= count)
                break;
            endif
            var m := 1;
            while (m <= r && placed < count)
                offsets.append(struct{ lat := (0 - m) * spacing, fwd := (0 - r) * spacing });
                placed := placed + 1;
                if (placed >= count)
                    break;
                endif
                offsets.append(struct{ lat := m * spacing, fwd := (0 - r) * spacing });
                placed := placed + 1;
                m := m + 1;
            endwhile
            r := r + 1;
        endwhile

    else
        // formação desconhecida → trata como LINHA (degrada suave)
        var half := CInt((count - 1) / 2);
        for (i := 0; i < count; i := i + 1)
            offsets.append(struct{ lat := (i - half) * spacing, fwd := 0 });
        endfor
    endif

    return offsets;
endfunction

// ── Rotaciona um offset local {lat,fwd} para deslocamento mundial {dx,dy} ─────
// Apenas 4 cardeais. Facing UO: 0=N, 2=E, 4=S, 6=O.
function RotateOffset(lat, fwd, facing)
    var fx, fy, rx, ry;
    case (facing)
        0:  fx := 0;  fy := -1; rx := 1;  ry := 0;   // Norte
        2:  fx := 1;  fy := 0;  rx := 0;  ry := 1;   // Leste
        4:  fx := 0;  fy := 1;  rx := -1; ry := 0;   // Sul
        6:  fx := -1; fy := 0;  rx := 0;  ry := -1;  // Oeste
        default:
            fx := 0;  fy := -1; rx := 1;  ry := 0;   // fallback Norte
    endcase

    return struct{ dx := (lat * rx) + (fwd * fx),
                   dy := (lat * ry) + (fwd * fy) };
endfunction

// ── Posição mundial {x,y} do slot de índice slot_index (1-indexed) ────────────
// squad: struct com AnchorX, AnchorY, Formation, Facing, Spacing, Members.
function SlotWorldPos(squad, slot_index)
    var count := squad.Members.size();
    if (slot_index < 1 || slot_index > count)
        return error{"errortext" := "slot_index fora do range"};
    endif

    var offsets := FormationOffsets(squad.Formation, count, squad.Spacing);
    var off := offsets[slot_index];
    var w := RotateOffset(off.lat, off.fwd, squad.Facing);

    return struct{ x := squad.AnchorX + w.dx, y := squad.AnchorY + w.dy };
endfunction

// ── Atribuição greedy-nearest de slots aos membros (Via A + refecha) ──────────
// members: array de seriais. Retorna array PARALELO de slot_index (1-indexed),
// onde resultado[k] = qual slot o membro members[k] deve ocupar.
// Usado na criação E pelo Reformar (refecha a linha do zero).
function AssignSlots(squad)
    var members := squad.Members;
    var count := members.size();
    var result := array{};
    if (count < 1)
        return result;
    endif

    // posições-alvo de cada slot
    var slot_pos := array{};
    var offsets := FormationOffsets(squad.Formation, count, squad.Spacing);
    var s;
    for (s := 1; s <= count; s := s + 1)
        var w := RotateOffset(offsets[s].lat, offsets[s].fwd, squad.Facing);
        slot_pos.append(struct{ x := squad.AnchorX + w.dx, y := squad.AnchorY + w.dy });
    endfor

    // slots ainda livres (1..count)
    var free_slots := array{};
    for (s := 1; s <= count; s := s + 1)
        free_slots.append(s);
    endfor

    // pré-popula result com 0
    var k;
    for (k := 1; k <= count; k := k + 1)
        result.append(0);
    endfor

    // para cada membro, pega o slot livre mais próximo da posição ATUAL dele
    for (k := 1; k <= count; k := k + 1)
        var mob := SystemFindObjectBySerial(members[k]);
        if (!mob)
            continue;   // membro morto/sumido: deixa result[k]=0, reconcilia depois
        endif

        var best_fi := 0;          // índice dentro de free_slots
        var best_dist := -1;
        var fi;
        for (fi := 1; fi <= free_slots.size(); fi := fi + 1)
            var sp := slot_pos[ free_slots[fi] ];
            var d := CoordinateDistance(mob.x, mob.y, sp.x, sp.y);
            if (best_dist == -1 || d < best_dist)
                best_dist := d;
                best_fi := fi;
            endif
        endfor

        if (best_fi > 0)
            result[k] := free_slots[best_fi];
            free_slots.erase(best_fi);
        endif
    endfor

    return result;
endfunction

// ============================================================================
// SEÇÃO 2 — PERSISTÊNCIA DE SQUAD (DataFile, molde fog_regions)
// ============================================================================

function _OpenSquadsDF()
    var df := OpenDataFile(FORMATION_DATA_FILE);
    if (!df)
        df := CreateDataFile(FORMATION_DATA_FILE, DF_KEYTYPE_STRING);
    endif
    return df;
endfunction

// ── monta struct de squad a partir de um elemento de datafile ─────────────────
function _ElemToSquad(elem, key)
    var sq := struct;
    sq.+SquadId         := key;
    sq.+CommanderSerial := CInt(elem.GetProp("CommanderSerial"));
    sq.+Formation       := CStr(elem.GetProp("Formation"));
    sq.+Facing          := CInt(elem.GetProp("Facing"));
    sq.+Spacing         := CInt(elem.GetProp("Spacing"));
    sq.+Order           := CStr(elem.GetProp("Order"));
    sq.+AnchorX         := CInt(elem.GetProp("AnchorX"));
    sq.+AnchorY         := CInt(elem.GetProp("AnchorY"));
    sq.+AnchorZ         := CInt(elem.GetProp("AnchorZ"));
    sq.+Realm           := CStr(elem.GetProp("Realm"));
    sq.+TargetX         := CInt(elem.GetProp("TargetX"));
    sq.+TargetY         := CInt(elem.GetProp("TargetY"));
    sq.+PrioritySerial  := CInt(elem.GetProp("PrioritySerial"));

    var members := elem.GetProp("Members");
    if (!members || members.errortext || TypeOf(members) != "Array")
        members := array{};
    endif
    sq.+Members := members;

    return sq;
endfunction

// ── carrega squad por id (chave) ──────────────────────────────────────────────
function SquadLoad(squad_id)
    var df := _OpenSquadsDF();
    if (!df)
        return error{"errortext" := "datafile de squads indisponivel"};
    endif
    var elem := df.FindElement(squad_id);
    if (!elem)
        return error{"errortext" := "squad nao encontrado: " + squad_id};
    endif
    return _ElemToSquad(elem, squad_id);
endfunction

// ── grava squad (cria se nao existe) ──────────────────────────────────────────
function SquadSave(squad)
    var df := _OpenSquadsDF();
    if (!df)
        return error{"errortext" := "datafile de squads indisponivel"};
    endif
    var elem := df.FindElement(squad.SquadId);
    if (!elem)
        elem := df.CreateElement(squad.SquadId);
    endif
    if (elem.errortext)
        return elem;
    endif

    elem.SetProp("CommanderSerial", squad.CommanderSerial);
    elem.SetProp("Formation",       squad.Formation);
    elem.SetProp("Facing",          squad.Facing);
    elem.SetProp("Spacing",         squad.Spacing);
    elem.SetProp("Order",           squad.Order);
    elem.SetProp("AnchorX",         squad.AnchorX);
    elem.SetProp("AnchorY",         squad.AnchorY);
    elem.SetProp("AnchorZ",         squad.AnchorZ);
    elem.SetProp("Realm",           squad.Realm);
    elem.SetProp("TargetX",         squad.TargetX);
    elem.SetProp("TargetY",         squad.TargetY);
    elem.SetProp("PrioritySerial",  squad.PrioritySerial);
    elem.SetProp("Members",         squad.Members);

    return 1;
endfunction

// ── cria squad novo a partir de um comandante + posicao inicial do anchor ─────
function SquadCreate(squad_id, commander, formation, facing, spacing)
    if (spacing < 1)
        spacing := 1;
    endif
    var sq := struct;
    sq.+SquadId         := squad_id;
    sq.+CommanderSerial := commander.serial;
    sq.+Formation       := formation;
    sq.+Facing          := facing;
    sq.+Spacing         := spacing;
    sq.+Order           := ORD_MANTER;
    sq.+AnchorX         := commander.x;
    sq.+AnchorY         := commander.y;
    sq.+AnchorZ         := commander.z;
    sq.+Realm           := commander.realm;
    sq.+TargetX         := commander.x;
    sq.+TargetY         := commander.y;
    sq.+PrioritySerial  := 0;
    sq.+Members         := array{};

    var res := SquadSave(sq);
    if (res.errortext)
        return res;
    endif
    return sq;
endfunction

function SquadDelete(squad_id)
    var df := _OpenSquadsDF();
    if (!df)
        return error{"errortext" := "datafile de squads indisponivel"};
    endif
    return df.DeleteElement(squad_id);
endfunction

// ── adiciona um serial ao roster (sem duplicar) ───────────────────────────────
function SquadAddMember(squad, serial)
    var m;
    foreach m in (squad.Members)
        if (m == serial)
            return squad;   // ja esta
        endif
    endforeach
    squad.Members.append(serial);
    return squad;
endfunction

// ── remove um serial do roster (constroi novo array — nunca erase em foreach) ─
function SquadRemoveMember(squad, serial)
    var novo := array{};
    var m;
    foreach m in (squad.Members)
        if (m != serial)
            novo.append(m);
        endif
    endforeach
    squad.Members := novo;
    return squad;
endfunction

// ── limpa membros mortos/sumidos do roster (chamar no Setup do controller) ────
function SquadReconcile(squad)
    var novo := array{};
    var m;
    foreach m in (squad.Members)
        var mob := SystemFindObjectBySerial(m);
        if (mob && !mob.dead)
            novo.append(m);
        endif
    endforeach
    squad.Members := novo;
    return squad;
endfunction
