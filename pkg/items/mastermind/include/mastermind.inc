use uo;
use os;
use math;

include ":attributes:attributes";
include ":charactercreation:habilidades";
include ":gumps:gumps";
include ":gumps:yesNo";
include "include/say";

var stones := {6255, 6250, 6251, 6252, 6253, 6254, 6249, 6256};
var stands := {"", "", "", "", ""};
var last := {};
var prev_attempts := array{}; // Array para armazenar tentativas anteriores
var max_stored_attempts := 10; // Número máximo de tentativas armazenadas
var enigma := {};
var counter := 10;
var marked_colors := dictionary{}; // Dicionário para armazenar cores marcadas

function mastermind(who)
    enigma := RandomSet(who);
    
    // Inicializar dicionário de cores marcadas
    foreach stone in stones
        marked_colors[stone] := 0; // 0 = não marcada, 1 = marcada
    endforeach
    
    while(who.connected)
        if(counter < 1)
            SendSysMessageEx(who, "Você falhou em resolver o enigma!", SSM_FAIL);
            return 0; // Retorna 0 para falha, fazendo o container desaparecer
        endif
        
        var gump := GFCreateGump(50, 50, 720, 400);
        
        GFResizePic(gump, 0, 0, 3500, 720, 400);
        
        GFGumpPic(gump, 30, 30, 103);
        
        GFTextLine(gump, 100, 50, 998, "A");
        GFTextLine(gump, 85, 70, 998, "Puzzle");
        GFTextLine(gump, 90, 90, 998, "Lock");
        
        GFTextLine(gump, 200, 40, 194, "Escolha a sequencia correta dos cilindros");
        GFTextLine(gump, 200, 60, 194, "necessaria para abrir a tranca. Cada cilindro");
        GFTextLine(gump, 200, 80, 194, "pode ser utilizado mais de uma vez.");
        GFTextLine(gump, 200, 100, 194, "Cuidado! Uma tentativa errada pode ser fatal!");
        
        GFTextLine(gump, 30, 370, 37, "Tentativas restantes: " + counter);
        
        //---------------------------------------------------------------------
        
        // Cores do lado esquerdo com posição corrigida
        GFResizePic(gump, 30, 150, 5100, 30, 30);
        GFAddButton(gump, 70, 160, 5032, 5032, GF_CLOSE_BTN, 11);  // button left
        GFTilePic(gump, 25, 160, 6255);        // white
        // Botão para marcar/desmarcar cor branca
        GFAddButton(gump, 70, 140, 3, 4, GF_CLOSE_BTN, 21);
        // Se a cor estiver marcada, mostrar um X sobre ela
        if(marked_colors[6255] == 1)
            GFTextLine(gump, 25, 155, 38, "X");
        endif
        
        GFResizePic(gump, 30, 200, 5100, 30, 30);
        GFAddButton(gump, 70, 210, 5032, 5032, GF_CLOSE_BTN, 12);  // button left
        GFTilePic(gump, 25, 210, 6250);        // blue
        // Botão para marcar/desmarcar cor azul
        GFAddButton(gump, 70, 190, 3, 4, GF_CLOSE_BTN, 22);
        if(marked_colors[6250] == 1)
            GFTextLine(gump, 25, 205, 38, "X");
        endif
        
        GFResizePic(gump, 30, 250, 5100, 30, 30);
        GFAddButton(gump, 70, 260, 5032, 5032, GF_CLOSE_BTN, 13);  // button left
        GFTilePic(gump, 25, 260, 6251);        // green
        // Botão para marcar/desmarcar cor verde
        GFAddButton(gump, 70, 240, 3, 4, GF_CLOSE_BTN, 23);
        if(marked_colors[6251] == 1)
            GFTextLine(gump, 25, 255, 38, "X");
        endif
        
        GFResizePic(gump, 30, 300, 5100, 30, 30);
        GFAddButton(gump, 70, 310, 5032, 5032, GF_CLOSE_BTN, 14);  // button left
        GFTilePic(gump, 25, 310, 6252);        // orange
        // Botão para marcar/desmarcar cor laranja
        GFAddButton(gump, 70, 290, 3, 4, GF_CLOSE_BTN, 24);
        if(marked_colors[6252] == 1)
            GFTextLine(gump, 25, 305, 38, "X");
        endif
        
        //--------------------------------------------------------------------
        
        // Cores do lado direito com posição corrigida
        GFResizePic(gump, 440, 150, 5100, 30, 30);
        GFAddButton(gump, 420, 160, 5032, 5032, GF_CLOSE_BTN, 15);  // button right
        GFTilePic(gump, 435, 160, 6253);        // purple
        // Botão para marcar/desmarcar cor roxa
        GFAddButton(gump, 420, 140, 3, 4, GF_CLOSE_BTN, 25);
        if(marked_colors[6253] == 1)
            GFTextLine(gump, 435, 155, 38, "X");
        endif
        
        GFResizePic(gump, 440, 200, 5100, 30, 30);
        GFAddButton(gump, 420, 210, 5032, 5032, GF_CLOSE_BTN, 16);  // button right
        GFTilePic(gump, 435, 210, 6254);        // red
        // Botão para marcar/desmarcar cor vermelha
        GFAddButton(gump, 420, 190, 3, 4, GF_CLOSE_BTN, 26);
        if(marked_colors[6254] == 1)
            GFTextLine(gump, 435, 205, 38, "X");
        endif
        
        GFResizePic(gump, 440, 250, 5100, 30, 30);
        GFAddButton(gump, 420, 260, 5032, 5032, GF_CLOSE_BTN, 17);  // button right
        GFTilePic(gump, 435, 260, 6249);        // black
        // Botão para marcar/desmarcar cor preta
        GFAddButton(gump, 420, 240, 3, 4, GF_CLOSE_BTN, 27);
        if(marked_colors[6249] == 1)
            GFTextLine(gump, 435, 255, 38, "X");
        endif
        
        GFResizePic(gump, 440, 300, 5100, 30, 30);
        GFAddButton(gump, 420, 310, 5032, 5032, GF_CLOSE_BTN, 18);  // button right
        GFTilePic(gump, 435, 310, 6256);        // yellow
        // Botão para marcar/desmarcar cor amarela
        GFAddButton(gump, 420, 290, 3, 4, GF_CLOSE_BTN, 28);
        if(marked_colors[6256] == 1)
            GFTextLine(gump, 435, 305, 38, "X");
        endif
        
        //---------------------------------------------- previous guess ------
        
        GFTextLine(gump,120, 215, 188, "Sua tentativa anterior: ");
        GFResizePic(gump,260,210,5100,120,30);
        
        if(last[1])
            GFTilePic(gump, 260, 220, last[1]);
        endif
        if(last[2])
            GFTilePic(gump, 280, 220, last[2]);
        endif
        if(last[3])
            GFTilePic(gump, 300, 220, last[3]);
        endif
        if(last[4])
            GFTilePic(gump, 320, 220, last[4]);
        endif
        if(last[5])
            GFTilePic(gump, 340, 220, last[5]);
        endif
        
        // Adicionar indicadores para a última tentativa
        if(prev_attempts.size() > 0)
            var last_entry := prev_attempts[prev_attempts.size()];
            GFTextLine(gump, 390, 215, 63, CStr(last_entry.hits)); // Posição correta
            GFTextLine(gump, 410, 215, 53, CStr(last_entry.colors)); // Cor correta
        endif
        
        //----------------------------------------- stands -----------
        
        GFTilePic(gump, 107, 260, 2818);
        GFTilePic(gump, 109, 259, 2818);
        GFTilePic(gump, 113, 259, 2816);
        GFTilePic(gump, 110, 262, 2816);
        GFTilePic(gump, 110, 260, 1310);
        
        GFTilePic(gump, 167, 260, 2818);
        GFTilePic(gump, 169, 259, 2818);
        GFTilePic(gump, 173, 259, 2816);
        GFTilePic(gump, 170, 262, 2816);
        GFTilePic(gump, 170, 260, 1310);
        
        GFTilePic(gump, 227, 260, 2818);
        GFTilePic(gump, 229, 259, 2818);
        GFTilePic(gump, 233, 259, 2816);
        GFTilePic(gump, 230, 262, 2816);
        GFTilePic(gump, 230, 260, 1310);
        
        GFTilePic(gump, 287, 260, 2818);
        GFTilePic(gump, 289, 259, 2818);
        GFTilePic(gump, 293, 259, 2816);
        GFTilePic(gump, 290, 262, 2816);
        GFTilePic(gump, 290, 260, 1310);
        
        GFTilePic(gump, 347, 260, 2818);
        GFTilePic(gump, 349, 259, 2818);
        GFTilePic(gump, 353, 259, 2816);
        GFTilePic(gump, 350, 262, 2816);
        GFTilePic(gump, 350, 260, 1310);
        
        //------------------------------------------------- stones on stands
        if(stands[1])
            GFTilePic(gump, 110, 275, stands[1]);
        endif
        if(stands[2])
            GFTilePic(gump, 170, 275, stands[2]);
        endif
        if(stands[3])
            GFTilePic(gump, 230, 275, stands[3]);
        endif
        if(stands[4])
            GFTilePic(gump, 290, 275, stands[4]);
        endif
        if(stands[5])
            GFTilePic(gump, 350, 275, stands[5]);
        endif
        
        // ---------------------------------------------------- radio under stands
        GFSetRadioGroup(gump, 1);
        GFRadioButton(gump, 118, 325, 2151, 2154, 0, 101);
        GFRadioButton(gump, 178, 325, 2151, 2154, 0, 102);
        GFRadioButton(gump, 238, 325, 2151, 2154, 0, 103);
        GFRadioButton(gump, 298, 325, 2151, 2154, 0, 104);
        GFRadioButton(gump, 358, 325, 2151, 2154, 0, 105);
        
        // ---------------------------------------------------- Histórico de tentativas anteriores
        // Mostrar todas as tentativas anteriores, sem limitação por skill
        var display_count := prev_attempts.size();
        
        // Adicionar legenda
        GFTextLine(gump, 450, 360, 63, "Posição correta");
        GFTextLine(gump, 450, 380, 53, "Cor correta");
        
        // Exibir histórico de tentativas - SEM LIMITAÇÃO POR SKILL
        if(display_count > 0)
            GFResizePic(gump, 510, 40, 3500, 200, 320);
            GFTextLine(gump, 520, 50, 37, "Histórico de Tentativas:");
            
            var i := 1;
            
            // Exibir tentativas em posições fixas, da mais recente para a mais antiga
            while(i <= display_count)
                var y_pos := 80 + ((i-1) * 40);
                var history_idx := prev_attempts.size() - (i-1);
                var entry := prev_attempts[history_idx];
                
                GFTextLine(gump, 520, y_pos, 194, "Tentativa #" + history_idx + ":");
                GFResizePic(gump, 520, y_pos + 15, 5100, 120, 30);                
                
                // Mostrar as pedras da tentativa
                var j := 1;
                while(j <= 5)
                    if(entry.attempt[j])
                        GFTilePic(gump, 520 + ((j-1) * 20), y_pos + 22, entry.attempt[j]);
                    endif
                    j := j + 1;
                endwhile
                
                // Mostrar os resultados já armazenados com a tentativa
                GFTextLine(gump, 630, y_pos + 22, 63, CStr(entry.hits)); // Posição correta
                GFTextLine(gump, 650, y_pos + 22, 53, CStr(entry.colors)); // Cor correta
                
                i := i + 1;
            endwhile
        endif
        
        GFAddButton(gump, 225, 365, 2130, 2129, GF_CLOSE_BTN, 999);  //  O K
        
        var Res := GFSendGump(who, gump);
        
        if(Res[0] < 1)
            if(YesNo(who, "Realmente cancelar?"))
                SendSysMessageEx(who, "Você cancelou o puzzle!", SSM_FAIL);
                return 0; // Retorna 0 para cancelamento, fazendo o container desaparecer
            else
                continue;
            endif
        endif
        
        // Processar botões de marcar/desmarcar cores
        var button_id := Res[0];
        if(button_id >= 21 && button_id <= 28)
            var stone_index := button_id - 20;
            var stone_id := stones[stone_index];
            // Alternar estado de marcação
            if(marked_colors[stone_id] == 0)
                marked_colors[stone_id] := 1;
                SendSysMessageEx(who, "Cor marcada como descartada!", SSM_INFO);
            else
                marked_colors[stone_id] := 0;
                SendSysMessageEx(who, "Marca removida da cor!", SSM_INFO);
            endif
        endif
        
        if(Res[0] >= 11 && Res[0] <= 18)
            foreach key in (Res.keys)
                if(key >= 101 && key <= 105)
                    stands[key-100] := stones[Res[0]-10];
                endif
            endforeach
        endif
        
        if(Res[0] == 999)
            if(stands[1] && stands[2] && stands[3] && stands[4] && stands[5])
                counter := counter - 1;
                
                // Calcular o resultado da tentativa atual
                var result := GetAttemptResult(stands);
                
                // Mostrar feedback para tentativa atual
                sleep(1);
                SendSysMessageEx(who, result.desc_hits, SSM_INFO);
                SendSysMessageEx(who, result.desc_colors, SSM_INFO);
                sleep(1);
                
                // Armazenar tentativa e resultado juntos
                StorePreviousAttempt(stands, result);
                
                // Atualizar para próxima rodada
                last := stands;
                stands := {"", "", "", "", ""};
                
                // Verificar vitória
                if(result.hits == 5)
                    SendSysMessageEx(who, "Parabéns! Você descobriu a combinação correta!", SSM_INFO);
                    return 1; // Retorna 1 para sucesso, mostrando o conteúdo do container
                endif
            endif
        endif
    endwhile
    
    // Se o jogador se desconectar, retorna 0 para fazer o container desaparecer
    return 0;
