/*
 * stealables.inc
 * 
 * Funções de gerenciamento de artefatos roubáveis
 * Sistema inspirado no :unicos
 */

use uo;

include ":datafile:include/datafile";
include ":gumps:yesNo";

//---=[ Stealable Artifacts specific functions ]=-----------------------------

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
 * StealableElemName(spawner)
 * 
 * Retorna o nome do elemento a ser usado (serial do spawner)
 */
function StealableElemName(spawner)
	return Hex(spawner.serial);
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
 * AddStealableToList(gm, spawner, artifact_name, artifact_desc, objtype, color, rarity, x, y, z, realm)
 * 
 * Adiciona um artefato roubável à lista
 */
function AddStealableToList(gm, spawner, artifact_name, artifact_desc, objtype, color, rarity, x, y, z, realm)
	var elem_name := StealableElemName(spawner);
	var data_elem := GetStealableDataElem(elem_name);

	data_elem.SetProp("SpawnerSerial", spawner.serial);
	data_elem.SetProp("ArtifactName", artifact_name);
	data_elem.SetProp("ArtifactDesc", artifact_desc);
	data_elem.SetProp("ArtifactObjtype", objtype);
	data_elem.SetProp("ArtifactColor", color);
	data_elem.SetProp("Rarity", rarity);
	data_elem.SetProp("X", x);
	data_elem.SetProp("Y", y);
	data_elem.SetProp("Z", z);
	data_elem.SetProp("Realm", realm);
	data_elem.SetProp("Creator", gm.acctname);
	data_elem.SetProp("CreatorName", gm.name);
	data_elem.SetProp("TimeCreated", polcore().systime);
	data_elem.SetProp("TimesStolen", 0);
	
	// Marca o spawner
	SetObjProperty(spawner, "StealableArtifact", 1);

	return data_elem;
endfunction

/*
 * RemoveStealableFromList(spawner)
 * 
 * Remove um artefato roubável da lista
 */
function RemoveStealableFromList(spawner)
	var data_file := GetStealablesDataFile();
	EraseObjProperty(spawner, "StealableArtifact");
	
	return data_file.DeleteElement(StealableElemName(spawner));
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

/*
 * GetRarityNameByLevel(rarity)
 * 
 * Retorna o nome da raridade
 */
function GetRarityNameByLevel(rarity)
	case (CInt(rarity))
		1: return "R1 - Comum";
		2: return "R2 - Incomum";
		3: return "R3 - Raro";
		4: return "R4 - Muito Raro";
		5: return "R5 - Épico";
		6: return "R6 - Lendário";
		7: return "R7 - Mítico";
		8: return "R8 - Supremo";
		9: return "R9 - Divino";
		10: return "R10 - Celestial";
		11: return "R11 - Primordial";
		12: return "R12 - Transcendente";
		default: return "Desconhecido";
	endcase
endfunction

/*
 * GetRespawnTimeByRarity(rarity)
 * 
 * Retorna o tempo de respawn em segundos para a raridade especificada
 * 
 * Tabela de tempos:
 * R1: 90 minutos, R2: 3 horas, R3: 6 horas, R4: 12 horas, R5: 24 horas (1 dia)
 * R6: 48 horas (2 dias), R7: 96 horas (4 dias), R8: 192 horas (8 dias)
 * R9: 384 horas (16 dias), R10: 768 horas (32 dias), R11: 1536 horas (64 dias), R12: 3072 horas (128 dias)
 */
function GetRespawnTimeByRarity(rarity)
	rarity := CInt(rarity);
	case (rarity)
		1: return 5400;     // 90 minutos
		2: return 10800;    // 3 horas
		3: return 21600;    // 6 horas
		4: return 43200;    // 12 horas
		5: return 86400;    // 1 dia
		6: return 172800;   // 2 dias
		7: return 30;   // 4 dias
		8: return 691200;   // 8 dias
		9: return 1382400;  // 16 dias
		10: return 2764800; // 32 dias
		11: return 5529600; // 64 dias
		12: return 11059200; // 128 dias
		default: return 0;
	endcase
endfunction

/*
 * CalculateStealingDifficulty(rarity, weight)
 * 
 * Calcula a dificuldade de roubo baseada na raridade e peso do item
 * Fórmula: 40.0 + (Raridade × 5.0) + (Peso × 2.0)
 * 
 * Parâmetros:
 *   rarity - Nível de raridade (1-12)
 *   weight - Peso do item em stones
 * 
 * Retorno:
 *   Dificuldade calculada
 */
function CalculateStealingDifficulty(rarity, weight)
	var difficulty := 40.0; // BASE_DIFFICULTY
	difficulty := difficulty + (CDbl(rarity) * 5.0); // RARITY_MULTIPLIER
	difficulty := difficulty + (CDbl(weight) * 2.0); // WEIGHT_MULTIPLIER
	
	return difficulty;
endfunction

/*
 * GetRarityName(rarity)
 * 
 * Retorna o nome descritivo da raridade com tempo
 * 
 * Parâmetros:
 *   rarity - Nível de raridade (1-12)
 * 
 * Retorno:
 *   String descritiva da raridade
 */
function GetRarityName(rarity)
	case (CInt(rarity))
		1: return "Comum (90 min)";
		2: return "Incomum (3 horas)";
		3: return "Raro (6 horas)";
		4: return "Muito Raro (12 horas)";
		5: return "Épico (1 dia)";
		6: return "Lendário (2 dias)";
		7: return "Mítico (4 dias)";
		8: return "Supremo (8 dias)";
		9: return "Divino (16 dias)";
		10: return "Celestial (32 dias)";
		11: return "Primordial (64 dias)";
		12: return "Transcendente (128 dias)";
		default: return "Desconhecido";
	endcase
endfunction

/*
 * FormatTimeRemaining(seconds)
 * 
 * Formata o tempo restante em uma string legível
 * 
 * Parâmetros:
 *   seconds - Tempo em segundos
 * 
 * Retorno:
 *   String formatada (ex: "2 dias, 5 horas, 30 minutos")
 */
function FormatTimeRemaining(seconds)
	if (seconds <= 0)
		return "Disponível agora";
	endif
	
	var days := CInt(seconds / 86400);
	seconds := seconds % 86400;
	
	var hours := CInt(seconds / 3600);
	seconds := seconds % 3600;
	
	var minutes := CInt(seconds / 60);
	
	var result := "";
	
	if (days > 0)
		result := result + days + " dia";
		if (days > 1)
			result := result + "s";
		endif
	endif
	
	if (hours > 0)
		if (result != "")
			result := result + ", ";
		endif
		result := result + hours + " hora";
		if (hours > 1)
			result := result + "s";
		endif
	endif
	
	if (minutes > 0)
		if (result != "")
			result := result + ", ";
		endif
		result := result + minutes + " minuto";
		if (minutes > 1)
			result := result + "s";
		endif
	endif
	
	if (result == "")
		result := "Menos de 1 minuto";
	endif
	
	return result;
endfunction

/*
 * IsStealableArtifact(item)
 * 
 * Verifica se um item é um artefato roubável
 * 
 * Parâmetros:
 *   item - Item a ser verificado
 * 
 * Retorno:
 *   1 se for artefato roubável, 0 caso contrário
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
 * 
 * Parâmetros:
 *   item - Artefato roubável
 * 
 * Retorno:
 *   Objeto spawner ou error
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
