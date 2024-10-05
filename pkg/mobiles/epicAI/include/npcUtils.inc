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