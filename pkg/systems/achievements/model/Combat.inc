include ":achievements:model/Achievement";

const ACHIEVEMENT_COMBAT := "ACHIEVEMENT_COMBAT";

class Combat(Achievement)
	var hits;
	var kills;
	var damage;
	
	function Combat(this, name, xp, level, hits, kills, damage)
		Super(name, xp, level);
		this.type := ACHIEVEMENT_COMBAT;
		this.hits := CInt(hits);
		this.kills := CInt(kills);
		this.damage := CInt(damage);
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

	function setHits(this, value)
		this.hits := value;
	endfunction

	function setKills(this, value)
		this.kills := value;
	endfunction

	function setDamage(this, value)
		this.damage := value;
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
	
	function Serialize(this)
		var base_data := Achievement::Serialize(this);
		base_data.+hits := this.hits;
		base_data.+kills := this.kills;
		base_data.+damage := this.damage;

		return base_data;
	endfunction
	
	function Deserialize(this, data)
		var result := Achievement::Deserialize(this, data);
		if (result == error)
			return result;
		endif

		this.hits := CInt(data.hits);
		this.kills := CInt(data.kills);
		this.damage := CInt(data.damage);
	endfunction
	
	function toString(this)
		return $"Combat Achievement para {this.name} ({this.type}) (xp={this.xp}, level={this.level}, hits={this.hits}, kills={this.kills}, damage={this.damage})";
	endfunction
endclass