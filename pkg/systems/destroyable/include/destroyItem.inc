use uo;
use os;
use basic;

include ":combat:combat";
include ":attributes:attributes";
include ":destroyable:include/destroyItemPersist";
include ":itemutils:itemInfo";
include "/include/utils";
include ":traps:traps";
include "include/packets";

const PROPNAME_DURABILIDADE := "durabItem"; 
const PROPNAME_DURABILIDADE_MAX := "maxDurabItem"; 
const PROPNAME_CHAVE_ORIGINAL := "serialOriginal"; 
const PROPNAME_RESITEM_MULT := "resItem"; 
const PROPNAME_ARTIFICE_SERIAL := "crafterserial"; 
const PROPNAME_ARMADILHAS := "TrapList"; 
const PROPNAME_DISPENSAVEL_FLAG := "isDispensavel"; 
const PROPNAME_INDESTRUTIVEL := "isIndestrutivel"; 
const PROPNAME_MATERIAL := "material"; 
const PROPNAME_DESTROCO := "isDestroco"; 
const PROPNAME_DEED_SERIALS := "ItemsCreatedSerials"; 

const ELEM_CFG_DEST := "Destruiveis";

enum TIPO_MATERIAIS
	PEDRA := "Stone",
	TIJOLO := "Brick",
	MARMORE := "marmore",
	ARENITO := "arenito",
	OSSOS := "ossos",
	VIDRO := "Glass",
	MADEIRA := "Wood",
	PANO := "Cloth",
	METAL := "Metal",
	COURO := "Leather",
	CORPO := "Corpo",
	CORPO_HUMANO := "CorpoHumano",
	INDEFINIDO := "indefinido"
endenum

var catalogoDestrocos := MontaCatalogoDestrocos();
var configRecursos := ReadConfigFile(":crafting:config/resourceList");
var configDestruiveis := ReadConfigFile(":destroyable:settings");

function ComportamentoWarMode(quem, itemFonte)

	EraseObjProperty(quem, "IsMeditating");
	EraseObjProperty(quem, "HealTimer");
	SendSysMessage(quem, "[Modo Guerra] Escolha o item que deseja destruir.");
	var targ := TargetCoordinates(quem);
	
	if ( !targ )
		SendSysMessage(quem, "Cancelado.");
		return 0;
	endif

	start_script (":tn:destroyitems", {quem, targ.item, itemFonte});
endfunction

