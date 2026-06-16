use uo;
use os;
use datafile;
use cfgfile;

include ":gumps:gumps";
include ":gumps:gumps_ex";
include ":gumps:requestGump";
include ":datafile:datafile";
include ":epicAI:epicai";

// =========================================================================
// Constants
// =========================================================================

const OBJTYPE_SPAWNPOINT_SP := 0xA401;
const OBJTYPE_EPICBOSSSPAWN := 0xA402;
const OBJTYPE_EPICSPAWNER   := 0x5000;

const NAME_MAX_LEN          := 28;
const ROWS_PER_PAGE         := 18;
const ROW_HEIGHT_PX         := 18;
const WINDOW_SIZE           := 100;

const GUMP_W                := 600;
const GUMP_H                := 500;

const BTN_TP_BASE           := 1000;
const BTN_EDIT_BASE         := 2000;
const BTN_PREV_WINDOW       := 9001;
const BTN_NEXT_WINDOW       := 9002;

const PIC_BG_OUTER          := 3600;
const PIC_BG_INNER          := 0x13BE;
const PIC_BTN_UP            := 2103;
const PIC_BTN_DOWN          := 2104;
const PIC_BTN_EDIT_UP       := 2117;
const PIC_BTN_EDIT_DOWN     := 2118;
const PIC_PAGE_NEXT         := 2437;
const PIC_PAGE_PREV         := 2436;
const TEXT_COLOR            := 1153;

// =========================================================================
// Type detectors
// =========================================================================

function IsEpicBossSpawner(sp)
	return (sp.objtype == OBJTYPE_EPICBOSSSPAWN);
endfunction

function IsEpicSpawner(sp)
	if (sp.objtype != OBJTYPE_EPICSPAWNER)
		return 0;
	endif
	var fd := GetObjProperty(sp, "form_data");
	return (fd && fd != error);
endfunction

function IsSpawnerObjtype(ot)
	return (ot == OBJTYPE_SPAWNPOINT_SP || ot == OBJTYPE_EPICBOSSSPAWN ||
			ot == OBJTYPE_EPICSPAWNER);
endfunction

// =========================================================================
// Spawner data accessors
// =========================================================================

// Lê o struct do epic boss (BossStruct novo ou form_data legado).
function GetEpicBossData(sp)
	var bs := GetObjProperty(sp, "BossStruct");
	if (bs && bs != error)
		return bs;
	endif
	bs := GetObjProperty(sp, "form_data");
	if (bs && bs != error)
		return bs;
	endif
	return 0;
endfunction

// Lê todos os templates configurados em um spawnpoint.
// Spawn Point clássico: Template1..3 + Spawn (legacy single).
// Epic Boss Spawner: BossStruct.npctemplate (ou form_data.npctemplate).
// Epic Spawner: form_data.mobslvl1/2/3.
// Fallback: form_data.spawner_name (resolve "horse1" como template quando
// o spawner foi nomeado mas não teve mobslvl* configurado).
function CollectSpawnTemplates(spawnpoint)
	var templates := array{};
	var i;
	for (i := 1; i <= 3; i += 1)
		var t := GetObjProperty(spawnpoint, "Template" + i);
		if (t && t != error && t != "" && t != "NONE")
			templates.append(t);
		endif
	endfor
	var single := GetObjProperty(spawnpoint, "Spawn");
	if (single && single != error && single != "" && single != "NONE")
		templates.append(single);
	endif

	var bd := GetObjProperty(spawnpoint, "BossStruct");
	if (!bd || bd == error)
		bd := GetObjProperty(spawnpoint, "form_data");
	endif
	if (bd && bd != error)
		if (bd.npctemplate)
			templates.append(bd.npctemplate);
		endif
		var mob_fields := array{ bd.mobslvl1, bd.mobslvl2, bd.mobslvl3 };
		foreach mob in mob_fields
			if (mob && mob != error && mob != "")
				templates.append(mob);
			endif
		endforeach
		if (bd.spawner_name && bd.spawner_name != "")
			templates.append(bd.spawner_name);
		endif
	endif
	return templates;
endfunction

// =========================================================================
// Respawn config (read/write das duas convenções de storage)
// =========================================================================

