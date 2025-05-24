include ":achievements:datafile";

print("INSTALLING: Methods for Character -> Achievement System... OK!");

exported function GetAchievementProgress(who, achievName)
	return Load(who, achievName);
endfunction

exported function SetAchievementProgress(who, achiev)
	Save(who, achiev);
endfunction