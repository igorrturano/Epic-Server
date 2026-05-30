/*
 * stealables.inc
 * 
 * Funções de gerenciamento de artefatos roubáveis
 * Sistema inspirado no :unicos
 * 

 */

use uo;

include ":datafile:include/datafile";

//---=[ Constantes ]=----------------------------------------------------------

// Constantes de dificuldade
const BASE_DIFFICULTY := 40.0;
const RARITY_MULTIPLIER := 5.0;
const WEIGHT_MULTIPLIER := 2.0;

//---=[ Tabela de Raridades ]=-----------------------------------------------

/*
 * BuildRarityTable()
 * 
 * Constrói e retorna a tabela de raridades
 * 
 * NOTA: POL Script não suporta const com structs complexos,
 * então usamos uma função que retorna o array
 */
function BuildRarityTable()
	var table := array{
		struct{level := 1,  name := "Comum",         time := 5400,    color := 2213, hex := "#00FF00"},
		struct{level := 2,  name := "Incomum",       time := 10800,   color := 2213, hex := "#00FF00"},
		struct{level := 3,  name := "Raro",          time := 21600,   color := 2125, hex := "#0080FF"},
		struct{level := 4,  name := "Muito Raro",    time := 43200,   color := 2125, hex := "#0080FF"},
		struct{level := 5,  name := "Épico",         time := 86400,   color := 1161, hex := "#A000FF"},
		struct{level := 6,  name := "Lendário",      time := 172800,  color := 1161, hex := "#A000FF"},
		struct{level := 7,  name := "Mítico",        time := 345600,  color := 1153, hex := "#FFD700"},
		struct{level := 8,  name := "Supremo",       time := 691200,  color := 1153, hex := "#FFD700"},
		struct{level := 9,  name := "Divino",        time := 1382400, color := 53,   hex := "#FF8C00"},
		struct{level := 10, name := "Celestial",     time := 2764800, color := 53,   hex := "#FF8C00"},
		struct{level := 11, name := "Primordial",    time := 5529600, color := 33,   hex := "#FF0000"},
		struct{level := 12, name := "Transcendente", time := 11059200, color := 33,   hex := "#FF0000"}
	};
	
	return table;
endfunction

//---=[ Funções de DataFile ]=------------------------------------------------

/*
 * GetStealablesDataFile()
 * 
 * Retorna o DataFile de artefatos roubáveis
 */
function GetStealablesDataFile()
	var data_file := DFOpenDataFile(":stealables:lista", DF_CREATE);
	return data_file;
endfunction

/*
 * GetStealableDataElem(elem_name)
 * 
 * Retorna o elemento do DataFile
 */
function GetStealableDataElem(elem_name)
	var data_file := GetStealablesDataFile();
	var data_elem := DFFindElement(data_file, elem_name, DF_CREATE);
	
	return data_elem;
endfunction

/*
 * StealableElemName(spawner_or_serial)
 * 
 * Retorna o nome do elemento (serial em hex)
 * Aceita tanto objeto quanto serial
 */
function StealableElemName(spawner_or_serial)
	if (TypeOf(spawner_or_serial) == "Integer")
		return Hex(spawner_or_serial);
	else
		return Hex(spawner_or_serial.serial);
	endif
endfunction

/*
 * GetStealablesList()
 * 
 * Retorna lista de todos os artefatos roubáveis
 */
function GetStealablesList()
	var data_file := GetStealablesDataFile();
	var data_list := DFGetElemNames(data_file);

	// Reversed so newer items show up at the top
	data_list.Reverse();

	return data_list;
endfunction

/*
 * GetStealableInfo(spawner)
 * 
 * Retorna as propriedades de um artefato roubável
 */
function GetStealableInfo(spawner)
	var data_elem := GetStealableDataElem(StealableElemName(spawner));
	
	return DFGetElemProps(data_elem);
endfunction

/*
 * AddStealableToList(gm, spawner, artifact_info)
 * 
 * Adiciona um artefato roubável à lista
 * 
 * Parâmetros:
 *   gm - GM que criou o artefato
 *   spawner - Objeto spawner
 *   artifact_info - Struct com todas as informações do artefato
 */
