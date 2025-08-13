include ":datafile:DFTools";
include ":achievements:model/Achievement";

function SaveAchievement(type, who, achievement)
	var DFController := DFTools(":achievements:" + type);
	var elem := Hex(who.serial);
	var key := CStr(achievement.GetRef());
	var prop := DFController.LoadValue(elem, key) ?: dictionary{};

	print (prop);

	prop[achievement.GetName()] := achievement.Serialize();

	return DFController.WriteValue(elem, key, prop);
endfunction

function LoadAchievement(type, who, key, name)
	var DFController := DFTools(":achievements:" + type);
	var prop := DFController.LoadValue(Hex(who.serial), key);
	var achiev;

	achiev := Achievement(name, key);
	achiev.Deserialize(prop[key]);

	return achiev;
endfunction

function LoadAllAchievements(who)
	// var achievements := array{};
	// var types := array{ ACHIEVEMENT_COMBAT };

	// foreach type in (types)
	// 	var DFController := DFTools(":achievements:" + type);
	// 	var propNames := DFController.GetElementPropNames(Hex(who.serial));

	// 	if (propNames.size() < 1)
	// 		continue;
	// 	endif

	// 	foreach propName in (propNames)
	// 		var achievData := DFController.LoadValue(Hex(who.serial), propName);
	// 		var achievement := LoadAchievement(who, achievData.type, propName);

	// 		achievements.append(achievement);
	// 	endforeach;

	// endforeach;

	// if (achievements.size() == 0) return error{ errortext := "No achievements found for this character." }; endif

	// return achievements;
endfunction