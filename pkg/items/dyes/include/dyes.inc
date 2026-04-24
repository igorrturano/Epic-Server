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
 * CanDye(item)
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
function CanDye(item)
	var config := ReadConfigFile(":*:itemdesc");
	config := config[item.objtype];
	
	if (cint(GetObjProperty(item, "cannotdye")) == 1)
		return 0;
	endif
	
	if ( config.CanDye )
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
 * Gets a random dye color based on RP-wise cultures and colors.
 *
 * Parameters
 *
 * Return value
 * Integer
 *
 */
function RandomDyeColor()
	// Available RP cultures and basic colors
	var cultures := {"AIGLANA", "HAROLOTH", "LESTE", "BJORSKA", "ALURIEL", "BADURAN", "GULTHRAK", "COLHESIANA"};
	var basicColors := {"Rosa", "Vermelho", "Azul", "Verde", "Amarelo", "Laranja", "Neutro", "Cinza"};
	
	// Combine all available color options
	var allColors := cultures + basicColors;
	
	// Select a random color/culture
	var selectedColor := allColors[RandomInt(allColors.size()) + 1];
	
	// Get the actual color value using the same logic as cor.src
	return GetRPColorValue(selectedColor);
endfunction

/*
 * GetRPColorValue(color)
 *
 * Purpose
 * Gets the actual color value for RP-wise colors and cultures.
 * Based on the logic from cor.src command.
 *
 * Parameters
 * color: String representing the culture or basic color name
 *
 * Return value
 * Integer color value
 *
 */
function GetRPColorValue(color)
	case (color)
		"Rosa":      return RandomIntMinMax(1201, 1254);
		"Vermelho":  return RandomIntMinMax(1601, 1654);
		"Azul":      return RandomIntMinMax(1301, 1354);
		"Verde":     return RandomIntMinMax(1401, 1454);
		"Amarelo":   return RandomIntMinMax(1701, 1754);
		"Laranja":   return RandomIntMinMax(1501, 1554);
		"Neutro":    return RandomIntMinMax(1801, 1908);
		"Cinza":     return RandomInt({0, 946, 947});
		"AIGLANA":   return RandomColorFromRanges({
						RandomIntMinMax(1601, 1654),  // Vermelho
						RandomIntMinMax(1701, 1754),  // Amarelo
						RandomIntMinMax(1801, 1908)   // Neutro
					 });
		"HAROLOTH":  return RandomColorFromRanges({
						RandomIntMinMax(1201, 1254),  // Rosa
						RandomInt({0, 946, 947}),     // Cinza
						RandomIntMinMax(1301, 1354),  // Azul
						RandomIntMinMax(1801, 1908)   // Neutro
					 });
		"LESTE":     return RandomColorFromRanges({
						RandomIntMinMax(1701, 1754),  // Amarelo
						RandomIntMinMax(1501, 1554),  // Laranja
						RandomIntMinMax(1201, 1254),  // Rosa
						RandomInt({0, 946, 947})      // Cinza
					 });
		"BJORSKA":   return RandomColorFromRanges({
						RandomIntMinMax(1401, 1454),  // Verde
						RandomIntMinMax(1701, 1754),  // Amarelo
						RandomIntMinMax(1801, 1908)   // Neutro
					 });
		"ALURIEL":   return RandomColorFromRanges({
						RandomIntMinMax(1201, 1254),  // Rosa
						RandomIntMinMax(1301, 1354),  // Azul
						RandomIntMinMax(1401, 1454),  // Verde
						RandomIntMinMax(1801, 1908)   // Neutro
					 });
		"BADURAN":   return RandomColorFromRanges({
						RandomIntMinMax(1501, 1554),  // Laranja
						RandomIntMinMax(1701, 1754),  // Amarelo
						RandomIntMinMax(1801, 1908),  // Neutro
						RandomInt({0, 946, 947})      // Cinza
					 });
		"GULTHRAK":  return RandomColorFromRanges({
						RandomIntMinMax(1201, 1254),  // Rosa
						RandomIntMinMax(1601, 1654),  // Vermelho
						RandomIntMinMax(1801, 1908),  // Neutro
						RandomInt({0, 946, 947})      // Cinza
					 });
		"COLHESIANA":  return RandomColorFromRanges({
						RandomIntMinMax(1501, 1554),  // Laranja
						RandomIntMinMax(1701, 1754),  // Amarelo
						RandomIntMinMax(1401, 1454),  // Verde
						RandomIntMinMax(1301, 1354),  // Azul
						RandomIntMinMax(1801, 1908)   // Neutro
					 });
		default:     
			// Fallback to original random color if somehow an invalid color is passed
			var color := RandomInt(MAX_COLOR - MIN_COLOR);
			return color + MIN_COLOR;
	endcase
endfunction

/*
 * RandomColorFromRanges(colorRanges)
 *
 * Purpose
 * Selects a random color from an array of color ranges.
 * Helper function for culture-based colors.
 *
 * Parameters
 * colorRanges: Array of color values/ranges
 *
 * Return value
 * Integer color value
 *
 */
function RandomColorFromRanges(colorRanges)
	return colorRanges[RandomInt(colorRanges.size()) + 1];
endfunction
