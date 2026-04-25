
//Author: Deiro | Rafael Santos
use os;

include "include/epicConstants";
include ":containers:storageAreas";

CONST MOEDA_MENOR_VALOR := SESTERTIUS;

enum class TIPO_CARTEIRA_MOEDA
	PLAYER := "jogador",
	BANCO := "banco",
endenum

function ConverterMoeda(quantidade,moedaDe,moedaPara := MOEDA_MENOR_VALOR)

	var retorno := struct;
	retorno.+valorConvertido := 0;
	retorno.+resto :=0;

	if(TypeOfInt(quantidade) != OT_LONG)
		return error{"errortext" := "Parâmetro quantidade com tipo errado."};
	endif

	if(moedaDe == moedaPara)
		retorno.valorConvertido := quantidade;
		retorno.resto :=0;
		return retorno;
	endif

	case(moedaDe)
		SESTERTIUS:
			case(moedaPara)
				DENARIUS:
					if (quantidade < 10)
						return error;
					endif
					retorno.valorConvertido := quantidade / 10;
					retorno.resto := quantidade % 10;
				AUREUS:
					if (quantidade < 100)
						return error;
					endif
					retorno.valorConvertido := quantidade / 100;
					retorno.resto := quantidade % 100;
			endcase
		DENARIUS:
			case(moedaPara)
				SESTERTIUS:
					retorno.valorConvertido := quantidade * 10;
				AUREUS:
					if (quantidade < 10)
						return error;
					endif
					retorno.valorConvertido := quantidade / 10;
					retorno.resto := quantidade % 10;
			endcase
		AUREUS:
			case(moedaPara)
				SESTERTIUS:
					retorno.valorConvertido := quantidade * 100;
				DENARIUS:
					retorno.valorConvertido := quantidade * 10;
			endcase
	endcase
	
	return retorno;

endfunction

//Conta a quantidade de pilhas e moedas que tem na mochila e as guarda. retorno struct.quantidade,struct.[array]items
function ContaMoedas(arrayItens, tiposAProcurar, color:=0)
	var envelopeMoedas := struct {totalMoedaMenorValor := 0, moedas := dictionary, container := 0};

	foreach obj_type in tiposAProcurar//varre os tipos
		sleepms(2);
		var props := struct;
		props.+quantidade := 0;
		props.+items := array;

		foreach item in arrayItens
			sleepms(2);
			if( item.objtype == obj_type )
				if ( !color || item.color == color )
					if(!envelopeMoedas.container)
						envelopeMoedas.container := item.container;
					endif
					props.quantidade += item.amount; // soma tudo.
					props.items.append(item); // guarda todas as pilhas de moedas
				endif
			endif
		endforeach

		envelopeMoedas.totalMoedaMenorValor += ConverterMoeda(props.quantidade, obj_type, MOEDA_MENOR_VALOR).valorConvertido;  
		envelopeMoedas.moedas.insert(cstr(obj_type),props);
	endforeach

	return envelopeMoedas;
endfunction

function ContaCapital(quem, regiaoBanco := 0)

	var objetoRetorno := struct;
	var carteiras := dictionary;
	var tiposMoedas := array {SESTERTIUS,DENARIUS,AUREUS};

	var itensNoJogador := EnumerateItemsInContainer(quem.backpack);
	carteiras.insert(TIPO_CARTEIRA_MOEDA::PLAYER,ContaMoedas(itensNoJogador,tiposMoedas));

	if(regiaoBanco)
		var containerBanco := CP_GetStorageContainerForMobile(quem, regiaoBanco, CP_NOCREATE);
		var itensNoBanco := EnumerateItemsInContainer(containerBanco);
		carteiras.insert(TIPO_CARTEIRA_MOEDA::BANCO,ContaMoedas(itensNoBanco,tiposMoedas));
	endif

	var totalMoedaMenorValor := 0;

	foreach tipoCarteira in (carteiras.keys())
		Sleepms(2);
			totalMoedaMenorValor += carteiras[tipoCarteira].totalMoedaMenorValor;
	endforeach

	objetoRetorno.+carteiras := carteiras;
	objetoRetorno.+valorTotal := totalMoedaMenorValor;

	return objetoRetorno;
endfunction

function EfetuaDebito( quem, debito, regiaoBanco := CP_DEFAULT_AREA , tipoMoeda:= MOEDA_MENOR_VALOR)
	var tiposMoedas := array {AUREUS,DENARIUS,SESTERTIUS};
	var capital := ContaCapital(quem, regiaoBanco);
	var debitoConvertido := ConverterMoeda(debito,tipoMoeda);
	
	if(capital.valorTotal >= debitoConvertido.valorConvertido)
		var chavesCarteiras := capital.carteiras.keys();
		chavesCarteiras.reverse(); //inverto para a carteira do jogador vir primeiro.
		foreach carteira in (chavesCarteiras)
			Sleepms(2);
			var carteiraAtual := capital.carteiras[carteira];
			var valorRestante := carteiraAtual.totalMoedaMenorValor - debitoConvertido.valorConvertido;

			if(debitoConvertido.valorConvertido > 0)
				var envelopeMoeda := carteiraAtual.moedas[cstr(MOEDA_MENOR_VALOR)];
				if(debitoConvertido.valorConvertido <= envelopeMoeda.quantidade && debitoConvertido.resto == 0) // se tem valor suficiente de moeda baixa, já cria debitando delas.
					CriarMoedas(
								capital.carteiras[carteira].container,
								MOEDA_MENOR_VALOR,
								envelopeMoeda.quantidade - debitoConvertido.valorConvertido,
								envelopeMoeda.items
								);
				else
					if(valorRestante <= 0) // se negativo, não há dinheiro suficiente na carteira, deleta todas moedas e segue em frente.
						var todasMoedas := carteiraAtual.moedas[cstr(SESTERTIUS)].items
											+ carteiraAtual.moedas[cstr(DENARIUS)].items
											+ carteiraAtual.moedas[cstr(AUREUS)].items;
						ConsomeMoedas(todasMoedas);
					else //valor restante positivo, mas chegou ate aqui, significa que não tem valor suficiente em moeda baixa, destroi todas e cria o restante variado.
						TrocarMoedas(carteiraAtual.container,valorRestante,tiposMoedas,carteiraAtual.moedas);
					endif
				endif
				debitoConvertido.valorConvertido := valorRestante * -1;
			endif
		endforeach
		return true;
	endif
	return error{"errortext" := "Moedas insuficientes."};
endfunction

//Troca todas as moedas por moedas da maior para a menor.
function TrocarMoedas(
						container,
						quantidade,
						tiposMoedas,
						moedasDict
					)

	var indice := 1;
	var valorRestante := quantidade;
	var moedaDe := MOEDA_MENOR_VALOR;

	foreach tipos in tiposMoedas
		var carteira := ConverterMoeda(valorRestante,moedaDe,tiposMoedas[indice]);
		var items := moedasDict[cstr(tiposMoedas[indice])].items;
		
		if(carteira != error)
			CriarMoedas(container,tiposMoedas[indice],carteira.valorConvertido,items);
		else
			CriarMoedas(container,moedaDe,valorRestante,items);
		endif

		valorRestante := carteira.resto;
		indice := indice + 1;
		if(indice > tiposMoedas.size())
			break;
		endif
	endforeach
	
endfunction

function ConsomeMoedas(items)
	foreach item in items
		 DestroyItem(item);
	endforeach
endfunction

function CriarMoedas(container,tipo, valor,items := array)
	ConsomeMoedas(items);
	if(valor > 0)
		var retorno := CreateItemInContainer( container, tipo, valor);
	endif
endfunction

