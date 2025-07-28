// :faccao:construcao/include/feudo_buildings.inc
//
// Módulo central para a lógica de construções do sistema de feudos.
// Responsável por ler as configurações e fornecer funções de alto nível
// para o resto do sistema.

use uo;
use os;
use cfgfile;
use guilds;

// Inclui para funções auxiliares que serão usadas no futuro
include ":faccao:include/logging";
include ":faccao:include/datafile_helpers";
include ":containers:storageAreas";

/*
 * FEUDO_GetBuildingCfg()
 * Carrega o arquivo de configuração de construções.
 */
function FEUDO_GetBuildingCfg()
    var cfg_file := ReadConfigFile(":faccao:construcao/config/construcoes");
    if (cfg_file.errortext)
        LogError("feudo_buildings", "FALHA CRÍTICA ao carregar construcoes.cfg: " + cfg_file.errortext);
    endif
    return cfg_file;
endfunction

/*
 * FEUDO_GetBuildingLevelDetails(building_type, level) - VERSÃO 3.1 (LEITURA ROBUSTA)
 */
function FEUDO_GetBuildingLevelDetails(building_type, level)
    var cfg := FEUDO_GetBuildingCfg();
    if (cfg.errortext) return cfg; endif

    var elem := FindConfigElem(cfg, building_type);
    if (elem.errortext)
        return error{"errortext" := "Construção '" + building_type + "' não encontrada."};
    endif

    var prefix := "Level" + level + "_";
    var details := struct;

    details.+Name               := GetConfigString(elem, prefix + "Name");
    details.+CustoProjetoCobre  := GetConfigInt(elem, prefix + "CustoProjetoCobre");
    details.+TempoConstrucao    := GetConfigInt(elem, prefix + "TempoConstrucao");
    
    details.+Materiais := dictionary{};
    var materiais_array := GetConfigStringArray(elem, prefix + "Materiais");
    foreach mat_line in materiais_array
        var parts := SplitWords(mat_line);
        if (parts.size() == 2)
            details.Materiais[Lower(parts[1])] := CInt(parts[2]);
        endif
    endforeach
    
    details.+Bonus     := GetConfigStringArray(elem, prefix + "Bonus");
    details.+Estagio0  := GetConfigStringArray(elem, prefix + "Estagio0");
    details.+Estagio1  := GetConfigStringArray(elem, prefix + "Estagio1");
    details.+Estagio2  := GetConfigStringArray(elem, prefix + "Estagio2");
    details.+Estagio3  := GetConfigStringArray(elem, prefix + "Estagio3");
    details.+Estagio4  := GetConfigStringArray(elem, prefix + "Estagio4");
    details.+Estagio5  := GetConfigStringArray(elem, prefix + "Estagio5");
    details.+Estagio6  := GetConfigStringArray(elem, prefix + "Estagio6");
    details.+Estagio7  := GetConfigStringArray(elem, prefix + "Estagio7");
    details.+Estagio8  := GetConfigStringArray(elem, prefix + "Estagio8");
    details.+Estagio9  := GetConfigStringArray(elem, prefix + "Estagio9");
    details.+Blueprint := GetConfigStringArray(elem, prefix + "Blueprint");

    if (!details.Name || details.Name.errortext)
        return error{"errortext" := "Nível " + level + " para '" + building_type + "' não definido corretamente."};
    endif
    
    return details;
endfunction

/*
 * FEUDO_UpdateBuildingVisuals(andaime_principal) - VERSÃO FINAL (CUMULATIVA)
 *
 * Redesenha o canteiro de obras de forma cumulativa, adicionando as peças
 * de cada novo estágio às peças dos estágios anteriores.
 */
