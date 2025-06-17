// :faccao:include/guild_xp.inc
//
// Sistema de XP para Guildas/Facções
// Gerencia o progresso coletivo da guild baseado no XP dos membros

use uo;
use os;
use guilds;
use datafile;

include ":faccao:faccao_constants";
include ":faccao:include/logging";
include ":faccao:include/safe_guild_utils";
include ":faccao:include/datafile_helpers";

// =======================================
// Constantes do Sistema de XP de Guild
// =======================================

// XP necessário para cada nível de guild (genérico)
const GUILD_LEVEL_1_XP := 50000;    // 50k XP
const GUILD_LEVEL_2_XP := 150000;   // 150k XP total (100k para subir de 1→2)
const GUILD_LEVEL_3_XP := 350000;   // 350k XP total (200k para subir de 2→3)
const GUILD_LEVEL_4_XP := 750000;   // 750k XP total (400k para subir de 3→4)
const GUILD_LEVEL_5_XP := 1500000;  // 1.5M XP total (750k para subir de 4→5)

// Percentual do XP do jogador que vai para a guild
const GUILD_XP_PERCENTAGE := 1.0;   // 100% do XP do jogador vai para a guild

// =======================================
// Funções para Níveis de Reino por Cultura
// =======================================

/*
 * GetKingdomLevels(guild_culture)
 * 
 * Purpose: Retorna os níveis específicos da cultura da guild
 * Parameters:
 *   guild_culture - Nome da cultura da guild
 * Return:
 *   Array com informações dos níveis
 */
function GetKingdomLevels(guild_culture)
    // DEBUG: Log da cultura recebida
    LogDebug("kingdom_levels", "GetKingdomLevels: Cultura recebida = '" + guild_culture + "'");
    var lower_culture := Lower(guild_culture);
    LogDebug("kingdom_levels", "GetKingdomLevels: Cultura em lowercase = '" + lower_culture + "'");
    
    case (lower_culture)        "aigla":
        "aiglanos":
        "aiglana":
        "saiglana":
            LogDebug("kingdom_levels", "GetKingdomLevels: Usando Aigla levels");
            return GetAiglaLevels();
        "leste":
        "povos_do_leste":
        "sleste":
            LogDebug("kingdom_levels", "GetKingdomLevels: Usando Leste levels");
            return GetLesteLevels();
        "haroloth":
        "drow":
        "sharoloth":
            LogDebug("kingdom_levels", "GetKingdomLevels: Usando Haroloth levels");
            return GetHarolothLevels();
        "bjorske":
        "bjornske":
        "sbjorske":
            LogDebug("kingdom_levels", "GetKingdomLevels: Usando Bjorske levels");
            return GetBjorskeLevels();
        "aluriel":
        "elfo":
        "saluriel":
            LogDebug("kingdom_levels", "GetKingdomLevels: Usando Aluriel levels");
            return GetAlurielLevels();
        "polkinea":
        "polski":
        "spolkinea":
            LogDebug("kingdom_levels", "GetKingdomLevels: Usando Polkinea levels");
            return GetPolkineaLevels();
        "baduran":
        "anao":
        "sbaduran":
            LogDebug("kingdom_levels", "GetKingdomLevels: Usando Baduran levels");
            return GetBaduranLevels();"gulthrak":
        "sgulthrak":
        "orc":
            LogDebug("kingdom_levels", "GetKingdomLevels: Usando Gulthrak levels");
            return GetGulthrakLevels();
        default:
            LogDebug("kingdom_levels", "GetKingdomLevels: FALLBACK - Usando Generic levels para cultura '" + guild_culture + "'");
            return GetGenericLevels();
    endcase
endfunction

// 🦅 Aiglanos - Tema: Ascensão Imperial e Águias
function GetAiglaLevels()
    return array{
        struct{ name := "Acampamento Legionário", xp := 50000, bonus := 5 },
        struct{ name := "Castrum Consolidado", xp := 150000, bonus := 10 },
        struct{ name := "Civitas Próspera", xp := 350000, bonus := 15 },
        struct{ name := "Império Aquilino", xp := 750000, bonus := 25 },
        struct{ name := "Hegemonia Augusta", xp := 1500000, bonus := 40 }
    };
endfunction

