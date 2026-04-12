// ============================================================================
// merchantRaceInit.inc
// Configuração aleatória de raça, gênero e nome para mercantes.
// REFACTORED: Calls racegump.inc functions (DRY Principle / GRASP Expert)
// ============================================================================

include ":faccao:include/racegump";

function MerchantSetupRace(npc)
    // Array of valid race button values (1-12 from racegump.inc)
    var race_buttons := array{
        FACCAO_BTN_HUMANO_M,      // 1
        FACCAO_BTN_HUMANO_F,      // 2
        FACCAO_BTN_ANAO_M,        // 3
        FACCAO_BTN_ANAO_F,        // 4
        FACCAO_BTN_ELFO_M,        // 5
        FACCAO_BTN_ELFO_F,        // 6
        FACCAO_BTN_DROW_M,        // 7
        FACCAO_BTN_DROW_F,        // 8
        FACCAO_BTN_ORC_M,         // 9
        FACCAO_BTN_ORC_F,         // 10
        FACCAO_BTN_PEQUILHO_M,    // 11
        FACCAO_BTN_PEQUILHO_F     // 12
    };
    
    var chosen_btn := race_buttons[RandomInt(race_buttons.size()) + 1];
    var namelist := GetRaceNameList(chosen_btn);
    var graphic := GetRaceGraphic(chosen_btn);
    
    // Set gender based on button (odd = male, even = female)
    npc.gender := (chosen_btn % 2 == 1) ? 0 : 1;
    npc.graphic := graphic;
    
    // Setup name from config
    var result := NPC_SetupName(npc, namelist);
    
    // Apply full race appearance (skin, hair, beard)
    ApplyRaceToNPC(npc, chosen_btn);
    
    return result;
endfunction
