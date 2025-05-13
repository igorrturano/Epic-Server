/***********************************
 *
 * habilidades.inc
 *
 * Created: 12/05/2025 13:15PM
 * Updated: 12/05/2025 13:15PM
 *
 * Responsável por:
 * Gerenciar a aplicar os cálculos das habilidades de combate.
 * Tanto as habilidades que possuem dano, como as que não.
 *
 **********************************/

// TODO: Ver futuramente se vai precisar criar um .inc para cada habilidade
// TODO: Pensar se dá pra trocar o parâmetro damage := 0 por algo genérico, por exemplo: valRef (valor de referência para a habilidade)
	// TODO: Talvez seja melhor passar um struct como byRef. Dessa forma, cada função acessa o que lhe convém (e receber).

include "damage";
include "hitchance";
include "mainHitScript";
include "include/epicMageryUtils"; // Remover isso daqui quando ajeitar a função cagada (magia) misturada com hab. Está aqui apenas pra poder compilar e testar

enum HAB_TAGS
	HAB_SUCCESS := "ATIVADA!",
	HAB_FAIL := "FALHOU!",
	HAB_COOLDOWN := "EM RECARGA!",
endenum

function IsPhysicalDamage(type)
	if (type == DMG_SLASH || type == DMG_PIERCING || type == DMG_BLUNT) return 1;
	else return 0;
	endif
endfunction

/**
 * CheckDefenderHabsBeforeDamage(byref defender, byref damage)
 *
 * Purpose
 * Checar as habilidades que são ativadas antes de o defensor tomar o dano;
 * Criado com byRef pois algumas habilidades modificadam quem é o defensor, dano, dentro outras possibiliades.
 * 
 * Parameters
 * attacker: Ref do atacante.
 * defender: Ref do alvo que recebe o dano.
 * damage: Valor (em int) do dano.
 * 
 * Return void
 *
 */
function CheckDefenderHabsBeforeDamage(byref attacker, byref defender, damage, type)
	var resist := damage;
	
	CheckHab_Martir(attacker, defender, damage, type);
	CheckHab_MantoRothen(attacker, defender, damage, type);
	CheckHab_KaniNoSutairu(attacker, defender, damage, type);
	CheckHab_Exoesqueleto(attacker, defender, damage, type);
	CheckHab_EspecialistaEscudos(attacker, defender, damage, type);
	CheckHab_Furia(attacker, defender, damage, type);
	CheckHab_ResilienciaBarbara(attacker, defender, damage, type);
	// resist := resist - damage; // Vínculo Elemental precisa saber quanto foi resistido até agora
	CheckHab_VinculoElemental(attacker, defender, damage, type);
	CheckHab_Backstab(attacker, defender, damage, type);
	CheckHab_Charge(attacker, defender, damage, type);

	// var totaldamage := CInt(damage-resist); 
	return damage < 1 ? 1 : damage; // Garante dano mínimo = 1.
endfunction

/**
 * CheckDefenderHabsAfterDamage(byref defender, byref damage)
 *
 * Purpose
 * Checar as habilidades que são ativadas depois de o defensor tomar o dano;
 * Criado com byRef pois algumas habilidades modificadam quem é o defensor, dano, dentro outras possibiliades.
 * 
 * Parameters
 * attacker: Ref do atacante.
 * defender: Ref do alvo que recebe o dano.
 * damage: Valor (em int) do dano.
 * 
 * Return void
 *
 */
function CheckDefenderHabsAfterDamage(byRef attacker, byRef defender, damage, type)
	CheckHab_RecuperacaoSalvadora(attacker, defender, damage, type);
	CheckHab_Vinganca(attacker, defender, damage, type);
	CheckHab_ChainOfPain(attacker, defender, damage, type);
endfunction


/**
 * CheckAttackerHabsOnHit(attacker, defender, hitchance, rolldice)
 *
 * Purpose
 * Função responsável por verificar e aplicar as habilidades do atacante ao atingir um defensor.
 * Executa verificações de habilidades específicas, como Armadura Arcana, Ainalaith, Death Dance,
 * Conhecimento Sobre Inimigo e Disparo Místico. Também trata efeitos de armas elementais,
 * consumindo cargas arcanas e aplicando dano adicional se aplicável.
 *
 * Parameters
 * attacker: Entidade atacante (objeto ou identificador do atacante).
 * defender: Entidade defensora (objeto ou identificador do defensor).
 * hitchance: Chance de acerto do ataque.
 * rolldice: Valor do dado rolado para determinar o sucesso do ataque.
 *
 * Return
 * Retorna 1 após a execução das verificações e aplicações das habilidades.
 *
 */