// 🐉 Povos do Leste - Tema: Dinastia e Dragão
function GetLesteLevels()
    return array{
        struct{ name := "Aldeia Unificada", xp := 50000, bonus := 5 },
        struct{ name := "Província Disciplinada", xp := 150000, bonus := 10 },
        struct{ name := "Domínio do Dragão", xp := 350000, bonus := 15 },
        struct{ name := "Império Celestial", xp := 750000, bonus := 25 },
        struct{ name := "Dinastia Imortal", xp := 1500000, bonus := 40 }
    };
endfunction

// 🕷️ Haroloth (Drows) - Tema: Casas Nobres e Subterrâneos
function GetHarolothLevels()
    return array{
        struct{ name := "Covil Nascente", xp := 50000, bonus := 5 },
        struct{ name := "Casa Menor", xp := 150000, bonus := 10 },
        struct{ name := "Casa Nobre", xp := 350000, bonus := 15 },
        struct{ name := "Grande Casa", xp := 750000, bonus := 25 },
        struct{ name := "Casa Suprema", xp := 1500000, bonus := 40 }
    };
endfunction

// ⚔️ Björske - Tema: Clãs e Tradições Nórdicas
function GetBjorskeLevels()
    return array{
        struct{ name := "Clã Unido", xp := 50000, bonus := 5 },
        struct{ name := "Jarl Respeitado", xp := 150000, bonus := 10 },
        struct{ name := "Reino dos Lobos", xp := 350000, bonus := 15 },
        struct{ name := "Grande Jarl", xp := 750000, bonus := 25 },
        struct{ name := "Lenda Imortal", xp := 1500000, bonus := 40 }
    };
endfunction

// 🌿 Aluriel (Elfos) - Tema: Natureza e Eternidade
function GetAlurielLevels()
    return array{
        struct{ name := "Bosque Guardião", xp := 50000, bonus := 5 },
        struct{ name := "Clarão Abençoado", xp := 150000, bonus := 10 },
        struct{ name := "Reino Silvestre", xp := 350000, bonus := 15 },
        struct{ name := "Corte Eterna", xp := 750000, bonus := 25 },
        struct{ name := "Luz Imperecível", xp := 1500000, bonus := 40 }
    };
endfunction

// 🌾 Polkinea (Polski) - Tema: Agricultura e Simplicidade
function GetPolkineaLevels()
    return array{
        struct{ name := "Sítio Próspero", xp := 50000, bonus := 5 },
        struct{ name := "Vila Abundante", xp := 150000, bonus := 10 },
        struct{ name := "Condado Fértil", xp := 350000, bonus := 15 },
        struct{ name := "Reino Pastoral", xp := 750000, bonus := 25 },
        struct{ name := "Terra Dourada", xp := 1500000, bonus := 40 }
    };
endfunction

// ⚒️ Baduran (Anões) - Tema: Forja e Montanhas
function GetBaduranLevels()
    return array{
        struct{ name := "Forja Acesa", xp := 50000, bonus := 5 },
        struct{ name := "Salão do Martelo", xp := 150000, bonus := 10 },
        struct{ name := "Cidadela de Ferro", xp := 350000, bonus := 15 },
        struct{ name := "Reino Subterrâneo", xp := 750000, bonus := 25 },
        struct{ name := "Khron Kaz'ad Supremo", xp := 1500000, bonus := 40 }
    };
endfunction

// 🗡️ Gulthrak (Orcs) - Tema: Tribos e Deserto
function GetGulthrakLevels()
    return array{
        struct{ name := "Bando Errante", xp := 50000, bonus := 5 },
        struct{ name := "Tribo Unida", xp := 150000, bonus := 10 },
        struct{ name := "Horda Temida", xp := 350000, bonus := 15 },
        struct{ name := "Grande Tribo", xp := 750000, bonus := 25 },
        struct{ name := "Legado de Adar", xp := 1500000, bonus := 40 }
    };
endfunction

// Níveis genéricos para guilds sem cultura definida
function GetGenericLevels()
    return array{
        struct{ name := "Reino Nível 1", xp := 50000, bonus := 5 },
        struct{ name := "Reino Nível 2", xp := 150000, bonus := 10 },
        struct{ name := "Reino Nível 3", xp := 350000, bonus := 15 },
        struct{ name := "Reino Nível 4", xp := 750000, bonus := 25 },
        struct{ name := "Reino Nível 5", xp := 1500000, bonus := 40 }
    };
endfunction

// Datafile para estatísticas de XP
const GUILD_XP_DATAFILE := "guild_xp_stats";

