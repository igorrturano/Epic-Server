// fogRegionGump.inc — Gumps GM do sistema fog regions.
// Include path: :fog_regions:gumps/fogRegionGump

include ":fog_regions:include/fogRegionCore";

// ── Constantes de layout ──────────────────────────────────────────────────────
const FR_GUMP_W  := 320;
const FR_GUMP_H  := 420;
const FR_CONT_Y  := 75;
const FR_ROW_H   := 44;
const FR_BTN_W   := 240;
const FR_BTN_X   := 40;
const FR_COLOR_TITLE := 53;
const FR_COLOR_BODY  := 90;
const FR_COLOR_WARN  := 33;
const FR_COLOR_DESC  := 1153;

// Botões de navegação largos: 2103/2104 (off/on)
const FR_BTN_UP   := 2103;
const FR_BTN_DN   := 2104;

// Botões compactos ◀▶: 2117/2118 (off/on)
const FR_BTN_SMALL_UP := 2117;
const FR_BTN_SMALL_DN := 2118;

// IDs de botão — Tela 1
const FR_BTN_CRIAR   := 1;
const FR_BTN_LISTAR  := 2;
const FR_BTN_ONDE    := 3;

// IDs de botão — Wizard (Telas 2-4)
const FR_BTN_WIZARD_VOLTAR  := 10;
const FR_BTN_WIZARD_PROXIMO := 11;
const FR_BTN_WIZARD_PRESET  := 14;
const FR_BTN_WIZARD_MANUAL  := 15;

// IDs de botão — Tela 5a (presets)
const FR_BTN_WIZARD_CRIAR    := 20;
const FR_BTN_PRESETS_MIN_DEC := 21;
const FR_BTN_PRESETS_MIN_INC := 22;

// Layout — Tela 5a
const FR_PRESET_ROW_H := 30;

// Grupos e gráficos de radio button
const FR_RADIO_GROUP_PRESET  := 1;
const FR_RADIO_GROUP_DURACAO := 2;
const FR_RADIO_PRESET_BASE   := 100;   // radio IDs: 101-106
const FR_RADIO_DURACAO_PERM  := 201;
const FR_RADIO_DURACAO_TIMED := 202;
const FR_RADIO_OFF := 208;
const FR_RADIO_ON  := 209;

// ObjProperty de estado do wizard
const FR_EDITOR_STATE_PROP := "#fog_editor_state";

const FR_BTN_ED_DENSITY_DEC := 30;
const FR_BTN_ED_DENSITY_INC := 31;
const FR_BTN_ED_RADIUS_DEC  := 32;
const FR_BTN_ED_RADIUS_INC  := 33;
const FR_BTN_ED_FALLOFF_DEC := 34;
const FR_BTN_ED_FALLOFF_INC := 35;
const FR_BTN_ED_VMIN_DEC    := 36;
const FR_BTN_ED_VMIN_INC    := 37;
const FR_BTN_ED_VMAX_DEC    := 38;
const FR_BTN_ED_VMAX_INC    := 39;
const FR_BTN_ED_NOISE_DEC   := 40;
const FR_BTN_ED_NOISE_INC   := 41;
const FR_BTN_ED_SCALE_DEC   := 42;
const FR_BTN_ED_SCALE_INC   := 43;
const FR_BTN_ED_SPEED_DEC   := 44;
const FR_BTN_ED_SPEED_INC   := 45;
const FR_BTN_ED_DARK_DEC    := 46;
const FR_BTN_ED_DARK_INC    := 47;
const FR_BTN_ED_DESAT_DEC   := 48;
const FR_BTN_ED_DESAT_INC   := 49;
const FR_BTN_ED_COLOR_R_DEC := 50;
const FR_BTN_ED_COLOR_R_INC := 51;
const FR_BTN_ED_COLOR_G_DEC := 52;
const FR_BTN_ED_COLOR_G_INC := 53;
const FR_BTN_ED_COLOR_B_DEC := 54;
const FR_BTN_ED_COLOR_B_INC := 55;
const FR_BTN_ED_DURACAO_DEC := 56;
const FR_BTN_ED_DURACAO_INC := 57;

const FR_BTN_ED_TESTAR      := 60;
const FR_BTN_ED_CRIAR       := 61;

const FR_BTN_ED_HELP_DENSITY := 70;
const FR_BTN_ED_HELP_RADIUS  := 71;
const FR_BTN_ED_HELP_FALLOFF := 72;
const FR_BTN_ED_HELP_VMIN    := 73;
const FR_BTN_ED_HELP_VMAX    := 74;
const FR_BTN_ED_HELP_NOISE   := 75;
const FR_BTN_ED_HELP_SCALE   := 76;
const FR_BTN_ED_HELP_SPEED   := 77;
const FR_BTN_ED_HELP_DARK    := 78;
const FR_BTN_ED_HELP_DESAT   := 79;
const FR_BTN_ED_HELP_COLOR   := 80;
// FOG-FASE5/F: controles de altura Z de mundo
const FR_BTN_ED_ZMIN_DEC  := 81;
const FR_BTN_ED_ZMIN_INC  := 82;
const FR_BTN_ED_ZMAX_DEC  := 83;
const FR_BTN_ED_ZMAX_INC  := 84;
const FR_BTN_ED_HELP_ZMIN := 85;
const FR_BTN_ED_HELP_ZMAX := 86;
// FOG-FASE6/G: controle de suavidade da borda Z
const FR_BTN_ED_ZFADE_DEC  := 87;
const FR_BTN_ED_ZFADE_INC  := 88;
const FR_BTN_ED_HELP_ZFADE := 89;

// FOG-FASE5: Steps de incremento/decremento por variável
const FR_STEP_DENSITY := 100;
const FR_STEP_RADIUS  := 10;
const FR_STEP_FALLOFF := 5;
const FR_STEP_VMIN    := 50;
const FR_STEP_VMAX    := 50;
const FR_STEP_NOISE   := 50;
const FR_STEP_SCALE   := 100;
const FR_STEP_SPEED   := 100;
const FR_STEP_DARK    := 50;
const FR_STEP_DESAT   := 50;
const FR_STEP_COLOR   := 13;
const FR_STEP_Z       := 5; // FOG-FASE5/F

// Limites min/max por variável
const FR_MIN_DENSITY := 100;
const FR_MAX_DENSITY := 5000;
const FR_MIN_RADIUS  := 50;
const FR_MAX_RADIUS  := 2000;
const FR_MIN_FALLOFF := 10;
const FR_MAX_FALLOFF := 500;
const FR_MIN_VMIN    := 0;
const FR_MAX_VMIN    := 2500;
const FR_MIN_VMAX    := 0;
const FR_MAX_VMAX    := 2500;
const FR_MIN_NOISE   := 0;
const FR_MAX_NOISE   := 1000;
const FR_MIN_SCALE   := 300;
const FR_MAX_SCALE   := 5000;
const FR_MIN_SPEED   := 0;
const FR_MAX_SPEED   := 10000;
const FR_MIN_DARK    := 0;
const FR_MAX_DARK    := 1000;
const FR_MIN_DESAT   := 0;
const FR_MAX_DESAT   := 1000;
const FR_MIN_COLOR   := 0;
const FR_MAX_COLOR   := 255;
const FR_MIN_DURACAO := 1;
const FR_MAX_DURACAO := 240;
const FR_MIN_Z       := -128; // FOG-FASE5/F
const FR_MAX_Z       := 127;  // FOG-FASE5/F
const FR_STEP_ZFADE  := 5;    // FOG-FASE6/G
const FR_MIN_ZFADE   := 1;    // FOG-FASE6/G
const FR_MAX_ZFADE   := 60;   // FOG-FASE6/G

// Layout Tooltips
const FR_TOOLTIP_W := 320;
const FR_TOOLTIP_H := 340;

const FR_RESIZEPIC_GFX := 9350;

// Layout Editor Manual
const FR_ED_GUMP_H  := 620;
const FR_ED_LABEL_X := 40;
const FR_ED_DEC_X   := 110;
const FR_ED_VAL_X   := 128;
const FR_ED_INC_X   := 170;
const FR_ED_HELP_X  := 265;
const FR_ED_ROW_H   := 22;

const FR_COLOR_STATUS_ACTIVE_PERM  := 68;
const FR_COLOR_STATUS_ACTIVE_TIMED := 53;
const FR_COLOR_STATUS_STOPPED      := 230;

const FR_BTN_LIST_PREV        := 200;
const FR_BTN_LIST_NEXT        := 201;
const FR_BTN_LIST_EDIT_1      := 210;
const FR_BTN_LIST_EDIT_2      := 211;
const FR_BTN_LIST_EDIT_3      := 212;
const FR_BTN_LIST_IR_1        := 213;
const FR_BTN_LIST_IR_2        := 214;
const FR_BTN_LIST_IR_3        := 215;
const FR_BTN_LIST_TOGGLE_1    := 216;
const FR_BTN_LIST_TOGGLE_2    := 217;
const FR_BTN_LIST_TOGGLE_3    := 218;
const FR_BTN_LIST_DEL_1       := 219;
const FR_BTN_LIST_DEL_2       := 220;
const FR_BTN_LIST_DEL_3       := 221;
const FR_BTN_LIST_FECHAR      := 230;
const FR_BTN_LIST_DEL_CONFIRM := 231;
const FR_BTN_LIST_DEL_CANCEL  := 232;

const FR_BTN_WAI_CRIAR   := 300;
const FR_BTN_WAI_FECHAR  := 301;


// ── Estado do wizard ─────────────────────────────────────────────────────────

function GetEditorState(who)
    var state := GetObjProperty(who, FR_EDITOR_STATE_PROP);
    if (!state || state.errortext)
        state := struct;
        state.+nome               := "";
        state.+nw_x               := -1;
        state.+nw_y               := -1;
        state.+nw_realm           := "";
        state.+se_x               := -1;
        state.+se_y               := -1;
        state.+se_realm           := "";
        state.+erro               := "";
        state.+preset_selecionado := "";
        state.+duracao_mode       := FOG_REGION_MODE_PERMANENT;
        state.+duracao_valor      := 30;
        state.+density_int        := 80;
        state.+radius             := 200;
        state.+falloff            := 100;
        state.+vmin_int           := 300;
        state.+vmax_int           := 1200;
        state.+noise_int          := 500;
        state.+scale_int          := 1000;
        state.+speed_int          := 500;
        state.+dark_int           := 300;
        state.+desat_int          := 0;
        state.+color_r            := 128;
        state.+color_g            := 133;
        state.+color_b            := 148;
        state.+world_z_min        := -128; // FOG-FASE5/F
        state.+world_z_max        := 127;  // FOG-FASE5/F
        state.+world_z_fade       := 10;   // FOG-FASE6/G
        state.+editing            := "";
    endif
    if (state.editing.errortext)
        state.+editing := "";
    endif
    return state;
