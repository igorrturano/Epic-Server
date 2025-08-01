include ":datafile:DFTools";
include ":achievements:model/Achievement";
include ":achievements:model/AchievementCombat";
include ":achievements:model/AchievementCrafting";
include ":achievements:model/AchievementExploration";

// var DFController := DFTools(":achievements:achievements");

function SaveAchievement(who, achievement)
	var DFController := DFTools(":achievements:" + achievement.GetType());

	return DFController.WriteValue(Hex(who.serial), achievement.GetKey(), achievement.Serialize());
endfunction

function LoadAchievement(who, type, key)
	var DFController := DFTools(":achievements:" + type);
	var achievData := DFController.LoadValue(Hex(who.serial), key);
	var achiev;

	if (!achievData) return error{ errortext := $"Achievement data not found for {key} ({type})" }; endif

	case (type)
		ACHIEVEMENT:
			achiev := Achievement(key);
			break;

		ACHIEVEMENT_COMBAT:
			achiev := AchievementCombat(key);
			break;

		ACHIEVEMENT_CRAFTING:
			achiev := AchievementCrafting(key);
			break;
		
		ACHIEVEMENT_EXPLORATION:
			achiev := AchievementExploration(key);
			break;

		default:
			return error{ errortext := $"Invalid achievement type: {type}"};
	endcase

	achiev.Deserialize(achievData);

	return achiev;
endfunction

function LoadAllAchievements(who)
	var achievements := array{};
	var propNames := array{};
	var types := array{ ACHIEVEMENT_COMBAT };

	foreach type in (types)
		var DFController := DFTools(":achievements:" + type);
		var propNamesType := DFController.GetElementPropNames(Hex(who.serial));

		if (propNamesType.size() > 0)
			propNames.append(propNamesType);
		else
			continue;
		endif

		foreach propName in (propNames)
			var achievData := DFController.LoadValue(Hex(who.serial), propName);
			var achievement := LoadAchievement(who, achievData.type, propName);

			achievements.append(achievement);
		endforeach;

	endforeach;

	if (achievements.size() == 0) return error{ errortext := "No achievements found for this character." }; endif

	return achievements;
endfunction