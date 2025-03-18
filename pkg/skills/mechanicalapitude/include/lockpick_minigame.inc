//ATENÇÃO, O MINIGAME NÃO TEM SKILLCHECK, LOGO NÃO UPA
use uo;
use os;
use math;
use util;

include ":attributes:attributes";
include ":charactercreation:habilidades";
include ":gumps:/include/epicGumps";
//1640
// Constantes de mensagem
const SSM_SUCCESS := 64;

// Pinos de lockpick (10 pinos)
var pins := {6249, 6250, 6251, 6252, 6253, 6254, 6255, 6256, 6257, 6258};
var sweet_spots := array{}; // Posições corretas dos pinos
var current_attempts := array{}; // Tentativas atuais do jogador
var max_attempts := 5; // Número máximo de tentativas
var difficulty := 3; // Dificuldade base (pode ser ajustada)
var tension := 0.0; // Tensão atual do lockpick
var correct_pins := array{}; // Rastrear pinos acertados

function CalculatePinCount(who, diff, mec_skill)
    // Calcula a diferença de skill
    var skill_diff := diff - mec_skill;
    
    // Base de pinos começa em 3
    var base_pins := 3;
    
    // Quanto MAIOR a diferença, MAIS pinos
    // Adiciona pinos baseado na diferença de skill
    var pin_variation := CInt(skill_diff / 10);
    
    // Rolagem de dados para adicionar randomicidade
    var dice_roll := RandomInt(3) + 1; // d3 para reduzir variação extrema
    
    // Calcula total de pinos
    var total_pins := base_pins + pin_variation + dice_roll;
    
    // Assegura limites entre 3 e 10
    if(total_pins < 3)
        total_pins := 3;
    elseif(total_pins > 10)
        total_pins := 10;
    endif
    
    return total_pins;
endfunction

function mini_game(who, diff, mec_skill, chest)
    // Calculate dynamic number of pins
    var total_pins := CalculatePinCount(who, diff, mec_skill);
    
    // Modify pins array based on total_pins
    pins := array{};
    var i;
    for(i := 1; i <= total_pins; i++)
        pins.append(6249 + (i-1)); // Assuming original pin IDs start at 6249
    endfor
    
    // Modify sweet_spots and other arrays to match new pin count
    sweet_spots := GenerateSweetSpots(mec_skill, total_pins);
    current_attempts := array{};
    correct_pins := array{};
    tension := 0.0;
    
    // IMPORTANTE: Reiniciar a contagem de pinos mágicos usados ao iniciar uma nova tentativa
    if(GetObjProperty(chest, "level") == 6)
        EraseObjProperty(chest, "magic_pins_used");
    endif
    
    // Track failure reason
    var failure_reason := 0;
    
    while(who.connected)
        var gump := CreateLockpickGump(who, total_pins, chest);
        var Res := GFSendGump(who, gump);
        
        if(Res[0] < 1)
            if(YesNo(who, "Se cancelar a fechadura será quebrada!"))
                failure_reason := 1; // User cancelled
                break;
            else
                continue;
            endif
        endif
        
        // Processar interação com os pinos
        if(Res[0] >= 11 && Res[0] <= (10 + total_pins))
            var pin_index := Res[0] - 10;
            TryPickPin(who, pin_index);
        endif
        
        // Botões de magia para fechaduras level 6
        if(GetObjProperty(chest, "level") == 6)
            // Botão para liberar um pino
            if(Res[0] == 100)
                UnlockMagicalPin(who, total_pins, chest);
            endif
            
            // Botão para liberar todos os pinos possíveis
            if(Res[0] == 101)
                UnlockAllMagicalPins(who, total_pins, chest);
            endif
        endif
        
        // Verificar condição de sucesso/falha
        var game_status := CheckLockpickStatus(who, total_pins);
        
        if(game_status == 1)
            return 1; // Success
        elseif(game_status == 2)
            failure_reason := 2; // Lock broke
            break;
        endif
    endwhile
    
    // Send specific failure message
    var lvl := CInt(GetObjProperty(chest, "level"));

    //if(lvl != 6)
        if(failure_reason == 1)
            SendSysMessageEx(who, "Você cancelou a tentativa de abrir a fechadura e a quebrou!", SSM_FAIL);
            Setobjproperty(chest, "fechadura_quebrada", 1);
        elseif(failure_reason == 2)
            Setobjproperty(chest, "fechadura_quebrada", 1);
        endif

   // else
        //SendSysMessageEx(who, "A fechadura mágica reaparece!", SSM_INFO);
        // IMPORTANTE: Reiniciar a contagem de pinos mágicos usados ao falhar em uma fechadura mágica
       // EraseObjProperty(chest, "magic_pins_used");
    //endif
    return 0; // Failure