function Destruir(quem,itemAlvo,fonteDano)
	LimpaObjPropsDestruiveis(itemAlvo);
	var envelopeDeed := struct;
	envelopeDeed.+isDeed := GetObjProperty(itemAlvo,PROPNAME_DEED_SERIALS);
	var deveMarcar := DeveMarcarRecriarItem(itemAlvo);

	if(envelopeDeed.isDeed)
		envelopeDeed.+itensAssociados := array;
		foreach serial in (envelopeDeed.isDeed)
			var item := SystemFindObjectBySerial(serial);
			envelopeDeed.itensAssociados.append(item);;
			Sleepms(2);
		endforeach
	endif

	if(deveMarcar)//se não foi criado por player, não é um corpo e não foi marcado como dispensável..
		var serialOriginal := GetObjProperty(itemAlvo,PROPNAME_CHAVE_ORIGINAL);
		if(!serialOriginal)
			SetObjProperty(itemAlvo,PROPNAME_CHAVE_ORIGINAL,itemAlvo.serial);//Grava serial do item original.
		endif
		if(envelopeDeed.isDeed)
			foreach item in (envelopeDeed.itensAssociados)
				MarcarRecriarItem(item);
				Sleepms(2);
			endforeach
		else
			MarcarRecriarItem(itemAlvo);
		endif
	endif 

	var isObjetoMovido := false;

	//TODO talvez criar um container "morto" e colocar todos os espolios dentro?.
	if(itemAlvo.ISA(POLCLASS_CONTAINER))
		var listaItens := EnumerateItemsInContainer(itemAlvo);
		foreach espolio in listaItens
			if(RandomInt(100) <= configDestruiveis[ELEM_CFG_DEST].espolioDestChance)
				continue; //deixa o equipamento para ser destruido junto com o container.
			else
				if(espolio.ISA(POLCLASS_EQUIPMENT))
					espolio.hp := espolio.hp - RandomIntMinMax(CInt(espolio.maxhp / 5), espolio.maxhp);//Danifica o equipamento caso não seja destruído.
				endif
				MoveObjectToLocation(espolio,itemAlvo.x,itemAlvo.y,itemAlvo.z, itemAlvo.realm, MOVEOBJECT_FORCELOCATION);
				isObjetoMovido := true;
			endif
			Sleepms(2);
		endforeach
	endif

	RunTraps(itemAlvo,quem);
	TocaSFXDestruicao(itemAlvo,isObjetoMovido);
	if(!IsDestroco(itemAlvo))
		var descFonteDano := fonteDano.desc;
		if(envelopeDeed.isDeed)
			CriaDestrocos(itemAlvo,envelopeDeed.isDeed.size(),4);
			descFonteDano += "[Deed]";
		else
			CriaDestrocos(itemAlvo,CalculaMaxDestrocos(itemAlvo),4);
		endif
		if(!descFonteDano)
			descFonteDano := "Desconhecido";
		endif
		InsereLogItemDestruido(quem,POLCore().systime,itemAlvo.x,itemAlvo.y,itemAlvo.z,itemAlvo.realm,itemAlvo.desc,descFonteDano,deveMarcar);
	endif
	if(!envelopeDeed.isDeed)
		DestroyItem(itemAlvo);
	else
		foreach item in (envelopeDeed.itensAssociados)
			DestroyItem(item);
			Sleepms(2);
		endforeach
	endif
	//TODO Criar kit de reparo de itens e opção do crafter reparar item.
	//TODO Disparar evento para guardas e mostros próximos. [ OBS Fazer Merge com a Develop Antes]
	//TODO Disparar discord web hook.
	//TODO Adicionar comentários e documentação do que já foi feito.


	//comandos:
	//TODO implementar comando de Indestrutível.
	//TODO Criar comando ou argumento do create para gms criarem itens dispensáveis.
endfunction

function DeveMarcarRecriarItem(itemAlvo)
	return !GetObjProperty(itemAlvo,PROPNAME_ARTIFICE_SERIAL) 
	&& !GetObjProperty(itemAlvo,PROPNAME_DISPENSAVEL_FLAG)
	&& !itemAlvo.ISA(POLCLASS_CORPSE)
	&& itemAlvo.ISA(POLCLASS_ITEM)
	&& !IsDestroco(itemAlvo);
endfunction

function CalculaMaxDestrocos(item)
	var numeroMaximoDest := Ceil(item.height / 5);
	numeroMaximoDest := numeroMaximoDest > 1 ? numeroMaximoDest : 1;
	return RandomInt(cInt(numeroMaximoDest)) + 1;
endfunction

function AvaliaDisparoArmadilha(item)
	var armadilhas := GetObjProperty(item, PROPNAME_ARMADILHAS);
	if(armadilhas)	
		var maiorForca := 0;
		foreach armadilha in armadilhas
			maiorforca := maiorForca < armadilha.strength? armadilha.strength: maiorForca;
			Sleepms(2);
		endforeach
		if(RandomInt(100) < maiorForca * configDestruiveis[ELEM_CFG_DEST].multDisparoArmadilha)
			return true;
		endif;
	endif
	return false;
endfunction

