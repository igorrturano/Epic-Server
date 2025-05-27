include ":mastery:model/Mastery";

const MASTERY_COMBAT := MASTERY + "_COMBAT";

class MasteryCombat(Mastery)
	var hits;
	var kills;
	var damage;
<<<<<<< HEAD:pkg/systems/achievements/model/Combat.inc
	var criticals;

	function Combat(this, name)
		Super(name);
		this.type := ACHIEVEMENT_COMBAT;
		this.hits := 0;
		this.kills := 0;
		this.damage := 0;
		this.criticals := 0;
=======

	function MasteryCombat(this, name)
		this.Mastery(name);
		this.type := MASTERY_COMBAT;

		this.hits := 0;
		this.kills := 0;
		this.damage := 0;
>>>>>>> 46062fc2b49c3141dbc0d4c15d368bc03f132839:pkg/systems/mastery/model/MasteryCombat.inc
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
		this.hits := CInt(value);
	endfunction

	function setKills(this, value)
		this.kills := CInt(value);
	endfunction

	function setDamage(this, value)
		this.damage := CInt(value);
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

<<<<<<< HEAD:pkg/systems/achievements/model/Combat.inc
	function AddCriticals(this, value)
		this.criticals += CInt(value);
	endfunction
	
=======
>>>>>>> 46062fc2b49c3141dbc0d4c15d368bc03f132839:pkg/systems/mastery/model/MasteryCombat.inc
	function Serialize(this)
		var base_data := Mastery::Serialize(this);
		base_data.+hits := this.hits;
		base_data.+kills := this.kills;
		base_data.+damage := this.damage;
<<<<<<< HEAD:pkg/systems/achievements/model/Combat.inc
		base_data.+criticals := this.criticals;

=======
		
>>>>>>> 46062fc2b49c3141dbc0d4c15d368bc03f132839:pkg/systems/mastery/model/MasteryCombat.inc
		return base_data;
	endfunction

	function Deserialize(this, data)
<<<<<<< HEAD:pkg/systems/achievements/model/Combat.inc
		Achievement::Deserialize(this, data);
=======
	var result := Mastery::Deserialize(this, data);
		if (result == error)
			return result;
		endif

>>>>>>> 46062fc2b49c3141dbc0d4c15d368bc03f132839:pkg/systems/mastery/model/MasteryCombat.inc
		this.hits := CInt(data.hits);
		this.kills := CInt(data.kills);
		this.damage := CInt(data.damage);
		this.criticals := CInt(data.criticals);
	endfunction

	function toString(this)
<<<<<<< HEAD:pkg/systems/achievements/model/Combat.inc
		return $"Achievement para {this.name} ({this.type}) (xp={this.xp}, level={this.level}, hits={this.hits}, kills={this.kills}, damage={this.damage}, criticals={this.criticals})";
=======
		return $"Combat para {this.name} ({this.type}) (Level={this.level}, XP={this.xp}, Hits={this.hits}, Kills={this.kills}, Damage={this.damage})";
>>>>>>> 46062fc2b49c3141dbc0d4c15d368bc03f132839:pkg/systems/mastery/model/MasteryCombat.inc
	endfunction
endclass