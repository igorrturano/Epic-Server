// ============================================================================
// Kingdom System - Conquest Management
// Epic! Shard - Kingdom Management System v0.1.0
// ============================================================================

use uo;
use os;
use npc;
use vitals;

include ":kingdom:constants";
include ":kingdom:datafile";
include ":kingdom:territory";
include ":kingdom:waves";

// ============================================================================
// CONQUEST INITIALIZATION
// ============================================================================

/*
 * CNQST_StartConquest(war_totem)
 * Initializes conquest on a war totem
 * Parameters:
 *   war_totem - War totem item
 * Returns: 1 on success, 0 on failure
 */
function CNQST_StartConquest(war_totem)
	if (!war_totem)
		return 0;
	endif

	var territory_name := war_totem.GetProp("territory_name");
	if (!territory_name || territory_name == "")
		syslog("KINGDOM: StartConquest failed - no territory_name on totem");
		return 0;
	endif

	var wave_template := war_totem.GetProp("wave_template");
	if (!wave_template || wave_template == "")
		syslog("KINGDOM: StartConquest failed - no wave_template configured");
		return 0;
	endif

	// Validate wave template
	if (!WAVE_ValidateTemplate(wave_template))
		syslog("KINGDOM: StartConquest failed - invalid wave template: " + wave_template);
		return 0;
	endif

	// Check if already active
	if (war_totem.GetProp("conquest_active"))
		syslog("KINGDOM: StartConquest failed - conquest already active");
		return 0;
	endif

	// Initialize conquest properties
	war_totem.SetProp("conquest_active", 1);
	war_totem.SetProp("current_wave", 1);
	war_totem.SetProp("total_kills", 0);
	war_totem.SetProp("started_time", POLCore().systime);

	// Load wave 1 info to set required kills
	var template := WAVE_LoadTemplate(wave_template);
	var wave1 := WAVE_GetWaveInfo(template, 1);
	war_totem.SetProp("required_kills", wave1.count);

	// Announce conquest start
	var territory := KD_GetTerritory(territory_name);
	if (territory)
		BroadcastNearTotem(war_totem, "A conquest has begun at " + territory.name + "!", 0x21);
	endif

	// Spawn wave 1
	CNQST_SpawnWave(war_totem, 1);

	syslog("KINGDOM: Conquest started on territory: " + territory_name + " with template: " + wave_template);

	return 1;
endfunction

/*
 * CNQST_CancelConquest(war_totem)
 * Aborts an active conquest
 * Parameters:
 *   war_totem - War totem item
 * Returns: 1 on success
 */
function CNQST_CancelConquest(war_totem)
	if (!war_totem)
		return 0;
	endif

	// Kill all conquest NPCs
	var conquest_npcs := CNQST_GetConquestNPCs(war_totem);
	foreach npc in (conquest_npcs)
		if (npc && npc.connected)
			ApplyRawDamage(npc, npc.hp);
		endif
		sleepms(2);
	endforeach

	// Reset conquest properties
	war_totem.EraseProp("conquest_active");
	war_totem.EraseProp("current_wave");
	war_totem.EraseProp("total_kills");
	war_totem.EraseProp("required_kills");
	war_totem.EraseProp("started_time");
	war_totem.EraseProp("boss_serial");

	BroadcastNearTotem(war_totem, "The conquest has been cancelled.", 0x26);

	syslog("KINGDOM: Conquest cancelled on totem: " + war_totem.serial);

	return 1;
endfunction

// ============================================================================
// WAVE MANAGEMENT
// ============================================================================

/*
 * CNQST_SpawnWave(war_totem, wave_number)
 * Spawns NPCs for a wave
 * Parameters:
 *   war_totem - War totem item
 *   wave_number - Wave number (1-3)
 * Returns: Number of NPCs spawned
 */
function CNQST_SpawnWave(war_totem, wave_number)
	if (!war_totem || wave_number < 1 || wave_number > 3)
		return 0;
	endif

	var wave_template := war_totem.GetProp("wave_template");
	if (!wave_template || wave_template == "")
		return 0;
	endif

	var template := WAVE_LoadTemplate(wave_template);
	if (!template)
		return 0;
	endif

	var wave_info := WAVE_GetWaveInfo(template, wave_number);
	if (!wave_info)
		return 0;
	endif

	// Calculate spawn points around totem
	var spawn_points := WAVE_CalculateSpawnPoints(war_totem.x, war_totem.y, wave_info.count);

	var spawned_count := 0;

	foreach spawn_point in (spawn_points)
		// Create NPC
		var npc := CreateNpcFromTemplate(wave_info.npc_template, spawn_point.x, spawn_point.y, war_totem.z, 0, war_totem.realm);

		if (npc)
			// Link NPC to totem for tracking
			npc.SetProp("conquest_totem", war_totem.serial);
			npc.SetProp("conquest_wave", wave_number);

			spawned_count := spawned_count + 1;
		endif

		sleepms(wave_info.delay);
	endforeach

	// Announce wave
	var territory_name := war_totem.GetProp("territory_name");
	var territory := KD_GetTerritory(territory_name);
	if (territory)
		BroadcastNearTotem(war_totem, "Wave " + wave_number + " has arrived at " + territory.name + "!", 0x21);
	endif

	syslog("KINGDOM: Spawned wave " + wave_number + " - " + spawned_count + " NPCs");

	return spawned_count;
