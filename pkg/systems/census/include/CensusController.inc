include ":datafile:datafile";

DF_DEBUG_MODE := 0;

const CENSUS_DATAFILE := ":census:CensusData";

// DataFile separates a prop's key from its value by whitespace, so a census
// key that itself contains spaces (e.g. "Story Mode", "Povos do Leste") got
// serialized inconsistently and split into two counters. Collapsing every
// whitespace run to a single underscore gives one stable key — the same
// convention skills ("Two_Handed") and subclasses ("Arqueiro_Arcano")
// already use, and those categories have zero split rows.
function CensusNormalizeKey(key)
	var parts := SplitWords(CStr(key));
	if (!parts || parts == error || parts.size() == 0)
		return CStr(key);
	endif
	var joined := parts[1];
	var idx := 2;
	while (idx <= parts.size())
		joined := joined + "_" + parts[idx];
		idx := idx + 1;
	endwhile
	return joined;
endfunction

function _CensusIncrement(category, key)
	var f := DFOpenDataFile(CENSUS_DATAFILE, DF_CREATE);
	if (!f || f == error)
		return;
	endif

	var elem := DFFindElement(f, category, DF_CREATE);
	if (!elem || elem == error)
		return;
	endif

	var nkey := CensusNormalizeKey(key);
	var new_val := CInt(elem.GetProp(nkey)) + 1;
	elem.SetProp(nkey, new_val);
endfunction

// Normalizes keys on read and SUMS collisions, so historical split rows
// ("Story Mode" + "Story\tMode" + "Story_Mode") fold into one entry for
// display without rewriting the legacy datafile.
function CensusGetCounters(category)
	var f := DFOpenDataFile(CENSUS_DATAFILE, DF_CREATE);
	if (!f || f == error)
		return dictionary;
	endif

	var elem := DFFindElement(f, category, DF_NO_CREATE);
	if (!elem || elem == error)
		return dictionary;
	endif

	var props := DFGetElemProps(elem);
	if (!props || props == error)
		return dictionary;
	endif

	var result := dictionary;
	foreach prop in (props)
		var val := elem.GetProp(prop);
		if (val != error)
			var nkey := CensusNormalizeKey(prop);
			var existing := result.Exists(nkey) ? result[nkey] : 0;
			result[nkey] := existing + CInt(val);
		endif
	endforeach

	return result;
endfunction

// Reads an element preserving raw string values (no CInt). Used by the
// live snapshot pages for packed "acct:xp;acct:xp" detail and meta.
function CensusGetRaw(category)
	var f := DFOpenDataFile(CENSUS_DATAFILE, DF_CREATE);
	if (!f || f == error)
		return dictionary;
	endif

	var elem := DFFindElement(f, category, DF_NO_CREATE);
	if (!elem || elem == error)
		return dictionary;
	endif

	var props := DFGetElemProps(elem);
	if (!props || props == error)
		return dictionary;
	endif

	var result := dictionary;
	foreach prop in (props)
		var val := elem.GetProp(prop);
		if (val != error)
			result[prop] := CStr(val);
		endif
	endforeach

	return result;
endfunction

// Replace semantics: wipes the element and rewrites it from kv so reruns
// of .censo atual overwrite the prior snapshot instead of accumulating
// (unlike _CensusIncrement, which is additive by design for creation data).
function CensusSnapshot(category, kv)
	var f := DFOpenDataFile(CENSUS_DATAFILE, DF_CREATE);
	if (!f || f == error)
		return;
	endif

	f.DeleteElement(category);

	var elem := DFFindElement(f, category, DF_CREATE);
	if (!elem || elem == error)
		return;
	endif

	foreach k in (kv.keys())
		elem.SetProp(CensusNormalizeKey(k), kv[k]);
	endforeach
endfunction

function CensusTrackAbility(ability_id)
	_CensusIncrement("abilities", CStr(ability_id));
endfunction

function _AddPair(byref pairs, cat, val)
	// Stricter than a bare truthiness check: living veterans carry partial
	// chardata structs, so a missing member resolves to error here. Skip
	// those instead of recording an "error" key.
	if (val != error && val)
		pairs.append(struct{ cat := cat, key := val });
	endif
endfunction

// Single source of truth: maps a chardata struct to the list of
// (category, key) census dimensions it contributes. Shared by the
// cumulative creation tracker and the live .censo atual snapshot so the
// two can never drift apart.
function CensusCreationPairs(chardata)
	var pairs := array;
	if (chardata == error || !chardata)
		return pairs;
	endif

	_AddPair(pairs, "creation_culture",    chardata.culture);
	_AddPair(pairs, "creation_classe",     chardata.classe);
	_AddPair(pairs, "creation_race",       chardata.raca);
	_AddPair(pairs, "creation_background", chardata.background);
	_AddPair(pairs, "creation_religion",   chardata.religion);
	_AddPair(pairs, "creation_path",       chardata.wisdom_path);
	_AddPair(pairs, "creation_trait",      chardata.trait);
	// --- Path-rework fields (new system) ---
	_AddPair(pairs, "creation_vocacao",    chardata.path);
	_AddPair(pairs, "creation_subclasse",  chardata.subclasse);
	_AddPair(pairs, "creation_game_mode",  chardata.game_mode);
	_AddPair(pairs, "creation_magia",      chardata.magia);

	// Selected skills, split by tier. Primary = tier 3, Secondary = tier 2.
	// "skills_all" counts every picked skill regardless of tier so we can
	// see raw popularity; the tiered buckets show how players weight builds.
	var myskills := chardata.Skills;
	if (myskills != error && myskills)
		foreach skill_name in (myskills.keys())
			var tier := CInt(myskills[skill_name]);
			pairs.append(struct{ cat := "skills_all", key := skill_name });
			if (tier == 3)
				pairs.append(struct{ cat := "skills_primary", key := skill_name });
			elseif (tier == 2)
				pairs.append(struct{ cat := "skills_secondary", key := skill_name });
			endif
		endforeach
	endif

	return pairs;
endfunction

function CensusTrackCreation(chardata)
	var pairs := CensusCreationPairs(chardata);
	foreach pr in (pairs)
		_CensusIncrement(pr.cat, pr.key);
	endforeach
endfunction
