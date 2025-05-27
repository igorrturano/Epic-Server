include ":mastery:model/Mastery";

const MASTERY_COMBAT := MASTERY + "_COMBAT";

class MasteryCombat(Mastery)
	var hits;
	var kills;
	var damage;

	function MasteryCombat(this, name)
		this.Mastery(name);
		this.type := MASTERY_COMBAT;

		this.hits := 0;
		this.kills := 0;
		this.damage := 0;
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
		this.hits := CInt(value);
	endfunction

	function setKills(this, value)
		this.kills := CInt(value);
	endfunction

	function setDamage(this, value)
		this.damage := CInt(value);
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
		var base_data := Mastery::Serialize(this);
		base_data.+hits := this.hits;
		base_data.+kills := this.kills;
		base_data.+damage := this.damage;
		
		return base_data;
	endfunction

	function Deserialize(this, data)
	var result := Mastery::Deserialize(this, data);
		if (result == error)
			return result;
		endif

		this.hits := CInt(data.hits);
		this.kills := CInt(data.kills);
		this.damage := CInt(data.damage);
	endfunction

	function toString(this)
		return $"Combat para {this.name} ({this.type}) (Level={this.level}, XP={this.xp}, Hits={this.hits}, Kills={this.kills}, Damage={this.damage})";
	endfunction
endclass