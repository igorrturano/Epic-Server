/***********************************
 *
 * habilidades.inc
 *
 * Created: 12/05/2025 13:15PM
 * Updated: 13/09/2025 05:15PM
 *
 * Responsável por:
 * Gerenciar a aplicar os cálculos das habilidades de combate.
 * Tanto as habilidades que possuem dano, como as que não.
 *
 **********************************/

// TODO: Depois de finalizar o combate refatorar isso novamente, só para encapsular as habilidades em CombatAbility

include "DamageController";
include "HitController";
include "SkillController";
include "FXController";
include ":combat:Model/CombatAbility";

// include ":message:Model/Message";
include ":charactercreation:resists";
include ":blood:blood";
// include "include/epicMageryUtils"; // Remover isso daqui quando ajeitar a função cagada (magia) misturada com hab. Está aqui apenas pra poder compilar e testar

enum class HAB_STATUS
	SUCCESS := "ATIVADA!",
	FAIL := "FALHOU!",
	COOLDOWN := "EM RECARGA!",
endenum

enum class TARGET
	BOTH,
	ATTACKER,
	DEFENDER
endenum

function ExecuteCombatAbilitiesBeforeDamage(byRef attackInfo)
endfunction

/**
 * CheckAttackerHabsOnHit(attacker, defender, hitchance, rolldice)
 *
 * Purpose
 * Função responsável por verificar e aplicar as habilidades do atacante ao atingir um defensor.
 *
 * Parameters
 * attacker: Entidade atacante (objeto ou identificador do atacante).
 * defender: Entidade defensora (objeto ou identificador do defensor).
 * attackInfo: Struct contendo as informações necessárias que as habs precisam tratar.
 *
 * Return
 * Retorna 1 após a execução das verificações e aplicações das habilidades.
 *
 */
function ExecuteCombatAbilitiesOnHit(byRef attackInfo)
	CheckHab_ArmaduraArcana(attackInfo);
	CheckHab_Ainalaith(attackInfo);
	CheckHab_DeathDance(attackInfo);
	CheckHab_ConhecimentoSobreInimigo(attackInfo);
	CheckHab_DisparoMistico(attackInfo);

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

function ExecuteCombatAbilitiesAfterDamage(byRef attackInfo)
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
 * attackInfo: Struct contendo as informações necessárias que as habs precisam tratar.
 *
 * Return
 * Retorna 1 após executar todas as verificações e efeitos.
 */
function ExecuteCombatAbilitiesOnMiss(byRef attackInfo)
	CheckHab_AtaqueInfalivel(attackInfo);
	CheckHab_MestreDuelista(attackInfo);
	CheckHab_Insistencia(attackInfo);

	EraseObjProperty(attackInfo.GetAttacker().GetObjRef(), "movement");
	EraseObjProperty(attackInfo.GetAttacker().GetObjRef(), "#shooting_effects");
	EraseObjProperty(attackInfo.GetAttacker().GetObjRef(), "charge_damage"); // TODO: Ver pra quê que serve esse troço

	return 1;
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
 * attackInfo: Struct contendo as informações necessárias que as habs precisam tratar.
 *
 * Return void
 *
 */
function CheckDefenderHabsBeforeDamage(byRef attackInfo)
	CheckHab_Martir(attackInfo);
	CheckHab_MantoRothen(attackInfo);
	CheckHab_KaniNoSutairu(attackInfo);
	CheckHab_Exoesqueleto(attackInfo);
	CheckHab_EspecialistaEscudos(attackInfo);
	CheckHab_Furia(attackInfo);
	CheckHab_ResilienciaBarbara(attackInfo);
	CheckHab_VinculoElemental(attackInfo);
	CheckHab_Backstab(attackInfo);
	CheckHab_Charge(attackInfo);
    CheckHab_DivineAuthority(attackInfo);
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
 * attackInfo: Struct contendo as informações necessárias que as habs precisam tratar.
 *
 * Return
 * Retorna 1 sempre que a função é executada.
 */
