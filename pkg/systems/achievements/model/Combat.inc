include ":achievements:model/Achievement";

const ACHIEVEMENT_COMBAT := ACHIEVEMENT + "_COMBAT";

class Combat(Achievement)
	var hits;
	var blocked;
	var kills;
	var damage;
	var damageTaken;
	var damageMax;
	var damageMaxTaken;
	var criticals;
	var criticalDamage;
	var criticalDamageMax;
	var manaSpent;

	function Combat(this, name)
		Super(name);
		this.type := ACHIEVEMENT_COMBAT;
		this.hits := 0;
		this.blocked := 0;
		this.kills := 0;
		this.damage := 0;
		this.damageTaken := 0;
		this.damageMax := 0;
		this.damageMaxTaken := 0;
		this.criticals := 0;
		this.criticalDamage := 0;
		this.criticalDamageMax := 0;
		this.manaSpent := 0;
	endfunction

	function getHits(this)
		return this.hits;
	endfunction

	function getBlocked(this)
		return this.blocked;
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

	function AddHits(this, value)
		this.hits += CInt(value);
	endfunction

	function AddBlocked(this, value)
		this.blocked += CInt(value);
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
		Achievement::Deserialize(this, data);
		this.hits := CInt(data.hits);
		this.kills := CInt(data.kills);
		this.damage := CInt(data.damage);
		this.criticals := CInt(data.criticals);
	endfunction
	
	function toString(this)
		return $"Achievement para {this.name} ({this.type}) (xp={this.xp}, level={this.level}, hits={this.hits}, kills={this.kills}, damage={this.damage}, criticals={this.criticals})";
	endfunction
endclass