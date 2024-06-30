/*CRIADO PARA SUBSTITUIR O RANDOM PORCO DO POL - By EvilLink
MODO DE USAR:
1. Random(VALORMAXIMO) -> Rola de 0 a VALORMAXIMO
2. Random(VALORMINIMO, VALORMAXIMO) -> Rola de VALORMINIMO a VALORMAXIMO
3. Random(VALORMINIMO, VALORMAXIMO, SEED) -> Rola de VALORMINIMO a VALORMAXIMO com um SEED específico (para testar um valor fixo)
*/
use math;
use basic;
use util;
include "arrays";
include "varutils";

function ELRandom(who, val1, val2, val3)
    //TALVEZ usar outro seed baseado em valores quadráticos ou outro método para adicionar mais variedade (a conferir de acordo com os testes)

	var valmin, valmax, valseed;

	if (!val2)
		valmin := 0;
		valmax := val1;
	else
		valmin := val1;
		valmax := val2;
	endif

    if (!valmax) return 0; endif
	if (!valmin) valmin := 0; endif
	if (!val3)
		valseed := polcore().systime;
		valseed := CStr(valseed); valseed := valseed[Len(valseed)-3, 4]; valseed := CInt(valseed); //pegar últimos 4 números
	endif

	valmax := CInt(valmax);
	valmin := CInt(valmin);
	var tamvalmax := len(cstr(valmax));
	var tamvalseed := len(cstr(valseed));
	var valores := tamvalmax * 4;
	var a := 4194304; //2^22
	var b := 8388608; //2^23
    var c := 16777216; //2^24

	//Embaralhar seeds na quantidade de valores e preencher array
	var embseeds := array;
	embseeds.append(valseed);
	for i:=2 to valores
		valseed := embValSeed(valseed);
		embseeds.append(valseed);
	endfor
	
	//Fazer a soma para usar de seed real
	var soma := 0;
	for i:=1 to valores
		soma := soma + embseeds[i];
	endfor
	valseed := soma; //Aqui sai o valor do seed para ser utilizada na fórmula

	var valor := (b * valseed + c) / a; //Calcula o valor
	var resultado := enquadraValor(valor, valmin, valmax, valseed); //Encaixa o valor no range recebido
	
    return resultado;
endfunction

function embValSeed(seed)
	var strSeed := CStr(seed);
	var tamanho := Len(strSeed);
	var qtd := tamanho / 2;
	var embarrseed := array;

	var ultimo := tamanho;
	for i:=1 to qtd
		var ultimonumero := CInt(strseed[ultimo]);
		embarrseed.append(ultimonumero);
		var primeironumero := CInt(strseed[i]);
		embarrseed.append(primeironumero);
		ultimo := ultimo - 1;
	endfor

	var embseed := CInt("".Join(embarrseed));

	return embseed;
endfunction

function enquadraValor(valor, valmin, valmax, valseed)
	var mult := 1;
	while ((valor < valmin) || (valor > valmax))
		if (valor > valmax)
			valor := valor - (valseed * mult);
		else
			valor := valor + (valseed * mult);
		endif
		mult := mult * 0.8;
	endwhile

	return valor;
endfunction