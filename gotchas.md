# Gotchas

Strict rules distilled from bugs we've actually hit. Read before touching the
referenced systems.

## SpellController resync: never re-apply additive opts unconditionally

**Bug:** Força do Touro (and every str/dex/int spell) stacked on logon —
`100 STR + 20 buff` read `140` after a logout/login. Repeated relogs kept
adding `+20` each time.

**Cause:** `SC_ResyncOnLogin` re-applied the stat `TemporaryMod` on every login
via the additive `AP_ModifyStatMod`. The bonus lives in the mobile's runtime
layer, which **survives a same-session reconnect** (the offline mobile stays
resident) but is **wiped on a full server restart**. `SpellOpts` is persistent
and can't tell the two apart — so an unconditional re-apply doubled the still-live
bonus on reconnect.

**Rule:** Any `_SC_ApplyOpts` side effect that is *additive* AND lives in
volatile runtime state must be gated on a lifecycle-matched signal before
resync re-applies it. Mirror the `#buff_resist` pattern: a `#`-prefixed marker
shares the bonus's runtime lifecycle (present ⇒ reconnect ⇒ skip; absent ⇒
restart ⇒ rebuild). Stat mods use `#sc_statmods_live` for exactly this.
See `docs/spells-pipeline.md` pitfall #4 — it warned about this; the str/dex/int
re-apply violated it.

**Don't:** "fix" the restart symptom (icon shows +N, stat is 0) by re-applying
on every login. **Don't** reverse-then-reapply on resync either — on a restart
the layer is already wiped, so the reverse subtracts from 0 and strands a
negative mod.

## Sourceless DealDamage double-sends the damage number to a player target

**Bug:** A boss combat-event "Dmg" action (e.g. 50 Cold to All Opponents) showed
the floating damage number **twice** ("50 50") on the player, even though HP only
dropped once.

**Cause:** `ExecuteDamageAction` called `DealDamage(targ, damage, dmgType)` with
no `source`. `DealDamage` does `attackerRef := source ? source : defender`, so
attacker collapsed to the **target player**. `ApplyCombatDamage` then calls
`DisplayDamage(defender, n, attacker)`, and `DisplayDamage`
(`scripts/include/packets.inc`) sends the damage packet to BOTH `source` and
`damaged` when each has an `acctname`. With attacker == damaged == the same
player, that player received the packet twice. Normal melee never doubled
because it always has a distinct attacker.

**Rule:** Always pass a real `source` to `DealDamage`/`ApplyFieldDamage` for
NPC/environmental damage. A sourceless call against a player target double-shows
the number AND mis-attributes aggro/kill-credit (player logged as its own
attacker). NPCs have no `acctname`, so the bug is invisible until a player is the
source-or-target.

## ListHostiles / opponent==npc is the REVERSE combat link — wrong for NPC->player AoE

**Bug:** A boss combat-event targeting "All Opponents" resolved **0 targets**
even mid-fight, so the action silently no-opped. "Opponent" (single) worked.

**Cause:** `ListHostiles(npc)` returns who is *attacking npc*, and
`mobile.opponent == npc` is the same reverse relationship. A boss's combat link
is one-directional: `npc.opponent` (boss->player) is set, but nothing points
back at the boss, so both reverse-direction lookups return empty.

**Rule:** To enumerate an NPC's AoE victims (players + their pets), use the house
idiom from every NPC AoE (HammerSlam/WingBuffet/ScreechAttack): proximity scan
`ListMobilesNearLocation(npc.x, npc.y, npc.z, range, npc.realm)`, skip self/dead,
skip wild NPCs (`mobile.npctemplate && !mobile.master`). Do NOT rely on
`ListHostiles` or `opponent ==` for NPC-sourced targeting. `GetAllOpponents` in
`epicBoss.inc` is the canonical helper.

## start_script-per-entity loops need a pid guard or re-saves stack them

**Bug:** A boss combat-event applied its effect N times per cycle. Each *execution*
was correct (verified single `DealDamage`); the event was simply being run by
multiple copies of `combatEventsControl` on the same boss. `types=2`->4 numbers,
`types=1`->2 numbers = exactly 2 live script instances.

**Cause:** Every `CreateEpicBoss` (including re-save/respawn) called
`start_script(combatEventsControl, npc)` with no single-instance guard, so old
loops kept running and accumulated.

**Rule:** Any `start_script` that spawns a long-lived per-entity loop must guard
against duplicates: store `GetPid()` in an ObjProperty on the entity at start; a
newer instance that finds a still-live pid (`GetProcess(pid)` succeeds) exits
immediately. Idiom: see `combatEventsControl.src` (`combatEventsPid`) and
`npclifetime.src`. Note: an already-running stale loop won't read the new guard —
kill/respawn the entity (or restart) to clear pre-existing duplicates.
