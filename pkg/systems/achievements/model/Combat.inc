include ":achievements:model/Achievement";

const ACHIEVEMENT_COMBAT := ACHIEVEMENT + "_COMBAT";

class Combat(Achievement)
	var hits;
	var kills;
	var damage;
	var criticals;

	function Combat(this, name)
		Super(name);
		this.type := ACHIEVEMENT_COMBAT;
		this.hits := 0;
		this.kills := 0;
		this.damage := 0;
		this.criticals := 0;
	endfunction

	function getHits(this)
		return this.hits;
	endfunction

	function getKills(this)
		return this.kills;
	endfunction

	function getDamage(this)
		return this.damage;
	endfunction

	function getCriticals(this)
		return this.criticals;
	endfunction

	function setHits(this, value)
		this.hits := value;
	endfunction

	function setKills(this, value)
		this.kills := value;
	endfunction

	function setDamage(this, value)
		this.damage := value;
	endfunction

	function setCriticals(this, value)
		this.criticals := value;
	endfunction

	function AddHits(this, value)
		this.hits += CInt(value);
	endfunction

	function AddKills(this, value)
		this.kills += CInt(value);
	endfunction

	function AddDamage(this, value)
		this.damage += CInt(value);
	endfunction

	function AddCriticals(this, value)
		this.criticals += CInt(value);
	endfunction
	
	function Serialize(this)
		var base_data := Achievement::Serialize(this);
		base_data.+hits := this.hits;
		base_data.+kills := this.kills;
		base_data.+damage := this.damage;
		base_data.+criticals := this.criticals;

		return base_data;
	endfunction
	
	function Deserialize(this, data)
		var result := Achievement::Deserialize(this, data);
		if (!data.hits || !data.kills || !data.damage || !data.criticals)
			result := error{ errortext := "Achievement data is missing required fields." };
		endif
		if (result == error)
			return result;
		endif

		this.hits := CInt(data.hits);
		this.kills := CInt(data.kills);
		this.damage := CInt(data.damage);
		this.criticals := CInt(data.criticals);
	endfunction
	
	function toString(this)
		return $"Achievement para {this.name} ({this.type}) (xp={this.xp}, level={this.level}, hits={this.hits}, kills={this.kills}, damage={this.damage}, criticals={this.criticals})";
	endfunction
endclass