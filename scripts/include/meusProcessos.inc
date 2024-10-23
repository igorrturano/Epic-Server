use uo;
use os;

enum eventosMeusProcessos
	EV_SPELLCAST := "evSpellCast"
endenum

const PROCESSOS_OBJ_NAME := "meusProcessos";

/* 
 * IniciaMeuProcesso(quem, funcao, params)
 *
 * Propósito:
 * inicia um processo dentro de uma pool (ObjProp).
 *
 * Parâmetros:
 * quem:	mobile onde o processo será iniciado.
 * funcao:	FunctObj com a lógica do script que será rodado
 * params:	parametros da função.
 *
 * Retorno:
 * o mesmo da função executada.
 *
 */
function IniciaMeuProcesso(quem,funcao,params := array)

	Set_Critical(1);
	var arrayProcessos := GetObjProperty(quem,PROCESSOS_OBJ_NAME);
	if(!arrayProcessos)
		arrayProcessos := array;
		SetObjProperty(quem,PROCESSOS_OBJ_NAME, arrayProcessos);
	endif

	var esteProcesso := GetProcess().pid;
	arrayProcessos.append(esteProcesso);
	SetObjProperty(quem,PROCESSOS_OBJ_NAME, arrayProcessos);
	Set_Critical(0);

	var retorno := funcao.call(params);

	Set_Critical(1);
	arrayProcessos := GetObjProperty(quem,PROCESSOS_OBJ_NAME);
	if(arrayProcessos)
		arrayProcessos := LimpaProcessosInativos(arrayProcessos); //limpa todos os processos inativos, incluindo o atual.
		SetObjProperty(quem,PROCESSOS_OBJ_NAME, arrayProcessos);
	endif
	Set_Critical(0);

	return retorno;
endfunction

/* 
 * LimpaProcessosInativos(quem, funcao, params)
 *
 * Propósito:
 * limpa o array de processos, tirando os que estiverem inativos e o atual que acaba de finalizar.
 *
 * Parâmetros:
 * arrayProcessos:	Array de processos
 *
 * Retorno:
 * um novo array sem os processos inativos e o atual.
 *
 */
function LimpaProcessosInativos(arrayProcessos)

	var restantes := array; 
	var i;
	for(i := 1; i <= arrayProcessos.size(); i++)
		var processo := GetProcess(arrayProcessos[i]);
		if (processo && arrayProcessos[i] != processo.pid)
			restantes.append(arrayProcessos[i]);
		endif
		Sleepms(2);
	endfor
	return restantes;
endfunction

/* 
 * DistribuiEvento(quem, rv)
 *
 * Propósito:
 * Dispara um evento para todos os processos dentro do pool de processos(ObjProp) do mobile informado.
 *
 * Parâmetros:
 * quem:	Mobile alvo
 * ev:	Objeto de evento desejado.
 *
 *
 */

function DistribuiEvento(quem,ev)
	var arrayProcessos := GetObjProperty(quem,PROCESSOS_OBJ_NAME);
	if(arrayProcessos)
		foreach idProcesso in arrayProcessos
			var processo := GetProcess(idProcesso);
			processo.SendEvent(ev);
		endforeach
	endif
endfunction
