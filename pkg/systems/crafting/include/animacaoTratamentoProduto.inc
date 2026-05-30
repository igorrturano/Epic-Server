use uo;
use os;

include "/include/epicPropNames";
include "include/epicValidAction";
include "include/client";
include "include/objtype";
include "include/say";
include ":megacliloc:/include/toolTipUtils";
include ":crafting:/include/fabricacaoUtil";
include ":attributes:vitals";
include ":crafting:toolWear";
include ":crafting:include/util/fabricacaoConstants";

function isAnimacaoFerreiroPossivel(quem, estacao, ferramentaNecessaria, posicaoInicial, saudeInicial)
	if(!CheckCanWork(quem,saudeInicial,posicaoInicial))
		return  false;
	endif
	if(!Accessible(quem,estacao[FONTE_CALOR],2) || !CheckLineOfSight( quem, estacao[FONTE_CALOR]))
		SendSysMessageEx(quem,"É necessário que haja uma fonte de calor próximo a você.",SSM_FAIL);
		return false;
	endif
	if(!Accessible(quem,estacao[FONTE_AGUA],2) || !CheckLineOfSight( quem, estacao[FONTE_AGUA]))
		SendSysMessageEx(quem,"É necessário que haja uma fonte de resfriamento próximo a você.",SSM_FAIL);
		return false;
	endif
	if(!Accessible(quem,estacao[BIGORNA],2) || !CheckLineOfSight( quem, estacao[BIGORNA]))
		SendSysMessageEx(quem,"É necessário que haja uma bigorna próxima a você.",SSM_FAIL);
		return false;
	endif
	if(ferramentaNecessaria.objtype != 0x13e3) //Martelo de ferreiro.
		SendSysMessageEx(quem,"Você não tem um martelo de ferreiro para executar esta ação.",SSM_FAIL);
		return false;
	endif
	
	return true;
endfunction

function isAnimacaoPadraoPossivel(quem, estacao, ferramentaNecessaria, posicaoInicial, saudeInicial, profissao)
	if(quem.hidden)
		SendSysMessageEx(quem,"Você não pode trabalhar enquanto está escondido.",SSM_FAIL);
		return false;
	endif
	var profissaoConfig := ReadConfigFile(PROFISSAO_CONFIG_PATH);
	var profissaoElem :=FindConfigElem( profissaoConfig, profissao );
	var verificaLos := !CheckLosBetween( quem.x, quem.y, quem.z, estacao.x, estacao.y, estacao.z, quem.realm)  && TypeOfInt(estacao.item) != OT_ITEMREF;
	if(CoordinateDistance(quem.x, quem.y, estacao.x, estacao.y) > 1 || !CheckLosBetween( quem.x, quem.y, quem.z, estacao.x, estacao.y, estacao.z, quem.realm))
		SendSysMessageEx(quem,"Você não alcança a estação escolhida",SSM_FAIL);
		return false;
	endif

	if(!ferramentaNecessaria && profissaoElem.Ferramenta)
		var ferramentas := SplitWords(GetConfigString(profissaoElem,"Ferramenta"));
		var texto := "";
		foreach ferramentaPossivel in ferramentas
			sleepms(2);
			var item_cfg := ReadConfigFile(":*:itemdesc");
			var nomeFerramenta := item_cfg[cint(ferramentaPossivel)];
			texto += "{} ".format(nomeFerramenta.desc? nomeFerramenta.desc : nomeFerramenta.name);
		endforeach
		SendSysMessageEx(quem,"Você não tem a ferramenta necessária para executar esta ação. [{}]".format(texto),SSM_FAIL);
		return false;
	endif
	
	return CheckCanWork(quem,saudeInicial,posicaoInicial);
endfunction

