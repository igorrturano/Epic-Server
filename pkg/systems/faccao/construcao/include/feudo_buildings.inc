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

/*
 * FEUDO_GetBuildingCfg()
 * Carrega o arquivo de configuração de construções de forma segura.
 */
function FEUDO_GetBuildingCfg()
    // O seu arquivo está em 'construcao/config', então o caminho é ':faccao:construcao/config/construcoes'
    var cfg_file := ReadConfigFile(":faccao:construcao/config/construcoes");

    if (cfg_file.errortext)
        LogError("feudo_buildings", "FALHA CRÍTICA ao carregar construcoes.cfg: " + cfg_file.errortext);
    endif
    
    return cfg_file;
endfunction

/*
 * FEUDO_GetBuildingLevelDetails(building_type, level) - VERSÃO 2 (PARSER ROBUSTO)
 */
function FEUDO_GetBuildingLevelDetails(building_type, level)
    var cfg := FEUDO_GetBuildingCfg();
    if (cfg.errortext)
        return cfg;
    endif

    var elem := FindConfigElem(cfg, building_type);
    if (elem.errortext)
        return error{"errortext" := "Construção '" + building_type + "' não encontrada."};
    endif

    var all_levels_dict := GetConfigStringDictionary(elem, "Level");
    if (all_levels_dict.errortext || !all_levels_dict.exists(CStr(level)))
        return error{"errortext" := "Nível " + level + " para '" + building_type + "' não definido."};
    endif

    var level_data_string := all_levels_dict[CStr(level)];
    
    var details := struct;
    details.+Materiais := dictionary{};
    details.+Bonus := array{};
    details.+Estagios := dictionary{};
    details.+Blueprint := array{};

    // Separa as seções principais (dados; estagios; blueprint) pelo caractere '|'
    var sections := SplitWords(level_data_string, "|");

    foreach section in sections
        section := Trim(section);
        var main_parts := SplitWords(section, "=");
        if (main_parts.size() < 2)
            continue;
        endif

        var section_name := Trim(Lower(main_parts[1]));
        var section_value := Trim(main_parts[2]);

        // Se a seção for uma das de dados principais, ela não tem nome (começa com Name=)
        if (section_name == "name")
            var data_pairs := SplitWords(section, ";");
            foreach pair in data_pairs
                var parts := SplitWords(pair, "=");
                if (parts.size() == 2)
                    var prop_name := Trim(Lower(parts[1]));
                    var prop_value := Trim(parts[2]);
                    case(prop_name)
                        "name":
                            details.+Name := prop_value;
                        "custoprojeto":
                            details.+CustoProjetoCobre := CInt(prop_value);
                        "tempo":
                            details.+TempoConstrucao := CInt(prop_value);
                        "materiais":    
                            foreach mat_entry in SplitWords(prop_value, ",")
                                var mat_parts := SplitWords(Trim(mat_entry));
                                if (mat_parts.size() == 2)
                                    details.Materiais[Lower(mat_parts[1])] := CInt(mat_parts[2]);
                                endif
                            endforeach
                        "bonus":
                            foreach bonus_entry in SplitWords(prop_value, ",")
                                details.Bonus.append(Trim(bonus_entry));
                            endforeach
                    endcase
                endif
            endforeach
        // Processa as seções de Estágios
        elseif (section_name.find("estagio") == 1)
            var stage_num_str := section_name[7, len(section_name)];
            var stage_num := CInt(stage_num_str);
            details.Estagios[stage_num] := array{};
            foreach piece_str in SplitWords(section_value, ",")
                details.Estagios[stage_num].append(SplitWords(Trim(piece_str)));
            endforeach
        // Processa a seção de Blueprint
        elseif (section_name == "blueprint")
            foreach piece_str in SplitWords(section_value, ",")
                details.Blueprint.append(SplitWords(Trim(piece_str)));
            endforeach
        endif
    endforeach
    
    return details;
