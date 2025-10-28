//Author: Deiro || Rafael Santos

include ":gumps:/include/epicGumps";

class DropDown()

	var VALOR_ESCOLHIDO_BASE := struct; 
	
	function DropDown(this,idBottao,x,y,pid,adicionalIdOpcoes , valorEscolhido, gerenciador, largura := 200)
		this.idBottao := CInt(idBottao);
		this.x := x;
		this.y := y;
		this.largura := largura;
		this.pid := pid;
		this.gerenciador := gerenciador;
		this.itens := array;
		this.valorEscolhido := valorEscolhido;
		this.adicionalIdOpcoes := adicionalIdOpcoes;
	endfunction

	function AdicionaOpcao(this,opcao,indice,valor)

		var isValida := ValidaOpcao(this,opcao);
		if(!isValida)
			return isValida;
		endif
		
		var objetoOpcao := MontaOpcao(this, opcao, indice, valor);

		this.itens.append(objetoOpcao);
		return 1;
	endfunction

	function AdicionaOpcoes(this,dictOpcoes)

		if(TypeOfInt( dictOpcoes ) == OT_DICTIONARY)
			var indice := 1;
			foreach opcao in (dictOpcoes.keys())
				Sleepms(2);
				this.AdicionaOpcao(opcao, _opcao_iter, dictOpcoes[opcao]);
			endforeach
		else
			return error{"errortext" := "parâmetro [dictOpcoes] precisa ser do tipo dictionary."};
		endif

	endfunction

	function MontaOpcao(this,opcao,indice, valor)

		var idOpcao :=  this.CalculaIdOpcaoInicial() + CInt(indice);
		
		var objeto := struct;
		objeto.+opcao := opcao;
		objeto.+botao := struct{isColunaBotao := true, valor := idOpcao, valorReal := valor}; 

		return objeto;
	endfunction

	function CalculaIdOpcaoInicial(this)
		return (this.PegaPosicaoBottao() - 1) * this.gerenciador.maxOpcoes  + this.adicionalIdOpcoes;
	endfunction

	function ValidaOpcao(this,opcao)
		if(!opcao)
			return error{"errortext" := "parâmetro obrigatório [opcao] não informado."};
		endif

		if(this.itens.size() > this.gerenciador.maxOpcoes)
			return error{"errortext" := "Limite de opções alcançado. opcao "  + opcao + " não incluída."};
		endif

		var opcaoExists := false;
		foreach item in (this.itens)
			Sleepms(2);
			if(item.opcao == opcao)
				opcaoExists:= true;
				break;
			endif
		endforeach

		if( opcaoExists)
			return error{"errortext" := "opção "  + opcao + " já foi incluída."};
		endif
		
		return 1;
	endfunction

	function ResetaItens(this)
		this.itens := array;
		this.valorEscolhido := VALOR_ESCOLHIDO_BASE;
	endfunction

	function LimpaValorEscolhido(this)
		this.valorEscolhido := VALOR_ESCOLHIDO_BASE;
	endfunction

	function RemoveOpcao(this,opcao)
		if(!opcao)
			return error{"errortext" := "parâmetro obrigatório não informado."};
		endif

		var indice := this.itens.findIndex( @(item) {
				return item == opcao;
		});

		if(indice)
			this.itens.erase(indice);
		endif
	endfunction

	function DefineValorEscolhido(this, idOpcaoSelecionada)
		var indiceValorSelecionado :=  (idOpcaoSelecionada - this.adicionalIdOpcoes) % this.gerenciador.maxOpcoes;
		this.valorEscolhido := this._PegaValorReal(this.itens[indiceValorSelecionado]);
		return this;
	endfunction

	function ContainIdOpcao(this, idOpcaoSelecionada)
		var inicial := this.CalculaIdOpcaoInicial();
		var maximo := this.gerenciador.maxOpcoes + inicial;

		return this.CalculaIdOpcaoInicial() <= idOpcaoSelecionada && idOpcaoSelecionada < maximo;
	endfunction

	function _PegaValorReal(this, opcao)
		return  opcao.botao.valorReal;
	endfunction

	function SendGump(this, quem, pidPai)
		var process := SendDropDownGump(quem, pidPai, this);
		
		if(process)
			this.AtribuiPid(process.pid);
		endif
	endfunction

	function PegaTexto(this)

		return "idBottao: " + this.idBottao +
		" x: " + this.x +
		" y: " + this.y +
		" pid: " + this.pid +
		" valorEscolhido: " + this.valorEscolhido +
		" itens: " + this.itens +
		" adicionalIdOpcoes: " + this.adicionalIdOpcoes;

	endfunction

	function PegaPid(this)
		return this.pid;
	endfunction

	function AtribuiPid(this, pid)
		this.pid := pid;
	endfunction
	
	function PegaValorEscolhido(this)
		return this.valorEscolhido;
	endfunction

	function PegaIdBottao(this)
		return this.idBottao;
	endfunction

	function PegaX(this)
		return this.x;
	endfunction
	
	function PegaY(this)
		return this.y;
	endfunction

	function AtribuiX(this, x)
		this.x := x;
	endfunction

	function AtribuiY(this, y)
		this.y := y;
	endfunction

	function PegaLargura(this)
		return this.largura;
	endfunction

	function AtribuiLargura(this, largura)
		this.largura := largura	;
	endfunction

	function PegaPosicaoBottao(this)
		return this.idBottao - this.gerenciador.adicionalIdDropDown;
	endfunction

	function PegaItens(this)
		return this.itens;
	endfunction

	function PegaGerenciador(this)
		return this.gerenciador;
	endfunction

