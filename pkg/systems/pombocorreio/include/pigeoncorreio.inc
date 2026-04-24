// =============================================================
// Sistema de Pombo-Correio - Funções Compartilhadas
// =============================================================

use uo;
use os;
use basic;
use math;

include ":currency:include/currency";

// Objtype do bilhete criado pelo pombo
const PIGEON_SCROLL_OBJTYPE := 0xCA02;

// Custo do envio em moedas de cobre
const PIGEON_COST          := 2;
const PIGEON_COIN_TYPE     := 0xBA63; // moedacobre (Sestertius)

// Limites do gump de envio
const PIGEON_MAX_PLAYERS   := 60;
const PIGEON_MSG_MAX_LEN   := 200;

// Estados do pombo (usados como CProp "state")
const PIGEON_DEPARTING  := 0;
const PIGEON_SEEKING    := 1;
const PIGEON_FOLLOWING  := 2;
const PIGEON_LEAVING    := 3;

// Distância de partida/chegada (tiles)
const PIGEON_FLIGHT_DIST := 50;

/*
 * PigeonConsumeCost(who)
 *
 * Tenta consumir PIGEON_COST moedas de cobre da mochila do jogador.
 * Retorna 1 em sucesso, 0 se não tiver moedas suficientes.
 */
function PigeonConsumeCost(who)
	if (!who.backpack)
		return 0;
	endif

	return SpendCoin(who, PIGEON_COST, who.backpack, PIGEON_COIN_TYPE);
endfunction

/*
 * PigeonGetConhecidos(who)
 *
 * Retorna array de structs {mob, apelido} com os jogadores online
 * que o remetente já reconheceu via .r e que estão conectados.
 * Limitado a PIGEON_MAX_PLAYERS.
 */
function PigeonGetConhecidos(who)
	var conhecidos := GetObjProperty(who, "conhecidos");
	if (!conhecidos)
		return array{};
	endif

	var result := array{};
	foreach chr in EnumerateOnlineCharacters()
		if (chr.serial == who.serial)
			SleepMS(2);
			continue;
		endif

		var chave := CStr(chr.serial);
		if (conhecidos.Exists(chave))
			var entry := struct;
			entry.+mob     := chr;
			entry.+apelido := CStr(conhecidos[chave]);
			result.Append(entry);
			if (result.Size() >= PIGEON_MAX_PLAYERS)
				break;
			endif
		endif
		SleepMS(2);
	endforeach
	return result;
endfunction
