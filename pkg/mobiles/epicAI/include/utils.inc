use cfgfile;

include "npcStats";
include "include/NPCUtil";

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
	var cfg := ReadConfigFile(":*:npcdesc");
	var template := NPC_ParseTemplateName(npc.npctemplate).template;
	return FindConfigElem(cfg, template);
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
	var elem := getNpcCfgElem(npc);
	var elemFiscResists := GetConfigString(elem, "EpicFisResists");
	elemFiscResists := SplitWords(elemFiscResists);
	var elemElemResists := GetConfigString(elem, "EpicElemResists");
	elemElemResists := SplitWords(elemElemResists);
	var stats := struct;

	for i:=1 to Len(elemFiscResists);
		if (elemFiscResists[i] == TIER0)
			elemFiscResists[i] := RESIST_TIER0;
		elseif (elemFiscResists[i] == TIER1)
			elemFiscResists[i] := RESIST_TIER1;
		elseif (elemFiscResists[i] == TIER2)
			elemFiscResists[i] := RESIST_TIER2;
		endif
		// case (elemFiscResists[i])
		// 	TIER0: elemFiscResists[i] := RESIST_TIER0; break;
		// 	TIER1: elemFiscResists[i] := RESIST_TIER1; break;
		// 	TIER2: elemFiscResists[i] := RESIST_TIER2; break;
		// 	TIER3: elemFiscResists[i] := RESIST_TIER3; break;
		// 	TIER4: elemFiscResists[i] := RESIST_TIER4; break;
		// 	TIER5: elemFiscResists[i] := RESIST_TIER5; break;
		// 	default: elemFiscResists[i] := 0; break;
		// endcase
		print("resist -> "+elemFiscResists[i]);
	endfor
	for i:=1 to Len(elemElemResists);
		case (elemElemResists[i])
			TIER0: elemElemResists[i] := RESIST_TIER0; break;
			TIER1: elemElemResists[i] := RESIST_TIER1; break;
			TIER2: elemElemResists[i] := RESIST_TIER2; break;
			TIER3: elemElemResists[i] := RESIST_TIER3; break;
			TIER4: elemElemResists[i] := RESIST_TIER4; break;
			TIER5: elemElemResists[i] := RESIST_TIER5; break;
			default: elemElemResists[i] := 0; break;
		endcase
		print("resist -> "+elemElemResists[i]);
	endfor

	stats.+SlashResist := elemFiscResists[1];
	stats.+PiercingResist := elemFiscResists[2];
	stats.+BluntResist := elemFiscResists[3];

	stats.+FireResist := elemElemResists[1];
	stats.+ColdResist := elemElemResists[2];
	stats.+PoisonResist := elemElemResists[3];
	stats.+EnergyResist := elemElemResists[4];

	print("elemFiscResists2 -> "+elemFiscResists);
	print("elemElemResists2 -> "+elemElemResists);
	print("stats -> "+stats);

	return stats;
endfunction