function FEUDO_UpdateBuildingVisuals(andaime_principal)
    var LOG_CATEGORY := "FeudoVisuals";
    LogInfo(LOG_CATEGORY, "==================================================");
    LogInfo(LOG_CATEGORY, "Iniciando atualização visual para andaime: " + andaime_principal.serial);

    var build_info := GetObjProperty(andaime_principal, "BuildingInfo");
    if (!build_info)
        LogError(LOG_CATEGORY, "FALHA: CProp 'BuildingInfo' não encontrada. Abortando.");
        return;
    endif
    LogDebug(LOG_CATEGORY, "  -> BuildingInfo: Tipo=" + build_info.type + ", Nível=" + build_info.level);

    var estagio_atual_num := GetStageNumber(andaime_principal);
    LogInfo(LOG_CATEGORY, "  -> Estágio numérico calculado: " + estagio_atual_num);

    var estagio_anterior_num := CInt(GetObjProperty(andaime_principal, "EstagioVisualAtual"));
    if (estagio_anterior_num == estagio_atual_num)
        LogDebug(LOG_CATEGORY, "  -> Estágio visual não mudou. Nenhuma atualização necessária.");
        return;
    endif
    SetObjProperty(andaime_principal, "EstagioVisualAtual", estagio_atual_num);
    LogInfo(LOG_CATEGORY, "  -> Novo estágio visual definido: " + estagio_atual_num);

    // Limpa TODOS os itens visuais antigos para redesenhar a cena completa
    var itens_antigos_seriais := GetObjProperty(andaime_principal, "PecasVisuais") ?: array{};
    if (itens_antigos_seriais.size() > 0)
        LogDebug(LOG_CATEGORY, "  -> Limpando " + itens_antigos_seriais.size() + " peças visuais antigas...");
        foreach serial in itens_antigos_seriais
            var item_antigo := SystemFindObjectBySerial(serial);
            if (item_antigo)
                DestroyItem(item_antigo);
            endif
        endforeach
    endif

    var details := FEUDO_GetBuildingLevelDetails(build_info.type, build_info.level);
    if (details.errortext)
        LogError(LOG_CATEGORY, "FALHA ao carregar detalhes do CFG: " + details.errortext);
        return;
    endif

    var novos_itens_seriais := array{};
    var centro_x := andaime_principal.x;
    var centro_y := andaime_principal.y;
    var centro_z := andaime_principal.z;
    var realm := andaime_principal.realm;

    // --- LÓGICA CUMULATIVA ---
    // Itera de 0 até o estágio atual, criando as peças de TODOS os estágios alcançados.
    var i;
    for (i := 0; i <= estagio_atual_num; i := i + 1)
        var stage_property_name := "Estagio" + i;
        var stage_lines := details[stage_property_name];
        
        if (stage_lines && !stage_lines.errortext)
            LogDebug(LOG_CATEGORY, "  -> Construindo peças para " + stage_property_name + "...");
            foreach stage_line in stage_lines
                var parts := SplitWords(stage_line);
                if (parts.size() >= 4)
                    var graphic    := CInt(parts[1]);
                    var x_offset   := CInt(parts[2]);
                    var y_offset   := CInt(parts[3]);
                    var z_offset   := CInt(parts[4]);

                    var novo_item := CreateItemAtLocation(centro_x + x_offset, centro_y + y_offset, centro_z + z_offset, graphic, 1, realm);
                    if (novo_item)
                        novo_item.movable := 0;
                        SetObjProperty(novo_item, "AndaimePrincipalSerial", andaime_principal.serial);
                        novos_itens_seriais.append(novo_item.serial);
                    endif
                endif
            endforeach
        endif
    endfor
    // --- FIM DA LÓGICA CUMULATIVA ---

    SetObjProperty(andaime_principal, "PecasVisuais", novos_itens_seriais);
    LogInfo(LOG_CATEGORY, "Atualização visual cumulativa concluída. Estágio: " + estagio_atual_num + ". Peças totais: " + novos_itens_seriais.size());
    LogInfo(LOG_CATEGORY, "==================================================");
endfunction

/*
 * GetStageNumber(andaime_principal) - VERSÃO FINAL (8 ESTÁGIOS DE TEMPO)
 *
 * Retorna o número do estágio visual (0 a 8) com base no progresso.
 * A fase de coleta de materiais sempre corresponde ao Estágio 0.
 */
function GetStageNumber(andaime_principal)
    var timer_info := GetObjProperty(andaime_principal, "ConstructionTimer");
    
    if (!timer_info)
        // Se NÃO há timer, estamos na fase de coleta de materiais.
        // O visual é sempre o Estágio 0.
        return 0;
    else
        // Se HÁ um timer, o progresso é baseado no tempo.
        var tempo_total := CInt(timer_info.duration);
        if (tempo_total <= 0)
            tempo_total := 1;
        endif

        var tempo_passado := polcore().systime - CInt(timer_info.start_time);
        var progresso_tempo := CInt((tempo_passado * 100) / tempo_total);
        
        // Mapeia a porcentagem de tempo para os seus 8 estágios
        if (progresso_tempo >= 90)
            return 8;
        elseif (progresso_tempo >= 80)
            return 7;
        elseif (progresso_tempo >= 65)
            return 6;
        elseif (progresso_tempo >= 55)
            return 5;
        elseif (progresso_tempo >= 45)
            return 4;
        elseif (progresso_tempo >= 30)
            return 3;
        elseif (progresso_tempo >= 20)
            return 2;
        elseif (progresso_tempo >= 10)
            return 1;
        else // 0-9% do tempo
            // Logo após o início do timer, ainda mostra o andaime inicial.
            return 0; 
        endif
    endif
endfunction
/*
 * FEUDO_CompleteConstruction(andaime_principal) - VERSÃO FINAL (PLACA GENÉRICA E RASTREAMENTO)
 *
 * Finaliza uma construção, substituindo o andaime pela estrutura completa
 * definida no 'Blueprint', promove a primeira placa encontrada a
 * "Controlador Mestre" e REGISTRA o serial desta placa na guilda dona.
 */
