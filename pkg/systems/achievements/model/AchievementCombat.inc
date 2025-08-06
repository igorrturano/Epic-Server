include ":achievements:model/Achievement";

const ACHIEVEMENT_COMBAT := ACHIEVEMENT + "_COMBAT";

class AchievementCombat(Achievement)
	function AchievementCombat(this, key)
		Super(key);
		this.type := ACHIEVEMENT_COMBAT;
	endfunction

	function Serialize(this)
		var base_data := Achievement::Serialize(this);

		return base_data;
	endfunction
	
	function Deserialize(this, data)
		if (!data) return error{ errortext := "Invalid data for Achievement deserialization." }; endif
		Achievement::Deserialize(this, data);
	endfunction
	
	function toString(this)
		return $"Achievement para {this.name} ({this.type}) (xp={this.xp}, level={this.level}, data={this.achievData})";
	endfunction
endclass

enum ACHIEVDATA
	ACHIEVDATA_HITS := "hits",
	ACHIEVDATA_BLOCKED := "blocked",
	ACHIEVDATA_KILLS := "kills",

	ACHIEVDATA_DAMAGE_DONE := "damagedone",
	ACHIEVDATA_DAMAGE_TAKEN := "damagetaken",
	ACHIEVDATA_DAMAGE_MAX := "damagemax",
	ACHIEVDATA_DAMAGE_MAX_TAKEN := "damagemaxtaken",

	ACHIEVDATA_CRITICAL_HITS := "criticalhits",
	ACHIEVDATA_CRITICAL_DAMAGE := "criticaldamage",
	ACHIEVDATA_CRITICAL_DAMAGE_MAX := "criticaldamagemax",

	ACHIEVDATA_HEALING_ONSELF := "healingonself",
	ACHIEVDATA_HEALING_ONOTHERS := "healingonothers",
	ACHIEVDATA_HEALING_MAX := "healingmax",
	ACHIEVDATA_HEALING_TOTAL := "healingtotal",
	
	ACHIEVDATA_MANA_SPENT := "manaspent",
	ACHIEVDATA_MANA_REGEN := "manaregen",
	ACHIEVDATA_MANA_REGEN_MAX := "manaregenmax",
	ACHIEVDATA_MANA_REGEN_TOTAL := "manaregentotal",

	ACHIEVDATA_SPELLS_CASTS := "spellscasts",
	ACHIEVDATA_SPELLS_MOSTCASTED := "spellscastsmostcasted",
endenum