endfunction

function SetEditorState(who, state)
    SetObjProperty(who, FR_EDITOR_STATE_PROP, state);
endfunction

function ClearEditorState(who)
    EraseObjProperty(who, FR_EDITOR_STATE_PROP);
endfunction

// Lê do result array qual preset radio (101-106) está selecionado.
// Retorna nome do preset ou "" se nenhum.
function _ReadPresetSelection(result)
    var names := array{ "atmosfera_padrao", "noite_profunda", "bosque_verde_musgo",
                        "sepia_arido", "vinheta_envolvente", "vale_enevoado" };
    for i := 1 to 6
        var val := result[FR_RADIO_PRESET_BASE + i];
        if (val != error && val)
            return names[i];
        endif
    endfor
    return "";
endfunction

// Formata int escalado para display de float (máx 3 casas decimais).
// Evita notação científica (ex: "8e-04") convertendo via inteiro de 6 decimais.
function _FmtScaled(int_value, scale_factor)
    if (int_value == 0)
        return "0";
    endif
    var div := CDbl(int_value) / scale_factor;
    var s := CStr(div);
    if (find(Lower(s), "e", 1) > 0)
        var big := CInt(div * 1000000 + 0.5);
        s := CStr(big);
        while (Len(s) < 7)
            s := "0" + s;
        endwhile
        var int_part  := SubStr(s, 1, Len(s) - 6);
        var frac_part := SubStr(s, Len(s) - 5, 6);
        while (Len(frac_part) > 1 && SubStr(frac_part, Len(frac_part), 1) == "0")
            frac_part := SubStr(frac_part, 1, Len(frac_part) - 1);
        endwhile
        return int_part + "." + frac_part;
    endif
    var dot_pos := find(s, ".", 1);
    if (dot_pos > 0 && Len(s) > dot_pos + 3)
        s := SubStr(s, 1, dot_pos + 3);
    endif
    return s;
endfunction

// Conversão RGB→HSV→hue UO para preview de cor no gump.
// 17 valores: 14 matizes (roda de cores em 16 buckets de ~22°) + preto/cinza/branco.
// IDs 38/48/58/75/1031/1038 são interpolados — calibrar in-game se necessário.
function _RgbToUoHue(r, g, b)
    var mx := r; if (g > mx) mx := g; endif; if (b > mx) mx := b; endif
    var mn := r; if (g < mn) mn := g; endif; if (b < mn) mn := b; endif
    var delta := mx - mn;

    if (delta < 35)
        if (mx < 60)  return 1;    endif  // preto
        if (mx > 190) return 1153; endif  // branco
        return 1154;                       // cinza
    endif

    var h;
    if (mx == r)
        h := CInt((g - b) * 60 / delta);
        if (h < 0) h := h + 360; endif
    elseif (mx == g)
        h := CInt((b - r) * 60 / delta) + 120;
    else
        h := CInt((r - g) * 60 / delta) + 240;
    endif

    if (h < 12)  return 33;   endif  // vermelho
    if (h < 35)  return 38;   endif  // vermelho-laranja
    if (h < 52)  return 43;   endif  // laranja
    if (h < 65)  return 48;   endif  // âmbar
    if (h < 80)  return 53;   endif  // amarelo
    if (h < 100) return 58;   endif  // amarelo-lima
    if (h < 140) return 68;   endif  // verde
    if (h < 165) return 75;   endif  // verde-teal
    if (h < 195) return 88;   endif  // ciano
    if (h < 220) return 90;   endif  // azul-ciano
    if (h < 255) return 1024; endif  // azul
    if (h < 285) return 1031; endif  // índigo
    if (h < 315) return 1038; endif  // violeta
    if (h < 345) return 1645; endif  // magenta
    return 33;
endfunction

function _ReadEdRadios(result, byref state)
    var perm_v := result[FR_RADIO_DURACAO_PERM];
    if (perm_v != error && perm_v)   state.duracao_mode := FOG_REGION_MODE_PERMANENT; endif
    var timed_v := result[FR_RADIO_DURACAO_TIMED];
    if (timed_v != error && timed_v) state.duracao_mode := FOG_REGION_MODE_TIMED; endif
endfunction

// `step` é palavra reservada — parâmetro nomeado `amt`.
function _Step(val, amt, mn, mx, inc)
    if (inc)
        if (val + amt <= mx) return val + amt; endif
    else
        if (val - amt >= mn) return val - amt; endif
    endif
    return val;
endfunction

function _ApplyEditorStep(byref state, btn)
    if      (btn == FR_BTN_ED_DENSITY_DEC || btn == FR_BTN_ED_DENSITY_INC) state.density_int   := _Step(state.density_int,   FR_STEP_DENSITY, FR_MIN_DENSITY, FR_MAX_DENSITY, btn == FR_BTN_ED_DENSITY_INC);
    elseif  (btn == FR_BTN_ED_RADIUS_DEC  || btn == FR_BTN_ED_RADIUS_INC)  state.radius        := _Step(state.radius,        FR_STEP_RADIUS,  FR_MIN_RADIUS,  FR_MAX_RADIUS,  btn == FR_BTN_ED_RADIUS_INC);
    elseif  (btn == FR_BTN_ED_FALLOFF_DEC || btn == FR_BTN_ED_FALLOFF_INC) state.falloff       := _Step(state.falloff,       FR_STEP_FALLOFF, FR_MIN_FALLOFF, FR_MAX_FALLOFF, btn == FR_BTN_ED_FALLOFF_INC);
    elseif  (btn == FR_BTN_ED_VMIN_DEC    || btn == FR_BTN_ED_VMIN_INC)    state.vmin_int      := _Step(state.vmin_int,      FR_STEP_VMIN,    FR_MIN_VMIN,    FR_MAX_VMIN,    btn == FR_BTN_ED_VMIN_INC);
    elseif  (btn == FR_BTN_ED_VMAX_DEC    || btn == FR_BTN_ED_VMAX_INC)    state.vmax_int      := _Step(state.vmax_int,      FR_STEP_VMAX,    FR_MIN_VMAX,    FR_MAX_VMAX,    btn == FR_BTN_ED_VMAX_INC);
    elseif  (btn == FR_BTN_ED_NOISE_DEC   || btn == FR_BTN_ED_NOISE_INC)   state.noise_int     := _Step(state.noise_int,     FR_STEP_NOISE,   FR_MIN_NOISE,   FR_MAX_NOISE,   btn == FR_BTN_ED_NOISE_INC);
    elseif  (btn == FR_BTN_ED_SCALE_DEC   || btn == FR_BTN_ED_SCALE_INC)   state.scale_int     := _Step(state.scale_int,     FR_STEP_SCALE,   FR_MIN_SCALE,   FR_MAX_SCALE,   btn == FR_BTN_ED_SCALE_INC);
    elseif  (btn == FR_BTN_ED_SPEED_DEC   || btn == FR_BTN_ED_SPEED_INC)   state.speed_int     := _Step(state.speed_int,     FR_STEP_SPEED,   FR_MIN_SPEED,   FR_MAX_SPEED,   btn == FR_BTN_ED_SPEED_INC);
    elseif  (btn == FR_BTN_ED_DARK_DEC    || btn == FR_BTN_ED_DARK_INC)    state.dark_int      := _Step(state.dark_int,      FR_STEP_DARK,    FR_MIN_DARK,    FR_MAX_DARK,    btn == FR_BTN_ED_DARK_INC);
    elseif  (btn == FR_BTN_ED_DESAT_DEC   || btn == FR_BTN_ED_DESAT_INC)   state.desat_int     := _Step(state.desat_int,     FR_STEP_DESAT,   FR_MIN_DESAT,   FR_MAX_DESAT,   btn == FR_BTN_ED_DESAT_INC);
    elseif  (btn == FR_BTN_ED_COLOR_R_DEC || btn == FR_BTN_ED_COLOR_R_INC) state.color_r       := _Step(state.color_r,       FR_STEP_COLOR,   FR_MIN_COLOR,   FR_MAX_COLOR,   btn == FR_BTN_ED_COLOR_R_INC);
    elseif  (btn == FR_BTN_ED_COLOR_G_DEC || btn == FR_BTN_ED_COLOR_G_INC) state.color_g       := _Step(state.color_g,       FR_STEP_COLOR,   FR_MIN_COLOR,   FR_MAX_COLOR,   btn == FR_BTN_ED_COLOR_G_INC);
    elseif  (btn == FR_BTN_ED_COLOR_B_DEC || btn == FR_BTN_ED_COLOR_B_INC) state.color_b       := _Step(state.color_b,       FR_STEP_COLOR,   FR_MIN_COLOR,   FR_MAX_COLOR,   btn == FR_BTN_ED_COLOR_B_INC);
    elseif  (btn == FR_BTN_ED_DURACAO_DEC || btn == FR_BTN_ED_DURACAO_INC) state.duracao_valor := _Step(state.duracao_valor, 1,               FR_MIN_DURACAO, FR_MAX_DURACAO, btn == FR_BTN_ED_DURACAO_INC);
    elseif  (btn == FR_BTN_ED_ZMIN_DEC   || btn == FR_BTN_ED_ZMIN_INC)   state.world_z_min   := _Step(state.world_z_min,   FR_STEP_Z,       FR_MIN_Z,       FR_MAX_Z,       btn == FR_BTN_ED_ZMIN_INC);  // FOG-FASE5/F
    elseif  (btn == FR_BTN_ED_ZMAX_DEC   || btn == FR_BTN_ED_ZMAX_INC)   state.world_z_max   := _Step(state.world_z_max,   FR_STEP_Z,       FR_MIN_Z,       FR_MAX_Z,       btn == FR_BTN_ED_ZMAX_INC);  // FOG-FASE5/F
    elseif  (btn == FR_BTN_ED_ZFADE_DEC  || btn == FR_BTN_ED_ZFADE_INC)  state.world_z_fade  := _Step(state.world_z_fade,  FR_STEP_ZFADE,   FR_MIN_ZFADE,   FR_MAX_ZFADE,   btn == FR_BTN_ED_ZFADE_INC); // FOG-FASE6/G
    endif
endfunction

// Lê preset radio + duração do result e atualiza state (unifica o padrão duplicado em Presets).
function _ReadPresetRadios(result, byref state)
    var sel := _ReadPresetSelection(result);
    if (Len(sel) > 0) state.preset_selecionado := sel; endif
    _ReadEdRadios(result, state);
endfunction

