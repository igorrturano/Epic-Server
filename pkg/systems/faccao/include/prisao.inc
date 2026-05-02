// =============================================================================
// prisao.inc — Helpers compartilhados do sistema de prisao de faccao
// =============================================================================
//
// Centraliza toda a logica reutilizavel de prisao para evitar duplicacao entre
// prisaotimer.src, prisaoteleport.src e outros scripts do modulo.
//
// CONSTANTES
//   FINE_PRISON         — valor base da multa por tier (tier * FINE_PRISON = multa)
//   PRISON_WAIT_MINUTES — minutos de espera entre gumps de cobranca
//
// FUNCOES PRINCIPAIS
//   GetCrimeDesc(tier)            — retorna descricao textual do crime por tier (1/2/3)
//   GetReleasePoint(g)            — ponto de liberacao: le prop "release_point" da guild
//                                   ou cai no padrao por staff_tipo
//   ContarMoedas(who)             — conta total de moedas no backpack (cobre/prata/ouro)
//   CobrarMoedas(who, amount)     — remove exatamente 'amount' moedas do backpack
//   AvisarLider(g, msg)           — envia sysmsg ao lider se estiver online
//   LabelMochila(who)             — gera label "Mochila de Raca Genero Cabelo [Barba]"
//                                   usando props raca/chardata, camadas de cabelo e barba
//   ConfiscarItens(who, g)        — confisca arma equipada, armas/municao da mochila e
//                                   itens proibidos por lei noitem; deposita no bau da guild
//   AguardarProximoGump(who, msg) — loop de espera com monitoramento de desconexao;
//                                   envia msg ao reconectar com tempo restante formatado
//   _EsperarConexao(who)          — loop simples aguardando who.connected == true
//
// REQUER no .src pai:
//   use uo; use os; use guilds; include "include/say";
// =============================================================================

const FINE_PRISON         := 20;  // Valor base da multa por tier
const PRISON_WAIT_MINUTES := 1;   // Minutos entre gumps de cobranca

// ── Crime ─────────────────────────────────────────────────────────────────────

function GetCrimeDesc(tier)
    if (tier == 1) return "Infracao Leve"; endif
    if (tier == 2) return "Infracao Moderada"; endif
    return "Infracao Grave";
endfunction

// ── Ponto de liberacao apos pagar multa ───────────────────────────────────────
// Verifica primeiro prop da guild (release_point); cai no padrao por staff_tipo.

function GetReleasePoint(g)
    var rp := g.getProp("release_point");
    if (rp != error && rp && rp.x)
        return rp;
    endif
    var tipo := CStr(g.getProp("staff_tipo"));
    case (tipo)
        "regentes":
            return struct{ "x" := 1170, "y" := 1044, "z" := 0, "realm" := "britannia_alt" };
        "conselho_kaijin": "conselho_shaman": "conselho_aradal":
        "conselho_velkyn": "conselho_evocatium":
            return struct{ "x" := 1149, "y" := 1091, "z" := 0, "realm" := "britannia_alt" };
        default:
            return struct{ "x" := 1195, "y" := 1083, "z" := 0, "realm" := "britannia_alt" };
    endcase
endfunction

// ── Moedas ────────────────────────────────────────────────────────────────────

function _CoinValor(objtype)
    if (objtype == 0xba63) return 1;   endif
    if (objtype == 0xba64) return 10;  endif
    if (objtype == 0xba65) return 100; endif
    return 0;
endfunction

function ContarMoedas(who)
    var bp := who.backpack;
    if (!bp) return 0; endif
    var total := 0;
    foreach item in EnumerateItemsInContainer(bp)
        sleepms(2);
        var v := _CoinValor(item.objtype);
        if (v) total += item.amount * v; endif
    endforeach
    return total;
endfunction

function CobrarMoedas(who, amount)
    var bp := who.backpack;
    if (!bp) return; endif
    var left := amount;
    foreach item in EnumerateItemsInContainer(bp)
        sleepms(2);
        if (left <= 0) break; endif
        var v := _CoinValor(item.objtype);
        if (!v) continue; endif
        var can_take := CInt(left / v);
        if (can_take >= item.amount)
            left -= item.amount * v;
            DestroyItem(item);
        else
            item.amount -= can_take;
            left := 0;
        endif
    endforeach
endfunction

// ── Avisar lider ──────────────────────────────────────────────────────────────

function AvisarLider(g, msg)
    var lider_serial := CInt(g.getProp("leader"));
    if (!lider_serial) return; endif
    var lider := SystemFindObjectBySerial(lider_serial);
    if (!lider || !lider.isA(POLCLASS_MOBILE) || !lider.connected) return; endif
    SendSysMessageEx(lider, "[FACCAO] " + msg, SSM_HARMFUL);
endfunction

// ── Label da mochila (descricao fisica do player) ─────────────────────────────

