include ":achievements:utils/datafile";
include ":achievements:model/Achievement";
include ":achievements:model/AchievementCombat";

print("INSTALLING: Methods for Character -> Achievement System... OK!");

exported function GetAchievement(who, type, key, name)
	return LoadAchievement(type, who, key, name);
endfunction

exported function GetListAchievement(who)
	return LoadListAchievement(who);
endfunction

exported function SetAchievement(who, type, elem, achievement)
	return SaveAchievement(type, elem, achievement);
endfunction