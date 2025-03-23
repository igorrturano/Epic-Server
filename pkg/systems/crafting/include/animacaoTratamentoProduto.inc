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


const LIMITE_MARTELADAS := 3;
const LIMITE_AQUECIMENTO := 4;

enum ITENS_ESTACAO
	FONTE_CALOR := "fonteCalor",
	FONTE_AGUA := "fonteAgua",
	BIGORNA := "bigorna"
endenum

var CATALOGO_FONTE_CALOR := array{0xe1d,0x197e,0x197f,0x1980,
								0x1981, 0x1982, 0x1983, 0x1984,
								0x1985,0x198a, 0x198b, 0x198c,
								0x198d, 0x198e, 0x198f, 0x1990,
								0x1991, 0x1996, 0x1997, 0x1998,
								0x1999, 0x199a, 0x199b, 0x199c,
								0x199d, 0x19a2, 0x19a3, 0x19a4,
								0x19a5, 0x19a6, 0x19a7, 0x19a8, 
								0x19a9};


var CATALOGO_FONTE_AGUA := array{0x0B41,0x0B42,0x0B43,0x0B44,
								0x154D, 0x7623, 0x7624, 0xB31D,
								0xB31E, 0xB31F, 0xB320};

var CORES_AQUECIMENTO_FORJA := array {1254,1256,1258,1260};

function isAnimacaoFerreiroPossivel(quem, estacao, ferramentaNecessaria, posicaoInicial, saudeInicial)
	if(!Accessible(quem,estacao[FONTE_CALOR],1) || !CheckLineOfSight( quem, estacao[FONTE_CALOR]))
		SendSysMessageEx(quem,"É necessário que haja uma fonte de calor próximo a você.",SSM_FAIL);
		return false;
	endif
	if(!Accessible(quem,estacao[FONTE_AGUA],1) || !CheckLineOfSight( quem, estacao[FONTE_AGUA]))
		SendSysMessageEx(quem,"É necessário que haja uma fonte de resfriamento próximo a você.",SSM_FAIL);
		return false;
	endif
	if(!Accessible(quem,estacao[BIGORNA],1) || !CheckLineOfSight( quem, estacao[BIGORNA]))
		SendSysMessageEx(quem,"É necessário que haja uma bigorna próxima a você.",SSM_FAIL);
		return false;
	endif
	if(ferramentaNecessaria.objtype != 0x13e3) //Martelo de ferreiro.
		SendSysMessageEx(quem,"Você não tem a ferramenta necessária para executar esta ação.",SSM_FAIL);
		return false;
	endif
	
	return CheckCanWork(quem,saudeInicial,posicaoInicial);
endfunction

function RodaAnimacaoFerreiro(quem, produto, estacao, processoGerente, classificacaoProduto)
	var estagios := array {"aquecendo", "martelando", "esfriando"};
	var contadorEstagio := 0;
	var contadorInterno := 0;
	var deveMudarEstagio;
	var ferramentaNecessaria := BuscaFerramenta(quem,"Metal_Working");
	var posicaoInicial := CriaObjetoPosicao(quem);
	var saudeInicial := AP_GetVital(quem, "Hits");

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
				PerformAction( quem, ANIM_CAST_DIR );

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
				PerformAction( quem, ANIM_ATTACK_1HAND_DOWN ); 
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
				PerformAction( quem, ANIM_ATTACK_1HAND_JAB ); 
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
		ferramentaNecessaria := BuscaFerramenta(quem,"Metal_Working");
		Sleep(2);
	endwhile

	return true;
endfunction

function DefineItemEstacao (byref estacao, quem, objProp, serialGuardada, itemEstacao)
	estacao.insert(itemEstacao, SystemFindObjectBySerial( objProp[serialGuardada], sysfind_flags := 0 ));
	if(!Accessible(quem,estacao[itemEstacao],1) || !CheckLineOfSight( quem, estacao[itemEstacao]))
		estacao.erase(itemEstacao);
	endif

endfunction

function BuscaFerramenta (quem, pericia)
	var ferramenta;
	case (pericia)
		"Metal_Working":
			if(quem.weapon.objtype == 0x13e3)
				ferramenta := quem.weapon;
			else
				ferramenta := FindObjtypeInContainer(quem.backpack, 0x13e3);
			endif
		break;
	endcase
	return ferramenta;
endfunction

function DesequipaArma(quem)
	var itemEquipado := quem.weapon;
	if(itemEquipado)
		MoveItemToContainer(itemEquipado,quem.backpack);
	endif
endfunction


function PegaEstacaoFerreiro (quem, bigorna)

	if(bigorna.objtype != UOBJ_ANVIL1)
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
		var itensProximos := ListItemsNearLocation(quem.x,quem.y,quem.z,1,quem.realm);
		foreach itemProximo in itensProximos
			Sleepms(2);
			if(!estacao.exists(FONTE_AGUA))
				if(itemProximo.objtype in CATALOGO_FONTE_AGUA)
					estacao.insert(FONTE_AGUA, itemProximo);
					propFoiAlterada := true;
				endif
			elseif(!estacao.exists(FONTE_CALOR))
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