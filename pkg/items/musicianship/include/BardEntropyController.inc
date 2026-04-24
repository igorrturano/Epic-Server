// ============================================================================
// BardEntropyController.inc
// Wild Mage entropy system for Bard Rework "Tecelao da Realidade"
// Phase 5
//
// Requires (provided by bardmusic.inc chain):
//   AP_GetSkill(), ARCANA, TemHabilidade(), AplicaEfeito()
//   SCALE_*, ABILITY_ID::*, BARD_FONT, ALVO_BUFF_PARTY, ALVO_DEBUFF_TODOS
// Does NOT include entropy.inc to avoid MobileWrapper data segment bloat.
// Uses raw props: "entropy" (mobile), "GlobalEntropy" (global).
// ============================================================================

const BEC_CHAOS_MIN_CHANCE   := 5;
const BEC_CHAOS_MAX_CHANCE   := 95;
const BEC_LISTENERS_REQUIRED := 2;
const BEC_LISTEN_RANGE       := 10;
const BEC_COOLDOWN_PROP      := "#bard_entropy_cooldown";

// Entropy amounts
const BEC_ENTROPY_C1          := 150;
const BEC_ENTROPY_C2          := 350;
const BEC_ENTROPY_C3          := 750;
const BEC_ENTROPY_C4          := 2000;
const BEC_ENTROPY_MAJOR       := 200;
const BEC_ENTROPY_MINOR       := -200;
const BEC_ENTROPY_CHROMATIC   := 500;

// ============================================================================
// Internal Helpers
// ============================================================================

// Updates mobile and global entropy directly without MobileWrapper
function _BEC_UpdateEntropy(who, amt)
    var current := CInt(GetObjProperty(who, "entropy"));
    if (!current) current := 0; endif
    SetObjProperty(who, "entropy", current + amt);

    var global_ent := CInt(GetGlobalProperty("GlobalEntropy"));
    if (!global_ent) global_ent := 0; endif
    SetGlobalProperty("GlobalEntropy", global_ent + amt);

    if (amt > 0)
        SendSysMessageEx(who, $"Voce ganhou {amt} pontos de entropia", SSM_REQUEST);
    else
        SendSysMessageEx(who, $"Voce perdeu {Abs(amt)} pontos de entropia", SSM_FAIL);
    endif
endfunction

// Returns 1 if there are BEC_LISTENERS_REQUIRED+ connected players nearby
function _BEC_HasListeners(who)
    var count := 0;
    foreach mobile in ListMobilesNearLocation(who.x, who.y, who.z, BEC_LISTEN_RANGE, who.realm)
        if (mobile.acctname && mobile.serial != who.serial && mobile.connected)
            count += 1;
            if (count >= BEC_LISTENERS_REQUIRED)
                return 1;
            endif
        endif
    endforeach
    return 0;
endfunction

// Award composition entropy based on scale type
function _BEC_AwardCompositionEntropy(who, scale_type)
    var amt := 0;
    case (scale_type)
        SCALE_MAJOR:
            amt := BEC_ENTROPY_MAJOR;
        SCALE_MINOR:
            amt := BEC_ENTROPY_MINOR;
        SCALE_CHROMATIC:
            if (RandomInt(2) == 0)
                amt := BEC_ENTROPY_CHROMATIC;
            else
                amt := -BEC_ENTROPY_CHROMATIC;
            endif
        default:
            amt := 0;
    endcase
    if (amt != 0)
        _BEC_UpdateEntropy(who, amt);
    endif
endfunction

// ============================================================================
// Chaos Event Table
// ============================================================================

