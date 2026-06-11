use uo;

include ":abilities:models/CombatAbility";
include ":attributes:attributes";
include ":timedScripts:timedScripts";
include ":hotbar:custombuff";
include ":message:model/Message";
include ":debug:Controller/DebugController";

/**
 * Hiperestimulo (Hyperstimulus) - ABILITY_ID::HIPERESTIMULO (900)
 *
 * Active (BEFORE_DAMAGE). Injects a chemical cocktail: pays 15% of current HP,
 * snapshots the three vicios, and turns each vicio's "missing" amount into a
 * FIXED combat bonus for the whole duration (low vicio = more saturated =
 * stronger). All three vicios are then consumed (set to 0).
 *
 *   damage bonus  = (Cap - VicioSTR) / 15   (slash + pierce + blunt, per swing)
 *   swing speed   = (Cap - VicioDEX) / 25   tiers -> swing_speed_mod
 *   hp/mana regen = (Cap - VicioINT) / 20   points -> regen rate mod
 *   duration      = Science/5 + avg(vicios)/15   (pre-consume, capped 30s)
 *
 * Cap is the vicio maximum (100, or 150 with Blindagem Quimica). Bonuses and
 * the duration are cached as deltas on CProps so OnDeactivate reverts EXACTLY
 * what it applied and the duration survives the post-OnActivate recompute in
 * AbilityService.Activate (which runs after the vicios are already zeroed).
 * Abstinencia is suppressed while #hiperestimulo_active is set; Letargia (10s
 * no-run) fires on deactivate.
 */

// swing_speed_mod points granted per DEX speed tier (tunable).
const HIPER_SWING_PER_TIER := 5;
// regen rate is stored in hundredths, so 1 regen "point" = 100 mod units.
const HIPER_REGEN_UNIT := 100;
// minimum current HP (points) required to safely pay the 15% cost.
const HIPER_MIN_HP := 20;
// hard cap on duration (seconds).
const HIPER_MAX_DURATION := 30;

