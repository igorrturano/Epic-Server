// =============================================================================
// voice_aliases.inc — Sistema de palavras-chave customizadas de voz
// =============================================================================
//
// Permite que o lider de uma faccao cadastre palavras-chave alternativas para
// os comandos de voz dos guardas (atacar/parar/seguir/etc), incluindo um slot
// de broadcast para substituir o "todos" canonico (ex: "Soldados sigam-me").
//
// ARMAZENAMENTO
//   guild.GetProp("voice_aliases") -> dictionary{ slot_key -> array<string> }
//
// SLOTS
//   attack, stop, follow, goto, patrol, stop_patrol, hold, resume, broadcast
//
// FLUXO
//   ApplyVoiceAliases(text, guild) injeta a palavra canonica no texto se algum
//   alias casar, ANTES dos if/elseif do ResolveSpeech rodarem. Os comandos
//   canonicos do guardfaccao.src continuam funcionando inalterados.
// =============================================================================

use uo;
use os;
use util;
use guilds;

include ":gumps:gumps";

const VA_PROP := "voice_aliases";
const VA_MAX_PER_SLOT := 5;

// ----------------------------------------------------------------------------
// Definicao dos slots canonicos
// ----------------------------------------------------------------------------
function _VA_Slots()
	return array{
		struct{ "key" := "attack",      "label" := "Atacar",            "defaults" := "mate, ataque, prenda",   "inject" := " mate "             },
		struct{ "key" := "stop",        "label" := "Parar",             "defaults" := "pare",                   "inject" := " pare "             },
		struct{ "key" := "follow",      "label" := "Seguir",            "defaults" := "siga",                   "inject" := " siga "             },
		struct{ "key" := "goto",        "label" := "Ir Ate",            "defaults" := "ir ate",                 "inject" := " ir at "            },
		struct{ "key" := "patrol",      "label" := "Patrulhar",         "defaults" := "patrulha",               "inject" := " patrulha "         },
		struct{ "key" := "stop_patrol", "label" := "Parar Patrulha",    "defaults" := "parar patrulha",         "inject" := " parar patrulha "   },
		struct{ "key" := "hold",        "label" := "Manter Posicao",    "defaults" := "manter posicao",         "inject" := " mant posicao "     },
		struct{ "key" := "resume",      "label" := "Assumir Posto",     "defaults" := "assumir posto",          "inject" := " assum posto "      },
		struct{ "key" := "broadcast",   "label" := "Broadcast (Todos)", "defaults" := "todos",                  "inject" := " todos "            }
	};
endfunction

// Reservadas: derivadas dos `inject` de _VA_Slots() para que adicionar um slot
// novo automaticamente registre suas palavras canonicas.
function _VA_ReservedCanonicals()
	var reserved := array{};
	foreach slot in _VA_Slots()
		foreach w in splitwords(slot.inject)
			sleepms(2);
			if (!(w in reserved)) reserved.append(w); endif
		endforeach
	endforeach
	return reserved;
endfunction

// True se duas palavras colidem por substring matching (em qualquer direcao).
function _VA_Conflicts(a, b)
	return (a[b] != error || b[a] != error);
endfunction

// Blocklist conversacional: palavras comuns que nao podem virar alias.
function _VA_Blocklist()
	return array{
		"para", "isso", "vamos", "voce", "esta", "estou", "estamos",
		"talvez", "agora", "depois", "antes", "mesmo", "porque",
		"muito", "menos", "mais", "olha", "fala", "diga", "onde",
		"como", "quem", "quando", "ainda", "aqui", "tudo", "essa",
		"esse", "aquele", "aquela", "alguem", "ninguem", "nada"
	};
endfunction

// ----------------------------------------------------------------------------
// IO basico
// ----------------------------------------------------------------------------
function GetVoiceAliases(guild)
	var data := guild.GetProp(VA_PROP);
	if (data == error || !data) return dictionary{}; endif
	return data;
endfunction

function _VA_GetSlotAliases(data, slot_key)
	var current := data[slot_key];
	if (current == error || !current) return array{}; endif
	return current;
endfunction

function _VA_SaveSlot(guild, slot_key, aliases)
	var data := GetVoiceAliases(guild);
	data[slot_key] := aliases;
	return guild.SetProp(VA_PROP, data);
endfunction

