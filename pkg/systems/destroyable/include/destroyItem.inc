use uo;
use os;

include ":combat:combat";
include ":attributes:attributes";
include ":destroyable:include/destroyItemPersist";
include ":itemutils:itemInfo";

const PROPNAME_DURABILIDADE := "durabItem"; 
const PROPNAME_CHAVE_ORIGINAL := "serialOriginal"; 
const PROPNAME_EMPERRADO := "isEmperrado"; 
const PROPNAME_RESITEM_MULT := "resItem"; 
const PROPNAME_ARTIFICE_SERIAL := "crafterserial"; 
const PROPNAME_DISPENSAVEL_FLAG := "dispensavel"; 
const RESISTENCIA_BASE := 5; 

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

	DestroyItem(itemAlvo);
	//TODO criar estilhaços no chão.
	//TODO Verificar se é container, destruir e espalhar seu conteúdo.
	//TODO Logar destruição.
	//TODO Se for item criado por gm, disparar script de respawn.
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

function PegaDurabilidadeTotal(itemAlvo)
	var itemdesc := ReadConfigFile(":*:itemdesc");
    var elem := FindConfigElem(itemdesc, itemAlvo.objtype);
	var pesoEscolhido := 0;

	if(elem)
		if(elem.weight > 0)
			pesoEscolhido := CInt(elem.weight);
			Print("Peso no itemDesc: " + pesoEscolhido);
			return Ceil(pesoEscolhido);
		endif
	endif

	var infoTile := IU_GetTileInfo(itemAlvo.Graphic);
	pesoEscolhido := CInt(infoTile.Weight);
	Print("Peso no Tile: " + infoTile.Weight);
	return Ceil(pesoEscolhido) * 2;
endfunction

function MontaAtributosItem(item)
	var itemAtrib := struct;
		itemAtrib.+resistencia := cInt(PegaResistenciaItem(item));
		itemAtrib.+durabTotal := PegaDurabilidadeTotal(item);

		var propDur := GetObjProperty(item,PROPNAME_DURABILIDADE);
		if(!propDur)
			SetObjProperty(item,PROPNAME_DURABILIDADE,itemAtrib.durabTotal);
			itemAtrib.+durabilidade := itemAtrib.durabTotal;
		else
			itemAtrib.+durabilidade := propDur;
		endif

	return itemAtrib;
endfunction