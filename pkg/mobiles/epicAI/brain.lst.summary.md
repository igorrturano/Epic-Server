# Phase 0 — Measured Data Segment Baseline

**Source:** `pkg/mobiles/epicAI/brain.lst` (1,646,419 bytes, 57,744 lines), generated 2026-04-18 20:19 via `ecompile -l -f`.

## Headline

**Brain compile unit is at 65,298 / 65,536 bytes = 99.6% full.** Only **238 bytes of headroom** remain before POL refuses to compile. Any new feature touching brain's chain triggers overflow today.

## Composition

| Bucket | Bytes | % of cap | Notes |
|--------|------:|---------:|-------|
| Unique string content | 29,315 | 44.7% | 1,951 unique offsets, 4,190 references (POL deduplicates strings) |
| Unique int literal slots | 19,452 | 29.7% | 4,863 unique slots, 4 bytes each — **POL does NOT deduplicate int literals** (every `:= 12` gets its own slot) |
| Alignment + struct slots + reserved | ~16,500 | 25.2% | Inferred from `total cap - measured strings - measured ints` |
| **Total** | **65,298** | **99.6%** | |

## .lst limitations

- **No file attribution.** The listing tags each literal with offset + content, but not source file. To trace a literal to its origin, grep its content across the codebase (done for the leaks below).
- **No `.dep` regenerated.** `ecompile -l -f` doesn't write `brain.dep` — that file requires a normal compile (without `-l`). The 2026-04-18 18:12 brain.dep is stale; trace the include graph manually.

## Top observations from string content

### 1. Crafting/sealedBooks/architect strings present in brain — major surprise leak

The .lst contains 36 `<BASEFONT…>` HTML literals (1,335 bytes) including crafting-menu fragments like `<BASEFONT COLOR=#7bf8fe> Tempo de Fabricação</BASEFONT><br>`, `<BASEFONT COLOR=#7bf8fe> Materiais Necessários</BASEFONT><br>`, plus the Architect wiki URL `http://www.uosigena.de/wiki/index.php/architect_english` and inscription-book strings.

**Trace:** `pkg/mobiles/epicAI/include/epicai.inc:5` → `include ":loot:include/lootParser"` → `pkg/systems/loot/include/lootParser.inc:10` → `include ":crafting:/include/fabricaComplexos"` → `controladoraComplexos.inc` + `fabricacaoGumps.inc`. lootParser also pulls `:destroyable:destroyItem` and `:sysbook:sealedBooks` (the inscription-book strings).

**Why it's a leak:** `Loot_Generate(me)` is called exactly once per NPC spawn in `InitializeEpicNPC`. Cold-init only. The entire transitive chain (lootParser → magicLoot → destroyItem → sealedBooks → fabricaComplexos → controladoraComplexos → fabricacaoGumps, plus architect via one of those) is paying compile-time rent for a one-shot init call.

**Fix scope:** Same pattern as the prior `:fls_core:fls_characters` removal — extract `Loot_Generate` to a `start_script` call so the entire loot/crafting subgraph leaves brain's compile unit. Estimated savings: **5–10 KB+** (gump HTML + error strings + crafting dispatch tables).

This is the single highest-leverage immediate action and should be Phase 1.5 (or replace original Phase 1) — see adjusted recommendation in the message body, not in this summary file.

### 2. Top 10 longest strings in brain.lst

| Off | Len | Origin (greppable) | Content |
|-----|----:|--------------------|---------|
| 0x3204 | 101 | epicAI brain — RP message | `A evolução de skills do seu personagem está bloqueada até você preencher sua ficha de Role-Play!` |
| 0xcbb7 | 98 | POL stack-trace lib | `[!!!] To a more detailed stack trace, compile with debug mode enabled: \"ecompile -r <path> -f -x\"\n` |
| 0x9bbb | 94 | crafting | error: ` ] Para receitas com componentes, o componente principal é obrigatório e não foi definido  ` |
| 0x0b93 | 84 | attributes | `Error::AP_GetSettingsCfgFile() - Unable to open [:attributes:config/settings.cfg] ->` |
| 0xe47c | 80 | passive gump | `<CENTER><BASEFONT SIZE=5 COLOR=#DDAA44>Golem - Gerenciamento</BASEFONT></CENTER>` |
| 0xc647 | 79 | timedScripts | `Error::TS_GetSettingsCfgFile() - Unable to open [:timedscripts:timedScripts] ->` |
| 0x9047 | 76 | mounts | `Error::MP_GetMountGraphic() - No mount tile graphic matches mobile graphic '` |
| 0x90e7 | 76 | mounts | `Error::MP_GetMountsCfgFile() - Unable to open [:mounts:config/mounts.cfg] ->` |
| 0xe543 | 72 | passive gump | `<CENTER><BASEFONT SIZE=4 COLOR=#AAAAAA>Trocar Gadget</BASEFONT></CENTER>` |
| 0x804c | 56 | traps | `GetTrapInfo()::Could not open :traps:config/traps.cfg ->` |

