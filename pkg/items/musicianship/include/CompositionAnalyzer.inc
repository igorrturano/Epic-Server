// ============================================================================
// CompositionAnalyzer.inc
// Analyzes bard note arrays to detect musical scale and calculate power.
// Phase 3 - Bard Rework "Tecelao da Realidade"
// ============================================================================

const SCALE_MAJOR     := "major";
const SCALE_MINOR     := "minor";
const SCALE_CHROMATIC := "chromatic";
const SCALE_MIXED     := "mixed";

// Convert note string to semitone 0-11. Returns -1 for pause/lyric.
function CA_NoteToSemitone(note)
    case (note)
        "C":
        "C2":
        "C3":
            return 0;
        "C#":
        "C#2":
            return 1;
        "D":
        "D2":
            return 2;
        "D#":
        "D#2":
            return 3;
        "E":
        "E2":
            return 4;
        "F":
        "F2":
            return 5;
        "F#":
        "F#2":
            return 6;
        "G":
        "G2":
            return 7;
        "G#":
        "G#2":
            return 8;
        "A":
        "A2":
            return 9;
        "A#":
        "A#2":
            return 10;
        "B":
        "B2":
            return 11;
        default:
            return -1;
    endcase
endfunction

// Returns 1 if relative semitone fits major scale {0,2,4,5,7,9,11}
function _CA_InMajorScale(relative)
    case (relative)
        0: 2: 4: 5: 7: 9: 11:
            return 1;
        default:
            return 0;
    endcase
endfunction

// Returns 1 if relative semitone fits natural minor scale {0,2,3,5,7,8,10}
function _CA_InMinorScale(relative)
    case (relative)
        0: 2: 3: 5: 7: 8: 10:
            return 1;
        default:
            return 0;
    endcase
endfunction

// Count notes from semitones[] that fit a major (is_major=1) or minor scale rooted at scale_root
function _CA_CountFit(semitones, scale_root, is_major)
    var count := 0;
    foreach s in (semitones)
        var relative := s - scale_root;
        if (relative < 0)
            relative := relative + 12;
        endif
        if (is_major)
            if (_CA_InMajorScale(relative))
                count += 1;
            endif
        else
            if (_CA_InMinorScale(relative))
                count += 1;
            endif
        endif
    endforeach
    return count;
endfunction

// Analyzes a note array and returns scale info + power.
// arcana_real: actual Arcana value 0-100 (caller should pass AP_GetSkill(who,ARCANA)/10)
// Returns struct{ scale_type, scale_purity(0-100), power, octave }
function CA_AnalyzeComposition(notas, arcana_real)
    var semitones   := array;
    var octave_sum  := 0;
    var pitch_count := 0;

    foreach note in (notas)
        var st := CA_NoteToSemitone(note);
        if (st >= 0)
            semitones.Append(st);
            pitch_count += 1;
            var last := note[len(note)];
            if (last == "3")
                octave_sum += 3;
            elseif (last == "2")
                octave_sum += 2;
            else
                octave_sum += 1;
            endif
        endif
    endforeach

    if (pitch_count < 3)
        return struct{
            scale_type   := SCALE_MIXED,
            scale_purity := 0,
            power        := 0,
            octave       := 1
        };
    endif

    // Find best-fit major and minor across all 12 roots
    var best_major := 0;
    var best_minor := 0;
    for scale_root := 0 to 11
        var fm := _CA_CountFit(semitones, scale_root, 1);
        var fn := _CA_CountFit(semitones, scale_root, 0);
        if (fm > best_major)
            best_major := fm;
        endif
        if (fn > best_minor)
            best_minor := fn;
        endif
    endfor

    var major_purity := CInt(best_major * 100 / pitch_count);
    var minor_purity := CInt(best_minor * 100 / pitch_count);

    // Count unique pitch classes for chromatic density
    var unique       := array;
    var unique_count := 0;
    foreach st in (semitones)
        var found := 0;
        foreach us in (unique)
            if (us == st)
                found := 1;
                break;
            endif
        endforeach
        if (!found)
            unique.Append(st);
            unique_count += 1;
        endif
    endforeach
    var chromatic_density := CInt(unique_count * 100 / 12);

    var scale_type;
    var scale_purity;
    if (chromatic_density >= 75)
        scale_type   := SCALE_CHROMATIC;
        scale_purity := chromatic_density;
    elseif (major_purity >= 70 && major_purity >= minor_purity)
        scale_type   := SCALE_MAJOR;
        scale_purity := major_purity;
    elseif (minor_purity >= 70)
        scale_type   := SCALE_MINOR;
        scale_purity := minor_purity;
    else
        scale_type   := SCALE_MIXED;
        scale_purity := CInt((major_purity + minor_purity) / 2);
    endif

    // Power formula (fixed-point):
    // power = (arcana/10) * (purity/100) * (octave_mod/100) * (note_mod/100)
    var avg_octave     := CInt(octave_sum / pitch_count);
    if (!avg_octave) avg_octave := 1; endif
    var octave_mod_pct := 100 + (avg_octave - 1) * 30; // 100, 130, or 160
    var note_mod_pct   := CInt(pitch_count * 100 / 7);
    if (note_mod_pct > 200) note_mod_pct := 200; endif
    var base_power     := CInt(arcana_real / 10);
    var power          := CInt(base_power * scale_purity * octave_mod_pct * note_mod_pct / 1000000);

    return struct{
        scale_type   := scale_type,
        scale_purity := scale_purity,
        power        := power,
        octave       := avg_octave
    };
endfunction

// Returns display label for gump scale indicator
function CA_GetScaleLabel(analysis)
    if (!analysis || analysis == error)
        return "? (notas insuficientes)";
    endif
    case (analysis.scale_type)
        SCALE_MAJOR:
            return $"Maior ({analysis.scale_purity}%) [Buff]";
        SCALE_MINOR:
            return $"Menor ({analysis.scale_purity}%) [Debuff]";
        SCALE_CHROMATIC:
            return $"Cromatica ({analysis.scale_purity}%) [Caos]";
        default:
            return $"Mista ({analysis.scale_purity}%) [Misto]";
    endcase
endfunction

// Returns gump text color code for the scale type
function CA_GetScaleColor(scale_type)
    case (scale_type)
        SCALE_MAJOR:     return 568; // verde
        SCALE_MINOR:     return 38;  // vermelho
        SCALE_CHROMATIC: return 88;  // roxo
        default:         return 275; // neutro
    endcase
endfunction