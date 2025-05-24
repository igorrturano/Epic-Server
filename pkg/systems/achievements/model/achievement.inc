class Achievement()
	var name;
	var xp;
	var level;
	var hits;
	var kills;
	var damage;

	function Achievement(this, name, xp, level, hits, kills, damage)
		// Inicializa os atributos da conquista
		this.name := CStr(name);
		this.xp := CInt(xp);
		this.level := CInt(level);
		this.hits := CInt(hits);
		this.kills := CInt(kills);
		this.damage := CInt(damage);
	endfunction

	function getName(this)
		return this.name;
	endfunction

	function getXp(this)
		return this.xp;
	endfunction

	function getLevel(this)
		return this.level;
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

	function setName(this, value)
		this.name := value;
	endfunction

	function setXp(this, value)
		this.xp := value;
	endfunction

	function setLevel(this, value)
		this.level := value;
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

	function Serialize(this)
		return struct{
			name := this.name,
			xp := this.xp,
			level := this.level,
			hits := this.hits,
			kills := this.kills,
			damage := this.damage
		};
	endfunction

	function Deserialize(this, data)
		if (!data)
			return error{ errortext := "Achievement data is null or undefined." };
		endif

		this.name := data.name;
		this.xp := CInt(data.xp);
		this.level := CInt(data.level);
		this.hits := CInt(data.hits);
		this.kills := CInt(data.kills);
		this.damage := CInt(data.damage);
	endfunction

	function toString(this)
		return $"Achievement para {this.name} (xp={this.xp}, level={this.level}, hits={this.hits}, kills={this.kills}, damage={this.damage})";
	endfunction
endclass