// Propriedades da guild
const GUILD_XP_PROP := "guild_xp";
const GUILD_LEVEL_PROP := "guild_level";
const GUILD_XP_HISTORY_PROP := "guild_xp_history";

// =======================================
// Funções Principais
// =======================================

/*
 * AddGuildXP(who, xp_amount)
 * 
 * Purpose: Adiciona XP à guild do jogador
 * Parameters:
 *   who - O jogador que ganhou XP
 *   xp_amount - Quantidade de XP que o jogador ganhou
 * Return:
 *   1 se sucesso, 0 se falha
 */
function AddGuildXP(who, xp_amount)
    if (!who || !who.guild || xp_amount <= 0)
        return 0;
    endif
    
    var guild := who.guild;
    var guild_name := SafeGetGuildName(guild);
    var guild_culture := SafeGetGuildProperty(guild, "culture", "");
    
    LogDebug("guild_xp", "Processando XP para guild " + guild_name + " (" + guild_culture + "): jogador=" + who.name + ", xp=" + xp_amount);
    
    // Calcular XP para a guild (porcentagem do XP do jogador)
    var guild_xp_gain := CInt(xp_amount * GUILD_XP_PERCENTAGE);
    if (guild_xp_gain <= 0)
        guild_xp_gain := 1; // Mínimo de 1 XP para a guild
    endif
    
    // Obter XP atual da guild
    var current_guild_xp := SafeGetGuildProperty(guild, GUILD_XP_PROP, 0);
    var current_guild_level := SafeGetGuildProperty(guild, GUILD_LEVEL_PROP, 0);
    
    // Adicionar XP
    var new_guild_xp := current_guild_xp + guild_xp_gain;
    SafeSetGuildProperty(guild, GUILD_XP_PROP, new_guild_xp);
    
    // Verificar se subiu de nível
    var new_level := CalculateGuildLevel(new_guild_xp, guild_culture);
    if (new_level > current_guild_level)
        GuildLevelUp(guild, new_level, current_guild_level, guild_culture);
    endif
    
    // Registrar estatísticas
    RecordGuildXPContribution(guild, who, guild_xp_gain);
    
    LogInfo("guild_xp", "XP adicionado à guild " + guild_name + ": +" + guild_xp_gain + " (total: " + new_guild_xp + ")");
    return 1;
endfunction

/*
 * CalculateGuildLevel(total_xp, guild_culture := "")
 * 
 * Purpose: Calcula o nível da guild baseado no XP total
 * Parameters:
 *   total_xp - XP total da guild
 *   guild_culture - Cultura da guild (opcional)
 * Return:
 *   Nível da guild (0-5)
 */
function CalculateGuildLevel(total_xp, guild_culture := "")
    var levels := GetKingdomLevels(guild_culture);
    
    var i := levels.size();
    while (i >= 1)
        if (total_xp >= levels[i].xp)
            return i;
        endif
        i := i - 1;
    endwhile
    
    return 0; // Nível 0 se não atingiu o mínimo
endfunction

/*
 * GetGuildXPForNextLevel(current_level, guild_culture := "")
 * 
 * Purpose: Retorna o XP necessário para o próximo nível
 * Parameters:
 *   current_level - Nível atual da guild
 *   guild_culture - Cultura da guild (opcional)
 * Return:
 *   XP necessário para o próximo nível (0 se já está no máximo)
 */
function GetGuildXPForNextLevel(current_level, guild_culture := "")
    var levels := GetKingdomLevels(guild_culture);
    
    // Se é nível 0, o próximo é nível 1 (primeiro da lista)
    if (current_level <= 0)
        if (levels.size() > 0)
            return levels[1].xp;
        else
            return GUILD_LEVEL_1_XP;
        endif
    endif
    
    // Se já está no nível máximo
    if (current_level >= levels.size())
        return 0; // Nível máximo atingido
    endif
    
    var next_level_index := current_level + 1;
    if (next_level_index > levels.size())
        return 0;
    endif
    
    return levels[next_level_index].xp;
endfunction

/*
 * GetGuildLevelName(level, guild_culture := "")
 * 
 * Purpose: Retorna o nome do nível da guild baseado na cultura
 * Parameters:
 *   level - Nível da guild (1-5)
 *   guild_culture - Cultura da guild (opcional)
 * Return:
 *   Nome do nível
 */
