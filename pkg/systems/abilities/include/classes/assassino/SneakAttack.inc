use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":combat:Controller/HitController";
include "include/epicCombatUtils";

/**
 * Ataque Furtivo (Sneak Attack) - ABILITY_ID::ATAQUE_FURTIVO (1201)
 *
 * OnUse: Prepares a backstab. Sets properties that bypass parry,
 * guarantee hit (if hidden), and guarantee critical (if has Instinto Assassino).
 * Grants hiddenattack privilege to allow attacking while hidden.
 */
class SneakAttack(CombatAbility)

	function SneakAttack(this)
		Super(ABILITY_ID::ATAQUE_FURTIVO);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function ExecuteOn(this, mobile)
		if (!CanBackStab(mobile) || !isAssassinWeapon(mobile.GetObjRef()))
			Message("Esta arma não é apropriada para um ataque furtivo.", Color(SYSTEM_COLOR::FAIL)).SendSys(mobile.GetObjRef());
			return false;
		endif

		mobile.SetProperty("backstab", 1);
		mobile.GetEquipBySlot(WEAPON).SetProperty("backstab", 1);

		if (mobile.IsHidden())
			mobile.SetProperty("#GuaranteedHit", 1);
			if (mobile.HasAbility(ABILITY_ID::INSTINTO_ASSASSINO))
				mobile.SetProperty("#nextcritical", 1);
			endif
		endif

		GrantPrivilege(mobile.GetObjRef(), "hiddenattack");
		mobile.GetObjRef().enable("hiddenattack");
		Message("Você está preparado para um ataque furtivo!", Color(SYSTEM_COLOR::SUCCESS)).SendSys(mobile.GetObjRef());
		return true;
	endfunction

endclass