// Event 1-40: Spawn 1-3 NoCorpse creatures (tier = Arcana/20)
function _BEC_SpawnCreatures(who, arcana_real)
    var tier := CInt(arcana_real / 20);
    if (tier < 1) tier := 1; endif
    if (tier > 5) tier := 5; endif

    var templates;
    case (tier)
        1: templates := array{ "skeleton1", "zombie", "headless" };
        2: templates := array{ "skeletonarcher", "evilmage", "quasit" };
        3: templates := array{ "evilmage2", "skeleton3", "skeletonmage" };
        4: templates := array{ "voidling", "voidling3", "voidspawn2" };
        5: templates := array{ "voidling4", "voidspawn", "voidspawn3" };
        default: templates := array{ "skeleton1", "zombie", "headless" };
    endcase

    var count   := RandomInt(3) + 1;
    var spawned := 0;
    for i := 1 to count
        var tpl      := templates[RandomInt(len(templates)) + 1];
        var offset_x := RandomInt(5) - 2;
        var offset_y := RandomInt(5) - 2;
        var npc := CreateNpcFromTemplate(":epicAI:" + tpl,
                                          who.x + offset_x,
                                          who.y + offset_y,
                                          who.z, 0, who.realm);
        if (npc && !npc.errortext)
            SetObjProperty(npc, "NoCorpse", 1);
            SetObjProperty(npc, "#bard_spawn", 1);
            spawned += 1;
        endif
    endfor

    if (spawned > 0)
        PrintTextAbove(who, $"*O caos invoca {spawned} criatura(s)!*", BARD_FONT, BARD_FONT_COLOR_BAD);
        PlaySoundEffect(who, 0x029); // thunder crash
    endif
    return spawned;
endfunction

// Event 41-70: Invert effect (buff→debuff on party; debuff→buff on enemies)
function _BEC_InvertEffect(who, scale_type, arcana_real)
    var range    := CInt(arcana_real / 20);
    if (range < 2) range := 2; endif
    var duration := CInt(arcana_real / 5);
    if (duration < 5) duration := 5; endif
    var power    := CInt(arcana_real / 10);

    PrintTextAbove(who, "*O efeito se inverte!*", BARD_FONT, 88);
    if (scale_type == SCALE_MAJOR || scale_type == SCALE_MIXED)
        AplicaEfeito(who, "bard_dex_down", duration, power, range,
                     ALVO_BUFF_PARTY, "*A melodia se volta contra voce!*", 0);
    else
        AplicaEfeito(who, "bard_inspirar_heroismo", duration, power * 5, range,
                     ALVO_DEBUFF_TODOS, "*Os inimigos absorvem a melodia!*", 0);
    endif
endfunction

// Event 71-95: Random effect
function _BEC_RandomEffect(who, arcana_real)
    var range    := CInt(arcana_real / 20);
    if (range < 2) range := 2; endif
    var duration := CInt(arcana_real / 5);
    if (duration < 5) duration := 5; endif
    var power    := CInt(arcana_real / 5);

    PrintTextAbove(who, "*O caos escolhe por voce!*", BARD_FONT, 88);
    case (RandomInt(6))
        0: AplicaEfeito(who, "bard_inspirar_heroismo", duration, power*5, range, ALVO_BUFF_PARTY, "*Bonus de batalha!*", 0); break;
        1: AplicaEfeito(who, "bard_dex_down", duration, power, range, ALVO_DEBUFF_TODOS, "*Fraqueza!*", 0); break;
        2: AplicaEfeito(who, "bard_str_up", duration, power, range, ALVO_BUFF_PARTY, "*Forca!*", 0); break;
        3: AplicaEfeito(who, "bard_int_up", duration, power, range, ALVO_BUFF_PARTY, "*Sabedoria!*", 0); break;
        4: AplicaEfeito(who, "bard_str_down", duration, power, range, ALVO_DEBUFF_TODOS, "*Debilidade!*", 0); break;
        default: _BEC_SpawnCreatures(who, arcana_real); break;
    endcase
endfunction

