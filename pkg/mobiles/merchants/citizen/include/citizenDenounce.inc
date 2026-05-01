// =============================================================================
// citizenDenounce.inc — Four-layer denunciation pipeline.
//
//   Layer 1  Vocal yell (PrintTextAbove with a randomized phrase per crime).
//   Layer 2  Walk to the nearest matching-faction authority NPC and speak
//            a private report (best-effort; skipped silently if none in range).
//   Layer 3  Persistent witness record on the perp via the citizenState DS.
//   Layer 4  Flag perp criminal via POL's built-in SetCriminal(1).
//
// Caller (witness.src) is responsible for the per-perp denunciation cooldown.
// =============================================================================

use uo;
use os;
use util;

include ":brainAI:include/npcCommands";
include ":merchants:citizen/include/citizenStore";
include ":merchants:citizen/include/citizenAuthority";

function _PickYellPhrase(perp_name, crime_type)
    var phrases;
    case (crime_type)
        "hide":
            phrases := array{
                "*aponta para " + perp_name + "* AJUDA GUARDAS! Esse esta se escondendo!",
                "AJUDA GUARDAS! Vi " + perp_name + " se esgueirando!",
                "Olha la! " + perp_name + " esta se escondendo no escuro!"
            };
        "lockpick":
            phrases := array{
                "*aponta para " + perp_name + "* AJUDA GUARDAS! Mexendo numa fechadura!",
                "GUARDAS! " + perp_name + " esta arrombando!",
                "Vi " + perp_name + " mexendo em algo trancado!"
            };
        "cast":
            phrases := array{
                "*aponta para " + perp_name + "* MAGIA! GUARDAS!",
                "Tem alguem invocando feiticos! " + perp_name + "!",
                "Magia proibida! " + perp_name + " conjurando!"
            };
        "attack":
            phrases := array{
                "*aponta para " + perp_name + "* AGRESSAO! GUARDAS!",
                "Socorro! " + perp_name + " esta atacando!",
                "Parem " + perp_name + "!"
            };
        default:
            phrases := array{
                "*aponta para " + perp_name + "* AJUDA GUARDAS!"
            };
    endcase
    return phrases[RandomInt(phrases.size()) + 1];
endfunction

function DoLayer1Yell(citizen, perp, crime_type)
    PrintTextAbove(citizen, _PickYellPhrase(perp.name, crime_type));
endfunction

function DoLayer2WalkAndSpeak(citizen, perp, crime_type)
    var factions := GetCitizenFactions(citizen);
    if (!factions.size())
        return;
    endif
    var authority := FindNearbyAuthority(citizen, factions);
    if (!authority)
        return;
    endif

    AI_Move(citizen, authority, NEMOVE_TOWARD, NEMOVE_RUN, WAKEUP, 0);

    var report := "Senhor, presenciei " + perp.name + " ";
    case (crime_type)
        "hide":     report := report + "tentando se esconder.";
        "lockpick": report := report + "mexendo em algo trancado.";
        "cast":     report := report + "invocando uma magia proibida.";
        "attack":   report := report + "atacando alguem.";
        default:    report := report + "agindo de forma suspeita.";
    endcase

    AI_Speak(citizen, report, SPEAK_TEXTTYPE_DEFAULT, SPEAK_DOEVENT_ENABLE);
endfunction

function DoLayer3Record(citizen, perp, entry)
    RecordDenounceOnPerp(citizen, perp, entry);
endfunction

function DoLayer4SetCriminal(citizen, perp)
    if (!perp.acctname)
        return;
    endif
    perp.SetCriminal(1);
endfunction
