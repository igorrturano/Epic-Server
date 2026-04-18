/*
 * $Id$
 *
 * Purpose
 * Handles parry calculations and logic
 *
 */
include ":combat:Model/AttackInfo";
include ":abilities:models/Ability";
include ":combat:Controller/FXController";
include ":combat:Controller/CombatAbilityController";
include ":combat:Controller/WearDownController";
include ":message:model/Message";

include ":tn:cooldown";
include "include/facings";

function CheckParry(byRef attackInfo)
	var attacker := attackInfo.GetAttacker();
	var defender := attackInfo.GetDefender();
	var attackerPos := attacker.GetPosition();
	
	if (attacker.GetObjRef().GetProp("backstab") || // Se for habilidade de ataque pelas costas
		IsBehind(defender.GetObjRef(), attackerPos.GetX(), attackerPos.GetY()) || // Se o atacante estiver atrás do defensor
		!defender.IsWarMode() || // Se o defensor não estiver em war mode
		GetCooldown(attacker.GetObjRef(), "parrydelay") > 0) // Se o atacante estiver em cooldown de parry

		return false; // Então não bloqueia
	endif

	return TryToParry(attackInfo);
endfunction

function TryToParry(byRef attackInfo)
	// Se estiver sem escudo, tenta bloquear com a arma
	var equip := attackInfo.GetDefender().GetEquipBySlot(SHIELD) ?: attackInfo.GetDefender().GetEquipBySlot(WEAPON);

	if (!equip || equip.GetAttribute() == RANGED)
		return false;
	endif

	attackInfo.SetParryEquip(equip);
	attackInfo.SetParryChance(CalculateParryChance(attackInfo));
	CheckTrueDefender(attackInfo); // Alguém pode ter habilidade pra bloquear no lugar do defensor
	attackInfo.RollParryChance();

	if (attackInfo.Parried())
		DoParry(attackInfo);
		return true;
	elseif (attackInfo.GetAttacker().IsNPC() && attackInfo.GetDefender().IsConnected())
		if (RandomInt(5) == 1) // Tem chance de upar mesmo se não bloquear
			SkillCheck(attackInfo.GetDefender().GetObjRef(), PARRY, -1);
		endif
	endif

	return false;
endfunction

function DoParry(byRef attackInfo)
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_HIT, Message("***ATAQUE BLOQUEADO***", Color(SYSTEM_COLOR::FAIL)));
	DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_HIT, Message("***ATAQUE BLOQUEADO***", Color(SYSTEM_COLOR::FAIL)));
	PlayParryFX(attackInfo);
	PlayParrySFX(attackInfo);

	Message($"Você parou o ataque de {attackInfo.GetAttacker().GetName()}!", Color(SYSTEM_COLOR::SUCCESS)).SendSys(attackInfo.GetDefender().GetObjRef());
	if (attackInfo.GetDefender().GetSerial() != attackInfo.GetDefender().GetObjRef().party.GetProp("BarreiraDeEscudos"))
		Message("defende o golpe").SendEmote(attackInfo.GetDefender().GetObjRef());
		ApplyParryDelay(attackInfo);
	else
		Message("defende o golpe pelo aliado").SendEmote(attackInfo.GetDefender().GetObjRef());
		ApplyParryDelay(attackInfo, 2); // Aplica delay maior se estiver defendendo por um aliado
	endif

	attackInfo.GetDefender().GetObjRef().EraseProp("#tempparrybonus"); // Remove o bônus temporário de parry, se houver
	WearDownEquip(attackInfo.GetParryEquip(), attackInfo.GetWeapon());

	CheckDisarmShield(attackInfo);

	if (attackInfo.GetAttacker().IsNPC() && attackInfo.GetDefender().IsConnected())
		SkillCheck(attackInfo.GetDefender().GetObjRef(), PARRY, -1);
	endif

	attackInfo.SetDamage(Damage().SetBluntRawDamage(1)); // Seta o dano pra 1 de blunt por causa do impacto do bloqueio

	return true;
endfunction

function CheckTrueDefender(byRef attackInfo);
	// Verifica se o defensor está sob efeito da a hab esudo do martir
	// TODO: Refatorar para buscar essa habilidade nem _defenderAbilities no attackInfo
	var wall, blocker;
	wall := attackInfo.GetDefender().GetObjRef().party.GetProp("BarreiraDeEscudos");

	if (!wall) // Se não tem, o defensor é o original
		return false;
	endif

	blocker := SystemFindObjectBySerial(wall.blocker);

	if (!(GetCooldown(blocker, "BarreiraDeEscudos") > 0) &&
		Distance(attackInfo.GetDefender().GetObjRef(), blocker) > 2
		)
		attackInfo.GetDefender().GetObjRef().party.EraseProp("BarreiraDeEscudos");

		return false;
	endif

	if (attackInfo.GetParryChance() >= CInt(wall.chance)) // Separado do anterior para não remover a hab sem necessidade
		return false;
	endif

	attackInfo.SetParryChance(CInt(wall.chance));
	attackInfo.SetDefender(blocker);

	return true;
