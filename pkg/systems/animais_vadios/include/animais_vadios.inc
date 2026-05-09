const VADIO_FEEDS_TO_FOLLOW  := 3;
const VADIO_STREAK_TO_BOND   := 3;
const VADIO_FOLLOW_DURATION  := 3600;
const VADIO_BARK_RANGE       := 4;
const VADIO_BARK_COOLDOWN    := 600;
const VADIO_EMOTE_COOLDOWN   := 900;
const VADIO_TELEPAD_DISTANCE := 30;
const VADIO_AFFECTION_TIER2  := 5;
const VADIO_AFFECTION_TIER3  := 15;
const VADIO_SOUND_DOG_LO     := 0x086;
const VADIO_SOUND_CAT_LO     := 0x06a;
const VADIO_SOUND_BIRD       := 0x00CE;

// Returns "cao" / "gato" / "passaro" for stray animals, 0 otherwise.
// Stray = eligible template AND no master/owner.
function GetVadioTipo(npc)
    if (npc.master)
        return 0;
    endif
    if (GetObjProperty(npc, "owner") != error)
        return 0;
    endif

    var t := npc.npctemplate;
    // Strip package prefix (":epicai:dog" → "dog")
    var last_colon := Find(t, ":", 2);
    if (last_colon)
        t := t[last_colon + 1, Len(t)];
    endif

    if (t == "dog" || t == "dog2" || t == "dog3" || t == "dog4" ||
        t == "dog5" || t == "dog6" || t == "dog7" || t == "dog8")
        return "cao";
    endif
    if (t == "cat")   return "gato";    endif
    if (t == "bird")  return "passaro"; endif
    return 0;
endfunction

function _VadioIsValidFood(tipo, item)
    var name := Lower(CStr(item.name));
    if (tipo == "cao")
        return Find(name, "carne", 1) || Find(name, "costela", 1) ||
               Find(name, "presunto", 1) || Find(name, "bisteca", 1) ||
               Find(name, "passaro", 1) || Find(name, "frango", 1) ||
               Find(name, "ave", 1) || Find(name, "cooked", 1);
    elseif (tipo == "gato")
        return Find(name, "peixe", 1) || Find(name, "atum", 1) ||
               Find(name, "sardinha", 1) || Find(name, "truta", 1);
    elseif (tipo == "passaro")
        return Find(name, "semente", 1) || Find(name, "grao", 1) ||
               Find(name, "milho", 1) || Find(name, "alpiste", 1) || Find(name, "trigo", 1);
    endif
    return 0;
endfunction

function _VadioReactionText(tipo)
    if (tipo == "cao")  return "*abana o rabo*"; endif
    if (tipo == "gato") return "*ronrona*";      endif
    return "*gorjeia*";
endfunction

function _VadioAffectionLevel(affection)
    if (affection < VADIO_AFFECTION_TIER2) return 1; endif
    if (affection < VADIO_AFFECTION_TIER3) return 2; endif
    return 3;
endfunction

function _VadioGetAffection(npc, player_serial)
    var feedlog := GetObjProperty(npc, "vadio_feedlog");
    if (feedlog == error) return 0; endif
    var entry := feedlog[CStr(player_serial)];
    if (entry == error) return 0; endif
    if (entry.affection == error) return 0; endif
    return CInt(entry.affection);
endfunction

function _VadioEmoteList(tipo, level)
    var emotes;
    if (tipo == "cao")
        if (level == 1)
            emotes := array{"*te fareja com cautela*", "*recua um pouco e te olha*"};
        elseif (level == 2)
            emotes := array{"*abana o rabo*", "*fareja o chão ao seu redor*"};
        else
            emotes := array{"*esfrega o focinho na sua perna*", "*fica colado em você*", "*late de alegria ao te ver*"};
        endif
    elseif (tipo == "gato")
        if (level == 1)
            emotes := array{"*te observa de longe*", "*pisca os olhos devagar*"};
        elseif (level == 2)
            emotes := array{"*ronrona baixinho*", "*esfrega no seu lado*"};
        else
            emotes := array{"*sobe no seu colo*", "*massageia seus pés com as patinhas*", "*ronrona alto e feliz*"};
        endif
    else
        if (level == 1)
            emotes := array{"*te olha com a cabeça inclinada*", "*se afasta um pouco*"};
        elseif (level == 2)
            emotes := array{"*gorjeia suavemente*", "*bate as asas contente*"};
        else
            emotes := array{"*pousa no seu ombro*", "*te bica carinhosamente*", "*canta uma melodia para você*"};
        endif
    endif
    return emotes;
