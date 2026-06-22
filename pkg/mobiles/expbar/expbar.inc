// =============================================================================
// EXPBAR - Experience & Level System v2.0
// Integrated with ClassicUO Hotbar via TithingPoints
// =============================================================================
use uo;
use os;
use vitals;
use math;
use util;
use polsys;

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
include ":diceroll:include/model/Dice";
include ":equipsys:Controller/WeaponController";
include "include/epicNPCUtils";
include "include/epicXpAudit";

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
function AddXP(who, amount, source := XP_UNKNOWN)
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
    XP_Record(who, amount, source, "");
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
        // Pergaminho vai pra fila de pendencias e e entregue por
        // EntregarPergaminhosPendentes — idempotente e re-tentavel. Caso
        // a entrega imediata falhe (mochila cheia, druida metamorfoseado,
        // banco indisponivel) o nivel fica pendente e e re-tentado no
        // logon e a cada pergaminho consumido em comprahab. Resolve o
        // burst do AddXP em que multiplos LevelUps em sequencia perdiam
        // pergaminhos por causa do discrete_gump bloqueante.
        AddPergaminhoPendente(who, curr_level);
        EntregarPergaminhosPendentes(who);
    else
        // ODD level — HP bonus.
        // Path-rework characters: HP grows automatically because GetPH() uses
        // CountOddLevelsReached(Level) · ClassDie. Bumping Level is sufficient;
        // a RecalcVitals applies the new max immediately. DO NOT also adjust
        // hitsmod here or HP would double-count.
        // Legacy characters: archetype-keyed hitsmod bump (Guerreiro/Ladino/Sabio).
        if (HasNewSystemDice(who))
            RecalcVitals(who);
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

// =============================================================================
// PERGAMINHOS PENDENTES - Entrega idempotente, retroativa.
//
// LevelUp acrescenta cada nivel par alcancado em "pergaminhosPendentes"
// (ObjProperty: array de niveis). EntregarPergaminhosPendentes percorre a
// lista, cria os itens (backpack primeiro, banco como fallback), e so remove
// o nivel da pendencia quando o item realmente entrou.
//
// Pontos de entrega:
//   - LevelUp: tentativa imediata, caso comum (sucede de primeira).
//   - expbar_logon: retoma caso o player tenha caido durante o burst.
//   - comprahab: apos consumir um pergaminho, libera o proximo da fila.
//
// Por que essa arquitetura: o discrete_gump no fim do LevelUp e bloqueante
// (GFSendGump espera input). Em um burst de XP que sobe varios niveis o
// while em AddXP nao consegue criar cada pergaminho de forma confiavel —
// gumps anteriores eram fechados (CloseGump no inicio do discrete_gump),
// CreateItemInBackpack podia falhar transitoriamente sem retentativa, e
// o player ficava com Level alto e zero pergaminhos. Fila + idempotencia
// elimina a perda.
// =============================================================================
// Validador: pergaminho so existe pra nivel par no intervalo do XP system.
// Guard em AddPergaminhoPendente impede gatilhos errados (ex.: delhabs em
// slot 0 do myhabs) de poluir a fila com niveis sem template.
function IsNivelPergaminhoValido(nivel)
    var n := CInt(nivel);
    return n >= 2 && n <= MAX_LEVEL && (n % 2) == 0;
endfunction

function AddPergaminhoPendente(who, nivel)
    if (!IsNivelPergaminhoValido(nivel))
        return 0;
    endif
    var pendentes := GetObjProperty(who, "pergaminhosPendentes");
    if (!pendentes || pendentes.errortext)
        pendentes := array{};
    endif
    if (nivel in pendentes)
        return 0;
    endif
    pendentes.append(nivel);
    SetObjProperty(who, "pergaminhosPendentes", pendentes);
    return 1;
endfunction