endfunction

function CalculateParryChance(byRef attackInfo)
	var shield, parryBonus, skill;
	shield := attackInfo.GetParryEquip();
	parryBonus := 0;
	skill := 0;

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

	if (attackInfo.GetDefender().HasAbility(ABILITY_ID::POSTURA_DE_DUELO))
		var offhand := GetEquipmentByLayer(attackInfo.GetDefender().GetObjRef(), LAYER_HAND2);
		if (offhand && offhand.objtype == 0x1B73)
			skill := CInt(attackInfo.GetDefender().GetSkill(TACTICS));
		else
			skill := CInt(attackInfo.GetDefender().GetSkill(PARRY));
		endif
	else
		skill := CInt(attackInfo.GetDefender().GetSkill(PARRY));
	endif

	// Calcula o bonus de bloqueio com base no parry usando escudo
	var parry_chance := CInt(skill / 3);

	// Aplica penalidade de chance de bloqueio se estiver usando apenas arma
	if (attackInfo.GetParryEquip().GetObjRef().IsA(POLCLASS_WEAPON))
		if (attackInfo.GetDefender().HasAbility(ABILITY_ID::WANDERER_PATH))
			parry_chance := CInt(parry_chance * 1.3);
		else
			parry_chance := CInt(parry_chance * 0.7);
		endif
	endif

	// Calcula a chance TOTAL de bloqueio
	parry_chance := parry_chance + parryBonus;

	return parry_chance;
endfunction

function ApplyParryDelay(byRef attackInfo, modifier := 1)
	var parryDelay := CalculateParryDelay(attackInfo);

	SetCooldown(attackInfo.GetAttacker().GetObjRef(), "parrydelay", parryDelay * modifier);
endfunction

function CalculateParryDelay(byRef attackInfo)
	// Aplica delay básico do escudo, reduz delay com base na skill e soma fator de balanceamento
	var parryDelay := CInt(
		-(CInt(attackInfo.GetParryEquip().GetShieldAttributes().GetParryDelay())) + // Parry Delay Básico do escudo
		-(CInt(attackInfo.GetDefender().GetSkill(PARRY) / 20)) + // Parry Delay Bônus de acordo com a skill
		8 // Fator de balanceamento
	);

	// TODO: Desativado porque nem na lista de habilidades está
	// if (MobileWrapper(attackInfo.GetDefender().GetObjRef().party.leader).HasAbility(Ability(ABILITY_ID::INSPIRAR_DEFESAS)))
	var tempParryBonus := CInt(attackInfo.GetDefender().GetProperty("#tempparrybonus"));
	if (tempParryBonus == 100)
		return 0;
	endif

	return CInt(parryDelay);
endfunction

function CheckDisarmShield(byRef attackInfo)
	if (!attackInfo.GetParryEquip().GetObjRef().IsA(POLCLASS_WEAPON))

		if (RandomInt(100 + 1) < attackInfo.GetParryChance())
			// Pike: disarms DEFENDER when blocking without shield
			// TODO: Trocar esse graphic hardcoded por algum identificador de arma do tipo Pike
			if (attackInfo.GetAttacker().GetObjRef().GetGraphic() == 0xFFA8)
				if (attackInfo.GetParryEquip().GetObjType() != 0x1F020)
					var defenderPos := attackInfo.GetDefender().GetPosition();
					defenderPos.SetX(defenderPos.GetX() + RandomIntMinMax(-2, 3));
					defenderPos.SetY(defenderPos.GetY() + RandomIntMinMax(-2, 3));
					MoveObjectToLocation(
						attackInfo.GetParryEquip().GetObjRef(),
						defenderPos.GetX(),
						defenderPos.GetY(),
						GetWorldHeight(defenderPos.GetX(), defenderPos.GetY()),
						attackInfo.GetAttacker().GetPosition().GetRealm(),
						MOVEOBJECT_FORCELOCATION
					);
				endif
			endif
		endif
	endif

	// Execute activated abilities that trigger on parry (e.g. ChallengerManeuver)
	ExecuteCombatAbilitiesOnParry(attackInfo);
	// Execute passive abilities that trigger on parry
	ExecutePassiveAbilitiesOnParry(attackInfo);
endfunction