class Hyperstimulus(CombatAbility)

	function Hyperstimulus(this)
		Super(ABILITY_ID::HIPERESTIMULO);
		this.ActivationMoment := ABILITY_ACTIVATION::BEFORE_DAMAGE;
	endfunction

	function CanActivate(this, mobile)
		var who := mobile.GetObjRef();
		if (CInt(AP_GetVital(who, "hits") / 100) <= HIPER_MIN_HP)
			Message("Voce esta fraco demais para o coquetel.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif
		return true;
	endfunction

	function GetDurationFor(this, mobile)
		var who := mobile.GetObjRef();
		// Activate recomputes this AFTER OnActivate zeroes the vicios, so prefer
		// the value OnActivate cached from the pre-consume snapshot.
		var cached := GetObjProperty(who, "#hiper_duration");
		if (cached != error)
			return CInt(cached);
		endif
		return this._ComputeDuration(who);
	endfunction

	function OnActivate(this, mobile)
		var who := mobile.GetObjRef();
		var capPts := CInt(AP_GetVitalMaximumValue(who, "viciostr") / 100);

		// Duration from the pre-consume snapshot; cache it before zeroing.
		var duration := this._ComputeDuration(who);
		SetObjProperty(who, "#hiper_duration", duration);

		var vstr := this._VicioPts(who, "viciostr");
		var vdex := this._VicioPts(who, "viciodex");
		var vint := this._VicioPts(who, "vicioint");

		var dmgBonus := CInt((capPts - vstr) / 15);
		var swingMod := CInt((capPts - vdex) / 25) * HIPER_SWING_PER_TIER;
		var regenMod := CInt((capPts - vint) / 20) * HIPER_REGEN_UNIT;

		// Apply, tracking exact deltas for a clean revert.
		who.swing_speed_mod := CInt(who.swing_speed_mod) + swingMod;
		AP_SetVitalRegenRateMod(who, "hits", AP_GetVitalRegenRateMod(who, "hits") + regenMod);
		AP_SetVitalRegenRateMod(who, "mana", AP_GetVitalRegenRateMod(who, "mana") + regenMod);

		SetObjProperty(who, "#hiper_dmg", dmgBonus);
		SetObjProperty(who, "#hiper_swing", swingMod);
		SetObjProperty(who, "#hiper_regen", regenMod);

		// Consume all three vicios.
		AP_SetVital(who, "viciostr", 0);
		AP_SetVital(who, "viciodex", 0);
		AP_SetVital(who, "vicioint", 0);

		// Pay 15% of current HP. AP_GetVital is x100; ApplyRawDamage is
		// display-scale (cf. spearpush), so divide by 100 before taking 15%.
		var hpCost := CInt(AP_GetVital(who, "hits") / 100 * 0.15);
		if (hpCost > 0)
			ApplyRawDamage(who, hpCost);
		endif

		SetObjProperty(who, "#hiperestimulo_active", 1);

		SendAbilityBuff(who, ABILITY_ID::HIPERESTIMULO, duration, "Coquetel quimico: dano, velocidade e regen fixos");
		Message("O coquetel quimico arde nas suas veias.", Color(SYSTEM_COLOR::SUCCESS)).SendSys(who);
		DebugMsg(who, DEBUG_HAB, Message($"[HIPERESTIMULO] dmg+{dmgBonus} swing+{swingMod} regen+{regenMod} dur {duration}s", Color(SYSTEM_COLOR::SUCCESS)));
	endfunction

	function OnBeforeDamage(this, attackInfo)
		var dmgBonus := CInt(GetObjProperty(attackInfo.GetAttacker().GetObjRef(), "#hiper_dmg"));
		if (dmgBonus > 0)
			attackInfo.GetDamage()
				.IncreaseSlashRawDamage(dmgBonus)
				.IncreasePiercingRawDamage(dmgBonus)
				.IncreaseBluntRawDamage(dmgBonus);
		endif
		return true;
	endfunction

	// Single teardown owner. Reached on natural expiry, manual cancel, or a
	// Blindagem Quimica save aborting our monitor. Idempotent via the
	// #hiperestimulo_active guard so it can never double-revert the mods.
	function OnDeactivate(this, mobile)
		var who := mobile.GetObjRef();
		if (GetObjProperty(who, "#hiperestimulo_active") == error)
			return;
		endif

		who.swing_speed_mod := CInt(who.swing_speed_mod) - CInt(GetObjProperty(who, "#hiper_swing"));
		var regenMod := CInt(GetObjProperty(who, "#hiper_regen"));
		AP_SetVitalRegenRateMod(who, "hits", AP_GetVitalRegenRateMod(who, "hits") - regenMod);
		AP_SetVitalRegenRateMod(who, "mana", AP_GetVitalRegenRateMod(who, "mana") - regenMod);

		EraseObjProperty(who, "#hiper_dmg");
		EraseObjProperty(who, "#hiper_swing");
		EraseObjProperty(who, "#hiper_regen");
		EraseObjProperty(who, "#hiper_duration");
		EraseObjProperty(who, "#hiperestimulo_active");

		// 10s of Letargia via the shared "lentidao" slow (no-run + -DEX). Queued
		// abstinencias fire on the next vicio.src tick now that the flag is gone.
		TS_StartTimer(who, "lentidao", 10);
		Message("A quimica se esgota. Voce esta letargico.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
	endfunction

	function _ComputeDuration(this, who)
		var avg := (this._VicioPts(who, "viciostr") + this._VicioPts(who, "viciodex") + this._VicioPts(who, "vicioint")) / 3;
		var duration := CInt(AP_GetSkill(who, SCIENCE) / 5 + avg / 15);
		if (duration < 1)
			duration := 1;
		elseif (duration > HIPER_MAX_DURATION)
			duration := HIPER_MAX_DURATION;
		endif
		return duration;
	endfunction

	// Vicio vitals are stored x100; work in points.
	function _VicioPts(this, who, vitalName)
		return CInt(AP_GetVital(who, vitalName) / 100);
	endfunction

endclass