endfunction

/*
 * CNQST_OnNPCDeath(npc)
 * Called when an NPC in conquest dies
 * Parameters:
 *   npc - NPC mobile that died
 * Returns: 1 if wave complete, 0 otherwise
 */
function CNQST_OnNPCDeath(npc)
	if (!npc)
		return 0;
	endif

	var totem_serial := npc.GetProp("conquest_totem");
	if (!totem_serial)
		return 0; // Not a conquest NPC
	endif

	var war_totem := SystemFindObjectBySerial(totem_serial);
	if (!war_totem)
		return 0;
	endif

	if (!war_totem.GetProp("conquest_active"))
		return 0;
	endif

	// Increment kill counter
	var total_kills := CInt(war_totem.GetProp("total_kills")) + 1;
	war_totem.SetProp("total_kills", total_kills);

	var required_kills := CInt(war_totem.GetProp("required_kills"));

	// Check if wave complete
	if (total_kills >= required_kills)
		return CNQST_AdvanceWave(war_totem);
	endif

	return 0;
endfunction

/*
 * CNQST_AdvanceWave(war_totem)
 * Advances to next wave or spawns boss
 * Parameters:
 *   war_totem - War totem item
 * Returns: 1 on success
 */
function CNQST_AdvanceWave(war_totem)
	if (!war_totem)
		return 0;
	endif

	var current_wave := CInt(war_totem.GetProp("current_wave"));

	if (current_wave < 3)
		// Advance to next wave
		var next_wave := current_wave + 1;
		war_totem.SetProp("current_wave", next_wave);
		war_totem.SetProp("total_kills", 0);

		// Load next wave info
		var wave_template := war_totem.GetProp("wave_template");
		var template := WAVE_LoadTemplate(wave_template);
		var wave_info := WAVE_GetWaveInfo(template, next_wave);
		war_totem.SetProp("required_kills", wave_info.count);

		// Spawn next wave
		CNQST_SpawnWave(war_totem, next_wave);

		BroadcastNearTotem(war_totem, "Wave " + current_wave + " complete! Prepare for wave " + next_wave + "!", 0x58);

		syslog("KINGDOM: Conquest advanced to wave " + next_wave);

	elseif (current_wave == 3)
		// Spawn boss
		CNQST_SpawnBoss(war_totem);
	endif

	return 1;
endfunction

// ============================================================================
// BOSS MANAGEMENT
// ============================================================================

/*
 * CNQST_SpawnBoss(war_totem)
 * Spawns final boss
 * Parameters:
 *   war_totem - War totem item
 * Returns: Boss mobile or 0 on failure
 */
function CNQST_SpawnBoss(war_totem)
	if (!war_totem)
		return 0;
	endif

	var wave_template := war_totem.GetProp("wave_template");
	if (!wave_template || wave_template == "")
		return 0;
	endif

	var template := WAVE_LoadTemplate(wave_template);
	if (!template)
		return 0;
	endif

	var boss_info := WAVE_GetBossInfo(template);
	if (!boss_info)
		return 0;
	endif

	// Spawn boss at totem location
	var boss := CreateNpcFromTemplate(boss_info.npc_template, war_totem.x, war_totem.y, war_totem.z, 0, war_totem.realm);

	if (!boss)
		syslog("KINGDOM: Failed to spawn boss: " + boss_info.npc_template);
		return 0;
	endif

	// Link boss to totem
	boss.SetProp("conquest_totem", war_totem.serial);
	boss.SetProp("conquest_boss", 1);
	war_totem.SetProp("boss_serial", boss.serial);
	war_totem.SetProp("current_wave", 4); // Mark as boss wave

	// Boss announces
	if (boss_info.phrase && boss_info.phrase != "")
		boss.Say(boss_info.phrase);
	endif

	if (boss_info.sound && boss_info.sound > 0)
		PlaySoundEffect(boss, boss_info.sound);
	endif

	BroadcastNearTotem(war_totem, "The champion has arrived! Defeat it to claim victory!", 0x25);

	syslog("KINGDOM: Boss spawned for conquest: " + boss_info.npc_template);

	return boss;
endfunction

/*
 * CNQST_OnBossDeath(boss)
 * Called when boss is killed - triggers victory
 * Parameters:
 *   boss - Boss mobile
 * Returns: 1 on success
 */
