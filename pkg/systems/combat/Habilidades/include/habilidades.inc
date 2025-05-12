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

//Template para as habilidades
// function CheckHab_(byref attacker, byref defender, byref damage, damageType)
// endfunction

function CheckHab_Martir(byref attacker, byref defender, byref damage, damageType)
	if (GetObjProperty(defender, "#martir"))
		var martir := SystemFindObjectBySerial(GetObjProperty(defender,"martirserial"));
		if (martir && !martir.dead)
			defender := martir;
		endif
	endif

	return CInt(damage);
endfunction

function CheckHab_MantoRothen(byref attacker, byref defender, byref damage, damageType)
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

function CheckHab_RecuperacaoSalvadora(byref attacker, byref defender, byref damage, damageType)
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

function CheckHab_Vinganca(attacker, defender, byref damage, damageType)
	if (GetObjProperty(defender, "#vinganca"))
		SetObjProperty(defender, "#vinganca", (cint(GetObjProperty(defender, "#vinganca")) + damage));
		return 1;
	endif

	return 0;
endfunction

function CheckHab_ChainOfPain(attacker, defender, byref damage, damageType)
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

function CheckHab_TaticasArcanas(byref attacker, byref defender, byref damage, damageType)
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

function CheckHab_BrigadorDeTaverna(attacker, defender, byref damage, damageType)
	if (TemHabilidade(attacker, "Brigador de Taverna") && attacker.weapon.objtype == 0x1F020)
		var damagemod := 2;
		damage += damagemod;
		DebugMsg(attacker, DEBUG_DAMAGE, "Dano de Habilidade[Brigador de Taverna]: " + CInt(damagemod));
		return 1;
	endif

	return 0;
endfunction

function CheckHab_KaniNoSutairu(attacker, defender, byref damage, damageType)
	if (IsPhysicalDamage(damageType))
		if (TemHabilidade(defender, "Kani no Sutairu"))
			damage -= RandomDiceRoll("1d4");
			DebugMsg(defender, DEBUG_DAMAGE, "Defesa de Habilidade[Kani No Sutairu]: " + CInt(resist));
			return 1;
		endif
	endif

	return 0;
endfunction

function CheckHab_Exoesqueleto(byref attacker, byref defender, byref damage, damageType)
	if (IsPhysicalDamage(damageType))
		if (TemHabilidade(defender, "Exoesqueleto"))
			damage -= 2;
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

function CheckHab_Furia(byref attacker, byref defender, byref damage, damageType)
	if (IsPhysicalDamage(damageType))
		if (GetCooldown(attacker, "furia"))
			damage += RandomDiceRoll("1d4");
			return 1;
		endif
	endif

	return 0;
endfunction

function CheckHab_ResilienciaBarbara(byref attacker, byref defender, byref damage, damageType)
	if (IsPhysicalDamage(damageType))
		if (CheckHab_Furia(defender, 0, 0, damageType) && TemHabilidade(defender, "Resiliência Bárbara")) // Antigo Pele de Rocha
			damage -= RandomDiceRoll("1d4");
			return 1;
		endif
	endif

	return 0;
endfunction

function CheckHab_SempreAlerta(byref attacker, byref defender, byref damage, damageType)
	if (TemHabilidade(defender, "Sempre Alerta"))
		TS_StartTimer(attacker, "paralysis", 5);
		PrintTextAbove(defender, "*Sempre Alerta*");
		return 1;
	endif

	return 0;
endfunction

function CheckHab_Charge(byref attacker, byref defender, byref damage, damageType)
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

function CheckHab_VinculoElemental(byref attacker, byref defender, byref damage, damageType)
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
		endif
	endif
endfunction

function CheckHab_(byref attacker, byref defender, byref damage, damageType)
endfunction

function CheckHab_(byref attacker, byref defender, byref damage, damageType)
endfunction

function CheckHab_(byref attacker, byref defender, byref damage, damageType)
endfunction