function CheckDefenderHabsOnHit(byRef attackInfo)
	CheckHab_DesviarProjetil(attackInfo);
	CheckHab_DesviarDisparos(attackInfo);
	CheckHab_FerocidadeEnsanguentada(attackInfo);
	CheckHab_Evasao(attackInfo);
	CheckHab_ContraAtaque(attackInfo);
	CheckHab_Rebuke(attackInfo);

	// Isso aqui estava perdido no meio do código.... Tem que ver para o que serve...
	if (GetObjProperty(attackInfo.GetDefender().GetObjRef(), "#contraataquepoderoso"))
		EraseObjProperty(attackInfo.GetDefender().GetObjRef(), "#contraataquepoderoso");
	endif
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
 * attackInfo: Struct contendo as informações necessárias que as habs precisam tratar.
 * 
 * Return void
 *
 */
function CheckDefenderHabsAfterDamage(byRef attackInfo)
	CheckHab_RecuperacaoSalvadora(attackInfo);
	CheckHab_Vinganca(attackInfo);
	CheckHab_ChainOfPain(attackInfo);
endfunction

/**
 * CheckDefenderHabsOnMiss(attacker, defender, hitchance, rolldice)
 *
 * Purpose
 * Função responsável por verificar e aplicar habilidades defensivas do defensor quando um ataque falha (miss).
 *
 * Parameters
 * attacker: objeto ou id do atacante.
 * defender: objeto ou id do defensor.
 * attackInfo: Struct contendo as informações necessárias que as habs precisam tratar.
 *
 * Return
 * Retorna 1 sempre ao final da execução.
 */
function CheckDefenderHabsOnMiss(byRef attackInfo)
	// TODO: Adicionar habilidades que serão incluídas aqui
endfunction

function CheckHab_Martir(byRef attackInfo)
	if (attackInfo.GetDefender().GetObjRef().GetProp("#martir"))
		var martir := SystemFindObjectBySerial(attackInfo.GetDefender().GetObjRef().GetProp("martirserial"));
		if (martir && !martir.dead)
			attackInfo.SetDefender(martir);
			DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::MARTIR}]: {HAB_STATUS::SUCCESS}!");
			return 1;
		endif
	endif

	return 0;
endfunction

function CheckHab_MantoRothen(byRef attackInfo)
	if (attackInfo.GetDefender().GetObjRef().GetProp("#mantorothen"))
		var healed := CInt(attackInfo.GetDamage().GetTotalBaseDamage() * 0.3); // TODO: Ver se o valor curado será esse mesmo.
		var party := attackInfo.GetDefender().GetObjRef().party;
		var amthealed := CInt(healed/party.members.size());
		// attackInfo.damage := attackInfo.damage - healed;

		if (amthealed < 1)
			amthealed := 1;
		endif

		foreach member in (party.members)
			HealDamage(member, amthealed);
		endforeach

		DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::MANTO_ROTHEN}]: {HAB_STATUS::SUCCESS}!");
		return 1;
	endif

	return 0;
endfunction

function CheckHab_RecuperacaoSalvadora(byRef attackInfo)
	var HAB_NAME := "Recuperacao Salvadora";
	
	if (isBloodied(attackInfo.GetDefender().GetObjRef()))
		attackInfo.GetDefender().Update();
		MakeBleed(attackInfo.GetDefender().GetObjRef());
		if (attackInfo.GetDefender().GetObjRef().GetProp("#recuperacaosalvadora"))
			attackInfo.GetDefender().GetObjRef().EraseProp("#recuperacaosalvadora");
			Message("Uma recuperacao salvadora salvou voce.", FONTCOLOR_SUCCESS).SendSys(attackInfo.GetDefender().GetObjRef());
			HealDamage(attackInfo.GetDefender().GetObjRef(), CInt(GetPH(attackInfo.GetDefender().GetObjRef())/2));

			DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{HAB_NAME}]: {HAB_STATUS::SUCCESS}!");
			return true;
		endif
	endif

	return false;
