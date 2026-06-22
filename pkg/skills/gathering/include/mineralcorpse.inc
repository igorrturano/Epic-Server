/*
 * mineralcorpse.inc
 *
 * Golens cujo cadáver pode ser minerado com picareta. Cada golem entrega um
 * único tipo de minério ao longo de MINERAL_CORPSE_MAX_SWINGS swings. Falha
 * no skill check consome o swing. Quando os swings zeram, o cadáver é
 * destruído.
 *
 * O loot normal do corpo (corpseitm/corpseamt) continua acessível enquanto
 * o cadáver existir — esta mecânica é paralela ao decompositor.
 */

use uo;

include "include/NPCUtil";

const MINERAL_CORPSE_MAX_SWINGS := 10;

const MINERAL_CORPSE_PROP_SWINGS  := "mineavel_swings";
const MINERAL_CORPSE_PROP_TEMPLATE := "mineavel_template";

/*
 * MineralGolemTable()
 *
 * Mapa template → struct com:
 *   ore       — objtype do minério bruto
 *   qty_min   — quantidade mínima por swing bem-sucedido
 *   qty_max   — quantidade máxima por swing bem-sucedido
 *   dif       — dificuldade do skill check (Mining)
 *   nome      — nome de exibição do golem (pra gump)
 */
function MineralGolemTable()
	return dictionary{
		// Tier 5 — risco alto, drop enxuto
		"golem"      -> struct{ "ore" := 0x6302, "ore_nome" := "Fulmetita",        "qty_min" := 1, "qty_max" := 2, "dif" := 70, "nome" := "Golem de Aço" },
		"golem2"     -> struct{ "ore" := 0x6074, "ore_nome" := "Argila",           "qty_min" := 3, "qty_max" := 6, "dif" := 20, "nome" := "Golem de Lama" },
		"golem3"     -> struct{ "ore" := 0x6304, "ore_nome" := "Ignitita",         "qty_min" := 1, "qty_max" := 2, "dif" := 70, "nome" := "Golem de Lava" },
		"golem9"     -> struct{ "ore" := 0x1bde, "ore_nome" := "Carvão",           "qty_min" := 2, "qty_max" := 4, "dif" := 10, "nome" := "Golem de Fogo" },
		"machina8"   -> struct{ "ore" := 0x6995, "ore_nome" := "Aço",              "qty_min" := 1, "qty_max" := 3, "dif" := 70, "nome" := "Golem Martelada" },

		// Tier 4 — corte leve só em metais
		"golem4"     -> struct{ "ore" := 0x6451, "ore_nome" := "Frigutita",        "qty_min" := 1, "qty_max" := 2, "dif" := 70, "nome" := "Golem das Águas" },
		"golem5"     -> struct{ "ore" := 0x6300, "ore_nome" := "Minério de Ferro", "qty_min" := 2, "qty_max" := 4, "dif" := 50, "nome" := "Golem de Ferro" },
		"golem6"     -> struct{ "ore" := 0xBD11, "ore_nome" := "Pedras",           "qty_min" := 4, "qty_max" := 6, "dif" := 5,  "nome" := "Golem de Terra" },
		"golem7"     -> struct{ "ore" := 0x8850, "ore_nome" := "Mármore",          "qty_min" := 1, "qty_max" := 3, "dif" := 40, "nome" := "Golem de Pedra" },
		"machina7"   -> struct{ "ore" := 0x62ff, "ore_nome" := "Ferro Impuro",     "qty_min" := 2, "qty_max" := 3, "dif" := 45, "nome" := "Sentinela Anciente" },

		// Tier 3 — farmáveis, drop um pouco mais generoso
		"golem8"     -> struct{ "ore" := 0x1BE3, "ore_nome" := "Vidro",            "qty_min" := 3, "qty_max" := 5, "dif" := 35, "nome" := "Golem de Obsidiana" },
		"golem10"    -> struct{ "ore" := 0x1BE3, "ore_nome" := "Vidro",            "qty_min" := 3, "qty_max" := 5, "dif" := 35, "nome" := "Golem de Cristal" },
		"golem11"    -> struct{ "ore" := 0x6314, "ore_nome" := "Illyra",           "qty_min" := 2, "qty_max" := 3, "dif" := 80, "nome" := "Golem de Illyra" }
	};
