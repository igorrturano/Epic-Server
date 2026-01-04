/////////////////////////////////////////////
// Icon Manager - Automatic System
// Loads icons.cfg automatically
/////////////////////////////////////////////

use cfgfile;

// Global cache (initialized automatically)
var g_icon_cache := 0;

// Initialize cache AUTOMATICALLY
function InitIconCache()
	// If already initialized, return existing cache
	if (g_icon_cache)
		return g_icon_cache;
	endif

	// Create cache structure
	g_icon_cache := struct{
		"specials" := dictionary,
		"commands" := dictionary,
		"skills" := dictionary,
		"abilities" := dictionary,
		"defaults" := dictionary
	};
	
	// Load icons.cfg
	var cfg := ReadConfigFile(":hotbar:config/icons");
	if (!cfg)
		print("WARNING: icons.cfg not found, using default icons");
		return g_icon_cache;
	endif

	var all_keys := GetConfigStringKeys(cfg);

	// Populate cache
	foreach key in all_keys
		var elem := FindConfigElem(cfg, key);
		if (!elem)
			continue;
		endif
		
		var iconid := CInt(GetConfigString(elem, "iconid"));
		var hue := CInt(GetConfigString(elem, "hue"));
		var name := GetConfigString(elem, "name");
		
		if (!iconid)
			iconid := 0x9b90; // Fallback
		endif
		
		var icon_data := struct{
			"iconid" := iconid,
			"hue" := hue
		};
		
		// Classificar por tipo baseado no nome
		// Specials: AtaqueExtra, Evasao, RecargaRapida, ConjuracaoRapida, VelocidadeArcana
		if (key == "AtaqueExtra" || key == "Evasao" || key == "RecargaRapida" || 
		    key == "ConjuracaoRapida" || key == "VelocidadeArcana")
			g_icon_cache.specials[name] := icon_data;
		// Commands: começam com ponto no name
		elseif (name && name[1] == ".")
			g_icon_cache.commands[name] := icon_data;
		// Habilidades default: Guerreiro, Ladino, Sabio, Artifice (sem campo name)
		elseif (key == "Guerreiro" || key == "Ladino" || key == "Sabio" || key == "Artifice")
			g_icon_cache.defaults[key] := icon_data;
		// Skills: Alchemy, Arcana, Sneak, etc (lista específica)
		elseif (key == "Alchemy" || key == "Arcana" || key == "Sneak" || key == "Pickpocketing" ||
		        key == "Awareness" || key == "Mechanics" || key == "Medicine" || key == "Survival" ||
		        key == "Gathering" || key == "HandleAnimal" || key == "Theology")
			g_icon_cache.skills[name] := icon_data;
		// Habilidades individuais: todo o resto
		else
			g_icon_cache.abilities[name] := icon_data;
		endif
	endforeach
	
	return g_icon_cache;
endfunction

// Obter ícone de special
function GetSpecialIcon(special_name)
	var cache := InitIconCache();
	
	if (cache.specials.exists(special_name))
		return cache.specials[special_name];
	endif
	
	// Fallback
	return struct{"iconid" := 0x7E, "hue" := 0};
endfunction

// Obter ícone de comando
function GetCommandIcon(command_name)
	var cache := InitIconCache();
	
	if (cache.commands.exists(command_name))
		return cache.commands[command_name];
	endif
	
	// Fallback
	return struct{"iconid" := 0x9b90, "hue" := 0};
endfunction

// Obter ícone de skill
function GetSkillIcon(skill_name)
	var cache := InitIconCache();
	
	if (cache.skills.exists(skill_name))
		return cache.skills[skill_name];
	endif
	
	// Fallback
	return struct{"iconid" := 0x9b90, "hue" := 0};
endfunction

// Obter ícone de habilidade
function GetAbilityIcon(hab_name, class_name)
	var cache := InitIconCache();

	// Tentar ícone específico (futuro)
	if (cache.abilities.exists(hab_name))
		return cache.abilities[hab_name];
	endif

	// Usar padrão da classe
	if (cache.defaults.exists(class_name))
		return cache.defaults[class_name];
	endif

	// Fallback final
	return struct{"iconid" := 0x9bac, "hue" := 0};
endfunction

// GetSpellIcon() has been removed - spells now read Icon/Hue directly from their config files
// This follows the "single source of truth" pattern used by timedscripts
