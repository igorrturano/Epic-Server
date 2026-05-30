include "Damage";

class CombatTraits()
	var _posture := "";
	var _stance := "";

	function CombatTraits(this, serializedData := false)
		if (serializedData)
			this.Deserialize(serializedData);
		else
			this._posture := DAMAGE_TYPE::BLUNT;
			this._stance := STANCE_TYPE::NEUTRAL;
		endif
	endfunction

	function GetPosture(this)
		return this._posture;
	endfunction

	function GetStance(this)
		return this._stance;
	endfunction

	function SetPosture(this, posture) // posture: enum DAMAGE_TYPE
		this._posture := posture;
		return this;
	endfunction

	function SetStance(this, stance) // stance: enum STANCE_TYPE
		this._stance := stance;
		return this;
	endfunction

	function Serialize(this)
		return struct{
			posture := this._posture,
			stance := this._stance
		};
	endfunction

	function Deserialize(this, dataStruct)
		if (TypeOfInt(dataStruct) != OT_STRUCT)
			return error{errortext := "The param must be a Struct of a serialized CombatTraits!"};
		endif

		// Backwards compatibility: provide defaults for missing fields
		this._posture := dataStruct.posture ? dataStruct.posture : DAMAGE_TYPE::BLUNT;
		this._stance := dataStruct.stance ? dataStruct.stance : STANCE_TYPE::NEUTRAL;

		return this;
	endfunction

	function ToString(this)
		return "CombatTraits: Posture=" + this._posture + ", Stance=" + this._stance;
	endfunction
endclass
