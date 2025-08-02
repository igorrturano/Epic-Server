const ACHIEVEMENT := "ACHIEVEMENT";

class Achievement()
	var type; // Readonly
	var key; // Readonly

	var name;
	var xp;
	var level;

	var achievData; // Dictionary para armazenar informações

	function Achievement(this, key)
		this.type := ACHIEVEMENT;
		this.key := key;

		this.name := CStr(key);
		this.xp := 0;
		this.level := 0;

		this.achievData := dictionary{};
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

	function GetData(this, name)
		return this.achievData[name];
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

	function SetData(this, name, value)
		this.achievData[name] := value;
	endfunction

	function AddXP(this, value)
		this.xp += CInt(value);
	endfunction
	
	function AddLevel(this)
		this.level++;
	endfunction

	function AddData(this, name, value)
		if (!this.achievData[name]) this.SetData(name, 0); endif
		this.achievData[name] += CInt(value);
	endfunction

	function Serialize(this)
		return struct{
			type := this.type,
			key := this.key,

			name := this.name,
			xp := this.xp,
			damage := this.damage,

			achievData := this.achievData
		};
	endfunction

	function Deserialize(this, data)
		if (!data) return error{ errortext := "Invalid data for Achievement deserialization." }; endif
		this.type := data.type;
		this.key := data.key;

		this.name := data.name;
		this.xp := CInt(data.xp);
		this.level := CInt(data.level);

		this.achievData := data.achievData ?: dictionary{};
	endfunction

	function toString(this)
		return $"Achievement para {this.name} ({this.type}) (xp={this.xp}, level={this.level})";
	endfunction
endclass