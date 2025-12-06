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


include "DamageController";
include "HitController";
include "SkillController";
include "FXController";
include ":combat:Model/CombatAbility";

include ":charactercreation:resists";
include ":blood:blood";

enum class HAB_STATUS
	SUCCESS := "ATIVADA!",
	FAIL := "FALHOU!",
	COOLDOWN := "EM RECARGA!",
endenum

function ExecuteCombatAbilitiesBeforeDamage(byRef attackInfo)
	var exec := 0;

	if (TryToExecuteHab_Martir(attackInfo)) exec++; endif
	if (TryToExecuteHab_MantoRothen(attackInfo)) exec++; endif
	if (TryToExecuteHab_KaniNoSutairu(attackInfo)) exec++; endif
	if (TryToExecuteHab_Exoesqueleto(attackInfo)) exec++; endif
	if (TryToExecuteHab_EspecialistaEscudos(attackInfo)) exec++; endif
	if (TryToExecuteHab_Furia(attackInfo)) exec++; endif
	if (TryToExecuteHab_ResilienciaBarbara(attackInfo)) exec++; endif
	if (TryToExecuteHab_VinculoElemental(attackInfo)) exec++; endif
	if (TryToExecuteHab_Backstab(attackInfo)) exec++; endif
	if (TryToExecuteHab_Charge(attackInfo)) exec++; endif
	if (TryToExecuteHab_DivineAuthority(attackInfo)) exec++; endif

	if (exec > 0)
		return true;
	endif

	DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_HAB, $"Nenhuma habilidade ativada antes de receber o dano.");

	return false;
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
	var exec := 0;

	// if (TryToExecuteHab_ArmaduraArcana(attackInfo)) exec++; endif
	// if (TryToExecuteHab_Ainalaith(attackInfo)) exec++; endif
	// if (TryToExecuteHab_DeathDance(attackInfo)) exec++; endif
	// if (TryToExecuteHab_DisparoMistico(attackInfo)) exec++; endif
	// if (TryToExecuteHab_DesviarProjetil(attackInfo)) exec++; endif
	// if (TryToExecuteHab_DesviarDisparos(attackInfo)) exec++; endif
	// if (TryToExecuteHab_FerocidadeEnsanguentada(attackInfo)) exec++; endif
	// if (TryToExecuteHab_Evasao(attackInfo)) exec++; endif
	// if (TryToExecuteHab_ContraAtaque(attackInfo)) exec++; endif
	// if (TryToExecuteHab_Rebuke(attackInfo)) exec++; endif

	if (exec > 0)
		return true;
	endif

	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_HAB, $"Nenhuma habilidade ativada no hit.");

	return false;
endfunction

function ExecuteCombatAbilitiesAfterDamage(byRef attackInfo)
	var exec := 0;

	// if (TryToExecuteHab_RecuperacaoSalvadora(attackInfo)) exec++; endif
	// if (TryToExecuteHab_Vinganca(attackInfo)) exec++; endif
	// if (TryToExecuteHab_ChainOfPain(attackInfo)) exec++; endif

	if (exec > 0)
		return true;
	endif

	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_HAB, $"Nenhuma habilidade ativada após causar o dano.");
	DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_HAB, $"Nenhuma habilidade ativada após receber o dano.");
	return false;
endfunction

function ExecuteCombatAbilitiesOnMiss(byRef attackInfo)
	var exec := 0;

	// if (TryToExecuteHab_AtaqueInfalivel(attackInfo)) exec++; endif
	// if (TryToExecuteHab_MestreDuelista(attackInfo)) exec++; endif

	if (exec > 0)
		return true;
	endif

	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_HAB, $"Nenhuma habilidade ativada no miss.");
	return false;
endfunction

// ================ Habilidades Individuais ================= //
function TryToExecuteHab_Martir(byRef attackInfo)
	if (attackInfo.GetDefender().GetProperty("#martir"))
		var martir := SystemFindObjectBySerial(attackInfo.GetDefender().GetProperty("martirserial"));
		if (martir && !martir.dead)
			attackInfo.GetDefender().EraseProperty("#martir");
			attackInfo.SetDefender(martir);
			DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::MARTIR}]: {HAB_STATUS::SUCCESS}!");
			return true;
		else
			attackInfo.GetDefender().EraseProperty("#martir");
			DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::MARTIR}]: {HAB_STATUS::FAIL}!");
		endif
	endif

	return false;
endfunction

