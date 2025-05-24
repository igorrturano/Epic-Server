class Achievement()
	var xp;
	var level;
	var hits;
	var kills;
	var damage;

	function Achievement(this, xp, level, hits, kills, damage)
		// Inicializa os atributos da conquista
		this.xp := CInt(xp);
		this.level := CInt(level);
		this.hits := CInt(hits);
		this.kills := CInt(kills);
		this.damage := CInt(damage);
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
			"xp" := this.xp,
			"level" := this.level,
			"hits" := this.hits,
			"kills" := this.kills,
			"damage" := this.damage
		};
	endfunction

	function toString(this)
		return $"Achievement(xp={this.xp}, level={this.level}, hits={this.hits}, kills={this.kills}, damage={this.damage})";
	endfunction
endclass