### 3. The "9.5 KB short-string tax" — structural pressure across all brains

962 unique strings of length 3–30 chars matching propkey/identifier patterns sum to **9,552 bytes (14.6% of cap)**. Examples (with reference counts — high-ref = good dedup, but the 1× cost still stings):

| Refs | Bytes | String | Likely source |
|-----:|------:|--------|---------------|
| 56 | 7 | `damage` | DamageController |
| 39 | 5 | `Hits` | attributes/vitals |
| 38 | 10 | `britannia` | mount/region |
| 28 | 5 | `IDLE` | stateManager |
| 23 | 10 | `Contusivo` | damage type (combat) |
| 22 | 12 | `DAMAGE_NONE` | DamageController |
| 21 | 9 | `Cortante` | damage type (combat) |
| 21 | 11 | `Perfurante` | damage type (combat) |
| 20 | 10 | `Dexterity` | attributes |
| 15 | 8 | `FLEEING` | stateManager |

Per-brain split (Phase 4) is the architectural answer because:
- `Aradalore`, `Kaijin`, `Velkyn`, `Evocatium` (spell-school strings — humanoid-only) are paid by every brain today
- Damage-type strings (`Cortante`/`Perfurante`/`Contusivo`) are paid by every brain via combat includes
- Locale codes (`pt-BR`, `en`) come from translation libs pulled by gumps/messages

### 4. The 19 KB int-literal tax — POL doesn't dedup ints

Every literal int in any inlined file gets its own 4-byte data-segment slot. `_BuildRaceAbilities` alone has ~100 int literals (cd/range_min/range_max values) = ~400 bytes. Cfg-ifying race tables saves the strings AND the int slots — actual recovery is larger than string-only estimates.

## Bytes by 4KB region (visualization of where data lives)

```
0x0000-0x0fff   2176 bytes  ##############################      (legacy core)
0x1000-0x1fff   1438 bytes  ###################
0x2000-0x2fff   1869 bytes  #########################
0x3000-0x3fff   2841 bytes  #######################################
0x4000-0x4fff   1705 bytes  #######################
0x5000-0x5fff   1519 bytes  ####################
0x6000-0x6fff   1607 bytes  ######################
0x7000-0x7fff   1278 bytes  #################
0x8000-0x8fff   1629 bytes  ######################
0x9000-0x9fff   2202 bytes  ##############################       (loot/crafting)
0xa000-0xafff   1937 bytes  ##########################           (loot/crafting)
0xb000-0xbfff   1939 bytes  ##########################
0xc000-0xcfff   2607 bytes  #####################################  (recent: ChargeImpact, TimerControl, Message struct)
0xd000-0xdfff   1807 bytes  ########################
0xe000-0xefff   1614 bytes  ######################                (passive gump HTML)
0xf000-0xffff   1147 bytes  ###############                       (just below cap)
```

Distribution is even — no single 4KB band dominates. Confirms the analysis: bloat is broad, not surgical. Eviction strategy must be architectural (per-brain split + cold-init extract), not byte-trimming.

## Recovery model — projected per phase

| Phase | Recovery (rough) | Confidence | Rationale |
|-------|-----------------:|-----------|-----------|
| 1 (race_abilities cfg + race_groups cfg) | 1.5–2.5 KB | Medium | F7 = 3.2 KB raw; cfg-ification removes most of it |
| **NEW: Loot_Generate → start_script** | **5–10 KB** | **High** | Evicts crafting/sealedBooks/architect/destroyItem/lootParser/magicLoot subgraph |
| 2 (passive gump → separate .src) | 3–5 KB | High | gump HTML + GFSendGump infrastructure |
| 3 (cold init → init_npc.src) | 5–10 KB | Medium | mounts errors + Setup* literals + makeRace strings |
| 4 (per-brain split via npcdesc Script) | 10–20 KB *per brain budget* | High | Structural — eliminates cross-brain inheritance |
| 5 (ExecuteCombatAbility cfg + template arrays) | 2–4 KB | Medium | Reactive only — skip if Phase 4 leaves headroom |

**Total recoverable across all phases: ~25–50 KB**, leaving each split brain at ~15–35 KB data segment with 30+ KB headroom.

## Recommended adjustment to plan

Promote **Loot_Generate extraction** to its own Phase before Phase 1. It's a single-line change in `epicai.inc` (replace the call with `start_script`), evicts the largest single subgraph, and follows the exact pattern already used for `fls_characters`. Lower risk than cfg-ification because there's no schema to design — just a call-site swap.
