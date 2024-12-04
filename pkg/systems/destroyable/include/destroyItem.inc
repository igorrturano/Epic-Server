use uo;
use os;
use basic;

include ":combat:combat";
include ":attributes:attributes";
include ":destroyable:include/destroyItemPersist";
include ":itemutils:itemInfo";

const PROPNAME_DURABILIDADE := "durabItem"; 
const PROPNAME_CHAVE_ORIGINAL := "serialOriginal"; 
const PROPNAME_RESITEM_MULT := "resItem"; 
const PROPNAME_ARTIFICE_SERIAL := "crafterserial"; 
const PROPNAME_EMPERRADO := "isEmperrado"; 
const PROPNAME_DISPENSAVEL_FLAG := "isDispensavel"; 
const PROPNAME_INDESTRUTIVEL := "isIndestrutivel"; 
const PROPNAME_MATERIAL := "material"; 

const RESISTENCIA_BASE := 5; 

enum TIPO_MATERIAIS
	PEDRA := "Stone",
	TIJOLO := "Brick",
	MARMORE := "marmore",
	ARENITO := "arenito",
	OSSOS := "ossos",
	VIDRO := "Glass",
	MADEIRA := "Wood",
	PANO := "Cloth",
	METAL := "Metal"
endenum

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
	&& !GetObjProperty(itemAlvo,PROPNAME_DISPENSAVEL_FLAG))//se não foi criado por player e não foi marcado como dispensável..
		var serialOriginal := GetObjProperty(itemAlvo,PROPNAME_CHAVE_ORIGINAL);
		if(!serialOriginal)
			SetObjProperty(itemAlvo,PROPNAME_CHAVE_ORIGINAL,itemAlvo.serial);//Grava serial do item original.
		endif
		MarcarRecriarItem(itemAlvo);
	endif 

	if(itemAlvo.ISA(POLCLASS_CONTAINER))

	endif

	DestroyItem(itemAlvo);
	//TODO implementar comando de Indestrutível.
	//TODO criar estilhaços no chão.
	//TODO Verificar se é container para destruir e espalhar seu conteúdo (Ou talvez criar um container "morto" e colocar tudo dentro).
	//TODO Logar destruição.
	//TODO Criar comando de gm para consultar os logs e gerênciar o respawn de itens
	//TODO Criar comando ou argumento do create para gms criarem itens dispensáveis.
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