// Epic Spawner armazena em form_data.min_respawn/max_respawn.
// Demais usam ObjProperty MinDelay/MaxDelay.
// Retorna struct{ minimum, maximum } em minutos. 0 = não configurado.
function GetRespawnConfig(spawnpoint)
	if (IsEpicSpawner(spawnpoint))
		var fd := GetObjProperty(spawnpoint, "form_data");
		if (fd && fd != error)
			var mn := (fd.min_respawn && fd.min_respawn != error) ? CInt(fd.min_respawn) : 0;
			var mx := (fd.max_respawn && fd.max_respawn != error) ? CInt(fd.max_respawn) : 0;
			return struct{ "minimum" := mn, "maximum" := mx };
		endif
		return struct{ "minimum" := 0, "maximum" := 0 };
	endif
	var mi := GetObjProperty(spawnpoint, "MinDelay");
	var ma := GetObjProperty(spawnpoint, "MaxDelay");
	mi := (mi && mi != error) ? CInt(mi) : 0;
	ma := (ma && ma != error) ? CInt(ma) : 0;
	return struct{ "minimum" := mi, "maximum" := ma };
endfunction

function SetRespawnConfig(spawnpoint, mi, ma)
	if (IsEpicSpawner(spawnpoint))
		var fd := GetObjProperty(spawnpoint, "form_data");
		if (fd && fd != error)
			fd.min_respawn := mi;
			fd.max_respawn := ma;
			SetObjProperty(spawnpoint, "form_data", fd);
		endif
		var cfg := GetObjProperty(spawnpoint, "config");
		if (cfg && cfg != error)
			cfg.min_respawn := mi;
			cfg.max_respawn := ma;
			SetObjProperty(spawnpoint, "config", cfg);
		endif
		return;
	endif
	SetObjProperty(spawnpoint, "MinDelay", mi);
	SetObjProperty(spawnpoint, "MaxDelay", ma);
	SetObjProperty(spawnpoint, "ResetMe", 1);
endfunction

// Para ordenação: spawnpoints sem MinDelay/MaxDelay vão para o fim (99999).
function GetSpawnAvgDelay(spawnpoint)
	var c := GetRespawnConfig(spawnpoint);
	if (c.minimum == 0 && c.maximum == 0)
		return 99999;
	endif
	if (c.minimum == 0)
		return c.maximum;
	endif
	if (c.maximum == 0)
		return c.minimum;
	endif
	return (c.minimum + c.maximum) / 2;
endfunction

// =========================================================================
// Lookup npcdesc para classificação de mount
// =========================================================================

// Procura primeiro no pacote indicado, depois no wildcard como fallback.
function LookupNpcdescElem(template_str)
	var parsed := ParseTemplateName(template_str);
	if (parsed.package && parsed.package != "")
		var cfg := ReadConfigFile(":" + parsed.package + ":npcdesc");
		if (cfg)
			var elem := FindConfigElem(cfg, parsed.template);
			if (elem && !elem.errortext)
				return elem;
			endif
		endif
	endif
	var wildcfg := ReadConfigFile(":*:npcdesc");
	if (!wildcfg)
		return 0;
	endif
	var welem := FindConfigElem(wildcfg, parsed.template);
	if (!welem || welem.errortext)
		return 0;
	endif
	return welem;
endfunction

function IsMountTemplate(template_str)
	var elem := LookupNpcdescElem(template_str);
	return (elem && GetConfigString(elem, "Environment") == "Mount");
endfunction

// =========================================================================
// Classificação e display
// =========================================================================

// Prioridade: mount > trueboss > regular.
// Vale para Spawn Point (.spawnpoint/.cspawn), Epic Boss Spawner (.epicboss)
// e Epic Spawner (.epicspawner). Montarias nunca caem em regular.
function ClassifySpawnpoint(spawnpoint)
	foreach t in (CollectSpawnTemplates(spawnpoint))
		if (IsMountTemplate(t))
			return "mount";
		endif
	endforeach

	if (IsEpicBossSpawner(spawnpoint))
		var bd := GetEpicBossData(spawnpoint);
		if (bd && bd.trueboss)
			return "trueboss";
		endif
		return "regular";
	endif

	if (IsEpicSpawner(spawnpoint))
		return "regular";
	endif

	var uniquestat := GetObjProperty(spawnpoint, "uniquestat");
	if (uniquestat && uniquestat != error && uniquestat.trueboss == 1)
		return "trueboss";
	endif

	return "regular";
