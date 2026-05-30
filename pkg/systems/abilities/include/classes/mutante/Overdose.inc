use uo;

include ":abilities:models/CombatAbility";
include ":attributes:attributes";
include ":timedScripts:timedScripts";
include ":tn:cooldown";
include ":hotbar:custombuff";
include ":message:model/Message";
include ":debug:Controller/DebugController";

/**
 * Sobredose (Overdose) - ABILITY_ID::SOBREDOSE (903)
 *
 * Active capstone. Held active for Science/10 seconds (windowed, NOT ON_USE, so
 * the monitor runs the duration and calls OnDeactivate on expiry). While the
 * "sobredose" cooldown is live the Mutie is:
 *   - immune to CC (each CC script early-returns on GetCooldown(who,"sobredose"))
 *   - immune to criticals (CalculateCriticalChance returns 0 vs that defender)
 * There is intentionally NO 1-HP save and NO movement bonus.
 *
 * Aftermath (5s "letargia_severa" cooldown, set in OnDeactivate): the shared
 * "lentidao" slow (no-run + -DEX) plus -50% outgoing damage, applied here via
 * OnPassiveBeforeDamage. The reduction runs pre-resist, which is mathematically
 * identical to reducing the final post-resist value (resists are multiplicative).
 */

const SOBREDOSE_LETHARGY := 5;

class Overdose(CombatAbility)

	function Overdose(this)
		Super(ABILITY_ID::SOBREDOSE);
		this.ActivationMoment := ABILITY_ACTIVATION::BEFORE_DAMAGE;
	endfunction

	function GetDurationFor(this, mobile)
		var duration := CInt(mobile.GetSkill(SCIENCE) / 10);
		if (duration < 1)
			duration := 1;
		endif
		return duration;
	endfunction

	function OnActivate(this, mobile)
		var who := mobile.GetObjRef();
		var duration := this.GetDurationFor(mobile);

		// Active-state flag the CC/crit immunity checks read (separate from the
		// cfg `delay 300` reuse cooldown).
		SetCooldown(who, "sobredose", duration);

		PlaySoundEffect(who, 0x64B);
		SendAbilityBuff(who, ABILITY_ID::SOBREDOSE, duration, "Imune a controle e a criticos");
		Message("Sobredose! Seu corpo se torna uma muralha quimica.", Color(SYSTEM_COLOR::SUCCESS)).SendSys(who);
		DebugMsg(who, DEBUG_HAB, Message($"[SOBREDOSE] imunidade por {duration}s", Color(SYSTEM_COLOR::SUCCESS)));
	endfunction

	function OnDeactivate(this, mobile)
		var who := mobile.GetObjRef();
		// Letargia Severa: shared "lentidao" slow (no-run + -DEX) for the visual/
		// movement penalty, plus the "letargia_severa" cooldown that gates the
		// -50% outgoing reduction in OnPassiveBeforeDamage below.
		TS_StartTimer(who, "lentidao", SOBREDOSE_LETHARGY);
		SetCooldown(who, "letargia_severa", SOBREDOSE_LETHARGY);
		Message("A sobredose cobra seu preco: letargia severa.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
	endfunction

	// Letargia Severa -50% outgoing damage. Attacker-side passive (default
	// IsDefenderSide 0), gated on the post-Sobredose cooldown.
	function OnPassiveBeforeDamage(this, attackInfo)
		if (!GetCooldown(attackInfo.GetAttacker().GetObjRef(), "letargia_severa"))
			return false;
		endif
		attackInfo.GetDamage()
			.DecreaseSlashPercentDamage(50)
			.DecreasePiercingPercentDamage(50)
			.DecreaseBluntPercentDamage(50)
			.DecreaseFirePercentDamage(50)
			.DecreaseColdPercentDamage(50)
			.DecreasePoisonPercentDamage(50)
			.DecreaseEnergyPercentDamage(50);
		return true;
	endfunction

endclass
