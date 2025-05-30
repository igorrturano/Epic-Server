use os;

include ":debug:main";
include ":equipsys:utils";
include ":tn:cooldown";

// Verificador de armas para situações especiais
var compatibleWeapons := struct{
	montante := array{0x26CF, 0xF9EA, 0xF9F7}
};

function ExecuteMasteryAbility(byRef attacker, byRef defender, isEngaging := false)
	var weapon, weaponCfg;
	if (GetCooldown(attacker, "MasteryAbility") > 0)
		DebugMsg(attacker, DEBUG_MASTERY, "Habilidade de maestria está em cooldown por mais " + GetCooldown(attacker, "MasteryAbility") + " segundos.");
		return false;
	endif

	weapon := attacker.weapon;
	weaponCfg := getItemCfgElem(attacker.weapon);

	if (isEngaging) // TODO: Analizar pra ver se não é melhor definir como isSpecialAction (pra todos os casos de ações especiais)
		ExecuteBeforeEngage(attacker, defender);
	else
		ExecuteAfterEngage(attacker, defender);
	endif
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
function ExecuteBeforeEngage(byRef attacker, byRef defender)
	var weapon := attacker.weapon;
	var weaponCfg := getItemCfgElem(attacker.weapon);

	if (weapon.objtype in compatibleWeapons.montante)
		ExecuteMasteryAbility_DownwardCut(attacker, defender);
	endif

endfunction

function ExecuteAfterEngage(byRef attacker, byRef defender)
	var weapon := attacker.weapon;
	var weaponCfg := getItemCfgElem(attacker.weapon);

	if (weapon.isRanged())

	else
		if (Upper(weaponCfg.Type) == Upper(UNARMED))
		elseif (weapon.objtype in compatibleWeapons.montante)
			ExecuteMasteryAbility_DownwardCut(attacker, defender);
		elseif (Upper(weaponCfg.Type) == Upper(DAGGER))
		elseif (Upper(weaponCfg.Type) == Upper(SWORD))
			if (weapon.isTwoHanded())
				// ExecuteMasteryAbility_(attacker, defender);
			else
				// ExecuteMasteryAbility_(attacker, defender);
			endif
		elseif (Upper(weaponCfg.Type) == Upper(AXE))
			if (weapon.isTwoHanded())
				// ExecuteMasteryAbility_(attacker, defender);
			else
				// ExecuteMasteryAbility_(attacker, defender);
			endif
		elseif (Upper(weaponCfg.Type) == Upper(SPEAR))
			ExecuteMasteryAbility_Whirldwind(attacker, defender);
		elseif (Upper(weaponCfg.Type) == Upper(MACE))
			if (weapon.isTwoHanded())
				// ExecuteMasteryAbility_(attacker, defender);
			else
				// ExecuteMasteryAbility_(attacker, defender);
			endif
		endif
	endif
endfunction

function CalculateCooldown(byRef attacker)
	var cooldown := CInt(150);
	return cooldown;
endfunction

function ExecuteMasteryAbility_DownwardCut(byRef attacker, byRef defender)
	print("Executing Downward Cut mastery ability for " + attacker.name + " against " + defender.name);
	Run_Script(":combat:specialAttackScripts/DownwardCut", struct{
		attacker := attacker,
		defender := defender
	});
	SetCooldown(attacker, "MasteryAbility", CalculateCooldown(attacker));
endfunction

function ExecuteMasteryAbility_Whirldwind(byRef attacker, byRef defender)
	print("Executing Whirlwind mastery ability for " + attacker.name + " against " + defender.name);
	Run_Script(":combat:specialAttackScripts/Whirlwind", struct{
		attacker := attacker,
		defender := defender
	});
	SetCooldown(attacker, "MasteryAbility", CalculateCooldown(attacker));
endfunction