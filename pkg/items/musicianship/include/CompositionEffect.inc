// ============================================================================
// CompositionEffect.inc
// Applies bard composition effects based on scale type analysis.
// Phase 3 - Bard Rework "Tecelao da Realidade"
//
// Requires (provided by bardmusic.inc before this include):
//   BARD_FONT, BARD_FONT_COLOR_*, ALVO_BUFF_PARTY, ALVO_DEBUFF_TODOS
//   AplicaEfeito(), AP_GetSkill(), ARCANA, TemHabilidade()
//   SCALE_* constants from CompositionAnalyzer.inc
// ============================================================================

// Returns 1 if the bard has the required ability for this scale type
function CE_CanUseScale(who, scale_type)
    if (!MobileWrapper(who).HasAbility(ABILITY_ID::VOX_ARCANA))
        return 0;
    endif
    case (scale_type)
        SCALE_MAJOR:
            return 1;
        SCALE_MINOR:
            return MobileWrapper(who).HasAbility(ABILITY_ID::RESONANCE);
        SCALE_CHROMATIC:
            return MobileWrapper(who).HasAbility(ABILITY_ID::REALITY_TEAR);
        SCALE_MIXED:
            return MobileWrapper(who).HasAbility(ABILITY_ID::FINAL_SYMPHONY);
        default:
            return 1;
    endcase
endfunction

// Main entry point: apply composition effect from a played partitura.
// Reads scale_type and comp_power props from the partitura item.
// Returns 1 on success, 0 if not a V3 composition or validation failed.
function CE_ApplyCompositionEffect(who, partitura, instrumento)
    var scale_type := GetObjProperty(partitura, "scale_type");
    var power      := CInt(GetObjProperty(partitura, "comp_power"));

    if (!scale_type)
        return 0; // Not a V3 composition, caller falls back to V1 path
    endif

    if (!CE_CanUseScale(who, scale_type))
        case (scale_type)
            SCALE_MINOR:
                SendSysMessage(who, "Voce precisa de Ressonancia para tocar escalas menores.", BARD_FONT, BARD_FONT_COLOR_BAD);
            SCALE_CHROMATIC:
                SendSysMessage(who, "Voce precisa de Tear da Realidade para tocar escalas cromaticas.", BARD_FONT, BARD_FONT_COLOR_BAD);
            SCALE_MIXED:
                SendSysMessage(who, "Voce precisa de Sinfonia Final para misturar escalas.", BARD_FONT, BARD_FONT_COLOR_BAD);
            default:
                SendSysMessage(who, "Voce nao possui a habilidade bardica necessaria.", BARD_FONT, BARD_FONT_COLOR_BAD);
        endcase
        return 0;
    endif

    // Arcana-based scaling
    var arcana   := CInt(AP_GetSkill(who, ARCANA) / 10);
    var range    := CInt(arcana / 20);
    if (range < 1) range := 1; endif
    if (range > 5) range := 5; endif
    var duration := CInt(arcana / 3);
    if (duration < 3) duration := 3; endif
    if (!power || power < 1) power := CInt(arcana / 5); endif

    // Sinfonia Final (active): double effect once
    if (MobileWrapper(who).HasAbility(ABILITY_ID::FINAL_SYMPHONY))
        if (GetObjProperty(who, "#bard_final_symphony_active"))
            power    := power * 2;
            duration := duration * 2;
            SetObjProperty(who, "#bard_final_symphony_active", 0);
            SendSysMessage(who, "A Sinfonia Final amplifica sua composicao!", BARD_FONT, BARD_FONT_COLOR_GOOD);
        endif
    endif

    case (scale_type)
        SCALE_MAJOR:
            _CE_ApplyMajorEffect(who, power, duration, range);
        SCALE_MINOR:
            _CE_ApplyMinorEffect(who, power, duration, range);
        SCALE_CHROMATIC:
            _CE_ApplyChromaticEffect(who, power, duration, range);
        default: // SCALE_MIXED
            _CE_ApplyMixedEffect(who, power, duration, range);
    endcase

    // Phase 5: Wild Mage entropy check
    BEC_CheckEntropy(who, scale_type, power);

    return 1;
endfunction

function _CE_ApplyMajorEffect(who, power, duration, range)
    SendSysMessage(who, "*A melodia maior envolve seus aliados em harmonia*", BARD_FONT, BARD_FONT_COLOR_GOOD);
    // bard_inspirar_heroismo divides modifier by 5 internally, so pass power*5
    AplicaEfeito(who, "bard_inspirar_heroismo", duration, power * 5, range,
                 ALVO_BUFF_PARTY, "*Voce se sente fortalecido pela melodia*", 0);
endfunction

function _CE_ApplyMinorEffect(who, power, duration, range)
    SendSysMessage(who, "*A melodia menor desceu sobre seus inimigos como sombras*", BARD_FONT, BARD_FONT_COLOR_BAD);
    AplicaEfeito(who, "bard_dex_down", duration, power, range,
                 ALVO_DEBUFF_TODOS, "*A melodia drena sua forca*", 0);
endfunction

function _CE_ApplyChromaticEffect(who, power, duration, range)
    SendSysMessage(who, "*As notas cromaticas distorcem a realidade!*", BARD_FONT, 88);
    var roll := RandomInt(2);
    if (roll == 0)
        AplicaEfeito(who, "bard_inspirar_heroismo", duration, power * 10, range,
                     ALVO_BUFF_PARTY, "*O caos sorri para voce!*", 0);
    else
        AplicaEfeito(who, "bard_dex_down", duration, power * 2, range,
                     ALVO_DEBUFF_TODOS, "*O caos ri dos seus inimigos!*", 0);
    endif
endfunction

function _CE_ApplyMixedEffect(who, power, duration, range)
    SendSysMessage(who, "*A melodia mista tece realidades opostas*", BARD_FONT, BARD_FONT_COLOR_NEUTRA);
    var half_dur := CInt(duration / 2);
    AplicaEfeito(who, "bard_str_up", half_dur, power, range,
                 ALVO_BUFF_PARTY, "*Seus aliados se fortalecem*", 0);
    AplicaEfeito(who, "bard_dex_down", half_dur, power, range,
                 ALVO_DEBUFF_TODOS, "*Seus inimigos enfraquecem*", 0);
endfunction