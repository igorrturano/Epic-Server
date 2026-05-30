use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":tn:cooldown";

/**
 * Furia Indomavel (Untamable Fury) - ABILITY_ID::FURIA_INDOMAVEL (602)
 *
 * Passive ability. While in fury:
 * - +10 hit chance (already in HitController.inc CalculateCooldownHitModifier)
 * - Immune to critical hits (sets "vitalidadeinabalavel" cooldown, checked by DamageController)
 *
 * Future: paralysis immunity, fortitude as willpower for magic resistance.
 */
class UntamableFury(CombatAbility)

	function UntamableFury(this)
		Super(ABILITY_ID::FURIA_INDOMAVEL);
		this.IsDefenderSide := 1;
	endfunction

	function OnPassiveBeforeDamage(this, attackInfo)
		var defender := attackInfo.GetDefender();
		var who := defender.GetObjRef();

		if (!GetCooldown(who, "furia"))
			return false;
		endif

		if (!GetCooldown(who, "vitalidadeinabalavel"))
			SetCooldown(who, "vitalidadeinabalavel", GetCooldown(who, "furia"));
		endif

		DebugMsg(
			who,
			DEBUG_HAB,
			Message("[FURIA_INDOMAVEL] Imunidade a critico ativada", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

endclass
