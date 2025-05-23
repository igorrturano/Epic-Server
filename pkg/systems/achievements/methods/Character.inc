print("INSTALLING: Methods for Character -> Achievement System... OK!");

exported function GetAchievementProp(who)
	var prop := who.GetProp("Achievements");

	if (!prop) 
		prop := dictionary;
	endif

	return prop;
endfunction

exported function SetAchievementProp(who, prop)
	who.SetProp("Achievements", prop);
endfunction