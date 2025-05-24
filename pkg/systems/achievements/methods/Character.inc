include ":achievements:datafile";
include ":achievements:model/Achievement";
include ":achievements:model/Combat";

print("INSTALLING: Methods for Character -> Achievement System... OK!");

exported function GetAchievementProgress(who, name)
	return LoadAchievement(who, name);
endfunction

exported function SetAchievementProgress(who, object)
	SaveAchievement(who, object);
endfunction