// Mapping nivel par -> objtype do pergaminho (fonte: itemdesc charactercreation).
// Sequencia nao e contigua: pula 0x6161 (lvl0hab, nao usado pelo XP system).
function PergaminhoObjtypePorNivel(nivel)
    case (nivel)
        2:  return 0x6157;
        4:  return 0x6158;
        6:  return 0x6159;
        8:  return 0x6160;
        10: return 0x6162;
        12: return 0x6163;
        14: return 0x6164;
        16: return 0x6165;
        18: return 0x6166;
        20: return 0x6167;
        default: return 0;
    endcase
endfunction

// Procura recursivamente um pergaminho do nivel em mochila e banco do player.
// Usado pela reconciliacao p/ nao duplicar item que ja existe no inventario.
function PossuiPergaminhoDoNivel(who, nivel)
    var objtype := PergaminhoObjtypePorNivel(nivel);
    if (!objtype)
        return 0;
    endif

    var bp := who.backpack;
    if (bp && !bp.errortext)
        foreach item in (EnumerateItemsInContainer(bp))
            if (item.objtype == objtype)
                return 1;
            endif
            SleepMS(2);
        endforeach
    endif

    var bank := CP_GetStorageContainerForMobile(who, CP_DEFAULT_AREA, CP_NOCREATE);
    if (bank && !bank.errortext)
        foreach item in (EnumerateItemsInContainer(bank))
            if (item.objtype == objtype)
                return 1;
            endif
            SleepMS(2);
        endforeach
    endif

    return 0;
endfunction

// Reconciliacao one-shot por char (guardada por flag pergaminhos_reconcile_v1).
//
// Para chars que ja estavam no estado bugado (Level alto, recebeu so o
// primeiro pergaminho), varre cada nivel par <= Level atual:
//   - Se buyhab[nivel] == 2 (habilidade ja comprada): pula, nada a devolver.
//   - Se o player ja tem o pergaminho desse nivel na mochila ou banco: pula.
//   - Senao: adiciona na fila de pendentes p/ entrega.
//
// Roda exatamente uma vez por personagem. Idempotente: a flag e setada antes
// de tentar entregar; se a entrega falhar (banco indisponivel), os niveis
// ficam na fila normal e sao retentados nos gatilhos comuns.
function ReconciliarPergaminhos(who)
    if (!who || who.errortext)
        return 0;
    endif
    if (GetObjProperty(who, "pergaminhos_reconcile_v1"))
        return 0;
    endif

    var curr_level := CInt(GetObjProperty(who, "Level"));
    if (curr_level < 2)
        SetObjProperty(who, "pergaminhos_reconcile_v1", 1);
        return 0;
    endif

    var buyhab := GetObjProperty(who, "buyhab");
    if (!buyhab || buyhab.errortext)
        buyhab := dictionary;
    endif

    var adicionados := 0;
    var nivel := 2;
    while (nivel <= curr_level)
        if (CInt(buyhab[nivel]) != 2 && !PossuiPergaminhoDoNivel(who, nivel))
            if (AddPergaminhoPendente(who, nivel))
                adicionados := adicionados + 1;
            endif
        endif
        nivel := nivel + 2;
    endwhile

    SetObjProperty(who, "pergaminhos_reconcile_v1", 1);

    if (adicionados > 0)
        SendSysMessageEx(who, "Restaurando " + adicionados + " pergaminho(s) que ficaram pendentes.", SSM_INFO);
    endif

    return adicionados;
endfunction