function GetGuildLevelName(level, guild_culture := "")
    var levels := GetKingdomLevels(guild_culture);
    
    // Se é nível 0 ou menor, retorna o primeiro nível (nível 1)
    if (level < 1)
        if (levels.size() > 0)
            return levels[1].name;
        else
            return "Reino Nível 1";
        endif
    endif
    
    // Se é maior que o máximo, retorna o último nível
    if (level > levels.size())
        if (levels.size() > 0)
            return levels[levels.size()].name;
        else
            return "Reino Máximo";
        endif
    endif
    
    return levels[level].name;
endfunction

/*
 * GetGuildLevelBonus(level, guild_culture := "")
 * 
 * Purpose: Retorna o bônus do nível da guild
 * Parameters:
 *   level - Nível da guild (1-5)
 *   guild_culture - Cultura da guild (opcional)
 * Return:
 *   Valor do bônus do nível
 */
function GetGuildLevelBonus(level, guild_culture := "")
    var levels := GetKingdomLevels(guild_culture);
    
    if (level < 1 || level > levels.size())
        return 0;
    endif
    
    return levels[level].bonus;
endfunction

/*
 * GuildLevelUp(guild, new_level, old_level, guild_culture := "")
 * 
 * Purpose: Processa o level up da guild
 * Parameters:
 *   guild - Referência da guild
 *   new_level - Novo nível
 *   old_level - Nível anterior
 *   guild_culture - Cultura da guild (opcional)
 * Return:
 *   1 se sucesso
 */
function GuildLevelUp(guild, new_level, old_level, guild_culture := "")
    var guild_name := SafeGetGuildName(guild);
    var level_name := GetGuildLevelName(new_level, guild_culture);
    
    // Atualizar nível da guild
    SafeSetGuildProperty(guild, GUILD_LEVEL_PROP, new_level);
    
    // Registrar no histórico
    var history := SafeGetGuildProperty(guild, GUILD_XP_HISTORY_PROP, array{});
    if (TypeOf(history) != "Array")
        history := array{};
    endif
    
    var level_up_entry := struct{
        "timestamp" := polcore().systime,
        "old_level" := old_level,
        "new_level" := new_level,
        "level_name" := level_name,
        "event_type" := "level_up"
    };
    
    history.append(level_up_entry);
    
    // Manter apenas as últimas 50 entradas
    if (history.size() > 50)
        history := history[history.size()-49, history.size()];
    endif
    
    SafeSetGuildProperty(guild, GUILD_XP_HISTORY_PROP, history);
    
    // Notificar todos os membros online
    NotifyGuildMembersOfLevelUp(guild, new_level, level_name);
    
    // Aplicar benefícios do novo nível
    ApplyGuildLevelBenefits(guild, new_level, guild_culture);
    
    LogInfo("guild_xp", "Guild " + guild_name + " subiu para " + level_name + " (Nível " + new_level + ")!");
    return 1;
endfunction

/*
 * NotifyGuildMembersOfLevelUp(guild, new_level, level_name := "")
 * 
 * Purpose: Notifica todos os membros online sobre o level up
 * Parameters:
 *   guild - Referência da guild
 *   new_level - Novo nível da guild
 *   level_name - Nome do nível (opcional)
 * Return:
 *   Número de membros notificados
 */
function NotifyGuildMembersOfLevelUp(guild, new_level, level_name := "")
    var guild_name := SafeGetGuildName(guild);
    var notified := 0;
    
    if (level_name == "")
        level_name := "Nível " + new_level;
    endif
    
    foreach member in (guild.members)
        if (member.connected)
            // Efeitos visuais e sonoros
            PlaySoundEffect(member, 0x1F5); // Som de level up diferente
            PlayObjectCenteredEffect(member, 0x373A, 10, 30); // Efeito dourado
            
            // Mensagem especial
            SendSysMessage(member, "*** " + guild_name + " ALCANÇOU: " + level_name + "! ***", 3, 53);
            PrintTextAbove(member, "*Guild Level Up!*", _DEFAULT_TEXT_FONT, 53);
            
            notified += 1;
        endif
        sleepms(2);
    endforeach
    
    return notified;
endfunction

/*
 * ApplyGuildLevelBenefits(guild, level, guild_culture := "")
 * 
 * Purpose: Aplica benefícios do nível da guild
 * Parameters:
 *   guild - Referência da guild
 *   level - Nível da guild
 *   guild_culture - Cultura da guild (opcional)
 * Return:
 *   1 se sucesso
 */