function CheckAttackerHabsOnHit( attacker, defender, hitchance, rolldice )
	SkillChecks(attacker, defender, attacker.weapon, 1);

	CheckHab_ArmaduraArcana(attacker, defender);
	CheckHab_Ainalaith(attacker, defender);
	CheckHab_DeathDance(attacker, defender);
	CheckHab_ConhecimentoSobreInimigo(attacker, defender);
	CheckHab_DisparoMistico(attacker, defender);

	// TODO: Refatorar para uma função de MAGIA (está misturado com habilidade)
	var elemental := GetObjProperty(attacker.weapon, "#elementalweapon");
	if ( elemental != error )
		If(elemental.cargas >= 1)
			var retorno := ConsumeArcaneCharges(attacker,1);
			ApplyDamageHabElementalWeapon( attacker, defender, elemental.bonus, elemental.type , retorno.divisor);
		endif
	endif

	return 1;
endfunction

/**
 * CheckDefenderHabsOnHit(attacker, defender, hitchance, rolldice)
 *
 * Purpose
 * Verifica e aplica as habilidades defensivas do defensor quando atingido por um ataque.
 * Chama funções específicas para cada habilidade defensiva relevante e limpa a propriedade
 * "#contraataquepoderoso" do defensor, se estiver presente.
 *
 * Parameters
 * attacker: Identificador do atacante.
 * defender: Identificador do defensor.
 * hitchance: Chance de acerto do ataque.
 * rolldice: Valor do dado rolado para o ataque.
 *
 * Return
 * Retorna 1 sempre que a função é executada.
 */
function CheckDefenderHabsOnHit(attacker, defender, hitchance, rolldice)
	CheckHab_DesviarProjetil(attacker, defender);
	CheckHab_DesviarDisparos(attacker, defender);
	CheckHab_FerocidadeEnsanguentada(attacker, defender);
	CheckHab_Evasao(attacker, defender);
	CheckHab_ContraAtaque(attacker, defender);
	CheckHab_Rebuke(attacker, defender);

	// Isso aqui estava perdido no meio do código.... Tem que ver para o que serve...
	if (GetObjProperty(defender, "#contraataquepoderoso"))
		EraseObjProperty(defender, "#contraataquepoderoso");
	endif

	return 1;
endfunction

/**
 * CheckAttackerHabsOnMiss(attacker, defender, hitchance, rolldice)
 *
 * Purpose
 * Executa verificações e efeitos relacionados às habilidades do atacante quando um ataque falha.
 *
 * Parameters
 * attacker: Objeto do atacante.
 * defender: Objeto do defensor.
 * hitchance: Chance de acerto do ataque.
 * rolldice: Valor do dado rolado para o ataque.
 *
 * Return
 * Retorna 1 após executar todas as verificações e efeitos.
 */
function CheckAttackerHabsOnMiss(attacker, defender, hitchance, rolldice)
	SkillChecks(attacker, defender, attacker.weapon, 1);
	EraseObjProperty(attacker, "#shooting_effects");

	CheckHab_AtaqueInfalivel(attacker, defender);
	CheckHab_MestreDuelista(attacker, defender);
	CheckHab_Insistencia(attacker, defender);

	ArrowRecovery(attacker, defender, attacker.weapon);

	EraseObjProperty(attacker, "movement");

	if (GetObjProperty(attacker, "charge_damage"))
		EraseObjProperty(attacker, "charge_damage");
	endif

	return 1;
endfunction

/**
 * CheckDefenderHabsOnMiss(attacker, defender, hitchance, rolldice)
 *
 * Purpose
 * Função responsável por verificar e aplicar habilidades defensivas do defensor quando um ataque falha (miss).
 * Avalia condições como contra-ataque poderoso, esquiva acrobática e ataques de oportunidade, 
 * além de manipular propriedades de objetos relacionadas a essas habilidades.
 * Também executa o ataque de oportunidade caso os requisitos sejam atendidos.
 *
 * Parameters
 * attacker: objeto ou id do atacante.
 * defender: objeto ou id do defensor.
 * hitchance: chance de acerto original do ataque.
 * rolldice: valor do dado rolado para o ataque.
 *
 * Return
 * Retorna 1 sempre ao final da execução.
 */
