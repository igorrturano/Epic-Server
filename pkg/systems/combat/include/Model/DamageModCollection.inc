
use os;

include "ModCollectionBase";
include "Damage";

class DamageModCollection(ModCollectionBase)

	function DamageModCollection(this)
		Super();
	endfunction

	function IsValidMod(this, mod)
		return mod.GetMod() is @Damage; //Queria colocar DamageModBase, mas parece ter algum bug no pol na versão atual.
	endfunction

	function GetTotal(this)

		var totalDamage := Damage();

		foreach damageMod in (this._modDict)
			SleepMS(2);
			totalDamage.SetSlashRawDamage( damageMod.GetMod().GetSlashRawDamage() + totalDamage.GetSlashRawDamage());
			totalDamage.SetPiercingRawDamage( damageMod.GetMod().GetPiercingRawDamage() + totalDamage.GetPiercingRawDamage());
			totalDamage.SetBluntRawDamage( damageMod.GetMod().GetBluntRawDamage() + totalDamage.GetBluntRawDamage());
			totalDamage.SetFireRawDamage( damageMod.GetMod().GetFireRawDamage() + totalDamage.GetFireRawDamage());
			totalDamage.SetColdRawDamage( damageMod.GetMod().GetColdRawDamage() + totalDamage.GetColdRawDamage());
			totalDamage.SetPoisonRawDamage( damageMod.GetMod().GetPoisonRawDamage() + totalDamage.GetPoisonRawDamage());
			totalDamage.SetEnergyRawDamage( damageMod.GetMod().GetEnergyRawDamage() + totalDamage.GetEnergyRawDamage());
		endforeach

		return totalDamage;
	endfunction 
endclass