endfunction

function CheckHab_Vinganca(byRef attackInfo)
	if (attackInfo.GetDefender().GetObjRef().GetProp("#vinganca"))
		attackInfo.GetDefender().GetObjRef().SetProp(
			"#vinganca",
			CInt(attackInfo.GetDefender().GetObjRef().GetProp("#vinganca")) + attackInfo.GetDamage().GetTotalBaseDamage()
		);
		
		DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[Vinganca]: {HAB_STATUS::SUCCESS}!");
		return true;
	endif

	return false;
endfunction

function CheckHab_ChainOfPain(byRef attackInfo)
	if(attackInfo.GetDefender().GetObjRef().GetProp("#chainofpain") != error )
		if (CInt(attackInfo.GetDefender().GetObjRef().GetProp("#chainofpain")) > readgameclock())
			var copmobs := attackInfo.GetDefender().GetObjRef().GetProp("#chainserials");
			var mage := MobileWrapper(SystemFindObjectBySerial(attackInfo.GetDefender().GetObjRef().GetProp("#mageserial")));
			var copdefenders := array;
			
			// Get the higher skill between ARCANA and THEOLOGY
			var arcana_skill := mage.GetSkill(ARCANA);
			var theology_skill := mage.GetSkill(THEOLOGY);
			var skill_value := (arcana_skill > theology_skill) ? arcana_skill : theology_skill;
			var lvl := CDbl(skill_value / 200);
			
			var originalDefenderSerial := attackInfo.GetDefender().GetSerial(); // Store original defender's serial

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
				var total := Cint(attackInfo.damage * lvl);
				if (total < 1)
					total := 1;
				endif

				// DamageFLS(opponent, total, DMG_FORCED, mage); // TODO: Refatorar isso daqui para o dano com uso de classe
			endforeach

			return 1;
		endif
	endif

	return 0;
endfunction

function CheckHab_TaticasArcanas(byRef attackInfo)
	if (attackInfo.GetAttacker().HasAbility(COMBAT_ABILITY::TATICAS_ARCANAS))
		var manaatual := attackInfo.GetAttacker().GetCurrentMana();
		var manatotal := attackInfo.GetAttacker().GetMaximumMana();
		if (manaatual >= manatotal / 2)
			var dmg := attackInfo.GetDamage();
			var damagemod := CInt(manaatual / 20);

			if (dmg.GetSlashBaseDamage() > 0)
				dmg.IncreaseSlashBaseDamage(damagemod);
			elseif (dmg.GetPiercingBaseDamage() > 0)
				dmg.IncreasePiercingBaseDamage(damagemod);
			elseif (dmg.GetBluntBaseDamage() > 0)
				dmg.IncreaseBluntBaseDamage(damagemod);
			endif

			attackInfo.SetDamage(dmg);
			
			DebugMsg(attackInfo.GetAttacker().ObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::TATICAS_ARCANAS}]: {HAB_STATUS::SUCCESS}!");
			DebugMsg(attackInfo.GetAttacker().ObjRef(), DEBUG_DAMAGE, $"Dano de Habilidade[{COMBAT_ABILITY::TATICAS_ARCANAS}]: " + CInt(damagemod));
			return true;
		endif
	endif

	return false;
endfunction

function CheckHab_BrigadorDeTaverna(byRef attackInfo)
	if (attackInfo.GetAttacker().HasAbility(COMBAT_ABILITY::BRIGADOR_DE_TAVERNA) &&
		attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetCategory() == WEAPON_CATEGORY::UNARMED
		)
		var damageMod := 2;

		attackInfo.SetDamage(attackInfo.GetDamage().IncreaseBluntBaseDamage(damageMod));
		
		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::BRIGADOR_DE_TAVERNA}]: {HAB_STATUS::SUCCESS}!");
		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"Dano de Habilidade[{COMBAT_ABILITY::BRIGADOR_DE_TAVERNA}]: " + CInt(damageMod));
		return true;
	endif

	return false;
