include ":datafile:datafile";

DF_DEBUG_MODE := 0;

const CENSUS_DATAFILE := ":census:CensusData";

function _CensusIncrement(category, key)
	var f := DFOpenDataFile(CENSUS_DATAFILE, DF_CREATE);
	if (!f || f == error)
		return;
	endif

	var elem := DFFindElement(f, category, DF_CREATE);
	if (!elem || elem == error)
		return;
	endif

	var new_val := CInt(elem.GetProp(key)) + 1;
	elem.SetProp(key, new_val);
endfunction

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
			result[prop] := CInt(val);
		endif
	endforeach

	return result;
endfunction

function CensusTrackAbility(ability_id)
	_CensusIncrement("abilities", CStr(ability_id));
endfunction

function CensusTrackCreation(chardata)
	if (chardata.culture)
		_CensusIncrement("creation_culture", chardata.culture);
	endif

	if (chardata.classe)
		_CensusIncrement("creation_classe", chardata.classe);
	endif

	if (chardata.raca)
		_CensusIncrement("creation_race", chardata.raca);
	endif

	if (chardata.background)
		_CensusIncrement("creation_background", chardata.background);
	endif

	if (chardata.religion)
		_CensusIncrement("creation_religion", chardata.religion);
	endif

	if (chardata.wisdom_path)
		_CensusIncrement("creation_path", chardata.wisdom_path);
	endif

	if (chardata.trait)
		_CensusIncrement("creation_trait", chardata.trait);
	endif
endfunction
