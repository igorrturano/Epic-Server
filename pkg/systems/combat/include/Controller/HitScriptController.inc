include ":combat:Model/AttackInfo";
include ":abilities:models/Ability";

include ":IO:CFGFile";
// include ":message:Model/Message";
include "include/epicMageryUtils";

function _NormalizeArmorsForWeaponHit(armorsInSlot)
	if (armorsInSlot == error)
		return array{uninit};
	endif

	return armorsInSlot;
endfunction

function _ArmorObjRefOrUninit(armor)
	if (!armor)
		return uninit;
	endif

	return armor.GetObjRef();
endfunction

function WeaponHitScripts(byRef attackInfo)
	var armorsInSlot := _NormalizeArmorsForWeaponHit(attackInfo.GetDefender().GetEquipBySlot(attackInfo.GetBodyRegion()));

	var weaponCfg := CFGFile("*", "itemdesc").SetElementName(attackInfo.GetWeapon().GetObjType());
	var weapon_scripts := array{};

	if (attackInfo.GetWeapon().GetObjRef().IsA(POLCLASS_WEAPON))
		weapon_scripts := attackInfo.GetWeapon().GetObjRef().GetProp("HitScripts");

		if (!weapon_scripts) weapon_scripts := array{}; endif

		var other_scripts := weaponCfg.GetPropertyValue("HitScripts");

		foreach script in other_scripts
			weapon_scripts.Append(script);
		endforeach
	endif

	var hitopt;
	if (attackInfo.GetAttacker().IsNPC())
		var npcCfg := CFGFile("combat", "npcdesc").SetElementName(attackInfo.GetAttacker().GetTemplate());
		var others_scripts := npcCfg.GetPropertyValue("HitScripts");

		if (weapon_scripts == error) weapon_scripts := array{}; endif

		var body_scripts := attackInfo.GetAttacker().GetObjRef().GetProp("HitScripts");

		foreach oscript in others_scripts
			weapon_scripts.append(oscript);
		endforeach

		foreach oscript in body_scripts
			weapon_scripts.append(oscript);
		endforeach

		hitopt := attackInfo.GetAttacker().GetObjRef().GetProp("generichit");
	else
		// Read body HitScripts for players (e.g., metamorphosed druids)
		var body_scripts := attackInfo.GetAttacker().GetObjRef().GetProp("HitScripts");
		if (body_scripts)
			foreach oscript in body_scripts
				weapon_scripts.append(oscript);
			endforeach
		endif
		hitopt := attackInfo.GetWeapon().GetObjRef().GetProp("generichit");
	endif

	foreach armor in (armorsInSlot)
		foreach hitscript in weapon_scripts
			var script := start_script(
				hitscript,
				array{
					attackInfo.GetAttacker().GetObjRef(),
					attackInfo.GetDefender().GetObjRef(),
					attackInfo.GetWeapon().GetObjRef(),
					_ArmorObjRefOrUninit(armor),
					attackInfo.GetDamage().GetTotalBaseDamage(),
					attackInfo.GetDamage().GetTotalRawDamage(),
					hitopt
				}
			);

			if (!script || script.errortext)
				Message("*Attacker* Weapon script error starting [" + hitscript + "] :" + script.errortext)
					.SetLanguageSource(LANGUAGE::ENGLISH)
					.SendSys(attackInfo.GetAttacker().GetObjRef())
					.SendSys(attackInfo.GetDefender().GetObjRef());
			endif
			sleepms(2);
		endforeach
	endforeach

	if (GetCooldown(attackInfo.GetWeapon().GetObjRef(), "ataqueelemental"))
		attackInfo.GetWeapon().GetObjRef().EraseProp("#ataqueelemental");
		var elemental := attackInfo.GetWeapon().GetObjRef().GetProp("#elementalweapon");
		print("Debug: Elemental weapon hit - element: " + (elemental != error ? elemental.type : "none") + ", attacker: " + attackInfo.GetAttacker().GetName());
		if (elemental != error)
			// Burst damage: Arcana/10 + INT/20 (hit script applies *0.5+1)
			var arcana := attackInfo.GetAttacker().GetSkill(ARCANA);
			var dmg := CInt(arcana / 10) + CInt(attackInfo.GetAttacker().GetInt() / 20);

			var hitScript;
			if (elemental.type == "Fire")
				hitScript := ":combat:weaponHitScripts/FireHit";
			elseif (elemental.type == "Cold")
				hitScript := ":combat:weaponHitScripts/IceHit";
			elseif (elemental.type == "Energy")
				hitScript := ":combat:weaponHitScripts/LightningHit";
			endif
			print("Debug: Elemental weapon hit - selected hit script: " + (hitScript ? hitScript : "none"));
			if (hitScript)
				foreach armor in (armorsInSlot)
					start_script(
						hitScript,
						array{
							attackInfo.GetAttacker().GetObjRef(),
							attackInfo.GetDefender().GetObjRef(),
							attackInfo.GetWeapon().GetObjRef(),
							_ArmorObjRefOrUninit(armor),
							0,
							dmg * 2
						}
					);
				endforeach
			endif

			// Consume charges after hit scripts fire
			ApplyChargesCalculation(attackInfo.GetWeapon().GetObjRef(), dmg, 1);
		endif
	endif

	if (attackInfo.GetAttacker().GetObjRef().GetProp("#golpesangrento")) // VIVISEC do Igor
		// EraseObjProperty(attacker, "#golpesangrento");
		var dmg := 1;

		dmg := attackInfo.GetDamage().GetTotalBaseDamage() + CInt(attackInfo.GetDamage().GetTotalRawDamage() * (attackInfo.GetAttacker().GetSkill(ALCHEMY) / 100));
		
		foreach armor in (armorsInSlot)
			start_script(
				":combat:weaponHitScripts/bleedingstrike",
				array{
					attackInfo.GetAttacker().GetObjRef(),
					attackInfo.GetDefender().GetObjRef(),
					attackInfo.GetWeapon().GetObjRef(),
					_ArmorObjRefOrUninit(armor),
					attackInfo.GetDamage().GetTotalBaseDamage(),
					dmg * 2
				}
			);
		endforeach
	endif

	if (GetCooldown(attackInfo.GetWeapon().GetObjRef(), "circulodevas"))
		var elemental := attackInfo.GetWeapon().GetObjRef().GetProp("#elementalweapon");

		if (elemental != error)
			var dmg := attackInfo.GetAttacker().GetSkill(ARCANA) / 10;
			dmg := ApplyChargesCalculation(attackInfo.GetWeapon().GetObjRef(), dmg, 2);

			if (dmg < 3)
				dmg := 3;
			endif

			foreach armor in (armorsInSlot)
				start_script(
					":combat:weaponHitScripts/areadmg",
					array{
						attackInfo.GetAttacker().GetObjRef(),
						attackInfo.GetDefender().GetObjRef(),
						attackInfo.GetWeapon().GetObjRef(),
						_ArmorObjRefOrUninit(armor),
						attackInfo.GetDamage().GetTotalBaseDamage(),
						dmg * 2,
						elemental.type,
						3
					}
				);
			endforeach
		endif
	endif

	if (GetCooldown(attackInfo.GetAttacker().GetObjRef(), "golpesamplos"))
		var dmg := attackInfo.GetAttacker().GetSkill(TACTICS) / 20;

		if (dmg < 1)
			dmg := 1;
		endif

		foreach armor in (armorsInSlot)
			start_script(
				":combat:weaponHitScripts/areadmg",
				array{
					attackInfo.GetAttacker().GetObjRef(),
					attackInfo.GetDefender().GetObjRef(),
					attackInfo.GetWeapon().GetObjRef(),
					_ArmorObjRefOrUninit(armor),
					attackInfo.GetDamage().GetTotalBaseDamage(),
					dmg,
					DMG_PHYSICAL
				}
			);
		endforeach
	endif

	if (GetCooldown(attackInfo.GetAttacker().GetObjRef(), "habelemental"))
		attackInfo.GetAttacker().GetObjRef().EraseProp("#habelemental");
		var elemental := attackInfo.GetWeapon().GetObjRef().GetProp("#elementalweapon");

		foreach armor in (armorsInSlot)
			if (elemental != error)
				start_script(
					":combat:weaponHitScripts/habelemental",
					array{
						attackInfo.GetAttacker().GetObjRef(),
						attackInfo.GetDefender().GetObjRef(),
						attackInfo.GetWeapon().GetObjRef(),
						_ArmorObjRefOrUninit(armor),
						attackInfo.GetDamage().GetTotalBaseDamage(),
						attackInfo.GetDamage().GetTotalRawDamage() * 2,
						elemental.type,
						3
					}
				);
			endif
		endforeach
	endif

	return true;
