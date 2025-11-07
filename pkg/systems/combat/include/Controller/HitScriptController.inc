include ":combat:Model/AttackInfo";
include ":combat:Model/CombatAbility";

include ":IO:CFGFile";
include ":message:Model/Message";
include "include/epicMageryUtils";

function WeaponHitScripts(byRef attackInfo)
	var armorsInSlot := attackInfo.GetDefender().GetEquipBySlot(attackInfo.GetBodyRegion());

	var weaponCfg := CFGFile("*", "itemdesc").SetElementName(attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetObjType());
	var weapon_scripts := array{};

	if (attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetObjRef().IsA(POLCLASS_WEAPON))
		weapon_scripts := attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetObjRef().GetProp("HitScripts");

		if (!weapon_scripts) weapon_scripts := array{}; endif

		var other_scripts := weaponCfg.SetKeyName("HitScripts").GetKeyValue();

		foreach script in other_scripts
			weapon_scripts.Append(script);
		endforeach
	endif

	var hitopt;
	if (attackInfo.GetAttacker().IsNPC())
		var npcCfg := CFGFile("combat", "npcdesc").SetElementName(attackInfo.GetAttacker().GetNPCTemplate());
		var others_scripts := npcCfg.SetKeyName("HitScripts").GetKeyValue();

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
		hitopt := attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetObjRef().GetProp("generichit");
	endif

	foreach armor in (armorsInSlot)
		foreach hitscript in weapon_scripts
			var script := start_script(
				hitscript,
				array{
					attackInfo.GetAttacker().GetObjRef(),
					attackInfo.GetDefender().GetObjRef(),
					attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetObjRef(),
					armor.GetObjRef(),
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

	if (GetCooldown(attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetObjRef(), "ataqueelemental"))
		attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetObjRef().EraseProp("#ataqueelemental");
		var elemental := attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetObjRef().GetProp("#elementalweapon");

		if (elemental != error)
			var dmg;
			if (elemental.type == "Fire")
				dmg := attackInfo.GetDamage().GetFireBaseDamage() + CInt(attackInfo.GetDamage().GetFireRawDamage() * (attackInfo.GetAttacker().GetSkill(ARCANA) / 100));
				dmg := ApplyChargesCalculation(attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetObjRef(), dmg, 1);
				
				foreach armor in (armorsInSlot)
					start_script(
						":combat:weaponHitScripts/FireHit",
						array{
							attackInfo.GetAttacker().GetObjRef(),
							attackInfo.GetDefender().GetObjRef(),
							attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetObjRef(),
							armor.GetObjRef(),
							attackInfo.GetDamage().GetFireBaseDamage(),
							dmg * 2
						}
					);
				endforeach
			elseif (elemental.type == "Cold")
				dmg := attackInfo.GetDamage().GetColdBaseDamage() + CInt(attackInfo.GetDamage().GetColdRawDamage() * (attackInfo.GetAttacker().GetSkill(ARCANA) / 100));
				dmg := ApplyChargesCalculation(attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetObjRef(), dmg, 1);

				foreach armor in (armorsInSlot)
					start_script(
						":combat:weaponHitScripts/IceHit",
						array{
							attackInfo.GetAttacker().GetObjRef(),
							attackInfo.GetDefender().GetObjRef(),
							attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetObjRef(),
							armor.GetObjRef(),
							attackInfo.GetDamage().GetColdBaseDamage(),
							dmg * 2
						}
					);
				endforeach
			elseif (elemental.type == "Energy")
				dmg := attackInfo.GetDamage().GetEnergyBaseDamage() + CInt(attackInfo.GetDamage().GetEnergyRawDamage() * (attackInfo.GetAttacker().GetSkill(ARCANA) / 100));
				dmg := ApplyChargesCalculation(attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetObjRef(), dmg, 1);
				
				foreach armor in (armorsInSlot)
					start_script(
						":combat:weaponHitScripts/LightningHit",
						array{
							attackInfo.GetAttacker().GetObjRef(),
							attackInfo.GetDefender().GetObjRef(),
							attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetObjRef(),
							armor.GetObjRef(),
							attackInfo.GetDamage().GetEnergyBaseDamage(),
							dmg * 2
						}
					);
				endforeach
			endif
		endif
	endif

	var arcane_bows := array{ 0xFA6D, 0XF997 };
	if (GetCooldown(attackInfo.GetAttacker().GetObjRef(), "arcane_bow") && attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetObjType() in arcane_bows)
		var dmg := attackInfo.GetDamage().GetTotalBaseDamage() + CInt(attackInfo.GetDamage().GetTotalRawDamage() * (attackInfo.GetAttacker().GetSkill(ARCANA) / 100));
		var arcane_damage;

		if (attackInfo.GetAttacker().HasAbility(COMBAT_ABILITY::TIROTHAN))
			arcane_damage := dmg * 1.0;
		else
			arcane_damage := dmg * 0.5;
		endif

		if (attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetObjRef().GetProp("fire_arrow"))
			foreach armor in (armorsInSlot)
				start_script(
					":combat:weaponHitScripts/FireHit",
					array{
						attackInfo.GetAttacker().GetObjRef(),
						attackInfo.GetDefender().GetObjRef(),
						attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetObjRef(),
						armor.GetObjRef(),
						attackInfo.GetDamage().GetFireBaseDamage(),
						arcane_damage
					}
				);
			endforeach
		elseif (attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetObjRef().GetProp("cold_arrow"))
			foreach armor in (armorsInSlot)
				start_script(
					":combat:weaponHitScripts/IceHit",
					array{
						attackInfo.GetAttacker().GetObjRef(),
						attackInfo.GetDefender().GetObjRef(),
						attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetObjRef(),
						armor.GetObjRef(),
						attackInfo.GetDamage().GetColdBaseDamage(),
						arcane_damage
					}
				);
			endforeach
		elseif (attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetObjRef().GetProp("energy_arrow"))
			foreach armor in (armorsInSlot)
				start_script(
					":combat:weaponHitScripts/LightningHit",
					array{
						attackInfo.GetAttacker().GetObjRef(),
						attackInfo.GetDefender().GetObjRef(),
						attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetObjRef(),
						armor.GetObjRef(),
						attackInfo.GetDamage().GetEnergyBaseDamage(),
						arcane_damage
					}
				);
			endforeach
		endif
	endif

	if (attackInfo.GetAttacker().GetObjRef().GetProp("#golpesangrento")) // VIVISEC do Igor
		// EraseObjProperty(attacker, "#golpesangrento");
		var dmg := 1;

		if (attackInfo.GetAttacker().HasAbility("Golpe Afiado"))
			dmg := attackInfo.GetDamage().GetTotalBaseDamage() + CInt(attackInfo.GetDamage().GetTotalRawDamage() * (attackInfo.GetAttacker().GetSkill(TACTICS) / 100));
		else
			dmg := attackInfo.GetDamage().GetTotalBaseDamage() + CInt(attackInfo.GetDamage().GetTotalRawDamage() * (attackInfo.GetAttacker().GetSkill(ALCHEMY) / 100));
		endif
		
		foreach armor in (armorsInSlot)
			start_script(
				":combat:weaponHitScripts/bleedingstrike",
				array{
					attackInfo.GetAttacker().GetObjRef(),
					attackInfo.GetDefender().GetObjRef(),
					attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetObjRef(),
					armor.GetObjRef(),
					attackInfo.GetDamage().GetTotalBaseDamage(),
					dmg * 2
				}
			);
		endforeach
	endif

	if (GetCooldown(attackInfo.GetAttacker().GetObjRef(), "circulodevas"))
		var elemental := attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetObjRef().GetProp("#elementalweapon");

		if (elemental != error)
			var dmg := attackInfo.GetAttacker().GetSkill(ARCANA) / 10;
			dmg := ApplyChargesCalculation(attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetObjRef(), dmg, 2);

			if (dmg < 3)
				dmg := 3;
			endif

			foreach armor in (armorsInSlot)
				start_script(
					":combat:weaponHitScripts/areadmg",
					array{
						attackInfo.GetAttacker().GetObjRef(),
						attackInfo.GetDefender().GetObjRef(),
						attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetObjRef(),
						armor.GetObjRef(),
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
					attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetObjRef(),
					armor.GetObjRef(),
					attackInfo.GetDamage().GetTotalBaseDamage(),
					dmg,
					DMG_PHYSICAL
				}
			);
		endforeach
	endif

	if (GetCooldown(attackInfo.GetAttacker().GetObjRef(), "habelemental"))
		attackInfo.GetAttacker().GetObjRef().EraseProp("#habelemental");
		var elemental := attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetObjRef().GetProp("#elementalweapon");

		foreach armor in (armorsInSlot)
			if (elemental != error)
				start_script(
					":combat:weaponHitScripts/habelemental",
					array{
						attackInfo.GetAttacker().GetObjRef(),
						attackInfo.GetDefender().GetObjRef(),
						attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetObjRef(),
						armor.GetObjRef(),
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
	
	foreach armor in (armorsInSlot)
		itemCfg.SetElementName(armor.GetObjType());
		
		var body_scripts := attackInfo.GetDefender().GetObjRef().GetProp("ArmorHitScripts");
		var armor_scripts := armor.GetObjRef().GetProp("ArmorHitScripts");
		var others_scripts := itemCfg.SetKeyName("ArmorHitScripts").GetKeyValue();

		if (attackInfo.GetDefender().IsNPC())
			others_scripts := CFGFile("combat", "npcdesc").SetElementName(attackInfo.GetDefender().GetNPCTemplate())
														  .SetKeyName("ArmorHitScripts")
														  .GetKeyValue();
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

		if (GetCooldown(attackInfo.GetDefender().GetObjRef(), "campoeletrico"))
			var e := start_script(
				":combat:armorHitScripts/campoeletrico",
				array{
					attackInfo.GetAttacker().GetObjRef(),
					attackInfo.GetDefender().GetObjRef(),
					attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetObjRef(),
					armor.GetObjRef(),
					attackInfo.GetDamage().GetTotalBaseDamage(),
					attackInfo.GetDamage().GetTotalRawDamage()
				}
			);

			if (e == error)
				Message(e).SendAbove(attackInfo.GetDefender().GetObjRef());
			endif
		endif

		armor_scripts := armor_scripts + body_scripts + others_scripts;

		foreach hitscript in armor_scripts
			var script := start_script(
				hitscript,
				array{
					attackInfo.GetAttacker().GetObjRef(),
					attackInfo.GetDefender().GetObjRef(),
					attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetObjRef(),
					armor.GetObjRef(),
					attackInfo.GetDamage().GetTotalBaseDamage(),
					attackInfo.GetDamage().GetTotalRawDamage()
				}
			);
			if (script.errortext)
				// Message("*Defender* armor script error starting [" + hitscript + "] :" + script.errortext)
				// 	.SendSys(attacker)
				// 	.SendSys(defender);
			endif
			sleepms(2);
		endforeach
	endforeach

	return 1;
endfunction