endfunction

function CheckHab_KaniNoSutairu(byRef attackInfo)
	if (attackInfo.GetDefender().HasAbility(COMBAT_ABILITY::KANI_NO_SUTAIRU))
		var resist := Dice("1d4").Roll().GetResult();
		var dmg := attackInfo.GetDamage();

		if (dmg.GetSlashBaseDamage() > 0)
			dmg.DecreaseSlashBaseDamage(resist);
		elseif (dmg.GetPiercingBaseDamage() > 0)
			dmg.DecreasePiercingBaseDamage(resist);
		elseif (dmg.GetBluntBaseDamage() > 0)
			dmg.DecreaseBluntBaseDamage(resist);
		endif

		attackInfo.SetDamage(dmg);
		
		DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::KANI_NO_SUTAIRU}]: {HAB_STATUS::SUCCESS}!");
		DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_DAMAGE, $"Defesa de Habilidade[{COMBAT_ABILITY::KANI_NO_SUTAIRU}]: " + CInt(resist));
		return true;
	endif

	return false;
endfunction

function CheckHab_Exoesqueleto(byRef attackInfo)
	if (attackInfo.GetDefender().HasAbility(COMBAT_ABILITY::EXOESQUELETO))
		var resist := 2;
		var dmg := attackInfo.GetDamage();

		if (dmg.GetSlashBaseDamage() > 0)
			dmg.DecreaseSlashBaseDamage(resist);
		elseif (dmg.GetPiercingBaseDamage() > 0)
			dmg.DecreasePiercingBaseDamage(resist);
		elseif (dmg.GetBluntBaseDamage() > 0)
			dmg.DecreaseBluntBaseDamage(resist);
		endif

		attackInfo.SetDamage(dmg);

		DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::EXOESQUELETO}]: {HAB_STATUS::SUCCESS}!");
		DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_DAMAGE, $"Defesa de Habilidade[{COMBAT_ABILITY::EXOESQUELETO}]: " + CInt(resist));
		return true;
	endif

	return false;
endfunction

function CheckHab_EspecialistaEscudos(byRef attackInfo)
	if (attackInfo.GetDefender().HasAbility(COMBAT_ABILITY::ESPECIALISTA_ESCUDOS) &&
		attackInfo.GetDefender().GetEquipBySlot(SHIELD).GetObjRef()) // Tem que estar com escudo equipado
		var resist := Dice("1d4").Roll().GetResult();
		var dmg := attackInfo.GetDamage();

		if (dmg.GetSlashBaseDamage() > 0)
			dmg.DecreaseSlashBaseDamage(resist);
		elseif (dmg.GetPiercingBaseDamage() > 0)
			dmg.DecreasePiercingBaseDamage(resist);
		elseif (dmg.GetBluntBaseDamage() > 0)
			dmg.DecreaseBluntBaseDamage(resist);
		endif

		attackInfo.SetDamage(dmg);

		DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::ESPECIALISTA_ESCUDOS}]: {HAB_STATUS::SUCCESS}!");
		DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_DAMAGE, $"Defesa de Habilidade[{COMBAT_ABILITY::ESPECIALISTA_ESCUDOS}]: " + CInt(resist));
		return true;
	endif

	return false;
endfunction

function CheckHab_Backstab(byRef attackInfo)
	if (GetObjProperty(attacker,  "backstab") && GetObjProperty(attacker.weapon, "backstab"))
		EraseObjProperty(attacker, "backstab");
		EraseObjProperty(attacker.weapon, "backstab");
		EraseObjProperty(attacker, "#nextcritical");
		if (!CheckHab_SempreAlerta(attacker, defender, attackInfo))
			CheckHab_AtaqueFurtivo(attacker, defender);
		endif

		return true;
	endif

	return false;