// Constrói struct de região a partir do estado do editor. Único ponto de montagem do esquema.
function _StateToRegionStruct(state, preset_name)
    var region := struct;
    region.+Name       := state.nome;
    region.+X1         := state.nw_x;
    region.+Y1         := state.nw_y;
    region.+X2         := state.se_x;
    region.+Y2         := state.se_y;
    region.+Realm      := state.nw_realm;
    region.+Status     := FOG_REGION_STATUS_ACTIVE;
    region.+Mode       := state.duracao_mode;
    region.+DensityInt := state.density_int;
    region.+Radius     := state.radius;
    region.+Falloff    := state.falloff;
    region.+VMinInt    := state.vmin_int;
    region.+VMaxInt    := state.vmax_int;
    region.+NoiseInt   := state.noise_int;
    region.+ScaleInt   := state.scale_int;
    region.+SpeedInt   := state.speed_int;
    region.+DarkInt    := state.dark_int;
    region.+ColorR     := state.color_r;
    region.+ColorG     := state.color_g;
    region.+ColorB     := state.color_b;
    region.+DesatInt   := state.desat_int;
    region.+WorldZMin  := state.world_z_min;  // FOG-FASE5/F
    region.+WorldZMax  := state.world_z_max;  // FOG-FASE5/F
    region.+FalloffZ   := state.world_z_fade; // FOG-FASE6/G
    region.+PresetName := preset_name;
    if (state.duracao_mode == FOG_REGION_MODE_TIMED)
        region.+TimerEnd := polcore().systime + (state.duracao_valor * 60);
    else
        region.+TimerEnd := 0;
    endif
    return region;
endfunction

// Inicia fogRegionEffect somente se a região for timed.
function _StartEffectIfTimed(region)
    if (region.Mode == FOG_REGION_MODE_TIMED)
        start_script(":fog_regions:fogRegionEffect", region.Name);
    endif
endfunction

// Retorna struct com os 13 parâmetros de fog escalados para o preset solicitado.
function _GetPreset(name)
    // FOG-FASE5: valores escalados conforme encoding do packet 0x0070 v4
    var p := struct;
    if (name == "atmosfera_padrao")
        p.+DensityInt := 80;   p.+Radius := 200;  p.+Falloff := 100;
        p.+VMinInt := 300;     p.+VMaxInt := 1200; p.+NoiseInt := 500;
        p.+ScaleInt := 1000;   p.+SpeedInt := 500; p.+DarkInt := 300;
        p.+ColorR := 128;      p.+ColorG := 133;   p.+ColorB := 148;  p.+DesatInt := 0;
    elseif (name == "noite_profunda")
        p.+DensityInt := 500;
        p.+Radius     := 50;
        p.+Falloff    := 10;
        p.+VMinInt    := 950;
        p.+VMaxInt    := 1050;
        p.+NoiseInt   := 50;
        p.+ScaleInt   := 1000;
        p.+SpeedInt   := 500;
        p.+DarkInt    := 0;
        p.+ColorR     := 51;
        p.+ColorG     := 51;
        p.+ColorB     := 51;
        p.+DesatInt   := 1000;
    elseif (name == "bosque_verde_musgo")
        p.+DensityInt := 800;
        p.+Radius     := 120;
        p.+Falloff    := 70;
        p.+VMinInt    := 950;
        p.+VMaxInt    := 1050;
        p.+NoiseInt   := 100;
        p.+ScaleInt   := 1000;
        p.+SpeedInt   := 500;
        p.+DarkInt    := 600;
        p.+ColorR     := 122;
        p.+ColorG     := 140;
        p.+ColorB     := 115;
        p.+DesatInt   := 800;
    elseif (name == "sepia_arido")
        p.+DensityInt := 800;
        p.+Radius     := 120;
        p.+Falloff    := 70;
        p.+VMinInt    := 950;
        p.+VMaxInt    := 1050;
        p.+NoiseInt   := 100;
        p.+ScaleInt   := 1000;
        p.+SpeedInt   := 500;
        p.+DarkInt    := 600;
        p.+ColorR     := 166;
        p.+ColorG     := 148;
        p.+ColorB     := 115;
        p.+DesatInt   := 600;
    elseif (name == "vinheta_envolvente")
        p.+DensityInt := 1500;
        p.+Radius     := 500;
        p.+Falloff    := 200;
        p.+VMinInt    := 950;
        p.+VMaxInt    := 1050;
        p.+NoiseInt   := 100;
        p.+ScaleInt   := 1000;
        p.+SpeedInt   := 500;
        p.+DarkInt    := 700;
        p.+ColorR     := 140;
        p.+ColorG     := 140;
        p.+ColorB     := 140;
        p.+DesatInt   := 1000;
    elseif (name == "vale_enevoado")
        p.+DensityInt := 800;
        p.+Radius     := 120;
        p.+Falloff    := 70;
        p.+VMinInt    := 2500;
        p.+VMaxInt    := 0;
        p.+NoiseInt   := 100;
        p.+ScaleInt   := 1000;
        p.+SpeedInt   := 500;
        p.+DarkInt    := 600;
        p.+ColorR     := 140;
        p.+ColorG     := 140;
        p.+ColorB     := 140;
        p.+DesatInt   := 1000;
    endif
    return p;
endfunction


// ── Wizard: Nome da região ────────────────────────────────────────────────────

function _AskFogRegionName(who)
    var erro := "";
    while (1)
        var gump := GFECreateGump("Criar Região", FR_GUMP_W, 200, array{ BORDER_GOLDEN });
        var y    := FR_CONT_Y;

        GFTextLine(gump, FR_BTN_X, y, FR_COLOR_BODY, "Nome da região:");
        y += 18;
        GFEAddTextBox(gump, FR_BTN_X, y, FR_BTN_W - 20, 16, FR_COLOR_BODY, "", 1, 64);
        y += 36;

        if (Len(erro) > 0)
            GFTextLine(gump, FR_BTN_X, y, FR_COLOR_WARN, erro);
            y += 18;
        endif

        GFResizePic(gump, FR_BTN_X, y, FR_RESIZEPIC_GFX, FR_BTN_W, 3);
        y += 14;

        GFAddButton(gump, FR_BTN_X, y, FR_BTN_DN, FR_BTN_UP, 1, 1);
        GFTextLine(gump, FR_BTN_X + 30, y + 4, FR_COLOR_BODY, "Continuar");

        GFAddButton(gump, FR_BTN_X + 140, y, FR_BTN_DN, FR_BTN_UP, 1, 2);
        GFTextLine(gump, FR_BTN_X + 170, y + 4, FR_COLOR_BODY, "Cancelar");

        var result := GFSendGump(who, gump);
        var btn    := result[0];

        if (!btn || !who.connected || btn == 2)
            return "";
        endif

        var nome := result[1];
        if (!nome || Len(Trim(nome)) == 0)
            erro := "Digite um nome para a região.";
        else
            return Trim(nome);
        endif
    endwhile
endfunction


// ── Wizard: Flow de criação (2 cliques no mapa + preview) ────────────────────

function StartFogRegionCreateFlow(who)
    ClearEditorState(who);

    var nome := _AskFogRegionName(who);
    if (!nome || Len(nome) == 0)
        return;
    endif

    SendSysMessage(who, "[FOG] Clique no primeiro canto da região.");
    var pt1 := TargetCoordinates(who);
    if (!pt1)
        SendSysMessage(who, "Cancelado.");
        return;
    endif

    SendSysMessage(who, "[FOG] Clique no segundo canto da região.");
    var pt2 := TargetCoordinates(who);
    if (!pt2)
        SendSysMessage(who, "Cancelado.");
        return;
    endif

    if (pt1.realm != pt2.realm)
        SendSysMessage(who, "[FOG] Os dois pontos devem estar no mesmo realm. Cancelado.");
        return;
    endif

    var x1 := (pt1.x < pt2.x) ? pt1.x : pt2.x;
    var y1  := (pt1.y < pt2.y) ? pt1.y : pt2.y;
    var x2  := (pt1.x > pt2.x) ? pt1.x : pt2.x;
    var y2  := (pt1.y > pt2.y) ? pt1.y : pt2.y;

    SendSysMessage(who, "[FOG] Área: " + (x2 - x1 + 1) + " x " + (y2 - y1 + 1) + " tiles — criando prévia...");
    var markers := _FRPreviewRect(x1, y1, x2, y2, pt1.realm);

    var serials := array{};
    foreach m in markers
        serials.append(m.serial);
    endforeach
    SetObjProperty(who, "#fogPreviewMarkers", serials);

    var state := GetEditorState(who);
    state.nome     := nome;
    state.nw_x     := x1;
    state.nw_y     := y1;
    state.se_x     := x2;
    state.se_y     := y2;
    state.nw_realm := pt1.realm;
    state.se_realm := pt1.realm;
    state.erro     := "";
    SetEditorState(who, state);

    ShowFogRegionCreateStep3(who);
    _FRClearPreview(markers);
    EraseObjProperty(who, "#fogPreviewMarkers");
endfunction


// ── Tela 1: Menu Principal ────────────────────────────────────────────────────

function ShowFogRegionMenu(who)
    while (1)
        var gump := GFECreateGump("Fog Regions", FR_GUMP_W, FR_GUMP_H, array{ BORDER_GOLDEN });
        var y := FR_CONT_Y;

        GFTextLine(gump, FR_BTN_X, y, FR_COLOR_TITLE, "Gerenciamento de Regiões de Névoa");
        y += 28;

        GFAddButton(gump, FR_BTN_X, y, FR_BTN_DN, FR_BTN_UP, 1, FR_BTN_CRIAR);
        GFTextLine(gump, FR_BTN_X + 30, y + 4, FR_COLOR_BODY, "Criar nova região");
        y += FR_ROW_H;

        GFAddButton(gump, FR_BTN_X, y, FR_BTN_DN, FR_BTN_UP, 1, FR_BTN_LISTAR);
        GFTextLine(gump, FR_BTN_X + 30, y + 4, FR_COLOR_BODY, "Listar regiões");
        y += FR_ROW_H;

        GFAddButton(gump, FR_BTN_X, y, FR_BTN_DN, FR_BTN_UP, 1, FR_BTN_ONDE);
        GFTextLine(gump, FR_BTN_X + 30, y + 4, FR_COLOR_BODY, "Onde estou agora?");
        y += FR_ROW_H + 20;

        GFResizePic(gump, FR_BTN_X, y, FR_RESIZEPIC_GFX, FR_BTN_W, 3);
        y += 12;

        GFAddButton(gump, FR_BTN_X, y, FR_BTN_DN, FR_BTN_UP, 1, 0);
        GFTextLine(gump, FR_BTN_X + 30, y + 4, FR_COLOR_BODY, "Fechar");

        var result := GFSendGump(who, gump);
        var btn    := result[0];

        if (!btn)
            return;
        elseif (!who.connected)
            return;
        elseif (btn == FR_BTN_CRIAR)
            StartFogRegionCreateFlow(who);
        elseif (btn == FR_BTN_LISTAR)
            ShowFogRegionList(who, 1);
        elseif (btn == FR_BTN_ONDE)
            ShowFogRegionWhereAmI(who);
            return;
        endif
    endwhile
