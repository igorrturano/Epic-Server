/////////////////////////////////////////////
// Icon Manager - Sistema Automático
// Carrega icons.cfg automaticamente
/////////////////////////////////////////////

use cfgfile;

// Cache global (inicializado automaticamente)
var g_icon_cache := 0;

// Inicializar cache AUTOMATICAMENTE
function InitIconCache()
	// Se já inicializado, retorna cache existente
	if (g_icon_cache)
		return g_icon_cache;
	endif
	
	// Criar estrutura de cache
	g_icon_cache := struct{
		"specials" := dictionary,
		"commands" := dictionary,
		"skills" := dictionary,
		"habilidades" := dictionary,
		"defaults" := dictionary
	};
	
	// Carregar icons.cfg
	var cfg := ReadConfigFile(":GMHab:config/icons");
	if (!cfg)
		print("AVISO: icons.cfg não encontrado, usando ícones padrão");
		return g_icon_cache;
	endif
	
	var all_keys := GetConfigStringKeys(cfg);
	
	// Popular cache
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
			g_icon_cache.habilidades[name] := icon_data;
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
function GetHabilidadeIcon(hab_name, class_name)
	var cache := InitIconCache();
	
	// Tentar ícone específico (futuro)
	if (cache.habilidades.exists(hab_name))
		return cache.habilidades[hab_name];
	endif
	
	// Usar padrão da classe
	if (cache.defaults.exists(class_name))
		return cache.defaults[class_name];
	endif
	
	// Fallback final
	return struct{"iconid" := 0x9bac, "hue" := 0};
endfunction
