include ":abilities:models/CombatAbility";

/**
 * Reaproveitar Pergaminho (Reuse Scroll) - ABILITY_ID::REAPROVEITAR_PERGAMINHO (1003)
 *
 * Passive: Chance to not consume scrolls on use. Also grants +5 reflexes.
 *
 * Logic lives in:
 *   - cast_spell.src consumeScroll() — scroll reuse chance based on ARCANA
 *   - intrinsicMods.src GetIntrinsicReflexes() — +5 reflexes bonus
 */
class ReuseScroll(CombatAbility)

	function ReuseScroll(this)
		Super(ABILITY_ID::REAPROVEITAR_PERGAMINHO);
	endfunction

endclass
