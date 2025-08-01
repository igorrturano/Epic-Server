include ":achievements:model/Achievement";

const ACHIEVEMENT_COMBAT := ACHIEVEMENT + "_COMBAT";

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
	ACHIEVDATA_CRITICAL_DAMAGE_MAX := "criticaldamage_max",

	ACHIEVDATA_MANA_SPENT := "manaspent"
endenum

class AchievementCombat(Achievement)
	var achievData; // Array de dictionary para armazenar informações

	function AchievementCombat(this, key)
		Super(key);
		this.type := ACHIEVEMENT_COMBAT;

		this.achievData := array{};
	endfunction

	function GetData(this, name)
		return this.achievData[name];
	endfunction

	function SetData(this, name, value)
		this.achievData[name] := value;
	endfunction

	function AddData(this, name, value)
		if (!this.achievData[name]) this.SetData(name, 0); endif
		this.achievData[name] += CInt(value);
	endfunction

	function Serialize(this)
		var base_data := Achievement::Serialize(this);

		return base_data;
	endfunction
	
	function Deserialize(this, data)
		if (!data) return error{ errortext := "Invalid data for Achievement deserialization." }; endif
		Achievement::Deserialize(this, data);

		this.achievData := data.achievData ?: array{};
	endfunction
	
	function toString(this)
		return $"Achievement para {this.name} ({this.type}) (xp={this.xp}, level={this.level}, data={this.achievData})";
	endfunction
endclass