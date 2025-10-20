use os;

include ":debug:Controller/DebugController";
include ":equipsys:utils/consts";
include ":tn:cooldown";
include ":attributes:attributes";
include ":combat:Model/AttackInfo";

function ExecuteMasteryAbility(attackInfo)
	var cooldown := GetCooldown(attackInfo.GetAttacker(), "MasteryAbilityCooldown");
	if (cooldown > 0)
		DebugMsg(attackInfo.GetAttacker(), DEBUG_MASTERY, $"Habilidade de maestria está em cooldown por mais {cooldown} segundos.");
		return false;
	endif

	if (attackInfo.GetMoment() == ATTACKMOMENT::ENGAGING) // Algumas habilidades são aplicadas no engaging
		ExecuteBeforeEngage(attackInfo);
	else
		ExecuteAfterEngage(attackInfo);
	endif

	SetCooldown(attackInfo.GetAttacker(), "MasteryAbilityCooldown", CalculateCooldown(attackInfo.GetAttacker()));
	return true;
endfunction

/**
 * ExecuteBeforeEngage(byRef attacker, byRef defender)
 *
 * Executa habilidades antes do engajamento de combate baseado no tipo de arma do atacante.
 * Esta função verifica se a arma do atacante é do tipo montante e executa a habilidade
 * correspondente "Corte Descendente" caso a condição seja atendida.
 * 
 * @param attacker Referência ao personagem atacante que possui a arma
 * @param defender Referência ao personagem defensor que receberá o ataque
 */
function ExecuteBeforeEngage(byRef attackInfo)
	if (attackInfo.GetAttackerWeapon().IsMelee()) // Armas melee
		if (attackInfo.GetAttackerWeapon().IsMontante())
			ExecuteMasteryAbility_DownwardCut(attackInfo);
		elseif (Upper(attackInfo.GetAttackerWeapon().GetType()) == Upper(SWORD))
			ExecuteMasteryAbility_Charge(attackInfo);
		endif
	else // Armas ranged
		// TODO
	endif
endfunction

function ExecuteAfterEngage(byRef attackInfo)
	if (attackInfo.GetAttackerWeapon().IsRanged())
		if (attackInfo.GetAttackerWeapon().IsBow())
			ExecuteMasteryAbility_DoubleShot(attackInfo);
		elseif (attackInfo.GetAttackerWeapon().IsCrossbow())
			ExecuteMasteryAbility_Lame(attackInfo);
		endif
	elseif (attackInfo.GetAttackerWeapon().IsTwoHanded())
		if (attackInfo.GetAttackerWeapon().IsMontante())
			ExecuteMasteryAbility_DownwardCut(attackInfo);
		elseif (Upper(attackInfo.GetAttackerWeapon().GetType()) == Upper(SWORD))
			ExecuteMasteryAbility_SwordBlock(attackInfo);
		elseif (Upper(attackInfo.GetAttackerWeapon().GetType()) == Upper(AXE))
			ExecuteMasteryAbility_Cleave(attackInfo);
		elseif (Upper(attackInfo.GetAttackerWeapon().GetType()) == Upper(SPEAR))
			ExecuteMasteryAbility_SpearPush(attackInfo);
		elseif (Upper(attackInfo.GetAttackerWeapon().GetType()) == Upper(MACE))
			ExecuteMasteryAbility_Knockdown(attackInfo);
		elseif (Upper(attackInfo.GetAttackerWeapon().GetType()) == Upper(POLEARM))
			ExecuteMasteryAbility_Whirlwind(attackInfo);
		elseif (Upper(attackInfo.GetAttackerWeapon().GetType()) == Upper(THROWING))
			ExecuteMasteryAbility_Pillum(attackInfo);
		endif
	elseif (attackInfo.GetAttackerWeapon().IsOneHanded())
		if (Upper(attackInfo.GetAttackerWeapon().GetType()) == Upper(DAGGER))
			ExecuteMasteryAbility_Cutthroat(attackInfo);
		elseif (Upper(attackInfo.GetAttackerWeapon().GetType()) == Upper(SWORD))
			ExecuteMasteryAbility_CounterAttack(attackInfo);
		elseif (Upper(attackInfo.GetAttackerWeapon().GetType()) == Upper(AXE))
			ExecuteMasteryAbility_KneeBreaker(attackInfo);
		elseif (Upper(attackInfo.GetAttackerWeapon().GetType()) == Upper(SPEAR))
			ExecuteMasteryAbility_SpearPush(attackInfo);
		elseif (Upper(attackInfo.GetAttackerWeapon().GetType()) == Upper(MACE))
			ExecuteMasteryAbility_BoneBreaker(attackInfo);
		endif
	elseif (attackInfo.GetAttackerWeapon().IsDualWield())
		if (Upper(attackInfo.GetAttackerWeapon().GetType()) == Upper(AXE))
			// TODO: Habilidade de maestria para machados em dual wield
		endif
	endif