// ----------------------------------------------------------------------------
// Resolver: injeta palavras canonicas se algum alias casar
// ----------------------------------------------------------------------------
function ApplyVoiceAliases(text, guild)
	if (!guild) return text; endif
	var data := GetVoiceAliases(guild);
	if (data.size() == 0) return text; endif

	foreach slot in _VA_Slots()
		var aliases := _VA_GetSlotAliases(data, slot.key);
		if (aliases.size() == 0) continue; endif
		foreach alias in aliases
			sleepms(2);
			if (text[alias] != error)
				text := text + slot.inject;
				break;
			endif
		endforeach
	endforeach
	return text;
endfunction

// ----------------------------------------------------------------------------
// Validacao
// ----------------------------------------------------------------------------
function _VA_Trim(s)
	while (len(s) > 0 && s[1] == " ") s := s[2, len(s)]; endwhile
	while (len(s) > 0 && s[len(s)] == " ") s := s[1, len(s) - 1]; endwhile
	return s;
endfunction

// Retorna struct{ ok, value | error }
function ValidateAlias(raw, slot_key, guild)
	var alias := _VA_Trim(lower(cstr(raw)));

	if (len(alias) < 4)
		return struct{ "ok" := 0, "msg" := "Alias precisa ter pelo menos 4 caracteres." };
	endif
	if (len(alias) > 20)
		return struct{ "ok" := 0, "msg" := "Alias muito longo (max 20 caracteres)." };
	endif
	if (alias[" "] != error)
		return struct{ "ok" := 0, "msg" := "Alias nao pode conter espaco." };
	endif

	foreach common in _VA_Blocklist()
		sleepms(2);
		if (alias == common)
			return struct{ "ok" := 0, "msg" := "Palavra muito comum em conversa." };
		endif
	endforeach

	// Conflito com canonicos (alias contem ou e contido por palavra reservada)
	foreach word in _VA_ReservedCanonicals()
		sleepms(2);
		if (_VA_Conflicts(alias, word))
			return struct{ "ok" := 0, "msg" := "Conflita com palavra reservada: " + word };
		endif
	endforeach

	// Conflito com aliases de OUTROS slots
	var data := GetVoiceAliases(guild);
	foreach slot in _VA_Slots()
		if (slot.key == slot_key) continue; endif
		foreach other in _VA_GetSlotAliases(data, slot.key)
			sleepms(2);
			if (_VA_Conflicts(alias, other))
				return struct{ "ok" := 0, "msg" := "Conflita com alias '" + other + "' em " + slot.label };
			endif
		endforeach
	endforeach

	// Limites do proprio slot
	var current := _VA_GetSlotAliases(data, slot_key);
	if (alias in current)
		return struct{ "ok" := 0, "msg" := "Esse alias ja existe." };
	endif
	if (current.size() >= VA_MAX_PER_SLOT)
		return struct{ "ok" := 0, "msg" := "Limite de " + cstr(VA_MAX_PER_SLOT) + " aliases por categoria." };
	endif

	return struct{ "ok" := 1, "value" := alias };
endfunction

// ----------------------------------------------------------------------------
// CRUD
// ----------------------------------------------------------------------------
function AddVoiceAlias(guild, slot_key, alias)
	var data := GetVoiceAliases(guild);
	var current := _VA_GetSlotAliases(data, slot_key);
	current.append(alias);
	return _VA_SaveSlot(guild, slot_key, current);
endfunction

function RemoveVoiceAlias(guild, slot_key, alias)
	var data := GetVoiceAliases(guild);
	var current := _VA_GetSlotAliases(data, slot_key);
	var idx := 0;
	foreach a in current
		if (a == alias)
			idx := _a_iter;
			break;
		endif
		sleepms(2);
	endforeach
	if (idx == 0) return 0; endif
	current.Erase(idx);
	return _VA_SaveSlot(guild, slot_key, current);
endfunction

// ----------------------------------------------------------------------------
// Permissao
// ----------------------------------------------------------------------------
function _VA_CanEdit(who, guild)
	if (who.cmdlevel) return 1; endif
	var leader := guild.GetProp("leader");
	if (who.serial == leader) return 1; endif
	var privs := guild.GetProp(cstr(who.serial));
	if (privs && privs.above_guards) return 1; endif
	var subguilds := who.guild.GetProp("subguilds");
	if (subguilds && guild.guildid in subguilds) return 1; endif
	return 0;
endfunction

