/*
 * LightController.inc
 *
 * Player vision handling.
 *
 * Philosophy:
 * - Let nature (weather, regions) handle all natural light
 * - Drow vision is handled client-side via DrowVisionMode (FLASH_DROW_ON)
 * - Only magical effects (potions, spells) use setlightlevel
 *
 * Config: :nature:config/daynight.cfg
 * Light scale: 0 = brightest, 30 = darkest
 */

use uo;
use cfgfile;

include "include/flash";
include "include/epicConstants";
include ":tn:cooldown";

function GetLightConfig()
	var config := ReadConfigFile(":nature:config/daynight");
	return config["settings"];
endfunction

function HasReverseVision(player)
	var chardata := GetObjProperty(player, "chardata");
	if (!chardata)
		return 0;
	endif

	if (chardata.raca == DROW)
		return 1;
	endif

	if (chardata.culture == HAROLOTH)
		return 1;
	endif

	if (CInt(GetObjProperty(player, "reversevision")) == 1)
		return 1;
	endif

	if (GetObjProperty(player, "metamorfose"))
		return 1;
	endif

	return 0;
endfunction

/*
 * InitPlayerVision(player)
 *
 * Call ONCE on login or race change to enable client-side drow vision.
 * The client handles light inversion automatically when DrowVisionMode is enabled.
 * Normal players don't need anything - DrowVisionMode defaults to false.
 */
function InitPlayerVision(player)
	if (!player || player.npctemplate)
		return 0;
	endif

	if (HasReverseVision(player))
		SendScreenFlash(player, FLASH_DROW_ON);
	endif

	return 1;
endfunction

/*
 * ApplyMagicalVision(player, duration)
 *
 * Only for magical effects (potions, spells).
 * Sets a personal light level for the duration of the effect.
 */
function ApplyMagicalVision(player, duration)
	var config := GetLightConfig();
	var lightLevel := CInt(config.MagicalVisionLevel);
	player.setlightlevel(lightLevel, duration);

	SetCooldown(player, "nightvision", duration);

	return lightLevel;
endfunction

function GetMoonLightMod()
	var config := GetLightConfig();
	var fase := GetGlobalProperty("moonfase");

	case (fase)
		"Kalendae":    return CInt(config.MoonKalendae);
		"Nonae":       return CInt(config.MoonNonae);
		"Idus":        return CInt(config.MoonIdus);
		"Decrescens":  return CInt(config.MoonDecrescens);
		default:       return 24;
	endcase
endfunction
