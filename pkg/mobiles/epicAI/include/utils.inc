use cfgfile;

include "npcStats";
include "include/NPCUtil";

/**
 * getNPCDamages(npc)
 *
 * Purpose
 * Passar todos os valores dos AttackDamage do NPC em um único struct.
 *
 * Parameters
 * npc: MobRef do NPC.
 *
 * Return struct
 * Retorna um struct com todos os AttackDamage do NPC.
 *
 */
function getNPCDamages(npc)
	var elem := NPC_GetNPCConfig(npc);
	var elemDamages := GetConfigString(elem, "EpicAttackDamage");
	elemDamages := SplitWords(elemDamages);
	var damages := struct;

	damages.+SlashDamage := elemDamages[1];
	damages.+PiercingDamage := elemDamages[2];
	damages.+BluntDamage := elemDamages[3];

	return damages;
endfunction

/**
 * getNPCResists(npc)
 *
 * Purpose
 * Passar todos os valores das resistências do NPC em um único struct.
 *
 * Parameters
 * npc: MobRef do NPC.
 *
 * Return struct
 * Retorna um struct com todos as resistências do NPC.
 *
 */
function getNPCResists(npc)
	var elem := NPC_GetNPCConfig(npc);
	var elemFiscResists := GetConfigString(elem, "EpicFisResists");
	elemFiscResists := SplitWords(elemFiscResists);
	var elemElemResists := GetConfigString(elem, "EpicElemResists");
	elemElemResists := SplitWords(elemElemResists);
	var stats := struct;

	foreach resist in elemFiscResists
	case (resist)
			"T0": resist := RESIST_TIER0; break;
			"T1": resist := RESIST_TIER1; break;
			"T2": resist := RESIST_TIER2; break;
			"T3": resist := RESIST_TIER3; break;
			"T4": resist := RESIST_TIER4; break;
			"T5": resist := RESIST_TIER5; break;
			default: resist := 0; break;
		endcase
	endforeach
	foreach resist in elemElemResists
	case (resist)
			"T0": resist := RESIST_TIER0; break;
			"T1": resist := RESIST_TIER1; break;
			"T2": resist := RESIST_TIER2; break;
			"T3": resist := RESIST_TIER3; break;
			"T4": resist := RESIST_TIER4; break;
			"T5": resist := RESIST_TIER5; break;
			default: resist := 0; break;
		endcase
	endforeach

	stats.+SlashResist := elemFiscResists[1];
	stats.+PiercingResist := elemFiscResists[2];
	stats.+BluntResist := elemFiscResists[3];

	stats.+FireResist := elemElemResists[1];
	stats.+ColdResist := elemElemResists[2];
	stats.+PoisonResist := elemElemResists[3];
	stats.+EnergyResist := elemElemResists[4];

	return stats;
endfunction