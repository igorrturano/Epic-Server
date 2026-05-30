# Epic Boss GM Tool — Combat & Death Event Tweaks

Date: 2026-05-29
Scope: three independent changes to the Epic Boss GM tool. Reference doc:
[docs/epicboss-gm-tool.md](../../epicboss-gm-tool.md).

---

## Feature 1 — Remove "Only when bloodied" (combat events)

The `onlybloodied` combat-event option is being retired. Remove every
touch-point; no migration required (stored values become inert).

Touch-points:
- [epicCombatEvents.inc:32](../../../pkg/systems/epicspawn/include/epicCombatEvents.inc#L32) — struct default `onlybloodied := 0` (remove the field).
- [epicCombatEvents.inc:86-87](../../../pkg/systems/epicspawn/include/epicCombatEvents.inc#L86-L87) — gump checkbox (id 3) + "Only when bloodied" label (remove both). Gump id 3 becomes free.
- [epicCombatEvents.inc:173](../../../pkg/systems/epicspawn/include/epicCombatEvents.inc#L173) — `combat_event.onlybloodied := input[3] ...` extraction (remove).
- [combatEventsControl.src:140-147](../../../pkg/systems/epicspawn/scripts/combatEventsControl.src#L140-L147) — runtime gate (remove the whole `if (event.onlybloodied)` block).
- [epicBoss.inc:366](../../../pkg/systems/epicspawn/include/epicBoss.inc#L366) — `clean_event.+onlybloodied` copy inside `NormalizeCombatEvent` (remove).

The Page-3 list summary never displayed bloodied, so no change there.

Backward compat: bosses already spawned carry `onlybloodied` in their stored
`CombatEvents`. After this change nothing reads it; it is dead weight, harmless.

---

## Feature 2 — Edit a combat event from Page 3

Today each combat-event row on Page 3 has only a delete button (id `44+i`,
range 45–54). Add an **edit** button per row.

- Button range: **160–169** (verified unused on Page 3). Edit button for event
  `i` = `159 + i`.
- New handler `HandleEditCombatEvent(who, byref form_data, idx)` in
  epicCombatEvents.inc, placed beside `HandleAddCombatEvent`. It:
  1. Guards `idx` against `form_data.combatevents.size()`.
  2. Calls the existing in-place editor `ShowCombatEventsList(who, byref event)`
     on the existing event struct (it edits by reference, returns 1 saved / 0
     cancelled).
  3. On save, re-runs the same `actions → steps` mirror that
     `HandleAddCombatEvent` does ([epicCombatEvents.inc:54-58](../../../pkg/systems/epicspawn/include/epicCombatEvents.inc#L54-L58)),
     so an edited event persists its steps the same way a new one does.
  4. On cancel, leaves the event unchanged.

  Keeping the steps-mirror logic shared with Add avoids the drift class of bug
  fixed earlier this session (the normalizer refactor).

- Page-3 rendering ([epicboss.src:1004-1014](../../../pkg/systems/epicspawn/commands/gm/epicboss.src#L1004-L1014)):
  draw an edit button at `159 + i` next to each row's delete button. Keep the
  row text identical.
- Page-3 input handling: add a branch
  `elseif (input[0] >= 160 && input[0] < 160 + 10)` →
  `HandleEditCombatEvent(who, form_data, input[0] - 159)` →
  `return ShowPage3(...)` to redraw. Place it adjacent to the existing
  add/delete branches ([epicboss.src:1130-1143](../../../pkg/systems/epicspawn/commands/gm/epicboss.src#L1130-L1143)).

No data-shape change; `NormalizeCombatEvent` already persists the edited event.

---

## Feature 3 — "Criar Itens" captures container contents

When the GM targets a **container** for a "Criar Itens" death event, the spawned
copy must include all items inside it, recursively.

The machinery largely exists in the shared
[item_template.inc](../../../pkg/mobiles/merchants/include/item_template.inc)
(`CreateItemInContainerFromTemplate` + `ApplyTemplatePropertiesToItem`). We
extend it with **opt-in** recursive contents capture so the other 13 consumers
of this include (merchants, faccao, GIVE_ITEM, addobj, destroyable, …) are
byte-for-byte unaffected.

Single source of truth: all "snapshot an item" and "recreate an item" logic
stays in item_template.inc. Death-event code only flips the flag on.

### Capture — `CreateItemTemplate(item, capture_contents := 0)`
- Add optional second param, default `0` → existing callers unchanged.
- When `capture_contents` is truthy **and** `item.IsA(POLCLASS_CONTAINER)`:
  - Build `template.+__contents := array{}` (a dedicated template field, **not**
    a CProp — must not leak back as an ObjProperty on restore).
  - For each child in the container, append the child's template (recursion
    carries the flag down).
  - Depth guard: implement via a private worker
    `CreateItemTemplateEx(item, capture_contents, depth)` that does the real
    work; `CreateItemTemplate(item, capture_contents := 0)` calls it with
    `depth := 0`, and the contents loop recurses into the worker with
    `depth + 1`. Abort the contents recursion (skip `__contents`) once
    `depth >= 10` and `Print` a warning, so pathological nesting can't hang
    capture. The public two-arg signature is the only entry callers see.

### Restore — recreate `__contents` after the parent exists
Two entry points must handle `__contents` and must **skip** it when applying
members (it is not a settable member):
- `CreateItemAtLocationFromTemplate(x, y, z, realm, template)` — used by the
  death-event runtime for the top-level drop. After the container item is
  created and its props applied, if `template.__contents` exists, for each child
  template call `CreateItemInContainerFromTemplate(container, child_template)`.
- `CreateItemInContainerFromTemplate(container, template)` — used for nested
  children. Same: after `ApplyTemplatePropertiesToItem`, recreate
  `template.__contents` into the freshly created (possibly container) item.

`ApplyTemplatePropertiesToItem` ([item_template.inc:234-263](../../../pkg/mobiles/merchants/include/item_template.inc#L234-L263))
gains a `"__contents": continue;` case so the contents array is never written as
a member/ObjProperty.

Cleanup: align the inline member-apply loop in
`CreateItemAtLocationFromTemplate` ([item_template.inc:187-190](../../../pkg/mobiles/merchants/include/item_template.inc#L187-L190))
with `ApplyTemplatePropertiesToItem` so both restore paths skip `__contents`
identically (avoid a second, divergent apply loop).

### Death-event config — opt in
- [epicDeathEvents.inc:402](../../../pkg/systems/epicspawn/include/epicDeathEvents.inc#L402):
  `var template := CreateItemTemplate(target_item, 1);` (pass the flag).
- No change to `deathEventsControl.src` `ProcessCreateItemsEvent` — it already
  routes through `CreateItemAtLocationFromTemplate`.

### Behaviour notes
- Targeting a non-container with the flag on is a no-op (no `__contents` added);
  identical to today.
- A nested container inside the targeted container is recreated with its own
  contents (recursion), to the depth limit.
- Stacked items keep `amount` (already carried by the template).

---

## Out of scope / non-goals
- No change to the other 13 `:merchants:include/item_template` consumers.
- No fix for the unrelated documented gotchas (magic_lore, Page-5 ID collision,
  tier bug, etc.).
- No UI for choosing capture depth; fixed guard of 10.

## Verification
eScript is compiled by the user (not via Bash in this session). After edits,
the user recompiles: epicCombatEvents.inc, combatEventsControl.src, epicBoss.inc,
epicboss.src, item_template.inc, epicDeathEvents.inc, deathEventsControl.src.

Manual test matrix:
1. Combat event editor no longer shows "Only when bloodied"; runtime ignores it.
2. Page 3: edit button reopens an existing event, saves changes, persists steps.
3. Death event "Criar Itens" on a filled container → boss death spawns the
   container with all (nested) contents; on a plain item → unchanged.

## Doc follow-up
After implementation, update [docs/epicboss-gm-tool.md](../../epicboss-gm-tool.md):
- §9.1 and gotcha #12 ("combat events stored twice / actions+steps") are already
  stale after this session's `NormalizeCombatEvent` refactor (single `steps`).
- Add the Page-3 edit button (range 160–169) to the §7 button map.
- Note `onlybloodied` removal.
