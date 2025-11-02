/*
 * $Id$
 *
 * Purpose
 * Handles parry calculations and logic
 *
 */
include ":combat:Model/AttackInfo";
include ":combat:Model/CombatAbility";

include ":tn:cooldown";
include "include/facings";

function CheckParry(byRef attackInfo)
	var attackerPos := attackInfo.GetAttacker().GetPosition();
	if (attackInfo.GetAttacker().GetObjRef().GetProp("backstab") ||
		IsBehind(attackInfo.GetDefender().GetObjRef(), attackerPos.x, attackerPos.y) ||
		!attackInfo.GetDefender().IsWarMode())
		
		return false;
	endif

	return TryToParry(attackInfo);
endfunction

function TryToParry(byRef attackInfo)
	if (GetCooldown(attackInfo.GetAttacker().GetObjRef(), "parrydelay") > 0)
		return false;
	endif

	var equip := attackInfo.GetDefender().GetEquipBySlot(SHIELD) ?: attackInfo.GetDefender().GetEquipBySlot(WEAPON); // Se estiver sem escudo, tenta bloquear com a arma
	
	if (!equip || equip.GetAttribute() == RANGED)
		return false;
	endif

	attackInfo.SetParryEquip(equip);
	attackInfo.SetParryChance(CalculateParryChance(attackInfo));
	attackInfo.RollParryChance();
	attackInfo.GetDefender().GetObjRef().EraseProp("#tempparrybonus"); // Remove o bônus temporário de parry, se houver

	if (attackInfo.Parried())
		DoParry(attackInfo);
		return true;
	endif

	return false;
endfunction

function DoParry(byRef attackInfo)
	Message($"Você parou o ataque de {attackInfo.GetAttacker().GetName()}!", FONTCOLOR_SUCCESS).SendSys(attackInfo.GetDefender().GetObjRef());
	DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_HIT, "***ATAQUE BLOQUEADO***");
	DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_HIT, "***ATAQUE BLOQUEADO***");

	// sobe a skill
	if (attacker.npctemplate && defender.connected)
		if (defendeu)
			SkillCheck(defender, PARRY, -1);
		else
			if (RandomInt(5) == 1)
				SkillCheck(defender, PARRY, -1);
			endif
		endif
	endif

	// TODO: Isso daqui vai pra algum check em algum lugar
	if (RandomInt(100) < parry_chance)
		// Pike desarma defensor que bloquear sem escudo
		if ((attacker.weapon.graphic == 0x26BE) && (usingWeapon))
			var defweapon := defender.weapon;
			if (defweapon.objtype != 0x1F020)
				var x := defender.x + RandomInt(2) + 1;
				var y := defender.y + RandomInt(2) + 1;
				MoveObjectToLocation(
					defweapon,
					x,
					y,
					GetWorldHeight(x, y),
					attacker.realm,
					MOVEOBJECT_FORCELOCATION
				);
			endif
		endif

		CheckHab_KenNoTatsujin(attacker, defender);
	endif

	// Aplica delay básico do escudo, reduz delay com base na skill e soma fator de balanceamento
	var parryDelay := CInt(
		-(CInt(attackInfo.GetParryEquip().GetShieldAttributes().GetParryDelay()))) + // Parry Delay Básico do escudo
		-(CInt(skill / 20)) +								// Parry Delay Bônus de acordo com a skill
		8 													// Fator de balanceamento
	);

	// Faz com que os bloqueios garantidos por cauda de habilidade resetem o delay de bloqueio
	if (tempParryBonus == 100)
		parryDelay := 0;
	endif

	// Aumenta a frequência em que é possível bloquear o mesmo atacante quando o defensor está sob efeito de habilidade Inspirar Defesas
	if (TemHabilidade(defender.party.leader, "Inspirar Defesas"))
		parryDelay := parryDelay - 1;
	endif

	// TODO: Adicionar o SetCooldown de parry
	
	WearDownShield(
		attackInfo.GetDefender().GetObjRef(),
		attackInfo.GetParryEquip().GetObjRef(),
		attackInfo.GetDamage().GetTotalDamage(),
		attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetObjRef()
	);

	return true;
