include ":io:DataFile";
include ":achievements:model/Achievement";

const pkg_name := "achievements";

function SaveAchievement(type, who, achievement)
	var DFController := DataFile(pkg_name, type, Hex(who.serial), CStr(achievement.GetRef()));
	var prop := DFController.GetPropertyValue() ?: dictionary{};

	prop[achievement.GetName()] := achievement.Serialize();

	return DFController.SetPropertyValue(prop);
endfunction

function LoadAchievement(type, who, key, name)
	var DFController := DataFile(pkg_name, type, Hex(who.serial), key);
	var prop := DFController.GetPropertyValue();
	var achiev := Achievement(name, key);

	achiev.Deserialize(prop[name]);

	return achiev;
endfunction

function LoadListAchievement(who)
	var achievements := array{};
	var types := array{ ACHIEVEMENT_COMBAT };

	foreach type in (types)
		var DFController := DataFile(pkg_name, type, Hex(who.serial));
		var elemKeys := DFController.GetElementKeys();
		print ("elemKeys: " + elemKeys);

		if (elemKeys.size() < 1)
			continue;
		endif

		foreach key in (elemKeys)
			DFController.SetPropertyName(key);
			var prop := DFController.GetPropertyValue();
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