endfunction

function CheckHab_Furia(byRef attackInfo)
	if (GetCooldown(attackInfo.GetAttacker().GetObjRef(), COMBAT_ABILITY::FURIA))
		var damagemod := Dice("1d4").Roll().GetResult();
		attackInfo.damage += damagemod;

		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::FURIA}]: {HAB_STATUS::SUCCESS}!");
		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"Dano de Habilidade[{COMBAT_ABILITY::FURIA}]: " + CInt(damagemod));
		return true;
	endif

	return false;
endfunction

function CheckHab_ResilienciaBarbara(byRef attackInfo)
	var HAB_NAME := "Resiliência Bárbara";
	
	if (IsPhysicalDamage(attackInfo.type))
		if (CheckHab_Furia(defender, 0, attackInfo) && TemHabilidade(defender, HAB_NAME)) // Antigo Pele de Rocha
			var resist := RandomDiceRoll("1d4");
			attackInfo.damage -= resist;
			DebugMsg(defender, DEBUG_HAB, $"Possui Habilidade[{HAB_NAME}]: {HAB_STATUS::SUCCESS}!");
			DebugMsg(defender, DEBUG_DAMAGE, $"Defesa de Habilidade[{HAB_NAME}]: " + CInt(resist));
			return 1;
		endif
	else
		DebugMsg(defender, DEBUG_HAB, $"Possui Habilidade[{HAB_NAME}]: {HAB_STATUS::FAIL}!");
	endif

	return 0;
endfunction

function CheckHab_SempreAlerta(byRef attackInfo)
	if (TemHabilidade(defender, "Sempre Alerta"))
		TS_StartTimer(attacker, "paralysis", 5);
		PrintTextAbove(defender, "*Sempre Alerta*");
		return 1;
	endif

	return 0;
endfunction

function CheckHab_Charge(byRef attackInfo)
	var HAB_NAME := "Charge";
	
	if (GetObjProperty(attacker, "charge_damage"))
		attackInfo.damage += Cint(GetObjProperty(attacker, "charge_damage"));

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

function CheckHab_VinculoElemental(byRef attackInfo)
	var HAB_NAME := "Vínculo Elemental";
	
	if (IsPhysicalDamage(attackInfo.type))
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

			attackInfo.damage -= resist;
			DebugMsg(defender, DEBUG_HAB, $"Possui Habilidade[{HAB_NAME}]: {HAB_STATUS::SUCCESS}!");
			DebugMsg(defender, DEBUG_DAMAGE, $"Defesa de Habilidade[{HAB_NAME}]: " + CInt(resist));

			return 1;
		endif
	endif

	return 0;
endfunction

function CheckHab_KenNoTatsujin(byRef attackInfo)
	if (!attackInfo.GetDefender().HasAbility(COMBAT_ABILITY::KEN_NO_TATSUJIN))
		return false;
	endif

	if (attackInfo.GetDefender().GetEquipBySlot(SHIELD))
		Message($"Você está usando escudo e falhou {COMBAT_ABILITY::KEN_NO_TATSUJIN}", FONTCOLOR_FAIL).SendSys(attackInfo.GetDefender().GetObjRef());
		DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::KEN_NO_TATSUJIN}]: {HAB_STATUS::FAIL}!");
		return false;
	endif

	Message($"*{COMBAT_ABILITY::KEN_NO_TATSUJIN}*").SendAbove(attackInfo.GetDefender().GetObjRef());
	PlaySoundEffect(attackInfo.GetDefender().GetObjRef(), 0x51f);
	attackInfo.GetDefender().GetObjRef().attack_once(attackInfo.GetAttacker().GetObjRef());
	DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::KEN_NO_TATSUJIN}]: {HAB_STATUS::SUCCESS}!");

	return true;
endfunction

