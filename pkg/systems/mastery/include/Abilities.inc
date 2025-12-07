use os;

include ":debug:Controller/DebugController";
include ":equipsys:utils/consts";
include ":tn:cooldown";
include ":attributes:attributeConstants";
include ":combat:Model/AttackInfo";

function ExecuteMasteryAbility(attackInfo)
	var cooldown := GetCooldown(attackInfo.GetAttacker(), "MasteryAbilityCooldown");
	if (cooldown > 0)
		DebugMsg(attackInfo.GetAttacker(), DEBUG_MASTERY, Message($"Habilidade de maestria está em cooldown por mais {cooldown} segundos."));
		return false;
	endif

	if (attackInfo.GetMoment() == ATTACK_MOMENT::ENGAGING) // Algumas habilidades são aplicadas no engaging
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
	if (attackInfo.GetAttackerWeapon().GetAttribute() != RANGED) // Armas melee
		if (attackInfo.GetAttackerWeapon().GetCategory() == WEAPON_CATEGORY::GREATSWORD)
			ExecuteMasteryAbility_DownwardCut(attackInfo);
		elseif (Upper(attackInfo.GetAttackerWeapon().GetCategory()) == Upper(WEAPON_CATEGORY::SWORD))
			ExecuteMasteryAbility_Charge(attackInfo);
		endif
	else // Armas ranged
		// TODO
	endif
endfunction

function ExecuteAfterEngage(byRef attackInfo)
	if (Upper(attackInfo.GetAttackerWeapon().GetAttribute()) == Upper(RANGED))
		if (Upper(attackInfo.GetAttackerWeapon().GetCategory()) == Upper(WEAPON_CATEGORY::BOW))
			ExecuteMasteryAbility_DoubleShot(attackInfo);
		elseif (Upper(attackInfo.GetAttackerWeapon().GetCategory()) == Upper(WEAPON_CATEGORY::CROSSBOW))
			ExecuteMasteryAbility_Lame(attackInfo);
		endif
	elseif (Upper(attackInfo.GetAttackerWeapon().GetAttribute()) == Upper(TWOHANDED))
		if (Upper(attackInfo.GetAttackerWeapon().GetCategory()) == Upper(WEAPON_CATEGORY::GREATSWORD))
			ExecuteMasteryAbility_DownwardCut(attackInfo);
		elseif (Upper(attackInfo.GetAttackerWeapon().GetCategory()) == Upper(WEAPON_CATEGORY::SWORD))
			ExecuteMasteryAbility_SwordBlock(attackInfo);
		elseif (Upper(attackInfo.GetAttackerWeapon().GetCategory()) == Upper(WEAPON_CATEGORY::AXE))
			ExecuteMasteryAbility_Cleave(attackInfo);
		elseif (Upper(attackInfo.GetAttackerWeapon().GetCategory()) == Upper(WEAPON_CATEGORY::SPEAR))
			ExecuteMasteryAbility_SpearPush(attackInfo);
		elseif (Upper(attackInfo.GetAttackerWeapon().GetCategory()) == Upper(WEAPON_CATEGORY::MACE))
			ExecuteMasteryAbility_Knockdown(attackInfo);
		elseif (Upper(attackInfo.GetAttackerWeapon().GetCategory()) == Upper(WEAPON_CATEGORY::POLEARM))
			ExecuteMasteryAbility_Whirlwind(attackInfo);
		elseif (Upper(attackInfo.GetAttackerWeapon().GetCategory()) == Upper(WEAPON_CATEGORY::THROWING))
			ExecuteMasteryAbility_Pillum(attackInfo);
		endif
	elseif (Upper(attackInfo.GetAttackerWeapon().GetAttribute()) == Upper(ONEHANDED))
		if (Upper(attackInfo.GetAttackerWeapon().GetCategory()) == Upper(WEAPON_CATEGORY::DAGGER))
			ExecuteMasteryAbility_Cutthroat(attackInfo);
		elseif (Upper(attackInfo.GetAttackerWeapon().GetCategory()) == Upper(WEAPON_CATEGORY::SWORD))
			ExecuteMasteryAbility_CounterAttack(attackInfo);
		elseif (Upper(attackInfo.GetAttackerWeapon().GetCategory()) == Upper(WEAPON_CATEGORY::AXE))
			ExecuteMasteryAbility_KneeBreaker(attackInfo);
		elseif (Upper(attackInfo.GetAttackerWeapon().GetCategory()) == Upper(WEAPON_CATEGORY::SPEAR))
			ExecuteMasteryAbility_SpearPush(attackInfo);
		elseif (Upper(attackInfo.GetAttackerWeapon().GetCategory()) == Upper(WEAPON_CATEGORY::MACE))
			ExecuteMasteryAbility_BoneBreaker(attackInfo);
		endif
	elseif (Upper(attackInfo.GetAttackerWeapon().GetAttribute()) == Upper(DUALWIELD))
		if (Upper(attackInfo.GetAttackerWeapon().GetCategory()) == Upper(WEAPON_CATEGORY::AXE))
			// TODO: Habilidade de maestria para MACHADOS em dual wield
		elseif (Upper(attackInfo.GetAttackerWeapon().GetCategory()) == Upper(WEAPON_CATEGORY::DAGGER))
			// TODO: Habilidade de maestria para ADAGAS em dual wield
		elseif (Upper(attackInfo.GetAttackerWeapon().GetCategory()) == Upper(WEAPON_CATEGORY::SWORD))
			// TODO: Habilidade de maestria para ESPADA em dual wield
		endif
	endif