endfunction


// ── Tela 4: Wizard Passo 3/3 — Escolher fog ──────────────────────────────────
// Retorna 1 se criação concluída com sucesso; 0 se cancelado/Anterior/disconnect.

function ShowFogRegionCreateStep3(who)
    while (1)
        var state := GetEditorState(who);
        var gump  := GFECreateGump("Criar Região (3/3)", FR_GUMP_W, FR_GUMP_H, array{ BORDER_GOLDEN });
        var y     := FR_CONT_Y;

        GFTextLine(gump, FR_BTN_X, y, FR_COLOR_TITLE, "Passo 3 de 3: Configurar névoa");
        y += 24;

        GFTextLine(gump, FR_BTN_X, y, FR_COLOR_BODY, "Região: " + state.nome);
        y += 18;
        GFTextLine(gump, FR_BTN_X, y, FR_COLOR_BODY,
            "NW: " + state.nw_x + ", " + state.nw_y +
            "  SE: " + state.se_x + ", " + state.se_y);
        y += 18;
        GFTextLine(gump, FR_BTN_X, y, FR_COLOR_BODY, "Realm: " + state.nw_realm);
        y += 32;

        GFResizePic(gump, FR_BTN_X, y, FR_RESIZEPIC_GFX, FR_BTN_W, 3);
        y += 14;

        GFTextLine(gump, FR_BTN_X, y, FR_COLOR_TITLE, "Escolha o tipo de configuração:");
        y += 24;

        GFAddButton(gump, FR_BTN_X, y, FR_BTN_DN, FR_BTN_UP, 1, FR_BTN_WIZARD_PRESET);
        GFTextLine(gump, FR_BTN_X + 30, y + 4, FR_COLOR_BODY, "Escolher preset pronto");
        y += FR_ROW_H;

        GFAddButton(gump, FR_BTN_X, y, FR_BTN_DN, FR_BTN_UP, 1, FR_BTN_WIZARD_MANUAL);
        GFTextLine(gump, FR_BTN_X + 30, y + 4, FR_COLOR_BODY, "Configurar manualmente");
        y += FR_ROW_H + 10;

        GFResizePic(gump, FR_BTN_X, y, FR_RESIZEPIC_GFX, FR_BTN_W, 3);
        y += 14;

        GFAddButton(gump, FR_BTN_X, y, FR_BTN_DN, FR_BTN_UP, 1, FR_BTN_WIZARD_VOLTAR);
        GFTextLine(gump, FR_BTN_X + 30, y + 4, FR_COLOR_BODY, "Cancelar");

        var result := GFSendGump(who, gump);
        var btn    := result[0];

        if (!btn)
            ClearEditorState(who);
            return 0;
        elseif (!who.connected)
            ClearEditorState(who);
            return 0;
        elseif (btn == FR_BTN_WIZARD_PRESET)
            var r := ShowFogRegionPresets(who);
            if (r == 1)
                return 1;
            endif
            // r == 0: Anterior em Presets — loop continua → re-exibe Step3
        elseif (btn == FR_BTN_WIZARD_MANUAL)
            var r := ShowFogRegionManualEditor(who);
            if (r == 1)
                return 1;
            endif
            // r == 0: cancelou — loop continua → re-exibe Step3
        elseif (btn == FR_BTN_WIZARD_VOLTAR)
            return 0;
        endif
    endwhile
endfunction


// ── Tela 5a: Presets prontos ─────────────────────────────────────────────────
// Retorna 1 se criação concluída com sucesso; 0 se cancelado/Anterior/disconnect.

function ShowFogRegionPresets(who)
    while (1)
        var state := GetEditorState(who);
        var gump  := GFECreateGump("Criar Região (4/4) — Preset", FR_GUMP_W, FR_GUMP_H, array{ BORDER_GOLDEN });
        var y     := FR_CONT_Y;

        GFTextLine(gump, FR_BTN_X, y, FR_COLOR_TITLE, "Escolha um preset:");
        y += 20;

        // 6 presets — grupo 1
        GFSetRadioGroup(gump, FR_RADIO_GROUP_PRESET);
        var p_sel := state.preset_selecionado;

        var i1 := 0; if (p_sel == "atmosfera_padrao")   i1 := 1; endif
        GFRadioButton(gump, FR_BTN_X, y, FR_RADIO_OFF, FR_RADIO_ON, i1, 101);
        GFTextLine(gump, FR_BTN_X + 22, y + 2,  FR_COLOR_BODY, "atmosfera_padrao");
        GFTextLine(gump, FR_BTN_X + 22, y + 16, FR_COLOR_DESC, "Bruma sutil, padrão Camada B");
        y += FR_PRESET_ROW_H;

        var i2 := 0; if (p_sel == "noite_profunda")     i2 := 1; endif
        GFRadioButton(gump, FR_BTN_X, y, FR_RADIO_OFF, FR_RADIO_ON, i2, 102);
        GFTextLine(gump, FR_BTN_X + 22, y + 2,  FR_COLOR_BODY, "noite_profunda");
        GFTextLine(gump, FR_BTN_X + 22, y + 16, FR_COLOR_DESC, "Atmosfera densa noturna");
        y += FR_PRESET_ROW_H;

        var i3 := 0; if (p_sel == "bosque_verde_musgo") i3 := 1; endif
        GFRadioButton(gump, FR_BTN_X, y, FR_RADIO_OFF, FR_RADIO_ON, i3, 103);
        GFTextLine(gump, FR_BTN_X + 22, y + 2,  FR_COLOR_BODY, "bosque_verde_musgo");
        GFTextLine(gump, FR_BTN_X + 22, y + 16, FR_COLOR_DESC, "Floresta úmida pantanosa");
        y += FR_PRESET_ROW_H;

        var i4 := 0; if (p_sel == "sepia_arido")        i4 := 1; endif
        GFRadioButton(gump, FR_BTN_X, y, FR_RADIO_OFF, FR_RADIO_ON, i4, 104);
        GFTextLine(gump, FR_BTN_X + 22, y + 2,  FR_COLOR_BODY, "sepia_arido");
        GFTextLine(gump, FR_BTN_X + 22, y + 16, FR_COLOR_DESC, "Tom árido de deserto/ruínas");
        y += FR_PRESET_ROW_H;

        var i5 := 0; if (p_sel == "vinheta_envolvente") i5 := 1; endif
        GFRadioButton(gump, FR_BTN_X, y, FR_RADIO_OFF, FR_RADIO_ON, i5, 105);
        GFTextLine(gump, FR_BTN_X + 22, y + 2,  FR_COLOR_BODY, "vinheta_envolvente");
        GFTextLine(gump, FR_BTN_X + 22, y + 16, FR_COLOR_DESC, "Cinematográfica, foco no player");
        y += FR_PRESET_ROW_H;

        var i6 := 0; if (p_sel == "vale_enevoado")      i6 := 1; endif
        GFRadioButton(gump, FR_BTN_X, y, FR_RADIO_OFF, FR_RADIO_ON, i6, 106);
        GFTextLine(gump, FR_BTN_X + 22, y + 2,  FR_COLOR_BODY, "vale_enevoado");
        GFTextLine(gump, FR_BTN_X + 22, y + 16, FR_COLOR_DESC, "Névoa rasteira, chão coberto");
        y += FR_PRESET_ROW_H;

        GFResizePic(gump, FR_BTN_X, y, FR_RESIZEPIC_GFX, FR_BTN_W, 3);
        y += 12;

        // Duração — grupo 2
        GFTextLine(gump, FR_BTN_X, y, FR_COLOR_TITLE, "Duração:");
        y += 18;

        GFSetRadioGroup(gump, FR_RADIO_GROUP_DURACAO);

        var ip := 0; if (state.duracao_mode == FOG_REGION_MODE_PERMANENT) ip := 1; endif
        GFRadioButton(gump, FR_BTN_X, y, FR_RADIO_OFF, FR_RADIO_ON, ip, FR_RADIO_DURACAO_PERM);
        GFTextLine(gump, FR_BTN_X + 22, y + 2, FR_COLOR_BODY, "Permanente");
        y += 20;

        var it := 0; if (state.duracao_mode == FOG_REGION_MODE_TIMED) it := 1; endif
        GFRadioButton(gump, FR_BTN_X, y, FR_RADIO_OFF, FR_RADIO_ON, it, FR_RADIO_DURACAO_TIMED);
        GFTextLine(gump, FR_BTN_X + 22, y + 2, FR_COLOR_BODY, "Por tempo:");

        GFAddButton(gump, 155, y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_PRESETS_MIN_DEC);
        GFTextLine(gump, 173, y + 2, FR_COLOR_BODY, CStr(state.duracao_valor));
        GFAddButton(gump, 195, y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_PRESETS_MIN_INC);
        GFTextLine(gump, 213, y + 2, FR_COLOR_BODY, "min");
        y += 22;

        if (Len(state.erro) > 0)
            GFTextLine(gump, FR_BTN_X, y, FR_COLOR_WARN, state.erro);
            y += 18;
        endif

        GFResizePic(gump, FR_BTN_X, y, FR_RESIZEPIC_GFX, FR_BTN_W, 3);
        y += 12;

        GFAddButton(gump, FR_BTN_X, y, FR_BTN_DN, FR_BTN_UP, 1, FR_BTN_WIZARD_VOLTAR);
        GFTextLine(gump, FR_BTN_X + 30, y + 4, FR_COLOR_BODY, "< Anterior");

        GFAddButton(gump, 190, y, FR_BTN_DN, FR_BTN_UP, 1, FR_BTN_WIZARD_CRIAR);
        GFTextLine(gump, 220, y + 4, FR_COLOR_BODY, "Criar");

        var result := GFSendGump(who, gump);
        var btn    := result[0];

        if (!btn)
            ClearEditorState(who);
            return 0;
        elseif (!who.connected)
            ClearEditorState(who);
            return 0;
        elseif (btn == FR_BTN_PRESETS_MIN_DEC || btn == FR_BTN_PRESETS_MIN_INC)
            _ReadPresetRadios(result, state);
            state.duracao_valor := _Step(state.duracao_valor, 1, FR_MIN_DURACAO, FR_MAX_DURACAO, btn == FR_BTN_PRESETS_MIN_INC);
            state.erro := "";
            SetEditorState(who, state);
        elseif (btn == FR_BTN_WIZARD_CRIAR)
            _ReadPresetRadios(result, state);
            if (Len(state.preset_selecionado) == 0)
                state.erro := "Selecione um preset antes de criar.";
                SetEditorState(who, state);
            else
                var p      := _GetPreset(state.preset_selecionado);
                state.density_int := p.DensityInt;
                state.radius      := p.Radius;
                state.falloff     := p.Falloff;
                state.vmin_int    := p.VMinInt;
                state.vmax_int    := p.VMaxInt;
                state.noise_int   := p.NoiseInt;
                state.scale_int   := p.ScaleInt;
                state.speed_int   := p.SpeedInt;
                state.dark_int    := p.DarkInt;
                state.color_r     := p.ColorR;
                state.color_g     := p.ColorG;
                state.color_b     := p.ColorB;
                state.desat_int   := p.DesatInt;
                var region := _StateToRegionStruct(state, state.preset_selecionado);
                var saved := SaveRegion(region);
                if (saved.errortext)
                    state.erro := "Erro ao salvar: " + saved.errortext;
                    SetEditorState(who, state);
                else
                    _StartEffectIfTimed(region);
                    ClearEditorState(who);
                    SendSysMessage(who, "[FOG] Região '" + region.Name + "' criada.");
                    return 1;
                endif
            endif
        elseif (btn == FR_BTN_WIZARD_VOLTAR)
            return 0;
        endif
    endwhile
