// =============================================================================
// EXPBAR - Experience & Level System v2.0
// Integrated with ClassicUO Hotbar via TithingPoints
// =============================================================================
use uo;
use os;
use vitals;
use math;

include "include/say";
include ":gumps:gumps";
include ":attributes:attributes";
include ":timedscripts:timedScripts";
include "include/utils";
include ":tn:tngumps";
include ":tn:npcutil";
include ":charactercreation:habilidades";
include ":gumps:epicGumps";
include ":mastery:CombatController";
include ":achievements:controllers/CombatController";

const MAX_LEVEL := 20;

// =============================================================================
// XP CALCULATION - Fibonacci sequence (1k, 3k, 5k, 8k, 13k, 21k...)
// =============================================================================
function CalculateNextLevelXP(level)
    if (level == 0)
        return 1000;
    elseif (level == 1)
        return 3000;
    elseif (level >= MAX_LEVEL)
        return 0;
    endif

    var prev := 2;
    var curr := 3;
    for i := 2 to level
        var next := prev + curr;
        prev := curr;
        curr := next;
    endfor

    return curr * 1000;
endfunction

function GetXPForLevel(level)
    if (level <= 1)
        return 0;
    endif
    return CalculateNextLevelXP(level - 1);
endfunction

// =============================================================================
// GETTERS
// =============================================================================
function GetPlayerLevel(who)
    var lvl := CInt(GetObjProperty(who, "Level"));
    return (lvl < 1) ? 1 : lvl;
endfunction

function GetPlayerXP(who)
    var xp := CInt(GetObjProperty(who, "Experience"));
    return (xp < 0) ? 0 : xp;
endfunction

function GetXPToNextLevel(who)
    var lvl := GetPlayerLevel(who);
    if (lvl >= MAX_LEVEL)
        return 0;
    endif
    return CalculateNextLevelXP(lvl);
endfunction

// =============================================================================
// CORE XP FUNCTIONS
// =============================================================================
function AddXP(who, amount)
    if (!who || amount <= 0)
        return 0;
    endif

    var curr_level := GetPlayerLevel(who);
    if (curr_level >= MAX_LEVEL)
        return 0;
    endif

    var curr_xp := GetPlayerXP(who) + amount;
    SetObjProperty(who, "Experience", curr_xp);

    if (curr_xp >= CalculateNextLevelXP(curr_level))
        LevelUp(who);
    endif

    SendStatus(who);
    return 1;
endfunction

function GiveXP(who, amount)
    return AddXP(who, amount);
endfunction

// =============================================================================
// LEVEL UP - Rewards, effects, healing
// =============================================================================
function LevelUp(who)
    var curr_level := CInt(GetObjProperty(who, "Level")) + 1;
    SetObjProperty(who, "Level", curr_level);

    // Even levels: scroll reward | Odd levels: HP bonus
    if (curr_level % 2 == 0)
        var scroll := CreateItemInBackpack(who, "pergaminho_" + curr_level);
        if (scroll)
            SetObjProperty(scroll, "owner", who.serial);
            scroll.color := 2117;
            scroll.newbie := 1;
        endif
    else
        var chardata := GetObjProperty(who, "chardata");
        var hp_up := 0;
        if (chardata)
            if (chardata.classe == GUERREIRO)
                hp_up := 10;
            elseif (chardata.classe == LADINO)
                hp_up := 8;
            elseif (chardata.classe == SABIO)
                hp_up := 6;
            endif
        endif

        if (hp_up > 0)
            var hitsmod := CInt(GetObjProperty(who, "hitsmod")) + hp_up;
            SetObjProperty(who, "hitsmod", hitsmod);
            RecalcVitals(who);
        endif
    endif

    PlaySoundEffect(who, 0x1F3);
    PlayObjectCenteredEffect(who, 0xB462, 10, 60);
    PrintTextAbove(who, "*LEVEL UP!*", _DEFAULT_TEXT_FONT, 1152);
    AP_RefreshVitals(who);
    SendSysMessage(who, "Voce alcancou o nivel " + curr_level + "!");

    discrete_gump(who);
    return 1;
endfunction

// =============================================================================
// ADMIN SETTERS
// =============================================================================
function SetPlayerLevel(who, new_level)
    if (new_level < 1)
        new_level := 1;
    elseif (new_level > MAX_LEVEL)
        new_level := MAX_LEVEL;
    endif

    SetObjProperty(who, "Level", new_level);
    SetObjProperty(who, "Experience", GetXPForLevel(new_level));
    SendStatus(who);
    return 1;
endfunction

function SetPlayerXP(who, new_xp)
    if (new_xp < 0)
        new_xp := 0;
    endif

    SetObjProperty(who, "Experience", new_xp);

    var lvl := 1;
    while (lvl < MAX_LEVEL && new_xp >= CalculateNextLevelXP(lvl))
        lvl += 1;
    endwhile

    SetObjProperty(who, "Level", lvl);
    SendStatus(who);
    return 1;
endfunction

// =============================================================================
// CLIENT HOTBAR INTEGRATION
// Packs XP into TithingPoints: (max << 16) | current
// =============================================================================
function PackXPForClient(who)
    var xp_current := GetPlayerXP(who);
    var xp_max := GetXPToNextLevel(who);

    if (xp_current > 65535)
        xp_current := 65535;
    endif
    if (xp_max > 65535)
        xp_max := 65535;
    endif

    return (xp_max * 65536) + xp_current;
endfunction

