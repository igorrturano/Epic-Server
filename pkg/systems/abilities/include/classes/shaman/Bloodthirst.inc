use uo;

include ":abilities:models/CombatAbility";
include ":attributes:attributes";
include ":attributes:vitals";
include ":message:model/Message";

/**
 * Sede de Sangue (Bloodthirst) - ABILITY_ID::BLOODTHIRST (2601)
 *
 * Toggle ability. While active, each landed melee hit consumes one
 * shaman_soul and heals the attacker for damage_dealt * nature / 200
 * (up to 50% lifesteal at GM nature). Auto-deactivates when souls
 * run out. The 120s recast delay is enforced by habilidades.cfg.
 *
 * Passive on-kill heal (nature/10 per nearby NPC death) is handled
 * externally in npcdeath.src and is unchanged by this rework.
 */
class Bloodthirst(CombatAbility)

	function Bloodthirst(this)
		Super(ABILITY_ID::BLOODTHIRST);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();

		if (GetObjProperty(who, "#bloodthirst_active"))
			EraseObjProperty(who, "#bloodthirst_active");
			Message("*A sede de sangue se acalma.*").SendAbove(who);
			return true;
		endif

		var souls := CInt(who.GetProp("shaman_souls"));
		if (souls < 1)
			Message("Voce nao possui almas para saciar sua sede.").SendSys(who);
			return false;
		endif

		SetObjProperty(who, "#bloodthirst_active", 1);
		PlaySoundEffect(who, 0x653);
		Message("*Sua sede de sangue desperta.*").SendAbove(who);
		return true;
	endfunction

	function OnPassiveAfterDamage(this, attackInfo)
		var attacker := attackInfo.GetAttacker();
		var who := attacker.GetObjRef();

		if (!GetObjProperty(who, "#bloodthirst_active"))
			return false;
		endif

		var weapon := attackInfo.GetWeapon();
		if (!weapon || weapon.GetAttribute() == RANGED)
			return false;
		endif

		var souls := CInt(who.GetProp("shaman_souls"));
		if (souls < 1)
			EraseObjProperty(who, "#bloodthirst_active");
			Message("*As almas se esgotam e sua sede esmorece.*").SendSys(who);
			return false;
		endif

		var damage := CInt(attackInfo.GetDamage().GetTotalBaseDamage());
		if (damage < 1)
			return false;
		endif

		var nature := CInt(attacker.GetSkill(NATURE));
		var healing := CInt(damage * nature / 200);
		if (healing < 1)
			healing := 1;
		endif

		SetObjProperty(who, "shaman_souls", souls - 1);
		applyEpicHealing(who, healing);

		var defender := attackInfo.GetDefender().GetObjRef();
		PlayMovingEffect(defender, who, 14202, 5, 1, 0);

		if (souls - 1 <= 0)
			EraseObjProperty(who, "#bloodthirst_active");
			Message("*A ultima alma sacia sua sede.*").SendAbove(who);
		endif

		return true;
	endfunction

endclass
