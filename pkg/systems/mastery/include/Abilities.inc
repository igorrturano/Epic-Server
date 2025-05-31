use os;

include ":debug:main";
include ":equipsys:utils";
include ":tn:cooldown";

// Verificador de armas para situações especiais
var compatibleWeapons := struct{
	montante := array{0x26CF, 0xF9EA, 0xF9F7}
};

function ExecuteMasteryAbility(byRef attacker, byRef defender, isEngaging := false)
	if (GetCooldown(attacker, "MasteryAbility") > 0)
		DebugMsg(attacker, DEBUG_MASTERY, "Habilidade de maestria está em cooldown por mais " + GetCooldown(attacker, "MasteryAbility") + " segundos.");
		return false;
	endif

	var combatInfo := struct{
		attacker := attacker,
		defender := defender
	};

	if (isEngaging) // TODO: Analizar pra ver se não é melhor definir como isSpecialAction (pra todos os casos de ações especiais)
		ExecuteBeforeEngage(combatInfo);
	else
		ExecuteAfterEngage(combatInfo);
	endif

	SetCooldown(attacker, "MasteryAbility", CalculateCooldown(combatInfo));
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
function ExecuteBeforeEngage(combatInfo)
	var weapon := combatInfo.attacker.weapon;
	var weaponCfg := getItemCfgElem(weapon);

	if (weapon.objtype in compatibleWeapons.montante)
		Run_Script(":combat:specialAttackScripts/DownwardCut", combatInfo);
	endif

endfunction

function ExecuteAfterEngage(combatInfo)
	var weapon := combatInfo.attacker.weapon;
	var weaponCfg := getItemCfgElem(weapon);
	Print($"Executing after engage for {combatInfo.attacker.name} with weapon {weapon.name} (type: {weaponCfg.Type})");

	if (weapon.isRanged())

	else
		if (Upper(weaponCfg.Type) == Upper(UNARMED))
		elseif (weapon.objtype in compatibleWeapons.montante)
			Run_Script(":combat:specialAttackScripts/DownwardCut", combatInfo);
		elseif (Upper(weaponCfg.Type) == Upper(DAGGER))
			// Cutthroat
		elseif (Upper(weaponCfg.Type) == Upper(SWORD))
			if (weapon.isTwoHanded())
				// Bloqueia o próximo ataque com a espada
			else
				// First blood
			endif
		elseif (Upper(weaponCfg.Type) == Upper(AXE))
			if (weapon.isTwoHanded())
				// Cleave
			else
				// Knee breaker
			endif
		elseif (Upper(weaponCfg.Type) == Upper(SPEAR))
			// Pushback
		elseif (Upper(weaponCfg.Type) == Upper(MACE))
			if (weapon.isTwoHanded())
				// Derruba o oponente
			else
				// Bone breaker
			endif
		elseif (Upper(weaponCfg.Type) == Upper(POLEARM))
			Run_Script(":combat:specialAttackScripts/Whirlwind", combatInfo);
		endif
	endif
endfunction

function CalculateCooldown(byRef attacker)
	var cooldown := CInt(1);
	return cooldown;
endfunction

// function ExecuteMasteryAbility_DownwardCut(combatInfo)
// 	print("Executing Downward Cut mastery ability for " + combatInfo.attacker.name + " against " + combatInfo.defender.name);
// 	Run_Script(":combat:specialAttackScripts/DownwardCut", combatInfo);
// endfunction

// function ExecuteMasteryAbility_Whirlwind(combatInfo)
// 	print("Executing Whirlwind mastery ability for " + combatInfo.attacker.name + " against " + combatInfo.defender.name);
// 	Run_Script(":combat:specialAttackScripts/Whirlwind", combatInfo);
// endfunction