include "IMastery";

class Mastery(IMastery)
	var type;
	var name;
	var level;
	var xp;

	function Mastery(this, name)
		this.name := CStr(name);
		this.level := CInt(0);
		this.xp := CInt(0);
	endfunction

	function GetType(this)
		return this.type;
	endfunction

	function SetType(this, value)
		this.type := value;
	endfunction

	function GetName(this)
		return this.name;
	endfunction

	function _SetName(this, value)
		this.name := CStr(value);
	endfunction

	function GetLevel(this)
		return this.level;
	endfunction

	function SetLevel(this, value)
		this.level := CInt(value);
	endfunction

	function AddLevel(this, value)
		this.level += CInt(value);
	endfunction

	function GetXP(this)
		return this.xp;
	endfunction

	function SetXP(this, value)
		this.xp := CInt(value);
	endfunction

	function AddXP(this, value)
		this.xp += CInt(value);
	endfunction

endclass

enum class MASTERY_TYPE
	COMBAT,
	WORKING
endenum