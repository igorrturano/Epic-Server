/* 
 * Este arquivo contém funções para gerenciar a aplicação das habilidades de combate.
 * Tanto as habilidades que possuem dano, como as que não.
 */

// TODO: Ver futuramente se vai precisar criar um .inc para cada habilidade

include ":combat:damage";

function IsPhysicalDamage(type)
	if (type == DMG_SLASH || type == DMG_PIERCING || type == DMG_BLUNT) return 1;
	else return 0;
	endif
endfunction

function CheckHab_Martir(byref attacker, byref defender, byref damage := 0, damageType := DMG_FORCED)
	if (GetObjProperty(defender, "#martir"))
		var martir := SystemFindObjectBySerial(GetObjProperty(defender,"martirserial"));
		if (martir && !martir.dead)
			defender := martir;
			return 1;
		endif
	endif

	return 0;
endfunction

function CheckHab_MantoRothen(byref attacker, byref defender, byref damage, damageType := DMG_FORCED)
	if (GetObjProperty(defender, "#mantorothen"))
		var healed := CInt(damage*0.3); // TODO: Ver se o valor curado será esse mesmo.
		damage := damage - healed;
		var party := defender.party;
		var amthealed := CInt(healed/party.members.size());
		if (amthealed < 1)
			amthealed := 1;
		endif
		foreach member in (party.members)
			HealDamage(member, amthealed);
		endforeach

		return 1;
	endif

	return 0;
endfunction

function CheckHab_RecuperacaoSalvadora(byref attacker, byref defender, byref damage := 0, damageType := DMG_FORCED)
	if (isBloodied(defender))
		RecalcVitals(defender);
		MakeBleed(defender);
		if (getObjProperty(defender, "#recuperacaosalvadora"))
			EraseObjProperty(defender, "#recuperacaosalvadora");
			SendSysMessageEx(defender, "Uma recuperacao salvadora salvou voce.", SSM_INFO);
			HealDamage(defender, CInt(GetPH(defender)/2));
			return 1;
		endif
	endif

	return 0;
endfunction

function CheckHab_Vinganca(byref attacker, defender, byref damage, damageType := DMG_FORCED)
	if (GetObjProperty(defender, "#vinganca"))
		SetObjProperty(defender, "#vinganca", (cint(GetObjProperty(defender, "#vinganca")) + damage));
		return 1;
	endif

	return 0;
endfunction

function CheckHab_ChainOfPain(byref attacker, defender, byref damage, damageType := DMG_FORCED)
	if( GetObjProperty(defender, "#chainofpain") != error )
		if ( CInt(GetObjProperty(defender, "#chainofpain")) > readgameclock())
			var copmobs := GetObjProperty(defender, "#chainserials");
			var mage := SystemFindObjectBySerial( GetObjProperty(defender, "#mageserial"));
			var copdefenders := array;
			
			// Get the higher skill between ARCANA and THEOLOGY
			var arcana_skill := AP_GetSkill(mage, ARCANA);
			var theology_skill := AP_GetSkill(mage, THEOLOGY);
			var skill_value := (arcana_skill > theology_skill) ? arcana_skill : theology_skill;
			var lvl := cdbl(skill_value/200);
			
			var originalDefenderSerial := defender.serial;  // Store original defender's serial

			foreach serial in copmobs
				// Only add other targets, not the original defender
				if (serial != originalDefenderSerial)
					var targ := SystemFindObjectBySerial(serial);
					if (targ)
						copdefenders.append(targ);
					endif
				endif
			endforeach
			
			// Apply damage to connected targets (excluding original)
			foreach opponent in copdefenders
				var total := Cint(damage * lvl);
				if (total < 1)
					total := 1;
				endif
				DamageFLS(opponent, total, DMG_FORCED, mage);
			endforeach

			return 1;
		endif
	endif

	return 0;
endfunction

function CheckHab_TaticasArcanas(byref attacker, byref defender, byref damage, damageType := DMG_FORCED)
	if (TemHabilidade(attacker, "Taticas Arcanas"))
		var manaatual := AP_GetVital(attacker, "MANA");
		var manatotal := AP_GetVitalMaximumValue(attacker, "MANA");
		if (manaatual >= manatotal / 2)
			var damagemod := CInt(manaatual / 20);
			damage += damagemod;
			DebugMsg(attacker, DEBUG_DAMAGE, "Dano de Habilidade[Taticas Arcanas]: " + CInt(damagemod));
			return 1;
		endif
	endif

	return 0;
