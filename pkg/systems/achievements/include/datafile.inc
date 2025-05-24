include ":datafile:datafile";
include ":achievements:model/Achievement";
include ":achievements:model/Combat";
include ":achievements:utils";

const FILENAME := ":achievements:achievements";

function SaveAchievement(who, achievement)
	var file := DFOpenDataFile(FILENAME, DF_CREATE);
	var elem := DFFindElement(file, Hex(who.serial), DF_CREATE);
	var achievData := achievement.Serialize();

	elem.SetProp(achievement.GetName(), achievData);

	return true;
endfunction

function LoadAchievement(who, name)
	var file := OpenDataFile(FILENAME);
	var elem := file.FindElement(Hex(who.serial));
	var achievData := elem.GetProp(name);
	var achiev := DeserializeAchievement(achievData);

	return achiev;
endfunction