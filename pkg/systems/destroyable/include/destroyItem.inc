use uo;
use os;

include ":combat:combat";
include ":attributes:attributes";

const PROPNAME_DURABILIDADE := "durabItem"; 
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