function FEUDO_CompleteConstruction(andaime_principal)
    // Etapa 1: Validação e obtenção de dados essenciais
    if (!andaime_principal)
        LogError("FeudoComplete", "Função chamada com andaime nulo. Abortando.");
        return 0;
    endif

    var build_info := GetObjProperty(andaime_principal, "BuildingInfo");
    var guild_id := GetObjProperty(andaime_principal, "GuildOwner");
    
    if (!build_info || !guild_id)
        LogError("FeudoComplete", "Andaime " + andaime_principal.serial + " não possui CProps 'BuildingInfo' ou 'GuildOwner'. Abortando.");
        DestroyItem(andaime_principal); 
        return 0;
    endif

    var guild := FindGuild(guild_id);
    if (!guild)
        LogError("FeudoComplete", "Guilda " + guild_id + " não encontrada. Andaime " + andaime_principal.serial + " será removido.");
        DestroyItem(andaime_principal);
        return 0;
    endif

    var details := FEUDO_GetBuildingLevelDetails(build_info.type, build_info.level);
    LogInfo("FeudoComplete", "Finalizando construção de '" + details.Name + "' para a guilda " + guild.name);

    // Etapa 2: Limpar todos os itens visuais do andaime
    var pecas_seriais := GetObjProperty(andaime_principal, "PecasVisuais") ?: array{};
    if (pecas_seriais.size() > 0)
        LogDebug("FeudoComplete", "Removendo " + pecas_seriais.size() + " peças visuais do andaime...");
        foreach serial in pecas_seriais
            var peca := SystemFindObjectBySerial(serial);
            if (peca)
                DestroyItem(peca);
            endif
        endforeach
    endif

    // Etapa 3: Obter a posição e destruir o "cérebro" do andaime
    var x := andaime_principal.x;
    var y := andaime_principal.y;
    var z := andaime_principal.z;
    var realm := andaime_principal.realm;
    DestroyItem(andaime_principal);

    // Etapa 4: Construir a partir do Blueprint e identificar a placa
    var blueprint_lines := details.Blueprint;
    if (!blueprint_lines || blueprint_lines.size() == 0)
        LogWarning("FeudoComplete", "Nenhum 'Blueprint' definido. Nenhum item físico será criado.");
    else
        LogInfo("FeudoComplete", "Construindo '" + details.Name + "' a partir de um blueprint com " + blueprint_lines.size() + " peças.");
        
        var pecas_da_construcao := array{};
        var placas_validas_graphics := FEUDO_GetPlacaObjtypes();
        var placa_mestre_criada := 0;

        foreach piece_line in blueprint_lines
            var parts := SplitWords(piece_line);
            if (parts.size() >= 4)
                var graphic_from_cfg := CInt(parts[1]);
                var x_offset   := CInt(parts[2]);
                var y_offset   := CInt(parts[3]);
                var z_offset   := CInt(parts[4]);
                var peca := 0;

                if (graphic_from_cfg in placas_validas_graphics && !placa_mestre_criada)
                    peca := CreateItemAtLocation(x + x_offset, y + y_offset, z + z_offset, 0xEEF1, 1, realm);
                    if (peca)
                        peca.graphic := graphic_from_cfg;
                        placa_mestre_criada := peca;
                    endif
                else
                    peca := CreateItemAtLocation(x + x_offset, y + y_offset, z + z_offset, graphic_from_cfg, 1, realm);
                endif

                if (peca)
                    peca.movable := 0;
                    SetObjProperty(peca, "GuildOwner", guild_id);
                    SetObjProperty(peca, "BuildingType", build_info.type);
                    SetObjProperty(peca, "IsFeudoBuildingPiece", 1);
                    pecas_da_construcao.append(peca.serial);
                endif
            endif
            sleepms(5);
        endforeach

        if (placa_mestre_criada)
            placa_mestre_criada.name := details.Name;
            
            var script_path := ":faccao:construcao/construcoes_todas/casa_uso";
            LogDebug("FeudoComplete", "  -> Tentando atribuir script: '" + script_path + "'");
            placa_mestre_criada.usescript := script_path;
            
            sleepms(10);
            if (placa_mestre_criada.usescript == script_path)
                LogInfo("FeudoComplete", "  -> SUCESSO: Script atribuído corretamente.");
            else
                LogError("FeudoComplete", "  -> FALHA: O script NÃO foi atribuído. Verifique o caminho e se o arquivo existe e compila.");
            endif
            
            SetObjProperty(placa_mestre_criada, "BuildingInfo", build_info);
            SetObjProperty(placa_mestre_criada, "BuildingPieces", pecas_da_construcao);
            SetObjProperty(placa_mestre_criada, "IsFeudoBuildingController", 1);

            // ====================================================================
            // >>>>> INÍCIO DA ADIÇÃO CRÍTICA PARA RASTREAMENTO <<<<<
            // ====================================================================
            LogInfo("FeudoComplete", "--> Etapa 4.5: Registrando construção na lista de propriedades da guilda...");
            var feudo_owned_constructions := guild.GetProp("FeudoOwnedConstructions") ?: array{};
            
            // Garantir que a CProp é um array antes de adicionar
            if (TypeOf(feudo_owned_constructions) != "Array")
                 feudo_owned_constructions := array{};
            endif
            
            feudo_owned_constructions.append(placa_mestre_criada.serial);
            guild.SetProp("FeudoOwnedConstructions", feudo_owned_constructions);
            LogInfo("FeudoComplete", "    ...Construção " + placa_mestre_criada.serial + " registrada com sucesso. Total na lista: " + feudo_owned_constructions.size());
            // ====================================================================
            // >>>>> FIM DA ADIÇÃO <<<<<
            // ====================================================================

        else
            LogError("FeudoComplete", "AVISO: Nenhuma placa encontrada no blueprint! Criando um controlador mestre invisível como fallback.");
            var mestre_fallback := CreateItemAtLocation(x, y, z, 0x31FA, 1, realm);
            if (mestre_fallback)
                mestre_fallback.name := "Controlador Mestre: " + details.Name;
                mestre_fallback.invisible := 1;
                SetObjProperty(mestre_fallback, "IsFeudoBuildingController", 1);
                SetObjProperty(mestre_fallback, "GuildOwner", guild_id);
                SetObjProperty(mestre_fallback, "BuildingInfo", build_info);
                SetObjProperty(mestre_fallback, "BuildingPieces", pecas_da_construcao);
                
                // Também registrar o fallback na lista da guilda
                var feudo_owned_constructions := guild.GetProp("FeudoOwnedConstructions") ?: array{};
                if (TypeOf(feudo_owned_constructions) != "Array") feudo_owned_constructions := array{}; endif
                feudo_owned_constructions.append(mestre_fallback.serial);
                guild.SetProp("FeudoOwnedConstructions", feudo_owned_constructions);
                LogInfo("FeudoComplete", "    ...Controlador fallback " + mestre_fallback.serial + " registrado na lista da guilda.");
            endif
        endif
    endif

    // Etapa 5: Registrar a construção como concluída na CProp da guilda (para bônus)
    var feudo_buildings := guild.GetProp("feudo_buildings") ?: dictionary{};
    feudo_buildings[build_info.type] := build_info.level;
    guild.SetProp("feudo_buildings", feudo_buildings);
    LogInfo("FeudoComplete", "--> Etapa 5: Construção registrada para ativação de bônus.");
    
    // ==========================================================
    // >>>>> INÍCIO DA NOVA LÓGICA DE ATUALIZAÇÃO DE POPULAÇÃO <<<<<
    // ==========================================================
    LogInfo("FeudoComplete", "--> Etapa 5.5: Processando bônus de população...");
    var bonus_string_array := details.Bonus;
    var populacao_a_adicionar := 0;

    foreach bonus_line in bonus_string_array
        var parts := SplitWords(bonus_line);
        if (Lower(parts[1]) == "popcap" && parts.size() >= 2)
            populacao_a_adicionar := CInt(parts[2]);
            LogDebug("FeudoComplete", "    Bônus 'PopCap' encontrado. Valor: " + populacao_a_adicionar);
            break;
        endif
    endforeach

    if (populacao_a_adicionar > 0)
        var guild_name := guild.GetProp("name");
        var df := SafeOpenDataFile(KINGDOM_RESOURCE_DATAFILE, 1);
        var elem_name := NormalizarNomeElementoFaccao(guild_name);

        if (df && elem_name != error)
            var elem := SafeFindElement(df, elem_name, 1);
            if (elem)
                var recursos := GetElementProp(elem, "recursos", CreateEmptyResourceStruct());
                
                var pop_antiga := CInt(recursos.populacao);
                recursos.populacao := pop_antiga + populacao_a_adicionar;
                
                SetElementProp(elem, "recursos", recursos);
                LogInfo("FeudoComplete", "    SUCESSO: População da facção '" + guild_name + "' atualizada de " + pop_antiga + " para " + recursos.populacao);
            else
                LogError("FeudoComplete", "    FALHA: Não foi possível encontrar/criar o elemento '" + elem_name + "' no datafile de recursos.");
            endif
        else
            LogError("FeudoComplete", "    FALHA: Não foi possível abrir o datafile de recursos ou normalizar o nome da facção '" + guild_name + "'.");
        endif
    endif
    // ==========================================================
    // >>>>> FIM DA NOVA LÓGICA <<<<<
    // ==========================================================
    
    
    // Etapa 6: Notificar TODOS os membros online da guilda
    var guild_name := guild.GetProp("name") ?: guild.guildid;
    var message := "A construção de '" + details.Name + "' no feudo de " + guild_name + " foi concluída!";
    LogInfo("FeudoComplete", "--> Etapa 6: Enviando notificação de conclusão para a guilda " + guild_name);
    
    foreach member in (guild.members)
        if (member.connected)
            SendSysMessage(member, message, 3, 68); // Fonte 3, Cor Verde Claro
            PlaySoundEffect(member, 0x0567); // Som de "sucesso/fanfarra"
        endif
    endforeach
    
    return 1;
