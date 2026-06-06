// coins.inc — Helpers de moedas (cobre/prata/ouro) para sistemas da Liga do Pacto Comum
//
// Uso: include ":ligafaccao:include/coins";
//      var total := CountCoins(player);
//      TakeCoins(player, 25);
//
// Conversão: 1 prata = 10 cobres, 1 ouro = 100 cobres. Pagamento consome moedas
// de maior valor quando necessário e devolve troco em cobres.

const MOEDA_COBRE := 0xba63;
const MOEDA_PRATA := 0xba64;
const MOEDA_OURO  := 0xba65;

function CountCoins(player)
    if (!player.backpack) return 0; endif
    var total := 0;
    foreach item in EnumerateItemsInContainer(player.backpack)
        sleepms(2);
        if (item.objtype == MOEDA_COBRE)     total += item.amount;
        elseif (item.objtype == MOEDA_PRATA) total += item.amount * 10;
        elseif (item.objtype == MOEDA_OURO)  total += item.amount * 100;
        endif
    endforeach
    return total;
endfunction

function TakeCoins(player, amount)
    if (!player.backpack) return 0; endif
    var left := amount;
    set_critical(1);
    foreach item in EnumerateItemsInContainer(player.backpack)
        sleepms(2);
        if (left <= 0) break; endif
        var valor := 1;
        if    (item.objtype == MOEDA_PRATA) valor := 10;
        elseif (item.objtype == MOEDA_OURO) valor := 100;
        elseif (item.objtype != MOEDA_COBRE) continue;
        endif
        var total_item := item.amount * valor;
        if (total_item <= left)
            left -= total_item;
            DestroyItem(item);
        else
            var take_units := CInt((left + valor - 1) / valor);
            var troco      := (take_units * valor) - left;
            SubtractAmount(item, take_units);
            if (troco > 0) CreateItemInBackpack(player, MOEDA_COBRE, troco); endif
            left := 0;
        endif
    endforeach
    set_critical(0);
    return (left <= 0);
endfunction
