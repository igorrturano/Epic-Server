// recallnpc.inc — Convocar/Localizar agentes contratados da facção
//
// Dois comportamentos no mesmo botão (ao lado do nome do NPC no mural):
//   GM    → teleporta o GM até o NPC (só observa, sem cooldown).
//   Player → puxa o NPC pra posição atual do player, se ele estiver em terreno
//            da facção. Cooldown de 1h por player.
//
// Permissão: privilégio "recallnpc" ("Convocar Agentes"). Líder tem por default.

use uo;
use os;
use polsys;
use guilds;

const RECALL_COOLDOWN  := 3600; // segundos
const RECALL_CD_PROP   := "recall_npc_cd";

// Formata um tempo em segundos como "Xm Ys" ou "Xh Ym".
function _FormatarCooldown(segundos)
    if (segundos <= 0) return "0s"; endif
    if (segundos < 60) return CStr(segundos) + "s"; endif
    if (segundos < 3600)
        var m := CInt(segundos / 60);
        var s := segundos - (m * 60);
        return CStr(m) + "m " + CStr(s) + "s";
    endif
    var h := CInt(segundos / 3600);
    var m := CInt((segundos - (h * 3600)) / 60);
    return CStr(h) + "h " + CStr(m) + "m";
endfunction

// Handler chamado quando o botão "convocar/teleportar" é clicado em um NPC.
//   who      — quem clicou
//   guild    — guild da facção (GuildRef, não guildid)
//   npc_idx  — índice do NPC em guild_obj.npcs_active (1-based)
function _RecallOrTeleportNpcHandler(who, guild, npc_idx)
    var guild_obj := GetFaccao(guild);
    if (!guild_obj || guild_obj == error)
        return SendSysMessageEx(who, "Faccao nao encontrada.", SSM_FAIL);
    endif

    var npcs := guild_obj.npcs_active;
    if (npc_idx < 1 || npc_idx > npcs.size())
        return SendSysMessageEx(who, "Agente nao encontrado.", SSM_FAIL);
    endif

    var npc_data := npcs[npc_idx];
    var npc := SystemFindObjectBySerial(npc_data.serial);
    if (!npc)
        return SendSysMessageEx(who, "Agente nao encontrado no mundo.", SSM_FAIL);
    endif

    // ── GM: apenas teleporta o GM até o NPC ────────────────────────────────
    if (who.cmdlevel >= 1)
        MoveObjectToLocation(who, npc.x, npc.y, npc.z, npc.realm, MOVEOBJECT_FORCELOCATION);
        SendSysMessageEx(who,
                         "Teleportado para " + npc.name + " ({}, {}, {})".format(npc.x, npc.y, npc.z),
                         SSM_INFO);
        return 1;
    endif

    // ── Player: convoca o NPC pra posição do player ────────────────────────

    // Cooldown
    var agora := polcore().systime;
    var cd_ate := CInt(GetObjProperty(who, RECALL_CD_PROP));
    if (cd_ate > agora)
        var restam := cd_ate - agora;
        return SendSysMessageEx(who,
                                "Seu agente ainda esta cansado. Tente novamente em " + _FormatarCooldown(restam) + ".",
                                SSM_FAIL);
    endif

    // Terreno da facção — inGuildRect (faccao.inc) cobre tanto guild.area
    // quanto guild.territories via DataFile :architect:areas.
    if (!inGuildRect(guild, who))
        return SendSysMessageEx(who,
                                "Voce deve estar em terreno da faccao, livre para o NPC voltar.",
                                SSM_FAIL);
    endif

    // Move o NPC
    var ok := MoveObjectToLocation(npc, who.x, who.y, who.z, who.realm, MOVEOBJECT_FORCELOCATION);
    if (!ok || ok == error)
        return SendSysMessageEx(who, "Nao foi possivel convocar o agente.", SSM_FAIL);
    endif

    SetObjProperty(who, RECALL_CD_PROP, agora + RECALL_COOLDOWN);

    SendSysMessageEx(who, npc.name + " foi convocado.", SSM_INFO);
    AddReport(guild, who.name + " convocou agente " + npc.name);
    return 1;
endfunction
