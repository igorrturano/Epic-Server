
/* O objetivo desse include é fornecer funções que são a parte do sistema default do POL. 
* por exemplo, no FLS itens de blacksmith podem consumir cabos, couros e cera. como o crafting
*do pol so procura por um material, essas funçoes tiveram que ser feitas pro FLS
*/

use uo;
use cfgfile;

include "fls_objtypes";


function CheckForPen(backpack, charges)

	foreach item in EnumerateItemsInContainer (backpack)
		if ( item.graphic == 4031 )
			if ( Cint(GetObjProperty(item, "cargas")) >= charges )
				SetObjProperty(item , "cargas", (Cint(GetObjProperty(item, "cargas")) - charges));
				return 1;
			endif
		endif
	endforeach

	return 0;
endfunction

function SearchForHides(who, hidesamt)

    var counter := 0;
      
    if (hidesamt != 0)
       foreach item in EnumerateItemsInContainer (who.backpack)
          if ( item.objtype == 0xee45)
             counter := GetAmount( item );
          endif
       endforeach
       if (counter < hidesamt)
          return 0;
       else
           return 1;
       endif
    endif

endfunction

function SearchForPoles(who, polesamt)
    var counter := 0;
      
    if (polesamt != 0)
       foreach item in EnumerateItemsInContainer (who.backpack)
          if ( item.objtype == 0xee71)
             counter := GetAmount( item );
          endif
       endforeach
       if (counter < polesamt)
          return 0;
       else
           return 1;
       endif
    endif

endfunction

function SearchForWax(who)

    var counter := 0;

       foreach item in EnumerateItemsInContainer (who.backpack)
          if ( item.objtype == 0x142a)
             counter := 1;
          endif
       endforeach
       if (counter >= 1)
          return 1;
       else
           return 0;
       endif


endfunction


function consumeWax(who)

	foreach k in EnumerateItemsInContainer(who.backpack)
		if (k.objtype ==  0x142a)
			DestroyItem(k);
			var item := CreateItemInContainer (who.backpack, UOBJ_EMPTY_KETTLE, 1);
			item.movable := 1;
			break;
		endif
	endforeach

endfunction

function consumeHides(who, amt)
	if (!ConsumeSubstance( who.backpack, 0xee45, amt ));
		return 0;
	else
		return 1;
	endif
endfunction

function consumePoles(who, amt)
	if (!ConsumeSubstance( who.backpack, 0xee71, amt ));
		return 0;
	else
		return 1;
	endif
endfunction

//essa funçao eh o generico do SearchFor acima. é so colocar if (AmountInPack(who, poles) > 3) 
// por exemplo.

function AmountInPack(mobile, obj_type, color:=0)
	var amount := 0;
	foreach item in EnumerateItemsInContainer(mobile.backpack)
		if( item.objtype == obj_type )
			if ( !color || item.color == color )
				amount := amount + item.amount;
			endif
		endif
		sleepms(2);
	endforeach
	
	return amount;
endfunction

//Check for nails nao pode usar a funcao acima :/ pq ele precisa checar a Prop "qt" em cada caida
//de prego
function CheckForNails(who)

	var pregos_total := 0;
	var qte; 	foreach item in EnumerateItemsInContainer (who.backpack)
		if ( item.objtype == 0xee72)
			qte := Cint(GetObjProperty(item, "qt"));
			if (qte <= 0)
				DestroyItem(item);
				SendSysMessage(who, "Uma das caixas de pregos estava vazia e foi destruida");
			else
				pregos_total := (pregos_total + qte);
			endif
		endif
	endforeach

	return pregos_total;

endfunction

function ConsumeNails(who, pregonec)

	var qte := 0;
	foreach item in EnumerateItemsInContainer (who.backpack)
		if (pregonec > 0)
			if ( item.objtype == 0xee72)
				qte := Cint(GetObjProperty(item, "qt"));
				if (qte < pregonec)
					pregonec := (pregonec - qte);
					DestroyItem(item);
				elseif (qte == pregonec)
					pregonec := (pregonec - qte);
					return 1;      
					DestroyItem(item);
				elseif (qte > pregonec)
					qte := (qte - pregonec);
					pregonec := (pregonec - qte);
					SetObjProperty(item, "qt", qte);
					return 1;
				endif
			endif
		endif
	endforeach

	if (pregonec > 0)
		return 0;
	else
		return 1;
	endif

endfunction


function CheckForAnvil(who)

	foreach itm in ListItemsNearLocation(who.x, who.y, who.z, 1, who.realm);
		if(itm.objtype == UOBJ_ANVIL1 or itm.objtype == UOBJ_ANVIL2)
			return 1;
		endif
	endforeach
	return 0;
endfunction

function CheckForForge(who)

	foreach itm in ListItemsNearLocation(who.x, who.y, who.z, 2, who.realm);
		if((itm.objtype == UOBJ_SMALL_FORGE) or (itm.objtype >= UOBJ_FORGE_START and itm.objtype <= UOBJ_FORGE_END))
                    //if (GetObjProperty(itm, "status"))
        return 1;
                    //endif
		endif
	endforeach
	return 0;
endfunction


