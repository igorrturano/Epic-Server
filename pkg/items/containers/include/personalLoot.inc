/*
 * Personal-loot rules for true_boss corpses.
 *
 * Lifecycle: 0..BOSS_LOOT_LOCKOUT seconds nobody can loot. After the
 * lockout, items the GM authored in the epicboss "Add Loot" gump
 * (HandleEpicBossLoot tags them with from_boss_loot := 1) are cloned
 * once per nearby participant and stamped with each player's serial as
 * loot_owner. Each player only sees and can take their own copy.
 *
 * Everything else in the corpse — the boss's equipped gear, hair, beard,
 * clothes, NPC backpack loot, etc. — is left as a single untagged copy,
 * visible to all. First to grab wins.
 *
 * Owned items stay locked to their owner until POL's natural corpse
 * decay removes the corpse — there is no FFA fallback.
 *
 * Per-viewer visibility is enforced by the 0x3C packet hook in
 * pkg/packetHooks/personalLootView.
 */
use uo;

const BOSS_LOOT_LOCKOUT := 15;
const BOSS_LOOT_NEAR    := 32;

// Snapshots nearby players (cmdlevel == 0) and arms the corpse. Returns 1
// on activation, 0 if no participants were nearby.
function PersonalLoot_Arm(corpse)
	if (!corpse || corpse == error)
		return 0;
	endif

	var participants := array{};
	foreach mob in (ListMobilesInLineOfSight(corpse, BOSS_LOOT_NEAR))
		// Players have no npctemplate; NPCs (pets, summons) do.
		if (mob.npctemplate)
			continue;
		endif
		if (mob.cmdlevel == 0)
			participants.Append(mob.serial);
		endif
	endforeach

	if (participants.size() == 0)
		return 0;
	endif

	SetObjProperty(corpse, "loot_unlock_at",    ReadGameClock() + BOSS_LOOT_LOCKOUT);
	SetObjProperty(corpse, "loot_participants", participants);
	return 1;
endfunction

// Idempotent. For each unowned item flagged from_boss_loot, keep the
// original for participants[1] and clone it for each remaining
// participant. Items without the flag (equipment, hair, beard, etc.) are
// left untouched — single copy, FFA.
function PersonalLoot_Stamp(corpse)
	if (!corpse || corpse == error)
		return 0;
	endif

	var participants := GetObjProperty(corpse, "loot_participants");
	if (participants == error || !participants || participants.size() == 0)
		return 0;
	endif

	var n := participants.size();

	// Two-pass: collect first so we don't iterate while adding clones.
	var pending := array{};
	foreach scan_item in (EnumerateItemsInContainer(corpse))
		if (GetObjProperty(scan_item, "loot_owner") != error)
			continue;
		endif
		if (GetObjProperty(scan_item, "from_boss_loot") != 1)
			continue;
		endif
		pending.Append(scan_item);
	endforeach

	foreach loot in pending
		SetObjProperty(loot, "loot_owner", participants[1]);

		var p_idx := 2;
		while (p_idx <= n)
			var clone := CreateItemCopyAtLocation(corpse.x, corpse.y, corpse.z,
												  loot, corpse.realm);
			if (clone && clone != error)
				MoveItemToContainer(clone, corpse);
				SetObjProperty(clone, "loot_owner", participants[p_idx]);
			endif
			p_idx := p_idx + 1;
		endwhile
	endforeach

	return 1;
endfunction

// True once the corpse has been armed. Stays true until POL removes the
// corpse via natural decay.
function PersonalLoot_IsActive(corpse)
	if (!corpse || corpse == error)
		return 0;
	endif
	if (GetObjProperty(corpse, "true_boss") != 1)
		return 0;
	endif
	return (GetObjProperty(corpse, "loot_unlock_at") != error);
endfunction
