
use os;

include "ModCollectionBase";
include "ArmorResist";

class ResistModCollection(ModCollectionBase)

	function ResistModCollection(this)
		Super();
	endfunction

	function IsValidMod(this, mod)
		return mod.GetMod() is @ArmorResist; //Queria colocar DamageModBase, mas parece ter algum bug no pol na versão atual.
	endfunction

	function GetTotal(this)

		var totalResist := ArmorResist();
		
		foreach resistMod in (this._modDict)
			SleepMS(2);
			totalResist.SetSlashResist( resistMod.GetMod().GetSlashResist() + totalResist.GetSlashResist());
			totalResist.SetPiercingResist( resistMod.GetMod().GetPiercingResist() + totalResist.GetPiercingResist());
			totalResist.SetBluntResist( resistMod.GetMod().GetBluntResist() + totalResist.GetBluntResist());
			totalResist.SetFireResist( resistMod.GetMod().GetFireResist() + totalResist.GetFireResist());
			totalResist.SetColdResist( resistMod.GetMod().GetColdResist() + totalResist.GetColdResist());
			totalResist.SetPoisonResist( resistMod.GetMod().GetPoisonResist() + totalResist.GetPoisonResist());
			totalResist.SetEnergyResist( resistMod.GetMod().GetEnergyResist() + totalResist.GetEnergyResist());
		endforeach

		return totalResist;
	endfunction 
endclass