function CheckDefenderHabsOnMiss(attacker, defender, hitchance, rolldice)
	var oportunidade := 0;
	if (GetObjProperty(defender, "#contraataquepoderoso"))
		SetObjProperty(defender, "#contraataquepoderoso", (GetObjProperty(defender, "#contraataquepoderoso")+1) );
	endif

	if (TemHabilidade(defender, "Esquiva Acrobatica"))
		SetObjProperty(defender, "#esquivaacrobatica", 1);
	endif

	if (GetObjProperty(defender, "maolivre") == 1)
		EraseObjPRoperty(defender, "maolivre");
		oportunidade := 1;
	endif

	if ( (GetCooldown(defender, "ataqueoportunidade") > 0) || oportunidade == 1)
		if (Distance(attacker,defender)<=1 && defender.weapon.attributte != "Ranged")
			if (GetObjProperty(defender, "#atacandooportunidade") == error)
				SetObjProperty(defender, "#atacandooportunidade", 1);
				printtext(defender, "*ataque de oportunidade*");
				//defender.attack_once(attacker);
				var new_hit_chance := CalculateHitChance(defender, attacker);
				var new_random := RandomInt(100)+1;
				PlayAttackAnimation(defender);
				if ( new_random < new_hit_chance )
					var armor :=  CS_GetEquipmentInArmorZone(attacker, CS_GetRandomArmorZone());
					armor := armor[ RandomInt( len( armor ) ) + 1];
					PlayHitSounds(defender, attacker);
					var time := ReadGameClock();
					var dmg := CalculateRawDamage(defender, defender.weapon);
					var result := Run_Script_To_Completion(":combat:mainHitScript", array{defender, attacker, defender.weapon, armor, dmg, dmg});
					SkillChecks(attacker, defender, wpn_a, dmg);
					oportunidade := 0;
				endif

				EraseObjProperty(defender, "#atacandooportunidade");
			endif

		elseif(Distance(attacker,defender)>1 && defender.weapon.attributte == "Ranged")
			if (GetObjProperty(defender, "#atacandooportunidade") == error)
				SetObjProperty(defender, "#atacandooportunidade", 1);
				printtext(defender, "*ataque de oportunidade*");
				//defender.attack_once(attacker);
				var new_hit_chance := CalculateHitChance(defender, attacker);
				var new_random := RandomInt(100)+1;
				PlayAttackAnimation(defender);
				if ( new_random < new_hit_chance )
					var armor :=  CS_GetEquipmentInArmorZone(attacker, CS_GetRandomArmorZone());
					armor := armor[ RandomInt( len( armor ) ) + 1];
					PlayHitSounds(defender, attacker);
					var time := ReadGameClock();
					var dmg := CalculateRawDamage(defender, defender.weapon);
					var result := Run_Script_To_Completion(":combat:mainHitScript", array{defender, attacker, defender.weapon, armor, dmg, dmg});
					SkillChecks(attacker, defender, wpn_a, dmg);
				endif

				EraseObjProperty(defender, "#atacandooportunidade");
			endif
		endif
	endif


	return 1;
endfunction


function CheckHab_Martir(byref attacker, byref defender, byref damage := 0, damageType := DMG_FORCED)
	var HAB_NAME := "Martir";
	
	if (GetObjProperty(defender, "#martir"))
		var martir := SystemFindObjectBySerial(GetObjProperty(defender,"martirserial"));
		if (martir && !martir.dead)
			defender := martir;
			DebugMsg(defender, DEBUG_HAB, $"Possui Habilidade[{HAB_NAME}]: {HAB_SUCCESS}!");
			return 1;
		endif
	endif

	return 0;
endfunction

