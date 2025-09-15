include ":datafile:DFTools";
include ":achievements:model/Achievement";

const pkg_name := ":achievements:";

function SaveAchievement(type, who, achievement)
	var DFController := DFTools(pkg_name + type);
	var elem := Hex(who.serial);
	var key := CStr(achievement.GetRef());
	var prop := DFController.LoadValue(elem, key) ?: dictionary{};

	prop[achievement.GetName()] := achievement.Serialize();

	return DFController.WriteValue(elem, key, prop);
endfunction

function LoadAchievement(type, who, key, name)
	var DFController := DFTools(pkg_name + type);
	var prop := DFController.LoadValue(Hex(who.serial), key);
	var achiev := Achievement(name, key);

	achiev.Deserialize(prop[name]);

	return achiev;
endfunction

function LoadListAchievement(who)
	var achievements := array{};
	var types := array{ ACHIEVEMENT_COMBAT };

	foreach type in (types)
		var DFController := DFTools(pkg_name + type);
		var elemKeys := DFController.GetElementKeys(Hex(who.serial));

		if (elemKeys.size() < 1)
			continue;
		endif

		foreach key in (elemKeys)
			var prop := DFController.LoadValue(Hex(who.serial), key);
			foreach value in (prop)
				var achievement := Achievement(value.name, key);
				achievement.Deserialize(value);
				
				achievements.append(achievement);
			endforeach
		endforeach;
	endforeach;

	if (achievements.size() == 0) return error{ errortext := "No achievements found for this player." }; endif

	return achievements;
endfunction