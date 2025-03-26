
include ":crafting:/include/controladoraItensComplexosGump";
include ":crafting:/include/controladoraComponentesGump";

function FabricaItensComplexos(quem,profissao)
	EnviaItensComplexosGump(quem,profissao);
endfunction

function FabricaComponentes(quem,profissao)
	EnviaGumpComponentes(quem);
endfunction