endfunction

function GenerateSweetSpots(skill, total_pins)
    var spots := array{};
    var randomness := (10 - (skill / 10)); // Quanto menor a skill, mais aleatório
    var i;
    
    for(i := 1; i <= total_pins; i++)
        // Gera uma posição "sweet spot" com base na skill
        var sweet_spot := RandomInt(10) + 1;
        spots.append(sweet_spot);
    endfor
    
    return spots;
endfunction

function CheckTensionBreakRoll(tension)
    // Rola um dado com número de lados igual à tensão, 
    // procurando tirar 1
    var break_roll := RandomDiceRoll("1d" + CInt(tension));
    
    // Retorna verdadeiro (continua) se tirar 1
    return (break_roll <= 3);
endfunction

function CheckLockpickStatus(who, total_pins)
    // Verificar se há status de falha forçada
    var forced_status := GetObjProperty(who, "lockpick_game_status");
    if(forced_status == 2)
        EraseObjProperty(who, "lockpick_game_status");
        return 2; // Lock broke
    endif
    
    // Verificar tensão a cada iteração
    if(tension > 5.0)
        // Adiciona teste de quebra
        if(!CheckTensionBreakRoll(tension))
            SendSysMessageEx(who, "Tranca quebrou! Tensão muito alta.", SSM_FAIL);
            return 2; // Lock broke
        endif
    endif
    
    // Resto do código original mantido igual
    if(current_attempts.size() < total_pins)
        return 0;
    endif
    
    var total_correct_pins := 0;
    var i;
    for(i := 1; i <= total_pins; i++)
        if(current_attempts[i] == sweet_spots[i])
            total_correct_pins += 1;
        endif
    endfor
    
    if(total_correct_pins == total_pins)
        SendSysMessageEx(who, "Fechadura aberta com sucesso!", SSM_SUCCESS);
        return 1;
    endif
    
    return 0;
endfunction

function CreateLockpickGump(who, total_pins, chest)
    // Substituir GFCreateGump por GFECreateGump
    var titulo;
    var border;
    var is_magical := (GetObjProperty(chest, "level") == 6);
    
    if(is_magical)
        titulo := "Fechadura Mágica";
        border := BORDER_BLUE;
    else
        titulo := "Forçar Fechadura Avançada";
        border := BORDER_GOLDEN;
    endif
    
    var gump := GFECreateGump(titulo, 580, 420, 
        array{border, HIDE_PAGE} // Usando opções do GFECreateGump
    );
    
    // Título e instruções
    GFTextLine(gump, 180, 70, 194, "Destrave os pinos cuidadosamente");
    GFTextLine(gump, 180, 90, 194, "Encontre a pressão correta para cada pino");
    
    // Retângulo de fundo preto para os pinos
    GFResizePic(gump, 35, 120, 2620, 514, 200);
    
    // Calculando quantidade de pinos mágicos usando a função dedicada
    var ident_mana := 0;
    if(is_magical)
        ident_mana := CalculateIdentifiablePins(who, chest);
    endif
    
    // Lockpick
    if(is_magical)
        GFGumpPic(gump, 40,330, 1640);
        GFGumpPic(gump, 90,320, 1648);
        GFGumpPic(gump, 100,375, 1651);
        GFTextLine(gump, 120, 335, 194, "Você sente as vibrações da fechadura e");
        GFTextLine(gump, 120, 355, 194, "identifica ");
        GFTextLine(gump, 175, 355, 54, ident_mana);
        GFTextLine(gump, 178, 355, 194, " pontos vulneráveis, liberáveis");
        GFTextLine(gump, 120, 375, 194, "com esforço arcano [5 mana cada]");
        
        // Adiciona os botões de magia (sempre, independente da mana)
        // Botão para liberar um pino
        GFAddButton(gump, 400, 325, 2151, 2152, GF_CLOSE_BTN, 100);
        GFTextLine(gump, 435, 327, 67, "Liberar 1 pino");
        
        // Botão para liberar todos os pinos possíveis
        GFAddButton(gump, 400, 358, 2151, 2152, GF_CLOSE_BTN, 101);
        GFTextLine(gump, 435, 360, 67, "Liberar todos ("+ident_mana+")");
    else
        GFResizePic(gump, 35,320, 2620, 514, 65);
        GFResizePic(gump, 35,320, 2356, 514, 90);
    endif
    
    // Definir cor da tensão baseado no valor
    var tension_color := 37; // Cor padrão (azul como no título)
    
    if(tension <= 1.5)
        tension_color := 67; // Verde
    elseif(tension >= 1.6 && tension < 5.0)
        tension_color := 55; // Amarelo
    elseif(tension >= 5.0)
        tension_color := 33; // Vermelho
    endif

    GFTextLine(gump, 250, 395, 194, "TENSÃO ATUAL: ");
    GFTextLine(gump, 350, 395, tension_color, CInt(tension * 100) / 100.0);
    
    // Pinos com botões para posicionamento
    var x_offset := 41; // Ajustado para começar dentro do retângulo
    var i;
    for(i := 0; i < total_pins; i++)
        // Se o pino já foi acertado, mostra como pressionado
        if(correct_pins[i+1])
            GFAddButton(gump, x_offset, 128, 2355, 2355, GF_CLOSE_BTN, 0);
        else
            // Senão, mantém como botão normal
            GFAddButton(gump, x_offset, 128, 2354, 2355, GF_CLOSE_BTN, 11+i);
        endif
        
        x_offset += 50; // Mantém o espaçamento entre os botões
    endfor
    
    return gump;