endfunction

function CheckHab_BrigadorDeTaverna(byref attacker, defender, byref damage, damageType := DMG_FORCED)
	if (TemHabilidade(attacker, "Brigador de Taverna") && attacker.weapon.objtype == 0x1F020)
		var damagemod := 2;
		damage += damagemod;
		DebugMsg(attacker, DEBUG_DAMAGE, "Dano de Habilidade[Brigador de Taverna]: " + CInt(damagemod));
		return 1;
	endif

	return 0;
endfunction

function CheckHab_KaniNoSutairu(byref attacker, defender, byref damage, damageType := DMG_FORCED)
	if (IsPhysicalDamage(damageType))
		if (TemHabilidade(defender, "Kani no Sutairu"))
			var resist := RandomDiceRoll("1d4");
			damage -= resist;
			DebugMsg(defender, DEBUG_DAMAGE, "Defesa de Habilidade[Kani No Sutairu]: " + CInt(resist));
			return 1;
		endif
	endif

	return 0;
endfunction

function CheckHab_Exoesqueleto(byref attacker, byref defender, byref damage, damageType)
	if (IsPhysicalDamage(damageType))
		if (TemHabilidade(defender, "Exoesqueleto"))
			var resist := 2;
			damage -= resist;
			DebugMsg(defender, DEBUG_DAMAGE, "Defesa de Habilidade[Exoesqueleto]: " + CInt(resist));
			return 1;
		endif
	endif

	return 0;
endfunction

function CheckHab_EspecialistaEscudos(byref attacker, byref defender, byref damage, damageType)
	if (IsPhysicalDamage(damageType))
		if (TemHabilidade(defender, "Especialista em Escudos") && (defender.shield).IsA(POLCLASS_ARMOR)) // Se tiver a hab e usar escudo
			damage -= RandomDiceRoll("1d4");
			return 1;
		endif
	endif

	return 0;
endfunction

function CheckHab_Backstab(byref attacker, byref defender, byref damage, damageType)
	if (IsPhysicalDamage(damageType))
		if (GetObjProperty(attacker,  "backstab") && GetObjProperty(attacker.weapon, "backstab"))
			EraseObjProperty(attacker, "backstab");
			EraseObjProperty(attacker.weapon, "backstab");
			EraseObjProperty(attacker, "#nextcritical");
			if (!CheckHab_SempreAlerta(attacker, defender, damage, damageType))
				ataqueFurtivo(attacker, defender);
			endif
		endif
	endif

	return 0;
endfunction

function CheckHab_Furia(byref attacker, byref defender, byref damage := 0, damageType)
	if (IsPhysicalDamage(damageType))
		if (GetCooldown(attacker, "furia"))
			damage += RandomDiceRoll("1d4");
			return 1;
		endif
	endif

	return 0;
endfunction

function CheckHab_ResilienciaBarbara(byref attacker, byref defender, byref damage := 0, damageType)
	if (IsPhysicalDamage(damageType))
		if (CheckHab_Furia(defender, 0, 0, damageType) && TemHabilidade(defender, "Resiliência Bárbara")) // Antigo Pele de Rocha
			damage -= RandomDiceRoll("1d4");
			return 1;
		endif
	endif

	return 0;
endfunction

function CheckHab_SempreAlerta(byref attacker, byref defender, byref damage := 0, damageType)
	if (TemHabilidade(defender, "Sempre Alerta"))
		TS_StartTimer(attacker, "paralysis", 5);
		PrintTextAbove(defender, "*Sempre Alerta*");
		return 1;
	endif

	return 0;
endfunction

function CheckHab_Charge(byref attacker, byref defender, byref damage := 0, damageType)
	if (GetObjProperty(attacker, "charge_damage"))
		damage += Cint(GetObjProperty(attacker, "charge_damage"));

		if (hasAntiMountedWeapon(attacker, defender))
			SetObjProperty(attacker, "#ForcedTarget", attacker.serial);
			defender := attacker;
			var resisted := CheckResist(defender, FORTITUDE, RESIST_MEDIUM);
			if (!resisted)
				TS_StartTimer(defender, "desmontar", 10);
				TS_StartTimer(defender, "tombar", 5);
				TS_StartTimer(defender, "paralysis", 5);
			endif
		endif

		EraseObjProperty(attacker, "charge_damage");
	endif

	return 0;