function TryToExecuteHab_MantoRothen(byRef attackInfo)
	if (attackInfo.GetDefender().GetObjRef().GetProp("#mantorothen"))
		var healed := CInt(attackInfo.GetDamage().GetTotalBaseDamage() * 0.3); // TODO: Ver se o valor curado será esse mesmo.
		var party := attackInfo.GetDefender().GetObjRef().party;
		var amthealed := CInt(healed/party.members.size());

		if (amthealed < 1)
			amthealed := 1;
		endif

		foreach member in (party.members)
			HealDamage(member, amthealed);
		endforeach

		DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::MANTO_ROTHEN}]: {HAB_STATUS::SUCCESS}!");
		return true;
	endif

	return false;
endfunction

function TryToExecuteHab_RecuperacaoSalvadora(byRef attackInfo)
	if (isBloodied(attackInfo.GetDefender().GetObjRef()))
		attackInfo.GetDefender().Update();
		MakeBleed(attackInfo.GetDefender().GetObjRef());
		if (attackInfo.GetDefender().GetObjRef().GetProp("#recuperacaosalvadora"))
			attackInfo.GetDefender().GetObjRef().EraseProp("#recuperacaosalvadora");
			Message("Uma recuperacao salvadora salvou voce.", FONTCOLOR_SUCCESS).SendSys(attackInfo.GetDefender().GetObjRef());
			HealDamage(attackInfo.GetDefender().GetObjRef(), CInt(GetPH(attackInfo.GetDefender().GetObjRef())/2));

			DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::RECUPERACAO_SALVADORA}]: {HAB_STATUS::SUCCESS}!");
			return true;
		endif
	endif

	return false;
endfunction

function TryToExecuteHab_Vinganca(byRef attackInfo)
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

function TryToExecuteHab_ChainOfPain(byRef attackInfo)
	if (attackInfo.GetDefender().GetObjRef().GetProp("#chainofpain") != error)
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

			return true;
		endif
	endif

	return false;
endfunction

function TryToExecuteHab_TaticasArcanas(byRef attackInfo)
	if (attackInfo.GetAttacker().HasAbility(COMBAT_ABILITY::TATICAS_ARCANAS))
		var manaatual := attackInfo.GetAttacker().GetCurrentMana();
		var manatotal := attackInfo.GetAttacker().GetMaximumMana();
		if (manaatual >= manatotal / 2)
			var damagemod := CInt(manaatual / 20);

			attackInfo.GetDamage().IncreaseSlashBaseDamage(damagemod);
			attackInfo.GetDamage().IncreasePiercingBaseDamage(damagemod);
			attackInfo.GetDamage().IncreaseBluntBaseDamage(damagemod);
			
			DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::TATICAS_ARCANAS}]: {HAB_STATUS::SUCCESS}!");
			DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"Dano de Habilidade[{COMBAT_ABILITY::TATICAS_ARCANAS}]: " + CInt(damagemod));
			return true;
		else
			DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::TATICAS_ARCANAS}]: {HAB_STATUS::FAIL}!");
		endif
	endif

	return false;
endfunction

function TryToExecuteHab_BrigadorDeTaverna(byRef attackInfo)
	if (attackInfo.GetAttacker().HasAbility(COMBAT_ABILITY::BRIGADOR_DE_TAVERNA))
		if (attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetCategory() == WEAPON_CATEGORY::UNARMED)
			var damageMod := CInt(attackInfo.GetAttacker().GetSkill(TACTICS) / 20);

			attackInfo.GetDamage().IncreaseBluntBaseDamage(damageMod);
			
			DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::BRIGADOR_DE_TAVERNA}]: {HAB_STATUS::SUCCESS}!");
			DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"Dano de Habilidade[{COMBAT_ABILITY::BRIGADOR_DE_TAVERNA}]: " + CInt(damageMod));
			return true;
		else
			DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::BRIGADOR_DE_TAVERNA}]: {HAB_STATUS::FAIL}!");
		endif
	endif

	return false;
endfunction

function TryToExecuteHab_KaniNoSutairu(byRef attackInfo)
	if (attackInfo.GetDefender().HasAbility(COMBAT_ABILITY::KANI_NO_SUTAIRU))
		var resist := CInt(attackInfo.GetDefender().GetSkill(TACTICS) / 20);

		attackInfo.GetDamage().DecreaseSlashBaseDamage(resist);
		attackInfo.GetDamage().DecreasePiercingBaseDamage(resist);
		attackInfo.GetDamage().DecreaseBluntBaseDamage(resist);
		
		DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::KANI_NO_SUTAIRU}]: {HAB_STATUS::SUCCESS}!");
		DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_DAMAGE, $"Defesa de Habilidade[{COMBAT_ABILITY::KANI_NO_SUTAIRU}]: " + CInt(resist));
		return true;
	endif

	return false;
