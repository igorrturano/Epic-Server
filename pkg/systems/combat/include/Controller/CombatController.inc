
use uo;


include "include/packets";
include ":brainai:npcUtil";
include ":tn:cooldown";
// include ":combat:weaponInfo";
// include ":combat:Controller/FXController";
include ":faccao:faccao";
include ":equipsys:ViewModel/EquipWrapperWeapon";
include ":message:model/Message";

function checkGender(object)
	var gender := 0;
	if (object.graphic)
		case (object.graphic)
			400:
			605: // elf
				gender := 1;
			401:
			606: // elf
				gender := 2;
		endcase
	elseif (object.corpsetype)
		case (object.graphic)
			400:
			605: // elf
			403:
			607: // elf
				gender := 1;
			401:
			606: // elf
			404:
			608: // elf
				gender := 2;
		endcase
	elseif (object.gender)
		if (gender == 0)
			gender := 1;
		else
			gender := 2;
		endif
	endif

	return gender;
endfunction

function PlayHurtSound(defender)
	if ( defender.IsA(POLCLASS_NPC) )
      var damaged_sound := NPC_GetNPCConfig(defender).HurtSound;
		if ( damaged_sound && RandomInt(2) == 2)
			PlaySoundEffect(defender, damaged_sound);
		endif
	endif
endfunction

function CanBackStab(who, weapon)
	var can_back_stab := 0;

	if ((weapon.attribute) != "Two_Handed")
		can_back_stab := 1;
	endif

	if (!can_back_stab)
		Message("Esta arma não é apropriada para um ataque furtivo.").SendSys(who);
		return 0;
	endif

	return can_back_stab;
endfunction

function AmmoCheck(attacker, defender)
	var attackerWeapon := EquipWrapperWeapon(attacker.weapon);

	if (!attackerWeapon.GetTemplate().ProjectileType)
		return 1;
	endif
    var arcane_bows := array{0xFA6D, 0XF997};

	if (!GetCooldown(attacker, "arcane_bow") && !(attackerWeapon.GetObjType() in arcane_bows) && !attackerWeapon.GetTemplate().ProjectileType)
		return 1;
	endif
	var has_ammo := 0;

	var aljavas := array{0x791c, 0x2fb7};
	var aljava := GetEquipmentByLayer(attacker, 0x14);

		if (aljava.objtype in aljavas)
			aljava := GetEquipmentByLayer(attacker, 0x14);
		else
			aljava := GetEquipmentByLayer(attacker, 0xc);
		endif
	var ammo := GetObjProperty(attacker.weapon, "ammo");
	if (!ammo)
		Message("Você não definiu o tipo de flecha que vai usar. [clique 2x no arco e na flecha]").SendSys(attacker);
		return 0;
	endif

    var in_aljava := 0;
    foreach arrow in (EnumerateItemsInContainer(aljava))
		if (arrow.serial == ammo)
			in_aljava := 1;
        endif
    endforeach
    if (in_aljava == 0)
        return 0;
    endif


	if ( SubtractAmount( ammo, 1 ) )
		var cfg:=ReadConfigFile(":woodworking:itemdesc");
		var elem := FindConfigElem(cfg, GetObjType( ammo ));
		var ammo_color := elem.color;
		PlayMovingEffectXYZHued(attacker, defender, !attackerWeapon.GetTemplate().ProjectileType, 10, 10, 0, ammo_color);
		return 1;
	else
		return 0;
	endif

endfunction
