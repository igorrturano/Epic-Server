use uo;
use os;
use basic;
use math;

include ":currency:include/currency";

const PIGEON_SCROLL_OBJTYPE  := 0xCA02;
const PIGEON_COST            := 2;
const PIGEON_COIN_TYPE       := 0xBA63;
const PIGEON_MAX_PLAYERS     := 60;
const PIGEON_MSG_MAX_LEN     := 200;
const PIGEON_FLIGHT_DIST     := 50;

const PIGEON_DEPARTING  := 0;
const PIGEON_SEEKING    := 1;
const PIGEON_FOLLOWING  := 2;
const PIGEON_LEAVING    := 3;

const PIGEON_ANIMAL_PIGEON   := 0;
const PIGEON_ANIMAL_RAT      := 1;
const PIGEON_ANIMAL_CAT      := 2;
const PIGEON_ANIMAL_FERRET   := 3;
const PIGEON_ANIMAL_SQUIRREL := 4;
const PIGEON_ANIMAL_CROW     := 5;
const PIGEON_ANIMAL_ARARA    := 6;
const PIGEON_ANIMAL_COUNT    := 7;

function PigeonGetTemplate(animal)
	case (CInt(animal))
		PIGEON_ANIMAL_PIGEON:   return ":pigeoncorreio:DeliveryPigeon";
		PIGEON_ANIMAL_RAT:      return ":pigeoncorreio:DeliveryRat";
		PIGEON_ANIMAL_CAT:      return ":pigeoncorreio:DeliveryCat";
		PIGEON_ANIMAL_FERRET:   return ":pigeoncorreio:DeliveryFerret";
		PIGEON_ANIMAL_SQUIRREL: return ":pigeoncorreio:DeliverySquirrel";
		PIGEON_ANIMAL_CROW:     return ":pigeoncorreio:DeliveryCrow";
		PIGEON_ANIMAL_ARARA:    return ":pigeoncorreio:DeliveryArara";
		default:                return ":pigeoncorreio:DeliveryPigeon";
	endcase
endfunction

function PigeonGetAnimalName(animal)
	case (CInt(animal))
		PIGEON_ANIMAL_PIGEON:   return "Pombo";
		PIGEON_ANIMAL_RAT:      return "Rato";
		PIGEON_ANIMAL_CAT:      return "Gato";
		PIGEON_ANIMAL_FERRET:   return "Furao";
		PIGEON_ANIMAL_SQUIRREL: return "Esquilo";
		PIGEON_ANIMAL_CROW:     return "Corvo";
		PIGEON_ANIMAL_ARARA:    return "Arara";
		default:                return "Pombo";
	endcase
endfunction

function PigeonGetArrivalMsg(animal)
	case (CInt(animal))
		PIGEON_ANIMAL_PIGEON:   return "Um pombo mensageiro está chegando até você!";
		PIGEON_ANIMAL_RAT:      return "Um rato mensageiro está chegando até você!";
		PIGEON_ANIMAL_CAT:      return "Um gato mensageiro está chegando até você!";
		PIGEON_ANIMAL_FERRET:   return "Um furao mensageiro está chegando até você!";
		PIGEON_ANIMAL_SQUIRREL: return "Um esquilo mensageiro está chegando até você!";
		PIGEON_ANIMAL_CROW:     return "Um corvo mensageiro está chegando até você!";
		PIGEON_ANIMAL_ARARA:    return "Uma arara mensageira está chegando até você!";
		default:                return "Um animal mensageiro está chegando até você!";
	endcase
endfunction

function PigeonGetArrivalEmote(animal)
	case (CInt(animal))
		PIGEON_ANIMAL_PIGEON:   return "*chega voando*";
		PIGEON_ANIMAL_RAT:      return "*chega correndo*";
		PIGEON_ANIMAL_CAT:      return "*chega furtivamente*";
		PIGEON_ANIMAL_FERRET:   return "*chega correndo velozmente*";
		PIGEON_ANIMAL_SQUIRREL: return "*salta e chega saltitando*";
		PIGEON_ANIMAL_CROW:     return "*pousa planejando*";
		PIGEON_ANIMAL_ARARA:    return "*chega grasnando*";
		default:                return "*chega*";
	endcase
endfunction

function PigeonGetFollowEmote(animal)
	case (CInt(animal))
		PIGEON_ANIMAL_PIGEON:   return "*para com um papel preso na perna*";
		PIGEON_ANIMAL_RAT:      return "*para com um rolo preso no pescoco*";
		PIGEON_ANIMAL_CAT:      return "*ronrona e exibe um rolo na coleira*";
		PIGEON_ANIMAL_FERRET:   return "*para com um papel preso na coleira*";
		PIGEON_ANIMAL_SQUIRREL: return "*segura um bilhete nas patas*";
		PIGEON_ANIMAL_CROW:     return "*exibe um papel no bico*";
		PIGEON_ANIMAL_ARARA:    return "*repete: mensagem! mensagem!*";
		default:                return "*para com um papel*";
	endcase
endfunction

function PigeonGetLeaveEmote(animal)
	case (CInt(animal))
		PIGEON_ANIMAL_PIGEON:   return "*voa embora*";
		PIGEON_ANIMAL_RAT:      return "*corre e some*";
		PIGEON_ANIMAL_CAT:      return "*se espreguica e vai embora*";
		PIGEON_ANIMAL_FERRET:   return "*some rapidamente*";
		PIGEON_ANIMAL_SQUIRREL: return "*salta de galho em galho e some*";
		PIGEON_ANIMAL_CROW:     return "*alca voo*";
		PIGEON_ANIMAL_ARARA:    return "*voa grasnando*";
		default:                return "*vai embora*";
	endcase
endfunction

function PigeonConsumeCost(who)
	if (!who.backpack)
		return 0;
	endif
	return SpendCoin(who, PIGEON_COST, who.backpack, PIGEON_COIN_TYPE);
endfunction

// Retorna {mob, apelido} de jogadores online. dict=0 retorna todos; dict=conhecidos filtra pelos reconhecidos.
function PigeonBuildPlayerList(who, dict)
	var result := array{};
	foreach chr in EnumerateOnlineCharacters()
		if (chr.serial == who.serial)
			SleepMS(2);
			continue;
		endif

		var chave := CStr(chr.serial);
		var apelido;
		if (dict)
			if (!dict.Exists(chave))
				SleepMS(2);
				continue;
			endif
			apelido := CStr(dict[chave]);
		else
			apelido := CStr(GetObjProperty(chr, "nome_inicial"));
			if (!apelido || apelido == "0")
				apelido := CStr(chr.name);
			endif
		endif

		var entry := struct;
		entry.+mob     := chr;
		entry.+apelido := apelido;
		result.Append(entry);
		if (result.Size() >= PIGEON_MAX_PLAYERS)
			break;
		endif
		SleepMS(2);
	endforeach
	return result;
endfunction

function PigeonGetAllOnline(who)
	return PigeonBuildPlayerList(who, 0);
endfunction

function PigeonGetConhecidos(who)
	var dict := GetObjProperty(who, "conhecidos");
	if (!dict)
		return array{};
	endif
	return PigeonBuildPlayerList(who, dict);
endfunction