// ------------        Animações Inicio			--------------
function RodaAnimacaoFerreiro(quem, produto, estacao, processoGerente, classificacaoProduto)
	var estagios := array {"aquecendo", "martelando", "esfriando"};
	var contadorEstagio := 0;
	var contadorInterno := 0;
	var deveMudarEstagio;
	var ferramentaNecessaria := BuscaFerramenta(quem,"Blacksmithy");
	var posicaoInicial := BuildPositionStruct(quem);
	var saudeInicial := AP_GetVital(quem, "Hits");
	var craftConfig := ReadConfigFile(":crafting:config/craftConfig");
	var espera := craftConfig["Blacksmithy"].CraftDelay;

	while (processoGerente.state && IsAnimacaoFerreiroPossivel(quem,estacao, ferramentaNecessaria, posicaoInicial, saudeInicial))
		var estagioAtual := contadorEstagio %  estagios.size();
		deveMudarEstagio := false;
		CheckToolWear(quem,ferramentaNecessaria,"Metal_Working");
		case (estagioAtual)
			0://Aquecendo
				DesequipaArma(quem);
				produto.movable := 0;
				quem.facing := GetFacing(quem.x,quem.y,estacao[FONTE_CALOR].x,estacao[FONTE_CALOR].y);
				MoveObjectToLocation( 
								produto,
								estacao[FONTE_CALOR].x,
								estacao[FONTE_CALOR].y,
								estacao[FONTE_CALOR].height, 
								estacao[FONTE_CALOR].realm, 
								MOVEOBJECT_FORCELOCATION+MOVEITEM_IGNOREMOVABLE );

				var corAquecido := CORES_AQUECIMENTO_FORJA[contadorInterno];
				if(corAquecido)
					produto.color := corAquecido;
				endif
				PlaySoundEffect(quem, SFX_2C);
				PerformAction( quem, HUMAN_ANIM::CAST_DIR );

				if(contadorInterno >=  CORES_AQUECIMENTO_FORJA.size())
					PrintTextAbove(produto,"*Item em brasa*");
				endif

				if(contadorInterno >= LIMITE_AQUECIMENTO)
					deveMudarEstagio := true;
					contadorInterno := 0;
					produto.movable := 1;
				endif
				break;
			1://Martelando
				if(quem.weapon != ferramentaNecessaria)
					DesequipaArma(quem);
					var equipou := EquipItem(quem, ferramentaNecessaria);
					if(!equipou)
						SendSysMessageEX(quem, "Você não pôde equipar seu martelo.", SSM_FAIL);
						return true;
					endif
				endif
				produto.movable := 0;
				quem.facing := GetFacing(quem.x,quem.y,estacao[BIGORNA].x,estacao[BIGORNA].y);
				MoveObjectToLocation( 
								produto,
								estacao[BIGORNA].x,
								estacao[BIGORNA].y,
								estacao[BIGORNA].height, 
								estacao[BIGORNA].realm, 
								MOVEOBJECT_FORCELOCATION+MOVEITEM_IGNOREMOVABLE);
				PerformAction( quem, HUMAN_ANIM::ATTACK_1HAND_BLUNT ); 
				PlaySoundEffect(quem, SFX_2B);
				
				if(contadorInterno >= LIMITE_MARTELADAS)
					deveMudarEstagio := true;
					contadorInterno := 0;
					produto.movable := 1;
				endif
				break;
			2://Esfriando
				produto.movable := 1;
				quem.facing := GetFacing(quem.x,quem.y,estacao[FONTE_AGUA].x,estacao[FONTE_AGUA].y);
				DesequipaArma(quem);

				if(classificacaoProduto == ITEM_ARMA || classificacaoProduto == TEMPLATE_ARMA)
					var equipou := EquipItem(quem, produto);
					if(!equipou)
						MoveObjectToLocation( 
								produto,
								estacao[FONTE_AGUA].x,
								estacao[FONTE_AGUA].y,
								estacao[FONTE_AGUA].height, 
								estacao[FONTE_AGUA].realm, 
								MOVEOBJECT_FORCELOCATION+MOVEITEM_IGNOREMOVABLE );
					endif
				else
					MoveObjectToLocation( 
								produto,
								estacao[FONTE_AGUA].x,
								estacao[FONTE_AGUA].y,
								estacao[FONTE_AGUA].height, 
								estacao[FONTE_AGUA].realm, 
								MOVEOBJECT_FORCELOCATION+MOVEITEM_IGNOREMOVABLE );
				endif
				PerformAction( quem, HUMAN_ANIM::ATTACK_1HAND_PIERCING ); 
				PlaySoundEffect(quem, SFX_27);
				PlayObjectCenteredEffectEx(estacao[FONTE_AGUA],FX_SMOKE,1,12);
				produto.color := 1175;
				deveMudarEstagio := true;
				contadorInterno := 0;
				
				break;
		endcase

		++contadorInterno;
		if(deveMudarEstagio)
			++contadorEstagio;
		endif
		Sleepms(10);
		ferramentaNecessaria := BuscaFerramenta(quem,"Blacksmithy");
		Sleep(Cint(espera));
	endwhile

	return true;
endfunction

