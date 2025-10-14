// ============================================================================
// Kingdom System - Wave Utilities
// Epic! Shard - Kingdom Management System v0.1.0
// ============================================================================

use uo;
use os;
use cfgfile;

include ":kingdom:constants";

// ============================================================================
// WAVE TEMPLATE LOADING
// ============================================================================

/*
 * WAVE_LoadTemplate(template_name)
 * Loads wave configuration by template name
 * Parameters:
 *   template_name - Name of wave template (e.g., "goblin_raid")
 * Returns: ConfigElem or 0 if not found
 */
function WAVE_LoadTemplate(template_name)
	if (!template_name || template_name == "")
		return 0;
	endif

	var cfg := ReadConfigFile(":kingdom:config/waves");
	if (!cfg)
		syslog("KINGDOM: Failed to load waves.cfg");
		return 0;
	endif

	// In POL cfg files, the element name is just the identifier, not "Type name"
	var template := FindConfigElem(cfg, template_name);
	if (!template)
		syslog("KINGDOM: Wave template not found: " + template_name);
		return 0;
	endif

	return template;
endfunction

/*
 * WAVE_GetTemplateNames()
 * Returns array of all available wave template names
 * Returns: Array of template names
 */
function WAVE_GetTemplateNames()
	var cfg := ReadConfigFile(":kingdom:config/waves");
	if (!cfg)
		return array{};
	endif

	var templates := array{};
	var keys := GetConfigStringKeys(cfg);

	foreach key in (keys)
		// Keys are just the template names (e.g., "goblin_raid")
		// No need to parse "WaveTemplate" prefix
		templates.append(key);
		sleepms(2);
	endforeach

	return templates;
endfunction

// ============================================================================
// WAVE INFORMATION RETRIEVAL
// ============================================================================

/*
 * WAVE_GetWaveInfo(template, wave_number)
 * Returns information about a specific wave
 * Parameters:
 *   template - ConfigElem from WAVE_LoadTemplate()
 *   wave_number - Wave number (1-3)
 * Returns: Struct with npc_template, count, delay or 0 on error
 */
function WAVE_GetWaveInfo(template, wave_number)
	if (!template || wave_number < 1 || wave_number > 3)
		return 0;
	endif

	var wave_info := struct{};

	case (wave_number)
		1:
			wave_info.+npc_template := GetConfigString(template, "Wave1Template");
			wave_info.+count := GetConfigInt(template, "Wave1Count");
			wave_info.+delay := GetConfigInt(template, "Wave1Delay");
		2:
			wave_info.+npc_template := GetConfigString(template, "Wave2Template");
			wave_info.+count := GetConfigInt(template, "Wave2Count");
			wave_info.+delay := GetConfigInt(template, "Wave2Delay");
		3:
			wave_info.+npc_template := GetConfigString(template, "Wave3Template");
			wave_info.+count := GetConfigInt(template, "Wave3Count");
			wave_info.+delay := GetConfigInt(template, "Wave3Delay");
		default:
			return 0;
	endcase

	// Default delay if not specified
	if (!wave_info.delay || wave_info.delay == 0)
		wave_info.delay := 2;
	endif

	return wave_info;
endfunction

/*
 * WAVE_GetBossInfo(template)
 * Returns information about the boss
 * Parameters:
 *   template - ConfigElem from WAVE_LoadTemplate()
 * Returns: Struct with npc_template, phrase, sound or 0 on error
 */
function WAVE_GetBossInfo(template)
	if (!template)
		return 0;
	endif

	var boss_info := struct{};
	boss_info.+npc_template := GetConfigString(template, "BossTemplate");
	boss_info.+phrase := GetConfigString(template, "BossPhrase");
	boss_info.+sound := GetConfigInt(template, "BossSound");
	boss_info.+victory_reward := GetConfigString(template, "VictoryReward");

	if (!boss_info.npc_template || boss_info.npc_template == "")
		return 0;
	endif

	return boss_info;