// Event 96-100: Surge entropico — double the current effect
// Base chance 5% (events 96-100); 20% with ChaosMastery (events 81-100)
function _BEC_SurgeEffect(who, scale_type, power, arcana_real)
    var range    := CInt(arcana_real / 20);
    if (range < 2) range := 2; endif
    var duration := CInt(arcana_real / 3);
    if (duration < 5) duration := 5; endif

    PrintTextAbove(who, "*SURGE ENTROPICO! O efeito e dobrado!*", BARD_FONT, BARD_FONT_COLOR_LENDARIA);
    SendSysMessage(who, "*** SURGE ENTROPICO! ***", BARD_FONT, BARD_FONT_COLOR_LENDARIA);

    if (scale_type == SCALE_MAJOR || scale_type == SCALE_MIXED)
        AplicaEfeito(who, "bard_inspirar_heroismo", duration, power * 10, range,
                     ALVO_BUFF_PARTY, "*O poder da musica e dobrado!*", 0);
    else
        AplicaEfeito(who, "bard_dex_down", duration, power * 2, range,
                     ALVO_DEBUFF_TODOS, "*A maldicao e dobrada!*", 0);
    endif
endfunction

// Roll and fire one chaos event
function _BEC_TriggerEvent(who, scale_type, power, arcana_real)
    // Surge threshold: 5% base (96-100), 20% with ChaosMastery (81-100)
    var surge_threshold := 96;
    if (MobileWrapper(who).HasAbility(ABILITY_ID::CHAOS_MASTERY))
        surge_threshold := 81;
    endif

    var event_roll := RandomInt(100) + 1;
    if (event_roll <= 40)
        _BEC_SpawnCreatures(who, arcana_real);
    elseif (event_roll <= 70)
        _BEC_InvertEffect(who, scale_type, arcana_real);
    elseif (event_roll < surge_threshold)
        _BEC_RandomEffect(who, arcana_real);
    else
        _BEC_SurgeEffect(who, scale_type, power, arcana_real);
    endif
endfunction

// ============================================================================
// Public API
// ============================================================================

// Main composition entropy check.
// Call from CompositionEffect.inc after applying composition effects.
function BEC_CheckEntropy(who, scale_type, power)
    // Always award entropy for the composition scale
    _BEC_AwardCompositionEntropy(who, scale_type);

    // Check chaos cooldown
    var cooldown_end := CInt(GetObjProperty(who, BEC_COOLDOWN_PROP));
    if (cooldown_end > ReadGameClock())
        return 0;
    endif

    // Need 2+ listeners for Wild Mage event
    if (!_BEC_HasListeners(who))
        return 0;
    endif

    // Chaos chance = 100 - arcana (min 5%, max 95%)
    var arcana_real := CInt(AP_GetSkill(who, ARCANA) / 10);
    var chaos_chance := 100 - arcana_real;
    if (chaos_chance < BEC_CHAOS_MIN_CHANCE) chaos_chance := BEC_CHAOS_MIN_CHANCE; endif
    if (chaos_chance > BEC_CHAOS_MAX_CHANCE) chaos_chance := BEC_CHAOS_MAX_CHANCE; endif

    if (RandomInt(100) + 1 > chaos_chance)
        return 0; // No chaos event this time
    endif

    // Set chaos cooldown: RandomIntMinMax(arcana-20, arcana) minutes
    var cd_min := arcana_real - 20;
    if (cd_min < 1) cd_min := 1; endif
    var cd_max := arcana_real;
    if (cd_max < 5) cd_max := 5; endif
    SetObjProperty(who, BEC_COOLDOWN_PROP, ReadGameClock() + RandomIntMinMax(cd_min, cd_max) * 60);

    _BEC_TriggerEvent(who, scale_type, power, arcana_real);
    return 1;
endfunction

// Entropy award for cantico spells.
// Call from cantico spell scripts (or spell_executor post-cast).
// circle: 0-4 matching cantico circle
function BEC_AwardCanticoEntropy(who, circle)
    var amt;
    case (circle)
        0: 1: amt := BEC_ENTROPY_C1;  break;
        2:    amt := BEC_ENTROPY_C2;  break;
        3:    amt := BEC_ENTROPY_C3;  break;
        4:    amt := -BEC_ENTROPY_C4; break; // C4 always generates negative entropy
        default: return 0;
    endcase
    _BEC_UpdateEntropy(who, amt);
    return 1;
endfunction