function ApplyGuildLevelBenefits(guild, level, guild_culture := "")
    var guild_name := SafeGetGuildName(guild);
    var level_name := GetGuildLevelName(level, guild_culture);
    var bonus := GetGuildLevelBonus(level, guild_culture);
    
    LogInfo("guild_xp", "Aplicando benefícios do " + level_name + " (+" + bonus + "% bônus) para guild " + guild_name);
    
    // Aqui você pode adicionar benefícios específicos por nível
    // Exemplos:
    case (level)
        1:
            LogInfo("guild_xp", "Guild " + guild_name + " desbloqueou benefícios básicos (" + level_name + ")");
            break;
        2:
            LogInfo("guild_xp", "Guild " + guild_name + " desbloqueou slots adicionais de NPCs (" + level_name + ")");
            break;
        3:
            LogInfo("guild_xp", "Guild " + guild_name + " desbloqueou bônus de geração de recursos (" + level_name + ")");
            break;
        4:
            LogInfo("guild_xp", "Guild " + guild_name + " desbloqueou benefícios avançados (" + level_name + ")");
            break;
        5:
            LogInfo("guild_xp", "Guild " + guild_name + " atingiu o nível máximo (" + level_name + ")");
            break;
    endcase
    
    return 1;
endfunction

/*
 * RecordGuildXPContribution(guild, who, xp_amount)
 * 
 * Purpose: Registra a contribuição de XP de um membro
 * Parameters:
 *   guild - Referência da guild
 *   who - Jogador que contribuiu
 *   xp_amount - Quantidade de XP contribuída
 * Return:
 *   1 se sucesso
 */
function RecordGuildXPContribution(guild, who, xp_amount)
    // Abrir datafile de estatísticas
    var df := SafeOpenDataFile(GUILD_XP_DATAFILE, 1);
    if (!df)
        return 0;
    endif
    
    var guild_name := SafeGetGuildName(guild);
    var elem := SafeFindElement(df, guild_name, 1);
    if (!elem)
        return 0;
    endif
    
    // Obter contribuições atuais
    var contributions := GetElementProp(elem, "contributions", dictionary{});
    if (TypeOf(contributions) != "Dictionary")
        contributions := dictionary{};
    endif
    
    // Adicionar contribuição do jogador
    var player_key := who.serial + "_" + who.name;
    var current_contrib := contributions.exists(player_key) ? contributions[player_key] : 0;
    contributions[player_key] := current_contrib + xp_amount;
    
    // Salvar de volta
    SetElementProp(elem, "contributions", contributions);
    SetElementProp(elem, "last_update", polcore().systime);
    
    return 1;
endfunction

/*
 * GetGuildXPInfo(guild)
 * 
 * Purpose: Obtém informações completas sobre o XP da guild
 * Parameters:
 *   guild - Referência da guild
 * Return:
 *   Struct com informações de XP
 */
function GetGuildXPInfo(guild)
    var current_xp := SafeGetGuildProperty(guild, GUILD_XP_PROP, 0);
    var current_level := SafeGetGuildProperty(guild, GUILD_LEVEL_PROP, 1); // Sempre começa no nível 1
    var guild_culture := SafeGetGuildProperty(guild, "culture", "");
    
    // Garantir que o nível nunca seja menor que 1
    if (current_level < 1)
        current_level := 1;
        SafeSetGuildProperty(guild, GUILD_LEVEL_PROP, 1);
    endif
    
    var next_level_xp := GetGuildXPForNextLevel(current_level, guild_culture);
    var level_name := GetGuildLevelName(current_level, guild_culture);
    var bonus := GetGuildLevelBonus(current_level, guild_culture);
    
    var info := struct{
        "current_xp" := current_xp,
        "current_level" := current_level,
        "level_name" := level_name,
        "bonus" := bonus,
        "culture" := guild_culture,
        "next_level_xp" := next_level_xp,
        "xp_to_next_level" := (next_level_xp > 0 ? next_level_xp - current_xp : 0),
        "progress_percentage" := 0,
        "is_max_level" := (current_level >= 5)
    };
    
    // Calcular progresso baseado no nível atual
    if (next_level_xp > 0)
        var levels := GetKingdomLevels(guild_culture);
        if (current_level <= levels.size())
            var current_level_start_xp := 0;
            if (current_level > 1)
                current_level_start_xp := levels[current_level - 1].xp;
            endif
            
            var level_xp_range := next_level_xp - current_level_start_xp;
            var current_level_progress := current_xp - current_level_start_xp;
            
            if (level_xp_range > 0)
                info.progress_percentage := CDbl(current_level_progress) / CDbl(level_xp_range) * 100.0;
            endif
        endif
    endif
    
    return info;
