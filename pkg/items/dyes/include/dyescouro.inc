// $Id: dyes.inc 891 2005-11-04 06:29:43Z muaddiblsd $

/*===============================================================
* Current Version
* DYES.INC - v1.0
* Updated 10/3/2005 12:22PM
*
* -- Revision v1.0 --
* Austin:
*  Created include file
===============================================================*/

use uo;
use util;
use cfgfile;

const MIN_COLOR := 2;
const MAX_COLOR := 1001;

/*
 * CanDyeCouro(item)
 *
 * Purpose
 * Checks an object's itemdesc.cfg entry to see if it is dyable.
 *
 * Parameters
 * item:	Item that will be dyed.
 *
 * Return value
 * Returns 1 if it can be dyed
 * Returns 0 if it can not
 *
 */
function CanDyeCouro(item)
	var config := ReadConfigFile(":*:itemdesc");
	config := config[item.objtype];
	
	if (cint(GetObjProperty(item, "cannotdye")) == 1)
		return 0;
	endif
	
	if ( config.CanDyeCouro )
		return 1;
	elseif ( config.dyeable )
		return 1;
	endif
	return 0;
endfunction

/*
 * ValidDyeColor(mobile, color)
 *
 * Purpose
 * Checks if the color sent by the dyes is a valid dye color.
 *
 * Parameters
 * mobile:	Mobile doing the dying.
 * color:	color set by the dyes.
 *
 * Return value
 * Returns 1 if the color is allowed
 * Returns 0 if the color is invalid
 *
 */
function ValidDyeColor(mobile, color)
	if ( mobile.cmdlevel )
		return 1;
	elseif ( (color >= MIN_COLOR) && (color <= MAX_COLOR) )
		return 1;
	else
		return 0;
	endif
endfunction

/*
 * RandomDyeColor()
 *
 * Purpose
 * Gets a random dye color.
 *
 * Parameters
 *
 * Return value
 * Integer
 *
 */
function RandomDyeColor()
	var color := RandomInt(MAX_COLOR - MIN_COLOR);
	color := color+MIN_COLOR;

	return color;
endfunction