// ----------------------------------------------------------------------------
// GUMP: Lista de slots
// ----------------------------------------------------------------------------
function OpenCommunicationGump(who, guild)
	if (!_VA_CanEdit(who, guild))
		SendSysMessage(who, "Voce nao tem permissao para configurar a comunicacao.");
		return;
	endif

	while (1)
		sleepms(5);
		var slots := _VA_Slots();
		var data := GetVoiceAliases(guild);

		var gump := GFCreateGump(180, 80);
		var h := 80 + (slots.size() * 24);
		GFResizePic(gump, 0, 0, 0x254E, 360, h);

		GFTextLine(gump, 90, 12, 2620, "Comunicacao do Guarda");
		GFTextLine(gump, 30, 32, 2100, "Edite as palavras-chave de voz por categoria.");

		var y := 58;
		foreach slot in slots
			var aliases := _VA_GetSlotAliases(data, slot.key);
			GFAddButton(gump, 25, y, 2117, 2118, 1, 100 + _slot_iter);
			GFTextLine(gump, 50, y - 2, 2100, slot.label);
			GFTextLine(gump, 220, y - 2, 2400, "[" + cstr(aliases.size()) + " custom]");
			y := y + 24;
			sleepms(2);
		endforeach

		GFAddButton(gump, 25, y + 4, 2117, 2118, 1, 99);
		GFTextLine(gump, 50, y + 2, 2100, "Fechar");

		var input := GFSendGump(who, gump)[0];
		if (input == 0 || input == 99) return; endif

		var idx := input - 100;
		if (idx >= 1 && idx <= slots.size())
			_VA_OpenSlotEditor(who, guild, slots[idx]);
		endif
	endwhile
endfunction

// ----------------------------------------------------------------------------
// GUMP: Editor de um slot especifico
// ----------------------------------------------------------------------------
function _VA_OpenSlotEditor(who, guild, slot)
	while (1)
		sleepms(5);
		var data := GetVoiceAliases(guild);
		var aliases := _VA_GetSlotAliases(data, slot.key);

		var gump := GFCreateGump(200, 100);
		var h := 180 + (aliases.size() * 22);
		GFResizePic(gump, 0, 0, 0x254E, 380, h);

		GFTextLine(gump, 110, 12, 2620, "Editar: " + slot.label);
		GFTextLine(gump, 25, 38, 2400, "Defaults (sempre ativos):");
		GFTextLine(gump, 25, 54, 2100, slot.defaults);

		GFTextLine(gump, 25, 80, 2400, "Aliases personalizados (" + cstr(aliases.size()) + "/" + cstr(VA_MAX_PER_SLOT) + "):");

		var y := 100;
		if (aliases.size() == 0)
			GFTextLine(gump, 45, y, 2400, "(nenhum)");
			y := y + 22;
		else
			foreach a in aliases
				GFAddButton(gump, 25, y, 4017, 4018, 1, 200 + _a_iter);
				GFTooltipText(gump, "Remover");
				GFTextLine(gump, 50, y - 2, 2100, a);
				y := y + 22;
				sleepms(2);
			endforeach
		endif

		if (aliases.size() < VA_MAX_PER_SLOT)
			GFAddButton(gump, 25, y + 8, 2117, 2118, 1, 300);
			GFTextLine(gump, 50, y + 6, 2100, "Adicionar novo alias");
			y := y + 22;
		endif

		GFAddButton(gump, 25, y + 18, 2117, 2118, 1, 99);
		GFTextLine(gump, 50, y + 16, 2100, "Voltar");

		var input := GFSendGump(who, gump)[0];
		if (input == 0 || input == 99) return; endif

		if (input == 300)
			var raw := RequestInput(who, who.backpack, "Digite o novo alias para " + slot.label + ":");
			if (!raw || raw == error) continue; endif
			var result := ValidateAlias(raw, slot.key, guild);
			if (!result.ok)
				SendSysMessage(who, "[Comunicacao] " + result.msg);
			else
				AddVoiceAlias(guild, slot.key, result.value);
				SendSysMessage(who, "[Comunicacao] Alias '" + result.value + "' adicionado a " + slot.label + ".");
			endif
			continue;
		endif

		// Remover alias N
		var rm_idx := input - 200;
		if (rm_idx >= 1 && rm_idx <= aliases.size())
			var target_alias := aliases[rm_idx];
			RemoveVoiceAlias(guild, slot.key, target_alias);
			SendSysMessage(who, "[Comunicacao] Alias '" + target_alias + "' removido.");
		endif
	endwhile
endfunction
