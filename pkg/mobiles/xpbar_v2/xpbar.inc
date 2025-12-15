//=============================================================================
// XPBAR V2 - Sistema de XP limpo e simples
// Integrado com ClassicUO Hotbar via TithingPoints
//=============================================================================
use uo;
use os;
use math;

include ":tn:npcutil";  // Para NPC_GetNPCConfig

//=============================================================================
// CONSTANTES
//=============================================================================
const MAX_LEVEL := 20;
const XP_MULTIPLIER := 100;  // Fórmula: level³ × 100
const XPBAR_DEBUG := 0;  // 1 = debug ativo, 0 = desativado

//=============================================================================
// DEBUG: Print no console do servidor
//=============================================================================
function XPDebug(msg)
    if (XPBAR_DEBUG)
        Print("[XPBAR_V2] " + msg);
    endif
endfunction

//=============================================================================
// CORE: Calcula XP necessário para atingir um level
// Fórmula: level³ × 100
// Level 1 = 100, Level 2 = 800, Level 10 = 100000, Level 20 = 800000
//=============================================================================
function GetXPForLevel(level)
    if (level <= 1)
        return 0;
    endif
    return CInt(Pow(level, 3) * XP_MULTIPLIER);
endfunction

//=============================================================================
// GETTERS
//=============================================================================
function GetPlayerLevel(who)
    var lvl := CInt(GetObjProperty(who, "lvl"));
    return (lvl < 1) ? 1 : lvl;
endfunction

function GetPlayerXP(who)
    var xp := CInt(GetObjProperty(who, "xp"));
    return (xp < 0) ? 0 : xp;
endfunction

function GetXPToNextLevel(who)
    var lvl := GetPlayerLevel(who);
    if (lvl >= MAX_LEVEL)
        return 0;
    endif
    return GetXPForLevel(lvl + 1);
endfunction

//=============================================================================
// CORE: Adiciona XP e processa level ups
//=============================================================================
function GiveXP(who, amount)
    XPDebug("GiveXP() chamado - who: " + who.name + ", amount: " + amount);
    
    if (!who || amount <= 0)
        XPDebug("GiveXP() ERRO - who ou amount invalido");
        return 0;
    endif
    
    var lvl := GetPlayerLevel(who);
    XPDebug("GiveXP() level atual: " + lvl);
    
    if (lvl >= MAX_LEVEL)
        XPDebug("GiveXP() nivel maximo atingido");
        return 0;
    endif
    
    var xp := GetPlayerXP(who) + amount;
    SetObjProperty(who, "xp", xp);
    XPDebug("GiveXP() novo XP: " + xp);
    
    // Verificar level ups
    var xp_needed := GetXPForLevel(lvl + 1);
    while (xp >= xp_needed && lvl < MAX_LEVEL)
        lvl += 1;
        SetObjProperty(who, "lvl", lvl);
        XPDebug("GiveXP() LEVEL UP! Novo level: " + lvl);
        OnLevelUp(who, lvl);
        xp_needed := GetXPForLevel(lvl + 1);
    endwhile
    
    // Atualizar client
    XPDebug("GiveXP() chamando SendStatus()");
    SendStatus(who);
    XPDebug("GiveXP() concluido");
    return 1;
endfunction

//=============================================================================
// CALLBACK: Executado quando sobe de level
//=============================================================================
function OnLevelUp(who, new_level)
    XPDebug("OnLevelUp() level: " + new_level);
    SendSysMessage(who, "Você alcançou o nível " + new_level + "!", 0x42);
    PlaySoundEffect(who, 0x1F3);
    PlayObjectCenteredEffect(who, 0x373A, 10, 16);
    
    // Recalcular vitals se as funções existirem
    if (GetPackageByName("attributes"))
        RecalcVitals(who);
    endif
endfunction

