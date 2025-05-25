include ":datafile:datafile";
include ":achievements:model/Achievement";
include ":achievements:model/Combat";

const FILENAME := ":achievements:achievements";

function SaveAchievement(who, achievement)
	var file := DFOpenDataFile(FILENAME, DF_CREATE);
	var elem := DFFindElement(file, Hex(who.serial), DF_CREATE);
	var achievData := achievement.Serialize();

	elem.SetProp(achievement.GetName(), achievData);

	return true;
endfunction

function LoadAchievement(who, type, name)
	var file := OpenDataFile(FILENAME);
	var elem := file.FindElement(Hex(who.serial));
	var achievData := elem.GetProp(name);
	var achiev;

	case (type)
		ACHIEVEMENT:
			achiev := Achievement(name);
			break;

		ACHIEVEMENT_COMBAT:
			achiev := Combat(name);
			break;

		default:
			return error{ errortext := $"Invalid achievement type: {type}"};
	endcase

	if (!achievData)
		return error{ errortext := $"Achievement data not found for {name} ({type})" };
	else
		achiev.Deserialize(achievData);
	endif

	return achiev;
endfunction