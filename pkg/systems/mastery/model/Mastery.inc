const MASTERY := "MASTERY";

class Mastery()
	var type;
	var key;
	var name;
	var level;
	var xp;

	function Mastery(this, key)
		this.type := MASTERY;
		this.key := key;

		this.name := "";
		this.level := 0;
		this.xp := 0;
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

	function GetLevel(this)
		return this.level;
	endfunction

	function GetXP(this)
		return this.xp;
	endfunction

	function _SetName(this, value)
		this.name := CStr(value);
	endfunction

	function SetLevel(this, value)
		this.level := CInt(value);
	endfunction

	function SetXP(this, value)
		this.xp := CInt(value);
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
			level := this.level
		};
	endfunction

	function Deserialize(this, data)
		if (!data) return error{ errortext := "Invalid data for MasteryCombat deserialization." }; endif
		this.name := CStr(data.name);
		this.key := data.key;
		this.type := data.type;
		this.xp := CInt(data.xp);
		this.level := CInt(data.level);
	endfunction
	

	function toString(this)
		return $"Maestria {this.key} (Name={this.name}, Type={this.type}, Level={this.level}, XP={this.xp})";
	endfunction
endclass