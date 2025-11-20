use uo;

include "IPositionWrapper";

class PositionWrapper(IPositionWrapper)
	var _x := 0;
	var _y := 0;
	var _z := 0;
	var _realm := "";

	function PositionWrapper(this, x := 0, y := 0, z := 0, realm := _DEFAULT_REALM)
		this._x := CInt(x);
		this._y := CInt(y);
		this._z := CInt(z);
		this._realm := CStr(realm);
	endfunction

	function GetX(this)
		return this._x;
	endfunction

	function GetY(this)
		return this._y;
	endfunction

	function GetZ(this)
		return this._z;
	endfunction

	function GetRealm(this)
		return this._realm;
	endfunction

	function SetPosition(this, x, y, z, realm := _DEFAULT_REALM)
		this._x := x;
		this._y := y;
		this._z := z;
		this._realm := realm;
	endfunction
	
	function SetX(this, x)
		this._x := CInt(x);
	endfunction

	function SetY(this, y)
		this._y := CInt(y);
	endfunction

	function SetZ(this, z)
		this._z := CInt(z);
	endfunction

	function SetRealm(this, realm)
		this._realm := CStr(realm);
	endfunction

	function PlayEffect(this, effectId, speed := 10, duration := 0, hue := 0, render := 0, effect3d := 0)
		return PlayStationaryEffectEx(this._x, this._y, this._z, this._realm, effectId, speed, duration, hue, render, effect3d);
	endfunction

	function PlayEffectTo(this, targetPosition, effectId, speed := 10, duration := 0, hue := 0, render := 0, fixeddirection := 0, explode := 0, effect3d := 0, effect3dexplode := 0, effect3dsound := 0)
		return PlayMovingEffectXYZEx(
			this._x, this._y, this._z,
			targetPosition.GetX(), targetPosition.GetY(), targetPosition.GetZ(), targetPosition.GetRealm(),
			effectId, speed, duration, hue, render, fixeddirection, explode, effect3d, effect3dexplode, effect3dsound
		);
	endfunction
endclass
