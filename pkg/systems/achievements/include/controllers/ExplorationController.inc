

// TODO: Fazer essa função ser chamada no WorldSaver
	// Busca o contador de passos, soma na função e apaga a prop
function AchievementExploration_Advance(player, explorationInfo)
	
endfunction


/*
 * IncrementStepCounter(player)
 *
 * Incrementa o contador de passos do jogador para o sistema de achievement de exploração.
 * 
 * Esta função recupera o número atual de passos do jogador armazenado
 * na propriedade "#steps", incrementa em 1 e verifica se atingiu o limite
 * de 50 passos. Se a propriedade não existir, ela é inicializada com 0.
 * Quando o jogador atinge 50 passos, o achievement de exploração é avançado
 * e o contador é resetado, caso contrário, o novo valor é salvo.
 * 
 * @param player O objeto jogador cujo contador de passos será incrementado
 */
function IncrementStepCounter(player)
    var steps := player.GetProp("#steps");
	if (!steps)	steps := 0; endif

    steps += 1;

	if (steps > 50) // Se chegar a 50 passos, atualiza o achievement de exploração e apaga a prop pra recomeçar a contagem
		AchievementExploration_Advance(player, struct{ steps := steps });
		player.EraseProp("#steps");
	else
    	player.SetProp("#steps", steps);
	endif
endfunction