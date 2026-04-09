use uo;
use os;

include ":attributes:attributes";
include ":combat:settings";
include ":spells:include/swordmage";
include ":runecarving:runeCarving";

function WearDownArmor(defender, armor, rawdamage, weapon)
	var bonus;
	var wearchance := 30;
	//Amplia o dano na armadura se a arma tiver a propriedade
	if (CInt(GetObjProperty(weapon, "ArmorDmg") == 1))
		bonus := RandomDiceRoll("1d4");
	endif
	var cfg := ReadConfigFile(":combat:itemdesc");
	var elem := FindConfigElem(cfg, armor.objtype);

	if (armor)
		var armr_elem := CS_GetSettingsCfgElem("Armor");

		// Verifica a poncentagem de manter equipado o item
		var dmg_to_wear := CDbl(CDbl(armr_elem.DmgToWear) * CDbl(armor.hp)) / 100.0;
		if (GetObjProperty(armor, "encerada") && elem.TipoProtecao)
			// Reduz a chance de desgaste em 10% se a armadura estiver encerada
			wearchance := wearchance - 15;
			if (wearchance < 5)
				wearchance := 5;
			endif
		endif

		//if (CInt(rawdamage) > dmg_to_wear)
		if (RandomDiceRoll("1d100") <= wearchance)
			if (bonus > 0)
				SendSysMessage(defender, armor.desc+" foi bem danificada.");
			else
				SendSysMessage(defender, armor.desc+" foi danificada.");
			endif
			var armorhp := armor.hp;
			armorhp := armorhp - Cint(armr_elem.WearRate + bonus);
			if ( armorhp <= 0 )
				SendSysMessage(defender, armor.desc+" foi destruida.");
				if (!GetObjProperty(armor, "broken"))
					SetObjProperty(armor, "broken", 1);
					SetObjProperty(armor, "realdesc", armor.desc);
					SetName(armor, armor.desc+" [danificado]");
				endif
				MoveItemToContainer(armor, defender.backpack);
				//DestroyItem(armor);
			elseif ( armor.hp > armor.maxhp )
				armor.hp := armor.maxhp;
			else
				armor.hp := armorhp;
			endif
			var nome := armor.name;
			SetName(armor, nome + " -");
			sleepms(30);
			SetName(armor, nome);
		endif
		//endif

		// Check for priest rune charges
		var hasRune := GetObjProperty(armor, "priestrune");
		if (hasRune)
			var charges := CInt(GetObjProperty(armor, "runecharges"));
			if (charges > 0)
				charges := charges - 1;
				SetObjProperty(armor, "runecharges", charges);

				if (charges <= 0)
					// Rune blessing worn out
					SendSysMessageEx(defender, "A benção divina da " + hasRune.name + " em " + armor.desc + " se esvaiu.", SSM_INFO);
					PlaySoundEffect(defender, SFX_SPELL_DISPEL);

					// Remove rune bonuses
					removeRuneBonus(armor, hasRune);

					// Update item name to remove [Rúnico]
					var currentName := armor.name;
					currentName := SubStrReplace(currentName, " [Rúnico]", "");
					SetName(armor, currentName);
				endif
			endif
		endif
	endif

	return 1;
endfunction