function MontaCatalogoDestrocos()
	var catalogo := Dictionary;
	
	catalogo.insert(PEDRA,{0x46C8,0x46CB,0x1038,0x19BA,0x19B8,0x0DF9,0x0F3B});
	catalogo.insert(TIJOLO,{0x46C8,0x46CB,0x1038,0x0F3B});
	catalogo.insert(MARMORE,{0x46C8,0x46CB,0x1038,0x19BA,0x19B8,0x0DF9,0x0F3B});
	catalogo.insert(ARENITO,{0x46C8,0x46CB,0x0DF9,0x0F3B});
	catalogo.insert(OSSOS,{0x1B1A,0x1B19,0x1B12,0x1B11});
	catalogo.insert(VIDRO,{0x5738});
	catalogo.insert(MADEIRA,{0x0c2f,0x0c2e,0x1038,0x1B9D,0xABEA,0xABED});
	catalogo.insert(PANO,{0x0DD5,0x0EE9,0x1766});
	catalogo.insert(METAL,{0x1038,0x1B26,0x1B9D,0x1B9E});
	catalogo.insert(COURO,{0x4006,0x1B9E});
	catalogo.insert(CORPO,{0x122F,0x1CF0,0x1D10,0x1D0E,0x1D09,0x1D0D});
	catalogo.insert(CORPO_HUMANO,{0x1CED,0x1CEE,0x1CF0,0x122F,0x1CE2,0x1CE9,0x1CE6,0x1CE4,0x1CE1,0x1CE5,0x1CEA,0x1CDD,0x1D10,0x1D0E,0x1D09,0x1D0D});
	catalogo.insert(INDEFINIDO,{0x1038,0x1B26,0x1B9D,0x1B9E,0x0F3B,0x1038});

	return catalogo;
endfunction

function TocaSFXDestruicao(itemAlvo,deveTocarTesouro)
	var material := "";
	var sfx := 0;

	if(itemAlvo.isA(POLCLASS_CORPSE))
		material := CORPO;
	else
		var materialId := GetObjProperty(itemAlvo,PROPNAME_MATERIAL);
		material := configRecursos[materialId].Type;
	endif
	
	case (material)
		PEDRA: sfx := 0x11C; break;
		TIJOLO: sfx := 0x11C; break;
		MARMORE: sfx := 0x11C; break;
		ARENITO: sfx := 0x11C; break;
		OSSOS: sfx := 0x342; break;
		VIDRO: sfx := 0x03E; break;
		MADEIRA: sfx := 0x476; break;
		PANO: sfx := 0x33A; break;
		METAL: sfx := 0x057; break;
		COURO: sfx := 0x04B; break;
		CORPO: sfx := 0x146; break;
		CORPO_HUMANO: sfx := 0x146; break;
		default: sfx := 0x33C;
	endcase
	PlaySoundEffect(itemAlvo,sfx);
	if(deveTocarTesouro)
		PlaySoundEffect(itemAlvo,0x2E1);
	endif
endfunction

function LimpaObjPropsDestruiveis(itemAlvo)
	EraseObjProperty(itemAlvo,PROPNAME_DURABILIDADE);
endfunction

function PegaResistenciaItem(itemAlvo)
	var propRes := GetObjProperty(itemAlvo,PROPNAME_RESITEM_MULT);
	var materialId := GetObjProperty(itemAlvo,PROPNAME_MATERIAL);
	var recursoMod := configRecursos[materialId].hpmod;

	var resFinal := configDestruiveis[ELEM_CFG_DEST].resistenciaBase;
	if(propRes)
		resFinal := resFinal * propRes;
	endif
	if(materialId)
		resFinal := resFinal + recursoMod/2;
	endif
	return resFinal;
endfunction

function CalculaDanoRecebido( fonteDano, atributosItem)
	var danoCru := 0; 
	if(fonteDano.ISA(POLCLASS_WEAPON))
		var dadoDano :=  RandomDiceRoll(fonteDano.getMainDamageDice());
		danoCru := dadoDano + (Ceil(GetAttribute(fonteDano.character_owner,"Strength") / 10));
	elseif(TypeOf("Struct"))
		danoCru := fonteDano.dano;
	else
		danoCru := fonteDano;
	endif

	var danoRecebido := danoCru - atributosItem.resistencia;
	var danoCalculado := struct;
	danoCalculado.+dano := danoRecebido < 0? 0 : danoRecebido;
	danoCalculado.+isEficaz := danoRecebido > 0;
	return danoCalculado;
endfunction

