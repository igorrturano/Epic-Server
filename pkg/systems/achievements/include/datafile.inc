include ":datafile:DFTools";
include ":achievements:model/Achievement";
include ":achievements:model/Combat";
include ":achievements:model/Crafting";
include ":achievements:model/Exploration";

var DFController := DFTools(":achievements:achievements");

function SaveAchievement(who, achievement)
	var achievData := achievement.Serialize();
	DFController.WriteValue(Hex(who.serial), achievement.GetName(), achievData);

	return true;
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
			achiev := Combat(name);
			break;

		ACHIEVEMENT_CRAFTING:
			achiev := Crafting(name);
			break;
		
		ACHIEVEMENT_EXPLORATION:
			achiev := Exploration(name);
			break;

		default:
			return error{ errortext := $"Invalid achievement type: {type}"};
	endcase

	achiev.Deserialize(achievData);

	return achiev;
endfunction