function WearDownShield(defender, shield, rawdamage, weapon)
	if (shield.objtype == 0x1F020) // Se for escudo natural, não desgasta
		return false;
	endif
	
	var bonus := 0;
	var wearchance;
	if (shield.IsA(POLCLASS_WEAPON))
		wearchance := 20;
	else
		wearchance := 80;
	endif
	//Amplia o dano no escudo se a arma tiver a propriedade
	if (CInt(GetObjProperty(weapon, "ArmorDmg") == 1))
		bonus := RandomInt(3)+1;
	endif

	if (shield)
		var armr_elem := CS_GetSettingsCfgElem("Parry");

		// Verifica a poncentagem de manter equipado o item
		var dmg_to_wear := CDbl(CDbl(armr_elem.DmgToWear) * CDbl(shield.hp)) / 100.0;

		//Diminui pela metade a chance de um equipamento quebrar com a habilidde Conservar Equipamento
		if (GetObjProperty(shield, "encerada"))
			// Reduz a chance de desgaste em 10% se a armadura estiver encerada
			wearchance := wearchance - 15;
			if (wearchance < 5)
				wearchance := 5;
			endif
		endif

		//if (CInt(rawdamage) > dmg_to_wear)
		if (RandomDiceRoll("1d100") <= wearchance)
			SendSysMessage(defender, shield.desc+" foi danificado.");
			var shieldhp := shield.hp;
			shieldhp := shieldhp - Cint(armr_elem.WearRate + bonus);
			if ( shieldhp <= 0 )
				SendSysMessage(defender, shield.desc+" foi destruido.");
				if (!GetObjProperty(shield, "broken"))
					SetObjProperty(shield, "broken", 1);
					SetObjProperty(shield, "realdesc", shield.desc);
					SetName(shield, shield.desc+" [danificado]");
				endif
				MoveItemToContainer(shield, defender.backpack);
				//DestroyItem(shield);
			elseif (shield.hp > shield.maxhp)
				shield.hp := shield.maxhp;
			else
				shield.hp := shieldhp;
			endif
			var nome := shield.name;
			SetName(shield, nome + " -");
			sleepms(30);
			SetName(shield, nome);
		endif
	endif
	return 1;
endfunction

function WearDownWeapon(attacker, weapon,wearRateMlt := 1)
	if (weapon.IsA(POLCLASS_WEAPON))
		//Se está desarmado não danifique a arma natural
		if (weapon.objtype == 0x1F020)
			return 1;
		//Calcula danos nas armas normais
		else
			var wpn_elem := CS_GetSettingsCfgElem("Weapons");
			var wearchance := 2;

			if (GetObjProperty(weapon, "encerada"))
				// Reduz a chance de desgaste em 10% se a armadura estiver encerada
				wearchance := wearchance - 15;
				if (wearchance < 5)
					wearchance := 5;
				endif
			endif
			
			if ( RandomDiceRoll("1d100") <= wearchance )
				var weaponhp := weapon.hp - 1 * wearRateMlt;

				if ( weaponhp <= 0 )
					SendSysMessage(attacker, weapon.desc+" quebrou.");
					MoveObjectToLocation(weapon, 1, 1, 1, attacker.realm, MOVEOBJECT_FORCELOCATION);
					if (!GetObjProperty(weapon, "broken"))
						SetObjProperty(weapon, "broken", 1);
						SetObjProperty(weapon, "realdesc", weapon.desc);
						SetName(weapon, weapon.desc+" [danificado]");
					endif
					releasegems(attacker, weapon);
					eraseobjproperty(weapon, "gems");
					MoveItemToContainer(weapon, attacker.backpack);
					//DestroyItem(weapon);
				elseif ( weapon.hp > weapon.maxhp )
					weapon.hp := weapon.maxhp;
				else
					weapon.hp := weaponhp;
				endif
				var nome := weapon.name;
				SetName(weapon, nome + " -");
				sleepms(30);
				SetName(weapon, nome);
			endif

			// Check for priest rune charges (Holy Weapon rune)
			var hasRune := GetObjProperty(weapon, "priestrune");
			if (hasRune)
				var charges := CInt(GetObjProperty(weapon, "runecharges"));
				if (charges > 0)
					charges := charges - 1;
					SetObjProperty(weapon, "runecharges", charges);

					if (charges <= 0)
						// Rune blessing worn out
						SendSysMessageEx(attacker, "A benção divina da " + hasRune.name + " em " + weapon.desc + " se esvaiu.", SSM_INFO);
						PlaySoundEffect(attacker, SFX_SPELL_DISPEL);

						// Remove rune bonuses
						removeRuneBonus(weapon, hasRune);

						// Update item name to remove [Rúnico]
						var currentName := weapon.name;
						currentName := SubStrReplace(currentName, " [Rúnico]", "");
						SetName(weapon, currentName);
					endif
				endif
			endif

			return 1;
		endif
	else
		return 0;
	endif
endfunction