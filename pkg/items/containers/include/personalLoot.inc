/*
 * Personal-loot rules for true_boss corpses.
 *
 * Lifecycle: 0..LOCKOUT seconds nobody can loot. After lockout, every item
 * has a loot_owner stamped on it; only that owner can take it. Items remain
 * locked to their owner until POL's natural corpse decay removes the corpse
 * — there is no FFA fallback.
 *
 * Items added later without a loot_owner (skinning hides, late deathscript
 * drops) are unrestricted at any time, matching today's behavior.
 *
 * Per-viewer visibility is enforced by the 0x3C packet hook in
 * pkg/packetHooks/personalLootView.
 */
use uo;
use util;

const BOSS_LOOT_LOCKOUT := 15;
const BOSS_LOOT_NEAR    := 32;

// Snapshots nearby online players (cmdlevel == 0) and arms the corpse for
// personal-loot enforcement. Caller decides when to invoke (typically in
// npcdeath.src right after sync loot is placed).
// Returns 1 on activation, 0 if no participants were nearby (corpse stays
// unrestricted — same as a non-boss corpse).
function PersonalLoot_Arm(corpse)
	if (!corpse || corpse == error)
		return 0;
	endif

	var participants := array{};
	foreach mob in (ListMobilesInLineOfSight(corpse, BOSS_LOOT_NEAR))
		// Players have no npctemplate; NPCs (incl. tamed pets, summons) do.
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

	// Fisher-Yates shuffle so unique-item round-robin doesn't always start
	// at the same player across kills.
	var i := participants.size();
	while (i > 1)
		var j := RandomInt(i) + 1;
		var tmp := participants[i];
		participants[i] := participants[j];
		participants[j] := tmp;
		i := i - 1;
	endwhile

	SetObjProperty(corpse, "loot_unlock_at",      ReadGameClock() + BOSS_LOOT_LOCKOUT);
	SetObjProperty(corpse, "loot_participants",   participants);
	SetObjProperty(corpse, "loot_assign_counter", 0);
	return 1;
endfunction

// Idempotent. Replicates each unowned item once per participant: the
// original is kept for participants[1], clones are created for the rest.
// Each copy gets its own loot_owner. Items flagged itemunico are NOT
// replicated (unique by design); they're round-robin assigned instead.
// Safe to call multiple times (sync path post-HandleEpicBossLoot, async
// path at end of Loot_Generate).
function PersonalLoot_Stamp(corpse)
	if (!corpse || corpse == error)
		return 0;
	endif

	var participants := GetObjProperty(corpse, "loot_participants");
	if (participants == error || !participants || participants.size() == 0)
		return 0;
	endif

	var n := participants.size();
	var counter := CInt(GetObjProperty(corpse, "loot_assign_counter"));

	// Two-pass: collect first to avoid iterating while we add clones.
	var pending := array{};
	foreach scan_item in (EnumerateItemsInContainer(corpse))
		if (GetObjProperty(scan_item, "loot_owner") == error)
			pending.Append(scan_item);
		endif
	endforeach

	foreach loot in pending
		if (GetObjProperty(loot, "itemunico") == 1)
			SetObjProperty(loot, "loot_owner", participants[(counter % n) + 1]);
			counter := counter + 1;
			continue;
		endif

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

	SetObjProperty(corpse, "loot_assign_counter", counter);
	return 1;
endfunction

// True while the corpse has been armed for personal loot. Items remain
// locked to their owner until the corpse decays naturally.
function PersonalLoot_IsActive(corpse)
	if (!corpse || corpse == error)
		return 0;
	endif
	if (GetObjProperty(corpse, "true_boss") != 1)
		return 0;
	endif
	return (GetObjProperty(corpse, "loot_unlock_at") != error);
endfunction