endfunction


// ── Tooltip helpers ───────────────────────────────────────────────────────────

function _GetTooltipData(var_name)
    var d := struct;
    d.+title   := "Tooltip";
    d.+desc    := "";
    d.+values  := array{};
    d.+analogy := "";
    d.+warning := "";

    if (var_name == "density")
        d.title   := "density — Densidade da névoa";
        d.desc    := "Controla quanta névoa existe na cena.";
        d.values  := array{ "0.001 = translucida, quase invisivel", "0.003 = nevoa leve (manha no pantano)",
                            "0.008 = nevoa media (floresta enevoada)", "0.020 = nevoa espessa (vale fechado)",
                            "0.050 = nevoa impenetravel (masmorra)" };
        d.analogy := "Quanto de tinta você dissolve na água.";
        d.warning := "Acima de 0.02 forma 'parede' proxima ao char.";
    elseif (var_name == "radius")
        d.title   := "radius — Raio de visibilidade";
        d.desc    := "Tamanho da bolha de ar limpo em torno do player.";
        d.values  := array{ "30 = sufoca, combate quase impossivel", "100 = confortavel para combate",
                            "300 = clearing amplo, nevoa so ao fundo", "800 = nevoa so nas bordas da tela" };
        d.analogy := "A bolha de ar limpo que o char carrega.";
        d.warning := "Abaixo de 50 com density alta = claustrofobia.";
    elseif (var_name == "falloff")
        d.title   := "falloff — Suavidade da transição";
        d.desc    := "Quão gradual é a borda do clearing.";
        d.values  := array{ "10 = borda nitida (circulo recortado)", "50 = transicao suave e natural",
                            "150 = gradiente amplo e difuso" };
        d.analogy := "A duração do fade entre ar limpo e névoa.";
        d.warning := "Falloff < 20 com density alta cria 'bolha obvia'.";
    elseif (var_name == "vmin")
        d.title   := "vmin — Intensidade no chão";
        d.desc    := "Multiplicador de névoa na altitude mínima.";
        d.values  := array{ "0.0 = sem nevoa no chao", "0.3 = nevoa so em elevacoes medias",
                            "0.95 = nevoa comeca quase no nivel do chao", "2.5 = nevoa muito densa na superficie" };
        d.analogy := "Quanto a névoa 'pesa' no chão.";
    elseif (var_name == "vmax")
        d.title   := "vmax — Intensidade na altitude";
        d.desc    := "Multiplicador de névoa em elevações altas.";
        d.values  := array{ "0.0 = sem nevoa no ar (rasante)", "1.2 = nevoa cresce normalmente",
                            "2.5 = nevoa muito intensa no alto" };
        d.analogy := "Quanto a névoa cresce com a altitude.";
        d.warning := "vmin > vmax inverte gradiente (preset Vale Enevoado).";
    elseif (var_name == "noise")
        d.title   := "noise — Força da variação volumétrica";
        d.desc    := "Quão pronunciadas são as ondulações internas.";
        d.values  := array{ "0.0 = nevoa completamente lisa", "0.3 = pequenas ondulacoes suaves",
                            "0.6 = ondulacoes claramente visiveis", "1.0 = turbulencia maxima" };
        d.analogy := "O quanto a névoa 'respira'.";
    elseif (var_name == "scale")
        d.title   := "scale — Tamanho das ondulações";
        d.desc    := "Escala espacial do padrão de noise.";
        d.values  := array{ "0.3 = massas muito grandes e lentas", "1.0 = tamanho medio natural (padrao)",
                            "3.0 = ondulacoes miudas e detalhadas", "5.0 = textura fina quase estatica" };
        d.analogy := "O tamanho das nuvens dentro da névoa.";
    elseif (var_name == "speed")
        d.title   := "speed — Velocidade do drift";
        d.desc    := "Quão rápido a névoa se move horizontalmente.";
        d.values  := array{ "0.0 = nevoa completamente imovel", "0.05 = drift suave e imperceptivel",
                            "0.3 = movimento moderado visivel", "1.0 = nevoa em movimento rapido" };
        d.analogy := "A velocidade do vento dentro da névoa.";
    elseif (var_name == "dark")
        d.title   := "dark — Escurecimento de silhuetas";
        d.desc    := "Quanto objetos na névoa perdem brilho.";
        d.values  := array{ "0.0 = cores originais mantidas", "0.3 = leve escurecimento ao longe",
                            "0.7 = silhuetas bem escuras", "1.0 = objetos na nevoa quase negros" };
        d.analogy := "Quanto a névoa absorve a luz.";
        d.warning := "Acima de 0.7 cria 'buracos escuros' em objetos.";
    elseif (var_name == "desat")
        d.title   := "desat — Dessaturação por distância";
        d.desc    := "Quanto as cores se apagam na névoa.";
        d.values  := array{ "0.0 = cores originais mantidas", "0.3 = levemente lavado",
                            "0.7 = quase monocromatico ao longe", "1.0 = escala de cinza na nevoa" };
        d.analogy := "A névoa 'lava' as cores.";
    elseif (var_name == "color")
        d.title   := "color R/G/B — Cor da névoa";
        d.desc    := "A tonalidade da névoa (cores em fração 0-1).";
        d.values  := array{ "(0.20, 0.20, 0.20) = quase preta", "(0.50, 0.52, 0.58) = fria neutra (padrao)",
                            "(0.65, 0.58, 0.45) = sepia / arida", "(0.48, 0.55, 0.45) = verde musgo",
                            "(1.0, 0.3, 0.1) = avermelhada / vulcanica" };
        d.analogy := "A cor da luz que a névoa emite.";
    elseif (var_name == "z_min") // FOG-FASE5/F
        d.title   := "z min — Altitude mínima de névoa"; // FOG-FASE5/F
        d.desc    := "Altitude (Z) do mundo abaixo da qual a névoa não aparece."; // FOG-FASE5/F
        d.values  := array{ "-128 = sem limite inferior (padrao)", "0 = sem nevoa abaixo do nivel do mar",
                            "30 = nevoa apenas em elevacoes medias ou altas" }; // FOG-FASE5/F
        d.analogy := "O piso acima do qual a névoa flutua."; // FOG-FASE5/F
        d.warning := "Deve ser menor ou igual a z max."; // FOG-FASE5/F
    elseif (var_name == "z_max") // FOG-FASE5/F
        d.title   := "z max — Altitude máxima de névoa"; // FOG-FASE5/F
        d.desc    := "Altitude (Z) do mundo acima da qual a névoa não aparece."; // FOG-FASE5/F
        d.values  := array{ "127 = sem limite superior (padrao)", "50 = nevoa so em areas baixas",
                            "0 = nevoa apenas abaixo do nivel do mar" }; // FOG-FASE5/F
        d.analogy := "O teto abaixo do qual a névoa fica confinada."; // FOG-FASE5/F
        d.warning := "Deve ser maior ou igual a z min."; // FOG-FASE5/F
    elseif (var_name == "z_fade") // FOG-FASE6/G
        d.title   := "z suave — Suavidade da borda de altura";
        d.desc    := "Largura da transição (em unidades de Z) nas bordas z min e z max. Valores baixos = corte seco (teto de névoa definido); valores altos = degradê suave e difuso.";
        d.values  := array{ "1 = corte quase seco (linha dura)", "10 = transição padrão",
                            "30 = bem difuso", "60 = degradê muito longo" };
        d.analogy := "Quão 'borrada' é a faixa onde a névoa aparece/some por altura.";
    endif
    return d;
endfunction

function _ButtonIdToVarName(btn)
    if (btn == FR_BTN_ED_HELP_DENSITY) return "density"; endif
    if (btn == FR_BTN_ED_HELP_RADIUS)  return "radius"; endif
    if (btn == FR_BTN_ED_HELP_FALLOFF) return "falloff"; endif
    if (btn == FR_BTN_ED_HELP_VMIN)    return "vmin"; endif
    if (btn == FR_BTN_ED_HELP_VMAX)    return "vmax"; endif
    if (btn == FR_BTN_ED_HELP_NOISE)   return "noise"; endif
    if (btn == FR_BTN_ED_HELP_SCALE)   return "scale"; endif
    if (btn == FR_BTN_ED_HELP_SPEED)   return "speed"; endif
    if (btn == FR_BTN_ED_HELP_DARK)    return "dark"; endif
    if (btn == FR_BTN_ED_HELP_DESAT)   return "desat"; endif
    if (btn == FR_BTN_ED_HELP_COLOR)   return "color"; endif
    if (btn == FR_BTN_ED_HELP_ZMIN)    return "z_min";  endif // FOG-FASE5/F
    if (btn == FR_BTN_ED_HELP_ZMAX)    return "z_max";  endif // FOG-FASE5/F
    if (btn == FR_BTN_ED_HELP_ZFADE)   return "z_fade"; endif // FOG-FASE6/G
    return "";
endfunction

// ── Tela tooltip: exibe documentação de uma variável de fog ───────────────────

function ShowFogTooltip(who, var_name)
    var d := _GetTooltipData(var_name);
    while (1)
        var gump := GFECreateGump(d.title, FR_TOOLTIP_W, FR_TOOLTIP_H, array{ BORDER_BLUE });
        var y    := FR_CONT_Y;

        GFTextLine(gump, FR_BTN_X, y, FR_COLOR_BODY, d.desc);
        y += 24;

        var tooltip_vals := d.values;
        foreach v in tooltip_vals
            GFTextLine(gump, FR_BTN_X, y, FR_COLOR_DESC, v);
            y += 16;
        endforeach
        y += 8;

        GFTextLine(gump, FR_BTN_X, y, FR_COLOR_TITLE, d.analogy);
        y += 22;

        if (Len(d.warning) > 0)
            GFTextLine(gump, FR_BTN_X, y, FR_COLOR_WARN, d.warning);
            y += 22;
        endif

        GFResizePic(gump, FR_BTN_X, y, FR_RESIZEPIC_GFX, FR_TOOLTIP_W - 80, 3);
        y += 14;

        var close_x := FR_TOOLTIP_W - 100;
        GFAddButton(gump, close_x, y, FR_BTN_DN, FR_BTN_UP, 1, 99);
        GFTextLine(gump, close_x + 30, y + 4, FR_COLOR_BODY, "Fechar");

        var result := GFSendGump(who, gump);
        var btn    := result[0];

        if (!btn || btn == 99 || !who.connected)
            return;
        endif
    endwhile
