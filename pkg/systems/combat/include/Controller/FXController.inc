/* $Id$
 *
 * Purpose
 * Criado para tratar os efeitos (animação e som) do combate
 *
 */

use util;
use cfgfile;

include "include/client";
include "include/epicConstants";
include ":attributes:attributeConstants";
include ":objWrapper:Model/WeaponWrapper";
include ":objWrapper:Model/MobileWrapper";

function PlayAttackFX(byRef attackInfo)
	var attackerAnim := GetAttackerFX(attackInfo.GetAttacker(), attackInfo.GetAttackType(), attackInfo.GetDamage().IsCritical());
	var defenderAnim := GetDefenderFX(attackInfo.GetDefender(), attackInfo.Succeded());

	attackInfo.GetAttacker().PlayAnimation(attackerAnim);
	if (defenderAnim) attackInfo.GetDefender().PlayAnimation(defenderAnim); endif
endfunction

function PlayAttackSFX(byRef attackInfo)
	var weaponSound;
	var attackerSound;
	var defenderSound;

	if (attackInfo.Succeded())
		if (!attackInfo.GetAttacker().GetEquipBySlot(WEAPON) && attackInfo.GetAttacker().IsNPC())
			weaponSound := attackInfo.GetAttacker().GetNPCTemplateCfg().AttackSound;
		else
			attackerSound := GetAttackerSFX(attackInfo.GetAttacker(), attackInfo.GetDamage().IsCritical(), attackInfo.GetDefender().IsDead());
			weaponSound := GetWeaponSFX(attackInfo.GetAttacker().GetEquipBySlot(WEAPON), attackInfo.GetDamage().IsCritical());
		endif
		
		defenderSound := GetDefenderSFX(attackInfo.GetDefender(), attackInfo.GetDamage().IsCritical());
	else
		if (attackInfo.GetAttacker().IsNPC() && !attackInfo.GetAttacker().GetEquipBySlot(WEAPON))
			weaponSound := attackInfo.GetAttacker().GetNPCTemplateCfg().AttackMissSound;
		else
			weaponSound := GetWeaponSFX(attackInfo.GetAttacker().GetEquipBySlot(WEAPON), attackInfo.GetAttackType(), attackInfo.GetDamage().IsCritical(), true);
		endif
	endif

	attackInfo.GetAttacker().PlaySound(weaponSound);
	if (attackerSound) attackInfo.GetAttacker().PlaySound(attackerSound); endif
	if (defenderSound) attackInfo.GetDefender().PlaySound(defenderSound); endif
endfunction

function PlayDefenseAnimation(byRef attackInfo)
	var defenseAnim := GetDefenseFX(attackInfo.GetDefender(), attackInfo.GetDamage().IsCritical());

	attackInfo.GetDefender().PlayAnimation(defenseAnim);
endfunction

