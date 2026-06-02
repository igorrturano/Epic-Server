use uo;
use os;

// Sistema de moedas compartilhado pelos NPCs da Liga.
// Extraído de curandeiro.src para evitar duplicação.

const MOEDA_COBRE := 0xba63;
const MOEDA_PRATA := 0xba64;
const MOEDA_OURO  := 0xba65;
const PRATA_VALUE := 10;
const OURO_VALUE  := 100;

function CountCoins(player)
    if (!player.backpack) return 0; endif
    var total := 0;
    foreach item in EnumerateItemsInContainer(player.backpack)
        sleepms(2);
        if (item.objtype == MOEDA_COBRE)      total += item.amount;
        elseif (item.objtype == MOEDA_PRATA)  total += item.amount * PRATA_VALUE;
        elseif (item.objtype == MOEDA_OURO)   total += item.amount * OURO_VALUE;
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
        if (item.objtype == MOEDA_PRATA) valor := PRATA_VALUE;
        elseif (item.objtype == MOEDA_OURO) valor := OURO_VALUE;
        elseif (item.objtype != MOEDA_COBRE) continue;
        endif
        var total_item := item.amount * valor;
        if (total_item <= left)
            left -= total_item;
            DestroyItem(item);
        else
            // Retorna troco em cobre se necessário
            var take_units := CInt((left + valor - 1) / valor);
            var troco      := (take_units * valor) - left;
            SubtractAmount(item, take_units);
            if (troco > 0)
                CreateItemInBackpack(player, MOEDA_COBRE, troco);
            endif
            left := 0;
        endif
    endforeach
    set_critical(0);
    return (left <= 0);
endfunction