function CheckHab_BarreiraIndestrutivel(byRef attackInfo)
	if (GetCooldown(attackInfo.GetDefender().GetObjRef(), "barreiraindestrutivel"))
		return true;
	endif

	return false;
endfunction

// TODO: Preparar para migrar a função Trespassar que está no mainHitScript para cá.
function CheckHab_Trespassar(byRef attackInfo)
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
			PrintTextAbove(attacker, "*trespassar*");
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

function CheckHab_AtaqueFurtivo(byRef attackInfo)
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
		if ( !CanBackStab(attackInfo.GetAttacker()) )
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

function CheckHab_ArmaduraArcana(byRef attackInfo)
	var HAB_NAME := "Armadura Arcana";
	var HAB_TYPE;
	var dist := Distance( attacker, defender );
	
	if (GetCooldown(defender, "freeze_enemy") && dist == 1 )
		var HAB_TYPE := "Congelamento";
		PlaySoundEffect(attacker, 0x65b);
		PlayObjectCenteredEffectHuefx(attacker, 0x54f7, 30, 60, COLOR_ELEMENTAL_ICE, 0);
		TS_Starttimer(attacker, "congelamento", 5);
		DebugMsg(defender, DEBUG_HAB, $"Possui Habilidade[{HAB_NAME} ({HAB_TYPE})]: {HAB_STATUS::SUCCESS}!");
		return 1;
	elseif (GetCooldown(defender, "incinerate_enemy") && dist == 1)
		var HAB_TYPE := "Incinerar";
		PlaySoundEffect(attacker, 0x5D0);
		PlayObjectCenteredEffect(attacker, 0x6d77, 7, 60);
		TS_Starttimer(attacker, "queimar", 5, 1, defender);
		DebugMsg(defender, DEBUG_HAB, $"Possui Habilidade[{HAB_NAME} ({HAB_TYPE})]: {HAB_STATUS::SUCCESS}!");
		return 1;
	elseif (GetCooldown(defender, "move_speed")) // Raio é pra ser acertado por flecha e ir mais rapido até o arqueiro
		var HAB_TYPE := "SEI LA QUE NOME E ESSE... PRINTA ISSO PRO GM AI!";
		PlaySoundEffect(attacker, 0x5CF);
		PlayObjectCenteredEffect(defender, 0xcdaf, 7, 30);
		TS_StartTimer(defender, "velocidade", 5);
		DebugMsg(defender, DEBUG_HAB, $"Possui Habilidade[{HAB_NAME} ({HAB_TYPE})]: {HAB_STATUS::SUCCESS}!");
		return 1;
	endif

	return 0;
endfunction

function CheckHab_Ainalaith(byRef attackInfo)
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

		DebugMsg(defender, DEBUG_HAB, $"Possui Habilidade[{HAB_NAME} ({HAB_TYPE})]: {HAB_STATUS::SUCCESS}!");

		return 1;
	endif

	return 0;
endfunction

function CheckHab_ConhecimentoSobreInimigo(byRef attackInfo)
	if (TemHabilidade(attacker, "Conhecimento Sobre o Inimigo"))
		SetObjProperty(defender, "#attackbonus", -20);
		return 1;
  	endif

	return 0;
endfunction

function CheckHab_DeathDance(byRef attackInfo)
	if (getCooldown(attacker, "deathdance") > 0)
		moveToBehind(attacker, defender);
		return 1;
	endif

	return 0;
endfunction

function CheckHab_DisparoMistico(byRef attackInfo)
	var trickster_bonus := CInt( AP_GetSkill( attacker, ARCANA ) / 20 );
	if ( getCooldown( attacker, "disparomistico" ) )
		PrintTextPrivate( attacker, "*passo nebuloso*", attacker );
		SetCooldown( attacker, "disparomisticoHit", trickster_bonus );
		return 1;
	endif
	
	return 0;
endfunction