endfunction

/*
 * GetMineavelInfo(template)
 *
 * Retorna o struct da tabela ou error se o template não é mineral.
 */
function GetMineavelInfo(template)
	if (!template || template == error)
		return error;
	endif
	// POL armazena no formato ":pkg:template" — extrai apenas o nome.
	var parsed := NPC_ParseTemplateName(CStr(template));
	var key := parsed.template;
	var table := MineralGolemTable();
	var info := table[key];
	if (!info || info == error)
		return error;
	endif
	return info;
endfunction

/*
 * IsMineableCorpse(item)
 *
 * Verdadeiro se o item é um cadáver com prop mineavel_swings > 0.
 */
function IsMineableCorpse(item)
	if (!item || item == error)
		return 0;
	endif
	// Não filtra por graphic: golpes críticos com arma 2H mudam o graphic
	// para 0xC9DE (gore) via TriggerCriticalKillExplosion. A prop
	// mineavel_swings é setada apenas em cadáveres no npcdeath.src.
	var swings := GetObjProperty(item, MINERAL_CORPSE_PROP_SWINGS);
	if (swings == error || !swings)
		return 0;
	endif
	return CInt(swings) > 0;
endfunction

/*
 * GetCorpseMineavelTemplate(corpse)
 *
 * Lê o nome do template marcado no cadáver (formato ":pkg:nome").
 */
function GetCorpseMineavelTemplate(corpse)
	var tmpl := GetObjProperty(corpse, MINERAL_CORPSE_PROP_TEMPLATE);
	if (tmpl == error || !tmpl)
		return error;
	endif
	return tmpl;
endfunction

/*
 * GetCorpseMineavelInfo(corpse)
 *
 * Lê o template marcado no cadáver e devolve o struct da tabela.
 */
function GetCorpseMineavelInfo(corpse)
	var tmpl := GetCorpseMineavelTemplate(corpse);
	if (tmpl == error)
		return error;
	endif
	return GetMineavelInfo(tmpl);
endfunction

/*
 * GetCorpseMineavelSwings(corpse)
 */
function GetCorpseMineavelSwings(corpse)
	var swings := GetObjProperty(corpse, MINERAL_CORPSE_PROP_SWINGS);
	if (swings == error)
		return 0;
	endif
	return CInt(swings);
endfunction

/*
 * ConsumeMineavelSwing(corpse)
 *
 * Decrementa swings. Retorna swings restantes. Em 0, destrói o cadáver.
 */
function ConsumeMineavelSwing(corpse)
	var swings := GetCorpseMineavelSwings(corpse) - 1;
	if (swings <= 0)
		DestroyItem(corpse);
		return 0;
	endif
	SetObjProperty(corpse, MINERAL_CORPSE_PROP_SWINGS, swings);
	return swings;
endfunction

/*
 * MarkCorpseAsMineavel(corpse, template)
 *
 * Hook chamado pelo npcdeath.src quando o template é mineral.
 */
function MarkCorpseAsMineavel(corpse, template)
	var info := GetMineavelInfo(template);
	if (info == error)
		return 0;
	endif
	SetObjProperty(corpse, MINERAL_CORPSE_PROP_TEMPLATE, template);
	SetObjProperty(corpse, MINERAL_CORPSE_PROP_SWINGS, MINERAL_CORPSE_MAX_SWINGS);
	return 1;
endfunction

/*
 * GetGolemTemplateFromMob(mob)
 *
 * Devolve o template se o mob vivo é um golem mineral, senão error.
 */
function GetGolemTemplateFromMob(mob)
	if (!mob || mob == error)
		return error;
	endif
	var tmpl := mob.npctemplate;
	if (tmpl == error || !tmpl)
		return error;
	endif
	if (GetMineavelInfo(tmpl) == error)
		return error;
	endif
	return tmpl;
endfunction
