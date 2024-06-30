
use uo;
include ":brainai:npcUtil";
include ":combat:weaponInfo";
include ":faccao:faccao";

function DistanceChecks(attacker, defender)
	var cur_range := Distance(attacker, defender);
	var wpn_a_elem2 := GetWpnCfgInfo(attacker, attacker.weapon);
	if ( wpn_a_elem2.MaxRange == error )
		return 1;
	endif
	if ( cur_range > wpn_a_elem2.MaxRange )
		CombatDbgMsg(attacker, "Oponente esta muito longe.");
		return 0;
	elseif ( cur_range < wpn_a_elem2.MinRange )
		CombatDbgMsg(attacker, "Oponente esta muito perto.");
		return 0;
	else
		return 1;
	endif
endfunction

function GetWpnCfgInfo(mobile, weapon)

	var npc_cfg2	:= ReadConfigFile("::npcdesc");
	var item_cfg2	:= ReadConfigFile(":*:itemdesc");

	if ( mobile.IsA(POLCLASS_NPC) && weapon.objtype == 0xF020 )
		var npctemplate := mobile.npctemplate;
		npctemplate := npctemplate[10, len(npctemplate)-9];
		return npc_cfg2[npctemplate];
	else
		return item_cfg2[weapon.objtype];
	endif

endfunction

function PlayHitSounds(attacker, defender)
	var wpn_a_elem2 := GetWpnCfgInfo(attacker, attacker.weapon);
	var npc_cfg2	:= ReadConfigFile("::npcdesc");

	if ( wpn_a_elem2.HitSound )
		PlaySoundEffect(attacker, wpn_a_elem2.HitSound);
	else
		//NPC intrinsic weapon
		PlaySoundEffect(attacker, wpn_a_elem2.AttackHitSound);
	endif

	var damaged_sound;
	if ( defender.npctemplate )
		var npctemplate := defender.npctemplate;
		npctemplate := npctemplate[10, len(npctemplate)-9];
		damaged_sound := npc_cfg2[npctemplate].DamageSound;
	else
		var sounds;
		if ( defender.gender == 0 )
			// Male
			sounds := array(341, 342, 343, 345, 346);
		else
			// Female
			sounds := array(332, 333, 334, 335, 336);
		endif
		damaged_sound := sounds[RandomInt(sounds.size())+1];
	endif

	PlaySoundEffect(defender, damaged_sound);
endfunction

function PlayMissSound(attacker)
	if ( wpn_a_elem.MissSound )
		PlaySoundEffect(attacker, wpn_a_elem.MissSound);
	else
		//NPC intrinsic weapon
		PlaySoundEffect(attacker, wpn_a_elem.AttackMissSound);
	endif
endfunction

function PlayAttackAnimation(attacker)
	var attack_anim := 0x0c;
	var wpn_a_elem2 := GetWpnCfgInfo(attacker, attacker.weapon);
	if ( wpn_a_elem2.Anim )
		var anim_list := GetConfigStringArray(wpn_a_elem2, "Anim");
		if (anim_list.size() > 1)
			attack_anim := anim_list[RandomIntMinMax(1, anim_list.size())+1];
		else
			attack_anim := anim_list[1];
		endif
	endif
	if ( attacker.graphic != 400 && attacker.graphic != 401 && attacker.graphic != 1828 && attacker.graphic != 1829 && attacker.graphic != 1830 && attacker.graphic != 1831 && attacker.graphic != 1832 && attacker.graphic != 1833 && attacker.graphic != 1834 && attacker.graphic != 1835)
		attack_anim := 0x0c;
	endif

	PerformAction(attacker, CInt(attack_anim));

	return attack_anim;
endfunction

function GetRawDmg(attacker)

    if (attacker.npctemplate)
      var npcelem := NPC_GetNPCConfig(attacker.npctemplate);
      var dmg := GetConfigString(npcelem, "AttackDamage");
      return RandomDiceRoll(dmg);
    else
      var dmg := CS_GetWeaponInfo(attacker.weapon, "Damage");
      return RandomDiceRoll(dmg);
    endif

endfunction

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
	if ((weapon.attribute) == "MeleeCombat1H")
		can_back_stab := 1;
	endif

	if ((weapon.attribute) == "RangedCombat")
		can_back_stab := 1;
	endif


	if ( !can_back_stab )
		SendSysMessage(who, "Esta arma nao e apropriada para um ataque furtivo.");
		return 0;
	endif

	return 1;
endfunction

function AmmoCheck(attacker, defender)
	var wpn_a_elem := GetWpnCfgInfo(attacker, attacker.weapon);

	if ( !wpn_a_elem.ProjectileType )
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
		SendSysMessage(attacker, "Voce nao definiu o tipo de flecha que vai usar. [clique 2x no arco e na flecha]");
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
		PlayMovingEffectXYZHued(attacker, defender, wpn_a_elem.ProjectileAnim, 10, 10, 0, ammo_color);
		return 1;
	else
		return 0;
	endif

endfunction
