use uo;
use os;
use cfgfile;
use datafile;
use guilds;

include ":datafile:datafile";


/* 
 * FindGuildByName(name)
 *
 * Purpose: Encontra uma guilda pelo nome
 * Parameters: 
 *   name - Nome da guilda a procurar
 * Return:
 *   guildid ou 0 se não encontrar
 */
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