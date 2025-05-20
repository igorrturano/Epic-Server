/* 
 * epicCombatEvents.inc
 *
 * Funções principais para execução de eventos de combate e ações do chefe
 *
 */
 
use uo;
use os;
use util;
use math;

include ":attributes:attributes";
include ":epicspawn:epicBoss";
/*
 * ExecuteCombatEvent(npc, event, actions)
 * Propósito:          // Aplicar dano
            ApplyRawDamageEX(targ, damage_per_type, damage_type, "Evento de Combate");
            success := 1;
        endforeach
    endforeach
    
    return success;
endfunctiones um evento de combate com suas ações
 * Parâmetros: npc - O NPC executando o evento
 *             event - A configuração do evento
 *             actions - As ações a serem executadas
 * Retorno: 1 se executado com sucesso, 0 se não
 */
function ExecuteCombatEvent(npc, event, actions)
    sleep(event.cd ? CInt(event.cd) : 3);
    // Processa cada ação em sequência
    foreach action in actions
        if (!ExecuteCombatAction(npc, action))
            continue; // Tenta a próxima ação
        endif
        
        // Pausa brevemente entre ações para permitir animações etc.
        sleepms(200);
    endforeach
    
    return 1;
endfunction

/*
 * ExecuteCombatAction(npc, action)
 * Propósito: Executa uma única ação de combate
 * Parâmetros: npc - O NPC executando a ação
 *             action - A configuração da ação
 * Retorno: 1 se executado com sucesso, 0 se não
 */
function ExecuteCombatAction(npc, action)
    if (!action.type)
        return 0;
    endif
    
    // Obtém alvos baseados na propriedade targ da ação
    var targets := GetActionTargets(npc, action);
    
    // Executa a ação baseada em seu tipo
    case (action.type)           
        "Teleport":
            return ExecuteTeleportAction(npc, targets, action);
            
        "Perform Action":
            return ExecutePerformAction(targets, action);
            
        "Text":
            return ExecuteTextAction(npc, action);
            
        "Play Sound":
            return ExecutePlaySoundAction(npc, action);
            
        "Delay":
            return ExecuteDelayAction(action);
            
        "Heal":
            return ExecuteHealAction(targets, action);
            
        "Dmg":
            return ExecuteDamageAction(targets, action);
            
        "Graphic Effect":
            return ExecuteGraphicEffectAction(npc, targets, action);
            
        "Summon Npcs":
            return ExecuteSummonNpcsAction(npc, action);
            
        "Create Items":
            return ExecuteCreateItemsAction(npc, action);
            
        "Pushback":
            return ExecutePushbackAction(npc, targets, action);
            
        "Timedscripts":
            return ExecuteTimedScriptAction(npc, targets, action);
            
        default:
            return 0;
    endcase
    
    return 0;
endfunction

/*
 * GetActionTargets(npc, action)
 * Propósito: Obtém alvos para uma ação baseada em sua propriedade targ
 * Parâmetros: npc - O NPC executando a ação
 *             action - A configuração da ação
 * Retorno: Array de alvos
 */
function GetActionTargets(npc, action)
    var targets := array{};

    var target_type := action.target;
    
    // Determina alvos baseados na propriedade target
    if (!target_type || target_type == "Self")
        targets.append(npc);
    elseif (target_type == "Opponent")
        var opponent := npc.opponent;
        if (opponent)
            targets.append(opponent);
        endif
    elseif (target_type == "All Opponents")
        targets := ListHostiles(npc, 8, LH_FLAG_LOS);
    endif
    
    return targets;
endfunction

function ExecuteTeleportAction(npc, targets, action)
    if (!action.x || !action.y)
        return 0;
    endif

    var x := CInt(action.x);
    var y := CInt(action.y);
    var z := CInt(action.z);
    
    var success := 0;
    foreach targ in targets
        var move_result := MoveObjectToLocation(targ, x, y, z, targ.realm, MOVEOBJECT_FORCELOCATION);
        if (move_result)
            success := 1;
        else
            PrintTextAbove(npc, "Falha ao teleportar: " + move_result.errortext);
        endif
    endforeach
    
    if (!targets.size())
        PrintTextAbove(npc, "Teleporte falhou - Nenhum alvo válido encontrado");
    endif
    
    return success;
endfunction