endfunction

// Função para armazenar tentativa e seu resultado
function StorePreviousAttempt(attempt, result)
    // Criar um struct que contém tanto a tentativa quanto seu resultado
    var entry := struct;
    
    // Copiar a tentativa
    entry.+attempt := array{};
    foreach stone in attempt
        entry.attempt.append(stone);
    endforeach
    
    // Armazenar o resultado
    entry.+hits := result.hits;
    entry.+colors := result.colors;
    
    // Adicionar a entrada combinada ao nosso histórico
    prev_attempts.append(entry);
    
    // Manter limite de tamanho do histórico
    if(prev_attempts.size() > max_stored_attempts)
        prev_attempts.erase(1);
    endif
endfunction

function RandomSet(who)
    var temp := {};
    temp[1] := stones[RandomInt(8)+1];
    sleepms(10);
    temp[2] := stones[RandomInt(8)+1];
    sleepms(10);
    temp[3] := stones[RandomInt(8)+1];
    sleepms(10);
    temp[4] := stones[RandomInt(8)+1];
    sleepms(10);
    temp[5] := stones[RandomInt(8)+1];
    sleepms(10);
    
    //developer debug  
    if(who.cmdlevel > 4)
        var gump := GFCreateGump(50, 50, 200, 100);
        GFResizePic(gump,0,0,3500,200,100);
        GFTilePic(gump, 30, 30, temp[1]);
        GFTilePic(gump, 60, 30, temp[2]);
        GFTilePic(gump, 90, 30, temp[3]);
        GFTilePic(gump, 120, 30, temp[4]);
        GFTilePic(gump, 150, 30, temp[5]);
        GFSendGump(who, gump);
    endif
    
    return temp;
