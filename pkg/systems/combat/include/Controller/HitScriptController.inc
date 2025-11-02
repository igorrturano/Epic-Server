include ":combat:Model/AttackInfo";

include ":IO:CFGFile";
include ":message:Model/Message";

function WeaponHitScripts(byRef attackInfo)
	// Script mantido como o anterior, modificado apenas a entrada dos parâmetros
	var attacker := attackInfo.GetAttacker().GetObjRef(),
		defender := attackInfo.GetDefender().GetObjRef(),
		weapon	 := attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetObjRef();

	var weaponCfg := CFGFile("*", "itemdesc").SetElementName(weapon.objtype);
	var weapon_scripts := array{};

	if (weapon.IsA(POLCLASS_WEAPON))
		weapon_scripts := GetObjProperty(weapon, "HitScripts");

		if (!weapon_scripts) weapon_scripts := array{}; endif

		var other_scripts := weaponCfg.SetKeyName("HitScripts").GetElementValue();

		foreach script in other_scripts
			weapon_scripts.Append(script);
		endforeach
	endif

	var hitopt;
	if (attacker.npctemplate)
		var npcCfg := CFGFile("combat", "npcdesc").SetElementName(attacker.npctemplate);
		var others_scripts := npcCfg.SetKeyName("HitScripts").GetElementValue();

		if (weapon_scripts == error) weapon_scripts := array{}; endif

		var body_scripts := GetObjProperty(attacker, "HitScripts");

		foreach oscript in others_scripts
			weapon_scripts.append(oscript);
		endforeach

		foreach oscript in body_scripts
			weapon_scripts.append(oscript);
		endforeach

		hitopt := GetObjProperty(attacker, "generichit");
	else
		hitopt := GetObjProperty(weapon, "generichit");
	endif

	foreach hitscript in weapon_scripts
		var script := start_script(
			hitscript,
			array{ attacker, defender, weapon, armor, basedamage, rawdamage, hitopt }
		);

		if (!script || script.errortext)
			Message("*Attacker* Weapon script error starting [" + hitscript + "] :" + script.errortext)
				.SetLanguageSource(LANGUAGE::ENGLISH)
				.SendSys(attacker)
				.SendSys(defender);
		endif
		sleepms(2);
	endforeach

	if (GetCooldown(weapon, "ataqueelemental"))
		EraseObjProperty(weapon, "#ataqueelemental");
		var elemental := GetObjProperty(weapon, "#elementalweapon");
		var dmg := basedamage + CInt(rawdamage * (AP_GetSkill(attacker, ARCANA) / 100));
		dmg := ApplyChargesCalculation(weapon, dmg, 1);

		if (elemental != error)
			if (elemental.type == DMG_FIRE)
				start_script(
					":combat:weaponHitScripts/FireHit",
					array{ attacker, defender, weapon, armor, basedamage, dmg * 2 }
				);
			elseif (elemental.type == DMG_COLD)
				start_script(
					":combat:weaponHitScripts/IceHit",
					array{ attacker, defender, weapon, armor, basedamage, dmg * 2 }
				);
			elseif (elemental.type == DMG_ENERGY)
				start_script(
					":combat:weaponHitScripts/LightningHit",
					array{ attacker, defender, weapon, armor, basedamage, dmg * 2 }
				);
			endif
		endif
	endif

	var arcane_bows := array{ 0xFA6D, 0XF997 };
	if (GetCooldown(attacker, "arcane_bow") && weapon.objtype in arcane_bows)
		var dmg := basedamage + CInt(rawdamage * (AP_GetSkill(attacker, ARCANA) / 100));
		var arcane_damage := dmg * 0.5;

		if (TemHabilidade(attacker, "Tirothan"))
			arcane_damage := dmg * 1.0;
		endif

		if (GetObjProperty(attacker.weapon, "fire_arrow"))
			start_script(
				":combat:weaponHitScripts/FireHit",
				array{ attacker, defender, weapon, armor, basedamage, arcane_damage }
			);
		elseif (GetObjProperty(attacker.weapon, "cold_arrow"))
			start_script(
				":combat:weaponHitScripts/IceHit",
				array{ attacker, defender, weapon, armor, basedamage, arcane_damage }
			);
		elseif (GetObjProperty(attacker.weapon, "energy_arrow"))
			start_script(
				":combat:weaponHitScripts/LightningHit",
				array{ attacker, defender, weapon, armor, basedamage, arcane_damage }
			);
		endif
	endif

	if (GetObjProperty(attacker, "#golpesangrento")) // VIVISEC do Igor
		// EraseObjProperty(attacker, "#golpesangrento");
		var dmg := 1;
		if (TemHabilidade(attacker, "Golpe Afiado"))
			dmg := basedamage + CInt(rawdamage * (AP_GetSkill(attacker, TACTICS) / 100));
			PlaySoundEffect(attacker, 0x510);
		else
			dmg := basedamage + CInt(rawdamage * (AP_GetSkill(attacker, ALCHEMY) / 100));
			PlaySoundEffect(attacker, 0x511);
		endif
		start_script(
			":combat:weaponHitScripts/bleedingstrike",
			array{ attacker, defender, weapon, armor, basedamage, dmg * 2 }
		);
	endif

	if (GetCooldown(weapon, "circulodevas"))
		var elemental := GetObjProperty(weapon, "#elementalweapon");
		if (elemental != error)
			var dmg := AP_GetSkill(attacker, ARCANA) / 10;
			dmg := ApplyChargesCalculation(weapon, dmg, 2);
			if (dmg < 3)
				dmg := 3;
			endif
			start_script(
				":combat:weaponHitScripts/areadmg",
				array{ attacker, defender, weapon, armor, basedamage, dmg * 2, elemental.type, 3 }
			);
		endif
	endif

	if (getCooldown(attacker, "golpesamplos"))
		var dmg := AP_GetSkill(attacker, TACTICS) / 20;
		if (dmg < 1)
			dmg := 1;
		endif
		start_script(
			":combat:weaponHitScripts/areadmg",
			array{ attacker, defender, weapon, armor, basedamage, dmg, DMG_PHYSICAL }
		);
	endif

	if (GetCooldown(weapon, "habelemental"))
		EraseObjProperty(weapon, "#habelemental");
		var elemental := GetObjProperty(weapon, "#elementalweapon");
		if (elemental != error)
			start_script(
				":combat:weaponHitScripts/habelemental",
				array{ attacker, defender, weapon, armor, basedamage, rawdamage * 2, elemental.type, 3 }
			);
		endif
	endif

	return true;