endfunction
    
  
/*
 * FEUDO_GetConstructionStatusString(andaime)
 *
 * Gera uma string formatada para a tooltip com o status atual da construção.
 */
function FEUDO_GetConstructionStatusString(andaime)
    var status_string := "";
    var timer_info := GetObjProperty(andaime, "ConstructionTimer");

    if (timer_info)
        // A construção está na fase de tempo
        var tempo_total := CInt(timer_info.duration);
        var tempo_passado := polcore().systime - CInt(timer_info.start_time);
        
        if (tempo_passado < 0)
            tempo_passado := 0;
        endif
        if (tempo_passado > tempo_total)
            tempo_passado := tempo_total;
        endif
        
        var progresso_tempo := 0;
        if (tempo_total > 0)
            progresso_tempo := CInt((tempo_passado * 100) / tempo_total);
        endif

        status_string := "<br><BASEFONT COLOR=#FFA500>Construindo: " + progresso_tempo + "%"; // Laranja
    else
        // A construção está na fase de coleta de materiais
        var materiais_status := GetObjProperty(andaime, "MateriaisStatus");
        if (materiais_status)
            var total_necessario := 1; // Evita divisão por zero
            var total_atual := 0;
            foreach mat in (materiais_status.keys())
                total_necessario += materiais_status[mat].necessario;
                total_atual += materiais_status[mat].atual;
            endforeach
            
            var progresso_mat := CInt((total_atual * 100) / total_necessario);
            status_string := "<br><BASEFONT COLOR=#FFFF00>Materiais: " + progresso_mat + "%"; // Amarelo
        endif
    endif
    
    return status_string;