endclass

class GerenciadorDropDown()

	//TODO criar metodo padrão onde se nenhuma das opções for informada, gera os ids dinâmicamente. [versões futuras.]
	function GerenciadorDropDown(this,maxOpcoes,adicionalIdDropDown,adicionalIdOpcoes,maxDropDown)
		this.maxOpcoes := CInt(maxOpcoes);
		this.maxDropDown := CInt(maxDropDown);
		this.adicionalIdDropDown := CInt(adicionalIdDropDown);
		this.adicionalIdOpcoes := CInt(adicionalIdOpcoes);
		this.idOpcoesMaximo := this.maxOpcoes * this.maxDropDown + this.adicionalIdOpcoes;
		this.dropDowns := dictionary;
	endfunction

	function _MontaIdBotao(this, posicao)
		return cInt(posicao + this.adicionalIdDropDown);
	endfunction

	function ObtemDropDown(this,posicao, largura := 200, valorEscolhido := "")
		var dropDown := this.BuscaDropDownPorPosicao(posicao);
		
		if(!dropDown) 
			dropDown := this.RegistraDropDown(posicao, largura , valorEscolhido);
		else
			DropDown::ResetaItens(dropDown);
		endif

		return dropDown;
	endfunction

	function RegistraDropDown(this,posicaoBottao, largura := 200, valorEscolhido := "")
		if(valorEscolhido == "")
			valorEscolhido := DropDown::VALOR_ESCOLHIDO_BASE;
		endif

		var dropDown := DropDown(this._MontaIdBotao(posicaoBottao),0,0,0,this.adicionalIdOpcoes,valorEscolhido, this, largura);
		this.dropDowns.insert(dropDown.PegaIdBottao(),dropDown);
		return dropDown;

	endfunction

	function BuscaDropDown(this,idBottao)
		return this.dropDowns[idBottao];
	endfunction

	function BuscaDropDownPorPosicao(this,posicaoBottao)
		return this.PegaDropDowns()[this._MontaIdBotao(posicaoBottao)];
	endfunction

	function DefineValorEscolhido(this,idOpcaoSelecionada)
		foreach dropdown in (this.PegaDropDowns())
			Sleepms(2);
			if(DropDown::ContainIdOpcao(dropdown, idOpcaoSelecionada))
				DropDown::DefineValorEscolhido(dropdown,idOpcaoSelecionada);
				return dropdown;
			endif
		endforeach

		return 0;
	endfunction

	function IsIdDentroOpcoes(this, id)
		return id >= this.adicionalIdOpcoes && id <= this.idOpcoesMaximo;
	endfunction

	function IsIdDentroDropDowns(this, id)
		return id > this.adicionalIdDropDown && id <= this.maxDropDown + this.adicionalIdDropDown;
	endfunction

	function LimpaDropDowns(this)
		this.dropDowns := dictionary;
	endfunction

	function PegaDropDowns(this)
		return this.dropDowns;
	endfunction

	function PegaAdicionalIdDropDown(this)
		return this.adicionalIdDropDown;
	endfunction

	function PegaAdicionalIdOpcoes(this)
		return this.adicionalIdOpcoes;
	endfunction

	function PegaMaxDropDown(this)
		return this.maxDropDown;
	endfunction

	function PegaIdOpcoesMaximo(this)
		return this.idOpcoesMaximo;
	endfunction

	function FechaTodosDropdowns(this, quem)
		foreach dropdown in (this.dropdowns)

			if(GetProcess( dropdown.PegaPid() ))
				CloseGump(quem,dropdown.PegaIdBottao());
			endif
			Sleepms(2);
		endforeach
	endfunction
endclass

function MontaDropDownGump(parx,pary,parLargura,valores, idInicialOpcao)
	var altura := 235;
	var gump := GFCreateGump(parx, pary, parLargura, altura);
	var x := 0;
	var y := 0;
	var diferencaValoresLinhas := 0;
	var opcoesTabela := dictionary{ 
							MAX_PAGINAS -> 30,
							QTD_LINHA_LIMITE -> 8
							};
	
	GFMovable(gump, 0);
	
	if(Valores.size() < opcoesTabela[QTD_LINHA_LIMITE])
		diferencaValoresLinhas := 20 * ( opcoesTabela[QTD_LINHA_LIMITE] - Valores.size()) + 33;
		altura :=  altura - diferencaValoresLinhas;
	endif
	GFResizePic( gump, x, y, 2620, parLargura, altura );
	GFAddAlphaRegion( gump, x + 5, y + 5, parLargura - 10, altura - 10 );

	var colCfg := dictionary;
	colCfg["opcao"] := struct{titulo := "Opção", largura := 9};
	colCfg["botao"] := struct{titulo := "X",ordem := 1,largura := 3};
	
	if(valores.size() > 0)
		CriaTabelaGumpPaginada(x + 10,y + 10,parLargura - 10,valores,gump,opcoesTabela,colCfg, idInicialOpcao);
	else
		GFTextLine(gump,x + 10,y + 10,1153, "Sem opções.");
	endif
	return gump;
endfunction 

function SendDropDownGump( quem, pid, objetoDropDown)
	return Start_Script( ":gumps:scripts/dropDown/dropDownGump", array{quem, pid, objetoDropDown} );
endfunction
