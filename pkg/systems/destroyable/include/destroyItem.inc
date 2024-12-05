use uo;
use os;
use basic;

include ":combat:combat";
include ":attributes:attributes";
include ":destroyable:include/destroyItemPersist";
include ":itemutils:itemInfo";
include "/include/utils";

const PROPNAME_DURABILIDADE := "durabItem"; 
const PROPNAME_DURABILIDADE_MAX := "maxDurabItem"; 
const PROPNAME_CHAVE_ORIGINAL := "serialOriginal"; 
const PROPNAME_RESITEM_MULT := "resItem"; 
const PROPNAME_ARTIFICE_SERIAL := "crafterserial"; 
const PROPNAME_EMPERRADO := "isEmperrado"; 
const PROPNAME_DISPENSAVEL_FLAG := "isDispensavel"; 
const PROPNAME_INDESTRUTIVEL := "isIndestrutivel"; 
const PROPNAME_MATERIAL := "material"; 

const RESISTENCIA_BASE := 8; 
const DECAYTIME_DESTROCO := 10;

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
	INDEFINIDO := "indefinido"
endenum

var catalogoDestrocos := MontaCatalogoDestrocos();
var configRecursos := ReadConfigFile(":crafting:config/resourceList");

function ComportamentoWarMode(quem, itemFonte)

	//TODO inserir esse hook em todos os scripts de arma.
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

function Destruir(quem,itemAlvo)
	LimpaObjPropsDestruiveis(itemAlvo);

	if(!GetObjProperty(itemAlvo,PROPNAME_ARTIFICE_SERIAL) 
	&& !GetObjProperty(itemAlvo,PROPNAME_DISPENSAVEL_FLAG)
	&& itemAlvo.ISA(POLCLASS_ITEM))//se não foi criado por player e não foi marcado como dispensável..
		var serialOriginal := GetObjProperty(itemAlvo,PROPNAME_CHAVE_ORIGINAL);
		if(!serialOriginal)
			SetObjProperty(itemAlvo,PROPNAME_CHAVE_ORIGINAL,itemAlvo.serial);//Grava serial do item original.
		endif
		MarcarRecriarItem(itemAlvo);
	endif 

	var isObjetoMovido := false;
	if(itemAlvo.ISA(POLCLASS_CONTAINER))
		var listaItens := EnumerateItemsInContainer(itemAlvo);
		foreach item in listaItens
			if(RandomInt(100) <= 30)
				continue;
			else
				MoveObjectToLocation(item,itemAlvo.x,itemAlvo.y,itemAlvo.z, itemAlvo.realm, MOVEOBJECT_FORCELOCATION);
				isObjetoMovido := true;
			endif
		endforeach
	endif

	TocaSFXDestruicao(itemAlvo,isObjetoMovido);
	CriaDestrocos(itemAlvo,RandomIntMinMax(1,3),4);
	DestroyItem(itemAlvo);
	//TODO implementar comando de Indestrutível.
	//TODO criar estilhaços no chão.
	//TODO Verificar se é container para destruir e espalhar seu conteúdo (Ou talvez criar um container "morto" e colocar tudo dentro).
	//TODO Logar destruição.
	//TODO Criar comando de gm para consultar os logs e gerênciar o respawn de itens
	//TODO Criar comando ou argumento do create para gms criarem itens dispensáveis.
endfunction

function TocaSFXDestruicao(itemAlvo,deveTocarTesouro)
	var material := GetObjProperty(itemAlvo,PROPNAME_MATERIAL);
	var sfx := 0;
	case (material)
		PEDRA: sfx := 0x11C; break;
		TIJOLO: sfx := 0x11C; break;
		MARMORE: sfx := 0x11C; break;
		ARENITO: sfx := 0x11C; break;
		OSSOS: sfx := 0x342; break;
		VIDRO: sfx := 0x03E; break;
		MADEIRA: sfx := 0x475; break;
		PANO: sfx := 0x33A; break;
		METAL: sfx := 0x056; break;
		COURO: sfx := 0x04A; break;
		default: sfx := 0x33C;
	endcase
	PlaySoundEffect(itemAlvo,sfx);
	if(deveTocarTesouro)
		PlaySoundEffect(itemAlvo,0x2E1);
	endif