endfunction

function TryToExecuteHab_Exoesqueleto(byRef attackInfo)
	if (attackInfo.GetDefender().HasAbility(COMBAT_ABILITY::EXOESQUELETO))
		var resist := 2;

		attackInfo.GetDamage().DecreaseSlashBaseDamage(resist);
		attackInfo.GetDamage().DecreasePiercingBaseDamage(resist);
		attackInfo.GetDamage().DecreaseBluntBaseDamage(resist);

		DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::EXOESQUELETO}]: {HAB_STATUS::SUCCESS}!");
		DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_DAMAGE, $"Defesa de Habilidade[{COMBAT_ABILITY::EXOESQUELETO}]: " + CInt(resist));
		return true;
	endif

	return false;
endfunction

function TryToExecuteHab_EspecialistaEscudos(byRef attackInfo)
	if (attackInfo.GetDefender().HasAbility(COMBAT_ABILITY::ESPECIALISTA_ESCUDOS)) // Tem que estar com escudo equipado
		if (attackInfo.GetDefender().GetEquipBySlot(SHIELD).GetObjRef())
			var resist := CInt(attackInfo.GetDefender().GetSkill(PARRY) / 20);

			attackInfo.GetDamage().DecreaseSlashBaseDamage(resist);
			attackInfo.GetDamage().DecreasePiercingBaseDamage(resist);
			attackInfo.GetDamage().DecreaseBluntBaseDamage(resist);

			DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::ESPECIALISTA_ESCUDOS}]: {HAB_STATUS::SUCCESS}!");
			DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_DAMAGE, $"Defesa de Habilidade[{COMBAT_ABILITY::ESPECIALISTA_ESCUDOS}]: " + CInt(resist));
			return true;
		else
			DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::ESPECIALISTA_ESCUDOS}]: {HAB_STATUS::FAIL}! Nenhum escudo equipado.");
		endif
	endif

	return false;
endfunction

function TryToExecuteHab_Backstab(byRef attackInfo)
	if (attackInfo.GetAttacker().GetProperty("backstab") &&
		attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetProperty("backstab")
		)
		attackInfo.GetAttacker().EraseProperty("backstab");
		attackInfo.GetAttacker().GetEquipBySlot(WEAPON).EraseProperty("backstab");
		attackInfo.GetAttacker().EraseProperty("#nextcritical");

		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::BACKSTAB}]: {HAB_STATUS::SUCCESS}!");
		
		if (TryToExecuteHab_SempreAlerta(attackInfo))
			DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_HAB, $"Habilidade[{COMBAT_ABILITY::BACKSTAB}]: {HAB_STATUS::FAIL}! Bloqueado pelo defensor.");
		else
			TryToExecuteHab_AtaqueFurtivo(attackInfo);
		endif

		return true;
	else
		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::BACKSTAB}]: {HAB_STATUS::FAIL}!");
	endif

	return false;
endfunction

function TryToExecuteHab_Furia(byRef attackInfo)
	if (GetCooldown(attackInfo.GetAttacker().GetObjRef(), COMBAT_ABILITY::FURIA))
		var damagemod := CInt(attackInfo.GetAttacker().GetStr() / 20);

		attackInfo.GetDamage().IncreaseSlashBaseDamage(damagemod);
		attackInfo.GetDamage().IncreasePiercingBaseDamage(damagemod);
		attackInfo.GetDamage().IncreaseBluntBaseDamage(damagemod);

		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::FURIA}]: {HAB_STATUS::SUCCESS}!");
		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"Dano de Habilidade[{COMBAT_ABILITY::FURIA}]: " + CInt(damagemod));
		return true;
	else
		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::FURIA}]: {HAB_STATUS::COOLDOWN}!");
	endif

	return false;
endfunction

