use uo;

include "IUObjectWrapper";

include "include/packets";

class UObjectWrapper(IUObjectWrapper)
	var _objRef; // ObjRef
	var _serial := 0x0;
	var _objType := 0x0;
	var _name := "";
	var _graphic := 0;
	var _color := 0;

	function UObjectWrapper(this, byRef objRef)
		this._objRef := objRef;
		this._serial := objRef.serial;
		this._objType := objRef.objtype;
		this._name := objRef.name;
		this._graphic := objRef.graphic;
		this._color := objRef.color;
	endfunction

	function GetObjRef(this)
		return this._objRef;
	endfunction

	function GetSerial(this)
		return this._serial;
	endfunction

	function GetObjType(this)
		return this._objType;
	endfunction

	function GetName(this)
		return this._name;
	endfunction

	function GetGraphic(this)
		return this._graphic;
	endfunction

	function _SetName(this, name)
		this._name := name;
		this._objRef.name := name;
		return this;
	endfunction

	function SetGraphic(this, graphic)
		this._graphic := graphic;
		this._objRef.graphic := graphic;
		return this;
	endfunction

	function SetColor(this, color)
		this._color := color;
		this._objRef.color := color;
		return this;
	endfunction

	function GetPosition(this)
		return struct{
			x := this._objRef.x,
			y := this._objRef.y,
			z := this._objRef.z,
			realm := this._objRef.realm
		};
	endfunction

	function MoveToLocation(this, x, y, z, realm := _DEFAULT_REALM, flags := MOVEOBJECT_NORMAL)
		return MoveObjectToLocation(this._objRef, x, y, z, realm, flags);
	endfunction

	function PlaySound(this, soundId)
		PlaySoundEffect(this._objRef, soundId);

		return this;
	endfunction

	function PlaySoundPrivate(this, soundId, soundSource := false)
		PlaySoundEffectPrivate(this._objRef, soundId, soundSource ?: this._objRef);

		return this;
	endfunction

	// Feito com Wrappeds para poder customizar futuramente
	function PlayEffectTo(this, targetWrapped, effectId, speed, loop := 0, explode := 0, color := 0, translucent := 0)
		PlayMovingEffectXYZHued(this._objRef, targetWrapped.GetObjRef(), effectId, speed, loop, explode, color, translucent);

		return this;
	endfunction

	function ToString(this)
		return "UObject: " +
			$"Serial: {Hex(this._serial)}, " +
			$"ObjType: {Hex(this._objType)}, " +
			$"Name: {this._name}, " +
			$"Graphic: {Hex(this._graphic)}, " +
			$"Color: {Hex(this._color)}";
	endfunction
endclass