function CheckHab_MantoRothen(byref attacker, byref defender, byref damage, damageType := DMG_FORCED)
	var HAB_NAME := "Manto de Rothen";
	
	if (GetObjProperty(defender, "#mantorothen"))
		DebugMsg(defender, DEBUG_HAB, $"Possui Habilidade[{HAB_NAME}]: {HAB_SUCCESS}!");
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
	var HAB_NAME := "Recuperacao Salvadora";
	
	if (isBloodied(defender))
		RecalcVitals(defender);
		MakeBleed(defender);
		if (getObjProperty(defender, "#recuperacaosalvadora"))
			EraseObjProperty(defender, "#recuperacaosalvadora");
			SendSysMessageEx(defender, "Uma recuperacao salvadora salvou voce.", SSM_INFO);
			DebugMsg(defender, DEBUG_HAB, $"Possui Habilidade[{HAB_NAME}]: {HAB_SUCCESS}!");
			HealDamage(defender, CInt(GetPH(defender)/2));
			return 1;
		endif
	endif

	return 0;
endfunction

function CheckHab_Vinganca(byref attacker, defender, byref damage, damageType := DMG_FORCED)
	if (GetObjProperty(defender, "#vinganca"))
		SetObjProperty(defender, "#vinganca", (cint(GetObjProperty(defender, "#vinganca")) + damage));
		DebugMsg(defender, DEBUG_HAB, $"Possui Habilidade[Vinganca]: {HAB_SUCCESS}!");
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
	var HAB_NAME := "Taticas Arcanas";
	
	if (TemHabilidade(attacker, HAB_NAME))
		var manaatual := AP_GetVital(attacker, "MANA");
		var manatotal := AP_GetVitalMaximumValue(attacker, "MANA");
		if (manaatual >= manatotal / 2)
			var damagemod := CInt(manaatual / 20);
			damage += damagemod;
			DebugMsg(attacker, DEBUG_HAB, $"Possui Habilidade[{HAB_NAME}]: {HAB_SUCCESS}!");
			DebugMsg(attacker, DEBUG_DAMAGE, $"Dano de Habilidade[{HAB_NAME}]: " + CInt(damagemod));
			return 1;
		endif
	endif

	return 0;
endfunction

function CheckHab_BrigadorDeTaverna(byref attacker, defender, byref damage, damageType := DMG_FORCED)
	var HAB_NAME := "Brigador de Taverna";
	
	if (TemHabilidade(attacker, HAB_NAME) && attacker.weapon.objtype == 0x1F020)
		var damagemod := 2;
		damage += damagemod;
		DebugMsg(attacker, DEBUG_HAB, $"Possui Habilidade[{HAB_NAME}]: {HAB_SUCCESS}!");
		DebugMsg(attacker, DEBUG_DAMAGE, $"Dano de Habilidade[{HAB_NAME}]: " + CInt(damagemod));
		return 1;
	endif

	return 0;
endfunction

function CheckHab_KaniNoSutairu(byref attacker, defender, byref damage, damageType := DMG_FORCED)
	var HAB_NAME := "Kani no Sutairu";
	
	if (IsPhysicalDamage(damageType))
		if (TemHabilidade(defender, HAB_NAME))
			var resist := RandomDiceRoll("1d4");
			damage -= resist;
			DebugMsg(defender, DEBUG_HAB, $"Possui Habilidade[{HAB_NAME}]: {HAB_SUCCESS}!");
			DebugMsg(defender, DEBUG_DAMAGE, $"Defesa de Habilidade[{HAB_NAME}]: " + CInt(resist));
			return 1;
		endif
	endif

	return 0;
endfunction

function CheckHab_Exoesqueleto(byref attacker, byref defender, byref damage, damageType)
	var HAB_NAME := "Exoesqueleto";

	if (IsPhysicalDamage(damageType))
		if (TemHabilidade(defender, HAB_NAME))
			var resist := 2;
			damage -= resist;
			DebugMsg(defender, DEBUG_HAB, $"Possui Habilidade[{HAB_NAME}]: {HAB_SUCCESS}!");
			DebugMsg(defender, DEBUG_DAMAGE, $"Defesa de Habilidade[{HAB_NAME}]: " + CInt(resist));
			return 1;
		endif
	endif

	return 0;
endfunction

