include ":achievements:model/Achievement";
include ":datafile:datafile";

const FILENAME := ":achievements:achievements";

function Save(who, achievement)
	var file := DFOpenDataFile(FILENAME, DF_CREATE);
	var elem := DFFindElement(file, Hex(who.serial), DF_CREATE);
	var achievData := achievement.Serialize();

	elem.SetProp(achievement.GetName(), achievData);
	return 1;
endfunction

function Load(who, name)
	var file := OpenDataFile(FILENAME);
	var elem := file.FindElement(Hex(who.serial));

	var achievData := elem.GetProp(name);
	var achiev := Achievement(name, 0, 0, 0, 0, 0); // Cria um novo Achievement para receber os dados.
	achiev.Deserialize(achievData);

	return achiev;
endfunction