function AddStealableToList(gm, spawner, artifact_info)
	var elem_name := StealableElemName(spawner);
	var data_elem := GetStealableDataElem(elem_name);

	// Salva as informações no datafile
	data_elem.SetProp("SpawnerSerial", spawner.serial);
	data_elem.SetProp("ArtifactName", artifact_info.name);
	data_elem.SetProp("ArtifactDesc", artifact_info.desc);
	data_elem.SetProp("ArtifactObjtype", artifact_info.objtype);
	data_elem.SetProp("ArtifactColor", artifact_info.color);
	data_elem.SetProp("Rarity", artifact_info.rarity);
	data_elem.SetProp("X", artifact_info.x);
	data_elem.SetProp("Y", artifact_info.y);
	data_elem.SetProp("Z", artifact_info.z);
	data_elem.SetProp("Realm", artifact_info.realm);
	data_elem.SetProp("Creator", artifact_info.creator);
	data_elem.SetProp("CreatorName", artifact_info.creator_name);
	data_elem.SetProp("TimeCreated", artifact_info.time_created);
	data_elem.SetProp("TimesStolen", artifact_info.times_stolen);
	data_elem.SetProp("LastStolen", artifact_info.last_stolen);
	
	// Salva informações de armadilha se existirem
	if (artifact_info.has_trap)
		data_elem.SetProp("HasTrap", artifact_info.has_trap);
		data_elem.SetProp("TrapType", artifact_info.trap_type);
		data_elem.SetProp("TrapLevel", artifact_info.trap_level);
	endif
	
	// Marca o spawner
	SetObjProperty(spawner, "StealableArtifact", 1);

	return data_elem;
endfunction

/*
 * RemoveStealableFromList(spawner_or_serial)
 * 
 * Remove um artefato roubável da lista
 * Aceita tanto objeto spawner quanto serial
 * 
 * BUG FIX: Agora aceita ambos os tipos
 */
function RemoveStealableFromList(spawner_or_serial)
	var data_file := GetStealablesDataFile();
	var elem_name := StealableElemName(spawner_or_serial);
	
	// Se passou objeto, remove a propriedade
	if (TypeOf(spawner_or_serial) != "Integer")
		EraseObjProperty(spawner_or_serial, "StealableArtifact");
	else
		// Se passou serial, tenta encontrar o objeto
		var spawner := SystemFindObjectBySerial(spawner_or_serial);
		if (spawner)
			EraseObjProperty(spawner, "StealableArtifact");
		endif
	endif
	
	return data_file.DeleteElement(elem_name);
endfunction

/*
 * IncrementStealCount(spawner)
 * 
 * Incrementa o contador de vezes que foi roubado
 */
function IncrementStealCount(spawner)
	var data_elem := GetStealableDataElem(StealableElemName(spawner));
	var current := CInt(data_elem.GetProp("TimesStolen"));
	data_elem.SetProp("TimesStolen", current + 1);
	data_elem.SetProp("LastStolen", polcore().systime);
	
	return 1;
endfunction

//---=[ Funções de Raridade (DRY - Centralizadas) ]=--------------------------

/*
 * GetRarityData(rarity)
 * 
 * Retorna struct com todas as informações da raridade
 * 
 * REFATORADO: Função única que substitui 3 funções antigas
 */
function GetRarityData(rarity)
	rarity := CInt(rarity);
	
	if (rarity < 1 || rarity > 12)
		return struct{
			level := 0,
			name := "Desconhecido",
			time := 0,
			color := 1153,
			hex := "#FFD700"
		};
	endif
	
	return BuildRarityTable()[rarity];
endfunction

/*
 * GetRarityName(rarity, include_time)
 * 
 * Retorna o nome da raridade
 * 
 * Parâmetros:
 *   rarity - Nível (1-12)
 *   include_time - Se true, inclui tempo de respawn (opcional)
 * 
 * Retorno:
 *   "R5 - Épico" ou "Épico (1 dia)"
 */
function GetRarityName(rarity, include_time := 0)
	var data := GetRarityData(rarity);
	
	if (include_time)
		var time_str := FormatRespawnTime(data.time);
		return data.name + " (" + time_str + ")";
	endif
	
	return "R" + CStr(data.level) + " - " + data.name;