function GetAttackerFX(attacker, attackType, isCriticalAttack := false)
	var attackerFX := struct{
		hit := array{},
		critical := array{}
	};
	
	if (attacker.IsNPC() && !attacker.GetEquipBySlot(WEAPON))
		attackerFX.hit := array{ 0x0c };
		attackerFX.critical := array{ 0x0c };
	else
		if (attacker.GetEquipBySlot(WEAPON).GetCategory() == UNARMED)
			attackerFX.hit := array{ 0x9, 0xA, 0xB };
			attackerFX.critical := array{ 0x1F };
		elseif (attacker.GetEquipBySlot(WEAPON).GetCategory() == BOW)
			attackerFX.hit := array{ 0x12 };
			attackerFX.critical := array{ 0x12 };
		elseif (attacker.GetEquipBySlot(WEAPON).GetCategory() == CROSSBOW)
			attackerFX.hit := array{ 0x13 };
			attackerFX.critical := array{ 0x13 };
		else
			if (Upper(attacker.GetEquipBySlot(WEAPON).GetAttribute()) == Upper(TWOHANDED))
				if (attackType == Upper(SLASHDAMAGE))
					attackerFX.hit := array{ 0xC };
					attackerFX.critical := array{ 0xC };
				elseif (attackType == Upper(PIERCINGDAMAGE))
					attackerFX.hit := array{ 0xE };
					attackerFX.critical := array{ 0xE };
				elseif (attackType == Upper(BLUNTDAMAGE))
					attackerFX.hit := array{ 0xB };
					attackerFX.critical := array{ 0xB };
				endif
			else
				if (attackType == Upper(SLASHDAMAGE))
					attackerFX.hit := array{ 0xD };
					attackerFX.critical := array{ 0xD };
				elseif (attackType == Upper(PIERCINGDAMAGE))
					attackerFX.hit := array{ 0xA };
					attackerFX.critical := array{ 0xA };
				elseif (attackType == Upper(BLUNTDAMAGE))
					attackerFX.hit := array{ 0xB };
					attackerFX.critical := array{ 0xB };
				endif
			endif
		endif
	endif

	if (isCriticalAttack)
		attackerFX := attackerFX.critical;
	else
		attackerFX := attackerFX.hit;
	endif

	attackerFX := attackerFX.Size() > 1 ? attackerFX[RandomIntMinMax(1, attackerFX.Size() + 1)] : attackerFX[1];

	return CInt(attackerFX);
endfunction

function GetDefenderFX(defender, isMiss := false)
	var defenderFX := struct{
		hit := array{},
		miss := array{}
	};

	if (defender.IsNPC())
		defenderFX.hit := array{ 0x0C };
		defenderFX.miss := array{};
	else
		defenderFX.hit := array{ 20 };
		defenderFX.miss := array{};
	endif

	if (isMiss)
		defenderFX := defenderFX.miss;
	else
		defenderFX := defenderFX.hit;
	endif

	defenderFX := defenderFX.Size() > 1 ? defenderFX[RandomIntMinMax(1, defenderFX.Size() + 1)] : defenderFX[1];

	return CInt(defenderFX);
endfunction

function GetWeaponSFX(weapon, attackType := false, isCriticalAttack := false, isMiss := false)
	var weaponSFX := struct{
		hit := array{},
		miss := array{},
		critical := array{}
	};

	if (weapon == error)
		weaponSFX.hit := array{ 0x0239, 0x023A, 0x023B, 0x539, 0x53A, 0x53B };
		weaponSFX.miss := array{ 0x0239, 0x023A, 0x023B, 0x539, 0x53A, 0x53B };
		weaponSFX.critical := array{ 0x0239, 0x023A, 0x023B, 0x539, 0x53A, 0x53B };
	elseif (weapon.GetCategory() == UNARMED)
		weaponSFX.hit := array{ 0x136, 0x138, 0x13C, 0x13D, 0x13E, 0x142, 0x145, 0x146, 0x149, 0x14A, 0x14B };
		weaponSFX.miss := array{ 0x0239, 0x023A, 0x023B, 0x539, 0x53A, 0x53B };
		weaponSFX.critical := array{};
	elseif (weapon.GetCategory() == BOW)
		weaponSFX.hit := array{ 0x235, 0x4C9, 0x4CA };
		weaponSFX.miss := array{ 0x0239, 0x023A, 0x023B, 0x539, 0x53A, 0x53B };
		weaponSFX.critical := array{};
	elseif (weapon.GetCategory() == CROSSBOW)
		weaponSFX.hit := array{ 0x235, 0x4C9, 0x4CA };
		weaponSFX.miss := array{ 0x0239, 0x023A, 0x023B, 0x539, 0x53A, 0x53B };
		weaponSFX.critical := array{};
	else
		if (Upper(weapon.GetAttribute()) == Upper(TWOHANDED))
			if (attackType == Upper(SLASHDAMAGE))
				weaponSFX.hit := array{ 0x237, 0x238 };
				weaponSFX.miss := array{ 0x0239, 0x023A, 0x023B, 0x539, 0x53A, 0x53B };
				weaponSFX.critical := array{};
			elseif (attackType == Upper(PIERCINGDAMAGE))
				weaponSFX.hit := array{ 0x23D, 0x23D };
				weaponSFX.miss := array{ 0x0239, 0x023A, 0x023B, 0x539, 0x53A, 0x53B };
				weaponSFX.critical := array{};
			elseif (attackType == Upper(BLUNTDAMAGE))
				weaponSFX.hit := array{ 0x3AE };
				weaponSFX.miss := array{ 0x0239, 0x023A, 0x023B, 0x539, 0x53A, 0x53B };
				weaponSFX.critical := array{};
			endif
		else
			if (attackType == Upper(SLASHDAMAGE))
				weaponSFX.hit := array{ 0x23C, 0x23D };
				weaponSFX.miss := array{ 0x0239, 0x023A, 0x023B, 0x539, 0x53A, 0x53B };
				weaponSFX.critical := array{};
			elseif (attackType == Upper(PIERCINGDAMAGE))
				weaponSFX.hit := array{ 0x23C, 0x23D };
				weaponSFX.miss := array{ 0x0239, 0x023A, 0x023B, 0x539, 0x53A, 0x53B };
				weaponSFX.critical := array{};
			elseif (attackType == Upper(BLUNTDAMAGE))
				weaponSFX.hit := array{ 0x3AC };
				weaponSFX.miss := array{ 0x0239, 0x023A, 0x023B, 0x539, 0x53A, 0x53B };
				weaponSFX.critical := array{};
			endif
		endif
	endif

	if (isMiss)
		weaponSFX := weaponSFX.miss;
	elseif (isCriticalAttack)
		weaponSFX := weaponSFX.critical;
	else
		weaponSFX := weaponSFX.hit;
	endif

	weaponSFX := weaponSFX.Size() > 1 ? weaponSFX[RandomIntMinMax(1, weaponSFX.Size() + 1)] : weaponSFX[1];

	return CInt(weaponSFX);