endfunction

function TryPickPin(who, pin_index)
    // Não permite reposicionar pinos já acertados
    if(correct_pins[pin_index])
        return;
    endif
    
    var current_position := RandomInt(10) + 1;
    var sweet_spot := sweet_spots[pin_index];
    
    // Chance fixa de 20% de acertar o sweet spot
    var success := (RandomInt(5) == 0);
    
    // Se sucesso, força a posição para o sweet spot
    if(success)
        current_position := sweet_spot;
    endif
    
    // Calcula a tensão baseado na distância do sweet spot
    var tension_increase := Abs(current_position - sweet_spot);
    tension += tension_increase * 0.1;
    
    // Adiciona a tentativa
    current_attempts[pin_index] := current_position;
    
    // Mensagem de feedback
    if(current_position == sweet_spot)
        SendSysMessageEx(who, "Pino " + pin_index + " destravado!", SSM_SUCCESS);
        correct_pins[pin_index] := 1; // Marca como acertado
		PlaySoundEffect(who, 0x3e5);
    else
        
        var shouldBreakLockpick := 0;
        PlaySoundEffect(who, 0x3e7);
        // NOVA LÓGICA: Chance de perder lockpick quando erra o pino
        if(tension_increase == 9)  // Erro máximo: 100% de chance de quebrar
            shouldBreakLockpick := 1;
        elseif(tension_increase >= 4 && RandomInt(100) < 35)  // Erro grande (4-8): 35% de chance
            shouldBreakLockpick := 1;
        endif
        
if(shouldBreakLockpick)
    // Verificar se o jogador tem uma lockpick
    var items := EnumerateItemsInContainer(who.backpack);
    var lockpicks := array{};

    // Filter for lockpicks
    foreach item in items
        if(item.objtype == 0x14FB)
            lockpicks.append(item);
        endif
    endforeach

    if(lockpicks.size() > 0)
        // Get the first lockpick found
        var lockpick_to_break := lockpicks[1];
        
        // Tocar som de quebra
        PlaySoundEffect(who, 0x040);
        
        // Reduzir a quantidade
        SubtractAmount(lockpick_to_break, 1);
        
        // Verificar se ainda tem gazuas após quebrar uma
        //lockpicks := EnumerateItemsInContainer(backpack, 0x14FC);
        
        if(lockpicks.size() == 0)
            // Não tem mais gazuas, definir código de falha e encerrar minigame
            SendSysMessageEx(who, "Sua última gazua quebrou! A fechadura danificou-se permanentemente.", SSM_FAIL);
            SetObjProperty(who, "lockpick_game_status", 2); // Código 2 = lock broke
            return;
        else
            // Ainda tem gazuas disponíveis
            if(tension_increase == 9)
                SendSysMessageEx(who, "Você quebrou uma gazua pela pressão extrema no pino!", SSM_FAIL);
            else
                SendSysMessageEx(who, "Você quebrou uma gazua pela pressão excessiva no pino!", SSM_FAIL);
            endif
        endif
    else// Não tem gazuas, definir código de falha e encerrar minigame
        SendSysMessageEx(who, "Você não tem mais gazuas! A fechadura danificou-se permanentemente.", SSM_FAIL);
        SetObjProperty(who, "lockpick_game_status", 2); // Código 2 = lock broke
        return;
    endif
        endif
    endif
endfunction