endfunction

// Função de resultado da tentativa
function GetAttemptResult(attempt_stands)
    var hits := 0;
    var colors := 0;
    var desc_hits;
    var desc_colors;
    
    // Cria cópias das arrays para não modificar os originais
    var enigma_copy := array{};
    var stands_copy := array{};
    var j := 1;
    
    while(j <= 5)
        enigma_copy.append(enigma[j]);
        stands_copy.append(attempt_stands[j]);
        j := j + 1;
    endwhile
    
    // Primeiro verifique as posições corretas
    var x := 1;
    while(x <= 5)
        if(stands_copy[x])
            if(stands_copy[x] == enigma_copy[x])
                hits := hits + 1;
                enigma_copy[x] := -10;
                stands_copy[x] := -20;
            endif
        endif
        x := x + 1;
    endwhile
    
    // Depois verifique as cores corretas em posições erradas
    x := 1;
    while(x <= 5)
        if(stands_copy[x] && stands_copy[x] != -20)
            var idx := FindInArray(enigma_copy, stands_copy[x]);
            if(idx)
                colors := colors + 1;
                enigma_copy[idx] := -10;
                stands_copy[x] := -20;
            endif
        endif
        x := x + 1;
    endwhile
    
    case(hits)
        0:         desc_hits := "0 elementos em posicao correta.";
        1:         desc_hits := "1 elemento em posicao correta.";
        default:    desc_hits := CInt(hits)+" elementos estao em posicoes corretas.";
    endcase
    
    case(colors)
        0:         desc_colors := "0 elemento em cor correta.";
        1:         desc_colors := "1 elemento em cor correta.";
        default:    desc_colors := CInt(colors)+" elementos estao em cores corretas.";
    endcase
    
    // Cria e retorna um struct com os resultados
    var result := struct;
    result.+hits := hits;
    result.+colors := colors;
    result.+desc_hits := desc_hits;
    result.+desc_colors := desc_colors;
    
    return result;
endfunction