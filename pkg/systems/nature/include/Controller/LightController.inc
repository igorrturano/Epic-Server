/*
 * LightController.inc
 *
 * Consolidated lighting system for all player vision handling.
 * Single source of truth for player lighting.
 *
 * Config: :nature:config/daynight.cfg
 * Light scale: 0 = brightest, 30 = darkest
 */

use uo;
use cfgfile;

include "include/dungeons";
include "include/flash";
include "include/epicConstants";
include ":tn:cooldown";

const LIGHT_DURATION := 9000;

function GetLightConfig()
	var config := ReadConfigFile(":nature:config/daynight");
	return config["settings"];
endfunction

function HasNightVisionEffect(player)
	return GetCooldown(player, "lightvision") || GetCooldown(player, "nightvision");
endfunction

function ApplyPlayerLight(player)
	if (!player || player.npctemplate)
		return 0;
	endif

	var hasNightVision := HasNightVisionEffect(player);

	if (HasReverseVision(player))
		return ApplyDrowLight(player, hasNightVision);
	else
		return ApplyNormalLight(player, hasNightVision);
	endif
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

function ApplyNormalLight(player, hasNightVision := 0)
	var config := GetLightConfig();

	if (InDungeon(player))
		if (hasNightVision)
			player.setlightlevel(CInt(config.MagicalVisionLevel), LIGHT_DURATION);
		else
			player.setlightlevel(CInt(config.DungeonDarkness), LIGHT_DURATION);
		endif
	else
		var globalLight := CInt(GetGlobalProperty("lightlevel"));
		player.setlightlevel(globalLight, LIGHT_DURATION);
	endif

	SendScreenFlash(player, 0);
	return 1;
endfunction

function ApplyDrowLight(player, hasNightVision := 0)
	var config := GetLightConfig();
	var dungeonDarkness := CInt(config.DungeonDarkness);

	if (InDungeon(player))
		if (hasNightVision)
			player.setlightlevel(CInt(config.MagicalVisionLevel), LIGHT_DURATION);
		else
			player.setlightlevel(CInt(config.DrowCaveVision), LIGHT_DURATION);
		endif
		SendScreenFlash(player, 0);
		return 1;
	endif

	var globalLight := CInt(GetGlobalProperty("lightlevel"));
	var reversedLight := Abs(dungeonDarkness - globalLight);
	player.setlightlevel(reversedLight, LIGHT_DURATION);

	if (globalLight < 20)
		SendScreenFlash(player, FLASH_WHITE);
	else
		SendScreenFlash(player, 0);
	endif

	return 1;
endfunction

function ApplyAllPlayersLight()
	foreach player in EnumerateOnlineCharacters()
		Sleepms(2);
		ApplyPlayerLight(player);
	endforeach
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

function ApplyMagicalVision(player, duration)
	var config := GetLightConfig();
	var lightLevel := CInt(config.MagicalVisionLevel);
	player.setlightlevel(lightLevel, duration);

	if (HasReverseVision(player))
		SetCooldown(player, "lightvision", duration);
	else
		SetCooldown(player, "nightvision", duration);
	endif

	return lightLevel;
endfunction