//=============================================================================
// ADMIN: Setters diretos
//=============================================================================
function SetPlayerLevel(who, new_level)
    if (new_level < 1)
        new_level := 1;
    elseif (new_level > MAX_LEVEL)
        new_level := MAX_LEVEL;
    endif
    
    SetObjProperty(who, "lvl", new_level);
    SetObjProperty(who, "xp", GetXPForLevel(new_level));
    SendStatus(who);
    return 1;
endfunction

function SetPlayerXP(who, new_xp)
    if (new_xp < 0)
        new_xp := 0;
    endif
    
    SetObjProperty(who, "xp", new_xp);
    
    // Recalcular level baseado no XP
    var lvl := 1;
    while (lvl < MAX_LEVEL && new_xp >= GetXPForLevel(lvl + 1))
        lvl += 1;
    endwhile
    
    SetObjProperty(who, "lvl", lvl);
    SendStatus(who);
    return 1;
endfunction

//=============================================================================
// HELPER: Empacota XP para envio via TithingPoints
// Formato: (max << 16) | current (16 bits cada, max 65535)
//=============================================================================
function PackXPForClient(who)
    XPDebug("PackXPForClient() chamado para: " + who.name);
    
    var xp := GetPlayerXP(who);
    var xp_max := GetXPToNextLevel(who);
    
    XPDebug("PackXPForClient() xp=" + xp + ", xp_max=" + xp_max);
    
    // Clamp para 16 bits
    if (xp > 65535)
        xp := 65535;
    endif
    if (xp_max > 65535)
        xp_max := 65535;
    endif
    
    var packed := (xp_max * 65536) + xp;
    XPDebug("PackXPForClient() packed=" + packed);
    
    return packed;
endfunction

//=============================================================================
// COMBAT: Detecta killers próximos e premia XP
// Chamado por: npcdeath.src
//=============================================================================
function HandleExpGain(corpse)
    XPDebug("HandleExpGain() INICIO - corpse: " + corpse.serial);
    
    var nearby := ListMobilesNearLocationEx(
        corpse.x, corpse.y, LIST_IGNORE_Z, 16,
        LISTEX_FLAG_HIDDEN + LISTEX_FLAG_NORMAL, corpse.realm
    );
    
    XPDebug("HandleExpGain() mobs proximos: " + nearby.size());
    
    var killers := array{};
    foreach mob in nearby
        if (mob.acctname)  // É player
            killers.append(mob);
            XPDebug("HandleExpGain() killer encontrado: " + mob.name);
        endif
    endforeach
    
    XPDebug("HandleExpGain() total killers: " + killers.size());
    
    if (killers.size() > 0)
        AwardXPFromKill(killers, corpse);
    else
        XPDebug("HandleExpGain() NENHUM killer encontrado!");
    endif
    
    XPDebug("HandleExpGain() FIM");
    return 1;
endfunction

//=============================================================================
// COMBAT: Calcula e distribui XP por kill
//=============================================================================
function AwardXPFromKill(killers, corpse)
    XPDebug("AwardXPFromKill() INICIO");
    
    var template := GetObjProperty(corpse, "npctemplate");
    XPDebug("AwardXPFromKill() template: " + template);
    
    var npc_cfg := NPC_GetNPCConfig(template);
    
    // XP base pelo tier do NPC
    var tier := npc_cfg.Tier;
    XPDebug("AwardXPFromKill() tier: " + tier);
    
    var xp_gain := tier ? (tier * 25) : 5;
    XPDebug("AwardXPFromKill() xp_base: " + xp_gain);
    
    // Bônus por grupo (+10% por membro extra)
    if (killers.size() > 1)
        xp_gain := CInt(xp_gain * (1 + killers.size() * 0.1));
    endif
    
    // Divide entre participantes
    xp_gain := CInt(xp_gain / killers.size());
    XPDebug("AwardXPFromKill() xp_por_killer: " + xp_gain);
    
    foreach killer in killers
        XPDebug("AwardXPFromKill() dando XP para: " + killer.name);
        GiveXP(killer, xp_gain);
    endforeach
    
    XPDebug("AwardXPFromKill() FIM");
    return 1;
endfunction