endfunction

/*
 * GetRarityColor(rarity)
 * 
 * Retorna a cor da raridade para gumps
 */
function GetRarityColor(rarity)
	var data := GetRarityData(rarity);
	return data.color;
endfunction

/*
 * GetRespawnTime(rarity)
 * 
 * Retorna o tempo de respawn em segundos
 */
function GetRespawnTime(rarity)
	var data := GetRarityData(rarity);
	return data.time;
endfunction

/*
 * FormatRespawnTime(seconds)
 * 
 * Formata tempo de respawn em string legível
 * 
 * Exemplos: "90 minutos", "3 horas", "2 dias"
 */
function FormatRespawnTime(seconds)
	var days := CInt(seconds / 86400);
	if (days > 0)
		return CStr(days) + (days == 1 ? " dia" : " dias");
	endif
	
	var hours := CInt(seconds / 3600);
	if (hours > 0)
		return CStr(hours) + (hours == 1 ? " hora" : " horas");
	endif
	
	var minutes := CInt(seconds / 60);
	return CStr(minutes) + (minutes == 1 ? " minuto" : " minutos");
endfunction

//---=[ Funções de Cálculo ]=------------------------------------------------

/*
 * CalculateStealingDifficulty(rarity, weight)
 * 
 * Calcula a dificuldade de roubo baseada na raridade e peso do item
 * Fórmula: 40.0 + (Raridade × 5.0) + (Peso × 2.0)
 */
function CalculateStealingDifficulty(rarity, weight)
	var difficulty := BASE_DIFFICULTY;
	difficulty += (CDbl(rarity) * RARITY_MULTIPLIER);
	difficulty += (CDbl(weight) * WEIGHT_MULTIPLIER);
	
	return difficulty;
endfunction

//---=[ Funções de Artefato ]=-----------------------------------------------

/*
 * IsStealableArtifact(item)
 * 
 * Verifica se um item é um artefato roubável
 */
function IsStealableArtifact(item)
	if (!item)
		return 0;
	endif
	
	return CInt(GetObjProperty(item, "PropStealable"));
endfunction

/*
 * GetArtifactSpawner(item)
 * 
 * Retorna o spawner vinculado ao artefato
 */
function GetArtifactSpawner(item)
	if (!item)
		return error{"errortext" := "Item inválido"};
	endif
	
	var spawner_serial := GetObjProperty(item, "StealableSpawnerSerial");
	if (!spawner_serial)
		return error{"errortext" := "Spawner não encontrado"};
	endif
	
	var spawner := SystemFindObjectBySerial(spawner_serial);
	if (!spawner)
		return error{"errortext" := "Spawner não existe mais"};
	endif
	
	return spawner;
endfunction

/*
 * GetArtifactStatus(spawner)
 * 
 * Retorna o status atual do artefato
 * 
 * NOVO: Função extraída para evitar duplicação (DRY)
 * 
 * Retorno:
 *   struct{text := "Status", color := cor_do_status}
 */
function GetArtifactStatus(spawner)
	var status := struct;
	status.+text := "Desconhecido";
	status.+color := 2100;
	
	var current_artifact_serial := GetObjProperty(spawner, "CurrentArtifactSerial");
	var next_spawn_time := CInt(GetObjProperty(spawner, "NextSpawnTime"));
	
	if (current_artifact_serial)
		var artifact := SystemFindObjectBySerial(current_artifact_serial);
		if (artifact && !GetObjProperty(artifact, "PropStealable"))
			status.text := "Roubado";
			status.color := 53; // Laranja
		elseif (artifact)
			status.text := "Disponível";
			status.color := 68; // Verde
		else
			status.text := "Respawn";
			status.color := 33; // Vermelho
		endif
	elseif (next_spawn_time && ReadGameClock() < next_spawn_time)
		var time_left := next_spawn_time - ReadGameClock();
		var hours_left := CInt(time_left / 3600);
		if (hours_left > 0)
			status.text := CStr(hours_left) + "h";
		else
			status.text := CStr(CInt(time_left / 60)) + "m";
		endif
		status.color := 2100; // Cinza
	else
		status.text := "Erro";
		status.color := 33; // Vermelho
	endif
	
	return status;
endfunction