function CNQST_OnBossDeath(boss)
	if (!boss)
		return 0;
	endif

	var totem_serial := boss.GetProp("conquest_totem");
	if (!totem_serial)
		return 0;
	endif

	var war_totem := SystemFindObjectBySerial(totem_serial);
	if (!war_totem)
		return 0;
	endif

	// Trigger victory
	CNQST_Victory(war_totem);

	return 1;
endfunction

/*
 * CNQST_Victory(war_totem)
 * Handles conquest victory - transforms totem
 * Parameters:
 *   war_totem - War totem item
 * Returns: Kingdom totem serial or 0 on failure
 */
function CNQST_Victory(war_totem)
	if (!war_totem)
		return 0;
	endif

	var territory_name := war_totem.GetProp("territory_name");
	if (!territory_name || territory_name == "")
		return 0;
	endif

	// Announce victory
	var territory := KD_GetTerritory(territory_name);
	if (territory)
		BroadcastNearTotem(war_totem, "VICTORY! " + territory.name + " has been conquered! A faction leader may now claim this territory.", 0x44);
	endif

	// Transform war totem to kingdom totem
	var kingdom_totem := CNQST_TransformToKingdomTotem(war_totem);

	syslog("KINGDOM: Conquest victory on territory: " + territory_name);

	return kingdom_totem;
endfunction

/*
 * CNQST_TransformToKingdomTotem(war_totem)
 * Converts war totem to claimable kingdom totem
 * Parameters:
 *   war_totem - War totem item
 * Returns: Kingdom totem serial or 0 on failure
 */
function CNQST_TransformToKingdomTotem(war_totem)
	if (!war_totem)
		return 0;
	endif

	var territory_name := war_totem.GetProp("territory_name");

	// Create kingdom totem at same location
	var kingdom_totem := CreateItemAtLocation(war_totem.x, war_totem.y, war_totem.z, 0x1183, 1, war_totem.realm);

	if (!kingdom_totem)
		syslog("KINGDOM: Failed to create kingdom totem");
		return 0;
	endif

	// Set properties
	kingdom_totem.SetProp("territory_name", territory_name);
	kingdom_totem.SetProp("claimable", 1);
	kingdom_totem.SetProp("victory_time", POLCore().systime);
	kingdom_totem.name := "Kingdom Totem [" + territory_name + "]";
	kingdom_totem.color := 0x47; // Gold color

	// Destroy old war totem
	DestroyItem(war_totem);

	return kingdom_totem.serial;
endfunction

// ============================================================================
// STATUS AND QUERIES
// ============================================================================

/*
 * CNQST_GetConquestStatus(war_totem)
 * Returns status struct for conquest
 * Parameters:
 *   war_totem - War totem item
 * Returns: Struct with conquest status or 0 if not active
 */
function CNQST_GetConquestStatus(war_totem)
	if (!war_totem)
		return 0;
	endif

	if (!war_totem.GetProp("conquest_active"))
		return 0;
	endif

	var status := struct{};
	status.+territory_name := war_totem.GetProp("territory_name");
	status.+wave_template := war_totem.GetProp("wave_template");
	status.+current_wave := CInt(war_totem.GetProp("current_wave"));
	status.+total_kills := CInt(war_totem.GetProp("total_kills"));
	status.+required_kills := CInt(war_totem.GetProp("required_kills"));
	status.+started_time := CInt(war_totem.GetProp("started_time"));
	status.+boss_serial := CInt(war_totem.GetProp("boss_serial"));
	status.+active := 1;

	return status;
endfunction

/*
 * CNQST_GetConquestNPCs(war_totem)
 * Returns array of all NPCs linked to this conquest
 * Parameters:
 *   war_totem - War totem item
 * Returns: Array of NPC mobiles
 */
function CNQST_GetConquestNPCs(war_totem)
	if (!war_totem)
		return array{};
	endif

	var conquest_npcs := array{};
	var totem_serial := war_totem.serial;

	// Search for NPCs near totem
	var npcs := ListMobilesNearLocation(war_totem.x, war_totem.y, war_totem.z, 50, war_totem.realm);

	foreach npc in (npcs)
		if (npc.npctemplate && npc.GetProp("conquest_totem") == totem_serial)
			conquest_npcs.append(npc);
		endif
		sleepms(2);
	endforeach

	return conquest_npcs;
endfunction

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/*
 * BroadcastNearTotem(war_totem, message, color)
 * Broadcasts message to players near totem
 * Parameters:
 *   war_totem - War totem item
 *   message - Message to broadcast
 *   color - Text color
 * Returns: None
 */
function BroadcastNearTotem(war_totem, message, color := 0x21)
	if (!war_totem)
		return;
	endif

	var players := ListMobilesNearLocation(war_totem.x, war_totem.y, war_totem.z, 30, war_totem.realm);

	foreach player in (players)
		if (player.connected && !player.npctemplate)
			SendSysMessage(player, message, color);
		endif
		sleepms(2);
	endforeach
endfunction