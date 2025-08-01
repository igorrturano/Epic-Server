const ACHIEVEMENT := "ACHIEVEMENT";

class Achievement()
	var type; // Readonly
	var key; // Readonly

	var name;
	var xp;
	var level;

	function Achievement(this, key)
		this.type := ACHIEVEMENT;
		this.key := key;

		this.name := CStr(key);
		this.xp := 0;
		this.level := 0;
	endfunction

	function GetType(this)
		return this.type;
	endfunction

	function GetKey(this)
		return this.key;
	endfunction

	function GetName(this)
		return this.name;
	endfunction

	function GetXP(this)
		return this.xp;
	endfunction

	function GetLevel(this)
		return this.level;
	endfunction


	function _SetName(this, value)
		this.name := value;
	endfunction

	function SetXP(this, value)
		this.xp := value;
	endfunction

	function SetLevel(this, value)
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
			type := this.type,
			key := this.key,

			name := this.name,
			xp := this.xp,
			damage := this.damage
		};
	endfunction

	function Deserialize(this, data)
		if (!data) return error{ errortext := "Invalid data for Achievement deserialization." }; endif
		this.type := data.type;
		this.key := data.key;

		this.name := data.name;
		this.xp := CInt(data.xp);
		this.level := CInt(data.level);
	endfunction

	function toString(this)
		return $"Achievement para {this.name} ({this.type}) (xp={this.xp}, level={this.level})";
	endfunction
endclass