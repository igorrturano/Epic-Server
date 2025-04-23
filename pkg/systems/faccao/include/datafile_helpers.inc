// :faccao:include/datafile_helpers.inc
//
// Funções auxiliares para acessar datafiles de forma consistente e segura
// seguindo o princípio KISS (Keep It Simple, Stupid)

use uo;
use os;
use datafile;

include ":datafile:datafile";
include ":faccao:include/logging";

/*
 * SafeOpenDataFile(datafile_name, create := 0)
 * 
 * Abre um datafile com tratamento de erro padronizado
 * Se create=1, cria o datafile se não existir
 * 
 * Retorna o datafile ou 0 em caso de erro
 */
function SafeOpenDataFile(datafile_name, create := 0)
    LogDebug("datafile", "SafeOpenDataFile: Tentando abrir " + datafile_name + (create ? " (criando se não existir)" : ""));
    
    var df;
    
    if (create)
        df := DFOpenDataFile(datafile_name, DF_CREATE);
        if (df == error)
            LogError("datafile", "Falha ao criar datafile: " + datafile_name + ", Erro: " + df);
            return 0;
        endif
        LogDebug("datafile", "Datafile criado com sucesso: " + datafile_name);
    else
        df := DFOpenDataFile(datafile_name);
        if (df == error)
            LogError("datafile", "Falha ao abrir datafile: " + datafile_name + ", Erro: " + df);
            return 0;
        endif
        LogDebug("datafile", "Datafile aberto com sucesso: " + datafile_name);
    endif
    
    return df;
endfunction

/*
 * SafeFindElement(datafile, elem_name, create := 0)
 * 
 * Encontra um elemento no datafile com tratamento de erro padronizado
 * Se create=1, cria o elemento se não existir
 * 
 * Retorna o elemento ou 0 em caso de erro
 */
function SafeFindElement(datafile, elem_name, create := 0)
    if (!datafile)
        LogError("datafile", "Datafile inválido ao buscar elemento: " + elem_name);
        return 0;
    endif
    
    var elem;
    
    if (create)
        elem := DFFindElement(datafile, elem_name, DF_CREATE);
        if (elem == error)
            LogError("datafile", "Falha ao criar elemento: " + elem_name);
            return 0;
        endif
    else
        elem := DFFindElement(datafile, elem_name);
        if (elem == error)
            LogDebug("datafile", "Elemento não encontrado: " + elem_name);
            return 0;
        endif
    endif
    
    return elem;
endfunction

/*
 * GetElementProp(elem, prop_name, default_value := 0)
 * 
 * Obtém uma propriedade de um elemento com segurança
 * Retorna default_value se a propriedade não existir
 */
function GetElementProp(elem, prop_name, default_value := 0)
    if (!elem)
        return default_value;
    endif
    
    var prop := elem.GetProp(prop_name);
    if (prop == error || prop == 0)
        return default_value;
    endif
    
    return prop;
endfunction

/*
 * SetElementProp(elem, prop_name, value)
 * 
 * Define uma propriedade em um elemento com tratamento de erro
 * Retorna 1 para sucesso, 0 para falha
 */
function SetElementProp(elem, prop_name, value)
    if (!elem)
        LogError("datafile", "Elemento inválido ao definir propriedade: " + prop_name);
        return 0;
    endif
    
    LogDebug("datafile", "SetElementProp: Definindo " + prop_name + " com valor do tipo " + TypeOf(value));
    if (TypeOf(value) == "Dictionary" || TypeOf(value) == "Array" || TypeOf(value) == "Struct")
        LogDebug("datafile", "SetElementProp: Objeto complexo, Size: " + (TypeOf(value) == "Dictionary" || TypeOf(value) == "Array" ? value.size() : "N/A"));
    endif
    
    var result := elem.SetProp(prop_name, value);

    if (result == error)
        LogError("datafile", "Falha ao definir propriedade: " + prop_name + ", Erro: " + result);
        return 0;
    endif
    
    LogDebug("datafile", "SetElementProp: Propriedade " + prop_name + " definida com sucesso");
    return 1;