endfunction

function LimpaObjPropsDestruiveis(itemAlvo)
	EraseObjProperty(itemAlvo,PROPNAME_DURABILIDADE);
	EraseObjProperty(itemAlvo,PROPNAME_EMPERRADO);
endfunction

function PegaResistenciaItem(itemAlvo)
	var propRes := GetObjProperty(itemAlvo,PROPNAME_RESITEM_MULT);

	if(propRes)
		return RESISTENCIA_BASE * propRes;
	else
		return RESISTENCIA_BASE;
	endif
endfunction

function PegaDurabilidadeTotal(item)
	var itemdesc := ReadConfigFile(":*:itemdesc");
    var elem := FindConfigElem(itemdesc, item.objtype);
	var pesoEscolhido := 0;

	if(item.ISA(POLCLASS_ARMOR) || item.ISA(POLCLASS_WEAPON))
		return item.maxhp;
	else
		var propMaxDurab := GetObjProperty(item,PROPNAME_DURABILIDADE_MAX);
		if(GetObjProperty(item,PROPNAME_DURABILIDADE_MAX))
			return propMaxDurab;
		endif
		
		if(elem)
			if(elem.weight > 0)
				pesoEscolhido := CInt(elem.weight);
				return Ceil(pesoEscolhido);
			endif
		endif

		var infoTile := IU_GetTileInfo(item.Graphic);
		pesoEscolhido := CInt(infoTile.Weight);
		return Ceil(pesoEscolhido) * 4;
	endif
endfunction

function MontaAtributosItem(item)
	var itemAtrib := struct;
		itemAtrib.+resistencia := cInt(PegaResistenciaItem(item));
		itemAtrib.+durabTotal := PegaDurabilidadeTotal(item);

		if(item.ISA(POLCLASS_ARMOR) || item.ISA(POLCLASS_WEAPON))
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

function DanificaItem(item,durabilidade,dano)
	var durabilidadeRestante :=  durabilidade - dano;

	if(Item.ISA(POLCLASS_ARMOR) || Item.ISA(POLCLASS_WEAPON))
		item.hp := CInt(durabilidadeRestante);
	endif
	SetObjProperty(item,PROPNAME_DURABILIDADE,durabilidadeRestante);

	return durabilidadeRestante;
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
	catalogo.insert(INDEFINIDO,{0x1038,0x1B26,0x1B9D,0x1B9E,0x0F3B,0x1038});

	return catalogo;
endfunction

function CriaDestrocos(itemAlvo,quantidade,distanciaMax := 0, deveCriarOrigem := true)
	var material := GetObjProperty(itemAlvo,PROPNAME_MATERIAL);
	var arrayDestrocos := array;
	var corDestroco := 1001;
	if(material)
		arrayDestrocos := catalogoDestrocos[configRecursos[material].Type];
		corDestroco := configRecursos[material].Color;
	else
		arrayDestrocos := catalogoDestrocos[INDEFINIDO];
	endif

	var i;
	for (i := 1; i <= quantidade; i += 1)
		var adicionalX := RandomInt(distanciaMax) * PositivoNegativo(RandomInt(100));
		var adicionalY := RandomInt(distanciaMax) * PositivoNegativo(RandomInt(100));
		var destrocoGraphic := arrayDestrocos[RandomIntMinMax(1,arrayDestrocos.size())];
		
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
		
		ConfiguraDestroco(destroco,configRecursos[material].type,corDestroco);
	endfor
endfunction

function ConfiguraDestroco(byref destroco,tipo,corDestroco)
	destroco.color := corDestroco;
	destroco.decayat := ReadGameClock() + DECAYTIME_DESTROCO;
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
			default:
				novoNome:=	"destroços";
		endcase
		SetName(destroco,novoNome);
endfunction