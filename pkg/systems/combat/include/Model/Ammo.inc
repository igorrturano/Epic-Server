include ":io:CFGFile";

include ":wrappers:Objects/Model/ItemWrapper";

class Ammo(ItemWrapper)
	var _anim := 0;
	var _sound := 0;

	function Ammo(this, objType)
		var cfg := CFGFile("*", "itemdesc").SetElementName(objType);
		
		this._objType := CInt(objType);
		this._name := CStr(cfg.SetKeyName("desc").GetKeyValue());
		this._anim := CInt(cfg.SetKeyName("Anim").GetKeyValue());
		this._sound := CInt(cfg.SetKeyName("Sound").GetKeyValue());
	endfunction

	function GetObjType(this)
		return this._objType;
	endfunction

	function GetName(this)
		return this._name;
	endfunction

	function GetAnim(this)
		return this._anim;
	endfunction

	function GetSound(this)
		return this._sound;
	endfunction

	function ToString(this)
		return $"Ammo: {this._name} (ObjType: {CStr(Hex(this._objType))}, Anim: {CStr(Hex(this._anim))}, Sound: {CStr(Hex(this._sound))})";
	endfunction
endclass