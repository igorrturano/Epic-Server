include ":gumps:/include/epicGumps";

class DropDown()
	
	function DropDown(this,idBottao,x,y,pid,adicionalIdValores , valorEscolhido, gerenciador)
		this.idBottao := CInt(idBottao);
		this.x := x;
		this.y := y;
		this.pid := pid;
		this.gerenciador := gerenciador;
		this.valores := array;
		this.valorEscolhido := valorEscolhido;
		this.adicionalIdValores := adicionalIdValores;
	endfunction

	function AdicionaOpcao(this,opcao,indice,valor)
		if(!opcao)
			return error{"errortext" := "parâmetro obrigatório [opcao] não informado."};
		endif

		if(this.valores.size() > this.gerenciador.maxOpcoes)
			return error{"errortext" := "Limite de opções alcançado. opcao "  + opcao + " não incluída."};
		endif
		
		var idOpcao :=  (this.PegaIdBottaoNormalizado() - 1) * this.gerenciador.maxOpcoes +  CInt(indice);
		
		var objeto := struct;
		objeto.+opcao := opcao;
		objeto.+botao := struct{isColunaBotao := true, valor := idOpcao + this.adicionalIdValores, valorReal := valor}; 
		this.valores.append(objeto);
		return 1;
	endfunction

	function RemoveOpcao(this,opcao)
		if(!opcao)
			return error{"errortext" := "parâmetro obrigatório não informado."};
		endif

		var indice := this.valores.findIndex( @(valor) {
				return valor == opcao;
		});

		if(indice)
			this.valores.erase(indice);
		endif
	endfunction

	function DefineValorEscolhido(this, idOpcaoSelecionada)
		var indiceValorSelecionado :=  (idOpcaoSelecionada - this.adicionalIdValores) % this.maxOpcoes;
		Print("indiceValorSelecionado: " + indiceValorSelecionado);
		this.valorEscolhido := this.valores[indiceValorSelecionado].botao.valorReal;
	endfunction

	function PegaTexto(this)

		return "idBottao: " + this.idBottao +
		"x: " + this.x +
		"y: " + this.y +
		"pid: " + this.pid +
		"valorEscolhido: " + this.valorEscolhido +
		"valores: " + this.valores +
		"adicionalIdValores: " + this.adicionalIdValores;

	endfunction

	function PegaIdBottaoNormalizado(this)
		return this.idBottao - this.gerenciador.adicionalIdDropDown;
	endfunction
endclass

class GerenciadorDropDown()

	//TODO criar metodo padrão onde se nenhuma das opções for informada, gera os ids dinâmicamente. [versões futuras.]
	function GerenciadorDropDown(this,maxOpcoes,adicionalIdDropDown,adicionalIdValores,maxDropDown)
		this.maxOpcoes := CInt(maxOpcoes);
		this.maxDropDown := CInt(maxDropDown);
		this.adicionalIdDropDown := CInt(adicionalIdDropDown);
		this.adicionalIdValores := CInt(adicionalIdValores);
		this.idOpcoesMaximo := this.maxOpcoes * this.maxDropDown + this.adicionalIdDropDown;
		this.dropDowns := array;
	endfunction

	function RegistraDropDown(this,idBottao, valorEscolhido := "")
		var dropDown := DropDown(idBottao + this.adicionalIdDropDown,0,0,0,this.adicionalIdValores,valorEscolhido, this);
			this.dropDowns.append(dropDown);
			this.idsRegistradas.append(dropDown.idBottao);
		return dropDown;
	endfunction

	function DefineValorEscolhido(this,idOpcaoSelecionada)

		var indiceDropDownSelecionado := CInt((idOpcaoSelecionada - this.adicionalIdValores) / this.maxOpcoes) + 1;
		if(indiceDropDownSelecionado && this.dropDowns.size() > 0 &&  indiceDropDownSelecionado <= this.dropDowns.size())
			DropDown::DefineValorEscolhido(this.dropDowns[indiceDropDownSelecionado],idOpcaoSelecionada);
		endif

		return error{"errortext" := "valor não encontrado"};
	endfunction

	function IsIdDentroOpcoes(this, id)
		return id >= this.adicionalIdValores && id <= this.idOpcoesMaximo;
	endfunction

	function IsIdDentroDropDowns(this, id)
		return id > this.adicionalIdDropDown && id <= this.maxDropDown + this.adicionalIdDropDown;
	endfunction

	function LimpaDropDowns(this)
		this.dropDowns := array;
	endfunction

endclass

function MontaDropDownGump(parx,pary,valores)
	var altura := 235;
	var gump := GFCreateGump(parx, pary, 200, altura);
	var x := 0;
	var y := 0;
	var diferencaValoresLinhas := 0;
	var opcoesTabela := dictionary{ 
							MAX_PAGINAS -> 30,
							QTD_LINHA_LIMITE -> 8
							};
	
	GFMovable(gump, 0);
	if(Valores.size() < opcoesTabela[QTD_LINHA_LIMITE])
		diferencaValoresLinhas := 20 * ( opcoesTabela[QTD_LINHA_LIMITE] - Valores.size()) + 20;
		altura :=  altura - diferencaValoresLinhas;
	endif
	GFResizePic( gump, x, y, 2620, 200, altura );
	GFAddAlphaRegion( gump, x + 5, y + 5, 190, altura - 10 );

	var colCfg := dictionary;
	colCfg["opcao"] := struct{titulo := "Opção", largura := 9};
	colCfg["botao"] := struct{titulo := "X",ordem := 1,largura := 3};
	if(valores.size() > 0)
		CriaTabelaGumpPaginada(x + 10,y + 10,190,valores,gump,opcoesTabela,colCfg);
	else
		GFTextLine(gump,x + 10,y + 10,86, "Sem opções");
	endif
	return gump;
endfunction 

function SendDropDownGump( quem, pid, objetoDropDown)
	return Start_Script( ":gumps:scripts/dropDown/dropDownGump", array{quem, pid, objetoDropDown} );
endfunction