function TryToExecuteHab_ResilienciaBarbara(byRef attackInfo)
	if (attackInfo.GetDefender().HasAbility(COMBAT_ABILITY::RESILIENCIA_BARBARA))
		if (GetCooldown(attackInfo.GetAttacker().GetObjRef(), COMBAT_ABILITY::FURIA))
			var resist := CInt(attackInfo.GetDefender().GetStr() / 20);

			attackInfo.GetDamage().DecreaseSlashBaseDamage(resist);
			attackInfo.GetDamage().DecreasePiercingBaseDamage(resist);
			attackInfo.GetDamage().DecreaseBluntBaseDamage(resist);

			DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::RESILIENCIA_BARBARA}]: {HAB_STATUS::SUCCESS}!");
			DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_DAMAGE, $"Defesa de Habilidade[{COMBAT_ABILITY::RESILIENCIA_BARBARA}]: " + CInt(resist));
			return true;
		else
			DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::RESILIENCIA_BARBARA}]: {HAB_STATUS::FAIL}! {COMBAT_ABILITY::FURIA} em recarga.");
		endif
	endif

	return false;
endfunction

function TryToExecuteHab_SempreAlerta(byRef attackInfo)
	if (attackInfo.GetDefender().HasAbility(COMBAT_ABILITY::SEMPRE_ALERTA))
		TS_StartTimer(attackInfo.GetDefender().GetObjRef(), "paralysis", 5);
		Message("Sempre Alerta").SendEmote(attackInfo.GetDefender().GetObjRef());
		DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::SEMPRE_ALERTA}]: {HAB_STATUS::SUCCESS}!");
		return true;
	endif

	return false;
endfunction

function TryToExecuteHab_Charge(byRef attackInfo)
	var damageMod := CInt(attackInfo.GetAttacker().GetProperty("charge_damage"));

	if (attackInfo.GetAttacker().GetProperty("charge_damage"))
		attackInfo.GetDamage().IncreaseSlashBaseDamage(damageMod);
		attackInfo.GetDamage().IncreasePiercingBaseDamage(damageMod);
		attackInfo.GetDamage().IncreaseBluntBaseDamage(damageMod);

		// TODO: Refatorar essa checagem sem codesmell
		if (":combat:weaponHitScripts/AntiMountedHit" in attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetTemplate().HitScripts)
			attackInfo.GetAttacker().GetObjRef().SetProp("#ForcedTarget", attackInfo.GetAttacker().GetSerial());

			var resisted := CheckResist(attackInfo.GetAttacker().GetObjRef(), FORTITUDE, RESIST_MEDIUM);
			if (!resisted)
				TS_StartTimer(attackInfo.GetAttacker().GetObjRef(), "desmontar", 10);
				TS_StartTimer(attackInfo.GetAttacker().GetObjRef(), "tombar", 5);
				TS_StartTimer(attackInfo.GetAttacker().GetObjRef(), "paralysis", 5);
			endif
		endif

		attackInfo.GetAttacker().EraseProperty("charge_damage");
		return true;
	endif

	return false;
endfunction

function TryToExecuteHab_VinculoElemental(byRef attackInfo)
	if (attackInfo.GetDefender().HasAbility(COMBAT_ABILITY::VINCULO_ELEMENTAL))
		var resist := 0;
		var arcane_armor := CInt(attackInfo.GetDefender().GetSkill(ARCANA) / 20);

		if (GetCooldown(attackInfo.GetDefender().GetObjRef(), "armor_drain") > 0)
			arcane_armor += CInt(attackInfo.GetDefender().GetSkill(ARCANA) / 30);
			PlayObjectCenteredEffectEx(attackInfo.GetDefender().GetObjRef(), 0x5454, 20, 20, 0);
		endif

		if (resist < arcane_armor)
			resist := arcane_armor;
			PlayObjectCenteredEffectEx(attackInfo.GetDefender().GetObjRef(), 0x5487, 1, 8, 0);
		endif

		attackInfo.GetDamage().DecreaseSlashBaseDamage(resist);
		attackInfo.GetDamage().DecreasePiercingBaseDamage(resist);
		attackInfo.GetDamage().DecreaseBluntBaseDamage(resist);

		DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::VINCULO_ELEMENTAL}]: {HAB_STATUS::SUCCESS}!");
		DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_DAMAGE, $"Defesa de Habilidade[{COMBAT_ABILITY::VINCULO_ELEMENTAL}]: " + CInt(resist));
		return true;
	endif

	return false;
endfunction