function RodaAnimacaoPadrao(quem, produto, estacao, processoGerente, propFabricado)
	var ferramentaNecessaria := BuscaFerramenta(quem,propFabricado.receita.profissao);
	var craftConfig := ReadConfigFile(":crafting:config/craftConfig");
	var posicaoInicial := BuildPositionStruct(quem);
	var saudeInicial := AP_GetVital(quem, "Hits");
	var skill := craftConfig[propFabricado.receita.profissao].Attribute;
	var espera := craftConfig[propFabricado.receita.profissao].CraftDelay;

	while (processoGerente.state && isAnimacaoPadraoPossivel(quem,estacao, ferramentaNecessaria, posicaoInicial, saudeInicial,propFabricado.receita.profissao))
		
		var alturaAlvo := estacao.z;
		if(estacao.mobile)
			return true;
		else
			if(estacao.item)
				alturaAlvo := estacao.item.height;
			endif
		endif

		produto.movable := 0;
		quem.facing := GetFacing(quem.x,quem.y,estacao.x,estacao.y);
		MoveObjectToLocation( 
						produto,
						estacao.x,
						estacao.y,
						alturaAlvo, 
						estacao.realm, 
						MOVEOBJECT_FORCELOCATION+MOVEITEM_IGNOREMOVABLE );
		
		if(RandomInt(10) <= 3 )
			PrintTextAbove(quem,"*Trabalhando*");
		endif
		PerformAction( quem, HUMAN_ANIM::ATTACK_BOW ); 
		PlaySoundEffect(quem, SFX_56);
		CheckToolWear(quem,ferramentaNecessaria,skill);
		Sleep(Cint(espera));

	endwhile

	return true;
endfunction

// ------------        Animações Fim			--------------

function DefineItemEstacao (byref estacao, quem, objProp, serialGuardada, itemEstacao)
	estacao.insert(itemEstacao, SystemFindObjectBySerial( objProp[serialGuardada], sysfind_flags := 0 ));
	if(!Accessible(quem,estacao[itemEstacao],1) || !CheckLineOfSight( quem, estacao[itemEstacao]))
		estacao.erase(itemEstacao);
	endif

endfunction

function BuscaFerramenta (quem, profissao)
	var ferramenta;
	var profissaoConfig := ReadConfigFile(PROFISSAO_CONFIG_PATH);
	var ferramentas := SplitWords(profissaoConfig[profissao].Ferramenta);
	
	foreach ferramentaPossivel in ferramentas
		Sleepms(2);
		if(cstr(hex(quem.weapon.objtype)) == cstr(hex(ferramentaPossivel)))
			ferramenta := quem.weapon;
			break;
		else
			var ferramentaEncontrada := FindObjtypeInContainer(quem.backpack, cint(hex(ferramentaPossivel)));
			if(ferramentaEncontrada)
				ferramenta := ferramentaEncontrada;
				break;
			endif
		endif
	endforeach
	
	return ferramenta;
endfunction

function DesequipaArma(quem)
	var itemEquipado := quem.weapon;
	if(itemEquipado)
		MoveItemToContainer(itemEquipado,quem.backpack);
	endif
endfunction

function PegaEstacaoFerreiro (quem, bigorna)

	if(!(bigorna.objtype  in array{UOBJ_ANVIL1,UOBJ_ANVIL2}))
		SendSysMessageEX(quem,"A estação escolhida não é uma bigorna.", SSM_FAIL);
		return 0;
	endif

	var propForja := GetObjProperty(bigorna,PROPNAME_ESTACAO_FORJA);
	var estacao := struct;
	var propFoiAlterada := false;

	if(propForja.serialFonteCalor)
		DefineItemEstacao(estacao,quem,propForja,"serialFonteCalor",FONTE_CALOR);
	elseif(propForja.serialFonteAgua)
		DefineItemEstacao(estacao,quem,propForja,"serialFonteAgua",FONTE_AGUA);
	endif
	
	if(!estacao.exists(FONTE_AGUA) || !estacao.exists(FONTE_CALOR))
		var itensProximos := ListItemsNearLocation(quem.x,quem.y,quem.z,2,quem.realm);
		var staticosProximos := ListStaticsNearLocation(quem.x,quem.y,quem.z,2,quem.realm);
		itensProximos := itensProximos + staticosProximos;

		foreach itemProximo in itensProximos
			Sleepms(2);
			if(!estacao.exists(FONTE_AGUA))
				if(itemProximo.objtype in CATALOGO_FONTE_AGUA)
					estacao.insert(FONTE_AGUA, itemProximo);
					propFoiAlterada := true;
				endif
			endif
			if(!estacao.exists(FONTE_CALOR))
				if(itemProximo.objtype in CATALOGO_FONTE_CALOR)
					estacao.insert(FONTE_CALOR, itemProximo);
					propFoiAlterada := true;
				endif
			endif
		endforeach
	endif

	if(propFoiAlterada)
		var novaProp := struct;

		if(estacao[FONTE_CALOR])
			novaProp.+serialFonteCalor := estacao[FONTE_CALOR].serial;
		endif

		if(estacao[FONTE_AGUA])
			novaProp.+serialFonteAgua := estacao[FONTE_AGUA].serial;
		endif

		SetObjProperty(
				bigorna,
				PROPNAME_ESTACAO_FORJA,
				novaProp
			);
	endif

	estacao.insert(BIGORNA,bigorna);
	return estacao;
endfunction