endfunction

function CalculateCooldown(byRef attackInfo)
	var maxTime := 160;
	var weaponSkill := attackInfo.GetAttacker().GetSkill(attackInfo.GetAttackerWeapon().GetAttribute());
	var mod := 1; // TODO: Balancear o modificador
	
	return (maxTime) - (weaponSkill / mod);
endfunction

function CalculateEffectTime(byRef attackInfo)
	return CInt(attackInfo.GetAttacker().GetAttribute(TACTICS) / 20);
endfunction

function ExecuteMasteryAbility_DownwardCut(byRef attackInfo)
	return Run_Script(":combat:specialAttackScripts/downwardcut", attackInfo);
endfunction

function ExecuteMasteryAbility_Whirlwind(byRef attackInfo)
	return Run_Script(":combat:specialAttackScripts/whirlwind", attackInfo);
endfunction

function ExecuteMasteryAbility_Cutthroat(byRef attackInfo)
	if (attackInfo.GetDefender().GetCurrentHP() < (attackInfo.GetDefender().GetMaxHP() * 0.05))
		return Run_Script(":combat:specialAttackScripts/cutthroat", attackInfo);
	endif
endfunction

function ExecuteMasteryAbility_CounterAttack(byRef attackInfo)
	return Run_Script(":combat:specialAttackScripts/counterattack", attackInfo);
endfunction

function ExecuteMasteryAbility_Cleave(byRef attackInfo)
	return Run_Script(":combat:specialAttackScripts/cleave", attackInfo);
endfunction

function ExecuteMasteryAbility_KneeBreaker(byRef attackInfo)
	return Run_Script(":combat:specialAttackScripts/kneebreaker", array{attackInfo, CalculateEffectTime(attackInfo)});
endfunction

function ExecuteMasteryAbility_BoneBreaker(byRef attackInfo)
	return Run_Script(":combat:specialAttackScripts/bonebreaker", attackInfo);
endfunction

function ExecuteMasteryAbility_SpearPush(byRef attackInfo)
	return Run_Script(":combat:specialAttackScripts/spearpush", attackInfo);
endfunction

function ExecuteMasteryAbility_Knockdown(byRef attackInfo)
	return Run_Script(":combat:specialAttackScripts/knockdown", attackInfo);
endfunction

function ExecuteMasteryAbility_SwordBlock(byRef attackInfo)
	return Run_Script(":combat:specialAttackScripts/swordblock", attackInfo);
endfunction

function ExecuteMasteryAbility_DoubleShot(byRef attackInfo)
	return Start_Script(":combat:specialAttackScripts/doubleshot", attackInfo);
endfunction

function ExecuteMasteryAbility_Lame(byRef attackInfo)
	return Run_Script(":combat:specialAttackScripts/lame", array{attackInfo, CalculateEffectTime(attackInfo)});
endfunction

function ExecuteMasteryAbility_Pillum(byRef attackInfo)
	return Start_Script(":combat:specialAttackScripts/pillum", array{attackInfo, CalculateEffectTime(attackInfo)});
endfunction

function ExecuteMasteryAbility_Charge(byRef attackInfo)
	return Run_Script(":combat:specialAttackScripts/charge", attackInfo);
endfunction