// =============================================================================
// COMBAT XP - Called from npcdeath.src
// =============================================================================
function handleExpGain(corpse)
    var nearby := ListMobilesNearLocationEx(
        corpse.x, corpse.y, LIST_IGNORE_Z, 16,
        LISTEX_FLAG_HIDDEN + LISTEX_FLAG_NORMAL, corpse.realm
    );

    var killers := array{};
    foreach mob in nearby
        if (mob.acctname)
            killers.append(mob);
        endif
    endforeach

    if (killers.size() > 0)
        AwardXPFromKill(killers, corpse);
    endif

    return 1;
endfunction

function AwardXPFromKill(killers, corpse)
    var template := GetObjProperty(corpse, "npctemplate");
    var npc_cfg := NPC_GetNPCConfig(template);

    var tier := npc_cfg.Tier;
    var xp_gain := tier ? (tier * 25) : 5;

    if (killers.size() > 1)
        xp_gain := CInt(xp_gain * (1 + killers.size() * 0.1));
    endif

    xp_gain := CInt(xp_gain / killers.size());

    foreach killer in killers
        AddXP(killer, xp_gain);
        MasteryCombat_Advance(killer, struct{ kills := 1 });
        AchievementCombat_Advance(killer, corpse, struct{ xp := xp_gain, kills := 1 });
    endforeach

    return 1;
endfunction

// =============================================================================
// LEVEL UP GUMP
// =============================================================================
function discrete_gump(who)
    var gump_id := 0x1E7E1;
    CloseGump(who, gump_id);

    var gump := GFCreateGump();
    if (!gump)
        return 1;
    endif

    GFSetId(gump, gump_id);
    GFClosable(gump, 1);
    GFDisposable(gump, 0);
    GFMovable(gump, 0);

    var width := 200;
    var x := 30;
    var y := 40;

    GFGumpPic(gump, x + 3, y + 7, 2520);
    GFGumpPic(gump, x + 41, y + 6, 2521);
    GFGumpPic(gump, x + 209, y + 6, 2522);
    GFGumpPic(gump, x + 42, y + 42, 2527);
    GFGumpPic(gump, x + 4, y + 41, 2526);
    GFGumpPic(gump, x + 210, y + 42, 2528);

    GFTextMid(gump, x + 45, y + 17, width, 1152, "LEVEL UP!");
    GFTextMid(gump, x + 45, y + 37, width, 1152, "Clique para distribuir pontos");
    GFAddButton(gump, x + 30, y + 17, 2117, 2118, GF_CLOSE_BTN, 1);

    var result := GFSendGump(who, gump);
    if (result[0] == 1)
        var hab_gump := GFECreateGump("Menu de Habilidades", 440, 565);
        ShowHabilidadesPage(hab_gump, who);
        AddNavigationBar(hab_gump, 2);
        GFSendGump(who, hab_gump);
    endif
endfunction

// =============================================================================
// LEGACY GUI XP BAR (server-side gump, kept for compatibility)
// =============================================================================
function UpdateXPBar(who)
    if (!GetObjProperty(who, "chardata"))
        return 0;
    endif

    const XPBAR_GUMP_ID := 0xE7B;
    _checkPid(who, "xpbar");

    var xpbar := GetObjProperty(who, "xpbar");
    if (!xpbar)
        xpbar := struct{ pos_x := 105, pos_y := 480 };
        SetObjProperty(who, "xpbar", xpbar);
    endif

    while (who.connected)
        var gump := BuildXPGump(who, xpbar, XPBAR_GUMP_ID);
        var input := SendDialogGump(who, gump.layout, gump.data, gump.base.x, gump.base.y, 0, XPBAR_GUMP_ID);

        if (input[0] == 2)
            CloseGump(who, XPBAR_GUMP_ID);
            sleepms(50);
            var new_pos := PositionGump(who, xpbar.pos_x, xpbar.pos_y);
            xpbar.pos_x := new_pos[1];
            xpbar.pos_y := new_pos[2];
            SetObjProperty(who, "xpbar", xpbar);
        endif

        sleepms(100);
        xpbar := GetObjProperty(who, "xpbar");
    endwhile
endfunction

function BuildXPGump(who, xpbar, gumpid)
    var gump := GFCreateGump(0, 0);
    GFMovable(gump, 0);
    GFDisposable(gump, 0);
    GFClosable(gump, 0);
    GFSetId(gump, gumpid);

    var curr_xp := GetPlayerXP(who);
    var curr_level := GetPlayerLevel(who);
    var next_level_xp := CalculateNextLevelXP(curr_level);

    var progress := CDbl(curr_xp) / CDbl(next_level_xp) * 100;
    var width := CInt((progress * 597) / 100);

    GFResizePic(gump, xpbar.pos_x + 10, xpbar.pos_y + 64, 9200, 597, 40);
    if (width > 0)
        GFResizePic(gump, xpbar.pos_x + 10, xpbar.pos_y + 64, 9350, width, 40);
    endif

    GFTextLine(gump, xpbar.pos_x + 18, xpbar.pos_y + 80, 1152, "Level {}".format(curr_level));
    GFTextLine(gump, xpbar.pos_x + 500, xpbar.pos_y + 74, 1152, "{}/{}".format(curr_xp, next_level_xp));
    GFAddButton(gump, xpbar.pos_x + 14, xpbar.pos_y + 67, 2103, 2103, GF_CLOSE_BTN, 2);

    return gump;
endfunction
