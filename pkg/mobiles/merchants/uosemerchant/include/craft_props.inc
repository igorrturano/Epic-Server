include "include/utils";
include "include/colormap";

function GetItemType(item)
   var item_type := 0;
	if (item.isA(POLCLASS_ARMOR))
      if (item.tile_layer == 6)
         item_type := "Elmos/Mascaras";
      else
		   item_type := "Armaduras";
      endif
	elseif (item.isBow())
		item_type := "Arcos";
	elseif (item.isMagicWeapon())
		item_type := "Armas Mágicas";
	elseif (item.isA(POLCLASS_WEAPON))
		item_type := "Armas";
	elseif (cint(item.tile_layer) in array{6,8,10,14,18})
		item_type := "Joias";
	elseif (item.isA(POLCLASS_CONTAINER))
		item_type := "Containers";
	endif

   return item_type;
endfunction
