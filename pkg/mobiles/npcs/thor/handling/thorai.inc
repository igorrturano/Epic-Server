use uo;
use cfgfile;
use util;

// ============================================================================
// HEURISTIC PATTERN MATCHING
// ============================================================================

function CheckHeuristicMatch(text, npc_type)
	var cfg := ReadConfigFile(":thor:config/heuristic_patterns");
	if (!cfg)
		return 0;
	endif

	text := Lower(text);

	// Check each pattern section
	foreach section_name in (GetConfigStringKeys(cfg))
		var elem := cfg[section_name];
		var patterns_str := GetConfigString(elem, "Patterns");
		if (!patterns_str)
			continue;
		endif

		var patterns := SplitWords(patterns_str, ",");

		// Check if text contains any pattern
		foreach pattern in patterns
			pattern := Lower(Trim(pattern));
			if (find(text, pattern, 1) > 0)
				var responses_str := GetConfigString(elem, "Responses");
				if (!responses_str)
					continue;
				endif

				var responses := SplitWords(responses_str, "|");
				if (responses.size() > 0)
					return responses[RandomInt(responses.size()) + 1];
				endif
			endif
		endforeach
	endforeach

	return 0; // No match
endfunction

function CheckNPCBusy(npc_type)
	var cfg := ReadConfigFile(":thor:config/npc_behavior");
	if (!cfg)
		return 0;
	endif

	var elem := cfg[npc_type];
	if (!elem)
		elem := cfg["Global"];
	endif

	if (!elem)
		return 0;
	endif

	var busy_chance := CInt(GetConfigString(elem, "BusyChance"));
	if (!busy_chance)
		busy_chance := 0;
	endif

	if (RandomInt(100) < busy_chance)
		var responses_str := GetConfigString(elem, "BusyResponses");
		if (!responses_str)
			return "Tô ocupado...";
		endif

		var responses := SplitWords(responses_str, "|");
		if (responses.size() > 0)
			return responses[RandomInt(responses.size()) + 1];
		endif
	endif

	return 0; // Not busy
endfunction

