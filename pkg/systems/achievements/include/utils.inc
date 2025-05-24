include ":achievements:model/Achievement";
include ":achievements:model/Combat";

function DeserializeAchievement(data)
	case (data.type)
		ACHIEVEMENT:
			return Achievement(data.name, data.xp, data.level);

		ACHIEVEMENT_COMBAT:
			return Combat(data.name, data.xp, data.level, data.hits, data.kills, data.damage);
	endcase

	return error{ errortext := "Unknown achievement type." };
endfunction