function PegaDurabilidadeTotal(item)
	var itemdesc := ReadConfigFile(":*:itemdesc");
    var elem := FindConfigElem(itemdesc, item.objtype);
	var pesoEscolhido := 0;
	var materialId := GetObjProperty(item,PROPNAME_MATERIAL);
	var recursoMod := configRecursos[materialId].hpmod;

	if(item.ISA(POLCLASS_EQUIPMENT))
		return item.maxhp;
	else
		var propMaxDurab := GetObjProperty(item,PROPNAME_DURABILIDADE_MAX);
		if(GetObjProperty(item,PROPNAME_DURABILIDADE_MAX))
			if(recursoMod)
				propMaxDurab := propMaxDurab + recursoMod;
			endif
			return propMaxDurab;
		endif
		
		if(elem)
			if(elem.weight > 0)
				pesoEscolhido := CInt(elem.weight);
				return Ceil(pesoEscolhido) *  configDestruiveis[ELEM_CFG_DEST].hpMaxMultiplicador + recursoMod;
			endif
		endif

		var infoTile := IU_GetTileInfo(item.Graphic);
		pesoEscolhido := CInt(infoTile.Weight) * configDestruiveis[ELEM_CFG_DEST].hpMaxMultiplicador + recursoMod;
		return Ceil(pesoEscolhido) > 0? pesoEscolhido: 1;
	endif
endfunction

function IsEmperrado(item)
	if(!item.ISA(POLCLASS_LOCKABLE))
		return false;
	endif

	var atributos := MontaAtributosItem(item);
	return atributos.durabTotal / 2 > atributos.durabilidade;   
endfunction

function MontaAtributosItem(item)
	var itemAtrib := struct;
		itemAtrib.+resistencia := cInt(PegaResistenciaItem(item));
		itemAtrib.+durabTotal := PegaDurabilidadeTotal(item);

		if(item.ISA(POLCLASS_EQUIPMENT))
			itemAtrib.+durabilidade := item.hp;
			SetObjProperty(item,PROPNAME_DURABILIDADE, item.hp);
		else
			var propDur := GetObjProperty(item,PROPNAME_DURABILIDADE);
			if(!propDur)
				SetObjProperty(item,PROPNAME_DURABILIDADE,itemAtrib.durabTotal);
				itemAtrib.+durabilidade := itemAtrib.durabTotal;
			else
				itemAtrib.+durabilidade := propDur;
			endif
		endif
	return itemAtrib;
endfunction

function DanificaItem(quemDanificou,item,fonteDano,gfx := 0,gfxHue := 0)
	if(item.concealed || item.invisible)
		return struct;
	endif

	var atributosItem := MontaAtributosItem(item);
	var danoCalculado := CalculaDanoRecebido(fonteDano,atributosItem);
	
	if(item.IsA(POLCLASS_CORPSE) && GetObjProperty(item,"account")) // Corpo de jogador.
		danoCalculado.isEficaz := false;
	endif

	if(!danoCalculado.isEficaz )
		return danoCalculado;
	endif

	var durabilidadeRestante :=  atributosItem.durabilidade - danoCalculado.dano;

	if(Item.ISA(POLCLASS_EQUIPMENT))
		item.hp := CInt(durabilidadeRestante);
	endif

	if(gfx != 0)
		if(gfxHue)
			PlayObjectCenteredEffectHuefx(item, gfx, 5, 6,gfxHue);
		else
			PlayObjectCenteredEffect(item, gfx, 5, 6);
		endif
	endif

	if(!IsDestroco(item))
		if(RandomInt(100) <= configDestruiveis[ELEM_CFG_DEST].criaDestrocoChance)
			CriaDestrocos(item,1,3,false);
		endif
	endif
	
	SetObjProperty(item,PROPNAME_DURABILIDADE,durabilidadeRestante);
	PrintTextAbovePrivate(item,CStr(danoCalculado.dano),quemDanificou,3,FOGO_LIGHT);

	if(durabilidadeRestante <= 0)
		Destruir(quemDanificou,item,fonteDano);
		danoCalculado.+isDestruido := true;
	else
		if(AvaliaDisparoArmadilha(item))
			RunTraps(item,quemDanificou);
		endif
	endif

	return danoCalculado;
endfunction

function IsDestroco(item)
	return GetObjProperty(item,PROPNAME_DESTROCO);
endfunction

function DanificaObjetosProximos(quem,x,y,z,realm,dano,alcance,gfx:=0,ignoreSerial := 0,gfxHue := 0)
	var itens:= ListItemsNearLocation(x,y,z,alcance,realm);

	foreach item in itens
		if(CheckLosBetween(x,y,z,item.x,item.y,item.z,realm) && item.serial != ignoreSerial)
			DanificaItem(quem,item,dano,gfx,gfxHue);
		endif
		Sleepms(3);
	endforeach
