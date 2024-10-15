
use os;

include "include/epicConstants";

CONST MOEDA_MENOR_VALOR := SESTERTIUS;

function ConverterMoeda(quantidade,moedaDe,moedaPara := MOEDA_MENOR_VALOR)

	var retorno := struct;
	retorno.+valorConvertido := 0;
	retorno.+resto :=0;

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
function GetVariosObjetosMochila(mobile, obj_type_array, color:=0)
	var quantidade := 0;
	var retorno :=  dictionary;
	var items := EnumerateItemsInContainer(mobile.backpack);

	foreach obj_type in obj_type_array//varre os tipos
		var props := struct;
		props.+quantidade := 0;
		props.+items := array;
		foreach item in items
			if( item.objtype == obj_type )
				if ( !color || item.color == color )
					props.quantidade := quantidade + item.amount; // soma tudo.
					props.items.append(item); // guarda todas as pilhas de moedas
				endif
			endif
			sleepms(2);
		endforeach
		retorno.insert(obj_type,props);
	endforeach
	return retorno;
endfunction

function ContaCapital(who, moedasDict := dictionary)

	if(moedasDict.size() <= 0)
		moedasDict := GetVariosObjetosMochila(who,array {SESTERTIUS,DENARIUS,AUREUS} );
	endif

	var totalMoedaMenorValor := 0;
	var keys := moedasDict.keys();

	foreach tipo in keys;
	 	var carteira := ConverterMoeda(moedasDict[tipo].quantidade,tipo,MOEDA_MENOR_VALOR);
		totalMoedaMenorValor := totalMoedaMenorValor + carteira.valorConvertido;
	endforeach
	
	return totalMoedaMenorValor;
endfunction

function EfetuaDebito( who, debito, tipoMoeda:= MOEDA_MENOR_VALOR)

	var tiposMoedas := array {AUREUS,DENARIUS,SESTERTIUS};
	var moedasDict := GetVariosObjetosMochila(who,tiposMoedas);
	var capital := ContaCapital(who, moedasDict);
	var carteiraDebito := ConverterMoeda(debito,tipoMoeda);
	
	if(capital >= carteiraDebito.valorConvertido)
		var valorRestante := capital - carteiraDebito.valorConvertido;
		 //se tiver o valor suficiente com moedas baixas, consome-as direto.
		if(carteiraDebito.valorConvertido <= moedasDict[MOEDA_MENOR_VALOR].quantidade  && carteiraDebito.resto == 0)
			CriarMoedas(
						who,
						MOEDA_MENOR_VALOR,
						moedasDict[MOEDA_MENOR_VALOR].quantidade - carteiraDebito.valorConvertido,
						moedasDict[MOEDA_MENOR_VALOR].items
						);
		else
			TrocarMoedas(who,valorRestante,tiposMoedas,moedasDict);
		endif
		return 1;
	endif
	return 0;
endfunction

//Troca todas as moedas por moedas da maior para a menor.
function TrocarMoedas(
						who,
						quantidade,
						tiposMoedas,
						moedasDict
					)

	var indice := 1;
	var valorRestante := quantidade;
	var moedaDe := MOEDA_MENOR_VALOR;

	foreach tipos in tiposMoedas
		var carteira := ConverterMoeda(valorRestante,moedaDe,tiposMoedas[indice]);
		var items := moedasDict[tiposMoedas[indice]].items;
		
		if(carteira != error)
			CriarMoedas(who,tiposMoedas[indice],carteira.valorConvertido,items);
		else
			CriarMoedas(who,moedaDe,valorRestante,items);
		endif

		valorRestante := carteira.resto;
		indice := indice + 1;
		if(indice > tiposMoedas.size())
			break;
		endif
	endforeach
	
endfunction

function CriarMoedas(who,tipo, valor,items := array)
	foreach item in items
		DestroyItem(item);
	endforeach
	if(valor > 0)
		CreateItemInBackpack( who, tipo, valor);
	endif
endfunction