function TryToExecuteHab_KenNoTatsujin(byRef attackInfo)
	if (attackInfo.GetDefender().HasAbility(COMBAT_ABILITY::KEN_NO_TATSUJIN))
		if (attackInfo.GetDefender().GetEquipBySlot(SHIELD).GetObjRef())
			Message($"Você está usando escudo e falhou {COMBAT_ABILITY::KEN_NO_TATSUJIN}", FONTCOLOR_FAIL).SendSys(attackInfo.GetDefender().GetObjRef());
			DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::KEN_NO_TATSUJIN}]: {HAB_STATUS::FAIL}!");
		else
			Message($"{COMBAT_ABILITY::KEN_NO_TATSUJIN}").SendEmote(attackInfo.GetDefender().GetObjRef());
			PlaySoundEffect(attackInfo.GetDefender().GetObjRef(), 0x51f);

			attackInfo.GetDefender().Attack(attackInfo.GetAttacker());
			
			DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::KEN_NO_TATSUJIN}]: {HAB_STATUS::SUCCESS}!");
			return true;
		endif
	endif

	return false;
endfunction

function TryToExecuteHab_BarreiraIndestrutivel(byRef attackInfo)
	if (GetCooldown(attackInfo.GetDefender().GetObjRef(), "barreiraindestrutivel"))
		DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::BARREIRA_INDESTRUTIVEL}]: {HAB_STATUS::SUCCESS}!");
		return true;
	else
		DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::BARREIRA_INDESTRUTIVEL}]: {HAB_STATUS::COOLDOWN}!");
	endif

	return false;
endfunction

function TryToExecuteHab_Trespassar(byRef attackInfo)
	if (attackInfo.GetAttacker().GetObjRef().GetProp("#trespassando") == 1)
		return false;
	endif

	var list := array;
	foreach mobile in (ListHostiles(attackInfo.GetAttacker().GetObjRef(), 1, LH_FLAG_LOS))
		if ((mobile.serial != attackInfo.GetAttacker().GetSerial()) && (mobile.serial != attackInfo.GetDefender().GetSerial()))
			list.append(mobile);
		endif
	endforeach

	if (list.size() > 0)
		attackInfo.GetAttacker().GetObjRef().SetProp("#trespassando", 1);
		var targ := list[RandomInt(list.size()) + 1];
		if (GetVital(targ, HITS) > 0)
			targ := MobileWrapper(targ);
			Message("trespassar").SendEmote(attackInfo.GetAttacker().GetObjRef());
			
			if (attackInfo.GetAttacker().HasAbility(COMBAT_ABILITY::TRESPASSAR_CRITICO))
				attackInfo.GetAttacker().SetProperty("#nextcritical", 1);
			endif
			
			attackInfo.GetAttacker().Attack(targ);
			attackInfo.GetAttacker().EraseProperty("#nextcritical");
		endif

		attackInfo.GetAttacker().EraseProperty("#trespassando");
		
		return true;
	endif

	return false;
endfunction

function TryToExecuteHab_AtaqueFurtivo(byRef attackInfo)
	var sneak_dmg := CInt(attackInfo.GetAttacker().GetSkill(SNEAK));
	RevokePrivilege(attackInfo.GetAttacker().GetObjRef(), "hiddenattack");
	attackInfo.GetAttacker().GetObjRef().disable("hiddenattack");

	if (attackInfo.GetAttacker().IsNPC())
		sneak_dmg := CInt(attackInfo.GetAttacker().GetTier() * 15);

		if (sneak_dmg < 15)
			sneak_dmg := 15;
		endif
	endif

	var attackerPos := attackInfo.GetAttacker().GetPosition();
	if (IsBehind(attackInfo.GetDefender().GetObjRef(), attackerPos.GetX(), attackerPos.GetY()))
		if (CanBackStab(attackInfo.GetAttacker()))
			attackInfo.GetDamage().IncreaseSlashBaseDamage(sneak_dmg);
			attackInfo.GetDamage().IncreasePiercingBaseDamage(sneak_dmg);
			attackInfo.GetDamage().IncreaseBluntBaseDamage(sneak_dmg);
			
			MakeBleed(attackInfo.GetDefender().GetObjRef());
			DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::ATAQUE_FURTIVO}]: {HAB_STATUS::SUCCESS}! Bônus de dano: {sneak_dmg}");
			return true;
		else
			DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::ATAQUE_FURTIVO}]: {HAB_STATUS::FAIL}! Você não consegue realizar um ataque furtivo com esse tipo de arma.");
        endif
	else
		DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::ATAQUE_FURTIVO}]: {HAB_STATUS::FAIL}!");
		Message("Você precisa estar atrás do inimigo para um ataque furtivo.", FONTCOLOR_FAIL).SendSys(attackInfo.GetAttacker().GetObjRef());
    endif

	return false;
endfunction

