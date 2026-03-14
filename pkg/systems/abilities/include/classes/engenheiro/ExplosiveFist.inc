use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":attributes:attributes";

class ExplosiveFist(CombatAbility)

	function ExplosiveFist(this)
		Super(ABILITY_ID::EXPLOSIVE_FIST);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();
		var gloves := GetEquipmentByLayer(who, 0x07); // LAYER_GLOVES

		if (GetObjProperty(who, "#explosive_fist"))
			EraseObjProperty(who, "#explosive_fist");
			if (gloves && gloves.objtype == 0xC629)
				gloves.color := Cint(GetObjProperty(gloves, "original_color"));
				EraseObjProperty(gloves, "original_color");
			endif
			Message("*Desativa os punhos explosivos*").SendAbove(who);
			DebugMsg(
				who,
				DEBUG_HAB,
				Message("[EXPLOSIVE_FIST] Desativado", Color(SYSTEM_COLOR::INFO))
			);
		else
			var powder := FindObjtypeInContainer(who.backpack, 0x9092);
			if (!powder)
				Message("Voce precisa de polvora para ativar os punhos explosivos!").SendSys(who);
				return false;
			endif
			SetObjProperty(who, "#explosive_fist", 1);
			if (gloves && gloves.objtype == 0xC629)
				SetObjProperty(gloves, "original_color", gloves.color);
				gloves.color := 0x489;
			endif
			Message("*Ativa os punhos explosivos*").SendAbove(who);
			PlaySoundEffect(who, 0x667);

			DebugMsg(
				who,
				DEBUG_HAB,
				Message("[EXPLOSIVE_FIST] Ativado", Color(SYSTEM_COLOR::SUCCESS))
			);
		endif

		return true;
	endfunction

	function OnPassiveBeforeDamage(this, attackInfo)
		var attacker := attackInfo.GetAttacker();
		var who := attacker.GetObjRef();

		if (!GetObjProperty(who, "#explosive_fist"))
			return false;
		endif

		var powder := FindObjtypeInContainer(who.backpack, 0x9092);
		if (!powder)
			EraseObjProperty(who, "#explosive_fist");
			Message("*Polvora esgotada! Punhos explosivos desativados.*").SendSys(who);
			return false;
		endif

		SubtractAmount(powder, 1);

		var bonus := CInt(attacker.GetSkill(METALWORKING) / 10);
		if (bonus > 0)
			attackInfo.GetDamage().IncreaseFireRawDamage(bonus);
		endif

		var defender := attackInfo.GetDefender().GetObjRef();
		PlaySoundEffect(defender, 0x208);

		PlayObjectCenteredEffect(defender, FX_EXPLODE_1, 0, 10);

		DebugMsg(
			who,
			DEBUG_HAB,
			Message($"[EXPLOSIVE_FIST] +{bonus} dano fogo, 1 polvora consumida", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

	function OnPassiveAfterDamage(this, attackInfo)
		var attacker := attackInfo.GetAttacker();
		var who := attacker.GetObjRef();

		if (!GetObjProperty(who, "#explosive_fist"))
			return false;
		endif

		var defender := attackInfo.GetDefender().GetObjRef();
		_PushBack(defender, 3, 300);

		return true;
	endfunction

endclass

function _PushBack(who, dist, delay)
	var x := 0;
	var y := 0;

	if (who.facing == 0)
		y := 1;
	elseif (who.facing == 1)
		y := 1;
		x := -1;
	elseif (who.facing == 2)
		x := -1;
	elseif (who.facing == 3)
		x := -1;
		y := -1;
	elseif (who.facing == 4)
		y := -1;
	elseif (who.facing == 5)
		x := 1;
		y := -1;
	elseif (who.facing == 6)
		x := 1;
	elseif (who.facing == 7)
		x := 1;
		y := 1;
	endif

	for i := 1 to dist
		var finalx := (who.x + x);
		var finaly := (who.y + y);
		var err := MoveObjectToLocation(who, finalx, finaly, who.z, who.realm);
		sleepms(delay);
		if (err == error)
			break;
		endif
	endfor
endfunction
