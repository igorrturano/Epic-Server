include "IDamage";

class Damage(IDamage)
	var CriticalDamage := false;

	var SlashRaw := 0;
	var SlashBase := 0;
	var PiercingRaw := 0;
	var PiercingBase := 0;
	var BluntRaw := 0;
	var BluntBase := 0;
	var FireRaw := 0;
	var FireBase := 0;
	var ColdRaw := 0;
	var ColdBase := 0;
	var PoisonRaw := 0;
	var PoisonBase := 0;
	var EnergyRaw := 0;
	var EnergyBase := 0;

	function Damage(this, slash := 0, piercing := 0, blunt := 0, fire := 0, cold := 0, poison := 0, energy := 0)
		this.CriticalDamage := false;

		this.SetSlashRawDamage(slash);
		this.SetPiercingRawDamage(piercing);
		this.SetBluntRawDamage(blunt);

		this.SetFireRawDamage(fire);
		this.SetColdRawDamage(cold);
		this.SetPoisonRawDamage(poison);
		this.SetEnergyRawDamage(energy);
	endfunction

	function GetSlashRawDamage(this)
		return CInt(this.SlashRaw);
	endfunction

	function GetPiercingRawDamage(this)
		return CInt(this.PiercingRaw);
	endfunction

	function GetBluntRawDamage(this)
		return CInt(this.BluntRaw);
	endfunction

	function GetFireRawDamage(this)
		return CInt(this.FireRaw);
	endfunction

	function GetColdRawDamage(this)
		return CInt(this.ColdRaw);
	endfunction

	function GetPoisonRawDamage(this)
		return CInt(this.PoisonRaw);
	endfunction

	function GetEnergyRawDamage(this)
		return CInt(this.EnergyRaw);
	endfunction

	function GetSlashBaseDamage(this)
		return CInt(this.SlashBase);
	endfunction

	function GetPiercingBaseDamage(this)
		return CInt(this.PiercingBase);
	endfunction

	function GetBluntBaseDamage(this)
		return CInt(this.BluntBase);
	endfunction

	function GetFireBaseDamage(this)
		return CInt(this.FireBase);
	endfunction

	function GetColdBaseDamage(this)
		return CInt(this.ColdBase);
	endfunction

	function GetPoisonBaseDamage(this)
		return CInt(this.PoisonBase);
	endfunction

	function GetEnergyBaseDamage(this)
		return CInt(this.EnergyBase);
	endfunction

	function GetTotalRawDamage(this)
		return CInt(
			CInt(this.GetSlashRawDamage()) +
			CInt(this.GetPiercingRawDamage()) +
			CInt(this.GetBluntRawDamage()) +
			CInt(this.GetFireRawDamage()) +
			CInt(this.GetColdRawDamage()) +
			CInt(this.GetPoisonRawDamage()) +
			CInt(this.GetEnergyRawDamage())
		);
	endfunction

	function GetTotalBaseDamage(this)
		return CInt(
			CInt(this.GetSlashBaseDamage()) +
			CInt(this.GetPiercingBaseDamage()) +
			CInt(this.GetBluntBaseDamage()) +
			CInt(this.GetFireBaseDamage()) +
			CInt(this.GetColdBaseDamage()) +
			CInt(this.GetPoisonBaseDamage()) +
			CInt(this.GetEnergyBaseDamage())
		);
	endfunction

	function SetSlashRawDamage(this, value)
		this.SlashRaw := CInt(value);
		this.SlashBase := this.SlashRaw;
		return this;
	endfunction

	function SetPiercingRawDamage(this, value)
		this.PiercingRaw := CInt(value);
		this.PiercingBase := this.PiercingRaw;
		return this;
	endfunction

	function SetBluntRawDamage(this, value)
		this.BluntRaw := CInt(value);
		this.BluntBase := this.BluntRaw;
		return this;
	endfunction

	function SetFireRawDamage(this, value)
		this.FireRaw := CInt(value);
		this.FireBase := this.FireRaw;
		return this;
	endfunction

	function SetColdRawDamage(this, value)
		this.ColdRaw := CInt(value);
		this.ColdBase := this.ColdRaw;
		return this;
	endfunction

	function SetPoisonRawDamage(this, value)
		this.PoisonRaw := CInt(value);
		this.PoisonBase := this.PoisonRaw;
		return this;
	endfunction

	function SetEnergyRawDamage(this, value)
		this.EnergyRaw := CInt(value);
		this.EnergyBase := this.EnergyRaw;
		return this;
	endfunction

	function SetSlashBaseDamage(this, value)
		this.SlashBase := CInt(value);
		return this;
	endfunction

	function SetPiercingBaseDamage(this, value)
		this.PiercingBase := CInt(value);
		return this;
	endfunction

	function SetBluntBaseDamage(this, value)
		this.BluntBase := CInt(value);
		return this;
	endfunction

	function SetFireBaseDamage(this, value)
		this.FireBase := CInt(value);
		return this;
	endfunction

	function SetColdBaseDamage(this, value)
		this.ColdBase := CInt(value);
		return this;
	endfunction

	function SetPoisonBaseDamage(this, value)
		this.PoisonBase := CInt(value);
		return this;
	endfunction

	function SetEnergyBaseDamage(this, value)
		this.EnergyBase := CInt(value);
		return this;
	endfunction

	function SetCritical(this, isCriticalDamage)
		this.CriticalDamage := isCriticalDamage;
		return this;
	endfunction

	function IncreaseSlashRawDamage(this, value)
		this.SlashRaw += CInt(value);
		this.SlashBase += CInt(value);
		return this;
	endfunction

	function IncreasePiercingRawDamage(this, value)
		this.PiercingRaw += CInt(value);
		this.PiercingBase += CInt(value);
		return this;
	endfunction

	function IncreaseBluntRawDamage(this, value)
		this.BluntRaw += CInt(value);
		this.BluntBase += CInt(value);
		return this;
	endfunction

	function IncreaseFireRawDamage(this, value)
		this.FireRaw += CInt(value);
		this.FireBase += CInt(value);
		return this;
	endfunction

	function IncreaseColdRawDamage(this, value)
		this.ColdRaw += CInt(value);
		this.ColdBase += CInt(value);
		return this;
	endfunction

	function IncreasePoisonRawDamage(this, value)
		this.PoisonRaw += CInt(value);
		this.PoisonBase += CInt(value);
		return this;
	endfunction

	function IncreaseEnergyRawDamage(this, value)
		this.EnergyRaw += CInt(value);
		this.EnergyBase += CInt(value);
		return this;
	endfunction

	function IncreaseSlashBaseDamage(this, value)
		this.SlashBase += CInt(value);
		return this;
	endfunction

	function IncreasePiercingBaseDamage(this, value)
		this.PiercingBase += CInt(value);
		return this;
	endfunction

	function IncreaseBluntBaseDamage(this, value)
		this.BluntBase += CInt(value);
		return this;
	endfunction

	function IncreaseFireBaseDamage(this, value)
		this.FireBase += CInt(value);
		return this;
	endfunction

	function IncreaseColdBaseDamage(this, value)
		this.ColdBase += CInt(value);
		return this;
	endfunction

	function IncreasePoisonBaseDamage(this, value)
		this.PoisonBase += CInt(value);
		return this;
	endfunction

	function IncreaseEnergyBaseDamage(this, value)
		this.EnergyBase += CInt(value);
		return this;
	endfunction

	function DecreaseSlashRawDamage(this, value)
		this.SlashRaw -= CInt(value);
		this.SlashBase -= CInt(value);
		return this;
	endfunction

	function DecreasePiercingRawDamage(this, value)
		this.PiercingRaw -= CInt(value);
		this.PiercingBase -= CInt(value);
		return this;
	endfunction

	function DecreaseBluntRawDamage(this, value)
		this.BluntRaw -= CInt(value);
		this.BluntBase -= CInt(value);
		return this;
	endfunction

	function DecreaseFireRawDamage(this, value)
		this.FireRaw -= CInt(value);
		this.FireBase -= CInt(value);
		return this;
	endfunction

	function DecreaseColdRawDamage(this, value)
		this.ColdRaw -= CInt(value);
		this.ColdBase -= CInt(value);
		return this;
	endfunction

	function DecreasePoisonRawDamage(this, value)
		this.PoisonRaw -= CInt(value);
		this.PoisonBase -= CInt(value);
		return this;
	endfunction

	function DecreaseEnergyRawDamage(this, value)
		this.EnergyRaw -= CInt(value);
		this.EnergyBase -= CInt(value);
		return this;
	endfunction

	function DecreaseSlashBaseDamage(this, value)
		this.SlashBase -= CInt(value);
		return this;
	endfunction

	function DecreasePiercingBaseDamage(this, value)
		this.PiercingBase -= CInt(value);
		return this;
	endfunction

	function DecreaseBluntBaseDamage(this, value)
		this.BluntBase -= CInt(value);
		return this;
	endfunction

	function DecreaseFireBaseDamage(this, value)
		this.FireBase -= CInt(value);
		return this;
	endfunction

	function DecreaseColdBaseDamage(this, value)
		this.ColdBase -= CInt(value);
		return this;
	endfunction

	function DecreasePoisonBaseDamage(this, value)
		this.PoisonBase -= CInt(value);
		return this;
	endfunction

	function DecreaseEnergyBaseDamage(this, value)
		this.EnergyBase -= CInt(value);
		return this;
	endfunction

	// Reducoes multiplicativas por % (0-100). Aplicam Base = Base * (100 - pct) / 100.
	// Usadas no novo pipeline de armor % (ver design_combate.md). Layers que ainda
	// trabalham com flat (manashield, abilities de absorcao pre-Phase 7) continuam
	// usando os Decrease*BaseDamage acima.
	function DecreaseSlashPercentDamage(this, pct)
		this.SlashBase := CInt(this.SlashBase * (100 - CInt(pct)) / 100);
		return this;
	endfunction

	function DecreasePiercingPercentDamage(this, pct)
		this.PiercingBase := CInt(this.PiercingBase * (100 - CInt(pct)) / 100);
		return this;
	endfunction

	function DecreaseBluntPercentDamage(this, pct)
		this.BluntBase := CInt(this.BluntBase * (100 - CInt(pct)) / 100);
		return this;
	endfunction

	function DecreaseFirePercentDamage(this, pct)
		this.FireBase := CInt(this.FireBase * (100 - CInt(pct)) / 100);
		return this;
	endfunction

	function DecreaseColdPercentDamage(this, pct)
		this.ColdBase := CInt(this.ColdBase * (100 - CInt(pct)) / 100);
		return this;
	endfunction

	function DecreasePoisonPercentDamage(this, pct)
		this.PoisonBase := CInt(this.PoisonBase * (100 - CInt(pct)) / 100);
		return this;
	endfunction

	function DecreaseEnergyPercentDamage(this, pct)
		this.EnergyBase := CInt(this.EnergyBase * (100 - CInt(pct)) / 100);
		return this;
	endfunction

	function IsCritical(this)
		return this.CriticalDamage;
	endfunction

	function ToString(this)
		return "Damage" + (this.IsCritical() ? " (CRITICAL)" : "") + ": " +
			$"Slash: [Raw={this.GetSlashRawDamage()}, Base={this.GetSlashBaseDamage()}], " +
			$"Piercing: [Raw={this.GetPiercingRawDamage()}, Base={this.GetPiercingBaseDamage()}], " +
			$"Blunt: [Raw={this.GetBluntRawDamage()}, Base={this.GetBluntBaseDamage()}], " +
			$"Fire: [Raw={this.GetFireRawDamage()}, Base={this.GetFireBaseDamage()}], " +
			$"Cold: [Raw={this.GetColdRawDamage()}, Base={this.GetColdBaseDamage()}], " +
			$"Poison: [Raw={this.GetPoisonRawDamage()}, Base={this.GetPoisonBaseDamage()}], " +
			$"Energy: [Raw={this.GetEnergyRawDamage()}, Base={this.GetEnergyBaseDamage()}]";
	endfunction
endclass

enum class DAMAGE_TYPE
	SLASH := "Cortante",
	PIERCING := "Perfurante",
	BLUNT := "Contusivo",

	FIRE := "Fogo",
	COLD := "Gelo",
	POISON := "Veneno",
	ENERGY := "Energia"
endenum

enum class STANCE_TYPE
	NEUTRAL := "Neutra",
	AGGRESSIVE := "Agressiva",
	VERY_AGGRESSIVE := "Muito Agressiva",
	DEFENSIVE := "Defensiva",
	VERY_DEFENSIVE := "Muito Defensiva"
endenum