endfunction



/*
 * FEUDO_CancelConstruction(who, andaime_principal) - VERSÃO SEM REEMBOLSO
 *
 * Cancela uma construção em andamento. TODOS os recursos depositados são perdidos.
 */
function FEUDO_CancelConstruction(who, andaime_principal)
    if (!IsGuildLeader(who))
        SendSysMessage(who, "Apenas o líder pode cancelar a construção.");
        return 0;
    endif

    // A mensagem de confirmação agora é mais severa
    if (!YesNo(who, "Tem certeza que deseja cancelar esta construção? TODOS os materiais depositados serão PERDIDOS."))
        return 0;
    endif

    var LOG_CATEGORY := "FeudoCancel";
    LogInfo(LOG_CATEGORY, "Líder " + who.name + " iniciou o cancelamento do andaime " + andaime_principal.serial);

    var guild_id := GetObjProperty(andaime_principal, "GuildOwner");
    var guild := FindGuild(guild_id);
    if (!guild)
        LogError(LOG_CATEGORY, "Guilda " + guild_id + " não encontrada. Limpando andaime órfão.");
        DestroyItem(andaime_principal);
        return 0;
    endif

    // 1. Remover a construção da fila do controlador
    var construcoes := guild.GetProp("feudo_constructions") ?: dictionary{};
    if (construcoes.exists(CStr(andaime_principal.serial)))
        construcoes.erase(CStr(andaime_principal.serial));
        guild.SetProp("feudo_constructions", construcoes);
        LogDebug(LOG_CATEGORY, "Construção removida da fila do controlador.");
    endif

    // 2. Limpar todos os itens visuais do andaime
    var pecas_seriais := GetObjProperty(andaime_principal, "PecasVisuais") ?: array{};
    foreach serial in pecas_seriais
        var peca := SystemFindObjectBySerial(serial);
        if (peca)
            DestroyItem(peca);
        endif
    endforeach
    LogDebug(LOG_CATEGORY, pecas_seriais.size() + " peças visuais removidas.");

    // 3. Destruir o item "cérebro" do andaime
    DestroyItem(andaime_principal);
    LogInfo(LOG_CATEGORY, "Andaime " + andaime_principal.serial + " cancelado e removido com sucesso.");

    SendSysMessage(who, "A construção foi cancelada e todos os materiais investidos foram perdidos.");
    return 1;
endfunction

/*
 * FEUDO_GetPlacaObjtypes()
 * Retorna uma lista de todos os objtypes de placas.
 */
