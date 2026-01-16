use uo;
use os;
use polsys;

include ":epicevents:Model/EventConfig";
include ":epicevents:Model/CreatureSpawnInfo";
include "include/epicNPCUtils";

function SpawnAllCreatures(item, config)
	print("[EpicEvents] SpawnAllCreatures called");

	if (!item || !config)
		print("[EpicEvents] ERROR: Invalid item or config!");
		return 0;
	endif

	var spawned_count := 0;

	foreach creature in (config.creatures)
		if (!creature.IsValid())
			print("[EpicEvents] Skipping invalid creature");
			continue;
		endif

		print("[EpicEvents] Spawning - Template: " + creature.template + ", Amount: " + creature.amount + ", Tier: " + creature.tier);
		spawned_count := spawned_count + SpawnCreatureType(item, creature, config.color, config.emote);
		sleepms(2);
	endforeach

	print("[EpicEvents] Total spawned: " + spawned_count + " creatures");
	UpdateSpawnTime(item, config);

	return spawned_count;
endfunction

function SpawnCreatureType(item, spawnInfo, color, emote)
	if (!item || !spawnInfo)
		print("[EpicEvents] ERROR: Invalid item or spawnInfo in SpawnCreatureType");
		return 0;
	endif

	var configured_amount := spawnInfo.GetAmount();
	var alive_count := CountAliveNPCs(item, spawnInfo.template);
	var needed := configured_amount - alive_count;

	print("[EpicEvents] Template: " + spawnInfo.template + " - Configured: " + configured_amount + ", Alive: " + alive_count + ", Needed: " + needed);

	if (needed <= 0)
		print("[EpicEvents] All NPCs still alive, skipping spawn");
		return 0;
	endif

	var spawned := 0;
	var count := 0;

	for (count := 1; count <= needed; count := count + 1)
		var npc := CreateNpc(spawnInfo.template, item.x, item.y, item.z, item.realm);

		if (npc && !npc.errortext)
			print("[EpicEvents] Successfully created NPC #" + count + ": " + npc.name);
			// Tag this NPC with spawner info
			SetObjProperty(npc, "event_spawner_serial", item.serial);
			SetObjProperty(npc, "event_spawner_template", spawnInfo.template);
			ApplyCreatureCustomization(npc, spawnInfo, color, emote);
			spawned := spawned + 1;
		else
			print("[EpicEvents] ERROR creating NPC #" + count + ": " + npc.errortext);
		endif

		sleepms(2);
	endfor

	print("[EpicEvents] Spawned " + spawned + "/" + needed + " creatures of type: " + spawnInfo.template);
	return spawned;
endfunction

function ApplyCreatureCustomization(npc, spawnInfo, color, emote)
	if (!npc || !spawnInfo)
		return 0;
	endif

	// Apply tier customization
	var tier := spawnInfo.GetTier();
	customizeNpc(npc, tier);

	// Apply custom name
	if (spawnInfo.name && spawnInfo.name != "")
		SetName(npc, spawnInfo.name);
	endif

	// Apply color
	if (color && color != "")
		var color_val := CInt(color);
		if (color_val > 0)
			npc.color := color_val;
			npc.truecolor := color_val;
		endif
	endif

	// Set event properties
	SetObjProperty(npc, "GuardKill", 1);
	SetObjProperty(npc, "NoCorpse", 1);

	// Show emote
	if (emote && emote != "")
		PrintTextAbove(npc, emote);
	endif

	return 1;
endfunction

function CreateNpc(template, x, y, z, realm)
	var result;

	var paths := array{":epicAI:" + template, ":brainAI:" + template, ":merchants:" + template};

	if (SplitWords(template)[1] != ":")
		print("[EpicEvents] Trying template paths for: " + template);
		foreach path in paths
			print("[EpicEvents]   Trying: " + path);
			result := CreateNpcFromTemplate(Trim(path), x, y, z, 0, realm);
			if (result && !result.errortext)
				print("[EpicEvents]   SUCCESS with: " + path);
				return result;
			else
				print("[EpicEvents]   FAILED: " + result.errortext);
			endif
			sleepms(2);
		endforeach
	else
		print("[EpicEvents] Using direct template path: " + template);
		result := CreateNpcFromTemplate(Trim(template), x, y, z, 0, realm);
	endif

	return result;
endfunction

function CheckCanSpawn(item)
	if (!item)
		print("[EpicEvents] ERROR: Invalid item in CheckCanSpawn");
		return 0;
	endif

	var next_spawn_time := GetObjProperty(item, "next_spawn_time");
	var current_time := polcore().systime;

	if (!next_spawn_time)
		print("[EpicEvents] First spawn - no next_spawn_time set, allowing spawn");
		return 1;
	endif

	var time_remaining := next_spawn_time - current_time;
	print("[EpicEvents] Current time: " + current_time + ", Next spawn: " + next_spawn_time + ", Time remaining: " + time_remaining + " seconds");

	if (current_time >= next_spawn_time)
		print("[EpicEvents] Cooldown expired, allowing spawn");
		return 1;
	else
		print("[EpicEvents] Still on cooldown, " + (time_remaining / 60) + " minutes remaining");
		return 0;
	endif
endfunction

function CountAliveNPCs(item, template)
	if (!item || !template || template == "")
		return 0;
	endif

	var count := 0;
	var npcs := ListMobilesNearLocation(item.x, item.y, item.z, 30, item.realm);

	foreach npc in npcs
		if (npc.npctemplate)  // Is an NPC, not a player
			var spawner_serial := GetObjProperty(npc, "event_spawner_serial");
			var spawner_template := GetObjProperty(npc, "event_spawner_template");

			if (spawner_serial == item.serial && spawner_template == template)
				if (!npc.dead)
					count := count + 1;
				endif
			endif
		endif
	endforeach

	return count;
endfunction

function UpdateSpawnTime(item, config)
	if (!item || !config)
		print("[EpicEvents] ERROR: Invalid item or config in UpdateSpawnTime");
		return 0;
	endif

	// Calculate random delay between min and max
	var min_delay := CInt(config.min_delay);
	var max_delay := CInt(config.max_delay);

	// Validate delays
	if (!min_delay || min_delay < 1)
		print("[EpicEvents] ERROR: Invalid min_delay (" + config.min_delay + "), defaulting to 1");
		min_delay := 1;
	endif

	if (!max_delay || max_delay < min_delay)
		print("[EpicEvents] ERROR: Invalid max_delay (" + config.max_delay + "), defaulting to " + min_delay);
		max_delay := min_delay;
	endif

	var random_delay_minutes := RandomIntMinMax(min_delay, max_delay);
	print("[EpicEvents] Random delay calculated: " + random_delay_minutes + " minutes (range: " + min_delay + "-" + max_delay + ")");

	var delay_seconds := random_delay_minutes * 60;

	// Set next spawn time
	var next_spawn_time := polcore().systime + delay_seconds;
	SetObjProperty(item, "next_spawn_time", next_spawn_time);

	print("[EpicEvents] Next spawn set to " + random_delay_minutes + " minutes from now (timestamp: " + next_spawn_time + ")");

	return 1;
endfunction