endfunction
/*
 * FEUDO_UpdateBuildingVisuals(andaime_principal) - VERSÃO 4.2 (FINAL E CORRIGIDA)
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

    // Limpa os itens antigos
    var itens_antigos_seriais := GetObjProperty(andaime_principal, "PecasVisuais") ?: array{};
    if (itens_antigos_seriais.size() > 0)
        LogDebug(LOG_CATEGORY, "  -> Limpando " + itens_antigos_seriais.size() + " peças visuais antigas...");
        foreach serial in itens_antigos_seriais
            var item_antigo := SystemFindObjectBySerial(serial);
            if (item_antigo) DestroyItem(item_antigo); endif
        endforeach
    endif

    // Cria os novos itens
    var cfg := FEUDO_GetBuildingCfg();
    var elem := FindConfigElem(cfg, build_info.type);
    
    // --- INÍCIO DA CORREÇÃO ---
    // Lê TODAS as linhas "Estagio" de uma vez.
    var all_stages := GetConfigStringArray(elem, "Estagio"); 
    // --- FIM DA CORREÇÃO ---

    if (all_stages.errortext)
        LogWarning(LOG_CATEGORY, "  -> AVISO: Nenhuma propriedade 'Estagio' encontrada para '" + build_info.type + "'.");
        SetObjProperty(andaime_principal, "PecasVisuais", array{});
        return;
    endif
    LogInfo(LOG_CATEGORY, "  -> Lidas " + all_stages.size() + " definições de estágio do CFG.");

    var novos_itens_seriais := array{};
    var centro_x := andaime_principal.x;
    var centro_y := andaime_principal.y;
    var centro_z := andaime_principal.z;
    var realm := andaime_principal.realm;

    foreach stage_line in all_stages
        var parts := SplitWords(stage_line);
        if (parts.size() >= 5) // Precisa de 5 partes: % graphic x y z
            var valor_gatilho := CInt(parts[1]);
            
            if (valor_gatilho <= estagio_atual_num)
                var graphic    := CInt(parts[2]);
                var x_offset   := CInt(parts[3]);
                var y_offset   := CInt(parts[4]);
                var z_offset   := CInt(parts[5]);

                LogDebug(LOG_CATEGORY, "    -> Criando peça: Graphic=0x" + Hex(graphic) + " (Gatilho: " + valor_gatilho + ")");
                var novo_item := CreateItemAtLocation(centro_x + x_offset, centro_y + y_offset, centro_z + z_offset, 0xA202, 1, realm);
                if (novo_item)
                    novo_item.graphic := graphic;
                    SetObjProperty(novo_item, "AndaimePrincipalSerial", andaime_principal.serial);
                    novos_itens_seriais.append(novo_item.serial);
                endif
            endif
        endif
    endforeach

    SetObjProperty(andaime_principal, "PecasVisuais", novos_itens_seriais);
    LogInfo(LOG_CATEGORY, "Atualização visual concluída. " + novos_itens_seriais.size() + " peças criadas.");
    LogInfo(LOG_CATEGORY, "==================================================");
endfunction


/*
 * GetStageNumber(andaime_principal) - VERSÃO FINAL
 * Retorna um número único que representa o estágio visual atual.
 */
function GetStageNumber(andaime_principal)
    var materiais_status := GetObjProperty(andaime_principal, "MateriaisStatus");
    
    var total_necessario := 1; // Evita divisão por zero
    var total_atual := 0;
    foreach mat in (materiais_status.keys())
        total_necessario += materiais_status[mat].necessario;
        total_atual += materiais_status[mat].atual;
    endforeach
    var progresso_mat := CInt((total_atual * 100) / total_necessario);

    var timer_info := GetObjProperty(andaime_principal, "ConstructionTimer");
    if (timer_info)
        // Se o timer começou, o progresso é baseado no tempo
        var tempo_passado := polcore().systime - timer_info.start_time;
        var progresso_tempo := CInt((tempo_passado * 100) / timer_info.duration);
        
        // Mapeia 0-100% de tempo para 100-199
        return 100 + progresso_tempo;
    else
        // Se não tem timer, o progresso é baseado nos materiais
        return progresso_mat;
    endif
