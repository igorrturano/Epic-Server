use os;

include ":debug:main";
include ":equipsys:utils/consts";
include ":tn:cooldown";
include ":attributes:attributes";

// Verificador de armas para situações especiais
var compatibleWeapons := struct{
	montante := array{0x26CF, 0xF9EA, 0xF9F7}
};

function ExecuteMasteryAbility(attacker, defender, attackInfo)
	var cooldown := GetCooldown(attacker, "MasteryAbilityCooldown");
	if (cooldown > 0)
		DebugMsg(attacker, DEBUG_MASTERY, $"Habilidade de maestria está em cooldown por mais {cooldown} segundos.");
		return false;
	endif

	if (!attackInfo) // Se não tem attackInfo, é um ataque anormal (especial, de engaging, etc.) [pré-combate]
		var combatInfo := struct{};
		combatInfo.+attacker := attacker;
		combatInfo.+defender := defender;
		
		ExecuteBeforeEngage(combatInfo);
	else // Aqui o combate já começou, então é um ataque normal
		var combatInfo := attackInfo;
		combatInfo.+attacker := attacker;
		combatInfo.+defender := defender;

		ExecuteAfterEngage(combatInfo);
	endif

	SetCooldown(attacker, "MasteryAbilityCooldown", CalculateCooldown(attackInfo));
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
function ExecuteBeforeEngage(byRef combatInfo)
	var weapon := combatInfo.attacker.weapon;
	var weaponCfg := weapon.getCfgElem();

	if (weapon.objtype in compatibleWeapons.montante)
		Run_Script(":combat:specialAttackScripts/DownwardCut", combatInfo);
	endif
	if (!weapon.isRanged())
		if (Upper(weaponCfg.Type) == Upper(SWORD))
			if (weapon.isDualWield())
				ExecuteMasteryAbility_Charge(combatInfo);
			endif
		endif
	endif

endfunction

function ExecuteAfterEngage(byRef combatInfo)
	var weapon := combatInfo.attacker.weapon;
	var weaponCfg := weapon.getCfgElem();

	if (weapon.isRanged())
		if (weapon.isBow())
			ExecuteMasteryAbility_DoubleShot(combatInfo);
		elseif (weapon.isCrossbow())
			ExecuteMasteryAbility_Lame(combatInfo);
		endif
	else
		if (Upper(weaponCfg.Type) == Upper(UNARMED))
		elseif (weapon.objtype in compatibleWeapons.montante)
			ExecuteMasteryAbility_DownwardCut(combatInfo);
		elseif (Upper(weaponCfg.Type) == Upper(DAGGER))
			ExecuteMasteryAbility_Cutthroat(combatInfo);
		elseif (Upper(weaponCfg.Type) == Upper(SWORD))
			if (weapon.isTwoHanded())
				ExecuteMasteryAbility_SwordBlock(combatInfo); // TODO: Implementar
			else
				ExecuteMasteryAbility_CounterAttack(combatInfo); // TODO: Implementar
			endif
		elseif (Upper(weaponCfg.Type) == Upper(AXE))
			if (weapon.isTwoHanded())
				ExecuteMasteryAbility_Cleave(combatInfo);
			elseif (weapon.isDualWield())
				
			else
				ExecuteMasteryAbility_KneeBreaker(combatInfo);
			endif
		elseif (Upper(weaponCfg.Type) == Upper(SPEAR))
			ExecuteMasteryAbility_SpearPush(combatInfo);
		elseif (Upper(weaponCfg.Type) == Upper(MACE))
			if (weapon.isTwoHanded())
				ExecuteMasteryAbility_Knockdown(combatInfo);
			else
				ExecuteMasteryAbility_BoneBreaker(combatInfo);
			endif
		elseif (Upper(weaponCfg.Type) == Upper(POLEARM))
			ExecuteMasteryAbility_Whirlwind(combatInfo);
		elseif (Upper(weaponCfg.Type) == Upper(THROWING))
			ExecuteMasteryAbility_Pillum(combatInfo);
		endif
	endif
endfunction

function CalculateCooldown(byRef combatInfo)
	var cooldown := CInt(150); // TODO: fazer basaeado na skill do personagem
	return cooldown;
endfunction

function CalculateEffectTime(attacker)
	// var effectTime := CInt(5);
	var effectTime := CInt(AP_GetSkill(attacker, TACTICS) / 20);
	return effectTime;
endfunction

function ExecuteMasteryAbility_DownwardCut(byRef combatInfo)
	return Run_Script(":combat:specialAttackScripts/DownwardCut", combatInfo);
endfunction

function ExecuteMasteryAbility_Whirlwind(byRef combatInfo)
	return Run_Script(":combat:specialAttackScripts/Whirlwind", combatInfo);
endfunction

function ExecuteMasteryAbility_Cutthroat(byRef combatInfo)
	if (AP_GetVital(combatInfo.defender, "Hits") < (AP_GetVitalMaximumValue(combatInfo.defender, "Hits") * 0.05))
		return Run_Script(":combat:specialAttackScripts/Cutthroat", combatInfo);
	endif
endfunction

function ExecuteMasteryAbility_CounterAttack(byRef combatInfo)
	return Run_Script(":combat:specialAttackScripts/CounterAttack", combatInfo);
endfunction

function ExecuteMasteryAbility_Cleave(byRef combatInfo)
	return Run_Script(":combat:specialAttackScripts/Cleave", combatInfo);
endfunction

function ExecuteMasteryAbility_KneeBreaker(byRef combatInfo)
	combatInfo.+effectTime := CalculateEffectTime(combatInfo.attacker);
	return Run_Script(":combat:specialAttackScripts/KneeBreaker", combatInfo);
endfunction

function ExecuteMasteryAbility_BoneBreaker(byRef combatInfo)
	return Run_Script(":combat:specialAttackScripts/BoneBreaker", combatInfo);
endfunction

function ExecuteMasteryAbility_SpearPush(byRef combatInfo)
	return Run_Script(":combat:specialAttackScripts/SpearPush", combatInfo);
endfunction

function ExecuteMasteryAbility_Knockdown(byRef combatInfo)
	return Run_Script(":combat:specialAttackScripts/Knockdown", combatInfo);
endfunction

function ExecuteMasteryAbility_SwordBlock(byRef combatInfo)
	return Run_Script(":combat:specialAttackScripts/SwordBlock", combatInfo);
endfunction

function ExecuteMasteryAbility_DoubleShot(byRef combatInfo)
	return Start_Script(":combat:specialAttackScripts/DoubleShot", combatInfo);
endfunction

function ExecuteMasteryAbility_Lame(byRef combatInfo)
	combatInfo.+effectTime := CalculateEffectTime(combatInfo.attacker);
	return Run_Script(":combat:specialAttackScripts/Lame", combatInfo);
endfunction

function ExecuteMasteryAbility_Pillum(byRef combatInfo)
	combatInfo.+effectTime := CalculateEffectTime(combatInfo.attacker);
	return Start_Script(":combat:specialAttackScripts/Pillum", combatInfo);
endfunction

function ExecuteMasteryAbility_Charge(byRef combatInfo)
	return Run_Script(":combat:specialAttackScripts/Charge", combatInfo);
endfunction