include ":achievements:model/achievement";
include ":datafile:datafile";

const FILENAME := ":achievements:achievements.dat";

function Save(who, achievement)
	var file := OpenDataFile(FILENAME);
	if (file == error)
		print("Error loading achievements data file: " + file.errortext);
		return;
	endif

	var elem := file.FindElement(Hex(who.serial));
	if (elem == error)
		print("Error finding achievements element: " + elem.errortext);
		return;
	endif

	var achievData := achievement.Serialize();
	foreach data in (achievData)
		elem.SetProp(who, data);
	endforeach
	
	file.Save();
	print("Achievements saved for " + who);

	return 1;
endfunction

function Load(who)
	var file := OpenDataFile(FILENAME);
	if (file == error)
		print("Error opening achievements data file: " + file.errortext);
		return;
	endif

	var elem := file.FindElement(Hex(who.serial));
	if (elem == error)
		print("Error finding achievements element: " + elem.errortext);
		return;
	endif

	var achievement := Achievement(elem.GetProp("xp"), elem.GetProp("level"), elem.GetProp("hits"), elem.GetProp("kills"), elem.GetProp("damage"));

	return achievement;
endfunction