function CheckHab_EspecialistaEscudos(byref attacker, byref defender, byref damage, damageType)
	var HAB_NAME := "Especialista em Escudos";
	
	if (IsPhysicalDamage(damageType))
		if (TemHabilidade(defender, HAB_NAME) && (defender.shield).IsA(POLCLASS_ARMOR)) // Se tiver a hab e usar escudo
			var resist := RandomDiceRoll("1d4");
			damage -= resist;
			DebugMsg(defender, DEBUG_HAB, $"Possui Habilidade[{HAB_NAME}]: {HAB_SUCCESS}!");
			DebugMsg(defender, DEBUG_DAMAGE, $"Defesa de Habilidade[{HAB_NAME}]: " + CInt(resist));
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
	var HAB_NAME := "Furia";
	
	if (IsPhysicalDamage(damageType))
		if (GetCooldown(attacker, "furia"))
			var damagemod := RandomDiceRoll("1d4");
			damage += damagemod;
			DebugMsg(attacker, DEBUG_HAB, $"Possui Habilidade[{HAB_NAME}]: {HAB_SUCCESS}!");
			DebugMsg(attacker, DEBUG_DAMAGE, $"Dano de Habilidade[{HAB_NAME}]: " + CInt(damagemod));
			return 1;
		endif
	endif

	return 0;
endfunction

function CheckHab_ResilienciaBarbara(byref attacker, byref defender, byref damage := 0, damageType)
	var HAB_NAME := "Resiliência Bárbara";
	
	if (IsPhysicalDamage(damageType))
		if (CheckHab_Furia(defender, 0, 0, damageType) && TemHabilidade(defender, HAB_NAME)) // Antigo Pele de Rocha
			var resist := RandomDiceRoll("1d4");
			damage -= resist;
			DebugMsg(defender, DEBUG_HAB, $"Possui Habilidade[{HAB_NAME}]: {HAB_SUCCESS}!");
			DebugMsg(defender, DEBUG_DAMAGE, $"Defesa de Habilidade[{HAB_NAME}]: " + CInt(resist));
			return 1;
		endif
	else
		DebugMsg(defender, DEBUG_HAB, $"Possui Habilidade[{HAB_NAME}]: {HAB_FAIL}!");
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
	var HAB_NAME := "Charge";
	
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
	var HAB_NAME := "Vínculo Elemental";
	
	if (IsPhysicalDamage(damageType))
		if (TemHabilidade(defender, HAB_NAME))
			var resist := 0; // TODO: Rever se esse valor vai ser assim, ou se a função vai precisar receber um parâmetro com o valor resistido até agora.
			var arcane_armor := AP_GetSkill(defender, ARCANA)/20;
			if (GetCooldown(defender, "armor_drain") > 0)
				arcane_armor += AP_GetSkill(defender, ARCANA)/30;
				PlayObjectCenteredEffectEx(defender, 0x5454, 20, 20, 0);
			endif
			if (resist < arcane_armor)
				resist := arcane_armor;
				PlayObjectCenteredEffectEx(defender, 0x5487, 1, 8, 0);
			endif

			damage -= resist;
			DebugMsg(defender, DEBUG_HAB, $"Possui Habilidade[{HAB_NAME}]: {HAB_SUCCESS}!");
			DebugMsg(defender, DEBUG_DAMAGE, $"Defesa de Habilidade[{HAB_NAME}]: " + CInt(resist));

			return 1;
		endif
	endif

	return 0;
endfunction

function CheckHab_KenNoTatsujin(byref attacker, byref defender, byref damage := 0, damageType := DMG_FORCED)
	var HAB_NAME := "Ken no Tatsujin";
	
	if (TemHabilidade(defender, HAB_NAME))
		if (!defender.shield)
			printtextabove(defender, $"*{HAB_NAME}*");
			PlaySoundEffect(defender, 0x51f);
			defender.attack_once(attacker);

			DebugMsg(defender, DEBUG_HAB, $"Possui Habilidade[{HAB_NAME}]: {HAB_SUCCESS}!");

			return 1;
		else
			SendSysMessageEX(defender, $"Você está usando escudo e falhou {HAB_NAME}", SSM_INFO);
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
	var HAB_NAME := "Armadura Arcana";
	var HAB_TYPE;
	var dist := Distance( attacker, defender );
	
	if (GetCooldown(defender, "freeze_enemy") && dist == 1 )
		var HAB_TYPE := "Congelamento";
		PlaySoundEffect(attacker, 0x65b);
		PlayObjectCenteredEffectHuefx(attacker, 0x54f7, 30, 60, GELO, 0);
		TS_Starttimer(attacker, "congelamento", 5);
		DebugMsg(defender, DEBUG_HAB, $"Possui Habilidade[{HAB_NAME} ({HAB_TYPE})]: {HAB_SUCCESS}!");
		return 1;
	elseif (GetCooldown(defender, "incinerate_enemy") && dist == 1)
		var HAB_TYPE := "Incinerar";
		PlaySoundEffect(attacker, 0x5D0);
		PlayObjectCenteredEffect(attacker, 0x6d77, 7, 60);
		TS_Starttimer(attacker, "queimar", 5, 1, defender);
		DebugMsg(defender, DEBUG_HAB, $"Possui Habilidade[{HAB_NAME} ({HAB_TYPE})]: {HAB_SUCCESS}!");
		return 1;
	elseif (GetCooldown(defender, "move_speed")) // Raio é pra ser acertado por flecha e ir mais rapido até o arqueiro
		var HAB_TYPE := "SEI LA QUE NOME E ESSE... PRINTA ISSO PRO GM AI!";
		PlaySoundEffect(attacker, 0x5CF);
		PlayObjectCenteredEffect(defender, 0xcdaf, 7, 30);
		TS_StartTimer(defender, "velocidade", 5);
		DebugMsg(defender, DEBUG_HAB, $"Possui Habilidade[{HAB_NAME} ({HAB_TYPE})]: {HAB_SUCCESS}!");
		return 1;
	endif

	return 0;
