use uo;
use os;
use util;
use datafile;
use cfgfile;
use guilds;

include "include/NPCUtil";

const KILLLOG_RETENTION_DAYS  := 30;
const KILLLOG_FILE            := ":killlog:killlog";
const KILLLOG_META_LAST_PRUNE := "__last_prune__";
const KILLLOG_META_PREFIX     := "__";
const KILLLOG_KEY_TIME_WIDTH  := 15;

const VICTIM_NPC_FACCAO := "npc_faccao";
const VICTIM_PET        := "pet";
const VICTIM_MONTARIA   := "montaria";
const VICTIM_JOGADOR    := "jogador";

// Props canonicas do projeto (DamageController, faccao, npcdesc).
const PROP_LAST_HIT       := "lastHit";
const PROP_LAST_DAMAGE    := "LastDamage";
const PROP_OWNER          := "owner";
const PROP_GUILD          := "guild";
const PROP_NPCTEMPLATE    := "npctemplate";
const PROP_DADOS_CONVITE  := "dados_convite";
const PROP_NOME_INICIAL   := "nome_inicial";

// Lista canonica dos campos persistidos. Append itera essa lista para gravar
// e LoadRecord extrai os mesmos por nome — duas listas em sincronia evitam
// drift entre escrita e leitura.
function KillLog_RecordProps()
	return array{
		"when", "victim_type", "victim_serial", "victim_name", "victim_template",
		"victim_nome_inicial", "victim_nome_personalizado", "victim_acctname",
		"x", "y", "z", "realm",
		"owner_serial", "owner_name", "owner_acctname",
		"guild_serial", "guild_name",
		"killer_serial", "killer_name", "killer_nome_inicial", "killer_nome_personalizado",
		"killer_acctname", "killer_cmdlevel", "killer_is_npc", "killer_template",
		"killer_master_serial", "killer_master_name", "killer_master_acctname"
	};
endfunction

function KillLog_OpenFile()
	var df := OpenDataFile(KILLLOG_FILE);
	if (!df)
		df := CreateDataFile(KILLLOG_FILE, DF_KEYTYPE_STRING);
		if (!df)
			SysLog("[killlog] CreateDataFile falhou: " + df.errortext);
		endif
	endif
	return df;
endfunction

// Chave width-padded para que sort lexicografico == sort temporal.
// 15 digitos cobre systime ate ~ano 33000 (10^15 segundos).
function KillLog_MakeKey(when, victim_serial)
	var t := CStr(when);
	while (len(t) < KILLLOG_KEY_TIME_WIDTH)
		t := "0" + t;
	endwhile
	return t + "_" + CStr(victim_serial);
endfunction

function KillLog_IsMetaKey(k)
	var s := CStr(k);
	return len(s) >= 2 && s[1, 2] == KILLLOG_META_PREFIX;
endfunction

// Fonte canonica de killer (DamageController.inc:1578 seta lastHit em combate;
// :128 seta LastDamage em qualquer dano). Resolve ate via SYSFIND_OFFLINE.
function KillLog_ResolveKiller(victim)
	if (!victim || victim == error)
		return 0;
	endif
	var serial := 0;

	var lastHit := GetObjProperty(victim, PROP_LAST_HIT);
	if (lastHit != error && lastHit && CInt(lastHit["attacker"]))
		serial := CInt(lastHit["attacker"]);
	endif

	if (!serial)
		var ld := GetObjProperty(victim, PROP_LAST_DAMAGE);
		if (ld != error && ld)
			foreach key in array{"serial", "attacker", "source", "who"}
				if (CInt(ld[key]))
					serial := CInt(ld[key]);
					break;
				endif
			endforeach
		endif
	endif

	if (!serial)
		return 0;
	endif

	var ref := SystemFindObjectBySerial(serial, SYSFIND_SEARCH_OFFLINE_MOBILES);
	if (ref && ref != error)
		return ref;
	endif
	return 0;
endfunction

// Nome RP do mobile. Para player de faccao, prioriza dados_convite.nome_personalizado.
function KillLog_ResolveDisplayName(mob)
	if (!mob || mob == error)
		return "";
	endif
	var dados := GetObjProperty(mob, PROP_DADOS_CONVITE);
	if (dados != error && dados)
		var nome := CStr(dados["nome_personalizado"]);
		if (nome != "" && nome != "error")
			return nome;
		endif
	endif
	var nome_inicial := GetObjProperty(mob, PROP_NOME_INICIAL);
	if (nome_inicial != error && nome_inicial && CStr(nome_inicial) != "")
		return CStr(nome_inicial);
	endif
	if (mob.acctname)
		return CStr(mob.acctname);
	endif
	return CStr(mob.name);