function TryToExecuteHab_ArmaduraArcana(byRef attackInfo)
	var dist := Distance(attackInfo.GetAttacker().GetObjRef(), attackInfo.GetDefender().GetObjRef());
	
	if (GetCooldown(attackInfo.GetDefender().GetObjRef(), "freeze_enemy") && dist == 1)
		PlaySoundEffect(attackInfo.GetAttacker().GetObjRef(), 0x65b);
		PlayObjectCenteredEffectHuefx(attackInfo.GetAttacker().GetObjRef(), 0x54f7, 30, 60, COLOR_ELEMENTAL_ICE, 0);
		TS_Starttimer(attackInfo.GetAttacker().GetObjRef(), "congelamento", 5);

		DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::ARMADURA_ARCANA} (Congelamento)]: {HAB_STATUS::SUCCESS}!");
		return true;
	elseif (GetCooldown(attackInfo.GetDefender().GetObjRef(), "incinerate_enemy") && dist == 1)
		PlaySoundEffect(attackInfo.GetAttacker().GetObjRef(), 0x5D0);
		PlayObjectCenteredEffect(attackInfo.GetAttacker().GetObjRef(), 0x6d77, 7, 60);
		TS_Starttimer(attackInfo.GetAttacker().GetObjRef(), "queimar", 5, 1, attackInfo.GetDefender().GetObjRef());

		DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::ARMADURA_ARCANA} (Incinerar)]: {HAB_STATUS::SUCCESS}!");
		return true;
	elseif (GetCooldown(attackInfo.GetDefender().GetObjRef(), "move_speed")) // Raio é pra ser acertado por flecha e ir mais rapido até o arqueiro
		PlaySoundEffect(attackInfo.GetAttacker().GetObjRef(), 0x5CF);
		PlayObjectCenteredEffect(attackInfo.GetDefender().GetObjRef(), 0xcdaf, 7, 30);
		TS_StartTimer(attackInfo.GetDefender().GetObjRef(), "velocidade", 5);

		DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::ARMADURA_ARCANA} (SEI LÁ QUE TIPO É ESSE... PRINTA ISSO PROS DEV DESCOBRIREM!)]: {HAB_STATUS::SUCCESS}!");
		return true;
	endif

	return false;
endfunction

function TryToExecuteHab_Ainalaith(byRef attackInfo)
	if (attackInfo.GetAttacker().HasAbility(COMBAT_ABILITY::AINALAITH))
		var HAB_TYPE := "";
		var arrow_effects := attackInfo.GetAttacker().GetObjRef().GetProp("#shooting_effects");
		if (!arrow_effects)
			arrow_effects := struct{
				fire_arrow := 0,
				cold_arrow := 0,
				energy_arrow := 0
			};
		endif

		var arrow_type := "";
		if (attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetObjRef().GetProp("fire_arrow"))
			arrow_type := "fire_arrow";
			HAB_TYPE := "Flecha de Fogo";
		elseif (attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetObjRef().GetProp("cold_arrow"))
			arrow_type := "cold_arrow";
			HAB_TYPE := "Flecha de Gelo";
		elseif (attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetObjRef().GetProp("energy_arrow"))
			arrow_type := "energy_arrow";
			HAB_TYPE := "Flecha de Energia";
		endif

		if (arrow_type != "")
			var current_count := arrow_effects[arrow_type];
			current_count += 1;
			arrow_effects[arrow_type] := current_count;

			attackInfo.GetAttacker().GetObjRef().SetProp("#shooting_effects", arrow_effects);

			DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::AINALAITH} ({HAB_TYPE})]: {HAB_STATUS::SUCCESS}!");
			return true;
		endif
	endif

	return false;
endfunction

function TryToExecuteHab_ConhecimentoSobreInimigo(byRef attackInfo)
	// TODO: Verificar se isso ainda existe
	// if (TemHabilidade(attacker, COMBAT_ABILITY::CONHECIMENTO_SOBRE_O_INIMIGO))
	// 	SetObjProperty(defender, "#attackbonus", -20);
	// 	return true;
	// endif

	return false;
endfunction

function TryToExecuteHab_DeathDance(byRef attackInfo)
	if (getCooldown(attackInfo.GetAttacker().GetObjRef(), "deathdance") > 0)
		moveToBehind(attackInfo.GetAttacker().GetObjRef(), attackInfo.GetDefender().GetObjRef());
		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::DEATH_DANCE}]: {HAB_STATUS::SUCCESS}!");
		return true;
	else
		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::DEATH_DANCE}]: {HAB_STATUS::COOLDOWN}!");
	endif

	return false;
