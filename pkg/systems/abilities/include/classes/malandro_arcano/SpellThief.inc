include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":tn:cooldown";

/**
 * Ladrao de Magias (Spell Thief) - ABILITY_ID::LADRAO_DE_MAGIAS (1001)
 *
 * OnUse: Sets "spellthief" cooldown for ARCANA seconds.
 * While active, any spell targeting the player is reflected back to the caster.
 * Reflection check already exists in tnmagery.inc:239 (swaps targ := mobile).
 */
class SpellThief(CombatAbility)

	function SpellThief(this)
		Super(ABILITY_ID::LADRAO_DE_MAGIAS);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();
		var arcana := CInt(mobile.GetSkill(ARCANA));

		SetCooldown(who, "spellthief", arcana);

		Message("*prepara para refletir magias*").SendAbove(who);
		Message($"Você reflete magias por {arcana} segundos!", Color(SYSTEM_COLOR::SUCCESS)).SendSys(who);

		return true;
	endfunction

endclass