endfunction

// Triplete de identidade do jogador (display, nome_inicial, nome_personalizado).
// Centraliza extracao usada tanto para vitima quanto para killer.
function KillLog_ExtractPlayerNames(mob)
	var r := struct;
	r.+display := "";
	r.+nome_inicial := "";
	r.+nome_personalizado := "";
	if (!mob || mob == error)
		return r;
	endif
	r.display := KillLog_ResolveDisplayName(mob);
	r.nome_inicial := CStr(GetObjProperty(mob, PROP_NOME_INICIAL));
	var d := GetObjProperty(mob, PROP_DADOS_CONVITE);
	if (d != error && d)
		r.nome_personalizado := CStr(d["nome_personalizado"]);
	endif
	return r;
endfunction

// "A corpse of Guarda Sentinela" -> "Guarda Sentinela".
// POL gera capitalizacao canonica; comparacao direta basta.
function KillLog_StripCorpsePrefix(name)
	var s := CStr(name);
	if (len(s) > 12 && s[1, 12] == "A corpse of ")
		return s[13, len(s) - 12];
	endif
	return s;
endfunction

function KillLog_IsMountTemplate(template)
	if (!template || template == "" || template == "<uninitialized object>")
		return 0;
	endif
	var cfgfile := ReadConfigFile(":*:npcdesc");
	if (!cfgfile)
		return 0;
	endif
	var parsed := NPC_ParseTemplateName(template).template;
	return CStr(GetConfigString(cfgfile[parsed], "Environment")) == "Mount";
endfunction

// IMPORTANTE: corpse e' POLCLASS_CORPSE — npctemplate so' via GetObjProperty.
function KillLog_ClassifyNpcVictim(corpse)
	if (CInt(GetObjProperty(corpse, PROP_OWNER)))
		var tmpl := CStr(GetObjProperty(corpse, PROP_NPCTEMPLATE));
		if (KillLog_IsMountTemplate(tmpl))
			return VICTIM_MONTARIA;
		endif
		return VICTIM_PET;
	endif
	if (GetObjProperty(corpse, PROP_GUILD))
		return VICTIM_NPC_FACCAO;
	endif
	return "";
endfunction

function KillLog_BuildBaseRecord(victim, vtype)
	var killer := KillLog_ResolveKiller(victim);

	var rec := struct;
	rec.+when := polcore().systime;
	rec.+victim_type := vtype;
	rec.+victim_serial := victim.serial;
	// Corpse: npctemplate so' via GetObjProperty. Mobile vivo: campo direto.
	var raw_tmpl := GetObjProperty(victim, PROP_NPCTEMPLATE);
	if (raw_tmpl == error || !raw_tmpl)
		raw_tmpl := victim.npctemplate;
	endif
	rec.+victim_template := CStr(raw_tmpl);
	rec.+x := victim.x;
	rec.+y := victim.y;
	rec.+z := victim.z;
	rec.+realm := CStr(victim.realm);

	if (vtype == VICTIM_JOGADOR)
		var v_names := KillLog_ExtractPlayerNames(victim);
		rec.+victim_name := v_names.display;
		rec.+victim_nome_inicial := v_names.nome_inicial;
		rec.+victim_nome_personalizado := v_names.nome_personalizado;
		rec.+victim_acctname := CStr(victim.acctname);
	else
		rec.+victim_name := KillLog_StripCorpsePrefix(victim.name);
		rec.+victim_nome_inicial := "";
		rec.+victim_nome_personalizado := "";
		rec.+victim_acctname := "";
	endif

	var owner_serial := CInt(GetObjProperty(victim, PROP_OWNER));
	rec.+owner_serial := owner_serial;
	rec.+owner_name := "";
	rec.+owner_acctname := "";
	if (owner_serial)
		var owner := SystemFindObjectBySerial(owner_serial, SYSFIND_SEARCH_OFFLINE_MOBILES);
		if (owner && owner != error)
			rec.owner_name := KillLog_ResolveDisplayName(owner);
			rec.owner_acctname := CStr(owner.acctname);
		endif
	endif

	var guild_serial := CInt(GetObjProperty(victim, PROP_GUILD));
	rec.+guild_serial := guild_serial;
	rec.+guild_name := "";
	if (guild_serial)
		var g := FindGuild(guild_serial);
		if (g && g != error)
			rec.guild_name := CStr(g.name);
		endif
	endif

	rec.+killer_serial := 0;
	rec.+killer_name := "";
	rec.+killer_nome_inicial := "";
	rec.+killer_nome_personalizado := "";
	rec.+killer_acctname := "";
	rec.+killer_cmdlevel := 0;
	rec.+killer_is_npc := 0;
	rec.+killer_template := "";
	rec.+killer_master_serial := 0;
	rec.+killer_master_name := "";
	rec.+killer_master_acctname := "";

	if (killer)
		rec.killer_serial := killer.serial;
		if (killer.IsA(POLCLASS_NPC))
			rec.killer_is_npc := 1;
			rec.killer_template := CStr(killer.npctemplate);
			rec.killer_name := KillLog_StripCorpsePrefix(killer.name);
			if (killer.master)
				rec.killer_master_serial := killer.master.serial;
				rec.killer_master_name := KillLog_ResolveDisplayName(killer.master);
				rec.killer_master_acctname := CStr(killer.master.acctname);
			endif
		else
			var k_names := KillLog_ExtractPlayerNames(killer);
			rec.killer_name := k_names.display;
			rec.killer_nome_inicial := k_names.nome_inicial;
			rec.killer_nome_personalizado := k_names.nome_personalizado;
			rec.killer_acctname := CStr(killer.acctname);
			rec.killer_cmdlevel := CInt(killer.cmdlevel);
		endif
	endif

	return rec;