function FEUDO_GetPlacaObjtypes()
    return array{
        0x0B95, 0x0B96, 0x0B97, 0x0B98, 0x0B99, 0x0B9A, 0x0B9B, 0x0B9C, 0x0B9D, 0x0B9E, 0x0B9F, 0x0BA0, 
        0x0BA1, 0x0BA2, 0x0BA3, 0x0BA4, 0x0BA5, 0x0BA6, 0x0BA7, 0x0BA8, 0x0BA9, 0x0BAA, 0x0BAB, 0x0BAC, 
        0x0BAD, 0x0BAE, 0x0BAF, 0x0BB0, 0x0BB1, 0x0BB2, 0x0BB3, 0x0BB4, 0x0BB5, 0x0BB6, 0x0BB7, 0x0BB8, 
        0x0BB9, 0x0BBA, 0x0BBB, 0x0BBC, 0x0BBD, 0x0BBE, 0x0BBF, 0x0BC0, 0x0BC1, 0x0BC2, 0x0BC3, 0x0BC4, 
        0x0BC5, 0x0BC6, 0x0BC7, 0x0BC8, 0x0BC9, 0x0BCA, 0x0BCB, 0x0BCC, 0x0BCD, 0x0BCE, 0x0BCF, 
        0x0BD0, 0x0BD1, 0x0BD2, 0x0BD3, 0x0BD4, 0x0BD5, 0x0BD6, 0x0BD7, 0x0BD8, 0x0BD9, 0x0BDA, 0x0BDB, 
        0x0BDC, 0x0BDD, 0x0BDE, 0x0BDF, 0x0BF0, 0x0BF1, 0x0BF2, 0x0BF3, 0x0BF4, 0x0BF5, 0x0BF6, 0x0BF7, 
        0x0BF8, 0x0BF9, 0x0BFA, 0x0BFB, 0x0BFC, 0x0BFD, 0x0BFE, 0x0BFF, 0x0C00, 0x0C01, 0x0C02, 0x0C03, 
        0x0C04, 0x0C05, 0x0C06, 0x0C07, 0x0C08, 0x0C09, 0x0C0A, 0x0C0B, 0x0C0C, 0x0C0D, 0x0C0E,
        0x72D0, 0x72D1, 0x72D2, 0x72D3, 0x72D4, 0x72D5, 0x72D6, 0x72D7, 0x72D8, 0x72D9, 0x72DA, 0x72DB, 
        0x72DC, 0x72DD, 0x72DE, 0x72DF, 0x72E0, 0x72E1,
        0xF109, 0xF10A, 0xF10B, 0xF10C, 0xF10D, 0xF10E, 0xF10F, 0xF110, 0xF111, 0xF112, 0xF113, 0xF114,
        0xF115, 0xF116, 0xF117, 0xF118
    };
endfunction


// NO ARQUIVO: feudo_buildings.inc

/*
 * PlayRandomSmokeEffect(andaime_principal) - VERSÃO 2.0 (BOUNDING BOX)
 *
 * Calcula a "caixa delimitadora" completa da construção final a partir do
 * blueprint e toca um efeito de fumaça em um ponto aleatório dentro dela.
 */
/*
 * PlayRandomSmokeEffect(andaime_principal) - VERSÃO 2.0 (BOUNDING BOX)
 *
 * Calcula a "caixa delimitadora" completa da construção final a partir do
 * blueprint e toca um efeito de fumaça em um ponto aleatório dentro dela.
 */
/*
 * PlayRandomSmokeEffect(andaime_principal) - VERSÃO 2.0 (BOUNDING BOX)
 *
 * Calcula a "caixa delimitadora" completa da construção final a partir do
 * blueprint e toca um efeito de fumaça em um ponto aleatório dentro dela.
 */
function PlayRandomSmokeEffect(andaime_principal)
    var build_info := GetObjProperty(andaime_principal, "BuildingInfo");
    if (!build_info)
        // Fallback: se não conseguir ler as infos, toca o efeito no centro.
        PlayObjectCenteredEffect(andaime_principal, 0x3728, 10, 10);
        return;
    endif

    var details := FEUDO_GetBuildingLevelDetails(build_info.type, build_info.level);
    var blueprint_lines := details.Blueprint;

    if (!blueprint_lines || blueprint_lines.size() == 0)
        // Fallback se não houver blueprint
        PlayObjectCenteredEffect(andaime_principal, 0x3728, 10, 10);
        return;
    endif

    // --- CÁLCULO DA CAIXA DELIMITADORA (BOUNDING BOX) ---
    var min_x_offset := 100, max_x_offset := -100;
    var min_y_offset := 100, max_y_offset := -100;
    var min_z_offset := 100, max_z_offset := -100;
    
    foreach piece_line in blueprint_lines
        var parts := SplitWords(piece_line);
        if (parts.size() >= 4)
            var x_offset := CInt(parts[2]);
            var y_offset := CInt(parts[3]);
            var z_offset := CInt(parts[4]);

            if (x_offset < min_x_offset) min_x_offset := x_offset; endif
            if (x_offset > max_x_offset) max_x_offset := x_offset; endif
            if (y_offset < min_y_offset) min_y_offset := y_offset; endif
            if (y_offset > max_y_offset) max_y_offset := y_offset; endif
            if (z_offset < min_z_offset) min_z_offset := z_offset; endif
            if (z_offset > max_z_offset) max_z_offset := z_offset; endif
        endif
    endforeach
    
    // Calcula um ponto aleatório dentro dos limites da casa final
    var rand_x := (RandomInt(max_x_offset - min_x_offset + 1) + min_x_offset) + andaime_principal.x;
    var rand_y := (RandomInt(max_y_offset - min_y_offset + 1) + min_y_offset) + andaime_principal.y;
    var rand_z := (RandomInt(max_z_offset - min_z_offset + 1) + min_z_offset) + andaime_principal.z;
    
    // Toca o efeito de fumaça (0x3728) no local aleatório
    PlayStationaryEffect(rand_x, rand_y, rand_z, 0x3728, 10, 15, andaime_principal.realm);
