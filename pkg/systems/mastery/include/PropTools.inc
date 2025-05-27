include ":mastery:model/MasteryCombat";

function LoadMastery(who, key)
	var mastery, data;
	var prop := who.GetProp("Masteries") ?: dictionary{};
	data := prop[key];

	if (!data) return error{ errortext := $"Mastery not found for key: {key}" }; endif

	case (data.type)
		MASTERY_COMBAT:
			mastery := MasteryCombat(key);
			break;
		
		default:
			return error{ errortext := $"Invalid mastery type: {data.type}" };
	endcase

	mastery.Deserialize(data);

	return mastery;
endfunction

function SaveMastery(who, mastery)
	var prop := who.GetProp("Masteries") ?: dictionary{};
	var data := mastery.Serialize();
	var key := mastery.GetName();

	prop[key] := data;

	who.SetProp("Masteries", prop);

	return true;
endfunction