endfunction

function CalculateCooldown(byRef attackInfo)
	var weaponSkill := AP_GetSkill(attackInfo.GetAttacker(), attackInfo.GetAttackerWeapon().GetAttribute());
	var cooldown := CInt(160) - (weaponSkill / 1); // TODO: Ajustar a fórmula de cooldown
	return cooldown;
endfunction

function CalculateEffectTime(byRef attackInfo)
	// var effectTime := CInt(5);
	var effectTime := CInt(attackInfo.GetAttacker().GetAttribute(TACTICS) / 20);
	// var effectTime := CInt(AP_GetSkill(attackInfo.GetAttacker(), TACTICS) / 20);
	return effectTime;
endfunction

function ExecuteMasteryAbility_DownwardCut(byRef combatInfo)
	return Run_Script(":combat:specialAttackScripts/downwardcut", combatInfo);
endfunction

function ExecuteMasteryAbility_Whirlwind(byRef combatInfo)
	return Run_Script(":combat:specialAttackScripts/whirlwind", combatInfo);
endfunction

function ExecuteMasteryAbility_Cutthroat(byRef combatInfo)
	if (AP_GetVital(combatInfo.defender, "Hits") < (AP_GetVitalMaximumValue(combatInfo.defender, "Hits") * 0.05))
		return Run_Script(":combat:specialAttackScripts/cutthroat", combatInfo);
	endif
endfunction

function ExecuteMasteryAbility_CounterAttack(byRef combatInfo)
	return Run_Script(":combat:specialAttackScripts/counterattack", combatInfo);
endfunction

function ExecuteMasteryAbility_Cleave(byRef combatInfo)
	return Run_Script(":combat:specialAttackScripts/cleave", combatInfo);
endfunction

function ExecuteMasteryAbility_KneeBreaker(byRef combatInfo)
	combatInfo.+effectTime := CalculateEffectTime(combatInfo.attacker);
	return Run_Script(":combat:specialAttackScripts/kneebreaker", combatInfo);
endfunction

function ExecuteMasteryAbility_BoneBreaker(byRef combatInfo)
	return Run_Script(":combat:specialAttackScripts/bonebreaker", combatInfo);
endfunction

function ExecuteMasteryAbility_SpearPush(byRef combatInfo)
	return Run_Script(":combat:specialAttackScripts/spearpush", combatInfo);
endfunction

function ExecuteMasteryAbility_Knockdown(byRef combatInfo)
	return Run_Script(":combat:specialAttackScripts/knockdown", combatInfo);
endfunction

function ExecuteMasteryAbility_SwordBlock(byRef combatInfo)
	return Run_Script(":combat:specialAttackScripts/swordblock", combatInfo);
endfunction

function ExecuteMasteryAbility_DoubleShot(byRef combatInfo)
	return Start_Script(":combat:specialAttackScripts/doubleshot", combatInfo);
endfunction

function ExecuteMasteryAbility_Lame(byRef combatInfo)
	combatInfo.+effectTime := CalculateEffectTime(combatInfo.attacker);
	return Run_Script(":combat:specialAttackScripts/lame", combatInfo);
endfunction

function ExecuteMasteryAbility_Pillum(byRef combatInfo)
	combatInfo.+effectTime := CalculateEffectTime(combatInfo.attacker);
	return Start_Script(":combat:specialAttackScripts/pillum", combatInfo);
endfunction

function ExecuteMasteryAbility_Charge(byRef combatInfo)
	return Run_Script(":combat:specialAttackScripts/charge", combatInfo);
endfunction