endfunction

function TryToExecuteHab_DisparoMistico(byRef attackInfo)
	// TODO: Verificar se isso ainda existe, e se for o caso, refatorar (que troço CAGADO)
	
	// if (getCooldown(attacker, "disparomistico"))
	// 	PrintTextPrivate(attacker, "*passo nebuloso*", attacker);
	// 	SetCooldown(attacker, "disparomisticoHit", CInt(AP_GetSkill(attacker, ARCANA) / 20));
	// 	return true;
	// endif
	
	return false;
endfunction

function TryToExecuteHab_ContraAtaque(byRef attackInfo)
	if (GetCooldown(attackInfo.GetDefender().GetObjRef(), "contraataque") > 0)
		Message("contra ataca").SendEmote(attackInfo.GetDefender().GetObjRef());
		
		attackInfo.GetDefender().Attack(attackInfo.GetAttacker());
		
		DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::CONTRA_ATAQUE}]: {HAB_STATUS::SUCCESS}!");
		return true;
	else
	endif

	return false;
endfunction

function TryToExecuteHab_DesviarProjetil(byRef attackInfo)
	if (attackInfo.GetDefender().GetProperty("#desviarprojetil"))
		attackInfo.GetDefender().EraseProperty("#desviarprojetil");

		// TODO: Adicionar lógica para impedir o dano e talvez adicionar um efeito visual

		Message("desvia projétil").SendEmote(attackInfo.GetDefender().GetObjRef());
		Message("O projétil foi desviado para longe de você.", FONTCOLOR_SUCCESS).SendSys(attackInfo.GetDefender().GetObjRef());
		Message("Seu projétil foi desviado do alvo.", FONTCOLOR_FAIL).SendSys(attackInfo.GetAttacker().GetObjRef());
		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::DESVIAR_PROJETIL}]: {HAB_STATUS::SUCCESS}!");
		return true;
	endif
	
	return false;
endfunction

function TryToExecuteHab_DesviarDisparos(byRef attackInfo)
	if (GetCooldown(attackInfo.GetDefender().GetObjRef(), "desviardisparos") > 0)
		// TODO: Adicionar lógica para impedir o dano e talvez adicionar um efeito visual

		Message("desvia disparo").SendEmote(attackInfo.GetDefender().GetObjRef());
		Message("O disparo foi desviado para longe de você.", FONTCOLOR_SUCCESS).SendSys(attackInfo.GetDefender().GetObjRef());
		Message("Seu disparo foi desviado do alvo.", FONTCOLOR_FAIL).SendSys(attackInfo.GetAttacker().GetObjRef());
		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::DESVIAR_DISPAROS}]: {HAB_STATUS::SUCCESS}!");
		return true;
	else
		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::DESVIAR_DISPAROS}]: {HAB_STATUS::COOLDOWN}!");
	endif

	return false;
endfunction

function TryToExecuteHab_MaoLivre(byRef attackInfo)
	if (attackInfo.GetDefender().GetProperty("maolivre") == 1)
		attackInfo.GetDefender().EraseProperty("maolivre");
		attackInfo.GetDefender().SetProperty("evasao", 1);
		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::MAO_LIVRE}]: {HAB_STATUS::SUCCESS}!");
		return true;
	endif

	return false;
endfunction

function TryToExecuteHab_FerocidadeEnsanguentada(byRef attackInfo)
	if (GetCooldown(attackInfo.GetDefender().GetObjRef(), "FerocidadeEnsanguentada"))
		if (attackInfo.GetDefender().GetProperty("#contraatacando") == error)
			attackInfo.GetDefender().SetProperty("#contraatacando", 1);
			attackInfo.GetDefender().Attack(attackInfo.GetAttacker());
			attackInfo.GetDefender().EraseProperty("#contraatacando");

			Message("contra ataque").SendEmote(attackInfo.GetDefender().GetObjRef());
			DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::FEROCIDADE_ENSANGUENTADA}]: {HAB_STATUS::SUCCESS}!");
			return true;
		endif
	else
		DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::FEROCIDADE_ENSANGUENTADA}]: {HAB_STATUS::COOLDOWN}!");
	endif

	return false;
endfunction