endfunction

function GetDefenseFX(defender, isCritical := false)
	var defenseFX := struct{
		hit := array{},
		critical := array{}
	};
	
	if (defender.IsNPC())
		defenseFX.hit := array{ 15, 16 };
		defenseFX.critical := array{ 15, 16 };
	else
		defenseFX.hit := array{ 30 };
		defenseFX.critical := array{ 30 };
	endif

	if (isCritical)
		defenseFX := defenseFX.critical;
	else
		defenseFX := defenseFX.hit;
	endif

	defenseFX := defenseFX.Size() > 1 ? defenseFX[RandomIntMinMax(1, defenseFX.Size() + 1)] : defenseFX[1];

	return CInt(defenseFX);
endfunction

function GetAttackerSFX(attacker, isCritical := false, defenderIsDead := false)
	if (isCritical)
		if (defenderIsDead)
			if (attacker.GetRace() == ANAO)
				return 0;
			elseif (attacker.GetRace() == DROW)
				return 0;
			elseif (attacker.GetRace() == ELFO)
				return 0;
			elseif (attacker.GetRace() == ORC)
				return 0;
			elseif (attacker.GetRace() == HUMANO)
				return 0;
			elseif (attacker.GetRace() == POLSKI)
				return 0;
			endif
		endif
	endif
endfunction

function GetDefenderSFX(defender, isCritical := false, isDead := false)
	if (defender.IsNPC())
		if (isDead)
			return defender.GetNPCTemplateCfg().DeathSound;
		else
			return defender.GetNPCTemplateCfg().DamagedSound;
		endif
	else
		if (isDead)
			if (defender.GetRace() == ANAO)
				return 0;
			elseif (defender.GetRace() == DROW)
				return 0;
			elseif (defender.GetRace() == ELFO)
				return 0;
			elseif (defender.GetRace() == ORC)
				return 0;
			elseif (defender.GetRace() == HUMANO)
				return 0;
			elseif (defender.GetRace() == POLSKI)
				return 0;
			endif
		else
		endif
	endif
endfunction