function EntregarPergaminhosPendentes(who)
    if (!who || who.errortext)
        return 0;
    endif

    var pendentes := GetObjProperty(who, "pergaminhosPendentes");
    if (!pendentes || pendentes.errortext || pendentes.size() == 0)
        return 0;
    endif

    // CLAIM imediato: zera a fila ANTES de iterar pra impedir que outra
    // chamada concorrente (logon + LevelUp recente, p.ex.) leia a mesma
    // lista e duplique os pergaminhos. Os que falharem voltam pra fila
    // no final, com merge p/ nao perder pendencias adicionadas durante
    // o nosso loop.
    EraseObjProperty(who, "pergaminhosPendentes");

    var entregues := 0;
    var restantes := array{};
    foreach nivel in pendentes
        // Defense-in-depth: descarta nivel invalido que tenha sido
        // persistido por versao antiga ou trigger errado. Sem template,
        // CreateItem falharia infinitamente e o nivel ficaria preso.
        if (!IsNivelPergaminhoValido(nivel))
            continue;
        endif

        var scroll := CreateItemInBackpack(who, "pergaminho_" + nivel);
        if (!scroll || scroll.errortext)
            var bank := CP_GetStorageContainerForMobile(who, CP_DEFAULT_AREA, CP_CREATE);
            if (bank && !bank.errortext)
                scroll := CreateItemInContainer(bank, "pergaminho_" + nivel);
                if (scroll && !scroll.errortext)
                    SendSysMessageEx(who, "Pergaminho do nivel " + nivel + " enviado para o banco.", SSM_INFO);
                endif
            endif
        endif

        if (scroll && !scroll.errortext)
            SetObjProperty(scroll, "owner", who.serial);
            scroll.color := 2117;
            scroll.newbie := 1;
            entregues := entregues + 1;
        else
            // Falha transiente — re-enfileira no final.
            restantes.append(nivel);
        endif
    endforeach

    if (restantes.size() > 0)
        // Merge com qualquer pendencia adicionada durante o loop por
        // gatilho concorrente — evita perder LevelUp que entrou no meio
        // do nosso EntregarPergaminhosPendentes.
        var atual := GetObjProperty(who, "pergaminhosPendentes");
        if (!atual || atual.errortext)
            atual := array{};
        endif
        foreach n in restantes
            if (!(n in atual))
                atual.append(n);
            endif
        endforeach
        SetObjProperty(who, "pergaminhosPendentes", atual);
    endif

    return entregues;
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
// CHALLENGE RATE - Combina HP, dano, velocidade e resist fis em um score.
// Formula: (eff_hp * threat) ^ 0.79; XP = tier * raw / 91, min 1 para tier >=1.
// Calibrada pra Lagarto~8, Netjekh~94, Red Dragon~507.
// =============================================================================
function NPC_ParseDiceAvg(diceStr)
    var d := Dice(diceStr);
    return (CDbl(d.GetMinimum()) + CDbl(d.GetMaximum())) / 2.0;
endfunction

function NPC_GetSpeedMult(speedTag)
    case (Upper(speedTag))
        "MELEE_LOW":   return 1.0;
        "MELEE_MID":   return 1.2;
        "MELEE_HIGH":  return 1.4;
        "MELEE_FAST":  return 1.6;
        "RANGED_LOW":  return 1.0;
        "RANGED_MID":  return 1.2;
        "RANGED_HIGH": return 1.4;
        "RANGED_FAST": return 1.6;
        default:       return 1.0;
    endcase
endfunction

function NPC_GetChallengeRate(npc_cfg)
    var hits_str := npc_cfg.HITS;
    if (hits_str == error)
        return 0.0;
    endif
    var avg_hp := NPC_ParseDiceAvg(hits_str);

    var phys_avg := 0.0;
    var fis_str := npc_cfg.EpicFisResists;
    if (fis_str != error)
        var tiers := SplitWords(fis_str);
        var rsum := 0;
        foreach t in tiers
            rsum := rsum + GetResistValueByTier(t);
        endforeach
        if (tiers.size() > 0)
            phys_avg := CDbl(rsum) / CDbl(tiers.size());
        endif
    endif

    // T5 daria divisão por zero; cap em 5x.
    var phys_mult := 1.0;
    if (phys_avg >= 100.0)
        phys_mult := 5.0;
    elseif (phys_avg > 0.0)
        phys_mult := 1.0 / (1.0 - phys_avg / 100.0);
    endif
    var eff_hp := avg_hp * phys_mult;

    var avg_dmg := 0.0;
    var dmg_str := npc_cfg.EpicAttackDamage;
    if (dmg_str != error)
        var dmg_tags := SplitWords(dmg_str);
        var dsum := 0.0;
        var n := 0;
        foreach tag in dmg_tags
            var dice_str := getDamageByTier(tag);
            if (dice_str == error)
                continue;
            endif
            if (dice_str == 0)
                continue;
            endif
            dsum := dsum + NPC_ParseDiceAvg(dice_str);
            n := n + 1;
        endforeach
        if (n > 0)
            avg_dmg := dsum / CDbl(n);
        endif
    endif

    var spd_mult := 1.0;
    var spd_str := npc_cfg.EpicAttackSpeed;
    if (spd_str != error)
        spd_mult := NPC_GetSpeedMult(spd_str);
    endif

    var threat := avg_dmg * spd_mult;

    if (eff_hp <= 0.0 || threat <= 0.0)
        return 0.0;
    endif

    return Pow(eff_hp * threat, 0.79);