function LabelMochila(who)
    var _raca_raw := GetObjProperty(who, "raca");
    var raca;
    if (_raca_raw != error && _raca_raw)
        raca := CStr(_raca_raw);
    else
        var _cd := GetObjProperty(who, "chardata");
        raca := (_cd != error && _cd && _cd.raca) ? CStr(_cd.raca) : "Desconhecido";
    endif
    var genero      := (who.gender == 0) ? "Masculino" : "Feminino";
    var cabelo_item := GetEquipmentByLayer(who, 0xB);
    var cabelo      := cabelo_item ? cabelo_item.name : "sem cabelo";
    var barba_item  := GetEquipmentByLayer(who, 0x10);
    var barba       := (barba_item && barba_item.name) ? barba_item.name : "";
    var label := "Mochila de {} {} {}".format(raca, genero, cabelo);
    if (barba != "") label += " " + barba; endif
    return label;
endfunction

// ── Confisco de itens ─────────────────────────────────────────────────────────

function ConfiscarItens(who, g)
    var law_tools := array{ 0xec4, 0x13e3, 0xFB4, 0xFB5, 0x13f6, 0xe85, 0xf43, 0xec2, 0x1F020 };

    var bau_serial := g.getProp("bau_serial");
    if (!bau_serial)
        AvisarLider(g, "A faccao nao possui um bau configurado. Itens de {} nao puderam ser guardados.".format(who.name));
        return;
    endif
    var bau := SystemFindObjectBySerial(bau_serial);
    if (!bau)
        AvisarLider(g, "O bau da faccao nao foi encontrado. Itens de {} nao puderam ser guardados.".format(who.name));
        return;
    endif

    var laws := g.getProp("laws");
    if (!laws) laws := array{}; endif
    var confiscados := array{};

    if (who.weapon && !(who.weapon.objtype in law_tools))
        confiscados.append(who.weapon);
    endif

    var bp := who.backpack;
    if (bp)
        foreach item in EnumerateItemsInContainer(bp)
            sleepms(2);
            if (item in confiscados) continue; endif
            var _isweapon := item.isA(POLCLASS_WEAPON);
            var _istool   := (item.objtype in law_tools);
            var _ismun    := (item.graphic == 0xf3f || item.graphic == 0x1bfb);
            if ((_isweapon && !_istool) || _ismun)
                confiscados.append(item);
            endif
        endforeach
    endif

    foreach law in laws
        sleepms(2);
        if (!law.on || law.type != "noitem" || !law.opt) continue; endif
        var opt_objtype := CInt(law.opt.objtype ? law.opt.objtype : law.opt);
        if (!opt_objtype) continue; endif
        foreach item in ListEquippedItems(who)
            sleepms(2);
            if (item.objtype == opt_objtype && !(item in confiscados))
                confiscados.append(item);
            endif
        endforeach
    endforeach

    if (!confiscados.size()) return; endif

    var label          := LabelMochila(who);
    var estava_travado := bau.locked;
    if (estava_travado) bau.locked := 0; endif

    var mochila := CreateItemInContainer(bau, 0xe75, 1);
    if (!mochila)
        if (estava_travado) bau.locked := 1; endif
        return;
    endif
    mochila.name := label;

    foreach item in confiscados
        sleepms(2);
        MoveItemToContainer(item, mochila);
    endforeach

    if (estava_travado) bau.locked := 1; endif

    SetObjProperty(who, "#prisao_mochila_serial", mochila.serial);
    SendSysMessageEx(who, "Seus itens foram confiscados pela faccao.", SSM_HARMFUL);
endfunction

// ── Devolucao de itens ao sair da prisao ─────────────────────────────────────

function DevolverItens(who, g)
    var mochila_serial := GetObjProperty(who, "#prisao_mochila_serial");
    if (mochila_serial == error || !mochila_serial) return; endif

    EraseObjProperty(who, "#prisao_mochila_serial");

    var mochila := SystemFindObjectBySerial(mochila_serial);
    if (!mochila) return; endif

    var bp := who.backpack;
    if (!bp) return; endif

    foreach item in EnumerateItemsInContainer(mochila)
        sleepms(2);
        MoveItemToContainer(item, bp);
    endforeach

    DestroyItem(mochila);
    SendSysMessageEx(who, "Seus itens confiscados foram devolvidos.", SSM_INFO);
endfunction

// ── Espera de conexao e timer entre gumps ────────────────────────────────────

function _EsperarConexao(who)
    while (!who.connected)
        Sleep(30);
    endwhile
endfunction

function AguardarProximoGump(who, msg_template)
    var deadline := ReadGameClock() + (PRISON_WAIT_MINUTES * 60);
    while (ReadGameClock() < deadline)
        Sleep(30);
        if (!who.connected)
            _EsperarConexao(who);
            var _falta := deadline - ReadGameClock();
            if (_falta < 0) _falta := 0; endif
            SendSysMessage(who, msg_template.format(CInt(_falta / 60) + 1));
        endif
    endwhile
endfunction
