use os;

include ":debug:main";
include ":equipsys:utils";
include ":tn:cooldown";

var CompatibleWeapons := struct{};
// CompatibleWeapons["GAREN"] := array{"Sword", "Axe"};

function ExecuteMasteryAbility(byRef attacker, byRef defender)
	var weapon, weaponCfg;
	if (GetCooldown(attacker, "MasteryAbility") > 0)
		DebugMsg(attacker, DEBUG_MASTERY, "Habilidade de maestria está em cooldown por mais " + GetCooldown(attacker, "MasteryAbility") + " segundos.");
		return false;
	endif

	weapon := attacker.weapon;
	weaponCfg := getItemCfgElem(attacker.weapon);
	print("Weapon type: " + weaponCfg.Type);

	if (weapon.isRanged())

	else
		if (Upper(weaponCfg.Type) == Upper(UNARMED))
		elseif (Upper(weaponCfg.Type) == Upper(DAGGER))
		elseif (Upper(weaponCfg.Type) == Upper(SWORD))
			if (weapon.isTwoHanded())
				ExecuteMasteryAbility_DownwardCut(attacker, defender);
			else
				// ExecuteMasteryAbility_ShadowDash(attacker, defender);
			endif
		elseif (Upper(weaponCfg.Type) == Upper(AXE))
			if (weapon.isTwoHanded())
				// ExecuteMasteryAbility_(attacker, defender);
			else
				// ExecuteMasteryAbility_(attacker, defender);
			endif
		elseif (Upper(weaponCfg.Type) == Upper(SPEAR))
			if (weapon.isTwoHanded())
				// ExecuteMasteryAbility_(attacker, defender);
			else
				// ExecuteMasteryAbility_(attacker, defender);
			endif
		elseif (Upper(weaponCfg.Type) == Upper(MACE))
			if (weapon.isTwoHanded())
				// ExecuteMasteryAbility_(attacker, defender);
			else
				// ExecuteMasteryAbility_(attacker, defender);
			endif
		endif
	endif

endfunction

function ExecuteMasteryAbility_DownwardCut(byRef attacker, byRef defender)
	print("Executing Downward Cut mastery ability for " + attacker.name + " against " + defender.name);
	Run_Script(":combat:specialAttackScripts/DownwardCut", struct{
		attacker := attacker,
		defender := defender
	});
	SetCooldown(attacker, "MasteryAbility", 20);
endfunction