/*
 * Ability Dispatch Table
 *
 * Purpose: Replace sequential if(me.canX()) chains with pre-built arrays.
 * Tables are built ONCE at brain activation from cached NPC data.
 * Per-tick cost: iterate 1-4 entries (this NPC's actual abilities) vs 12+ checks.
 *
 * Each ability entry has: id (dispatch key), phase (combat/enter),
 * range_min/range_max, cooldown_key.
 */

// Build combat ability table from NPC cache - called once at brain activation
function BuildCombatAbilities(who)
    var cache := NPC_GetCachedData(who);
    var ab := cache.abilities;
    var table := array{};

    // Ranged abilities (distance 2-6)
    if (ab.canSpitAcid)
        table.append(struct{ "id" := "AcidSpray", "range_min" := 2, "range_max" := 6 });
    endif
    if (ab.CanShootWeb)
        table.append(struct{ "id" := "WebShot", "range_min" := 2, "range_max" := 6 });
    endif
    if (ab.canThrow)
        table.append(struct{ "id" := "ThrowSpecial", "range_min" := 2, "range_max" := 8 });
    endif

    // Melee abilities (distance 0-1)
    if (ab.canBite)
        table.append(struct{ "id" := "Bite", "range_min" := 0, "range_max" := 1 });
    endif
    if (ab.CanTongueGrab)
        table.append(struct{ "id" := "TongueGrab", "range_min" := 0, "range_max" := 5 });
    endif
    if (ab.CanPoisonBite)
        table.append(struct{ "id" := "PoisonBite", "range_min" := 0, "range_max" := 1 });
    endif
    if (ab.CanCharm)
        table.append(struct{ "id" := "CharmOpponent", "range_min" := 0, "range_max" := 8 });
    endif

    // Passive/aura abilities (any range)
    if (ab.isMyrkonid)
        table.append(struct{ "id" := "SporeSpecial", "range_min" := 0, "range_max" := 8 });
    endif
    if (ab.hasHiveMind)
        table.append(struct{ "id" := "HiveMind", "range_min" := 0, "range_max" := 99 });
    endif
    if (ab.canDoConfusion)
        table.append(struct{ "id" := "doConfusion", "range_min" := 0, "range_max" := 4 });
    endif
    if (ab.CanClimbWall)
        table.append(struct{ "id" := "WallClimb", "range_min" := 0, "range_max" := 8 });
    endif
    if (ab.canPetrify)
        table.append(struct{ "id" := "petrify", "range_min" := 0, "range_max" := 4 });
    endif

    // Frighten (dragons, etc.) - checked via method since not all templates are in cache
    if (who.CanFrighten())
        table.append(struct{ "id" := "frightenOpponent", "range_min" := 0, "range_max" := 6 });
    endif

    // Dragon abilities
    if (ab.isDragon)
        table.append(struct{ "id" := "BreathWeapon", "range_min" := 0, "range_max" := 6 });
        table.append(struct{ "id" := "TailSwipe", "range_min" := 0, "range_max" := 2 });
        table.append(struct{ "id" := "WingBuffet", "range_min" := 0, "range_max" := 4 });
    endif

    // Undead abilities
    if (ab.isUndead)
        table.append(struct{ "id" := "FearAura", "range_min" := 0, "range_max" := 4 });
        table.append(struct{ "id" := "UndeadImpedirCura", "range_min" := 0, "range_max" := 3 });
        table.append(struct{ "id" := "MaldicaoMetamorfose", "range_min" := 0, "range_max" := 4 });
    endif

    // Demon abilities
    if (ab.isDemon)
        table.append(struct{ "id" := "BanishSummon", "range_min" := 0, "range_max" := 6 });
        table.append(struct{ "id" := "CurseHeal", "range_min" := 0, "range_max" := 6 });
        table.append(struct{ "id" := "InfernalBurst", "range_min" := 0, "range_max" := 3 });
    endif

    // Beast abilities
    if (ab.isBeast)
        table.append(struct{ "id" := "PackTactics", "range_min" := 0, "range_max" := 4 });
        table.append(struct{ "id" := "Enrage", "range_min" := 0, "range_max" := 99 });
    endif

    // Caster behaviors - these manage their own cooldowns internally
    if (ab.hasMageConfig)
        table.append(struct{ "id" := "MageBehavior", "range_min" := 0, "range_max" := 99 });
    endif

    return table;
endfunction

// Build enter-area ability table - called once at brain activation
function BuildEnterAbilities(who)
    var cache := NPC_GetCachedData(who);
    var ab := cache.abilities;
    var table := array{};

    if (ab.canCharge)
        table.append("Charge");
    endif
    if (ab.hasTunnelTactics)
        table.append("TunnelTactics");
    endif
    if (ab.CanAmbush)
        table.append("Ambush");
    endif

    return table;
endfunction

// Execute combat abilities from pre-built table. Returns 1 if any ability fired.
function TickCombatAbilities(who, opponent, ability_table)
    var dist := Distance(who, opponent);

    foreach ab in ability_table
        if (dist >= ab.range_min && dist <= ab.range_max)
            if (ExecuteCombatAbility(ab.id, who, opponent))
                return 1;
            endif
        endif
    endforeach

    return 0;
endfunction

// Dispatch to actual ability function. Returns 1 if ability executed.
function ExecuteCombatAbility(id, who, opponent)
    case (id)
        "AcidSpray":          return AcidSpray(who, opponent);
        "WebShot":            return WebShot(who, opponent);
        "ThrowSpecial":       return ThrowSpecial(who, opponent);
        "Bite":               return Bite(who, opponent);
        "TongueGrab":         return TongueGrab(who, opponent);
        "PoisonBite":         return PoisonBite(who, opponent);
        "CharmOpponent":      return CharmOpponent(who, opponent);
        "SporeSpecial":       return SporeSpecial(who, opponent);
        "HiveMind":           return HiveMind(who);
        "doConfusion":        return doConfusion(opponent);
        "WallClimb":          return WallClimb(who);
        "petrify":            return petrify(who, opponent);
        "frightenOpponent":   return frightenOpponent(who, opponent);
        "MageBehavior":       return MageBehavior(who, opponent);
        // Dragon
        "BreathWeapon":       return BreathWeapon(who, opponent);
        "TailSwipe":          return TailSwipe(who, opponent);
        "WingBuffet":         return WingBuffet(who, opponent);
        // Undead
        "FearAura":           return FearAura(who, opponent);
        "UndeadImpedirCura":  return UndeadImpedirCura(who, opponent);
        "MaldicaoMetamorfose": return MaldicaoMetamorfose(who, opponent);
        // Demon
        "BanishSummon":       return BanishSummon(who, opponent);
        "CurseHeal":          return CurseHeal(who, opponent);
        "InfernalBurst":      return InfernalBurst(who, opponent);
        // Beast
        "PackTactics":        return PackTactics(who, opponent);
        "Enrage":             return Enrage(who, opponent);
    endcase
    return 0;
endfunction

// Execute enter-area abilities from pre-built table
function TickEnterAbilities(who, source, enter_table)
    foreach ab in enter_table
        case (ab)
            "Charge":        StartCharge(who, source);
            "TunnelTactics": TunnelTactics(who, source);
            "Ambush":        HandleAmbush(who, source);
        endcase
    endforeach
endfunction
