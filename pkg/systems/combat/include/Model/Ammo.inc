include ":io:CFGFile";

include ":wrappers:Objects/Model/ItemWrapper";

class Ammo()
	var Item; // ItemWrapper object
	var Anim := 0;
	var Sound := 0;

	function Ammo(this, object)
		this.Item := ItemWrapper(object); // TODO: Ver se recebe itemRef, serial ou template pra não armazenar o wrapper nested
		this.Anim := this.Item.GetTemplate().Anim;
		this.Sound := this.Item.GetTemplate().Sound;
	endfunction

	function GetItem(this)
		return this.Item;
	endfunction

	function GetAnim(this)
		return this.Anim;
	endfunction

	function GetSound(this)
		return this.Sound;
	endfunction

	function NeedsQuiver(this)
		var ammoType := array{0xf3f, 0x1bfb};
		return this.GetItem().GetObjType() in ammoType;
	endfunction

	function NeedsBandolier(this)
		var ammoType := array{0x9090};
		return this.GetItem().GetObjType() in ammoType;
	endfunction

	function ToString(this)
		return $"Ammo: {this.GetItem().GetName()} (ObjType: {CStr(Hex(this.GetItem().GetObjType()))}, Anim: {CStr(Hex(this.GetAnim()))}, Sound: {CStr(Hex(this.GetSound()))}, NeedsQuiver: {CStr(this.NeedQuiver)})";
	endfunction
endclass