use uo;
use os;

include ":attributes:attributes";
include ":combat:settings";
include ":spells:include/swordmage";

function WearDownArmor(defender, armor, rawdamage, weapon)
	var bonus;
	//Amplia o dano na armadura se a arma tiver a propriedade
	if (CInt(GetObjProperty(weapon, "ArmorDmg") == 1))
		bonus := RandomDiceRoll("1d4");
	endif
	var cfg := ReadConfigFile(":combat:itemdesc");
	var elem := FindConfigElem(cfg, armor.objtype);

	if (!GetObjProperty(armor, "encerada") && elem.TipoProtecao)
		bonus := RandomDiceRoll("1d4");
	endif

	if (armor)
		var armr_elem := CS_GetSettingsCfgElem("Armor");

		// Verifica a poncentagem de manter equipado o item
		var dmg_to_wear := CDbl(CDbl(armr_elem.DmgToWear) * CDbl(armor.hp)) / 100.0;
		//var dmg_to_wear := CInt(CInt(armr_elem.DmgToWear) * CInt(armor.hp) / 100);

		//Diminui pela metade a chance de um equipamento quebrar com a habilidde Conservar Equipamento
		var wearchance := armr_elem.WearChance;
		if ( TemHabilidade(defender, "Conservar Equipamento"))
			wearchance := wearchance/2;
			//if (wearchance < 10)
			//	wearchance := 10;
			//endif
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
				SetObjProperty(armor, "broken", 1);
				SetObjProperty(armor, "realdesc", armor.desc);
				SetName(armor, armor.desc+" [danificado]");
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
	endif

	return 1;
endfunction

function WearDownShield(defender, shield, rawdamage, weapon)

	var bonus := 0;
	//Amplia o dano no escudo se a arma tiver a propriedade
	if (CInt(GetObjProperty(weapon, "ArmorDmg") == 1))
		bonus := RandomInt(3)+1;
	endif

	if (shield)
		var armr_elem := CS_GetSettingsCfgElem("Parry");

		// Verifica a poncentagem de manter equipado o item
		//var dmg_to_wear := CInt(CInt(armr_elem.DmgToWear) * CInt(shield.hp) / 100);
		var dmg_to_wear := CDbl(CDbl(armr_elem.DmgToWear) * CDbl(shield.hp)) / 100.0;

		//Diminui pela metade a chance de um equipamento quebrar com a habilidde Conservar Equipamento
		var wearchance := armr_elem.WearChance;
		if ( TemHabilidade(defender, "Conservar Equipamento"))
			wearchance := wearchance/2;
			//if (wearchance < 20)
			//	wearchance := 20;
			//endif
		endif
		//if (CInt(rawdamage) > dmg_to_wear)
		if (RandomDiceRoll("1d100") <= wearchance)
			SendSysMessage(defender, shield.desc+" foi danificado.");
			var shieldhp := shield.hp;
			shieldhp := shieldhp - Cint(armr_elem.WearRate + bonus);
			if ( shieldhp <= 0 )
				SendSysMessage(defender, shield.desc+" foi destruido.");
				SetObjProperty(shield, "broken", 1);
				SetObjProperty(shield, "realdesc", shield.desc);
				SetName(shield, shield.desc+" [danificado]");
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
			var wearchance := CDbl(wpn_elem.WearChance);

			//Diminui pela metade a chance de um equipamento quebrar com a habilidde Conservar Equipamento
			if (TemHabilidade(attacker, "Conservar Equipamento"))
				wearchance := wearchance/2;
				//if (wearchance < 10)
				//	wearchance := 10;
				//endif
			endif

			if ( RandomDiceRoll("1d100") <= wearchance )
				var weaponhp := weapon.hp - 1 * wearRateMlt;

				if ( weaponhp <= 0 )
					SendSysMessage(attacker, weapon.desc+" quebrou.");
					MoveObjectToLocation(weapon, 1, 1, 1, attacker.realm, MOVEOBJECT_FORCELOCATION);
					SetObjProperty(weapon, "broken", 1);
					SetObjProperty(weapon, "realdesc", weapon.desc);
					releasegems(attacker, weapon);
					eraseobjproperty(weapon, "gems");
					SetName(weapon, weapon.desc+" [danificado]");
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
			return 1;
		endif
	else
		return 0;
	endif
endfunction