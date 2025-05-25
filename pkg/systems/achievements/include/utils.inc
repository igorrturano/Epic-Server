include ":achievements:model/Achievement";
include ":achievements:model/Combat";
include ":achievements:model/Crafting";

function DeserializeAchievement(data)
	case (data.type)
		ACHIEVEMENT:
			return Achievement(data.name, data.xp, data.level);

		ACHIEVEMENT_COMBAT:
			return Combat(data.name, data.xp, data.level, data.hits, data.kills, data.damage);

		ACHIEVEMENT_CRAFTING:
			return Crafting(data.name, data.xp, data.level, data.craftedNormal, data.craftedExceptional);
	endcase

	return error{ errortext := "Unknown achievement type." };
endfunction