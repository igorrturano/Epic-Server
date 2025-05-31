use os;

include ":debug:main";
include ":equipsys:utils";
include ":tn:cooldown";

// Verificador de armas para situações especiais
var compatibleWeapons := struct{
	montante := array{0x26CF, 0xF9EA, 0xF9F7}
};

function ExecuteMasteryAbility(byRef attacker, byRef defender, attackInfo)
	var cooldown := GetCooldown(attacker, "MasteryAbilityCooldown");
	if (cooldown > 0)
		DebugMsg(attacker, DEBUG_MASTERY, $"Habilidade de maestria está em cooldown por mais {cooldown} segundos.");
		return false;
	endif

	if (!attackInfo) // Se não tem attackInfo, é um ataque anormal (especial, de engaging, etc.) [pré-combate]
		var combatInfo := struct{
			attacker := attacker,
			defender := defender
		};
		
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
function ExecuteBeforeEngage(combatInfo)
	var weapon := combatInfo.attacker.weapon;

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
			Run_Script(":combat:specialAttackScripts/SpearPush", combatInfo);
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

function CalculateCooldown(byRef combatInfo)
	var cooldown := CInt(5);
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