endfunction

// =============================================================================
// CR BOOSTED - Same as NPC_GetChallengeRate, but applies the EpicBossStruct
// buffs the GM dialed in (.epicboss / spawner). Without this, a boss spawned
// with +HP, +resist and +dmg keeps paying the XP of the base template.
//
// Stacking model (matches how the buffs apply at spawn in epicBoss.inc):
//   boss.hits     -> hitsmod, added flat to avg_hp
//   boss.phresist -> resist_physical_mod, added to phys_avg (capped at 100)
//   boss.dmg      -> dmgbonus, treated here as a flat addition to avg_dmg
//
// Conservative on .dmg: even if a future combat path treats dmgbonus as a
// multiplier instead of a flat add, a flat add still moves XP in the right
// direction and never inflates threat into runaway territory.
// =============================================================================
function NPC_GetChallengeRateBoosted(npc_cfg, boss)
    var hits_str := npc_cfg.HITS;
    if (hits_str == error)
        return 0.0;
    endif
    var avg_hp := NPC_ParseDiceAvg(hits_str);
    if (boss.hits)
        avg_hp := avg_hp + CDbl(boss.hits);
    endif

    var phys_avg := 0.0;
    var fis_str := npc_cfg.EpicFisResists;
    if (fis_str != error)
        var tiers := SplitWords(fis_str);
        var rsum := 0;
        foreach t in tiers
            rsum := rsum + GetResistValueByTier(t);
        endforeach
        if (tiers.size() > 0)
            phys_avg := CDbl(rsum) / CDbl(tiers.size());
        endif
    endif
    if (boss.phresist)
        phys_avg := phys_avg + CDbl(boss.phresist);
    endif
    if (phys_avg > 100.0)
        phys_avg := 100.0;
    endif

    var phys_mult := 1.0;
    if (phys_avg >= 100.0)
        phys_mult := 5.0;
    elseif (phys_avg > 0.0)
        phys_mult := 1.0 / (1.0 - phys_avg / 100.0);
    endif
    var eff_hp := avg_hp * phys_mult;

    var avg_dmg := 0.0;
    var dmg_str := npc_cfg.EpicAttackDamage;
    if (dmg_str != error)
        var dmg_tags := SplitWords(dmg_str);
        var dsum := 0.0;
        var n := 0;
        foreach tag in dmg_tags
            var dice_str := getDamageByTier(tag);
            if (dice_str == error)
                continue;
            endif
            if (dice_str == 0)
                continue;
            endif
            dsum := dsum + NPC_ParseDiceAvg(dice_str);
            n := n + 1;
        endforeach
        if (n > 0)
            avg_dmg := dsum / CDbl(n);
        endif
    endif
    if (boss.dmg)
        avg_dmg := avg_dmg + CDbl(boss.dmg);
    endif

    var spd_mult := 1.0;
    var spd_str := npc_cfg.EpicAttackSpeed;
    if (spd_str != error)
        spd_mult := NPC_GetSpeedMult(spd_str);
    endif

    var threat := avg_dmg * spd_mult;

    if (eff_hp <= 0.0 || threat <= 0.0)
        return 0.0;
    endif

    return Pow(eff_hp * threat, 0.79);
endfunction

