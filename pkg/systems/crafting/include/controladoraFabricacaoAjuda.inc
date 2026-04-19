use uo;
use cfgfile;

include ":crafting:gumps/fabricacaoGumps";
include ":crafting:include/util/fabricacaoConstants";

//TODO remover o gump de ajuda. Criar um gump que mostra as receitas aprendidas no lugar.

function EnviaFabricacaoAjudaGump(quem, profissao, idAba)
	var retorno := true;
	var parametros := PreparaFabricacaoAjudaGump(profissao, idAba);
	var gump;

	while (retorno)

		MontaFabricacaoAjudaGump(gump, parametros);
		retorno := GFSendGump(quem,gump);

	endwhile
endfunction

function PreparaFabricacaoAjudaGump(profissao, idAba)
	var parametros := struct;
		parametros.+geral := struct{ textoPrincipal :=  ReadConfigFile(FABRICACAO_AJUDA_CONFIG_PATH)["Geral"].textoPrincipal};
		parametros.+profissao := struct{
								label := profissao, 
								textoComponentes :=  ReadConfigFile(FABRICACAO_AJUDA_CONFIG_PATH)[profissao].textoComponentes,
								textoMontagem :=  ReadConfigFile(FABRICACAO_AJUDA_CONFIG_PATH)[profissao].textoMontagem,
								textoTratamento :=  ReadConfigFile(FABRICACAO_AJUDA_CONFIG_PATH)[profissao].textoTratamento
								};
		parametros.+pagina := idAba;

	return parametros;
endfunction
