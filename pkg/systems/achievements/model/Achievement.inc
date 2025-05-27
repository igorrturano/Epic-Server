const ACHIEVEMENT := "ACHIEVEMENT";

class Achievement()
	var type;
	var name;
	var xp;
	var level;

	function Achievement(this, name)
		this.type := ACHIEVEMENT;

		this.name := CStr(name);
		this.xp := 0;
		this.level := 0;
	endfunction

	function getName(this)
		return this.name;
	endfunction

	function getType(this)
		return this.type;
	endfunction

	function getXp(this)
		return this.xp;
	endfunction

	function getLevel(this)
		return this.level;
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

	function AddXP(this, value)
		this.xp += CInt(value);
	endfunction
	
	function AddLevel(this, value)
		this.level += CInt(value);
	endfunction

	function Serialize(this)
		return struct{
			name := this.name,
			type := this.type,
			xp := this.xp,
			damage := this.damage
		};
	endfunction

	function Deserialize(this, data)
		if (!data) return error{ errortext := "Invalid data for Achievement deserialization." }; endif
		this.name := data.name;
		this.type := data.type;
		this.xp := CInt(data.xp);
		this.level := CInt(data.level);
	endfunction

	function toString(this)
		return $"Achievement para {this.name} ({this.type}) (xp={this.xp}, level={this.level})";
	endfunction
endclass