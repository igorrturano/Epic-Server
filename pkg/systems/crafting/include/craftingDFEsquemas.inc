use uo;
use os;

include ":datafile:datafile";

function AdicionaEsquema(quem, nomeReceitaClassificada, esquema)
	var dFile := DFOpenDataFile(":crafting:esquemas", DF_CREATE);
	var elemento := DFFindElement(dFile, quem.client.accname, DF_CREATE);

	if(elemento)
		TrataRegistrosMortos(); // Limpa informação de personagens deletados.

		var receitas := elemento.GetProp(Hex(quem.serial));
		var registroEsquemas := dictionary;
		var isNovaReceita := true;

		if(receitas)
			if(receitas.exists(nomeReceitaClassificada))
				registroEsquemas := receitas[nomeReceitaClassificada];
				isNovaReceita := false;
			endif
		else
			receitas := dictionary;
		endif
		
		if(!registroEsquemas.exists(esquema.nome))
			registroEsquemas.insert(esquema.nome,esquema);
		else
			return error{errortext := "Já existe esquema com este nome para esta receita."};
		endif
		
		if(isNovaReceita)
			receitas.insert(nomeReceitaClassificada,registroEsquemas);
		else
			receitas[nomeReceitaClassificada] := registroEsquemas;
		endif

		elemento.SetProp(Hex(quem.serial), receitas);
		return elemento;
	endif

	return error{"errortext" := "Erro inesperado."};
endfunction

function EditaEsquema(quem, nomeReceitaClassificada,esquema)
	var dFile := DFOpenDataFile(":crafting:esquemas", DF_CREATE);
	var elemento := DFFindElement(dFile, quem.client.accname, DF_CREATE);

	if(elemento)
		TrataRegistrosMortos(); // Limpa informação de personagens deletados.

		var receitas := elemento.GetProp(Hex(quem.serial));
		var registroEsquemas := receitas[nomeReceitaClassificada];
		registroEsquemas[esquema.nome] := esquema;

		receitas[nomeReceitaClassificada] := registroEsquemas;

		elemento.SetProp(Hex(quem.serial), receitas);
		return esquema;
	endif

	return error{"errortext" := "Erro inesperado."};
endfunction

function ListaEsquemas(quem, nomeReceitaClassificada)
	var dFile := DFOpenDataFile(":crafting:esquemas", DF_CREATE);
	var elemento := DFFindElement(dFile, quem.client.accname, DF_CREATE);
	
	if(elemento)
		TrataRegistrosMortos(); // Limpa informação de personagens deletados.

		var receitas := elemento.GetProp(Hex(quem.serial));
		var esquemas := receitas[nomeReceitaClassificada];

		return esquemas;
		
	endif

	return error{"errortext" := "Nenhum registro Encontrado."};
endfunction

function RemoveEsquema(quem, nomeReceitaClassificada,esquema)
	var dFile := DFOpenDataFile(":crafting:esquemas", DF_CREATE);
	var elemento := DFFindElement(dFile, quem.client.accname, DF_CREATE);
	
	if(elemento)
		TrataRegistrosMortos(); // Limpa informação de personagens deletados.

		var receitas := elemento.GetProp(Hex(quem.serial));
		var esquemas := receitas[nomeReceitaClassificada];
		if(esquemas.exists(esquema.nome))
			esquemas.erase(esquema.nome);
			receitas[nomeReceitaClassificada] := esquemas;
			elemento.SetProp(Hex(quem.serial),receitas);
			return esquemas;
		endif
	endif
	return error{"errortext" := "Erro inesperado ao remover esquema."};
endfunction

function TrataRegistrosMortos()
	//TODO implementar.
endfunction