// =============================================================================
// LEVEL GAP MODIFIER - Banda por tier: top = tier*4, bot = top - 3
// (T1=1-4, T2=5-8, T3=9-12, T4=13-16, T5=17-20).
// Acima da banda decai; abaixo, recompensa matar acima do peso.
// Newbies (lvl 1-5) nunca sofrem decay.
// =============================================================================
function GetLevelGapModifier(player_level, tier)
    if (tier <= 0)
        return 1.0;
    endif

    var band_top := tier * 4;
    var band_bot := band_top - 3;
    var modifier := 1.0;

    if (player_level > band_top)
        var gap := player_level - band_top;
        if (gap == 1)
            modifier := 0.85;
        elseif (gap == 2)
            modifier := 0.70;
        elseif (gap == 3)
            modifier := 0.50;
        elseif (gap == 4)
            modifier := 0.25;
        else
            modifier := 0.0;
        endif
    elseif (player_level < band_bot)
        var gap := band_bot - player_level;
        if (gap >= 3)
            modifier := 1.30;
        elseif (gap == 2)
            modifier := 1.20;
        elseif (gap == 1)
            modifier := 1.10;
        endif
    endif

    if (player_level <= 5 && modifier < 1.0)
        modifier := 1.0;
    endif

    return modifier;
endfunction

// =============================================================================
// GLOBAL XP BOOST - Multiplier global ativado por GM; expira sozinho.
// Estado em GlobalProperty "XPBoost": struct{ mult, until } (gameclock).
// Cleared = 0.
// =============================================================================
function GetActiveXPBoostMult()
    var boost := GetGlobalProperty("XPBoost");
    if (boost == error || boost == 0)
        return 1.0;
    endif
    if (CInt(boost.expires) <= ReadGameClock())
        return 1.0;
    endif
    return CDbl(boost.mult);
endfunction

function SetXPBoost(mult, minutes)
    var expires := ReadGameClock() + (minutes * 60);
    SetGlobalProperty("XPBoost", struct{ mult := mult, expires := expires });
    return expires;
endfunction

function ClearXPBoost()
    SetGlobalProperty("XPBoost", 0);
    return 1;
endfunction

function GetXPBoostStatus()
    var boost := GetGlobalProperty("XPBoost");
    if (boost == error || boost == 0)
        return 0;
    endif
    var remaining := CInt(boost.expires) - ReadGameClock();
    if (remaining <= 0)
        return 0;
    endif
    return struct{ mult := CDbl(boost.mult), seconds := remaining };
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
    var tier := CInt(npc_cfg.Tier);

    // .epicboss saves the GM's buff intent as EpicBossStruct on the NPC; it
    // survives onto the corpse. Without honoring it, a custom-buffed boss pays
    // the XP of its base template and players above the original tier band
    // hit the level-gap decay (or zero XP at +5). Recompute CR from the
    // struct, and let the struct's tier override the template tier so the
    // level-gap band tracks the encounter the GM actually built.
    var boss := GetObjProperty(corpse, "EpicBossStruct");
    var cr := 0.0;
    if (boss && boss != error)
        if (CInt(boss.tier) > 0)
            tier := CInt(boss.tier);
        endif
        cr := NPC_GetChallengeRateBoosted(npc_cfg, boss);
    else
        cr := NPC_GetChallengeRate(npc_cfg);
    endif

    var base_xp := 0.0;
    if (tier > 0)
        base_xp := tier * cr / 91.0;
    endif

    var party_mult := 1.0;
    if (killers.size() > 1)
        party_mult := 1.0 + killers.size() * 0.1;
    endif
    var split := CDbl(killers.size());
    var boost_mult := GetActiveXPBoostMult();

    foreach killer in killers
        var killer_level := GetPlayerLevel(killer);
        var lvl_mod := GetLevelGapModifier(killer_level, tier);
        var killer_xp := CInt(base_xp * lvl_mod * boost_mult * party_mult / split);

        // Floor universal: fórmula antiga (tier*25) modulada pelos mesmos modifiers
        // que a CR. Pra trash, OLD formula prevalece (garante monotonicidade por tier);
        // pra boss, CR vence o piso. Decay e bonus do lvl_mod aplicam aos dois.
        if (tier > 0)
            var floor_xp := CInt(tier * 25 * lvl_mod * boost_mult * party_mult / split);
            if (killer_xp < floor_xp)
                killer_xp := floor_xp;
            endif
        endif

        if (killer_xp == 0 && lvl_mod > 0.0 && base_xp > 0.0)
            killer_xp := 1;
        endif

        // Aviso throttled (10 min) quando o decay zerou a recompensa por gap >=5
        // tiers acima da banda. Sem isso o player acha que e bug. Mesmo padrao
        // de throttle por prop usado em GrantCraftXP.
        if (tier > 0 && lvl_mod == 0.0)
            var now := POLCore().systime;
            var lastNotice := GetObjProperty(killer, "xpEasyKillNoticeAt");
            if (lastNotice == error || !lastNotice || now - CInt(lastNotice) >= 600)
                SendSysMessage(killer, "Você não aprende nada matando criaturas tão abaixo do seu nível.");
                SetObjProperty(killer, "xpEasyKillNoticeAt", now);
            endif
        endif

        AddXP(killer, killer_xp, XP_NPC_KILL);
        MasteryCombat_Advance(killer, struct{ kills := 1 });
        AchievementCombat_Advance(killer, corpse, struct{ xp := killer_xp, kills := 1 });
    endforeach

    return 1;