endfunction

function FirstTemplateAsName(spawnpoint, fallback)
	var templates := CollectSpawnTemplates(spawnpoint);
	if (templates.size() >= 1)
		return ParseTemplateName(templates[1]).template;
	endif
	return fallback;
endfunction

function GetSpawnDisplayName(spawnpoint)
	if (IsEpicBossSpawner(spawnpoint))
		var bd := GetEpicBossData(spawnpoint);
		if (bd && bd.name)
			return bd.name;
		endif
		return "(EpicBoss sem nome)";
	endif

	if (IsEpicSpawner(spawnpoint))
		var fd := GetObjProperty(spawnpoint, "form_data");
		if (fd && fd != error && fd.spawner_name && fd.spawner_name != "")
			return fd.spawner_name;
		endif
		return FirstTemplateAsName(spawnpoint, "(EpicSpawner sem nome)");
	endif

	var i;
	for (i := 1; i <= 3; i += 1)
		var nm := GetObjProperty(spawnpoint, "Name" + i);
		if (nm && nm != error && nm != "")
			return nm;
		endif
	endfor
	return FirstTemplateAsName(spawnpoint, "(sem template)");
endfunction

function GetFilterTitle(filter_mode)
	if (filter_mode == "trueboss")
		return "Spawns: TrueBosses";
	elseif (filter_mode == "mount")
		return "Spawns: Montarias";
	endif
	return "Spawns: Monstros (sem boss/montaria)";
endfunction

// =========================================================================
// Coleta e ordenação
// =========================================================================

// Insertion sort por avg crescente.
function SortSpawnEntries(byref entries)
	var n := entries.size();
	var i;
	for (i := 2; i <= n; i += 1)
		var key := entries[i];
		var j := i - 1;
		while (j >= 1 && entries[j].avg > key.avg)
			entries[j + 1] := entries[j];
			j := j - 1;
			sleepms(2);
		endwhile
		entries[j + 1] := key;
	endfor
endfunction

// Helper: avalia um candidato e acrescenta ao resultado se passar no filtro.
// Centraliza validação, dedup e classificação para as 4 fontes.
// byref obrigatório — eScript passa arrays/dicionários por valor sem ele.
function ProcessSpawnerCandidate(spawnpoint, filter_mode, byref seen, byref entries)
	if (!spawnpoint || spawnpoint == error)
		return;
	endif
	if (seen.exists(spawnpoint.serial))
		return;
	endif
	if (ClassifySpawnpoint(spawnpoint) != filter_mode)
		return;
	endif
	entries.append(struct{ "sp" := spawnpoint, "avg" := GetSpawnAvgDelay(spawnpoint) });
	seen[spawnpoint.serial] := 1;
endfunction

// Coleta + filtra + ordena entradas das 4 fontes:
//   :tn:spawns                   → Spawn Points (.spawnpoint, .cspawn)
//   :epicspawn:epicbosses        → Epic! Boss Spawners (.epicboss)
//   GlobalProperty(epicSpawns)   → Epic Spawners (.epicspawner)
//   Varredura por realm (legacy) → spawners criados antes do registro.
function CollectFilteredSpawns(filter_mode)
	var entries := array{};
	var seen := dictionary{};

	var df := OpenDataFile(":tn:spawns");
	if (df)
		foreach key in (df.keys())
			ProcessSpawnerCandidate(SystemFindObjectBySerial(key), filter_mode, seen, entries);
			sleepms(2);
		endforeach
	endif

	var dfeb := OpenDataFile(":epicspawn:epicbosses");
	if (dfeb)
		foreach key2 in (dfeb.keys())
			ProcessSpawnerCandidate(SystemFindObjectBySerial(key2), filter_mode, seen, entries);
			sleepms(2);
		endforeach
	endif

	var globalSpawns := GetGlobalProperty("epicSpawns");
	if (globalSpawns)
		foreach serial in globalSpawns
			ProcessSpawnerCandidate(SystemFindObjectBySerial(serial), filter_mode, seen, entries);
			sleepms(2);
		endforeach
	endif

	foreach realm_info in (Realms())
		var realm_name := _realm_info_iter;
		if (!realm_info.width || !realm_info.height)
			continue;
		endif
		var scanned := 0;
		foreach thing in ListObjectsInBox(1, 1, -128, realm_info.width, realm_info.height,
										   127, realm_name)
			scanned := scanned + 1;
			if (scanned % 500 == 0)
				sleepms(2);
			endif
			if (IsSpawnerObjtype(thing.objtype))
				ProcessSpawnerCandidate(thing, filter_mode, seen, entries);
			endif
		endforeach
	endforeach

	SortSpawnEntries(entries);
	return entries;
