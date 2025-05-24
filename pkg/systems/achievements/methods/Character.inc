include ":achievements:datafile";

print("INSTALLING: Methods for Character -> Achievement System... OK!");

exported function GetAchievementProgress(who)
	return Load(who);
	// var prop := who.GetProp("Achievements");

	// if (!prop) 
	// 	prop := dictionary;
	// endif

	// return prop;
endfunction

exported function SetAchievementProgress(who, achiev)
	Save(who, achiev);
	// who.SetProp("Achievements", prop);
endfunction