include ":io:CFGFile";

include ":wrappers:Objects/Model/ItemWrapper";

class Ammo()
	var _item; // ItemWrapper object
	var _anim := 0;
	var _sound := 0;
	var _needQuiver := false;

	function Ammo(this, object)
		this._item := ItemWrapper(object);
		this._anim := this._item.GetTemplate().Anim;
		this._sound := this._item.GetTemplate().Sound;
		this._needQuiver := this.NeedsQuiver();
	endfunction

	function GetItem(this)
		return this._item;
	endfunction

	function GetAnim(this)
		return this._anim;
	endfunction

	function GetSound(this)
		return this._sound;
	endfunction

	function NeedsQuiver(this)
		return this._item.GetObjType() != 0x5001;
	endfunction

	function ToString(this)
		return $"Ammo: {this._item.GetName()} (ObjType: {CStr(Hex(this._item.GetObjType()))}, Anim: {CStr(Hex(this._anim))}, Sound: {CStr(Hex(this._sound))}, NeedsQuiver: {CStr(this._needQuiver)})";
	endfunction
endclass