function CheckHab_ContraAtaque(byRef attackInfo)
	// TODO: Revisar se esse código realmente precisar calcular toda a estrutura do dano aqui...
	
	if (GetCooldown(defender, "contraataque") > 0)
		if (GetObjProperty(defender, "#contraatacando") == error)
			SetObjProperty(defender, "#contraatacando", 1);
			PrintTextAbove(defender, "*contra ataque*");
			//defender.attack_once(attacker);
			var new_hit_chance := CalculateHitChance(defender, attacker);
			var new_random := RandomInt(100);
			PlayAttackFX(defender);
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

function CheckHab_DesviarProjetil(byRef attackInfo)
	if (getObjProperty(defender, "#desviarprojetil"))
		var item_cfg := ReadConfigFile(":combat:itemdesc");
		if ((item_cfg[attacker.weapon.objtype]).Projectile)
			PrintTextAbove(defender, "*esquiva-se*");
			SendSysMessageEx(defender, "O projetil foi desviado para longe de voce.", SSM_INFO);
			SendSysMessageEx(attacker, "Seu projetil foi desviado do alvo.", SSM_FAIL);
			EraseObjProperty(defender, "#desviarprojetil");
			return 1;
		endif
	endif
	
	return 0;
endfunction

function CheckHab_DesviarDisparos(byRef attackInfo)
	if (getCooldown(defender, "desviardisparos") > 0)
		var item_cfg := ReadConfigFile(":combat:itemdesc");
		if ((item_cfg[attacker.weapon.objtype]).Projectile)
			PrintTextAbove(defender, "*desvia o disparo*");
			SendSysMessageEx(defender, "O projetil foi desviado para longe de voce.", SSM_INFO);
			SendSysMessageEx(attacker, "Seu projetil foi desviado do alvo.", SSM_FAIL);
			return 1;
		endif
	endif

	return 0;
endfunction

function CheckHab_MaoLivre(byRef attackInfo)
	if (GetObjProperty(defender, "maolivre") == 1)
		EraseObjPRoperty(defender, "maolivre");
		SetObjProperty(defender, "evasao", 1);
		return 1;
	endif

	return 0;
endfunction

function CheckHab_FerocidadeEnsanguentada(byRef attackInfo)
	if (GetCooldown(defender, "FerocidadeEnsanguentada"))
		// if (IsRaging(defender))
			if (GetObjProperty(defender, "#contraatacando") == error)
				SetObjProperty(defender, "#contraatacando", 1);
				PrintTextAbove(defender, "*contra ataque*");
				//defender.attack_once(attacker);
				var new_hit_chance := CalculateHitChance(defender, attacker);
				var new_random := RandomInt(100);
				PlayAttackFX(defender);
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

function CheckHab_Evasao(byRef attackInfo)
	// TODO: Passar rolldice e hitchance
	// TODO: Talvez esse código não precise ser poluído desse jeito, calculando e executando toda a estrutura do ataque....
		// TODO: Ver se dá pra executar o ataque através do Attack do combatHook...
	// Os 2 valores estão 0 apenas pra poder compilar, mas a função tem que receber
	// var rolldice := 0;
	// var hitchance := 0;
	
	if (Cint(GetObjProperty(defender, "evasao")))
		PrintTextAbove(defender, "*esquiva-se*");
		EraseObjProperty(defender, "evasao");
		if (TemHabilidade(defender, "Esquiva Oportunista"))
			if (GetObjProperty(defender, "#atacandooportunidade") == error)
				SetObjProperty(defender, "#atacandooportunidade", 1);
				PrintTextAbove(defender, "*ataque de oportunidade*");
				
				// TODO: Testar para ver se isso não vai explodir o combat:
					// Possibilidade de erros:
						// 1. Loop infinito
						// 2. Chamar hab que não deveria
					// Em caso de erro, pensar em uma forma de criar uma função pra realizar esse ataque sem chamar as habs

				defender.attack_once(attacker);
				EraseObjProperty(defender, "#atacandooportunidade");
			endif
		endif

		return 1;
	elseif ((GetObjProperty(defender, "#mobilidade") == 1) && ((attackInfo.dice-attackInfo.hitchance) < 16)) //Esquivou por conta do jogo de pés da "Mobilidade em Combate"
		PrintTextAbove(defender, "*esquiva com jogo de pés*");
		if (TemHabilidade(defender, "Esquiva Oportunista"))
			if (GetObjProperty(defender, "#atacandooportunidade") == error)
				SetObjProperty(defender, "#atacandooportunidade", 1);
				PrintTextAbove(defender, "*ataque de oportunidade*");
				// Mesma coisa de lá de cima
				defender.attack_once(attacker);
				EraseObjProperty(defender, "#atacandooportunidade");
				return 1;
			endif
		endif
	endif

	return 0;