endfunction

/*
 * GetTopGuildContributors(guild, limit := 10)
 * 
 * Purpose: Obtém os maiores contribuidores de XP da guild
 * Parameters:
 *   guild - Referência da guild
 *   limit - Número máximo de contribuidores a retornar
 * Return:
 *   Array de structs com informações dos contribuidores
 */
function GetTopGuildContributors(guild, limit := 10)
    var df := SafeOpenDataFile(GUILD_XP_DATAFILE);
    if (!df)
        return array{};
    endif
    
    var guild_name := SafeGetGuildName(guild);
    var elem := SafeFindElement(df, guild_name);
    if (!elem)
        return array{};
    endif
    
    var contributions := GetElementProp(elem, "contributions", dictionary{});
    if (TypeOf(contributions) != "Dictionary")
        return array{};
    endif
    
    var contributors := array{};
    foreach player_key in (contributions.keys())
        var parts := SplitWords(player_key, "_");
        if (parts.size() >= 2)
            var player_serial := parts[1];
            var player_name := parts[2];
            
            contributors.append(struct{
                "serial" := player_serial,
                "name" := player_name,
                "contribution" := contributions[player_key]
            });
        endif
    endforeach
    
    // Ordenar por contribuição (maior para menor)
    contributors.sort(array{"contribution"});
    contributors.reverse();
    
    // Limitar resultados
    if (contributors.size() > limit)
        contributors := contributors[1, limit];
    endif
    
    return contributors;
endfunction

/*
 * GetCultureFromFactionName(faction_name)
 * 
 * Purpose: Mapeia nomes de facções para suas culturas correspondentes
 * Parameters:
 *   faction_name - Nome da facção
 * Return:
 *   Nome da cultura correspondente
 */
function GetCultureFromFactionName(faction_name)
    if (!faction_name || faction_name == "")
        return "";
    endif
    
    var faction_lower := Lower(faction_name);
    
    // Mapeamento de facções conhecidas para culturas
    case (faction_lower)
        "aiglanos":
        "aigla":
        "aiglana":
        "aquileia":
        "imperio_aiglanos":
            return "aiglanos";
        "leste":
        "povos_do_leste":
        "orientais":
            return "leste";
        "haroloth":
        "har'oloth":
        "reino_haroloth":
        "drow":
        "drows":
            return "haroloth";
        "bjorske":
        "björske":
        "bjorska":
        "reino_bjorske":
        "nórdicos":
        "nordicos":
            return "bjorske";
        "aluriel":
        "alüriel":
        "reino_aluriel":
        "elfos":
        "élfico":
        "elfico":
            return "aluriel";
        "polkinea":
        "reino_polkinea":
        "polski":
        "halflings":
            return "polkinea";
        "baduran":
        "badûran":
        "reino_baduran":
        "anões":
        "anoes":
        "anão":
        "anao":        "forja":
        "badoran":
            return "baduran";        "guerreiros de jô":
        "guerreiros de jo":
            return "haroloth";        "gulthrak":
        "reino_gulthrak":
        "orcs":
        "orc":
        "deserto":
            return "gulthrak";
        
        // Facções de teste - assumir cultura neutra
        "teste":
        "teste 1":
        "teste 2":
        "teste 3":
        "teste 4":
        "teste 5":
        "test":
        "testing":
            return "polkinea"; // Usar Polkinea como padrão para testes

        default:
            // Verificar se contém palavras-chave específicas usando find()
            if (faction_lower.find("guerreiros") != 0 && faction_lower.find("jo") != 0)
                return "baduran";
            endif
            if (faction_lower.find("anao") != 0 || faction_lower.find("anoes") != 0)
                return "baduran";
            endif
            if (faction_lower.find("orc") != 0)
                return "gulthrak";
            endif
            if (faction_lower.find("elfo") != 0 || faction_lower.find("elfos") != 0)
                return "aluriel";
            endif
            if (faction_lower.find("drow") != 0)
                return "haroloth";
            endif
            if (faction_lower.find("teste") != 0)
                return "polkinea";
            endif
            return "polkinea"; // Fallback padrão para facções desconhecidas
    endcase
endfunction