function TryToExecuteHab_Evasao(byRef attackInfo)
	// TODO: Refatorar esse código após combate funcionando, vai precisar de um carinho PESADO aqui	
	
	// if (Cint(GetObjProperty(defender, "evasao")))
	// 	PrintTextAbove(defender, "*esquiva-se*");
	// 	EraseObjProperty(defender, "evasao");
	// 	if (TemHabilidade(defender, "Esquiva Oportunista"))
	// 		if (GetObjProperty(defender, "#atacandooportunidade") == error)
	// 			SetObjProperty(defender, "#atacandooportunidade", 1);
	// 			PrintTextAbove(defender, "*ataque de oportunidade*");
				
	// 			defender.attack_once(attacker);
	// 			EraseObjProperty(defender, "#atacandooportunidade");
	// 		endif
	// 	endif

	// 	return 1;
	// elseif ((GetObjProperty(defender, "#mobilidade") == 1) && ((attackInfo.dice-attackInfo.hitchance) < 16)) //Esquivou por conta do jogo de pés da "Mobilidade em Combate"
	// 	PrintTextAbove(defender, "*esquiva com jogo de pés*");
	// 	if (TemHabilidade(defender, "Esquiva Oportunista"))
	// 		if (GetObjProperty(defender, "#atacandooportunidade") == error)
	// 			SetObjProperty(defender, "#atacandooportunidade", 1);
	// 			PrintTextAbove(defender, "*ataque de oportunidade*");
	// 			// Mesma coisa de lá de cima
	// 			defender.attack_once(attacker);
	// 			EraseObjProperty(defender, "#atacandooportunidade");
	// 			return 1;
	// 		endif
	// 	endif
	// endif

	return 0;
endfunction

function TryToExecuteHab_Rebuke(byRef attackInfo)
	if (attackInfo.GetDefender().GetObjRef().GetProp("#campeaodivino"))
		Run_Script_To_Completion(":charactercreation:habilidades/paladin/golpedivino", array{attackInfo.GetDefender().GetObjRef(), attackInfo.GetAttacker().GetObjRef()});
		
		DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::REBUKE}]: {HAB_STATUS::SUCCESS}!");
		return true;
	endif

	return false;
endfunction

function TryToExecuteHab_AtaqueInfalivel(byRef attackInfo)
	if (GetCooldown(attackInfo.GetAttacker().GetObjRef(), "ataqueinfalivel") > 0)
		attackInfo.SetHitChance(1000);

		Message("ataque infalivel").SendEmote(attackInfo.GetAttacker().GetObjRef());
		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::ATAQUE_INFALIVEL}]: {HAB_STATUS::SUCCESS}!");
		return true;
	else
		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::ATAQUE_INFALIVEL}]: {HAB_STATUS::COOLDOWN}!");
	endif

	return false;
endfunction

function TryToExecuteHab_MestreDuelista(byRef attackInfo)
	if (GetCooldown(attackInfo.GetAttacker().GetObjRef(), "mestreduelista") > 0)
		// TODO: Ver o que diabos é pra isso fazer...		
		Message("mestre duelista").SendEmote(attackInfo.GetAttacker().GetObjRef());
		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::MESTRE_DUELISTA}]: {HAB_STATUS::SUCCESS}!");
		return true;
	else
		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::MESTRE_DUELISTA}]: {HAB_STATUS::COOLDOWN}!");
	endif

	return false;
endfunction

function TryToExecuteHab_ArrowRecovery(byRef attackInfo)
	if (attackInfo.GetAttacker().HasAbility(COMBAT_ABILITY::RECUPERAR_FLECHAS))
		attackInfo.SetHitChance(attackInfo.GetHitChance() + 35);
		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_HAB, $"Possui Habilidade[{COMBAT_ABILITY::RECUPERAR_FLECHAS}]: {HAB_STATUS::SUCCESS}!");
		return true;
	endif

	return false;
endfunction

function TryToExecuteHab_DivineAuthority(byRef attackInfo)
	// TODO: VEI... QUE PORRA DE HAB É ESSA?!?! HUAHUUHahuHUaUHahuA
	
	// var HAB_NAME := "Divine Authority - Mountain's Strength";
	
	// if (IsPhysicalDamage(attackInfo.type))
	// 	if (GetCooldown(defender, "mountains_strength") > 0)
	// 		attackInfo.damage := 1; // Reduce physical damage to 1
	// 		DebugMsg(defender, DEBUG_HAB, $"Possui Habilidade[{HAB_NAME}]: {HAB_STATUS::SUCCESS}!");
	// 		DebugMsg(defender, DEBUG_DAMAGE, $"Defesa de Habilidade[{HAB_NAME}]: Dano reduzido para 1");
	// 		return 1;
	// 	endif
	// endif

	// return 0;
endfunction