endfunction

function CheckHab_Ainalaith(byref attacker, byref defender, byref damage := 0, damageType := DMG_FORCED)
	var HAB_NAME := "Ainalaith";
	var HAB_TYPE;
	
	if (TemHabilidade(attacker, HAB_NAME))
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
			HAB_TYPE := "Flecha de Fogo";
		elseif ( GetObjProperty( attacker.weapon, "cold_arrow" ) )
			arrow_type := "cold_arrow";
			HAB_TYPE := "Flecha de Gelo";
		elseif ( GetObjProperty( attacker.weapon, "energy_arrow" ) )
			arrow_type := "energy_arrow";
			HAB_TYPE := "Flecha de Energia";
		endif

		if ( arrow_type != "" )
			var current_count := arrow_effects[arrow_type];
			current_count += 1;
			arrow_effects[arrow_type] := current_count;

			SetObjProperty( attacker, "#shooting_effects", arrow_effects );
		endif

		DebugMsg(defender, DEBUG_HAB, $"Possui Habilidade[{HAB_NAME} ({HAB_TYPE})]: {HAB_SUCCESS}!");

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

function CheckHab_ContraAtaque(byref attacker, byref defender, byref damage := 0, damageType := DMG_FORCED)
	// TODO: Revisar se esse código realmente precisar calcular toda a estrutura do dano aqui...
	
	if (GetCooldown(defender, "contraataque") > 0)
		if (GetObjProperty(defender, "#contraatacando") == error)
			SetObjProperty(defender, "#contraatacando", 1);
			PrintTextAbove(defender, "*contra ataque*");
			//defender.attack_once(attacker);
			var new_hit_chance := CalculateHitChance(defender, attacker);
			var new_random := RandomInt(100);
			PlayAttackAnimation(defender);
			if ( new_random < new_hit_chance )
				var armor :=  CS_GetEquipmentInArmorZone(attacker, CS_GetRandomArmorZone());
				armor := armor[ RandomInt( len( armor ) ) + 1];
				PlayHitSounds(defender, attacker);
				var time := ReadGameClock();
				var dmg := CalculateRawDamage(defender, defender.weapon);
				var result := Run_Script_To_Completion(":combat:mainHitScript", array{defender, attacker, defender.weapon, armor, dmg, dmg});
				SkillChecks(attacker, defender, attacker.weapon, dmg);
			endif
			EraseObjProperty(defender, "#contraatacando");

			return 1;
		endif
	endif
	return 0;
endfunction

function CheckHab_DesviarProjetil(byref attacker, byref defender, byref damage := 0, damageType := DMG_FORCED)
	if (getObjProperty(defender, "#desviarprojetil"))
		var item_cfg := ReadConfigFile(":combat:itemdesc");
		if ((item_cfg[attacker.weapon.objtype]).Projectile)
			PrintText(defender, "*esquiva-se*");
			SendSysMessageEx(defender, "O projetil foi desviado para longe de voce.", SSM_INFO);
			SendSysMessageEx(attacker, "Seu projetil foi desviado do alvo.", SSM_FAIL);
			EraseObjProperty(defender, "#desviarprojetil");
			return 1;
		endif
	endif
	
	return 0;
