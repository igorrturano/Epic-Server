// =============================================================================
// aluguelporta.inc — Helpers do sistema de aluguel de portas de faccao
// =============================================================================
//
// Permite que portas registradas em "portas_serials" de uma faccao sejam
// marcadas como alugaveis. O lider seleciona o inquilino via mural, o player
// recebe um gump de oferta, aceita e ganha uma chave de ferro nova. A cobranca
// semanal e feita pelo daemon GlobalInnControl (estalagemcontrol.src) que le a
// global property FACCAO_ALUGAVEL_DOORS_PROP. O dinheiro vai pra pool da
// guild via prop "money".
//
// PROPS NA PORTA
//   Alugavel         (1)      flag de quarto a venda
//   RentName         (string) nome do quarto
//   RentValue        (int)    aluguel semanal em cobre
//   RentGuildSerial  (int)    guildid pra creditar
//   RentInquilino    (int)    serial do player; 0 = vago
//   RentData         (int)    polcore().systime do ultimo pagamento
//
// GLOBAL
//   FACCAO_ALUGAVEL_DOORS_PROP — array de seriais. O daemon itera so essa lista
//   pra nao varrer toda guild a cada ciclo.
//
// REQUER no .src pai:
//   use uo; use os; use guilds;
// =============================================================================

const FACCAO_ALUGAVEL_DOORS_PROP := "FaccaoAlugavelDoors";
const FACCAO_RENT_DAYS           := 7;

const _APORT_COBRE := 0xba63;
const _APORT_PRATA := 0xba64;
const _APORT_OURO  := 0xba65;
const _APORT_GOLD  := 0xeed;

// ── Registro global ──────────────────────────────────────────────────────────

function FaccaoAlugavel_AddDoor(porta_serial)
    var lst := GetGlobalProperty(FACCAO_ALUGAVEL_DOORS_PROP);
    if (lst == error || TypeOf(lst) != "Array")
        lst := array{};
    endif
    if (CInt(porta_serial) in lst)
        return 0;
    endif
    lst.append(CInt(porta_serial));
    SetGlobalProperty(FACCAO_ALUGAVEL_DOORS_PROP, lst);
    return 1;
endfunction

function FaccaoAlugavel_RemoveDoor(porta_serial)
    var lst := GetGlobalProperty(FACCAO_ALUGAVEL_DOORS_PROP);
    if (lst == error || TypeOf(lst) != "Array")
        return 0;
    endif
    var i := 1;
    while (i <= lst.size())
        if (CInt(lst[i]) == CInt(porta_serial))
            lst.erase(i);
            SetGlobalProperty(FACCAO_ALUGAVEL_DOORS_PROP, lst);
            return 1;
        endif
        i := i + 1;
    endwhile
    return 0;
endfunction

// Retorna serial da porta onde o player ja e inquilino, ou 0. Bloqueia
// inquilino duplo entre portas de faccao — analogo ao _AchaQuartoExistente
// da estalagem.
function FaccaoAlugavel_FindInquilinoDoor(player_serial)
    var lst := GetGlobalProperty(FACCAO_ALUGAVEL_DOORS_PROP);
    if (lst == error || TypeOf(lst) != "Array")
        return 0;
    endif
    foreach serial in lst
        var porta := SystemFindObjectBySerial(CInt(serial));
        if (porta == error)
            sleepms(2);
            continue;
        endif
        if (CInt(GetObjProperty(porta, "RentInquilino")) == CInt(player_serial))
            return CInt(serial);
        endif
        sleepms(2);
    endforeach
    return 0;
endfunction

// ── LockId ───────────────────────────────────────────────────────────────────

// Aloca novo LockId, seta na porta e devolve. Usado em selecao de inquilino e
// em despejo — invalida chave antiga em ambos os casos.
function FaccaoAlugavel_RotateLockId(porta)
    var lockid := AllocLockId();
    SetObjProperty(porta, "LockId", lockid);
    porta.locked := 1;
    return lockid;
endfunction

// ── Pool de dinheiro da guild ────────────────────────────────────────────────

function FaccaoAlugavel_CreditarGuild(guild_serial, amount)
    var g := FindGuild(CInt(guild_serial));
    if (!g) return 0; endif
    var atual := CInt(g.getProp("money"));
    g.SetProp("money", atual + CInt(amount));
    return 1;
endfunction

// ── Cobranca: banco -> mochila ───────────────────────────────────────────────

// Conta moedas (em cobre) na mochila do player.
function _AlugFac_ContarMochila(player)
    var bp := player.backpack;
    if (!bp) return 0; endif
    var total := 0;
    foreach item in EnumerateItemsInContainer(bp)
        sleepms(2);
        if (item.objtype == _APORT_COBRE)
            total += item.amount;
        elseif (item.objtype == _APORT_PRATA)
            total += item.amount * 10;
        elseif (item.objtype == _APORT_OURO || item.objtype == _APORT_GOLD)
            total += item.amount * 100;
        endif
    endforeach
    return total;