endfunction

/*
 * GetDatafileEntries(datafile_name, entry_type := "")
 * 
 * Obtém todas as entradas de um determinado tipo em um datafile
 * Se entry_type for fornecido, filtra por tipo
 * 
 * Retorna um array de structs com {name, props}
 */
function GetDatafileEntries(datafile_name, entry_type := "")
    var df := SafeOpenDataFile(datafile_name);
    if (!df)
        return array{};
    endif
    
    var entries := array{};
    var keys := df.keys();
    
    foreach elem_name in keys
        var elem := SafeFindElement(df, elem_name);
        if (!elem)
            continue;
        endif
        
        // Filtrar por tipo se especificado
        if (entry_type != "")
            var type := GetElementProp(elem, "type", "");
            if (type != entry_type)
                continue;
            endif
        endif
        
        // Coletar todas as propriedades do elemento
        var props := dictionary{};
        var prop_names := elem.PropNames();
        
        foreach prop_name in prop_names
            props[prop_name] := elem.GetProp(prop_name);
            sleepms(1);
        endforeach
        
        entries.append(struct{
            "name" := elem_name,
            "props" := props
        });
        
        sleepms(5);
    endforeach
    
    return entries;
endfunction

/*
 * DeleteDatafileEntry(datafile_name, elem_name)
 * 
 * Remove um elemento do datafile com tratamento de erro e lógica aprimorada
 * para lidar tanto com nomes quanto com IDs de território
 */
function DeleteDatafileEntry(datafile_name, elem_name)
    var df := SafeOpenDataFile(datafile_name);
    if (!df)
        LogError("datafile", "Datafile não encontrado: " + datafile_name);
        return 0;
    endif
    
    // Tentar deletar diretamente pelo nome
    var result := df.DeleteElement(elem_name);
    if (result && result != error)
        return 1;
    endif
    
    // Se falhou e parece ser um ID de território (começando com TER_)
    if (elem_name.find("TER_") == 1)
        foreach key in (df.Keys())
            var elem := df.FindElement(key);
            if (elem)
                var id := elem.GetProp(TERRITORY_ID_PROP);
                if (id == elem_name)
                    result := df.DeleteElement(key);
                    if (result && result != error)
                        return 1;
                    endif
                endif
            endif
            sleepms(2);
        endforeach
    endif
    
    LogError("datafile", "Falha ao deletar elemento: " + elem_name);
    return 0;
endfunction

/*
 * GetStructProp(struct_obj, prop_name, default_value := 0)
 * 
 * Obtém uma propriedade de um struct com segurança
 * Retorna default_value se a propriedade não existir
 */
function GetStructProp(struct_obj, prop_name, default_value := 0)
    if (!struct_obj || typeof(struct_obj) != "Struct")
        return default_value;
    endif
    
    if (!struct_obj.exists(prop_name))
        return default_value;
    endif
    
    return struct_obj[prop_name];
endfunction

/*
 * FindDatafileByPath(path_list)
 * 
 * Tenta encontrar um datafile em vários caminhos
 * path_list deve ser um array de strings com caminhos possíveis
 * 
 * Retorna o primeiro datafile encontrado ou 0
 */
function FindDatafileByPath(path_list)
    foreach path in path_list
        var df := DFOpenDataFile(path);
        if (df != error)
            return df;
        endif
        sleepms(2);
    endforeach
    
    return 0;
endfunction