endfunction

/*
 * WAVE_GetDescription(template)
 * Returns the description of a wave template
 * Parameters:
 *   template - ConfigElem from WAVE_LoadTemplate()
 * Returns: Description string or ""
 */
function WAVE_GetDescription(template)
	if (!template)
		return "";
	endif

	return GetConfigString(template, "Description");
endfunction

// ============================================================================
// SPAWN POINT CALCULATION
// ============================================================================

/*
 * WAVE_CalculateSpawnPoints(center_x, center_y, count)
 * Calculates spawn points around a center location
 * Parameters:
 *   center_x, center_y - Center coordinates
 *   count - Number of spawn points to generate
 * Returns: Array of structs with x, y coordinates
 */
function WAVE_CalculateSpawnPoints(center_x, center_y, count)
	if (count <= 0)
		return array{};
	endif

	var spawn_points := array{};
	var radius := 10; // Base radius for spawning

	// If many NPCs, increase radius
	if (count > 5)
		radius := 15;
	endif
	if (count > 10)
		radius := 20;
	endif

	// Generate points in a circle around center
	var angle_increment := 360.0 / count;
	var i := 0;
	for (i := 0; i < count; i := i + 1)
		var angle := i * angle_increment;
		var radian := angle * 3.14159 / 180.0;

		var offset_x := CInt(radius * cos(radian));
		var offset_y := CInt(radius * sin(radian));

		var spawn_point := struct{};
		spawn_point.+x := center_x + offset_x;
		spawn_point.+y := center_y + offset_y;

		spawn_points.append(spawn_point);
		sleepms(2);
	endfor

	return spawn_points;
endfunction

/*
 * WAVE_GetRandomSpawnPoint(center_x, center_y, radius)
 * Returns a random spawn point within radius of center
 * Parameters:
 *   center_x, center_y - Center coordinates
 *   radius - Maximum distance from center
 * Returns: Struct with x, y coordinates
 */
function WAVE_GetRandomSpawnPoint(center_x, center_y, radius)
	var angle := RandomInt(360);
	var distance := RandomInt(radius) + 1;
	var radian := angle * 3.14159 / 180.0;

	var offset_x := CInt(distance * cos(radian));
	var offset_y := CInt(distance * sin(radian));

	var spawn_point := struct{};
	spawn_point.+x := center_x + offset_x;
	spawn_point.+y := center_y + offset_y;

	return spawn_point;
endfunction

// ============================================================================
// VALIDATION
// ============================================================================

/*
 * WAVE_ValidateTemplate(template_name)
 * Validates that a wave template is properly configured
 * Parameters:
 *   template_name - Name of wave template
 * Returns: 1 if valid, 0 if invalid
 */
function WAVE_ValidateTemplate(template_name)
	var template := WAVE_LoadTemplate(template_name);
	if (!template)
		return 0;
	endif

	// Check wave 1
	var wave1 := WAVE_GetWaveInfo(template, 1);
	if (!wave1 || !wave1.npc_template || wave1.count <= 0)
		syslog("KINGDOM: Invalid wave 1 in template: " + template_name);
		return 0;
	endif

	// Check wave 2
	var wave2 := WAVE_GetWaveInfo(template, 2);
	if (!wave2 || !wave2.npc_template || wave2.count <= 0)
		syslog("KINGDOM: Invalid wave 2 in template: " + template_name);
		return 0;
	endif

	// Check wave 3
	var wave3 := WAVE_GetWaveInfo(template, 3);
	if (!wave3 || !wave3.npc_template || wave3.count <= 0)
		syslog("KINGDOM: Invalid wave 3 in template: " + template_name);
		return 0;
	endif

	// Check boss
	var boss := WAVE_GetBossInfo(template);
	if (!boss || !boss.npc_template)
		syslog("KINGDOM: Invalid boss in template: " + template_name);
		return 0;
	endif

	return 1;
endfunction