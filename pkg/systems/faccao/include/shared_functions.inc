use uo;
use os;
use cfgfile;
use datafile;
use guilds;

include ":datafile:datafile";


// Funções compartilhadas entre o sistema de baú e o sistema de gestão

/* 
 * ContarItensDoadosNoBau(bau)
 * 
 * Purpose: Conta todos os itens doados no baú, classificando-os por tipo
 * Parameters: 
 *   bau - Referência ao container do baú
 * Return:
 *   Struct com contagens de cada tipo de recurso
 */
function ContarItensDoadosNoBau(bau)
    var recursos := struct{
        comida := 0,
        moedas := 0,
        couro := 0,
        metal := 0,
        madeira := 0,
        pano := 0,
        pedras := 0,
        joias := 0
    };
    
    foreach item in EnumerateItemsInContainer(bau)
        if (GetObjProperty(item, "IsKingdomDonationComida"))
            recursos.comida += item.amount;
        elseif (GetObjProperty(item, "IsKingdomDonationMoedas"))
            recursos.moedas += item.amount;
        elseif (GetObjProperty(item, "IsKingdomDonationCouro"))
            recursos.couro += item.amount;
        elseif (GetObjProperty(item, "IsKingdomDonationMetal"))
            recursos.metal += item.amount;
        elseif (GetObjProperty(item, "IsKingdomDonationMadeira"))
            recursos.madeira += item.amount;
        elseif (GetObjProperty(item, "IsKingdomDonationPano"))
            recursos.pano += item.amount;
        elseif (GetObjProperty(item, "IsKingdomDonationPedra"))
            recursos.pedras += item.amount;
        elseif (GetObjProperty(item, "IsKingdomDonationJoias"))
            recursos.joias += item.amount;
        endif
    endforeach

    return recursos;
endfunction

/* 
 * AtualizarRecursosNoDataFile(recursos)
 * 
 * Purpose: Atualiza os valores de recursos no datafile do reino
 * Parameters: 
 *   recursos - Struct com contagens de recursos
 * Return:
 *   1 para sucesso, 0 para falha
 */
function AtualizarRecursosNoDataFile(recursos)
    var data_file := DFOpenDataFile("kingdom_resources", DF_CREATE);
    var elem := DFFindElement(data_file, "resources", DF_CREATE);
    
    if (!elem)
        return 0;
    endif
    
    // Atualizar cada recurso no datafile
    elem.SetProp("comida", recursos.comida);
    elem.SetProp("moedas", recursos.moedas);
    elem.SetProp("couro", recursos.couro);
    elem.SetProp("metal", recursos.metal);
    elem.SetProp("madeira", recursos.madeira);
    elem.SetProp("pano", recursos.pano);
    elem.SetProp("pedras", recursos.pedras);
    elem.SetProp("joias", recursos.joias);
    
    return 1;
endfunction

/* 
 * ObterRecursosDoReino()
 * 
 * Purpose: Obtém os valores atuais dos recursos do datafile
 * Return:
 *   Struct com os valores dos recursos
 */
function ObterRecursosDoReino()
    var data_file := DFOpenDataFile("kingdom_resources", DF_CREATE);
    var elem := DFFindElement(data_file, "resources", DF_CREATE);
    
    var recursos := struct{
        comida := elem.GetProp("comida") ? elem.GetProp("comida") : 0,
        moedas := elem.GetProp("moedas") ? elem.GetProp("moedas") : 0,
        couro := elem.GetProp("couro") ? elem.GetProp("couro") : 0,
        metal := elem.GetProp("metal") ? elem.GetProp("metal") : 0,
        madeira := elem.GetProp("madeira") ? elem.GetProp("madeira") : 0,
        pano := elem.GetProp("pano") ? elem.GetProp("pano") : 0,
        pedras := elem.GetProp("pedras") ? elem.GetProp("pedras") : 0,
        joias := elem.GetProp("joias") ? elem.GetProp("joias") : 0
    };
    
    return recursos;
endfunction

/* 
 * GetFactionOfChest(item)
 * 
 * Purpose: Determina a qual facção o baú pertence
 * Parameters: 
 *   item - Referência ao baú
 * Return:
 *   Nome da facção ou "Reino" se não tiver facção específica
 */
function GetFactionOfChest(item)
    // Primeiro tenta obter a facção diretamente do baú
    var faction := GetObjProperty(item, "faction");
    if (faction)
        return faction;
    endif
    
    // Se não encontrar, busca pela facção que tem este baú registrado
    var guilds := ListGuilds();
    foreach guild in guilds
        var chest_serial := guild.GetProp("donation_chest");
        if (chest_serial && chest_serial == item.serial)
            return guild.GetProp("name");
        endif
        sleepms(2);
    endforeach
    
    return "Reino"; // Valor padrão caso não encontre
endfunction

function FindGuildByName(name)
    // Primeiro tenta usar ListGuilds (mais completo)
    var guilds := ListGuilds();
    if (guilds)
        foreach guild in guilds
            if (guild.GetProp("name") == name)
                return guild.guildid;
            endif
            sleepms(2);
        endforeach
    endif
    
    // Backup: procura por jogadores online se ListGuilds falhar
    foreach player in (EnumerateOnlineCharacters())
        if (player.guildid && player.guild.GetProp("name") == name)
            return player.guild.guildid;
        endif
        sleepms(2);
    endforeach
    
    return 0;
endfunction


function ForEachInArray(array_to_iterate, callback)
    if (!array_to_iterate || array_to_iterate.size() < 1)
        return;
    endif
    
    // POL usa índices baseados em 1, não 0
    for i := 1 to array_to_iterate.size()
        callback.call(array_to_iterate[i], i);
        sleepms(2);
    endfor
endfunction

/*
 * MySplitWords(text, delimiter := " ")
 *
 * Purpose: Versão mais robusta de SplitWords que lida melhor com delimitadores personalizados
 * Parameters:
 *   text - O texto a ser dividido
 *   delimiter - O delimitador a ser usado (padrão: espaço)
 * Return: Array de substrings
 */
function MySplitWords(text, delimiter := " ")
    if (!text || text == "")
        return array{};
    endif
    
    var result := array{};
    var current_pos := 1;
    var delim_pos := Find(text, delimiter, current_pos);
    
    while (delim_pos)
        var part := text[current_pos, delim_pos - current_pos];
        part := Trim(part);
        if (part != "")
            result.append(part);
        endif
        
        current_pos := delim_pos + len(delimiter);
        delim_pos := Find(text, delimiter, current_pos);
        
        sleepms(2);
    endwhile
    
    // Processar a última parte
    if (current_pos <= len(text))
        var last_part := text[current_pos, len(text) - current_pos + 1];
        last_part := Trim(last_part);
        if (last_part != "")
            result.append(last_part);
        endif
    endif
    
    return result;
endfunction