function ExecutePerformAction(targets, action)
    if (!action.id)
        return 0;
    endif
    
    var success := 0;
    foreach targ in targets
        PerformAction(targ, CInt(action.id));
        success := 1;
    endforeach
    
    return success;
endfunction

function ExecuteTextAction(npc, action)
    if (!action.text)
        return 0;
    endif
    
    // Cor padrão se não especificada
    var color := action.color ? CInt(action.color) : 1153;
    if (action.target)
        var targets := GetActionTargets(npc, action);
        foreach targ in targets
            PrintTextAbove(targ, action.text, color);
        endforeach
    else
        PrintTextAbove(npc, action.text, color);
    endif

    return 1;
endfunction

function ExecutePlaySoundAction(npc, action)
    if (!action.id)
        return 0;
    endif
    
    PlaySoundEffect(npc, CInt(action.id));
    return 1;
endfunction

function ExecuteDelayAction(action)
    var delay_time := action.delay ? CInt(action.delay) : 3;
    sleep(delay_time);
    return 1;
endfunction

function ExecuteHealAction(targets, action)
    if (!action.healamt)
        return 0;
    endif
    
    var heal_amount := CInt(action.healamt);
    var success := 0;
    
    foreach targ in targets
        HealDamage(targ, heal_amount);
        PlayObjectCenteredEffect(targ, 0x376A, 10, 10);
        success := 1;
    endforeach
    
    return success;
endfunction

function ExecuteDamageAction(targets, action)
    if (!action.dmg)
        return 0;
    endif
    
    var damage := CInt(action.dmg);
    var damage_types := array{};
    
    // Trata ambos os formatos antigo e novo dos tipos de dano
    if (action.dmgtypes && action.dmgtypes.size())
        damage_types := action.dmgtypes;
    elseif (action.dmgtype)
        damage_types.append(action.dmgtype);
    else
        damage_types.append("Physical"); // Padrão
    endif
    
    // Obtém os vitais a afetar
    var vitals := action.vitals;
    if (!vitals || !vitals.size())
        vitals := array{"Hits"}; // Padrão para Hits se não especificado
    endif
    
    var success := 0;
    
    foreach targ in targets
        
        // Processa cada vital
        foreach vital in vitals
            if (vital == "Hits")
                // Para Hits, aplica cada tipo de dano separadamente
                foreach damage_type in damage_types
                    // Usa DamageFLS para tratamento adequado do tipo de dano
                    var dmg_type;
                    case (damage_type)
                        "Physical": dmg_type := DMG_PHYSICAL; break;
                        "Fire":     dmg_type := DMG_FIRE; break;
                        "Cold":     dmg_type := DMG_COLD; break;
                        "Poison":   dmg_type := DMG_POISON; break;
                        "Energy":   dmg_type := DMG_ENERGY; break;
                        default:    dmg_type := DMG_PHYSICAL;
                    endcase
                    
                    DamageFLS(targ, damage, dmg_type);
                endforeach
            elseif (vital == "Mana")
                AP_ConsumeVital(targ, MANA, damage);
            elseif (vital == "Stam")
                AP_ConsumeVital(targ, STAMINA, damage);
            endif
        endforeach
            
        success := 1;
    endforeach
    
    return success;
endfunction

function ExecuteGraphicEffectAction(npc, targets, action)
    if (!action.fxid)
        return 0;
    endif
    
    var fx_id := CInt(action.fxid);
    var speed := action.speed ? CInt(action.speed) : 7;
    var loops := action.loops ? CInt(action.loops) : 5;
    var success := 0;
    
    // Obtém tipos de efeito
    var effect_types := array{};
    
    // Trata ambos os formatos antigo e novo dos tipos de efeito
    if (action.graphictypes && action.graphictypes.size())
        effect_types := action.graphictypes;
    elseif (action.graphictype)
        effect_types.append(action.graphictype);
    else
        effect_types.append("Stationary"); // Padrão
    endif
    
    foreach targ in targets
        foreach effect_type in effect_types
            if (effect_type == "Moving")
                // Obtém a direção (padrão para NPC -> Alvo se não especificado)
                var direction := action.effect_direction ? action.effect_direction : "NpcToTarget";
                
                if (direction == "NpcToTarget")
                    // Efeito em movimento do NPC para o alvo
                    PlayMovingEffect(npc, targ, fx_id, speed, loops);
                else // TargetToNpc
                    // Efeito em movimento do alvo para o NPC
                    PlayMovingEffect(targ, npc, fx_id, speed, loops);
                endif
            else // Stationary
                // Efeito estacionário no alvo
                PlayObjectCenteredEffect(targ, fx_id, speed, loops);
            endif
            success := 1;
        endforeach
    endforeach
    
    return success;