endfunction

function CheckHab_Rebuke(byRef attackInfo)
	if (GetObjProperty(defender, "#campeaodivino"))
        // Apply divine retaliation
        Start_Script(":charactercreation:habilidades/paladin/golpedivino", array{defender, attacker});
		
		return 1;
    endif
	return 0;
endfunction

function CheckHab_AtaqueInfalivel(byRef attackInfo)
	if (GetCooldown(attacker, "ataqueinfalivel") > 0)
		PrintTextAbove(attacker, "*ataque infalivel*");
		var armor := GetArmorToHit(defender);
		var dmg := CalculateRawDamage(attacker, attacker.weapon)*0.33;
		var result := Run_Script_To_Completion(":combat:mainHitScript", array{attacker, defender, attacker.weapon, armor, dmg, dmg});
		SkillChecks(attacker, defender, attacker.weapon, dmg);

		return 1;
	endif

	return 0;
endfunction

function CheckHab_MestreDuelista(byRef attackInfo)
	if (GetCooldown(attacker, "mestreduelista") > 0)
		PrintTextAbove(attacker, "*mestre duelista*");
		var armor := GetArmorToHit(defender);
		var dmg := CalculateRawDamage(attacker, attacker.weapon);
		var result := Run_Script_To_Completion(":combat:mainHitScript", array{attacker, defender, attacker.weapon, armor, dmg, dmg});
		SkillChecks(attacker, defender, attacker.weapon, dmg);
		
		return 1;
	endif

	return 0;
endfunction

function CheckHab_Insistencia(byRef attackInfo)
	if (TemHabilidade(attacker, "Insistencia"))
		SetObjProperty(attacker, "insistencia", 1);
		return 1;
	endif

	return 0;
endfunction

function CheckHab_ArrowRecovery(byRef attackInfo)
	if (attackInfo.GetAttacker().HasAbility(COMBAT_ABILITY::RECUPERAR_FLECHAS))
		attackInfo.SetHitChance(attackInfo.GetHitChance() + 35);
		return true;
	endif

	return false;
endfunction

function CheckHab_DivineAuthority(byRef attackInfo)
    var HAB_NAME := "Divine Authority - Mountain's Strength";
    
    if (IsPhysicalDamage(attackInfo.type))
        if (GetCooldown(defender, "mountains_strength") > 0)
            attackInfo.damage := 1; // Reduce physical damage to 1
            DebugMsg(defender, DEBUG_HAB, $"Possui Habilidade[{HAB_NAME}]: {HAB_STATUS::SUCCESS}!");
            DebugMsg(defender, DEBUG_DAMAGE, $"Defesa de Habilidade[{HAB_NAME}]: Dano reduzido para 1");
            return 1;
        endif
    endif

    return 0;
endfunction

// function CheckHab_(byRef attackInfo)

// 	return 0;
// endfunction

// function CheckHab_(byRef attackInfo)

// 	return 0;
// endfunction

// function CheckHab_(byRef attackInfo)

// 	return 0;
// endfunction

// function CheckHab_(byRef attackInfo)

// 	return 0;
// endfunction

// function CheckHab_(byRef attackInfo)

// 	return 0;
// endfunction

