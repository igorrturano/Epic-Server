include ":datafile:DFTools";
include ":achievements:model/Achievement";
include ":achievements:model/AchievementCombat";
include ":achievements:model/AchievementCrafting";
include ":achievements:model/AchievementExploration";

var DFController := DFTools(":achievements:achievements");

function SaveAchievement(who, achievement)
	return DFController.WriteValue(Hex(who.serial), achievement.GetName(), achievement.Serialize());
endfunction

function LoadAchievement(who, type, name)
	var achievData := DFController.LoadValue(Hex(who.serial), name);
	var achiev;

	if (!achievData) return error{ errortext := $"Achievement data not found for {name} ({type})" }; endif

	case (type)
		ACHIEVEMENT:
			achiev := Achievement(name);
			break;

		ACHIEVEMENT_COMBAT:
			achiev := AchievementCombat(name);
			break;

		ACHIEVEMENT_CRAFTING:
			achiev := AchievementCrafting(name);
			break;
		
		ACHIEVEMENT_EXPLORATION:
			achiev := AchievementExploration(name);
			break;

		default:
			return error{ errortext := $"Invalid achievement type: {type}"};
	endcase

	achiev.Deserialize(achievData);

	return achiev;
endfunction