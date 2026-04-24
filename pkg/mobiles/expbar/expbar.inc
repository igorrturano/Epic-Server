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
include ":containers:storageAreas";

const MAX_LEVEL := 20;

// =============================================================================
// XP CALCULATION - Linear: current_level * 1000
// Level 1→2: 1k | 2→3: 2k | 3→4: 3k | ... | 19→20: 19k
// =============================================================================
function CalculateNextLevelXP(level)
    if (level >= MAX_LEVEL)
        return 0;
    endif
    return CInt(level * (level + 1) / 2 * 1000);
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

    while (curr_xp >= CalculateNextLevelXP(curr_level) && curr_level < MAX_LEVEL)
        LevelUp(who);
        curr_level := GetPlayerLevel(who);
    endwhile

    UpdateXPClient(who);
    return 1;
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
        if (!scroll)
            var bank := CP_GetStorageContainerForMobile(who, CP_DEFAULT_AREA, CP_CREATE);
            scroll := CreateItemInContainer(bank, "pergaminho_" + curr_level);
            SendSysMessageEx(who, "Pergaminho enviado para o banco.", SSM_INFO);
        endif
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

    UpdateXPClient(who);
    discrete_gump(who);
    return 1;
endfunction

function UpdateXPClient(who)
    var curr_level := GetPlayerLevel(who);
    var curr_xp := GetPlayerXP(who);

    var xp_progress;
    var xp_needed;

    if (curr_level >= MAX_LEVEL)
        // Max level: show full bar
        var prev_cost := GetXPForLevel(curr_level) - GetXPForLevel(curr_level - 1);
        xp_progress := prev_cost;
        xp_needed := prev_cost;
    else
        var level_start := GetXPForLevel(curr_level);
        var level_end := CalculateNextLevelXP(curr_level);
        xp_progress := curr_xp - level_start;
        xp_needed := level_end - level_start;
    endif

    // Novo Tamanho: 1(ID) + 2(Len) + 2(Sub) + 4(XP) + 4(MaxXP) = 13 bytes
    var packet := CreatePacket(0xD7, MSGLEN_VARIABLE);
    packet.SetInt16(1, 13);          // Tamanho total ajustado

    packet.SetInt16(3, 0x57);        // Subcomando agora vai direto no Offset 3
    packet.SetInt32(5, CInt(xp_progress));  // XP progress within level
    packet.SetInt32(9, CInt(xp_needed));    // Total XP needed for this level

    packet.SendPacket(who);
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

    return nearby;
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
// CRAFT XP - Called from controladoraComplexos.inc FabricaItemMontado
// Formula: tier = diff/20, xp = tier * 5. diff < 20 => no award.
// Stackable items and non-whitelisted (skill, category) pairs are skipped.
// =============================================================================
function AwardXPFromCraft(who, receita, skill, categoria)
    if (!who || !receita)
        return 0;
    endif
    if (receita.isEmpilhavel)
        return 0;
    endif

    var allowed := dictionary{
        "Tailoring"         -> array{"Armas", "LeatherArmor", "StuddedArmor",
                                      "Gibao de Peles", "Armaduras de Ossos",
                                      "Armaduras Especiais"},
        "Blacksmithy"       -> array{"Axes", "Blades", "Blunt", "Polearms",
                                      "Shields", "RingMail", "ChainMail",
                                      "Platemail", "Helmets", "SteamArmor"},
        "Carpentry"         -> array{"Armas e Escudos"},
        "BowcraftFletching" -> array{"Armas"},
        "Alchemy"           -> array{"Elixires", "Tonicos", "Toxinas",
                                      "Explosivos", "Diversos"}
    };

    if (!allowed.Exists(skill))
        return 0;
    endif
    if (!(categoria in allowed[skill]))
        return 0;
    endif

    var tier := Cint(Cint(receita.dificuldade.skill) / 20);
    if (tier < 1)
        return 0;
    endif

    var xp_gain := tier * 5;
    AddXP(who, xp_gain);
    return xp_gain;
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