endfunction

function KillLog_Append(rec)
	var df := KillLog_OpenFile();
	if (!df)
		return 0;
	endif
	var key := KillLog_MakeKey(rec.when, rec.victim_serial);
	var elem := df.FindElement(key);
	if (!elem)
		elem := df.CreateElement(key);
		if (!elem)
			SysLog("[killlog] CreateElement falhou: " + elem.errortext);
			return 0;
		endif
	endif
	foreach k in (KillLog_RecordProps())
		elem.SetProp(k, rec[k]);
	endforeach
	return 1;
endfunction

// API publica
function LogVictimDeath(corpse)
	if (!corpse || corpse == error)
		return 0;
	endif
	var vtype := KillLog_ClassifyNpcVictim(corpse);
	if (vtype == "")
		return 0;
	endif
	return KillLog_Append(KillLog_BuildBaseRecord(corpse, vtype));
endfunction

function LogPlayerDeath(who)
	if (!who || who == error)
		return 0;
	endif
	return KillLog_Append(KillLog_BuildBaseRecord(who, VICTIM_JOGADOR));
endfunction

// CStr(uninit) vira literal "<uninitialized object>"; CStr(error) vira "error{...}".
// Normaliza para "" inline (sem foreach de pos-processamento).
function KillLog_NormStr(v)
	var s := CStr(v);
	if (s == "<uninitialized object>")
		return "";
	endif
	if (len(s) >= 6 && s[1, 6] == "error{")
		return "";
	endif
	return s;
endfunction

// Extracao explicita: rec.+(p) := value com p dinamico nao popula struct
// em eScript (bug confirmado em teste). Verboso e' a unica forma confiavel.
function KillLog_LoadRecord(elem)
	var rec := struct;
	rec.+when                       := CInt(elem.GetProp("when"));
	rec.+victim_type                := KillLog_NormStr(elem.GetProp("victim_type"));
	rec.+victim_serial              := CInt(elem.GetProp("victim_serial"));
	rec.+victim_name                := KillLog_NormStr(elem.GetProp("victim_name"));
	rec.+victim_template            := KillLog_NormStr(elem.GetProp("victim_template"));
	rec.+victim_nome_inicial        := KillLog_NormStr(elem.GetProp("victim_nome_inicial"));
	rec.+victim_nome_personalizado  := KillLog_NormStr(elem.GetProp("victim_nome_personalizado"));
	rec.+victim_acctname            := KillLog_NormStr(elem.GetProp("victim_acctname"));
	rec.+x                          := CInt(elem.GetProp("x"));
	rec.+y                          := CInt(elem.GetProp("y"));
	rec.+z                          := CInt(elem.GetProp("z"));
	rec.+realm                      := KillLog_NormStr(elem.GetProp("realm"));
	rec.+owner_serial               := CInt(elem.GetProp("owner_serial"));
	rec.+owner_name                 := KillLog_NormStr(elem.GetProp("owner_name"));
	rec.+owner_acctname             := KillLog_NormStr(elem.GetProp("owner_acctname"));
	rec.+guild_serial               := CInt(elem.GetProp("guild_serial"));
	rec.+guild_name                 := KillLog_NormStr(elem.GetProp("guild_name"));
	rec.+killer_serial              := CInt(elem.GetProp("killer_serial"));
	rec.+killer_name                := KillLog_NormStr(elem.GetProp("killer_name"));
	rec.+killer_nome_inicial        := KillLog_NormStr(elem.GetProp("killer_nome_inicial"));
	rec.+killer_nome_personalizado  := KillLog_NormStr(elem.GetProp("killer_nome_personalizado"));
	rec.+killer_acctname            := KillLog_NormStr(elem.GetProp("killer_acctname"));
	rec.+killer_cmdlevel            := CInt(elem.GetProp("killer_cmdlevel"));
	rec.+killer_is_npc              := CInt(elem.GetProp("killer_is_npc"));
	rec.+killer_template            := KillLog_NormStr(elem.GetProp("killer_template"));
	rec.+killer_master_serial       := CInt(elem.GetProp("killer_master_serial"));
	rec.+killer_master_name         := KillLog_NormStr(elem.GetProp("killer_master_name"));
	rec.+killer_master_acctname     := KillLog_NormStr(elem.GetProp("killer_master_acctname"));
	return rec;
