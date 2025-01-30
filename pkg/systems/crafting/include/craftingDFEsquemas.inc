use uo;
use os;

include ":datafile:datafile";

function AdicionaEsquema(quem, receitaObjtype, esquema)
	var dFile := DFOpenDataFile(":crafting:esquemas", DF_CREATE);
	var elemento := DFFindElement(dFile, quem.client.accname, DF_CREATE);
	receitaObjtype :=  Hex(receitaObjtype);

	if(elemento)
		TrataRegistrosMortos(); // Limpa informação de personagens deletados.

		var receitas := elemento.GetProp(Hex(quem.serial));
		var registroEsquemas := dictionary;
		var isNovaReceita := true;

		if(receitas)
			if(receitas.exists(receitaObjtype))
				registroEsquemas := receitas[receitaObjtype];
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
			receitas.insert(receitaObjtype,registroEsquemas);
		else
			receitas[receitaObjtype] := registroEsquemas;
		endif

		elemento.SetProp(Hex(quem.serial), receitas);
		return elemento;
	endif

	return error{"errortext" := "Erro inesperado."};
endfunction

function EditaEsquema(quem, receitaObjtype,esquema)
	var dFile := DFOpenDataFile(":crafting:esquemas", DF_CREATE);
	var elemento := DFFindElement(dFile, quem.client.accname, DF_CREATE);
	receitaObjtype :=  Hex(receitaObjtype);

	if(elemento)
		TrataRegistrosMortos(); // Limpa informação de personagens deletados.

		var receitas := elemento.GetProp(Hex(quem.serial));
		var registroEsquemas := receitas[receitaObjtype];
		registroEsquemas[esquema.nome] := esquema;

		receitas[receitaObjtype] := registroEsquemas;

		elemento.SetProp(Hex(quem.serial), receitas);
		return esquema;
	endif

	return error{"errortext" := "Erro inesperado."};
endfunction

function ListaEsquemas(quem, receitaObjtype)
	var dFile := DFOpenDataFile(":crafting:esquemas", DF_CREATE);
	var elemento := DFFindElement(dFile, quem.client.accname, DF_CREATE);
	
	if(elemento)
		TrataRegistrosMortos(); // Limpa informação de personagens deletados.

		var receitas := elemento.GetProp(Hex(quem.serial));
		var esquemas := receitas[Hex(receitaObjtype)];

		return esquemas;
		
	endif

	return error{"errortext" := "Nenhum registro Encontrado."};
endfunction

function RemoveEsquema(quem, receitaObjtype,esquema)
	var dFile := DFOpenDataFile(":crafting:esquemas", DF_CREATE);
	var elemento := DFFindElement(dFile, quem.client.accname, DF_CREATE);
	
	if(elemento)
		TrataRegistrosMortos(); // Limpa informação de personagens deletados.

		var receitas := elemento.GetProp(Hex(quem.serial));
		var esquemas := receitas[Hex(receitaObjtype)];
		if(esquemas.exists(esquema.nome))
			esquemas.erase(esquema.nome);
			receitas[Hex(receitaObjtype)] := esquemas;
			elemento.SetProp(Hex(quem.serial),receitas);
			return esquemas;
		endif
	endif
	return error{"errortext" := "Erro inesperado ao remover esquema."};
endfunction

function TrataRegistrosMortos()
	//TODO implementar.
endfunction