endfunction

function ArmorHitScripts(byRef attackInfo)
	var armorsInSlot := attackInfo.GetDefender().GetEquipBySlot(attackInfo.GetBodyRegion());
	var itemCfg := CFGFile("*", "itemdesc");

	var defenderRef := attackInfo.GetDefender().GetObjRef();
	var baseArgs := array{
		attackInfo.GetAttacker().GetObjRef(),
		defenderRef,
		attackInfo.GetWeapon().GetObjRef(),
		uninit,
		attackInfo.GetDamage().GetTotalBaseDamage(),
		attackInfo.GetDamage().GetTotalRawDamage()
	};

	// Defender-wide hooks (SC spell effects like retribuicao): fire once per
	// hit, independent of armor presence in the slot.
	var body_scripts := defenderRef.GetProp("ArmorHitScripts");
	if (!body_scripts) body_scripts := array{}; endif

	foreach hitscript in body_scripts
		start_script(hitscript, baseArgs);
		sleepms(2);
	endforeach

	if (GetCooldown(defenderRef, "campoeletrico"))
		var e := start_script(":combat:armorHitScripts/campoeletrico", baseArgs);
		if (e == error)
			Message(e).SendAbove(defenderRef);
		endif
	endif

	// Armor-specific hooks (per-piece): require armor in slot.
	if (armorsInSlot is @Exception)
		return 1;
	endif

	foreach armor in (armorsInSlot)
		itemCfg.SetElementName(armor.GetObjType());

		var armor_scripts := armor.GetObjRef().GetProp("ArmorHitScripts");
		var others_scripts := itemCfg.GetPropertyValue("ArmorHitScripts");

		if (attackInfo.GetDefender().IsNPC())
			others_scripts := CFGFile("combat", "npcdesc").SetElementName(attackInfo.GetDefender().GetTemplate())
														  .GetPropertyValue("ArmorHitScripts");
		endif

		if (!armor_scripts) armor_scripts := array{}; endif
		if (!others_scripts) others_scripts := array{}; endif

		foreach hitscript in (armor_scripts + others_scripts)
			start_script(
				hitscript,
				array{
					attackInfo.GetAttacker().GetObjRef(),
					defenderRef,
					attackInfo.GetWeapon().GetObjRef(),
					armor.GetObjRef(),
					attackInfo.GetDamage().GetTotalBaseDamage(),
					attackInfo.GetDamage().GetTotalRawDamage()
				}
			);
			sleepms(2);
		endforeach
	endforeach

	return 1;
endfunction