function UnlockAllMagicalPins(who, total_pins, chest)
    // Verifica se há mana suficiente
    if(AP_GetVital(who, MANA) < 5)
        SendSysMessageEx(who, "Você não tem mana suficiente!", SSM_FAIL);
        return;
    endif
    
    // Usar a função dedicada para calcular os pontos identificáveis
    var ident_mana := CalculateIdentifiablePins(who, chest);
    
    // Verifica se ainda há pontos vulneráveis identificados
    if(ident_mana <= 0)
        SendSysMessageEx(who, "Você não identificou mais pontos vulneráveis!", SSM_FAIL);
        return;
    endif
    
    // Limita o máximo de pinos com base na mana disponível (5 mana por pino)
    var max_pins := CInt(AP_GetVital(who, MANA) / 5);
    
    // Não podemos destravar mais que os pinos identificados
    if(max_pins > ident_mana)
        max_pins := ident_mana;
    endif
    
    // Encontra pinos que ainda não foram destravados
    var pins_to_unlock := array{};
    var i;
    for(i := 1; i <= total_pins; i++)
        if(!correct_pins[i])
            pins_to_unlock.append(i);
            
            // Limita ao máximo possível com a mana disponível
            if(pins_to_unlock.size() >= max_pins)
                break;
            endif
        endif
    endfor
    
    if(pins_to_unlock.size() == 0)
        SendSysMessageEx(who, "Não há mais pinos para desbloquear!", SSM_INFO);
        return;
    endif
    
    // Consumir mana para todos os pinos
    var mana_cost := pins_to_unlock.size() * 5;
    AP_ConsumeVital(who, MANA, mana_cost);
    
    // Destrava os pinos
    foreach pin_id in pins_to_unlock
        current_attempts[pin_id] := sweet_spots[pin_id];
        correct_pins[pin_id] := 1;
    endforeach
    
    // Atualiza contador de pinos mágicos usados
    var magic_pins_used := CInt(GetObjProperty(chest, "magic_pins_used"));
    if(!magic_pins_used)
        magic_pins_used := 0;
    endif
    magic_pins_used += pins_to_unlock.size();
    SetObjProperty(chest, "magic_pins_used", magic_pins_used);
    
    SendSysMessageEx(who, "Você destrava magicamente " + pins_to_unlock.size() + " pinos!", SSM_SUCCESS);
endfunction

// Também devemos atualizar a função UnlockMagicalPin para usar nossa nova função
function UnlockMagicalPin(who, total_pins, chest)
    // Verifica se há mana suficiente
    if(AP_GetVital(who, MANA) < 5)
        SendSysMessageEx(who, "Você não tem mana suficiente!", SSM_FAIL);
        return;
    endif
    
    // Usar a função dedicada para calcular os pontos identificáveis
    var ident_mana := CalculateIdentifiablePins(who, chest);
    
    // Verifica se ainda há pontos vulneráveis identificados
    if(ident_mana <= 0)
        SendSysMessageEx(who, "Você não identificou mais pontos vulneráveis!", SSM_FAIL);
        return;
    endif
    
    // Procura um pino que ainda não foi destravado
    var found_pin := 0;
    var i;
    for(i := 1; i <= total_pins; i++)
        if(!correct_pins[i])
            found_pin := i;
            break;
        endif
    endfor
    
    if(!found_pin)
        SendSysMessageEx(who, "Não há mais pinos para desbloquear!", SSM_INFO);
        return;
    endif
    
    // Consumir mana
    AP_ConsumeVital(who, MANA, 5);
    
    // Destrava o pino
    current_attempts[found_pin] := sweet_spots[found_pin];
    correct_pins[found_pin] := 1;
    
    // Atualiza contador de pinos mágicos usados
    var magic_pins_used := CInt(GetObjProperty(chest, "magic_pins_used"));
    if(!magic_pins_used)
        magic_pins_used := 0;
    endif
    magic_pins_used += 1;
    SetObjProperty(chest, "magic_pins_used", magic_pins_used);
    
    SendSysMessageEx(who, "Você destrava magicamente o pino " + found_pin + "!", SSM_SUCCESS);
endfunction

function CalculateIdentifiablePins(who, chest)
    // Obter valores de mana e arcana
    var player_mana := AP_GetVital(who, MANA);
    var magic_lore := AP_GetSkill(who, ARCANA);
    
    // Usar o maior valor entre mana e arcana
    var identification_value;
    if(magic_lore > player_mana)
        identification_value := magic_lore;
    else
        identification_value := player_mana;
    endif
    
    // Valor de identificação dividido por 10
    var max_ident_mana := CInt(identification_value / 10);
    
    // Limitado a 10
    if(max_ident_mana > 10)
        max_ident_mana := 10;
    endif
    
    // Reduzir pela quantidade de pinos já destravados magicamente
    var used_magic_pins := CInt(GetObjProperty(chest, "magic_pins_used"));
    if(!used_magic_pins)
        used_magic_pins := 0;
    endif
    
    var ident_mana := max_ident_mana - used_magic_pins;
    if(ident_mana < 0)
        ident_mana := 0;
    endif
    
    return ident_mana;
endfunction