endfunction

function _VadioOnCooldown(npc, prop)
    var cd := GetObjProperty(npc, prop);
    return cd != error && POLCore().systime < cd;
endfunction

// Writes or clears a following slot on the player. serial=0 clears.
function _VadioWriteFollower(player, tipo, serial)
    var seguindo := GetObjProperty(player, "vadio_seguindo");
    if (seguindo == error) seguindo := struct{}; endif
    seguindo[tipo] := serial;
    SetObjProperty(player, "vadio_seguindo", seguindo);
endfunction

function _VadioPlayerHasFollower(player, tipo)
    var seguindo := GetObjProperty(player, "vadio_seguindo");
    if (seguindo == error) return 0; endif
    var existing_serial := seguindo[tipo];
    if (existing_serial == error || existing_serial == 0) return 0; endif
    var existing := SystemFindObjectBySerial(CInt(existing_serial));
    if (!existing || existing.dead) return 0; endif
    // Self-healing: confirm the NPC is actually still targeting this player.
    var its_target := GetObjProperty(existing, "#vadio_follow_target");
    if (its_target == error || CInt(its_target) != player.serial) return 0; endif
    return 1;
endfunction

function _VadioStopFollowing(npc, tipo, follow_target)
    if (follow_target) _VadioWriteFollower(follow_target, tipo, 0); endif
    SM_TransitionTo("IDLE");
    EraseObjProperty(npc, "#vadio_follow_target");
endfunction

function VadioHandleItemGiven(npc, ev)
    var tipo := GetVadioTipo(npc);
    if (!tipo)
        MoveItemToContainer(ev.item, ev.source.backpack);
        return;
    endif

    if (!_VadioIsValidFood(tipo, ev.item))
        MoveItemToContainer(ev.item, ev.source.backpack);
        SendSysMessage(ev.source, "O animal fareja o item e recusa.");
        return;
    endif

    // Consume item and react before any follow-specific guards.
    SubtractAmount(ev.item, 1);
    PrintTextAbove(npc, _VadioReactionText(tipo));

    // Bonded animals eat from anyone but only follow their bonded player.
    var bonded := GetObjProperty(npc, "vadio_bonded");
    if (bonded != error && bonded != ev.source.serial)
        SendSysMessage(ev.source, "O animal agradece, mas parece preferir outra companhia.");
        return;
    endif

    var feedlog := GetObjProperty(npc, "vadio_feedlog");
    if (feedlog == error) feedlog := dictionary; endif
    var serial_str := CStr(ev.source.serial);
    var entry := feedlog[serial_str];
    if (entry == error)
        entry := struct{};
        entry.streak      := 0;
        entry.last_day    := 0;
        entry.today_feeds := 0;
        entry.affection   := 0;
    endif

    // Backward compat: old entries may be missing newer fields.
    if (entry.today_feeds == error) entry.today_feeds := 0; endif
    if (entry.affection   == error) entry.affection   := 0; endif

    var current_day := CInt(POLCore().systime / 86400);
    if (entry.last_day == current_day)
        entry.today_feeds := entry.today_feeds + 1;
    elseif (entry.last_day == current_day - 1)
        entry.streak      := entry.streak + 1;
        entry.today_feeds := 1;
        entry.last_day    := current_day;
    else
        entry.streak      := 1;
        entry.today_feeds := 1;
        entry.last_day    := current_day;
    endif
    entry.affection := entry.affection + 1;

    feedlog[serial_str] := entry;
    SetObjProperty(npc, "vadio_feedlog", feedlog);

    if (entry.streak >= VADIO_STREAK_TO_BOND && bonded == error)
        SetObjProperty(npc, "vadio_bonded", ev.source.serial);
    endif

    if (entry.today_feeds >= VADIO_FEEDS_TO_FOLLOW)
        if (_VadioPlayerHasFollower(ev.source, tipo))
            SendSysMessage(ev.source, "Você já tem esse animal te acompanhando.");
        else
            _VadioWriteFollower(ev.source, tipo, npc.serial);
            SetObjProperty(npc, "#vadio_follow_target", ev.source.serial);
            TS_StartTimer(npc, "encantar", VADIO_FOLLOW_DURATION);
            PrintTextAbove(npc, "*segue*");
            SM_TransitionTo("FOLLOWING", struct{ "target" := ev.source });
        endif
    endif
