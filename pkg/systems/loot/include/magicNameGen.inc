use uo;
use cfgfile;
use util;

// ============================================================
// Loot_GenerateMagicName(base_desc) — composes a roleplay-flavored
// name for a magic loot item by appending random entries from
// pkg/systems/loot/config/item-names.cfg sections Substantivo,
// Adjetivo, and Adverbio. Example output:
//   "Anel da Sereia Brilhante da Aurora"
//
// Reuses the existing item-names.cfg pools — no new content
// needed. If a section is missing/empty its slot is dropped.
// ============================================================

function Loot_PickMagicNamePart(elem)
	if (!elem || elem.errortext)
		return "";
	endif
	var names := GetConfigStringArray(elem, "Name");
	if (!names || names.errortext || names.size() == 0)
		return "";
	endif
	return names[RandomInt(names.size()) + 1];
endfunction

function Loot_GenerateMagicName(base_desc)
	var cfg := ReadConfigFile(":loot:config/item-names");
	if (!cfg || cfg.errortext)
		return base_desc;
	endif

	var sub := Loot_PickMagicNamePart(FindConfigElem(cfg, "Substantivo"));
	var adj := Loot_PickMagicNamePart(FindConfigElem(cfg, "Adjetivo"));
	var adv := Loot_PickMagicNamePart(FindConfigElem(cfg, "Adverbio"));

	var result := CStr(base_desc);
	if (sub != "")
		result := result + " " + sub;
	endif
	if (adj != "")
		result := result + " " + adj;
	endif
	if (adv != "")
		result := result + " " + adv;
	endif
	return result;
endfunction
