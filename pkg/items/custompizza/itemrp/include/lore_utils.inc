use uo;
include ":charactercreation:habilidades";

enum FONTE_CONHECIMENTO_LINGUA
    CONHECE_LINGUA := "Você sabe ler essa língua",
	ANTIGO_ARQUEOLOGO :=  "Seu conhecimento de arqueologia decifrou a escrita",
	ARQUEOLOGO := "A arqueologia permitiu decifrar essa língua morta"
endenum

function SabeLerLinguagem(who, lingua)

	var retorno := struct;

	retorno.+sabeLer := false;
	retorno.+fonte := "";

	var linguas_conhecidas := GetObjProperty(who, "linguas_conhecidas");
	if (!linguas_conhecidas)
		linguas_conhecidas := array{};
	endif

	if (lingua in linguas_conhecidas)
		retorno.sabeLer := true;
		retorno.fonte := CONHECE_LINGUA;
	elseif (lingua == "Antiga" && temHabilidade(who, "Arqueologo"))
		retorno.sabeLer := true;
		retorno.fonte := ANTIGO_ARQUEOLOGO;
	elseif (temHabilidade(who, "Arqueologo"))
		retorno.sabeLer := true;
		retorno.fonte :=  ARQUEOLOGO;
	endif

	return retorno;
endfunction