endfunction

// Called when SM_TickFollowing ends follow naturally (timer expiry or target death).
function VadioOnFollowEnd(npc)
    var tipo := GetVadioTipo(npc);
    if (!tipo) return; endif
    var target_serial := GetObjProperty(npc, "#vadio_follow_target");
    if (target_serial == error) return; endif
    var follow_target := SystemFindObjectBySerial(target_serial);
    if (follow_target) _VadioWriteFollower(follow_target, tipo, 0); endif
    EraseObjProperty(npc, "#vadio_follow_target");
endfunction

function VadioCheckBark(npc)
    var tipo := GetVadioTipo(npc);
    if (!tipo) return; endif
    if (_VadioOnCooldown(npc, "#bark_cd")) return; endif

    var bark_text := "*Au Au!*";
    var sound;
    if (tipo == "gato")
        bark_text := "*Miau!*";
        sound := VADIO_SOUND_CAT_LO + RandomInt(2);
    elseif (tipo == "passaro")
        bark_text := "*Piu Piu!*";
        sound := VADIO_SOUND_BIRD;
    else
        sound := VADIO_SOUND_DOG_LO + RandomInt(2);
    endif

    var bonded_serial := GetObjProperty(npc, "vadio_bonded");
    var mobs := ListMobilesNearLocation(npc.x, npc.y, npc.z, VADIO_BARK_RANGE, npc.realm);
    foreach mob in mobs
        if (mob.npctemplate || mob.dead || mob.offline) continue; endif
        if (bonded_serial != error && mob.serial == bonded_serial) continue; endif

        PrintTextAbove(npc, bark_text);
        PlaySoundEffect(npc, sound);
        SetObjProperty(npc, "#bark_cd", POLCore().systime + VADIO_BARK_COOLDOWN);
        return;
    endforeach
endfunction

function VadioCheckTelepad(npc)
    var tipo := GetVadioTipo(npc);
    if (!tipo) return; endif
    var target_serial := GetObjProperty(npc, "#vadio_follow_target");
    if (target_serial == error) return; endif
    var follow_target := SystemFindObjectBySerial(target_serial);
    if (!follow_target || follow_target.offline)
        _VadioStopFollowing(npc, tipo, follow_target);
        return;
    endif
    if (CoordinateDistance(npc.x, npc.y, follow_target.x, follow_target.y) > VADIO_TELEPAD_DISTANCE ||
        npc.realm != follow_target.realm)
        _VadioStopFollowing(npc, tipo, follow_target);
    endif
endfunction

function VadioCheckEmote(npc)
    var tipo := GetVadioTipo(npc);
    if (!tipo) return; endif
    if (_VadioOnCooldown(npc, "#emote_cd")) return; endif

    var follow_serial := GetObjProperty(npc, "#vadio_follow_target");
    if (follow_serial == error) return; endif

    var affection := _VadioGetAffection(npc, follow_serial);
    var level     := _VadioAffectionLevel(affection);
    var emotes    := _VadioEmoteList(tipo, level);

    PrintTextAbove(npc, emotes[RandomInt(Len(emotes)) + 1]);
    SetObjProperty(npc, "#emote_cd", POLCore().systime + VADIO_EMOTE_COOLDOWN);
endfunction
