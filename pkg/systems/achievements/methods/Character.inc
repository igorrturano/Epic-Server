include ":achievements:utils/datafile";
include ":achievements:model/Achievement";
include ":achievements:model/AchievementCombat";

print("INSTALLING: Methods for Character -> Achievement System... OK!");

exported function GetAchievementProgress(who, type, name)
	return LoadAchievement(who, type, name);
endfunction

exported function GetAllAchievementsProgress(who)
	return LoadAllAchievements(who);
endfunction

exported function SetAchievementProgress(who, type, object)
	return SaveAchievement(who, type, object);
endfunction