endfunction


// ── Tela 5b: Editor Manual ────────────────────────────────────────────────────
// Retorna 1 se criação concluída com sucesso; 0 se cancelado/Anterior/disconnect.

function ShowFogRegionManualEditor(who)
    while (1)
        var state := GetEditorState(who);
        var ed_title := "Criar Região — Editor Manual";
        if (Len(state.editing) > 0)
            ed_title := "Editar Região — " + state.editing;
        endif
        var gump  := GFECreateGump(ed_title, FR_GUMP_W, FR_ED_GUMP_H, array{ BORDER_GOLDEN });
        var y     := FR_CONT_Y;

        GFTextLine(gump, FR_ED_LABEL_X, y, FR_COLOR_TITLE, "Parâmetros de névoa:");
        GFTextLine(gump, 265, y, FR_COLOR_DESC, "info");
        y += 28;

        // 10 variáveis fog com ◀▶ e [?]
        GFTextLine(gump, FR_ED_LABEL_X,  y + 3, FR_COLOR_BODY, "density");
        GFAddButton(gump, FR_ED_DEC_X,   y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_ED_DENSITY_DEC);
        GFTextLine(gump, FR_ED_VAL_X,    y + 3, FR_COLOR_BODY, _FmtScaled(state.density_int, 100000.0));
        GFAddButton(gump, FR_ED_INC_X,   y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_ED_DENSITY_INC);
        GFAddButton(gump, FR_ED_HELP_X,  y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_ED_HELP_DENSITY);
        y += FR_ED_ROW_H;

        GFTextLine(gump, FR_ED_LABEL_X,  y + 3, FR_COLOR_BODY, "radius");
        GFAddButton(gump, FR_ED_DEC_X,   y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_ED_RADIUS_DEC);
        GFTextLine(gump, FR_ED_VAL_X,    y + 3, FR_COLOR_BODY, CStr(state.radius));
        GFAddButton(gump, FR_ED_INC_X,   y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_ED_RADIUS_INC);
        GFAddButton(gump, FR_ED_HELP_X,  y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_ED_HELP_RADIUS);
        y += FR_ED_ROW_H;

        GFTextLine(gump, FR_ED_LABEL_X,  y + 3, FR_COLOR_BODY, "falloff");
        GFAddButton(gump, FR_ED_DEC_X,   y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_ED_FALLOFF_DEC);
        GFTextLine(gump, FR_ED_VAL_X,    y + 3, FR_COLOR_BODY, CStr(state.falloff));
        GFAddButton(gump, FR_ED_INC_X,   y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_ED_FALLOFF_INC);
        GFAddButton(gump, FR_ED_HELP_X,  y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_ED_HELP_FALLOFF);
        y += FR_ED_ROW_H;

        GFTextLine(gump, FR_ED_LABEL_X,  y + 3, FR_COLOR_BODY, "vmin");
        GFAddButton(gump, FR_ED_DEC_X,   y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_ED_VMIN_DEC);
        GFTextLine(gump, FR_ED_VAL_X,    y + 3, FR_COLOR_BODY, _FmtScaled(state.vmin_int, 1000.0));
        GFAddButton(gump, FR_ED_INC_X,   y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_ED_VMIN_INC);
        GFAddButton(gump, FR_ED_HELP_X,  y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_ED_HELP_VMIN);
        y += FR_ED_ROW_H;

        GFTextLine(gump, FR_ED_LABEL_X,  y + 3, FR_COLOR_BODY, "vmax");
        GFAddButton(gump, FR_ED_DEC_X,   y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_ED_VMAX_DEC);
        GFTextLine(gump, FR_ED_VAL_X,    y + 3, FR_COLOR_BODY, _FmtScaled(state.vmax_int, 1000.0));
        GFAddButton(gump, FR_ED_INC_X,   y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_ED_VMAX_INC);
        GFAddButton(gump, FR_ED_HELP_X,  y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_ED_HELP_VMAX);
        y += FR_ED_ROW_H;

        GFTextLine(gump, FR_ED_LABEL_X,  y + 3, FR_COLOR_BODY, "noise");
        GFAddButton(gump, FR_ED_DEC_X,   y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_ED_NOISE_DEC);
        GFTextLine(gump, FR_ED_VAL_X,    y + 3, FR_COLOR_BODY, _FmtScaled(state.noise_int, 1000.0));
        GFAddButton(gump, FR_ED_INC_X,   y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_ED_NOISE_INC);
        GFAddButton(gump, FR_ED_HELP_X,  y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_ED_HELP_NOISE);
        y += FR_ED_ROW_H;

        GFTextLine(gump, FR_ED_LABEL_X,  y + 3, FR_COLOR_BODY, "scale");
        GFAddButton(gump, FR_ED_DEC_X,   y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_ED_SCALE_DEC);
        GFTextLine(gump, FR_ED_VAL_X,    y + 3, FR_COLOR_BODY, _FmtScaled(state.scale_int, 1000.0));
        GFAddButton(gump, FR_ED_INC_X,   y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_ED_SCALE_INC);
        GFAddButton(gump, FR_ED_HELP_X,  y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_ED_HELP_SCALE);
        y += FR_ED_ROW_H;

        GFTextLine(gump, FR_ED_LABEL_X,  y + 3, FR_COLOR_BODY, "speed");
        GFAddButton(gump, FR_ED_DEC_X,   y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_ED_SPEED_DEC);
        GFTextLine(gump, FR_ED_VAL_X,    y + 3, FR_COLOR_BODY, _FmtScaled(state.speed_int, 10000.0));
        GFAddButton(gump, FR_ED_INC_X,   y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_ED_SPEED_INC);
        GFAddButton(gump, FR_ED_HELP_X,  y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_ED_HELP_SPEED);
        y += FR_ED_ROW_H;

        GFTextLine(gump, FR_ED_LABEL_X,  y + 3, FR_COLOR_BODY, "dark");
        GFAddButton(gump, FR_ED_DEC_X,   y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_ED_DARK_DEC);
        GFTextLine(gump, FR_ED_VAL_X,    y + 3, FR_COLOR_BODY, _FmtScaled(state.dark_int, 1000.0));
        GFAddButton(gump, FR_ED_INC_X,   y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_ED_DARK_INC);
        GFAddButton(gump, FR_ED_HELP_X,  y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_ED_HELP_DARK);
        y += FR_ED_ROW_H;

        GFTextLine(gump, FR_ED_LABEL_X,  y + 3, FR_COLOR_BODY, "desat");
        GFAddButton(gump, FR_ED_DEC_X,   y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_ED_DESAT_DEC);
        GFTextLine(gump, FR_ED_VAL_X,    y + 3, FR_COLOR_BODY, _FmtScaled(state.desat_int, 1000.0));
        GFAddButton(gump, FR_ED_INC_X,   y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_ED_DESAT_INC);
        GFAddButton(gump, FR_ED_HELP_X,  y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_ED_HELP_DESAT);
        y += FR_ED_ROW_H;

        GFTextLine(gump, FR_ED_LABEL_X,  y + 3, FR_COLOR_BODY, "z min"); // FOG-FASE5/F
        GFAddButton(gump, FR_ED_DEC_X,   y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_ED_ZMIN_DEC); // FOG-FASE5/F
        GFTextLine(gump, FR_ED_VAL_X,    y + 3, FR_COLOR_BODY, CStr(state.world_z_min)); // FOG-FASE5/F
        GFAddButton(gump, FR_ED_INC_X,   y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_ED_ZMIN_INC); // FOG-FASE5/F
        GFAddButton(gump, FR_ED_HELP_X,  y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_ED_HELP_ZMIN); // FOG-FASE5/F
        y += FR_ED_ROW_H; // FOG-FASE5/F

        GFTextLine(gump, FR_ED_LABEL_X,  y + 3, FR_COLOR_BODY, "z max"); // FOG-FASE5/F
        GFAddButton(gump, FR_ED_DEC_X,   y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_ED_ZMAX_DEC); // FOG-FASE5/F
        GFTextLine(gump, FR_ED_VAL_X,    y + 3, FR_COLOR_BODY, CStr(state.world_z_max)); // FOG-FASE5/F
        GFAddButton(gump, FR_ED_INC_X,   y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_ED_ZMAX_INC); // FOG-FASE5/F
        GFAddButton(gump, FR_ED_HELP_X,  y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_ED_HELP_ZMAX); // FOG-FASE5/F
        y += FR_ED_ROW_H; // FOG-FASE5/F

        GFTextLine(gump, FR_ED_LABEL_X,  y + 3, FR_COLOR_BODY, "z suave"); // FOG-FASE6/G
        GFAddButton(gump, FR_ED_DEC_X,   y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_ED_ZFADE_DEC); // FOG-FASE6/G
        GFTextLine(gump, FR_ED_VAL_X,    y + 3, FR_COLOR_BODY, CStr(state.world_z_fade)); // FOG-FASE6/G
        GFAddButton(gump, FR_ED_INC_X,   y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_ED_ZFADE_INC); // FOG-FASE6/G
        GFAddButton(gump, FR_ED_HELP_X,  y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_ED_HELP_ZFADE); // FOG-FASE6/G
        y += FR_ED_ROW_H; // FOG-FASE6/G

        GFResizePic(gump, FR_ED_LABEL_X, y, FR_RESIZEPIC_GFX, FR_BTN_W, 3);
        y += 12;

        // 3 linhas de cor; preview ████ na linha G
        GFTextLine(gump, FR_ED_LABEL_X,  y + 3, FR_COLOR_BODY, "color R");
        GFAddButton(gump, FR_ED_DEC_X,   y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_ED_COLOR_R_DEC);
        GFTextLine(gump, FR_ED_VAL_X,    y + 3, FR_COLOR_BODY, _FmtScaled(state.color_r, 255.0));
        GFAddButton(gump, FR_ED_INC_X,   y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_ED_COLOR_R_INC);
        y += FR_ED_ROW_H;

        GFTextLine(gump, FR_ED_LABEL_X,  y + 3, FR_COLOR_BODY, "color G");
        GFAddButton(gump, FR_ED_DEC_X,   y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_ED_COLOR_G_DEC);
        GFTextLine(gump, FR_ED_VAL_X,    y + 3, FR_COLOR_BODY, _FmtScaled(state.color_g, 255.0));
        GFAddButton(gump, FR_ED_INC_X,   y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_ED_COLOR_G_INC);
        var hue_preview := _RgbToUoHue(state.color_r, state.color_g, state.color_b);
        GFTextLine(gump, 225, y + 3, hue_preview, "████");
        y += FR_ED_ROW_H;

        GFTextLine(gump, FR_ED_LABEL_X,  y + 3, FR_COLOR_BODY, "color B");
        GFAddButton(gump, FR_ED_DEC_X,   y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_ED_COLOR_B_DEC);
        GFTextLine(gump, FR_ED_VAL_X,    y + 3, FR_COLOR_BODY, _FmtScaled(state.color_b, 255.0));
        GFAddButton(gump, FR_ED_INC_X,   y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_ED_COLOR_B_INC);
        y += FR_ED_ROW_H;

        GFResizePic(gump, FR_ED_LABEL_X, y, FR_RESIZEPIC_GFX, FR_BTN_W, 3);
        y += 12;

        // Duração — grupo 2 (reaproveitado do C3.4)
        GFTextLine(gump, FR_ED_LABEL_X, y, FR_COLOR_TITLE, "Duração:");
        y += 20;

        GFSetRadioGroup(gump, FR_RADIO_GROUP_DURACAO);

        var ip := 0; if (state.duracao_mode == FOG_REGION_MODE_PERMANENT) ip := 1; endif
        GFRadioButton(gump, FR_ED_LABEL_X, y, FR_RADIO_OFF, FR_RADIO_ON, ip, FR_RADIO_DURACAO_PERM);
        GFTextLine(gump, FR_ED_LABEL_X + 22, y + 2, FR_COLOR_BODY, "Permanente");
        y += 22;

        var it := 0; if (state.duracao_mode == FOG_REGION_MODE_TIMED) it := 1; endif
        GFRadioButton(gump, FR_ED_LABEL_X, y, FR_RADIO_OFF, FR_RADIO_ON, it, FR_RADIO_DURACAO_TIMED);
        GFTextLine(gump, FR_ED_LABEL_X + 22, y + 2, FR_COLOR_BODY, "Por tempo:");

        GFAddButton(gump, 155, y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_ED_DURACAO_DEC);
        GFTextLine(gump, 173, y + 2, FR_COLOR_BODY, CStr(state.duracao_valor));
        GFAddButton(gump, 195, y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_ED_DURACAO_INC);
        GFTextLine(gump, 213, y + 2, FR_COLOR_BODY, "min");
        y += 24;

        if (Len(state.erro) > 0)
            GFTextLine(gump, FR_ED_LABEL_X, y, FR_COLOR_WARN, state.erro);
            y += 18;
        endif

        GFResizePic(gump, FR_ED_LABEL_X, y, FR_RESIZEPIC_GFX, FR_BTN_W, 3);
        y += 12;

        // Botões de ação
        var voltar_label := "< Anterior";
        var criar_label  := "Criar";
        if (Len(state.editing) > 0)
            voltar_label := "Cancelar";
            criar_label  := "Salvar";
        endif

        GFAddButton(gump, FR_ED_LABEL_X, y, FR_BTN_DN, FR_BTN_UP, 1, FR_BTN_WIZARD_VOLTAR);
        GFTextLine(gump, FR_ED_LABEL_X + 30, y + 4, FR_COLOR_BODY, voltar_label);

        GFAddButton(gump, 155, y, FR_BTN_DN, FR_BTN_UP, 1, FR_BTN_ED_TESTAR);
        GFTextLine(gump, 185, y + 4, FR_COLOR_BODY, "Testar");

        GFAddButton(gump, 225, y, FR_BTN_DN, FR_BTN_UP, 1, FR_BTN_ED_CRIAR);
        GFTextLine(gump, 255, y + 4, FR_COLOR_BODY, criar_label);

        var result := GFSendGump(who, gump);
        var btn    := result[0];

        if (!btn)
            SendFogOffToGm(who);
            ClearEditorState(who);
            return 0;
        elseif (!who.connected)
            SendFogOffToGm(who);
            ClearEditorState(who);
            return 0;
        elseif (btn >= FR_BTN_ED_DENSITY_DEC && btn <= FR_BTN_ED_DURACAO_INC)
            _ReadEdRadios(result, state);
            _ApplyEditorStep(state, btn);
            state.erro := "";
            SetEditorState(who, state);
        elseif (btn >= FR_BTN_ED_ZMIN_DEC && btn <= FR_BTN_ED_ZMAX_INC) // FOG-FASE5/F
            _ApplyEditorStep(state, btn); // FOG-FASE5/F
            state.erro := ""; // FOG-FASE5/F
            SetEditorState(who, state); // FOG-FASE5/F
        elseif (btn == FR_BTN_ED_ZFADE_DEC || btn == FR_BTN_ED_ZFADE_INC) // FOG-FASE6/G
            _ApplyEditorStep(state, btn); // FOG-FASE6/G
            state.erro := ""; // FOG-FASE6/G
            SetEditorState(who, state); // FOG-FASE6/G
        elseif (btn >= FR_BTN_ED_HELP_DENSITY && btn <= FR_BTN_ED_HELP_COLOR)
            var var_name := _ButtonIdToVarName(btn);
            if (Len(var_name) > 0)
                ShowFogTooltip(who, var_name);
            endif
        elseif (btn == FR_BTN_ED_HELP_ZMIN || btn == FR_BTN_ED_HELP_ZMAX || btn == FR_BTN_ED_HELP_ZFADE) // FOG-FASE5/F FOG-FASE6/G
            var var_name_z := _ButtonIdToVarName(btn); // FOG-FASE5/F
            if (Len(var_name_z) > 0) // FOG-FASE5/F
                ShowFogTooltip(who, var_name_z); // FOG-FASE5/F
            endif // FOG-FASE5/F
        elseif (btn == FR_BTN_ED_TESTAR)
            _ReadEdRadios(result, state);
            SetEditorState(who, state);
            SendFogTestPacket(who, state);
        elseif (btn == FR_BTN_ED_CRIAR)
            _ReadEdRadios(result, state);
            if (state.world_z_min > state.world_z_max) // FOG-FASE5/F
                state.erro := "z min não pode ser maior que z max."; // FOG-FASE5/F
                SetEditorState(who, state); // FOG-FASE5/F
            else // FOG-FASE5/F
            var is_edit := (Len(state.editing) > 0);
            var region  := _StateToRegionStruct(state, "manual");
            var saved   := SaveRegion(region);
            if (saved.errortext)
                state.erro := "Erro ao salvar: " + saved.errortext;
                SetEditorState(who, state);
            else
                SendFogOffToGm(who);
                _StartEffectIfTimed(region);
                ClearEditorState(who);
                var msg_verb := "criada";
                if (is_edit) msg_verb := "atualizada"; endif
                SendSysMessage(who, "[FOG] Região '" + region.Name + "' " + msg_verb + ".");
                return 1;
            endif
            endif // FOG-FASE5/F
        elseif (btn == FR_BTN_WIZARD_VOLTAR)
            SendFogOffToGm(who);
            if (Len(state.editing) > 0)
                ClearEditorState(who);
            endif
            return 0;
        endif
    endwhile