endfunction

function CheckHab_DesviarDisparos(byref attacker, byref defender, byref damage := 0, damageType := DMG_FORCED)
	if (getCooldown(defender, "desviardisparos") > 0)
		var item_cfg := ReadConfigFile(":combat:itemdesc");
		if ((item_cfg[attacker.weapon.objtype]).Projectile)
			PrintText(defender, "*desvia o disparo*");
			SendSysMessageEx(defender, "O projetil foi desviado para longe de voce.", SSM_INFO);
			SendSysMessageEx(attacker, "Seu projetil foi desviado do alvo.", SSM_FAIL);
			return 1;
		endif
	endif

	return 0;
endfunction

function CheckHab_MaoLivre(byref attacker, byref defender, byref damage := 0, damageType := DMG_FORCED)
	if (GetObjProperty(defender, "maolivre") == 1)
		EraseObjPRoperty(defender, "maolivre");
		SetObjProperty(defender, "evasao", 1);
		return 1;
	endif

	return 0;
endfunction

function CheckHab_FerocidadeEnsanguentada(byref attacker, byref defender, byref damage := 0, damageType := DMG_FORCED)
	if (GetCooldown(defender, "FerocidadeEnsanguentada"))
		// if (IsRaging(defender))
			if (GetObjProperty(defender, "#contraatacando") == error)
				SetObjProperty(defender, "#contraatacando", 1);
				PrintText(defender, "*contra ataque*");
				//defender.attack_once(attacker);
				var new_hit_chance := CalculateHitChance(defender, attacker);
				var new_random := RandomInt(100);
				PlayAttackAnimation(defender);
				if ( new_random < new_hit_chance )
					var armor := CS_GetEquipmentInArmorZone(attacker, CS_GetRandomArmorZone());
					armor := armor[ RandomInt( len( armor ) ) + 1];
					PlayHitSounds(defender, attacker);
					var time := ReadGameClock();
					var dmg := CalculateRawDamage(defender, defender.weapon);
					Run_Script_To_Completion(":combat:mainHitScript", array{defender, attacker, defender.weapon, armor, dmg, dmg});
				endif
				EraseObjProperty(defender, "#contraatacando");

				return 1;
			endif
		// endif
	endif

	return 0;
endfunction

function CheckHab_Evasao(byref attacker, byref defender, byref damage := 0, damageType := DMG_FORCED)
	// TODO: Passar rolldice e hitchance
	// TODO: Talvez esse código não precise ser poluído desse jeito, calculando e executando toda a estrutura do ataque....
		// TODO: Ver se dá pra executar o ataque através do Attack do combatHook...
	// Os 2 valores estão 0 apenas pra poder compilar, mas a função tem que receber
	var rolldice := 0;
	var hitchance := 0;
	
	if (Cint(GetObjProperty(defender, "evasao")))
		PrintText(defender, "*esquiva-se*");
		EraseObjProperty(defender, "evasao");
		if (TemHabilidade(defender, "Esquiva Oportunista"))
			if (GetObjProperty(defender, "#atacandooportunidade") == error)
				SetObjProperty(defender, "#atacandooportunidade", 1);
				printtext(defender, "*ataque de oportunidade*");
				//defender.attack_once(attacker);
				var new_hit_chance := CalculateHitChance(defender, attacker);
				var new_random := RandomInt(100);
				PlayAttackAnimation(defender);
				if ( new_random < new_hit_chance )
					var armor :=  CS_GetEquipmentInArmorZone(attacker, CS_GetRandomArmorZone());
					armor := armor[ RandomInt( len( armor ) ) + 1];
					PlayHitSounds(defender, attacker);
					var time := ReadGameClock();
					var dmg := CalculateRawDamage(defender, defender.weapon);
					Run_Script_To_Completion(":combat:mainHitScript", array{defender, attacker, defender.weapon, armor, dmg, dmg});
					SkillChecks(attacker, defender, attacker.weapon, dmg);
				endif

			EraseObjProperty(defender, "#atacandooportunidade");
			endif
		endif

		return 1;
	elseif ((GetObjProperty(defender, "#mobilidade") == 1) && ((rolldice-hitchance) < 16)) //Esquivou por conta do jogo de pés da "Mobilidade em Combate"
		printtext(defender, "*esquiva com jogo de pés*");
		if (TemHabilidade(defender, "Esquiva Oportunista"))
			if (GetObjProperty(defender, "#atacandooportunidade") == error)
				SetObjProperty(defender, "#atacandooportunidade", 1);
				printtext(defender, "*ataque de oportunidade*");
				//defender.attack_once(attacker);
				var new_hit_chance := CalculateHitChance(defender, attacker);
				var new_random := RandomInt(100);
				PlayAttackAnimation(defender);
				if ( new_random < new_hit_chance )
					var armor :=  CS_GetEquipmentInArmorZone(attacker, CS_GetRandomArmorZone());
					armor := armor[ RandomInt( len( armor ) ) + 1];
					PlayHitSounds(defender, attacker);
					var time := ReadGameClock();
					var dmg := CalculateRawDamage(defender, defender.weapon);
					Run_Script_To_Completion(":combat:mainHitScript", array{defender, attacker, defender.weapon, armor, dmg, dmg});
					SkillChecks(attacker, defender, attacker.weapon, dmg);
				endif
				EraseObjProperty(defender, "#atacandooportunidade");
				return 1;
			endif
		endif
	endif

	return 0;
