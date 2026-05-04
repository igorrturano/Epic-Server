use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":timedScripts:timedScripts";
include ":attributes:attributes";

/**
 * Kani no Sutairu / Estilo do Caranguejo (Crab Style) - ABILITY_ID::CRAB_STYLE (702)
 *
 * Active (SingleUse AFTER_DAMAGE): proximo ataque causa sangramento por
 * Tactics/10 segundos.
 *
 * Passive (BeforeHitDefender): subtrai Tactics/10 da chance de acerto do
 * atacante (= +Tactics/10 evasao para o defensor). GM Tactics = +10 evasao,
 * combina com MartialArtist (tactics/5 = +20) para total +30 evasao em monge nu.
 *
 * (Pos-Phase 7) A absorcao passiva foi trocada por evasao — UnarmoredCombat
 * agora e a unica fonte de reducao de dano para builds sem armor pesada.
 *
 * Climate resistance passive stays in coldeffect.src/caloreffect.src (outside combat).
 */
class CrabStyle(CombatAbility)

	function CrabStyle(this)
		Super(ABILITY_ID::CRAB_STYLE);
		this.ActivationMoment := ABILITY_ACTIVATION::AFTER_DAMAGE;
		this.SingleUse := true;
	endfunction

	function GetDurationFor(this, mobile)
		var duration := CInt(mobile.GetSkill(TACTICS) / 10);
		if (duration < 2)
			duration := 2;
		endif
		return duration;
	endfunction

	function OnActivate(this, mobile)
		var who := mobile.GetObjRef();
		Message("*estilo do caranguejo*").SendAbove(who);
	endfunction

	// Active: Bleed on next hit
	function OnAfterDamage(this, attackInfo)
		var attacker := attackInfo.GetAttacker();
		var defender := attackInfo.GetDefender();

		var bleedDuration := CInt(attacker.GetSkill(TACTICS) / 10);
		if (bleedDuration < 3)
			bleedDuration := 3;
		endif

		TS_StartTimer(defender.GetObjRef(), "bleed", bleedDuration);

		Message("*faz um corte profundo!*").SendAbove(attacker.GetObjRef());

		DebugMsg(
			attacker.GetObjRef(),
			DEBUG_HAB,
			Message($"[CRAB_STYLE] Sangramento: {bleedDuration}s", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

	// Passive: evasion bonus (defender harder to hit)
	function OnPassiveBeforeHitDefender(this, attackInfo)
		var defender := attackInfo.GetDefender();

		var penalty := CInt(defender.GetSkill(TACTICS) / 10);
		if (penalty <= 0)
			return false;
		endif

		attackInfo.SetHitChanceMod(CInt(attackInfo.GetHitChanceMod()) - penalty);

		DebugMsg(
			defender.GetObjRef(),
			DEBUG_HAB,
			Message($"[CRAB_STYLE] Evasao: -{penalty} acerto inimigo", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

endclass
