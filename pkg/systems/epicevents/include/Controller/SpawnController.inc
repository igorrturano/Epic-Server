use uo;
use os;
use polsys;

include ":epicevents:Model/EventConfig";
include ":epicevents:Model/CreatureSpawnInfo";
include "include/epicNPCUtils";

// Default NPC lifetime in minutes (auto-despawn after this time)
const DEFAULT_NPC_LIFETIME_MINUTES := 30;

function SpawnAllCreatures(item, config)
	if (!item || !config)
		return 0;
	endif

	var spawned_count := 0;
	var npc_lifetime := GetNpcLifetime(config);

	foreach creature in (config.creatures)
		if (!creature.IsValid())
			continue;
		endif

		spawned_count += SpawnCreatureType(item, creature, config.color, config.emote, npc_lifetime);
	endforeach

	return spawned_count;
endfunction

function SpawnCreatureType(item, spawnInfo, color, emote, npc_lifetime)
	if (!item || !spawnInfo)
		return 0;
	endif

	var amount := spawnInfo.GetAmount();
	var spawned := 0;

	for i := 1 to amount
		var npc := CreateNpc(spawnInfo.template, item.x, item.y, item.z, item.realm);

		if (npc && !npc.errortext)
			ApplyCreatureCustomization(npc, spawnInfo, color, emote);
			ScheduleNpcDespawn(npc, npc_lifetime);
			spawned += 1;
		endif
		sleepms(20);
	endfor

	return spawned;
endfunction

function ApplyCreatureCustomization(npc, spawnInfo, color, emote)
	if (!npc || !spawnInfo)
		return 0;
	endif

	// Apply tier customization
	customizeNpc(npc, spawnInfo.GetTier());

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

	// Show emote
	if (emote && emote != "")
		PrintTextAbove(npc, emote);
	endif

	return 1;
endfunction

function CreateNpc(template, x, y, z, realm)
	// If template already has package prefix, use directly
	if (Find(template, ":", 1))
		return CreateNpcFromTemplate(Trim(template), x, y, z, 0, realm);
	endif

	// Only valid package is :epicAI:
	return CreateNpcFromTemplate(":epicAI:" + Trim(template), x, y, z, 0, realm);
endfunction

function GetNpcLifetime(config)
	// Use max_delay as lifetime (NPCs live until next possible trigger)
	var lifetime := CInt(config.max_delay);
	if (lifetime < 1)
		lifetime := DEFAULT_NPC_LIFETIME_MINUTES;
	endif
	return lifetime;
endfunction

function ScheduleNpcDespawn(npc, lifetime_minutes)
	if (!npc)
		return 0;
	endif

	// Set despawn time property for debugging/inspection
	var despawn_time := polcore().systime + (lifetime_minutes * 60);
	SetObjProperty(npc, "event_npc_despawn_time", despawn_time);

	// Start despawn script
	start_script(":epicevents:despawnNpc", array{npc, lifetime_minutes});
	return 1;
endfunction