endfunction

function ExecuteSummonNpcsAction(npc, action)
    if (!action.npctemplate)
        return 0;
    endif
    
    // Inicializa target se não definido (esta é a correção chave)
    if (!action.target)
        action.+target := "Self"; // Padrão para Self para invocar
    endif
    
    var template := action.npctemplate;
    var amount := action.amount ? CInt(action.amount) : 3;
    var range := action.range ? CInt(action.range) : 5;
    var lifetime := action.tempo ? CInt(action.tempo) : 120;
    var success := 0;
    
    // Sempre usa a localização do NPC (localização do chefe) como ponto de spawn
    var spawn_x, spawn_y, spawn_z;
    spawn_x := npc.x;
    spawn_y := npc.y;
    spawn_z := npc.z;
    
    // Gera os NPCs
    var spawned_count := 0;
    for i := 1 to amount
        // Encontra um ponto de spawn válido
        var loc := FindValidLocation(npc, spawn_x, spawn_y, spawn_z, range);
        if (loc)
            // Usa CreateNpcWithPrefix para lidar com diferentes prefixos de pacote
            var summoned := CreateNpcWithPrefix(template, loc.x, loc.y, loc.z, npc.realm);
            
            if (summoned && !summoned.errortext)
                // Define propriedade de invocado
                SetObjProperty(summoned, "summoned", 1);
                SetObjProperty(summoned, "summoner", npc.serial);
                
                // Define tempo de vida se especificado
                if (lifetime > 0)
                    SetObjProperty(summoned, "lifetime", ReadGameClock() + lifetime);
                    // Inicia nosso script de tempo de vida personalizado
                    start_script(":epicspawn:scripts/npclifetime", {summoned, lifetime});
                endif
                
                // Se o chefe estiver em combate, define o invocado para atacar o mesmo alvo
                if (npc.opponent)
                    summoned.SetOpponent(npc.opponent);
                endif
                
                spawned_count += 1;
                success := 1;
            endif
        endif
    endfor
    
    return success;
endfunction

function ExecuteCreateItemsAction(npc, action)
    if (!action.objtype)
        return 0;
    endif
    
    var objtype := action.objtype;
    var amount := action.amount ? CInt(action.amount) : 1;
    var success := 0;
    
    // Cria o item aos pés do NPC
    var item := CreateItemAtLocation(npc.x, npc.y, npc.z, objtype, amount, npc.realm);
    if (item)
        success := 1;
    endif
    
    return success;
endfunction

function ExecutePushbackAction(npc, targets, action)
    if (!targets.size())
        return 0;
    endif
    
    // Obtém parâmetros de empurrão
    var dist := action.distance ? CInt(action.distance) : 3;
    var delay := action.delay ? CInt(action.delay) : 100;
    var invert := action.invert ? 1 : 0;
    var success := 0;
    
    foreach targ in targets
        // Chama a função PushBack de utils.inc
        var result := PushBack(targ, dist, delay, invert);
        if (result)
            success := 1;
        endif
    endforeach
    
    return success;
endfunction

function ExecuteTimedScriptAction(npc, targets, action)
    if (!action.name)
        return 0;
    endif
    var duration := action.duration ? CInt(action.duration) : 30;
    var level := action.level ? CInt(action.level) : 1;
    var success := 0;
    foreach targ in targets
        if (targ && !targ.dead)
            var result := TS_StartTimer(targ, action.name, duration, level);
            if (TypeOf(result) != "Error")
                success := 1;
            endif
        endif
    endforeach
    return success;
endfunction

/*
 * Funções Auxiliares
 */

function FindValidLocation(npc, x, y, z, range)
    var attempts := 0;
    var max_attempts := 10;
    
    while (attempts < max_attempts)
        var try_x := x + RandomInt(range*2+1) - range;
        var try_y := y + RandomInt(range*2+1) - range;
        var try_z := GetMapInfo(try_x, try_y, npc.realm).z;

        if (CheckLosAt(npc, try_x, try_y, try_z))
            return struct{ "x" := try_x, "y" := try_y, "z" := try_z };
        endif
        
        attempts += 1;
    endwhile
    
    return 0;
endfunction