function handle_conversation_history_quester_esqueleto(sourceDescription)
    // Inicializar ou obter a variável global last_char_description
    var last_char_description_esqueleto_quester := GetGlobalProperty("last_char_description_esqueleto_quester");
    if (!last_char_description_esqueleto_quester)
        last_char_description_esqueleto_quester := "";
        SetGlobalProperty("last_char_description_esqueleto_quester", last_char_description_esqueleto_quester);
    endif

    // Inicializar ou obter a variável global conversation_history
    var conversation_history_esqueleto_quester := GetGlobalProperty("conversation_history_esqueleto_quester");
    if (!conversation_history_esqueleto_quester)
        conversation_history_esqueleto_quester := array{};
        SetGlobalProperty("conversation_history_esqueleto_quester", conversation_history_esqueleto_quester);
    endif

    if (sourceDescription != last_char_description_esqueleto_quester || sourceDescription == "error{ errortext = 'Subscript out of range'")
        // Novo personagem, reinicia o histórico de conversas
        conversation_history_esqueleto_quester := array{};
        last_char_description_esqueleto_quester := sourceDescription;  // Atualiza para a nova descrição
        SetGlobalProperty("last_char_description_esqueleto_quester", last_char_description_esqueleto_quester);
        SetGlobalProperty("conversation_history_esqueleto_quester", conversation_history_esqueleto_quester); // Save updated history
    endif

    return conversation_history_esqueleto_quester;
endfunction