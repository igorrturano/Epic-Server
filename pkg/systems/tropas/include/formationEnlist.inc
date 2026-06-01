use uo;
use os;
use guilds;

include ":tropas:formationCore";
include ":faccao:include/membership";

// ============================================================================
// formationEnlist.inc — Emissão de ordens de formação + permissão de comando.
// ============================================================================
// MIOLO REUSÁVEL: chamado HOJE pela fala (ProcessSpeech do tamedBrain) e DEPOIS
// pelo gump. Ambos chamam as MESMAS funções aqui — a fala/gump são só cascas
// finas. O brain (tamedBrain) só RECEBE o FORMATION_CMD; quem EMITE é aqui.
//
// IMPORTANTE — formação NÃO mexe em master/owner: o swarm do GM já chega tamado
// (makeswarm seta master). Soldado de facção (fase futura) não é pet. A formação
// só lê o vínculo pra decidir quem pode comandar (CanCommandNPC) e seta CProps
// de slot no NPC — nunca cria nem remove posse.
// ============================================================================

const FORMATION_DEFAULT_RANGE := 12;   // raio pra juntar "meus NPCs perto"

// ── Permissão: este comandante pode comandar este NPC? (regra X — membro basta)
function CanCommandNPC(commander, npc)
    // (1) Staff (GM+) comanda qualquer NPC.
    if (commander.cmdlevel >= 3)
        return 1;
    endif

    // (2) Tame proprio — master nativo, com fallback no CProp owner.
    if (npc.master == commander)
        return 1;
    endif
    if (GetObjProperty(npc, "owner") == commander.serial)
        return 1;
    endif

    // (3) Mesma faccao — REGRA X: basta ser membro (sem checar cargo).
    var npc_guildid := GetObjProperty(npc, "guild");
    if (!npc_guildid)
        return 0;
    endif
    var faccao := FindGuild(npc_guildid);
    if (!faccao)
        return 0;
    endif
    if (IsMemberOfFaccao(faccao, commander))
        return 1;
    endif

    return 0;
endfunction

// ── squad_id determinístico por comandante ───────────────────────────────────
// Mesmo comandante = mesma chave. Garante idempotência: se N NPCs ouvem a fala
// e cada um dispara FormationOrder, todos recriam o MESMO squad (último vence),
// em vez de criar N squads. Ver nota de idempotência no FormationOrder.
function FormationSquadId(commander)
    return "squad_" + CStr(commander.serial);
endfunction

// ── Junta "meus NPCs perto" — padrão B (ListMobiles + master), confirmado E1/E3
// Inclui swarm GM (que NAO está em TamedPets) e exclui estabulados/limbo de graca.
function _CollectMyNPCs(commander, raio)
    var membros := array{};
    foreach mob in (ListMobilesNearLocation(commander.x, commander.y, commander.z, raio, commander.realm))
        if (mob.IsA(POLCLASS_NPC) && !mob.dead && CanCommandNPC(commander, mob))
            membros.append(mob.serial);
        endif
        sleepms(2);
    endforeach
    return membros;
endfunction

// ── ORDEM DE FORMAÇÃO — o ponto de entrada que fala E gump chamam ─────────────
// commander: quem ordena (player/GM). formation_type: FORM_LINHA/COLUNA/etc.
// Retorna struct{count, squad_id} ou error{}.
function FormationOrder(commander, formation_type, facing, spacing, raio)
    if (raio < 1)
        raio := FORMATION_DEFAULT_RANGE;
    endif
    if (spacing < 1)
        spacing := 1;
    endif

    var membros := _CollectMyNPCs(commander, raio);
    if (membros.size() < 1)
        return error{"errortext" := "nenhum NPC seu por perto"};
    endif

    var squad_id := FormationSquadId(commander);

    // Idempotente: recria o squad do zero a cada ordem. Anchor = posicao atual
    // do comandante no momento da ordem. (Anchor movel = fase futura.)
    SquadDelete(squad_id);
    var squad := SquadCreate(squad_id, commander, formation_type, facing, spacing);
    if (squad.errortext)
        return squad;
    endif

    var s;
    foreach s in (membros)
        squad := SquadAddMember(squad, s);
    endforeach
    var saved := SquadSave(squad);
    if (saved.errortext)
        return saved;
    endif

    // Atribui slots (greedy nearest, do formationCore) e emite FORMATION_CMD
    // individual pra cada membro com SEU slot.
    var slots := AssignSlots(squad);
    var k;
    for (k := 1; k <= membros.size(); k := k + 1)
        var npc := SystemFindObjectBySerial(membros[k]);
        if (!npc)
            continue;
        endif
        var ev := struct{ "type"   := FORMATION_CMD,
                          "source" := commander,
                          "targ"   := struct{ "squad_id"   := squad_id,
                                              "slot_index" := slots[k] } };
        SendEvent(npc, ev);
        sleepms(2);
    endfor

    return struct{ "count" := membros.size(), "squad_id" := squad_id };
endfunction

// ── DISPERSAR — tira todos os meus NPCs da formação ───────────────────────────
function FormationDisperse(commander, raio)
    if (raio < 1)
        raio := FORMATION_DEFAULT_RANGE;
    endif

    var n := 0;
    foreach mob in (ListMobilesNearLocation(commander.x, commander.y, commander.z, raio, commander.realm))
        if (mob.IsA(POLCLASS_NPC) && !mob.dead && CanCommandNPC(commander, mob))
            SendEvent(mob, struct{ "type" := FORMATION_BREAK, "source" := commander });
            n := n + 1;
        endif
        sleepms(2);
    endforeach

    SquadDelete(FormationSquadId(commander));
    return n;
endfunction