endfunction

function CheckHab_Rebuke(byref attacker, byref defender, byref damage := 0, damageType := DMG_FORCED)
	if (GetObjProperty(defender, "#campeaodivino"))
        // Apply divine retaliation
        Start_Script(":charactercreation:habilidades/paladin/golpedivino", array{defender, attacker});
		
		return 1;
    endif
	return 0;
endfunction

function CheckHab_AtaqueInfalivel(byref attacker, byref defender, byref damage := 0, damageType := DMG_FORCED)
	if (GetCooldown(attacker, "ataqueinfalivel") > 0)
		printtext(attacker, "*ataque infalivel*");
		var armor :=  CS_GetEquipmentInArmorZone(attacker, CS_GetRandomArmorZone());
		armor := armor[ RandomInt( len( armor ) ) + 1];
		PlayHitSounds(defender, attacker);
		var dmg := CalculateRawDamage(attacker, attacker.weapon);
		dmg := dmg*0.33;
		var result := Run_Script_To_Completion(":combat:mainHitScript", array{attacker, defender, wpn_a, armor, dmg, dmg});
		SkillChecks(attacker, defender, attacker.weapon, dmg);

		return 1;
	endif

	return 0;
endfunction

function CheckHab_MestreDuelista(byref attacker, byref defender, byref damage := 0, damageType := DMG_FORCED)
	if (GetCooldown(attacker, "mestreduelista") > 0)
		printtext(attacker, "*mestre duelista*");
		var armor :=  CS_GetEquipmentInArmorZone(attacker, CS_GetRandomArmorZone());
		armor := armor[ RandomInt( len( armor ) ) + 1];
		PlayHitSounds(defender, attacker);
		var dmg := CalculateRawDamage(attacker, attacker.weapon);
		dmg := dmg;
		var result := Run_Script_To_Completion(":combat:mainHitScript", array{attacker, defender, wpn_a, armor, dmg, dmg});
		SkillChecks(attacker, defender, attacker.weapon, dmg);
		
		return 1;
	endif

	return 0;
endfunction

function CheckHab_Insistencia(byref attacker, byref defender, byref damage := 0, damageType := DMG_FORCED)
	if (TemHabilidade(attacker, "Insistencia"))
		SetObjProperty(attacker, "insistencia", 1);
	endif

	return 0;
endfunction

// function CheckHab_(byref attacker, byref defender, byref damage := 0, damageType := DMG_FORCED)

// 	return 0;
// endfunction

// function CheckHab_(byref attacker, byref defender, byref damage := 0, damageType := DMG_FORCED)

// 	return 0;
// endfunction

// function CheckHab_(byref attacker, byref defender, byref damage := 0, damageType := DMG_FORCED)

// 	return 0;
// endfunction

// function CheckHab_(byref attacker, byref defender, byref damage := 0, damageType := DMG_FORCED)

// 	return 0;
// endfunction

// function CheckHab_(byref attacker, byref defender, byref damage := 0, damageType := DMG_FORCED)

// 	return 0;
// endfunction

// function CheckHab_(byref attacker, byref defender, byref damage := 0, damageType := DMG_FORCED)

// 	return 0;
// endfunction