function BuildVariedContext(base_context, npc_type, npc := 0)
	// Default generic variations
	var culture_moods := array{"cansado", "irritado", "desconfiado", "apressado", "nervoso", "melancólico"};
	var culture_activities := array{
		"organizando mercadorias empoeiradas",
		"contando moedas com desconfiança",
		"limpando uma ferramenta velha",
		"olhando nervoso para a porta",
		"arrumando coisas na pressa",
		"suspirando fundo"
	};
	var culture_environments := array{
		"uma rajada de vento frio entra pela janela quebrada",
		"ouve-se gritos distantes lá fora",
		"a luz da vela tremula e quase apaga",
		"alguém corre apressado na rua",
		"um barulho estranho ecoa ao longe",
		"o ambiente está tenso e silencioso"
	};

	// Add culture-specific variations if NPC provided
	if (npc)
		var region := GetNPCRegion(npc);
		var culture := MapRegionToCulture(region);

		case (culture)
			"aigla":
				// Military, disciplined
				culture_moods := array{"disciplinado mas cansado", "alerta", "nostálgico da glória", "ríspido", "formal"};
				culture_activities := array{
					"afiando gladius",
					"verificando armadura de lorica",
					"polindo escudo scutum",
					"lendo relatório militar",
					"recontando moedas (denários)"
				};
				culture_environments := array{
					"ouve-se marcha de legionários ao longe",
					"bandeira aiglana (águia dourada) tremula",
					"muros improvisados cercam assentamento",
					"som de treinamento militar ecoa",
					"cheiro de óleo de armadura no ar"
				};
			"gulthrak":
				// Brutal, desert
				culture_moods := array{"brutal", "direto", "desconfortável (longe do deserto)", "tenso", "objetivo"};
				culture_activities := array{
					"afiando lâmina de osso",
					"examinando crânio Aklat",
					"bebendo água com reverência",
					"tocando areia em bolsa",
					"verificando troféu de caça"
				};
				culture_environments := array{
					"vento traz areia do deserto distante",
					"crânios decoram altar improvisado",
					"cheiro de couro e osso",
					"marcas tribais nas paredes",
					"ausência de vegetação lembra deserto"
				};
			"haroloth":
				// Drow, underdark, scheming
				culture_moods := array{"desconfiado", "calculista", "arrogante", "observador", "frio"};
				culture_activities := array{
					"polindo armadura de Illyra (inútil na superfície)",
					"observando sombras",
					"contando com dedos ágeis",
					"sussurrando em Idrith'har",
					"evitando luz direta"
				};
				culture_environments := array{
					"velas fracas - drow prefere penumbra",
					"decoração minimalista, útil",
					"símbolo de Idrith na parede",
					"cheiro de fungos e vinhos potentes",
					"eco lembra cavernas do Underdark"
				};
			"baduran":
				// Dwarf, forge, temperamental
				culture_moods := array{"temperamental", "hospitaleiro", "orgulhoso", "trabalhador", "irritado"};
				culture_activities := array{
					"retorcendo barba enfeitada",
					"martelando metal quente",
					"servindo cerveja",
					"ajustando jóias na barba",
					"examinando forja"
				};
				culture_environments := array{
					"som de martelo na bigorna ecoa",
					"cheiro de metal quente e cerveja",
					"forja fumega ao fundo",
					"gravuras ancestrais nas paredes de pedra",
					"barril de cerveja sempre cheio"
				};
			"bjorske":
				// Norse, honorable, generous
				culture_moods := array{"orgulhoso", "valente", "generoso", "saudoso do norte", "honorável"};
				culture_activities := array{
					"polindo lança nórdica",
					"verificando cavalo",
					"servindo mead (hidromel)",
					"olhando para norte com saudade",
					"afiando espada ao lado de fogueira"
				};
				culture_environments := array{
					"fogueira sagrada crepita",
					"vento frio do norte sopra",
					"estandarte nórdico tremula",
					"cavalos relincham próximos",
					"cheiro de mead e fogo de lenha"
				};
			"aluriel":
				// Elf, contemplative, arrogant
				culture_moods := array{"contemplativo", "arrogante", "gracioso", "paciente", "melancólico"};
				culture_activities := array{
					"ajustando arco élfico",
					"contemplando planta viva",
					"tocando música suave",
					"lendo manuscrito antigo",
					"polindo lâmina élfica fina"
				};
				culture_environments := array{
					"plantas vivas decoram espaço",
					"luz natural filtrada suavemente",
					"música élfica toca ao longe",
					"cheiro de ervas e flores",
					"silêncio contemplativo prevalece"
				};
			"colhesiana":
				// Halfling, comfortable, peaceful
				culture_moods := array{"hospitaleiro", "nervoso com grandes", "confortável", "orgulhoso de jardim", "pacífico"};
				culture_activities := array{
					"servindo biscoitos frescos",
					"cuidando de ervas",
					"preparando chá",
					"ajeitando almofadas",
					"limpando cachimbo"
				};
				culture_environments := array{
					"lareira crepita aconchegante",
					"cheiro de comida deliciosa",
					"toca limpa e organizada",
					"jardim floresce pela janela",
					"fumo de cachimbo paira suave"
				};
			"eastern":
				// Eastern, honorable, formal
				culture_moods := array{"disciplinado", "honorável", "formal", "reverente", "sábio"};
				culture_activities := array{
					"polindo katana sagrada",
					"realizando reverência",
					"servindo chá com cerimônia",
					"meditando em silêncio",
					"praticando caligrafia"
				};
				culture_environments := array{
					"ambiente minimalista perfeito",
					"incenso queima suavemente",
					"símbolo de dragão na parede",
					"silêncio reverente prevalece",
					"cada objeto tem propósito preciso"
				};
		endcase
	endif

	var mood := culture_moods[RandomInt(culture_moods.size()) + 1];
	var activity := culture_activities[RandomInt(culture_activities.size()) + 1];
	var environment := culture_environments[RandomInt(culture_environments.size()) + 1];

	var varied_context := base_context +
		"\n\nSeu humor atual: " + mood +
		"\nVocê está: " + activity +
		"\nNo ambiente: " + environment;

	return varied_context;
endfunction

// ============================================================================
// LORE LOADING HELPERS
// ============================================================================

function GetNPCRegion(npc)
	// Hybrid approach: check property first, calculate if missing
	var region := GetObjProperty(npc, "region");
	if (region)
		return region; // Static property set on spawn
	endif

	// Fallback: dynamic calculation
	region := GetRegionName(npc);
	if (!region)
		return "generic"; // Fallback to generic lore
	endif

	// Return region name - will be mapped to culture
	return region;
endfunction

