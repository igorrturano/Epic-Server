
use uo;
include ":brainai:npcUtil";
include ":combat:weaponInfo";
include ":combat:combatFX";
include ":faccao:faccao";
include ":equipsys:props";

function DistanceChecks(attacker, defender)
	var cur_range := Distance(attacker, defender);
	var wpn_a_elem2 := GetWpnCfgInfo(attacker, attacker.weapon);
	var weapon_max_range := wpn_a_elem2.MaxRange;
	var weapon_min_range := wpn_a_elem2.MinRange;
	if ( wpn_a_elem2.MaxRange == error )
		return 1;
	endif
	
	if ( cur_range > weapon_max_range )
		return 0;
	elseif ( cur_range < weapon_min_range )
		return 0;
	else
		return 1;
	endif
endfunction

function GetWpnCfgInfo(mobile, weapon)

	var npc_cfg2	:= ReadConfigFile(":*:npcdesc");
	var item_cfg2	:= ReadConfigFile(":*:itemdesc");

	if ( mobile.IsA(POLCLASS_NPC) && weapon.objtype == 0x1F020 )
		var npctemplate := mobile.npctemplate;
		npctemplate := npctemplate[10, len(npctemplate)-9];
		return npc_cfg2[npctemplate];
	else
		return item_cfg2[weapon.objtype];
	endif

endfunction

function PlayHitSounds(attacker, defender)
	var wpn_a_elem2 := GetWpnCfgInfo(attacker, attacker.weapon);
	var npc_cfg2	:= ReadConfigFile(":*:npcdesc");

	var attack_sound;
	if ( wpn_a_elem2.HitSound )
		attack_sound := wpn_a_elem2.HitSound;
	else
		//NPC intrinsic weapon
		attack_sound := wpn_a_elem2.AttackHitSound;
	endif

	if (attacker.weapon)
		attack_sound := getWeaponFX(WEAPON_FX_SOUND, attacker.weapon);
	endif

	PlaySoundEffect(attacker, attack_sound);

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
	PlaySoundEffect(attacker, getSound(MISS_FX));
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
	if ((weapon.attribute) != "Two_Handed")
		can_back_stab := 1;
	endif

	if ( !can_back_stab )
		SendSysMessage(who, "Esta arma nao e apropriada para um ataque furtivo.");
		return 0;
	endif

	return can_back_stab;
endfunction

function AmmoCheck(attacker, defender)
	var wpn_a_elem := GetWpnCfgInfo(attacker, attacker.weapon);

	if ( !wpn_a_elem.ProjectileType )
		return 1;
	endif
    var arcane_bows := array{0xFA6D, 0XF997};

	if ( !GetCooldown(attacker, "arcane_bow") && !(attacker.weapon.objtype in arcane_bows) && !wpn_a_elem.ProjectileType)
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