endfunction

// =========================================================================
// Gump de edição de respawn
// =========================================================================

function EditRespawnGump(who, spawnpoint)
	var cur := GetRespawnConfig(spawnpoint);
	var cur_min := (cur.minimum > 0) ? CStr(cur.minimum) : "1";
	var cur_max := (cur.maximum > 0) ? CStr(cur.maximum) : "1";

	var new_min := Requestgump(who, "Novo Min Respawn (minutos). Atual: " + cur_min);
	if (!new_min || new_min == "")
		SendSysMessage(who, "Edição cancelada.");
		return 0;
	endif
	var new_max := Requestgump(who, "Novo Max Respawn (minutos). Atual: " + cur_max);
	if (!new_max || new_max == "")
		SendSysMessage(who, "Edição cancelada.");
		return 0;
	endif

	var mi := CInt(new_min);
	var ma := CInt(new_max);
	if (mi < 1)
		mi := 1;
	endif
	if (ma < mi)
		ma := mi;
	endif

	SetRespawnConfig(spawnpoint, mi, ma);
	SendSysMessage(who, "Respawn atualizado: " + mi + " / " + ma + " minutos.");
	return 1;
endfunction

// =========================================================================
// Gump principal (listagem paginada)
// =========================================================================

function HandleGumpResult(who, btn, filter_mode, entries, oldentries, start, finish)
	if (btn == BTN_PREV_WINDOW && oldentries)
		var ns := start - WINDOW_SIZE;
		var nf := finish - WINDOW_SIZE;
		if (ns < 1)
			ns := 1;
			nf := WINDOW_SIZE;
		endif
		if (nf > oldentries.size())
			nf := oldentries.size();
		endif
		ShowSpawnFilteredGump(who, filter_mode, oldentries, ns, nf);
		return;
	endif

	if (btn == BTN_NEXT_WINDOW && oldentries)
		var ns := start + WINDOW_SIZE;
		var nf := finish + WINDOW_SIZE;
		if (ns > oldentries.size())
			SendSysMessage(who, "Fim da lista.");
			ShowSpawnFilteredGump(who, filter_mode, oldentries, start, finish);
			return;
		endif
		if (nf > oldentries.size())
			nf := oldentries.size();
		endif
		ShowSpawnFilteredGump(who, filter_mode, oldentries, ns, nf);
		return;
	endif

	if (btn >= BTN_TP_BASE && btn < BTN_EDIT_BASE)
		var idx := btn - BTN_TP_BASE;
		if (idx >= 1 && idx <= entries.size())
			var target_sp := entries[idx].sp;
			MoveObjectToLocation(who, target_sp.x, target_sp.y, target_sp.z,
								  target_sp.realm, MOVEOBJECT_FORCELOCATION);
			SendSysMessage(who, "Teleportado para spawn de " + GetSpawnDisplayName(target_sp));
		endif
		return;
	endif

	if (btn >= BTN_EDIT_BASE && btn < BTN_PREV_WINDOW)
		var idx2 := btn - BTN_EDIT_BASE;
		if (idx2 >= 1 && idx2 <= entries.size())
			EditRespawnGump(who, entries[idx2].sp);
			if (oldentries)
				ShowSpawnFilteredGump(who, filter_mode, oldentries, start, finish);
			else
				ShowSpawnFilteredGump(who, filter_mode);
			endif
		endif
		return;
	endif
endfunction

