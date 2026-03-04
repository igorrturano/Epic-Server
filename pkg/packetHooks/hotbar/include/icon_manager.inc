/////////////////////////////////////////////
// Icon Manager - Direct CFG Access
//
// Reads icons from icons.cfg using POL's built-in CFG cache.
// No custom caching - POL already caches CFG files in memory.
// Changes to icons.cfg take effect immediately.
/////////////////////////////////////////////

use cfgfile;

// ============================================================================
// GetSpecialIcon - Get icon for class special ability
// ============================================================================
// Reads directly from icons.cfg (no caching - POL caches CFG files)
// ============================================================================
function GetSpecialIcon(special_name)
	var cfg := ReadConfigFile(":hotbar:config/icons");
	if (!cfg)
		return struct{"iconid" := 0x7E, "hue" := 0};
	endif

	var key := StrReplace(special_name, " ", "");
	var elem := FindConfigElem(cfg, key);
	if (!elem)
		return struct{"iconid" := 0x7E, "hue" := 0};
	endif

	return struct{
		"iconid" := CInt(GetConfigString(elem, "iconid")),
		"hue" := CInt(GetConfigString(elem, "hue"))
	};
endfunction

// ============================================================================
// GetCommandIcon - Get icon for player command
// ============================================================================
// Reads directly from icons.cfg (no caching - POL caches CFG files)
// Strips leading dot from command name if present
// ============================================================================
function GetCommandIcon(command_name)
	var cfg := ReadConfigFile(":hotbar:config/icons");
	if (!cfg)
		return struct{"iconid" := 0x9b90, "hue" := 0};
	endif

	var elem := FindConfigElem(cfg, command_name);
	if (!elem)
		return struct{"iconid" := 0x9b90, "hue" := 0};
	endif

	var iconid := CInt(GetConfigString(elem, "iconid"));
	var hue := CInt(GetConfigString(elem, "hue"));

	return struct{
		"iconid" := iconid,
		"hue" := hue
	};
endfunction

// ============================================================================
// GetAbilityIcon - Get icon for class ability
// ============================================================================
// Reads directly from icons.cfg (no caching - POL caches CFG files)
// Falls back to class default icon if ability-specific icon not found
// ============================================================================
function GetAbilityIcon(hab_id)
	var cfg := ReadConfigFile(":abilities:configs/habilidades");
	if (!cfg)
		return struct{"iconid" := DEFAULT_ABILITY_ICON, "hue" := 0};
	endif

	var elem := FindConfigElem(cfg, hab_id);
	if (elem)
		var icon := CInt(GetConfigString(elem, "Icon"));
		var hue := CInt(GetConfigString(elem, "Hue"));
		return struct{
			"iconid" := icon,
			"hue" := hue
		};
	endif

	// Final fallback
	return struct{"iconid" := DEFAULT_ABILITY_ICON, "hue" := 0};
endfunction