endfunction

function CheckHab_VinculoElemental(byref attacker, byref defender, byref damage := 0, damageType := DMG_FORCED)
	if (IsPhysicalDamage(damageType))
		if (TemHabilidade(defender, "Vínculo Elemental"))
			var arcane_armor := AP_GetSkill(defender, ARCANA)/20;
			if (GetCooldown(defender, "armor_drain") > 0)
				arcane_armor += AP_GetSkill(defender, ARCANA)/30;
				PlayObjectCenteredEffectEx(defender, 0x5454, 20, 20, 0);
			endif
			if (resist < arcane_armor)
				resist := arcane_armor;
				PlayObjectCenteredEffectEx(defender, 0x5487, 1, 8, 0);
			endif
			return 1;
		endif
	endif

	return 0;
endfunction

function CheckHab_KenNoTatsujin(byref attacker, byref defender, byref damage := 0, damageType := DMG_FORCED)
	if (TemHabilidade(defender, "Ken no Tatsujin"))
		if (!defender.shield)
			printtextabove(defender, "*Ken no Tatsujin*");
			PlaySoundEffect(defender, 0x51f);
			defender.attack_once(attacker);

			return 1;
		else
			SendSysMessageEX(defender, "Você está usando escudo e falhou a habilidade", SSM_INFO);
		endif;
	endif

	return 0;
endfunction

function CheckHab_BarreiraIndestrutivel(byref attacker, byref defender, byref damage := 0, damageType := DMG_FORCED)
	if (GetCooldown(defender, "barreiraindestrutivel"))
		return 1;
	endif

	return 0;
endfunction

// TODO: Preparar para migrar a função Trespassar que está no mainHitScript para cá.
function CheckHab_Trespassar(byref attacker, byref defender, byref damage := 0, damageType := DMG_FORCED)
	if (GetObjProperty(attacker, "#trespassando") == 1)
		return 0;
	endif

	var list := array;
	foreach mobile in (ListHostiles(attacker, 1, LH_FLAG_LOS))
		if ( (mobile.serial != attacker.serial) && ( mobile.serial != defender.serial ))
			list.append(mobile);
		endif
	endforeach

	if (list.size() > 0)
		SetObjProperty(attacker, "#trespassando", 1);
		var targ := list[RandomInt(list.size())+1 ];
		if (GetVital(targ, HITS) > 0)
			PrintText(attacker, "*trespassar*");
			if (TemHabilidade(attacker, "Trespassar Critico")) // TODO: Criar função essa hab também
				SetObjProperty(attacker, "#nextcritical", 1);
			endif
			attacker.attack_once(targ);
			EraseObjProperty(attacker, "#nextcritical");
		endif
		EraseObjProperty(attacker, "#trespassando");
	endif

	return 1;
endfunction

function CheckHab_AtaqueFurtivo(byref attacker, byref defender, byref damage := 0, damageType := DMG_FORCED)
	var weapon := attacker.weapon;
	var weaponBow := 0;
	var sneak_dmg := Cint(AP_GetSkill(attacker, SNEAK));

	if (attacker.npctemplate)
		var npc_elem := NPC_GetNPCConfig(attacker);
		sneak_dmg := Cint(attacker.getTier() * 15);
		if (sneak_dmg < 15)
			sneak_dmg := 15;
		endif
	endif

	if (lower(weapon.attribute) == "rangedcombat")
		weaponBow := 1;
	endif

	if ( IsBehind(defender, attacker.x, attacker.y) )
		if ( !CanBackStab(attacker, weapon) )
			return 0;
		else
			sleepms(100);
			if ( weaponBow)
				if (!AmmoCheck(attacker, defender))
					return 1;
				endif
			else
				PerformAction(attacker, 0x000a );
			endif
        endif
    endif

	DamageFLS(defender, sneak_dmg, DMG_FORCED, attacker);
    PlaySoundEffect(defender, 0X510);
    MakeBleed(defender);
    RevokePrivilege(attacker, "hiddenattack");
    attacker.disable("hiddenattack");
endfunction

