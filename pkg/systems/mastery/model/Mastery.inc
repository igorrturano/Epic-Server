const MASTERY := "MASTERY";

class Mastery()
	var type;
	var name;
	var level;
	var xp;

	function Mastery(this, name)
		this.type := MASTERY;

		this.name := CStr(name);
		this.level := 0;
		this.xp := 0;
	endfunction

	function getType(this)
		return this.type;
	endfunction

	function getName(this)
		return this.name;
	endfunction

	function getLevel(this)
		return this.level;
	endfunction

	function getXP(this)
		return this.xp;
	endfunction

	function setName(this, value)
		this.name := value;
	endfunction
	
	function setLevel(this, value)
		this.level := value;
	endfunction
	
	function setXP(this, value)
		this.xp := value;
	endfunction

	function AddXP(this, value)
		this.xp += CInt(value);
	endfunction

	function AddLevel(this, value)
		this.level += CInt(value);
	endfunction

	function Serialize(this)
		return struct{
			type := this.type,
			name := this.name,
			xp := this.xp,
			level := this.level
		};
	endfunction

	function Deserialize(this, data)
		if (!data) return error{ errortext := "Invalid data for MasteryCombat deserialization." }; endif
		this.name := data.name;
		this.type := data.type;
		this.xp := CInt(data.xp);
		this.level := CInt(data.level);
	endfunction
	

	function toString(this)
		return $"Maestria {this.name} ({this.type}) (Level={this.level}, XP={this.xp})";
	endfunction
endclass