endfunction


// ── C3.7: Helpers de lista ───────────────────────────────────────────────────

// FOG-FASE5: Formata segundos restantes em "Xh Ym", "Ym" ou "expirou".
function _FormatTimer(seconds)
    if (seconds <= 0)
        return "expirou";
    endif
    var mins := CInt(seconds / 60);
    if (mins >= 60)
        var hrs   := CInt(mins / 60);
        var rmins := mins - (hrs * 60);
        return CStr(hrs) + "h " + CStr(rmins) + "m";
    endif
    return CStr(mins) + "m";
endfunction

// FOG-FASE5: Gump de confirmação antes de excluir região.
function _ConfirmDelete(who, region_name)
    while (1)
        var gump := GFECreateGump("Excluir Região", FR_GUMP_W, 200, array{ BORDER_GOLDEN });
        var y    := FR_CONT_Y;

        GFTextLine(gump, FR_BTN_X, y, FR_COLOR_WARN, "Excluir região permanentemente?");
        y += 24;
        GFTextLine(gump, FR_BTN_X, y, FR_COLOR_BODY, region_name);
        y += 36;

        GFResizePic(gump, FR_BTN_X, y, FR_RESIZEPIC_GFX, FR_BTN_W, 3);
        y += 14;

        GFAddButton(gump, FR_BTN_X, y, FR_BTN_DN, FR_BTN_UP, 1, FR_BTN_LIST_DEL_CANCEL);
        GFTextLine(gump, FR_BTN_X + 30, y + 4, FR_COLOR_BODY, "Cancelar");

        GFAddButton(gump, 200, y, FR_BTN_DN, FR_BTN_UP, 1, FR_BTN_LIST_DEL_CONFIRM);
        GFTextLine(gump, 230, y + 4, FR_COLOR_WARN, "Excluir");

        var result := GFSendGump(who, gump);
        var btn    := result[0];

        if (!btn || !who.connected || btn == FR_BTN_LIST_DEL_CANCEL)
            return 0;
        elseif (btn == FR_BTN_LIST_DEL_CONFIRM)
            return 1;
        endif
    endwhile
endfunction

// FOG-FASE5: Popula editor state com dados de região existente para edição.
function _PopulateEditorStateFromRegion(who, region)
    var state := struct;
    state.+nome               := region.Name;
    state.+nw_x               := region.X1;
    state.+nw_y               := region.Y1;
    state.+nw_realm           := region.Realm;
    state.+se_x               := region.X2;
    state.+se_y               := region.Y2;
    state.+se_realm           := region.Realm;
    state.+erro               := "";
    state.+preset_selecionado := "";
    state.+duracao_mode       := region.Mode;
    state.+duracao_valor      := 30;
    state.+density_int        := region.DensityInt;
    state.+radius             := region.Radius;
    state.+falloff            := region.Falloff;
    state.+vmin_int           := region.VMinInt;
    state.+vmax_int           := region.VMaxInt;
    state.+noise_int          := region.NoiseInt;
    state.+scale_int          := region.ScaleInt;
    state.+speed_int          := region.SpeedInt;
    state.+dark_int           := region.DarkInt;
    state.+desat_int          := region.DesatInt;
    state.+color_r            := region.ColorR;
    state.+color_g            := region.ColorG;
    state.+color_b            := region.ColorB;
    state.+world_z_min        := region.WorldZMin;  // FOG-FASE5/F
    state.+world_z_max        := region.WorldZMax;  // FOG-FASE5/F
    state.+world_z_fade       := region.FalloffZ;   // FOG-FASE6/G
    state.+editing            := region.Name;
    SetEditorState(who, state);
endfunction


// ── Tela 7: Lista de regiões ──────────────────────────────────────────────────

