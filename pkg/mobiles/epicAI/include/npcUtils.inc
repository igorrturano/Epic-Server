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
    // OPTIMIZATION: Use 1-second cached scan to reduce expensive 8-tile scans during combat
    var cache_key := "#allies_8";
    var cached := GetObjProperty(mobile, cache_key);
    var now := polcore().systime;

    if (cached && (CDbl(now) - CDbl(cached.time)) < 1.0)
        return cached.allies;
    endif

    // Cache miss - perform scan
    var mobiles := ListMobilesNearLocation(mobile.x, mobile.y, mobile.z, 8);
    var allies := array{};
    foreach mob in mobiles
        if (isAlly(mob))
            allies.append(mob);
        endif
        sleepms(2);
    endforeach

    SetObjProperty(mobile, cache_key, struct{"allies" := allies, "time" := now});
    return allies;
endfunction

function NPC_GetPlayersPetsAndSummonsNearby(mobile)
    // OPTIMIZATION: Use 1-second cached scan to reduce expensive 8-tile scans during combat
    var cache_key := "#enemies_8";
    var cached := GetObjProperty(mobile, cache_key);
    var now := polcore().systime;

    if (cached && (CDbl(now) - CDbl(cached.time)) < 1.0)
        return cached.enemies;
    endif

    // Cache miss - perform scan
    var mobiles := ListMobilesNearLocation(mobile.x, mobile.y, mobile.z, 8);
    var enemies := array{};
    foreach mob in mobiles
        if (!isAlly(mob))
            enemies.append(mob);
        endif
        sleepms(2);
    endforeach

    SetObjProperty(mobile, cache_key, struct{"enemies" := enemies, "time" := now});
    return enemies;
endfunction