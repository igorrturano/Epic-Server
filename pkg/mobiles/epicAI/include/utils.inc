use cfgfile;

include "npcStats";

/**
 * getNpcCfgElem(npc)
 *
 * Purpose
 * Ler o .cfg dos NPCs e já retornar seus elements (para não precisar fazendo essa leitura de forma despadronizada em cada script)
 *
 * Parameters
 * npc: MobRef do NPC.
 *
 */
function getNpcCfgElem(npc)
	return FindConfigElem(ReadConfigFile(":*:npcdesc"), npc.npctemplate);
endfunction

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
	var elem := getNpcCfgElem(npc);
	var elemDamages := GetConfigStringArray(elem, "AttackDamage");
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
	var elem := getNpcCfgElem(npc);
	var elemFiscResists := GetConfigStringArray(elem, "EpicFisResists");
	var elemElemResists := GetConfigStringArray(elem, "EpicElemResists");
	var stats := struct;

	foreach resist in elemFiscResists
		case (resist)
			TIER1: resist := RESIST_TIER1; break;
			TIER2: resist := RESIST_TIER2; break;
			TIER3: resist := RESIST_TIER3; break;
			TIER4: resist := RESIST_TIER4; break;
			TIER5: resist := RESIST_TIER5; break;
			default: resist := 0; break;
		endcase
	endforeach
	foreach resist in elemElemResists
		case (resist)
			TIER1: resist := RESIST_TIER1; break;
			TIER2: resist := RESIST_TIER2; break;
			TIER3: resist := RESIST_TIER3; break;
			TIER4: resist := RESIST_TIER4; break;
			TIER5: resist := RESIST_TIER5; break;
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