function MapRegionToCulture(region)
	// Maps region names to culture/lore files
	// Based on game lore about which cultures live where

	if (!region || region == "generic")
		return "aigla"; // Default fallback
	endif

	// Convert to lowercase for case-insensitive matching
	var region_lower := Lower(region);

	// Underdark = Drows (Haroloth)
	if (find(region_lower, "underdark", 1) || find(region_lower, "umbreterna", 1))
		return "haroloth";
	endif

	// Desert regions = Orcs (Gulthrak)
	if (find(region_lower, "deserto", 1) || find(region_lower, "perdição", 1) || find(region_lower, "perdic", 1))
		return "gulthrak";
	endif

	// Frozen north = Bjorske (Norse)
	if (find(region_lower, "nortegelado", 1) || find(region_lower, "ice_dungeon", 1) || find(region_lower, "gelado", 1))
		return "bjorske";
	endif

	// Lava/volcano regions = could be multiple, default to Gulthrak (desert orcs)
	if (find(region_lower, "vulcao", 1) || find(region_lower, "lava", 1))
		return "gulthrak";
	endif

	// Swamp regions = could be multiple, default to Colhesiana (halflings)
	if (find(region_lower, "swamp", 1) || find(region_lower, "pantano", 1))
		return "colhesiana";
	endif

	// Default: Aigla (most common post-Calamity, surface regions)
	return "aigla";
endfunction

function LoadNPCLoreContext(npc_type, npc := 0)
	// NEW: Support region-specific lore files
	var culture := "generic";

	if (npc)
		var region := GetNPCRegion(npc);
		culture := MapRegionToCulture(region);
	endif

	// Try loading region-specific lore file
	var lore_file := ":thor:config/lore_" + culture;
	var cfg := ReadConfigFile(lore_file);

	// Fallback to generic if region file missing
	if (!cfg)
		cfg := ReadConfigFile(":thor:config/lore_context");
	endif

	if (!cfg)
		return struct{ "Base" := "", "MaxChars" := 60, "Style" := "", "Culture" := "", "Tone" := "" };
	endif

	var context_key := npc_type + "Context";
	var elem := cfg[context_key];
	if (!elem)
		elem := cfg["MerchantContext"]; // Fallback to merchant
	endif

	// Get World section for culture/tone info
	var world_elem := cfg["World"];

	return struct{
		"Base" := GetConfigString(elem, "Base"),
		"MaxChars" := CInt(GetConfigString(elem, "MaxChars")),
		"Style" := GetConfigString(elem, "Style"),
		"Culture" := GetConfigString(world_elem, "Culture"),
		"Tone" := GetConfigString(world_elem, "Tone")
	};
endfunction

function GetRaceBehavior(race, npc := 0)
	var culture := "generic";

	if (npc)
		var region := GetNPCRegion(npc);
		culture := MapRegionToCulture(region);
	endif

	// Try region-specific lore file first
	var lore_file := ":thor:config/lore_" + culture;
	var cfg := ReadConfigFile(lore_file);

	// Fallback to generic
	if (!cfg)
		cfg := ReadConfigFile(":thor:config/lore_context");
	endif

	if (!cfg)
		return "";
	endif

	var elem := cfg["RacesBrief"];
	if (!elem)
		return "";
	endif

	var behavior := GetConfigString(elem, race);
	if (!behavior)
		return "";
	endif

	return " " + behavior + ".";
endfunction

// ============================================================================
// COMBAT SPEECH (TEMPLATE ONLY - NO AI)
// ============================================================================

function handle_combat_speech(npc_name, player_input, playerRaca, npcState := "healthy")
	// Only use config templates - NO AI calls
	var cfg := ReadConfigFile(":epicAI:config/npc_speech");
	if (!cfg)
		return handle_error_combat_AI_speech();
	endif

	var elem;
	foreach entry in (GetConfigStringKeys(cfg))
		var temp_elem := cfg[entry];
		if (GetConfigString(temp_elem, "Name") == npc_name)
			elem := temp_elem;
			break;
		endif
	endforeach

	if (!elem)
		return handle_error_combat_AI_speech();
	endif

	// Get responses for current state
	var responses := GetConfigStringArray(elem, npcState);
	if (!responses || !responses.size())
		responses := GetConfigStringArray(elem, "healthy");
	endif

	if (!responses || !responses.size())
		return handle_error_combat_AI_speech();
	endif

	// Return random template
	return responses[RandomInt(responses.size()) + 1];
endfunction

function handle_error_combat_AI_speech()
	var responses := array{
		"*Grunhe*",
		"*Rosna*",
		"*Ignora*",
		"*Olha fixamente*",
		"*Range*",
		"*Murmura*"
	};
	return responses[RandomInt(responses.size()) + 1];
endfunction

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

// Trim function removed - using built-in Trim() from basic.em module
