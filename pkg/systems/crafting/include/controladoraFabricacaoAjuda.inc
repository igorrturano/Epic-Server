use uo;
use cfgfile;

include ":crafting:gumps/fabricacaoGumps";

const FABRICACAO_AJUDA_CONFIG_PATH := ":crafting:config/ajuda";

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
