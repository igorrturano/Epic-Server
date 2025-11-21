include ":io:CFGFile";

include ":wrappers:Objects/Model/ItemWrapper";

class Ammo()
	var _item; // ItemWrapper object
	var _name := "";
	var _anim := 0;
	var _sound := 0;

	function Ammo(this, objType)
		this._item := ItemWrapper(objType);
		this._anim := this._item.GetTemplate().Anim;
		this._sound := this._item.GetTemplate().Sound;
	endfunction

	function GetObjType(this)
		return this._item.GetObjType();
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

	function NeedsQuiver(this)
		return this._objType != 0x5001;
	endfunction

	function ToString(this)
		return $"Ammo: {this._name} (ObjType: {CStr(Hex(this._objType))}, Anim: {CStr(Hex(this._anim))}, Sound: {CStr(Hex(this._sound))})";
	endfunction
endclass