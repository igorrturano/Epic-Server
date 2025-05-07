include ":equipsys:props";
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
    var allies := array{};
    foreach mobile in ListMobilesNearLocation(mobile.x, mobile.y, mobile.z, 8)
        if (isAlly(mobile))
            allies.append(mobile);
        endif
        sleepms(2);
    endforeach
    
    return allies;
endfunction

function NPC_GetPlayersPetsAndSummonsNearby(mobile)
    var enemies := array{};
    foreach mobile in ListMobilesNearLocation(mobile.x, mobile.y, mobile.z, 8)
        if (!isAlly(mobile))
            enemies.append(mobile);
        endif
        sleepms(2);
    endforeach
    
    return enemies;
endfunction