// FOG-FASE5: C3.7 — Lista paginada (3/pág) com Edit/Ir/Toggle/Del por região.
function ShowFogRegionList(who, page)
    while (1)
        var regions  := LoadRegions();
        var total    := Len(regions);
        if (total == 0)
            SendSysMessage(who, "[FOG] Nenhuma região cadastrada.");
            return;
        endif

        var per_page  := 3;
        var total_pgs := (total + per_page - 1) / per_page;
        if (page < 1)          page := 1;          endif
        if (page > total_pgs)  page := total_pgs;  endif

        var start := (page - 1) * per_page + 1;
        var end   := start + per_page - 1;
        if (end > total) end := total; endif

        var gump := GFECreateGump("Fog Regions — Lista", FR_GUMP_W, FR_GUMP_H, array{ BORDER_GOLDEN });
        var y    := FR_CONT_Y;

        GFTextLine(gump, FR_BTN_X, y, FR_COLOR_TITLE,
            "Regiões (" + CStr(total) + ")  Pág " + CStr(page) + "/" + CStr(total_pgs));
        y += 28;

        var slot := 1;
        for i := start to end
            var r := regions[i];

            var sc := FR_COLOR_STATUS_STOPPED;
            if (r.Status == FOG_REGION_STATUS_ACTIVE)
                if (r.Mode == FOG_REGION_MODE_TIMED)
                    sc := FR_COLOR_STATUS_ACTIVE_TIMED;
                else
                    sc := FR_COLOR_STATUS_ACTIVE_PERM;
                endif
            endif

            GFTextLine(gump, FR_BTN_X, y, sc, r.Name);
            y += 16;

            GFTextLine(gump, FR_BTN_X, y, FR_COLOR_DESC,
                "(" + r.X1 + "," + r.Y1 + ")→(" + r.X2 + "," + r.Y2 + ") [" + r.Realm + "]");
            y += 14;

            if (r.Mode == FOG_REGION_MODE_TIMED && r.Status == FOG_REGION_STATUS_ACTIVE)
                var remaining := r.TimerEnd - polcore().systime;
                GFTextLine(gump, FR_BTN_X, y, FR_COLOR_STATUS_ACTIVE_TIMED, _FormatTimer(remaining));
                y += 14;
            endif

            var btn_y := y;
            GFAddButton(gump, FR_BTN_X,       btn_y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_LIST_EDIT_1   + (slot - 1));
            GFTextLine( gump, FR_BTN_X + 18,  btn_y + 2, FR_COLOR_BODY, "Edit");

            GFAddButton(gump, FR_BTN_X + 55,  btn_y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_LIST_IR_1     + (slot - 1));
            GFTextLine( gump, FR_BTN_X + 73,  btn_y + 2, FR_COLOR_BODY, "Ir");

            var toggle_label := "Parar";
            if (r.Status != FOG_REGION_STATUS_ACTIVE) toggle_label := "Ativar"; endif
            GFAddButton(gump, FR_BTN_X + 95,  btn_y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_LIST_TOGGLE_1 + (slot - 1));
            GFTextLine( gump, FR_BTN_X + 113, btn_y + 2, FR_COLOR_BODY, toggle_label);

            GFAddButton(gump, FR_BTN_X + 155, btn_y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_LIST_DEL_1    + (slot - 1));
            GFTextLine( gump, FR_BTN_X + 173, btn_y + 2, FR_COLOR_WARN, "Del");

            y += 22;
            GFResizePic(gump, FR_BTN_X, y, FR_RESIZEPIC_GFX, FR_BTN_W, 2);
            y += 10;

            slot += 1;
        endfor

        GFResizePic(gump, FR_BTN_X, y, FR_RESIZEPIC_GFX, FR_BTN_W, 3);
        y += 12;

        if (page > 1)
            GFAddButton(gump, FR_BTN_X,      y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_LIST_PREV);
            GFTextLine( gump, FR_BTN_X + 18, y + 2, FR_COLOR_BODY, "◀");
        endif
        if (page < total_pgs)
            GFAddButton(gump, FR_BTN_X + 45, y, FR_BTN_SMALL_UP, FR_BTN_SMALL_DN, 1, FR_BTN_LIST_NEXT);
            GFTextLine( gump, FR_BTN_X + 63, y + 2, FR_COLOR_BODY, "▶");
        endif

        GFAddButton(gump, 235, y, FR_BTN_DN, FR_BTN_UP, 1, FR_BTN_LIST_FECHAR);
        GFTextLine(gump, 265, y + 4, FR_COLOR_BODY, "Fechar");

        var result := GFSendGump(who, gump);
        var btn    := result[0];

        if (!btn || !who.connected || btn == FR_BTN_LIST_FECHAR)
            return;
        elseif (btn == FR_BTN_LIST_PREV)
            page -= 1;
        elseif (btn == FR_BTN_LIST_NEXT)
            page += 1;
        else
            var slot_idx := -1;
            var action   := "";

            if (btn >= FR_BTN_LIST_EDIT_1 && btn <= FR_BTN_LIST_EDIT_3)
                slot_idx := btn - FR_BTN_LIST_EDIT_1;
                action   := "edit";
            elseif (btn >= FR_BTN_LIST_IR_1 && btn <= FR_BTN_LIST_IR_3)
                slot_idx := btn - FR_BTN_LIST_IR_1;
                action   := "ir";
            elseif (btn >= FR_BTN_LIST_TOGGLE_1 && btn <= FR_BTN_LIST_TOGGLE_3)
                slot_idx := btn - FR_BTN_LIST_TOGGLE_1;
                action   := "toggle";
            elseif (btn >= FR_BTN_LIST_DEL_1 && btn <= FR_BTN_LIST_DEL_3)
                slot_idx := btn - FR_BTN_LIST_DEL_1;
                action   := "del";
            endif

            if (slot_idx >= 0)
                var real_idx := start + slot_idx;
                if (real_idx <= total)
                    _HandleListAction(who, regions[real_idx], action);
                endif
            endif
        endif
    endwhile
endfunction


// FOG-FASE5: C3.7 — Executa ação de lista em uma região (ir/toggle/del/edit).
function _HandleListAction(who, region, action)
    if (action == "ir")
        var cx := region.X1 + CInt((region.X2 - region.X1) / 2);
        var cy := region.Y1 + CInt((region.Y2 - region.Y1) / 2);
        MoveObjectToLocation(who, cx, cy, who.z, region.Realm, MOVEOBJECT_FORCELOCATION);
        SendSysMessage(who, "[FOG] Teletransportado para " + region.Name + ".");
    elseif (action == "toggle")
        var updated := GetRegionByName(region.Name);
        if (updated.errortext)
            SendSysMessage(who, "[FOG] Erro: região não encontrada.");
            return;
        endif
        if (updated.Status == FOG_REGION_STATUS_ACTIVE)
            updated.Status := FOG_REGION_STATUS_STOPPED;
        else
            updated.Status := FOG_REGION_STATUS_ACTIVE;
        endif
        var saved := SaveRegion(updated);
        if (saved.errortext)
            SendSysMessage(who, "[FOG] Erro ao salvar: " + saved.errortext);
        else
            if (updated.Status == FOG_REGION_STATUS_ACTIVE)
                _StartEffectIfTimed(updated);
            endif
            var label := "parada";
            if (updated.Status == FOG_REGION_STATUS_ACTIVE) label := "ativada"; endif
            SendSysMessage(who, "[FOG] Região '" + region.Name + "' " + label + ".");
        endif
    elseif (action == "del")
        var confirmed := _ConfirmDelete(who, region.Name);
        if (confirmed)
            var del_result := DeleteRegion(region.Name);
            if (del_result.errortext)
                SendSysMessage(who, "[FOG] Erro ao excluir: " + del_result.errortext);
            else
                SendSysMessage(who, "[FOG] Região '" + region.Name + "' excluída.");
            endif
        endif
    elseif (action == "edit")
        _PopulateEditorStateFromRegion(who, region);
        ShowFogRegionManualEditor(who);
    endif
endfunction


// ── Tela 8: Onde estou ────────────────────────────────────────────────────────

// FOG-FASE5: C3.8 — Mostra posição atual e qual fog region cobre este ponto.
function ShowFogRegionWhereAmI(who)
    while (1)
        var gump := GFECreateGump("Onde estou agora", FR_GUMP_W, 280, array{ BORDER_GOLDEN });
        var y    := FR_CONT_Y;

        GFTextLine(gump, FR_BTN_X, y, FR_COLOR_TITLE, "Posição:");
        y += 20;
        GFTextLine(gump, FR_BTN_X + 10, y, FR_COLOR_BODY, "X: " + CStr(who.x));
        y += 16;
        GFTextLine(gump, FR_BTN_X + 10, y, FR_COLOR_BODY, "Y: " + CStr(who.y));
        y += 16;
        GFTextLine(gump, FR_BTN_X + 10, y, FR_COLOR_BODY, "Z: " + CStr(who.z));
        y += 16;
        GFTextLine(gump, FR_BTN_X + 10, y, FR_COLOR_BODY, "Realm: " + who.realm);
        y += 24;

        var pol_region := GetRegionNameAtLocation(who.x, who.y, who.realm);
        if (!pol_region.errortext && Len(pol_region) > 0)
            GFTextLine(gump, FR_BTN_X, y, FR_COLOR_DESC, "Região POL: " + pol_region);
            y += 20;
        endif

        var fog_region := GetRegionAtPosition(who.x, who.y, who.realm);
        if (fog_region.errortext)
            GFTextLine(gump, FR_BTN_X, y, FR_COLOR_DESC, "Nenhuma fog region aqui.");
            y += 20;
        else
            GFTextLine(gump, FR_BTN_X, y, FR_COLOR_TITLE, "Fog region ativa:");
            y += 18;
            GFTextLine(gump, FR_BTN_X + 10, y, FR_COLOR_BODY,
                "→ '" + fog_region.Name + "' (" + fog_region.PresetName + ")");
            y += 24;
        endif

        GFResizePic(gump, FR_BTN_X, y, FR_RESIZEPIC_GFX, FR_BTN_W, 3);
        y += 14;

        GFAddButton(gump, FR_BTN_X, y, FR_BTN_DN, FR_BTN_UP, 1, FR_BTN_WAI_CRIAR);
        GFTextLine(gump, FR_BTN_X + 30, y + 4, FR_COLOR_BODY, "Criar região aqui");

        GFAddButton(gump, 220, y, FR_BTN_DN, FR_BTN_UP, 1, FR_BTN_WAI_FECHAR);
        GFTextLine(gump, 250, y + 4, FR_COLOR_BODY, "Fechar");

        var result := GFSendGump(who, gump);
        var btn    := result[0];

        if (!btn || !who.connected)
            return;
        elseif (btn == FR_BTN_WAI_FECHAR)
            return;
        elseif (btn == FR_BTN_WAI_CRIAR)
            StartFogRegionCreateFlow(who);
            return;
        endif
    endwhile
endfunction