endfunction

function CalculateParryChance(byRef attackInfo)
	var shield := attackInfo.GetParryEquip(),
		parryBonus := 0,
		skill := 0,
		blocker;

	// Verifica se o defensor está sob efeito da a hab esudo do martir
	// TODO: Refatorar para buscar essa habilidade nem _defenderAbilities no attackInfo
	var wall := attackInfo.GetDefender().GetObjRef().party.GetProp("BarreiraDeEscudos"),
		wallChance := 0;

	if (wall)
		blocker := SystemFindObjectBySerial(wall.blocker);
		if (GetCooldown(blocker, "BarreiraDeEscudos") > 0)
			if (Distance(attackInfo.GetDefender().GetObjRef(), blocker) < 2)
				wallChance := CInt(wall.chance);
			endif
		else
			attackInfo.GetDefender().GetObjRef().party.EraseProp("BarreiraDeEscudos");
			blocker := 0;
		endif
	endif

	// Calcula o Bonus basico de parry do escudo, Bonus de parry específico da CProp do escudo, Bonus de parry especifico da CProp do personagem
	parryBonus := CInt(
		CInt(attackInfo.GetParryEquip().GetShieldAttributes().GetParryBonus()) + // Parry Bônus Básico do escudo
		CInt(attackInfo.GetDefender().GetProp("parrybonus"))					 // Parry Bônus Extra do defensor
	);

	// Calcula o Bonus temporário de parry, se houver
	var tempParryBonus := CInt(attackInfo.GetDefender().GetObjRef().GetProp("#tempparrybonus"));
	if (tempParryBonus)
		parryBonus := parryBonus + tempParryBonus;
	endif

	if (attackInfo.GetDefender().HasAbility(COMBAT_ABILITY::REFLEXO_ESTRATEGICO))
		skill := CInt(attackInfo.GetDefender().GetSkill(TACTICS));
	else
		skill := CInt(attackInfo.GetDefender().GetSkill(PARRY));
	endif


	// Calcula o bonus de bloqueio com base no parry usando escudo
	var parry_chance := CInt(skill / 3);

	// Aplica penalidade de chance de bloqueio se estiver usando apenas arma
	if (attackInfo.GetParryEquip().GetObjRef().IsA(POLCLASS_WEAPON) && !TemHabilidade(defender, "Tabibito no Michi"))
		parry_chance := CInt(parry_chance * 0.7);
	endif

	if (attackInfo.GetParryEquip().GetObjRef().IsA(POLCLASS_WEAPON) && TemHabilidade(defender, "Tabibito no Michi"))
		parry_chance := CInt(parry_chance * 1.3);
	endif

	// Calcula a chance TOTAL de bloqueio
	parry_chance := parry_chance + parryBonus;

	// A habilidade do companheiro de party do defensor tentar bloquear em seu lugar só ativa se a chance de bloqueio dele for melhor que do defensor
	if (wallChance > parry_chance)
		parry_chance := wallChance;
	endif

	// Aplica o efeito da habilidade que deixa o defensor bloquear sempre
	if (CheckHab_BarreiraIndestrutivel(attacker, defender))
		parry_chance := 100;
		parryDelay := 2;
	endif

	if (!defendeu)
		return false;
	endif

	// Exibe o efeito visual do bloqueio
	PlayDefenseFX(attackInfo);
	PerformAction(defender, ANIM_TWIST_DODGE);

	if (blocker)
		PerformAction(defender, 13);
		PrintTextAbove(blocker, "*defende o golpe pelo aliado*");
		SetCooldown(attacker, "parrydelay", parryDelay * 2);
	else
		PerformAction(defender, 13);
		PrintTextAbove(defender, "*defende o golpe*");
		SetCooldown(attacker, "parrydelay", parryDelay);
	endif

	return parry_chance;
endfunction