endfunction

// =============================================================================
// CRAFT XP - two-step: compute at montagem, pay at completion.
//
// ComputeCraftXP: pure. Whitelist + tier + boost -> int, no side effect.
//   Called from FabricaItemMontado and stamped onto the montado prop
//   (xpPendente), because receita.dificuldade is destroyed before the
//   montado is persisted (DefinePropFabricado strips it).
// GrantCraftXP: pays the stamped amount at EV_ITEM_PRODUZIDO, i.e. when a
//   real item is finished and materials were consumed - never at montagem,
//   so spamming the assembly step grants nothing. Farm closed.
//
// SUPPRESSED BY DEFAULT: logged as CRAFT_SUPPRESSED unless the
// "CraftXpEnabled" GlobalProperty == 1. Tinkering / non-whitelisted pairs
// and recipes with skill requirement < 20 yield nothing by design.
// =============================================================================
function ComputeCraftXP(receita, skill, categoria)
    if (!receita)
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

    return CInt(tier * 5 * GetActiveXPBoostMult());
endfunction

function GrantCraftXP(who, xp_gain, categoria)
    xp_gain := CInt(xp_gain);
    if (!who || xp_gain <= 0)
        return 0;
    endif

    if (CInt(GetGlobalProperty("CraftXpEnabled")) == 1)
        AddXP(who, xp_gain, XP_CRAFT);
        return xp_gain;
    endif

    // Neutral, throttled notice (max once/hour per char) so crafters know
    // the lack of XP is intentional, without naming the farm.
    var now := POLCore().systime;
    var lastNotice := GetObjProperty(who, "xpSuppressNoticeAt");
    if (lastNotice == error || !lastNotice || now - CInt(lastNotice) >= 3600)
        SendSysMessage(who, "O ganho de XP por criação está temporariamente em ajustes.");
        SetObjProperty(who, "xpSuppressNoticeAt", now);
    endif

    XP_Record(who, xp_gain, XP_CRAFT_SUPPRESSED, categoria);
    return 0;
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
        // Open the buyhab slots earned for the new level before rendering,
        // same as core_skillWin does on menu entry — otherwise the slot
        // just reached doesn't show until the player opens the full menu.
        SetPlayerSkillLevel(who);
        var hab_gump := GFECreateGump("Menu de Habilidades", 440, 565);
        ShowHabilidadesPage(hab_gump, who);
        AddNavigationBar(hab_gump, 2);
        GFSendGump(who, hab_gump);
    endif
endfunction