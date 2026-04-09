include ":io:DataFile";
include ":achievements:models/Achievement";
include ":serialization:StructSerializer";

const achiev_pkg := "achievements";

function SaveAchievement(type, who, achievement)
	var DFController := DataFile(achiev_pkg, type, Hex(who.serial)).SetElementName(CStr(achievement.GetRef()));
	var prop := DFController.GetPropertyValue() ?: dictionary{};

	prop[achievement.GetName()] := StructSerializer().Serialize(achievement);

	return DFController.SetPropertyValue(prop);
endfunction

function LoadAchievement(type, who, key, name)
	var DFController := DataFile(achiev_pkg, type).SetElementName(Hex(who.serial));
	var prop := DFController.GetPropertyValue(key);
	var achiev := StructSerializer().Deserialize(Achievement(name, key), prop[name]);

	return achiev;
endfunction

function LoadListAchievement(who)
	var achievements := array{};
	var types := array{ ACHIEVEMENT_TYPE::COMBAT };

	foreach type in (types)
		var DFController := DataFile(achiev_pkg, type).SetElementName(Hex(who.serial));
		var elemKeys := DFController.GetElementProps();
		print ("elemKeys: " + elemKeys);

		if (elemKeys.size() < 1)
			continue;
		endif

		foreach key in (elemKeys)
			var prop := DFController.GetPropertyValue(key);
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