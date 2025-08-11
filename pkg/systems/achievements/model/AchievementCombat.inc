include ":achievements:model/Achievement";

const ACHIEVEMENT_COMBAT := ACHIEVEMENT + "_COMBAT";

class AchievementCombat(Achievement)
	var data; // Readonly - Dictionary com dados do achievement (hits, misses, damage, totaldamage, etc.) [o enum que rotula eles está na classe filha]

	function AchievementCombat(this, playerRef, criteriaRef, name := false, level := 0, xp := 0, data := dictionary{})
		Super(playerRef, criteriaRef, name, level, xp);
		this.data := data;
	endfunction

	function GetData(this, key)
		return this.data[key];
	endfunction

	function GetDataKeys(this)
		return this.data.keys();
	endfunction

	function SetData(this, key, value)
		this.data[key] := value;
	endfunction

	function IncrementData(this, key, value)
		this.data[key] += CInt(value);
	endfunction

	function Serialize(this)
		var base_data := Achievement::Serialize(this);
		base_data.+data := this.data;

		return base_data;
	endfunction
	
	function Deserialize(this, data)
		if (!data) return error{ errortext := "Invalid data for Achievement deserialization." }; endif
		Achievement::Deserialize(this, data);

		this.data := data.data;
	endfunction
	
	function toString(this)
		return $"Achievement para {this.name} (playerRef={this.playerRef}, criteriaRef={this.criteriaRef}, xp={this.xp}, level={this.level}, data={this.data})";
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