endfunction

// Conta moedas (em cobre) no banco do player.
function _AlugFac_ContarBanco(bank_box)
    if (!bank_box) return 0; endif
    var total := 0;
    foreach item in EnumerateItemsInContainer(bank_box)
        sleepms(2);
        if (item.objtype == _APORT_COBRE)
            total += item.amount;
        elseif (item.objtype == _APORT_PRATA)
            total += item.amount * 10;
        elseif (item.objtype == _APORT_OURO || item.objtype == _APORT_GOLD)
            total += item.amount * 100;
        endif
    endforeach
    return total;
endfunction

// Devolve troco para um container, em ouro/prata/cobre.
function _AlugFac_DarTroco(container, valor)
    if (valor <= 0) return; endif
    var falta := valor;
    var ouro := CInt(falta / 100);
    if (ouro > 0)
        CreateItemInContainer(container, _APORT_OURO, ouro);
    endif
    falta -= ouro * 100;
    var prata := CInt(falta / 10);
    if (prata > 0)
        CreateItemInContainer(container, _APORT_PRATA, prata);
    endif
    falta -= prata * 10;
    if (falta > 0)
        CreateItemInContainer(container, _APORT_COBRE, falta);
    endif
endfunction

// Cobra 'amount' (em cobre) de um container, drenando cobre->prata->ouro com
// troco. Retorna 1 se cobrou tudo, 0 se nao tinha o suficiente (nao mexe em
// nada se nao tiver). Usado tanto pra banco quanto pra mochila.
function _AlugFac_DebitarContainer(container, amount)
    if (!container || amount <= 0) return 0; endif
    var total := _AlugFac_ContarBanco(container);
    if (total < amount) return 0; endif

    Set_Critical(1);
    var falta := amount;

    // Cobre primeiro (sem troco).
    foreach item in EnumerateItemsInContainer(container)
        if (falta <= 0) break; endif
        if (item.objtype != _APORT_COBRE) continue; endif
        if (item.amount <= falta)
            falta -= item.amount;
            DestroyItem(item);
        else
            SubtractAmount(item, falta);
            falta := 0;
        endif
        sleepms(2);
    endforeach

    // Prata depois (troco em cobre se sobrar).
    if (falta > 0)
        foreach item in EnumerateItemsInContainer(container)
            if (falta <= 0) break; endif
            if (item.objtype != _APORT_PRATA) continue; endif
            var valor_item := item.amount * 10;
            if (valor_item <= falta)
                falta -= valor_item;
                DestroyItem(item);
            else
                var troco := valor_item - falta;
                DestroyItem(item);
                _AlugFac_DarTroco(container, troco);
                falta := 0;
            endif
            sleepms(2);
        endforeach
    endif

    // Ouro por ultimo.
    if (falta > 0)
        foreach item in EnumerateItemsInContainer(container)
            if (falta <= 0) break; endif
            if (item.objtype != _APORT_OURO && item.objtype != _APORT_GOLD) continue; endif
            var valor_item := item.amount * 100;
            if (valor_item <= falta)
                falta -= valor_item;
                DestroyItem(item);
            else
                var troco := valor_item - falta;
                DestroyItem(item);
                _AlugFac_DarTroco(container, troco);
                falta := 0;
            endif
            sleepms(2);
        endforeach
    endif

    Set_Critical(0);
    return (falta == 0) ? 1 : 0;
endfunction

// Tenta cobrar 'amount' do banco do player (na regiao 'bank_region') e, se
// faltar, cai pra mochila. Retorna 1 em sucesso, 0 se nao tinha o suficiente
// somando os dois. Nada e debitado parcialmente — ou cobra tudo ou nada.
function FaccaoAlugavel_DebitarInquilino(player, bank_region, amount)
    if (!player || player == error) return 0; endif
    if (amount <= 0) return 1; endif

    var bp := player.backpack;
    var bank_box := CP_GetStorageContainerForMobile(player, bank_region, CP_NOCREATE);

    var total_banco := _AlugFac_ContarBanco(bank_box);
    var total_mochila := _AlugFac_ContarMochila(player);
    if (total_banco + total_mochila < amount) return 0; endif

    if (total_banco >= amount)
        return _AlugFac_DebitarContainer(bank_box, amount);
    endif

    // Banco cobre o que puder, mochila completa.
    var do_banco := total_banco;
    var do_mochila := amount - do_banco;
    var ok_banco := 1;
    if (do_banco > 0)
        ok_banco := _AlugFac_DebitarContainer(bank_box, do_banco);
    endif
    if (!ok_banco) return 0; endif
    return _AlugFac_DebitarContainer(bp, do_mochila);
endfunction
