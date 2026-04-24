
include "IModCollection";
include ":combat:Exceptions/_all";

class ModCollectionBase(IModCollection)

	function ModCollectionBase(this)
		this._modDict := dictionary;
	endfunction

	uninit function GetTotal(this);
	uninit function IsValidMod(this, mod);

	function InsertMod(this,byref mod)
		if(this.IsValidMod(mod))
			var ret := this._modDict.insert(mod.GetSource(), mod);
			if(!ret)
				return InvalidModTypeException(ret.errortext);
			else
				return ret;
			endif
		endif

		return InvalidModTypeException();
	endfunction

	function RemoveMod(this,byref mod)
		return this._modDict.remove(mod.GetSource(), mod);
	endfunction

	function Count(this)
		return this._modDict.size();
	endfunction

	function Contains(this, mod)
		return this._modDict.exists(mod.GetSource());
	endfunction

	function FindModBySource(this, source)
		return this._modDict[source];
	endfunction

	function FindMod(this, resistMod)
		return this._modDict[resistMod.GetSouce()];
	endfunction

endclass