endfunction


/*
 * DemolirConstrucao(placa_mestre) - VERSÃO 2.0 COM AUTO-LIMPEZA
 */
function DemolirConstrucao(placa_mestre)
    LogInfo("FeudoDemolicao", "Iniciando demolição para a placa: " + placa_mestre.serial);
    
    // --- LÓGICA DE AUTO-LIMPEZA DA LISTA DA GUILDA ---
    var owner_guild_id := GetObjProperty(placa_mestre, "GuildOwner");
    if (owner_guild_id)
        var guild := FindGuild(owner_guild_id);
        if (guild)
            var feudo_owned_constructions := guild.GetProp("FeudoOwnedConstructions") ?: array{};
            var new_list := array{};
            foreach serial in feudo_owned_constructions
                if (CInt(serial) != placa_mestre.serial)
                    new_list.append(serial);
                endif
            endforeach
            guild.SetProp("FeudoOwnedConstructions", new_list);
            LogInfo("FeudoDemolicao", "Construção " + placa_mestre.serial + " removida da lista de propriedades da guilda.");
        endif
    endif
    // --- FIM DA LÓGICA DE AUTO-LIMPEZA ---

    var pecas_seriais := GetObjProperty(placa_mestre, "BuildingPieces");
    if (pecas_seriais)
        foreach serial in pecas_seriais
            var peca := SystemFindObjectBySerial(serial);
            if (peca)
                DestroyItem(peca);
            endif
        endforeach
    endif
    
    DestroyItem(placa_mestre);
    
    LogInfo("FeudoDemolicao", "Construção " + placa_mestre.serial + " demolida com sucesso.");
endfunction


// Adicione esta nova função ao arquivo

/*
 * FEUDO_GetOwnedConstructions(guild_obj)
 *
 * Retorna um array de referências a todas as placas de construção
 * pertencentes a uma guilda.
 */
function FEUDO_GetOwnedConstructions(guild_obj)
    var owned_constructions := array{};
    if (!guild_obj)
        return owned_constructions;
    endif

    // Usa a lista de seriais que já mantemos na CProp da guilda.
    // Isso é MUITO mais rápido do que varrer o mundo inteiro.
    var construction_serials := guild_obj.GetProp("FeudoOwnedConstructions") ?: array{};

    foreach serial_str in construction_serials
        var placa := SystemFindObjectBySerial(CInt(serial_str));
        if (placa)
            owned_constructions.append(placa);
        endif
    endforeach
    
    return owned_constructions;
endfunction


// Função de débito agora usa o realm como o nome da StorageArea (VERSÃO FINAL CORRIGIDA)
// Função de débito agora SEMPRE busca na StorageArea "World Bank" (VERSÃO FINAL SIMPLIFICADA)
function DebitarAluguelDoBanco(player, amount_due, realm_name)
    // O parâmetro 'realm_name' agora é ignorado, mas mantido para compatibilidade com a chamada da função.
    realm_name := realm_name;

    var LOG_CATEGORY := "FeudoRent_Debit";
    
    // CORREÇÃO CRÍTICA: Força a busca na área de banco global.
    var storage_area_name := "World Bank";

    LogInfo(LOG_CATEGORY, "--> [MODO GLOBAL] Procurando banco para " + player.name + " na StorageArea alvo: '" + storage_area_name + "'");
    
    // Chama a função do sistema de storage com o nome correto da área
    var bank_box := CP_GetStorageContainerForMobile(player, storage_area_name, CP_NOCREATE);
	
    if (!bank_box)
        LogWarning(LOG_CATEGORY, "--> FALHA: Jogador " + player.name + " não possui um banco na StorageArea '" + storage_area_name + "'. Verifique se o jogador já usou um banqueiro.");
        return 0; // Falha: não tem banco nesta área
    endif

    LogInfo(LOG_CATEGORY, "--> Banco de " + player.name + " encontrado na StorageArea '" + storage_area_name + "' (Container Serial: " + bank_box.serial + ").");
    LogInfo(LOG_CATEGORY, "--> Tentando debitar " + amount_due + " de Cobre (0xBA63).");
    
    if (ConsumeSubstance(bank_box, 0xBA63, amount_due))
        LogInfo(LOG_CATEGORY, "--> SUCESSO: Débito de " + amount_due + " moedas realizado.");
        return 1;
    else
        LogWarning(LOG_CATEGORY, "--> FALHA: Jogador " + player.name + " não tem " + amount_due + " moedas de cobre em seu banco na StorageArea '" + storage_area_name + "'.");
        return 0;
    endif