// Função para verificar a integridade do datafile - corrigida
function VerificarIntegridadeDatafile()
    LogInfo("recursos", "Verificando integridade do datafile de recursos...");
    
    var df := SafeOpenDataFile(KINGDOM_RESOURCE_DATAFILE, 1);
    if (!df)
        return 0;
    endif
    
    // Correção: Limpar elementos duplicados primeiro
    LimparElementosDuplicados(df);
    
    // Verificar elementos do datafile
    var correcoes := 0;
    
    // 1. Verificar elemento do Reino
    var reino_elem := SafeFindElement(df, KINGDOM_RESOURCE_ELEM, 1);
    var recursos_reino := GetElementProp(reino_elem, "recursos");
    
    if (!recursos_reino)
        recursos_reino := CreateEmptyResourceStruct();
        SetElementProp(reino_elem, "recursos", recursos_reino);
        correcoes += 1;
        LogInfo("recursos", "Criado struct de recursos do Reino");
    else
        // Validar e corrigir
        var recursos_validados := ValidarRecursos(recursos_reino);
        
        // Se houve alterações
        if (!SaoRecursosIguais(recursos_reino, recursos_validados))
            SetElementProp(reino_elem, "recursos", recursos_validados);
            correcoes += 1;
            LogInfo("recursos", "Corrigidos recursos do Reino");
        endif
    endif
    
    // 2. Verificar elementos de facções
    foreach elem_name in (df.keys())
        if (elem_name != KINGDOM_RESOURCE_ELEM && elem_name.find("faction_") == 1)
            var faction_elem := SafeFindElement(df, elem_name);
            var recursos := GetElementProp(faction_elem, "recursos");
            
            if (!recursos)
                recursos := CreateEmptyResourceStruct();
                SetElementProp(faction_elem, "recursos", recursos);
                correcoes += 1;
                LogInfo("recursos", "Criado struct de recursos para: " + elem_name);
            else
                // Validar e corrigir
                var recursos_validados := ValidarRecursos(recursos);
                
                // Se houve alterações
                if (!SaoRecursosIguais(recursos, recursos_validados))
                    SetElementProp(faction_elem, "recursos", recursos_validados);
                    correcoes += 1;
                    LogInfo("recursos", "Corrigidos recursos para: " + elem_name);
                endif
            endif
        endif
        Sleep(1);
    endforeach
    
    LogInfo("recursos", "Verificação de integridade do datafile concluída. Correções: " + correcoes);
    return correcoes;
endfunction



// Função para limpar elementos duplicados - nova função simples
function LimparElementosDuplicados(df)
    var elementos_removidos := 0;
    var elementos_a_remover := array{};
    var dados_a_migrar := dictionary{};
    
    // Identificar elementos incorretos e planejar migrações
    foreach elem_name in (df.keys())
        if (elem_name.find("faction_faction_") == 1)
            // Extrair o nome real da facção
            var nome_base := "";
            var partes := SplitWords(elem_name, "_");
            
            // Pular os prefixos "faction_" e pegar o resto
            for i := 3 to partes.size()
                if (i > 3)
                    nome_base += "_";
                endif
                nome_base += partes[i];
            endfor
            
            // Nome correto para migração
            var nome_correto := "faction_" + nome_base;
            
            // Marcar para remoção
            elementos_a_remover.append(elem_name);
            
            // Guardar dados para migração
            var elem := SafeFindElement(df, elem_name);
            if (elem)
                var recursos := elem.GetProp("recursos");
                if (recursos)
                    dados_a_migrar[nome_correto] := recursos;
                endif
            endif
        endif
        sleepms(2);
    endforeach
    
    // Migrar dados para elementos corretos
    foreach nome_correto in (dados_a_migrar.keys())
        var elem := SafeFindElement(df, nome_correto, 1);
        var recursos_existentes := elem.GetProp("recursos");
        
        // Só migrar se não tiver recursos ou forem diferentes
        if (!recursos_existentes || !SaoRecursosIguais(recursos_existentes, dados_a_migrar[nome_correto]))
            SetElementProp(elem, "recursos", dados_a_migrar[nome_correto]);
            LogInfo("recursos", "Dados migrados para: " + nome_correto);
        endif
        sleepms(2);
    endforeach
    
    // Remover elementos incorretos
    foreach elem_name in elementos_a_remover
        df.DeleteElement(elem_name);
        elementos_removidos += 1;
        LogInfo("recursos", "Elemento incorreto removido: " + elem_name);
        sleepms(2);
    endforeach
    
    if (elementos_removidos)
        LogInfo("recursos", "Total de elementos incorretos removidos: " + elementos_removidos);
    endif
    
    return elementos_removidos;
endfunction