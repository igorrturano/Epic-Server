//$Id: checks.inc 128 2006-05-27 17:36:42Z austinheilman $

/*===============================================================
* Current Version
* CHECKS.INC - v1.0
* Updated 9/28/2005 12:15PM
*
* -- Revision v1.0 --
* Austin:
*  Created include file
===============================================================*/

use uo;
use os;

/*
 * CP_RootContainerLocked(object)
 *
 * Purpose
 * Determines if any containers the object is in are locked.
 *
 * Parameters
 * Object:	Object to check the containers of.
 *
 * Return value
 * Returns 1 if a locked container is found.
 *
 */
function CP_RootContainerLocked(object)
	var temp := object;
	while ( temp.container )
		if ( (temp.container).locked )
			return 1;
		else 
			// Move onto checking the next container.
			temp := temp.container;
		endif
		SleepMS(2);
	endwhile
	
	return 0;
endfunction

/*
 * IsMountSwappableEquip(item)
 *
 * Itens que um jogador montado pode tirar/guardar da mochila para
 * trocar de equipamento sem desmontar: armas e escudos. Demais
 * armaduras e itens continuam bloqueados enquanto montado.
 *
 * @return 1 para arma (POLCLASS_WEAPON) ou escudo, 0 caso contrário.
 */
function IsMountSwappableEquip(item)
	if (!item)
		return 0;
	endif

	return item.IsA(POLCLASS_WEAPON) || (item.IsA(POLCLASS_ARMOR) && item.isShield());
endfunction