endfunction

function ArmorHitScripts(byRef attackInfo)
	// Script mantido como o anterior, modificado apenas a entrada dos parâmetros
	var attacker := attackInfo.GetAttacker().GetObjRef(),
		defender := attackInfo.GetDefender().GetObjRef(),
		weapon	 := attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetObjRef();

	var itemCfg := CFGFile("*", "itemdesc").SetElementName(armor.objtype);
	
	var body_scripts := GetObjProperty(defender, "ArmorHitScripts");
	var armor_scripts := GetObjProperty(armor, "ArmorHitScripts");
	var others_scripts := itemCfg.SetKeyName("ArmorHitScripts").GetElementValue();
	if (defender.npctemplate)
		var npcelem := NPC_GetNPCConfig(defender.npctemplate);
		var npcCfg := CFGFile("combat", "npcdesc").SetElementName(defender.npctemplate);
		others_scripts := npcCfg.SetKeyName("ArmorHitScripts").GetElementValue();
	endif
	if (!body_scripts)
		body_scripts := array{};
	endif
	if (!armor_scripts)
		armor_scripts := array{};
	endif
	if (!others_scripts)
		others_scripts := array{};
	endif

	if (GetCooldown(defender, "campoeletrico"))
		var e := start_script(
			":combat:armorHitScripts/campoeletrico",
			array{ attacker, defender, weapon, armor, basedamage, rawdamage }
		);
		if (e == error)
			Message(e).SendAbove(defender);
		endif
	endif

	armor_scripts := armor_scripts + body_scripts + others_scripts;

	foreach hitscript in armor_scripts
		var script := start_script(
			hitscript,
			array{ attacker, defender, weapon, armor, basedamage, rawdamage }
		);
		if (script.errortext)
			// Message("*Defender* armor script error starting [" + hitscript + "] :" + script.errortext)
			// 	.SendSys(attacker)
			// 	.SendSys(defender);
		endif
		sleepms(2);
	endforeach

	return 1;
endfunction