endfunction


/*
 * DebitarAluguelDoBanco_Robusto(player, amount_due) - VERSÃO APENAS COBRE
 * 
 * Versão robusta que soma o valor total APENAS de moedas de cobre no banco
 * e debita de múltiplos stacks se necessário.
 */
function DebitarAluguelDoBanco_Robusto(player, amount_due)
    var LOG_CATEGORY := "FeudoRent_Debit_Robusto";
    LogInfo(LOG_CATEGORY, "==========================================================");
    LogInfo(LOG_CATEGORY, "INICIANDO COBRANÇA ROBUSTA (APENAS COBRE) para " + player.name + ". Valor: " + amount_due);

    var storage_area_name := "World Bank";
    var bank_box := CP_GetStorageContainerForMobile(player, storage_area_name, CP_NOCREATE);

    if (!bank_box)
        LogWarning(LOG_CATEGORY, "--> FALHA (Passo 1): Jogador " + player.name + " não possui banco na StorageArea '" + storage_area_name + "'.");
        return 0;
    endif
    LogInfo(LOG_CATEGORY, "--> SUCESSO (Passo 1): Banco encontrado (Serial: " + bank_box.serial + ")");

    // Passo 2: Calcular o valor total disponível APENAS em cobre
    var total_disponivel_cobre := 0;
    var pilhas_de_cobre := array{};

    LogDebug(LOG_CATEGORY, "--> PASSO 2: Varrendo o conteúdo do banco para encontrar moedas de cobre (0xBA63)...");
    foreach item in EnumerateItemsInContainer(bank_box)
        if (item.objtype == 0xBA63)
            total_disponivel_cobre += item.amount;
            pilhas_de_cobre.append(item);
            LogDebug(LOG_CATEGORY, "    -> Pilha de cobre encontrada: Serial " + item.serial + ", Quantidade: " + item.amount);
        endif
    endforeach
    
    if (pilhas_de_cobre.size() == 0)
        LogWarning(LOG_CATEGORY, "    -> Nenhuma pilha de moedas de cobre (0xBA63) foi encontrada no banco.");
    endif
    LogInfo(LOG_CATEGORY, "--> SUCESSO (Passo 2): Total disponível no banco: " + total_disponivel_cobre + " moedas de cobre.");

    // Passo 3: Verificar se há fundos suficientes
    if (total_disponivel_cobre < amount_due)
        LogWarning(LOG_CATEGORY, "--> FALHA (Passo 3): Fundos insuficientes. Disponível: " + total_disponivel_cobre + ", Necessário: " + amount_due);
        return 0;
    endif
    LogInfo(LOG_CATEGORY, "--> SUCESSO (Passo 3): Fundos suficientes. Iniciando débito.");

    // Passo 4: Debitar o valor
    var valor_restante_a_debitar := amount_due;
    Set_Critical(1);

    LogDebug(LOG_CATEGORY, "--> PASSO 4: Iniciando débito de " + valor_restante_a_debitar + " moedas das pilhas encontradas.");
    foreach moeda in pilhas_de_cobre
        if (valor_restante_a_debitar <= 0)
            break;
        endif
        
        var a_remover := (moeda.amount > valor_restante_a_debitar) ? valor_restante_a_debitar : moeda.amount;
        LogDebug(LOG_CATEGORY, "    -> Debitando " + a_remover + " da pilha " + moeda.serial + " (que tem " + moeda.amount + ").");
        if (SubtractAmount(moeda, a_remover))
            valor_restante_a_debitar -= a_remover;
            LogDebug(LOG_CATEGORY, "    -> Sucesso. Resta debitar: " + valor_restante_a_debitar);
        else
            LogError(LOG_CATEGORY, "    -> FALHA ao usar SubtractAmount na pilha " + moeda.serial + ". Abortando débito.");
            valor_restante_a_debitar := -1; // Sinaliza erro
            break;
        endif
    endforeach
    
    Set_Critical(0);

    if (valor_restante_a_debitar > 0)
        LogError(LOG_CATEGORY, "--> FALHA (Passo 4): Erro de lógica no débito. Restou " + valor_restante_a_debitar + " a debitar. Estornando valor pago.");
        CreateItemInContainer(bank_box, 0xBA63, amount_due - valor_restante_a_debitar);
        return 0;
    endif

    LogInfo(LOG_CATEGORY, "--> SUCESSO (Passo 4): Débito de " + amount_due + " realizado com sucesso.");
    LogInfo(LOG_CATEGORY, "==========================================================");
    return 1;
endfunction