endfunction
/*
 * FEUDO_CompleteConstruction(andaime_principal) - VERSÃO FINAL
 *
 * Finaliza uma construção, substituindo o andaime pela estrutura completa
 * definida no 'Blueprint' do arquivo de configuração.
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

    // Etapa 2: Limpar todos os itens visuais do andaime (peças decorativas)
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

    // Etapa 3: Obter a posição central e destruir o item "cérebro" do andaime
    var x := andaime_principal.x;
    var y := andaime_principal.y;
    var z := andaime_principal.z;
    var realm := andaime_principal.realm;
    DestroyItem(andaime_principal);

    // Etapa 4: Criar a construção final a partir do Blueprint
    var cfg := FEUDO_GetBuildingCfg();
    var elem := FindConfigElem(cfg, build_info.type);
    var blueprint_lines := GetConfigStringArray(elem, "Blueprint");
    
    if (blueprint_lines.errortext || blueprint_lines.size() == 0)
        LogWarning("FeudoComplete", "Nenhum 'Blueprint' definido para '" + build_info.type + "'. Nenhum item físico será criado.");
    else
        LogInfo("FeudoComplete", "Construindo '" + details.Name + "' a partir de um blueprint com " + blueprint_lines.size() + " peças.");
        
        var pecas_da_construcao := array{};

        foreach piece_line in blueprint_lines
            var parts := SplitWords(piece_line);
            if (parts.size() >= 4)
                var graphic    := CInt(parts[1]);
                var x_offset   := CInt(parts[2]);
                var y_offset   := CInt(parts[3]);
                var z_offset   := CInt(parts[4]);

                var peca := CreateItemAtLocation(x + x_offset, y + y_offset, z + z_offset, graphic, 1, realm);
                if (peca)
                    peca.movable := 0;
                    SetObjProperty(peca, "GuildOwner", guild_id);
                    SetObjProperty(peca, "BuildingType", build_info.type);
                    pecas_da_construcao.append(peca.serial);
                else
                    LogError("FeudoComplete", "Falha ao criar peça do blueprint: " + piece_line);
                endif
            endif
            sleepms(5);
        endforeach

        // Cria um "item mestre" invisível que representa a casa toda
        var mestre_da_construcao := CreateItemAtLocation(x, y, z, 0xA202, 1, realm);
        if (mestre_da_construcao)
            mestre_da_construcao.name := "Controlador Mestre: " + details.Name;
            mestre_da_construcao.invisible := 1;
            SetObjProperty(mestre_da_construcao, "GuildOwner", guild_id);
            SetObjProperty(mestre_da_construcao, "BuildingInfo", build_info);
            SetObjProperty(mestre_da_construcao, "BuildingPieces", pecas_da_construcao);
            LogInfo("FeudoComplete", "Controlador Mestre criado com " + pecas_da_construcao.size() + " peças vinculadas.");
        endif
    endif

    // Etapa 5: Registrar a construção como concluída na CProp da guilda
    var feudo_buildings := guild.GetProp("feudo_buildings") ?: dictionary{};
    feudo_buildings[build_info.type] := build_info.level;
    guild.SetProp("feudo_buildings", feudo_buildings);
    LogInfo("FeudoComplete", "Construção registrada com sucesso para a guilda. Bônus agora estão ativos.");
    
    // Etapa 6: Notificar o líder da guilda, se estiver online
    var leader := SystemFindObjectBySerial(guild.leader);
    if (leader && leader.connected)
        SendSysMessage(leader, "A construção de '" + details.Name + "' em seu feudo foi concluída!");
    endif
    
    return 1;
endfunction