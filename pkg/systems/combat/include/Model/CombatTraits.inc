include "Damage";

class CombatTraits()
	var _posture := "";

	function CombatTraits(this, serializedData := false)
		if (serializedData)
			this.Deserialize(serializedData);
		else
			this._posture := DAMAGE_TYPE::BLUNT;
		endif
	endfunction

	function GetPosture(this)
		return this._posture;
	endfunction

	function SetPosture(this, posture) // posture: enum DAMAGE_TYPE
		this._posture := posture;
		return this;
	endfunction

	function Serialize(this)
		return struct{
			posture := this._posture
		};
	endfunction

	function Deserialize(this, dataStruct)
		if (TypeOfInt(dataStruct) != OT_STRUCT)
			return error{errortext := "The param must be a Struct of a serialized CombatTraits!"};
		else
			if (!dataStruct.posture)
				return error{errortext := "Posture not found in data! Verify the data structure passed."};
			endif
		endif

		this._posture := dataStruct.posture;
		return this;
	endfunction

	function ToString(this)
		return "CombatTraits: Posture=" + this._posture;
	endfunction
endclass