function CheckHab_ArmaduraArcana(byref attacker, byref defender, byref damage := 0, damageType := DMG_FORCED)
	var dist := Distance( attacker, defender );
	
	if (GetCooldown(defender, "freeze_enemy") && dist == 1 )
		PlaySoundEffect(attacker, 0x65b);
		PlayObjectCenteredEffectHuefx(attacker, 0x54f7, 30, 60, GELO, 0);
		TS_Starttimer(attacker, "congelamento", 5);
		return 1;
	elseif (GetCooldown(defender, "incinerate_enemy") && dist == 1)
		PlaySoundEffect(attacker, 0x5D0);
		PlayObjectCenteredEffect(attacker, 0x6d77, 7, 60);
		TS_Starttimer(attacker, "queimar", 5, 1, defender);
		return 1;
	elseif (GetCooldown(defender, "move_speed")) // Raio é pra ser acertado por flecha e ir mais rapido até o arqueiro
		PlaySoundEffect(attacker, 0x5CF);
		PlayObjectCenteredEffect(defender, 0xcdaf, 7, 30);
		TS_StartTimer(defender, "velocidade", 5);
		return 1;
	endif

	return 0;
endfunction

function CheckHab_Ainalaith(byref attacker, byref defender, byref damage := 0, damageType := DMG_FORCED)
	if ( TemHabilidade( attacker, "Ainalaith" ) )
		var arrow_effects := GetObjProperty( attacker, "#shooting_effects" );
		if ( !arrow_effects )
			arrow_effects := struct;
			arrow_effects.+fire_arrow := 0;
			arrow_effects.+cold_arrow := 0;
			arrow_effects.+energy_arrow := 0;
		endif

		var arrow_type := "";
		if ( GetObjProperty( attacker.weapon, "fire_arrow" ) )
			arrow_type := "fire_arrow";
		elseif ( GetObjProperty( attacker.weapon, "cold_arrow" ) )
			arrow_type := "cold_arrow";
		elseif ( GetObjProperty( attacker.weapon, "energy_arrow" ) )
			arrow_type := "energy_arrow";
		endif

		if ( arrow_type != "" )
			var current_count := arrow_effects[arrow_type];
			current_count += 1;
			arrow_effects[arrow_type] := current_count;

			SetObjProperty( attacker, "#shooting_effects", arrow_effects );
		endif

		return 1;
	endif

	return 0;
endfunction

function CheckHab_ConhecimentoSobreInimigo(byref attacker, byref defender, byref damage := 0, damageType := DMG_FORCED)
	if (TemHabilidade(attacker, "Conhecimento Sobre o Inimigo"))
		SetObjProperty(defender, "#attackbonus", -20);
		return 1;
  	endif

	return 0;
endfunction

function CheckHab_DeathDance(byref attacker, byref defender, byref damage := 0, damageType := DMG_FORCED)
	if (getCooldown(attacker, "deathdance") > 0)
		moveToBehind(attacker, defender);
		return 1;
	endif

	return 0;
endfunction

function CheckHab_DisparoMistico(byref attacker, byref defender, byref damage := 0, damageType := DMG_FORCED)
	var trickster_bonus := CInt( AP_GetSkill( attacker, ARCANA ) / 20 );
	if ( getCooldown( attacker, "disparomistico" ) )
		PrintTextAbovePrivate( attacker, "*passo nebuloso*", attacker );
		SetCooldown( attacker, "disparomisticoHit", trickster_bonus );
		return 1;
	endif
	
	return 0;
endfunction

function CheckHab_(byref attacker, byref defender, byref damage := 0, damageType := DMG_FORCED)

	return 0;
endfunction

function CheckHab_(byref attacker, byref defender, byref damage := 0, damageType := DMG_FORCED)

	return 0;
endfunction

function CheckHab_(byref attacker, byref defender, byref damage := 0, damageType := DMG_FORCED)

	return 0;
endfunction

function CheckHab_(byref attacker, byref defender, byref damage := 0, damageType := DMG_FORCED)

	return 0;
endfunction

function CheckHab_(byref attacker, byref defender, byref damage := 0, damageType := DMG_FORCED)

	return 0;
endfunction

function CheckHab_(byref attacker, byref defender, byref damage := 0, damageType := DMG_FORCED)

	return 0;
endfunction

function CheckHab_(byref attacker, byref defender, byref damage := 0, damageType := DMG_FORCED)

	return 0;
endfunction