endfunction

function CriaDestrocos(itemAlvo,quantidade,alcanceMax := 0, deveCriarOrigem := true)
	var materialId := GetObjProperty(itemAlvo,PROPNAME_MATERIAL);

	var arrayDestrocos := array;
	var corDestroco := 1001;
	var material := "";
	
	if(materialId)
		arrayDestrocos := catalogoDestrocos[configRecursos[materialId].Type];
		corDestroco := configRecursos[materialId].Color;
		material := configRecursos[materialId].Type;
	else
		if(itemAlvo.ISA(POLCLASS_CORPSE))
			var templateProp := GetObjProperty(itemAlvo,"npctemplate");
			if(templateProp)
				var parsed := NPC_ParseTemplateName(templateProp);
				var cfg := ReadConfigFile(":brainai:npcdesc");
				Case (cfg[parsed.template].Taxonomy)
					"homo": 
						arrayDestrocos:= catalogoDestrocos[CORPO_HUMANO];
						material := CORPO_HUMANO;
						corDestroco := 0;
						break;
					default:
						arrayDestrocos:= catalogoDestrocos[CORPO];
						material := CORPO;
						corDestroco := 0;
				endcase
			endif
		else
			arrayDestrocos := catalogoDestrocos[INDEFINIDO];
		endif
	endif

	var i;
	for (i := 1; i <= quantidade; i += 1)
		var adicionalX := RandomInt(alcanceMax) * PositivoNegativo(RandomInt(100));
		var adicionalY := RandomInt(alcanceMax) * PositivoNegativo(RandomInt(100));
		var destrocoGraphic := arrayDestrocos.randomentry();
		var destrocoPos := struct;
		destrocoPos.+x := itemAlvo.x + adicionalX;
		destrocoPos.+y := itemAlvo.y + adicionalY;
		destrocoPos.+z := itemAlvo.z;

		if(!CheckLosBetween( itemAlvo.x, itemAlvo.y, itemAlvo.z,destrocoPos.x,destrocoPos.y,destrocoPos.z,BRITANNIA_REALM) || (deveCriarOrigem && i == 1))
			destrocoPos.x := itemAlvo.x; 
			destrocoPos.y := itemAlvo.y; 
			destrocoPos.z := itemAlvo.z; 
		endif

		PlayMovingEffectXYZEx(itemAlvo.x,
			itemAlvo.y,
			itemAlvo.z,
			destrocoPos.x,
			destrocoPos.y,
			destrocoPos.z,
			BRITANNIA_REALM,
			destrocoGraphic,
			2,
			2,
			corDestroco
			);

		var destroco := CreateItemAtLocation(destrocoPos.x , destrocoPos.y, destrocoPos.z, destrocoGraphic);
		ConfiguraDestroco(destroco,material,corDestroco);
		Sleepms(2);
	endfor
endfunction

function ConfiguraDestroco(byref destroco,tipo,corDestroco)
	destroco.color := corDestroco;
	destroco.decayat := ReadGameClock() + configDestruiveis[ELEM_CFG_DEST].decayDestroco;
	var novoNome := "destroços de ";
		case (tipo)
			PEDRA: 
				novoNome := novoNome + "pedra";
				break;
			TIJOLO:  
				novoNome :=	novoNome + "tijolo";
				break;
			MARMORE:  
				novoNome :=	novoNome + "marmore";
				break;
			ARENITO:  
				novoNome :=	novoNome + "arenito";
				break;
			OSSOS: 
				novoNome :=	novoNome + "ossos";
				break;
			VIDRO: 
				novoNome :=	novoNome + "vidro";
				break;
			MADEIRA: 
				novoNome :=	novoNome + "madeira";
				break;
			PANO:  
				novoNome :=	novoNome + "pano";
				break;
			METAL:  
				novoNome :=	novoNome + "metal";
				break;
			CORPO:  
			CORPO_HUMANO:  
				novoNome :=	"Restos de corpo";
				break;
			default:
				novoNome:=	"destroços";
		endcase
		SetObjProperty(destroco,PROPNAME_DESTROCO,1);
		SetName(destroco,novoNome);
endfunction