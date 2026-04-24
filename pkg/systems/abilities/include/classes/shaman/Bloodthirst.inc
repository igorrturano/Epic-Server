use uo;

include ":abilities:models/CombatAbility";
include ":attributes:attributes";
include ":attributes:vitals";
include ":tn:cooldown";
include ":message:model/Message";

/**
 * Sede de Sangue (Bloodthirst) - ABILITY_ID::BLOODTHIRST (2601)
 *
 * ON_USE: Consumes all collected shaman_souls to heal the caster.
 *   Healing = shaman_souls * (Theology / 25)
 *   Sets "bloodlust" cooldown for shaman_souls duration.
 *
 * Passive: On nearby NPC death, heals Theology/10 HP.
 *   (Handled externally in npcdeath.src, checked via HasAbility)
 */
class Bloodthirst(CombatAbility)

	function Bloodthirst(this)
		Super(ABILITY_ID::BLOODTHIRST);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();

		var shaman_souls := CInt(who.GetProp("shaman_souls"));
		if (!shaman_souls)
			shaman_souls := 1;
		endif

		var bonus_mlore := CInt(mobile.GetSkill(THEOLOGY) / 25);
		var healing := shaman_souls * bonus_mlore;

		SetCooldown(who, "bloodlust", shaman_souls);
		applyEpicHealing(who, healing);

		PlayStationaryEffect(who.x, who.y, who.z + 2, 0x546C, 0, 30);
		who.EraseProp("shaman_souls");

		Message("Voce concede as almas um novo proposito.").SendSys(who);

		return true;
	endfunction

endclass