function ShowSpawnFilteredGump(who, filter_mode, oldentries := 0, start := 0, finish := 0)
	var entries;
	if (oldentries)
		entries := array{};
		var k := start;
		while (k <= finish && k <= oldentries.size())
			entries.append(oldentries[k]);
			k := k + 1;
			sleepms(2);
		endwhile
	else
		entries := CollectFilteredSpawns(filter_mode);
	endif

	var total := entries.size();
	SendSysMessage(who, "Total " + GetFilterTitle(filter_mode) + ": " + total);

	// Macro paginação: > WINDOW_SIZE entradas → quebra em janelas.
	if (!oldentries && total > WINDOW_SIZE)
		ShowSpawnFilteredGump(who, filter_mode, entries, 1, WINDOW_SIZE);
		return 1;
	endif

	var gump := GFCreateGump();
	GFDisposable(gump, 0);
	GFResizePic(gump, 0, 25, PIC_BG_OUTER, GUMP_W, GUMP_H);
	GFResizePic(gump, 15, 40, PIC_BG_INNER, GUMP_W - 30, GUMP_H - 30);
	GFTextMid(gump, 20, 50, GUMP_W - 40, TEXT_COLOR, GetFilterTitle(filter_mode));

	if (oldentries)
		GFTextLine(gump, 30, 75, TEXT_COLOR,
				   "Exibindo " + start + " a " + finish + " de " + oldentries.size());
		GFTextLine(gump, 350, 75, TEXT_COLOR, "Prev " + WINDOW_SIZE);
		GFTextLine(gump, 450, 75, TEXT_COLOR, "Next " + WINDOW_SIZE);
		GFAddButton(gump, 330, 78, PIC_BTN_UP, PIC_BTN_DOWN, 1, BTN_PREV_WINDOW);
		GFAddButton(gump, 430, 78, PIC_BTN_UP, PIC_BTN_DOWN, 1, BTN_NEXT_WINDOW);
	endif

	GFTextLine(gump, 30, 100, TEXT_COLOR, "Nome");
	GFTextLine(gump, 220, 100, TEXT_COLOR, "X / Y / Z / Realm");
	GFTextLine(gump, 410, 100, TEXT_COLOR, "Min/Max (min)");
	GFTextLine(gump, 510, 100, TEXT_COLOR, "TP");
	GFTextLine(gump, 540, 100, TEXT_COLOR, "Edit");

	GFSetRadioGroup(gump, 1);
	GFPage(gump, 1);

	var y := 120;
	var pagecount := 0;
	var page := 1;
	var i := 1;
	foreach entry in entries
		var sp := entry.sp;
		var name := GetSpawnDisplayName(sp);
		if (Len(name) > NAME_MAX_LEN)
			name := name[1, NAME_MAX_LEN];
		endif
		var cfg := GetRespawnConfig(sp);
		var min_str := (cfg.minimum > 0) ? CStr(cfg.minimum) : "?";
		var max_str := (cfg.maximum > 0) ? CStr(cfg.maximum) : "?";
		var realm_str := sp.realm ? CStr(sp.realm) : "?";

		GFTextLine(gump, 30, y, TEXT_COLOR, name);
		GFTextLine(gump, 220, y, TEXT_COLOR,
				   sp.x + " " + sp.y + " " + sp.z + " " + realm_str);
		GFTextLine(gump, 410, y, TEXT_COLOR, min_str + " / " + max_str);
		GFAddButton(gump, 510, y + 2, PIC_BTN_UP, PIC_BTN_DOWN, 1, BTN_TP_BASE + i);
		GFAddButton(gump, 542, y + 2, PIC_BTN_EDIT_UP, PIC_BTN_EDIT_DOWN, 1, BTN_EDIT_BASE + i);

		y := y + ROW_HEIGHT_PX;
		i := i + 1;
		pagecount := pagecount + 1;
		if (pagecount == ROWS_PER_PAGE)
			GFAddButton(gump, 560, 480, PIC_PAGE_NEXT, PIC_PAGE_NEXT, GF_PAGE_BTN, page + 1);
			page := page + 1;
			GFPage(gump, page);
			GFAddButton(gump, 560, 100, PIC_PAGE_PREV, PIC_PAGE_PREV, GF_PAGE_BTN, page - 1);
			y := 120;
			pagecount := 0;
		endif
		sleepms(2);
	endforeach

	var result := GFSendGump(who, gump);
	if (!result || !result[0])
		return 0;
	endif

	HandleGumpResult(who, result[0], filter_mode, entries, oldentries, start, finish);
	return 1;
endfunction