endfunction

// Itera datafile em ordem temporal decrescente (key e' time-padded).
function KillLog_Query(filters)
	var df := KillLog_OpenFile();
	var results := array;
	if (!df)
		return results;
	endif
	var keys := df.Keys();
	keys.Reverse();
	foreach k in keys
		if (KillLog_IsMetaKey(k))
			continue;
		endif
		var elem := df.FindElement(k);
		if (!elem)
			continue;
		endif
		if (!MatchFilter(elem, filters))
			sleepms(2);
			continue;
		endif
		results.append(KillLog_LoadRecord(elem));
		sleepms(2);
	endforeach
	return results;
endfunction

function MatchFilter(elem, filters)
	var when := CInt(elem.GetProp("when"));
	if (filters.Exists("from_time") && when < CInt(filters["from_time"]))
		return 0;
	endif
	if (filters.Exists("to_time") && when > CInt(filters["to_time"]))
		return 0;
	endif
	if (filters.Exists("victim_type") && filters["victim_type"] != ""
	    && CStr(elem.GetProp("victim_type")) != filters["victim_type"])
		return 0;
	endif
	if (filters.Exists("owner_name") && filters["owner_name"] != ""
	    && !Lower(CStr(elem.GetProp("owner_name")))[Lower(filters["owner_name"])])
		return 0;
	endif
	if (filters.Exists("killer_name") && filters["killer_name"] != ""
	    && !Lower(CStr(elem.GetProp("killer_name")))[Lower(filters["killer_name"])])
		return 0;
	endif
	if (filters.Exists("guild_name") && filters["guild_name"] != ""
	    && !Lower(CStr(elem.GetProp("guild_name")))[Lower(filters["guild_name"])])
		return 0;
	endif
	if (filters.Exists("victim_name") && filters["victim_name"] != ""
	    && !Lower(CStr(elem.GetProp("victim_name")))[Lower(filters["victim_name"])])
		return 0;
	endif
	return 1;
endfunction

// Apaga registros mais antigos que `days`. Use `days=0` para apagar tudo (wipe).
function KillLog_Prune(days := KILLLOG_RETENTION_DAYS)
	var df := KillLog_OpenFile();
	if (!df)
		return 0;
	endif
	var cutoff := polcore().systime - (CInt(days) * 86400);
	var removed := 0;
	// Loop ate' esvaziar — registros adicionados durante o sleepms entram na
	// proxima passada.
	while (1)
		var keys := df.Keys();
		var found_any := 0;
		foreach k in keys
			if (KillLog_IsMetaKey(k))
				continue;
			endif
			var elem := df.FindElement(k);
			if (!elem)
				continue;
			endif
			if (CInt(elem.GetProp("when")) < cutoff)
				df.DeleteElement(k);
				removed := removed + 1;
				found_any := 1;
				sleepms(5);
			endif
		endforeach
		if (!found_any)
			break;
		endif
	endwhile
	return removed;
endfunction

// Roda Prune so' se passou >=1 dia desde o ultimo. Marca via elemento meta.
function KillLog_PruneIfNeeded()
	var df := KillLog_OpenFile();
	if (!df)
		return 0;
	endif
	var meta := df.FindElement(KILLLOG_META_LAST_PRUNE);
	var last := meta ? CInt(meta.GetProp("when")) : 0;
	if (polcore().systime - last < 86400)
		return 0;
	endif
	var removed := KillLog_Prune();
	if (!meta)
		meta := df.CreateElement(KILLLOG_META_LAST_PRUNE);
	endif
	if (meta)
		meta.SetProp("when", polcore().systime);
	endif
	return removed;
endfunction
