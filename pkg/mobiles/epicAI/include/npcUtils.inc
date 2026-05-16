include "include/epicNPCUtils";

function NPCRotateAttackType(npc)
	var npcDamages := getNPCDamages(npc);
	var npcPostura := getPropNPCAttackType(npc);
	var damage := struct;

	if ((npcDamages.SlashDamage > npcDamages.PiercingDamage) && (npcPostura != SLASHDAMAGE))
		NPCRotateAttackTypeTo(npc, SLASHDAMAGE);
	elseif ((npcDamages.PiercingDamage > npcDamages.BluntDamage) && (npcPostura != PIERCINGDAMAGE))
		NPCRotateAttackTypeTo(npc, PIERCINGDAMAGE);
	elseif ((npcPostura != BLUNTDAMAGE))
		NPCRotateAttackTypeTo(npc, BLUNTDAMAGE);
	endif

	return damage;
endfunction

function NPCRotateAttackTypeTo(npc, type)
	PrintTextAbove(npc, $"*muda postura de ataque para golpe {Lower(type)}*");
	setPropNPCAttackType(npc, type);
endfunction

function NPC_HasSummonedCreaturesNearby(opponent)
    var summons := NPC_GetPlayersPetsAndSummonsNearby(opponent);
    foreach summon in summons
        if (GetObjProperty(summon, "summoned"))
            return 1;
        endif
        sleepms(2);
    endforeach
    
    return 0;
endfunction

function NPC_GetNearbyAllies(mobile)
    // Use GetNearbyMobilesOnce (already script-level cached) and filter
    var mobiles := GetNearbyMobilesOnce(mobile, 8, LISTEX_FLAG_NORMAL);
    var allies := array{};
    foreach mob in mobiles
        if (isAlly(mob))
            allies.append(mob);
        endif
    endforeach
    return allies;
endfunction

function NPC_GetPlayersPetsAndSummonsNearby(mobile)
    // Use GetNearbyMobilesOnce (already script-level cached) and filter
    var mobiles := GetNearbyMobilesOnce(mobile, 8, LISTEX_FLAG_NORMAL);
    var enemies := array{};
    foreach mob in mobiles
        if (!isAlly(mob))
            enemies.append(mob);
        endif
    endforeach
    return enemies;
endfunction

// IsNonCombatantPet: an owned animal that must never fight, never be
// targeted by hostile AI, always flee, and contribute zero threat. This is
// the grey (neutral-alignment) pet that walks beside its owner. War pets
// (alignment == 1) and summons are combatants and are excluded. Mounts
// while ridden are already excluded upstream by the defender.hidden guard
// in CanFight; the economy-side mount slot exemption is handled in Phase 3.
function IsNonCombatantPet(creature)
    if (!creature || !creature.master)
        return 0;
    endif
    if (GetObjProperty(creature, "summoned"))
        return 0;
    endif
    return (creature.alignment == 0);
endfunction