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
	var attackerAnim := GetAttackerFX(attackInfo);
	var defenderAnim := GetDefenderFX(attackInfo);

	attackInfo.GetAttacker().PlayAnimation(attackerAnim);
	if (defenderAnim) attackInfo.GetDefender().PlayAnimation(defenderAnim); endif
endfunction

function PlayAttackSFX(byRef attackInfo)
	var weaponSound;
	var attackerSound;
	var defenderSound;

	if (attackInfo.GetAttacker().IsNPC() && !attackInfo.GetAttacker().GetEquipBySlot(WEAPON))
		var npcTemplate := attackInfo.GetAttacker().GetNPCTemplate();
		weaponSound := attackInfo.Succeeded() ? npcTemplate.AttackSound : npcTemplate.AttackMissSound;
	else
		attackerSound 	:= GetAttackerSFX(attackInfo);
		defenderSound 	:= GetDefenderSFX(attackInfo);
		weaponSound 	:= GetWeaponSFX(attackInfo);
	endif

	attackInfo.GetAttacker().PlaySound(weaponSound);
	if (attackerSound) attackInfo.GetAttacker().PlaySound(attackerSound); endif
	if (defenderSound) attackInfo.GetDefender().PlaySound(defenderSound); endif
endfunction

function PlayDefenseFX(byRef attackInfo)
	var defenseAnim := GetDefenseFX(attackInfo);

	attackInfo.GetDefender().PlayAnimation(defenseAnim);
endfunction

function GetAttackerFX(byRef attackInfo)
	var attackerFX := struct{
		hit := array{},
		critical := array{}
	};
	
	if (attackInfo.GetAttacker().IsNPC() && !attackInfo.GetAttacker().GetEquipBySlot(WEAPON))
		attackerFX.hit := array{ 0x0c };
		attackerFX.critical := array{ 0x0c };
	else
		if (attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetCategory() == UNARMED)
			attackerFX.hit := array{ 0x9, 0xA, 0xB };
			attackerFX.critical := array{ 0x1F };
		elseif (attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetCategory() == BOW)
			attackerFX.hit := array{ 0x12 };
			attackerFX.critical := array{ 0x12 };
		elseif (attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetCategory() == CROSSBOW)
			attackerFX.hit := array{ 0x13 };
			attackerFX.critical := array{ 0x13 };
		else
			if (Upper(attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetAttribute()) == Upper(TWOHANDED))
				if (Upper(attackInfo.GetAttackType()) == Upper(SLASHDAMAGE))
					attackerFX.hit := array{ 0xC };
					attackerFX.critical := array{ 0xC };
				elseif (Upper(attackInfo.GetAttackType()) == Upper(PIERCINGDAMAGE))
					attackerFX.hit := array{ 0xE };
					attackerFX.critical := array{ 0xE };
				elseif (Upper(attackInfo.GetAttackType()) == Upper(BLUNTDAMAGE))
					attackerFX.hit := array{ 0xB };
					attackerFX.critical := array{ 0xB };
				endif
			else
				if (Upper(attackInfo.GetAttackType()) == Upper(SLASHDAMAGE))
					attackerFX.hit := array{ 0xD };
					attackerFX.critical := array{ 0xD };
				elseif (Upper(attackInfo.GetAttackType()) == Upper(PIERCINGDAMAGE))
					attackerFX.hit := array{ 0xA };
					attackerFX.critical := array{ 0xA };
				elseif (Upper(attackInfo.GetAttackType()) == Upper(BLUNTDAMAGE))
					attackerFX.hit := array{ 0xB };
					attackerFX.critical := array{ 0xB };
				endif
			endif
		endif
	endif

	if (attackInfo.GetDamage().IsCritical())
		attackerFX := attackerFX.critical;
	else
		attackerFX := attackerFX.hit;
	endif

	attackerFX := attackerFX.Size() > 1 ? attackerFX[RandomIntMinMax(1, attackerFX.Size() + 1)] : attackerFX[1];

	return CInt(attackerFX);
endfunction

function GetDefenderFX(byRef attackInfo)
	var defenderFX := struct{
		hit := array{},
		miss := array{}
	};

	if (attackInfo.GetDefender().IsNPC())
		defenderFX.hit := array{ 0x0C };
		defenderFX.miss := array{};
	else
		defenderFX.hit := array{ 20 };
		defenderFX.miss := array{};
	endif

	if (attackInfo.Succeeded())
		defenderFX := defenderFX.hit;
	else
		defenderFX := defenderFX.miss;
	endif

	defenderFX := defenderFX.Size() > 1 ? defenderFX[RandomIntMinMax(1, defenderFX.Size() + 1)] : defenderFX[1];

	return CInt(defenderFX);
endfunction

function GetWeaponSFX(byRef attackInfo)
	var weaponSFX := struct{
		hit := array{},
		miss := array{},
		critical := array{}
	};

	if (attackInfo.GetAttacker().GetEquipBySlot(WEAPON) == error)
		weaponSFX.hit := array{ 0x0239, 0x023A, 0x023B, 0x539, 0x53A, 0x53B };
		weaponSFX.miss := array{ 0x0239, 0x023A, 0x023B, 0x539, 0x53A, 0x53B };
		weaponSFX.critical := array{ 0x0239, 0x023A, 0x023B, 0x539, 0x53A, 0x53B };
	elseif (attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetCategory() == UNARMED)
		weaponSFX.hit := array{ 0x136, 0x138, 0x13C, 0x13D, 0x13E, 0x142, 0x145, 0x146, 0x149, 0x14A, 0x14B };
		weaponSFX.miss := array{ 0x0239, 0x023A, 0x023B, 0x539, 0x53A, 0x53B };
		weaponSFX.critical := array{};
	elseif (attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetCategory() == BOW)
		weaponSFX.hit := array{ 0x235, 0x4C9, 0x4CA };
		weaponSFX.miss := array{ 0x0239, 0x023A, 0x023B, 0x539, 0x53A, 0x53B };
		weaponSFX.critical := array{};
	elseif (attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetCategory() == CROSSBOW)
		weaponSFX.hit := array{ 0x235, 0x4C9, 0x4CA };
		weaponSFX.miss := array{ 0x0239, 0x023A, 0x023B, 0x539, 0x53A, 0x53B };
		weaponSFX.critical := array{};
	else
		if (Upper(attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetAttribute()) == Upper(TWOHANDED))
			if (Upper(attackInfo.GetAttackType()) == Upper(SLASHDAMAGE))
				weaponSFX.hit := array{ 0x237, 0x238 };
				weaponSFX.miss := array{ 0x0239, 0x023A, 0x023B, 0x539, 0x53A, 0x53B };
				weaponSFX.critical := array{};
			elseif (Upper(attackInfo.GetAttackType()) == Upper(PIERCINGDAMAGE))
				weaponSFX.hit := array{ 0x23D, 0x23D };
				weaponSFX.miss := array{ 0x0239, 0x023A, 0x023B, 0x539, 0x53A, 0x53B };
				weaponSFX.critical := array{};
			elseif (Upper(attackInfo.GetAttackType()) == Upper(BLUNTDAMAGE))
				weaponSFX.hit := array{ 0x3AE };
				weaponSFX.miss := array{ 0x0239, 0x023A, 0x023B, 0x539, 0x53A, 0x53B };
				weaponSFX.critical := array{};
			endif
		else
			if (Upper(attackInfo.GetAttackType()) == Upper(SLASHDAMAGE))
				weaponSFX.hit := array{ 0x23C, 0x23D };
				weaponSFX.miss := array{ 0x0239, 0x023A, 0x023B, 0x539, 0x53A, 0x53B };
				weaponSFX.critical := array{};
			elseif (Upper(attackInfo.GetAttackType()) == Upper(PIERCINGDAMAGE))
				weaponSFX.hit := array{ 0x23C, 0x23D };
				weaponSFX.miss := array{ 0x0239, 0x023A, 0x023B, 0x539, 0x53A, 0x53B };
				weaponSFX.critical := array{};
			elseif (Upper(attackInfo.GetAttackType()) == Upper(BLUNTDAMAGE))
				weaponSFX.hit := array{ 0x3AC };
				weaponSFX.miss := array{ 0x0239, 0x023A, 0x023B, 0x539, 0x53A, 0x53B };
				weaponSFX.critical := array{};
			endif
		endif
	endif

	if (!attackInfo.Succeeded())
		weaponSFX := weaponSFX.miss;
	elseif (attackInfo.GetDamage().IsCritical())
		weaponSFX := weaponSFX.critical;
	else
		weaponSFX := weaponSFX.hit;
	endif

	weaponSFX := weaponSFX.Size() > 1 ? weaponSFX[RandomIntMinMax(1, weaponSFX.Size() + 1)] : weaponSFX[1];

	return CInt(weaponSFX);
endfunction

function GetDefenseFX(byRef attackInfo)
	var defenseFX := struct{
		hit := array{},
		critical := array{}
	};
	
	if (attackInfo.GetDefender().IsNPC())
		defenseFX.hit := array{ 15, 16 };
		defenseFX.critical := array{ 15, 16 };
	else
		defenseFX.hit := array{ 30 };
		defenseFX.critical := array{ 30 };
	endif

	if (attackInfo.GetDamage().IsCritical())
		defenseFX := defenseFX.critical;
	else
		defenseFX := defenseFX.hit;
	endif

	defenseFX := defenseFX.Size() > 1 ? defenseFX[RandomIntMinMax(1, defenseFX.Size() + 1)] : defenseFX[1];

	return CInt(defenseFX);
endfunction

function GetAttackerSFX(byRef attackInfo)
	if (attackInfo.GetDamage().IsCritical())
		if (attackInfo.GetDefender().IsDead())
			if (attackInfo.GetAttacker().GetRace() == ANAO)
				return 0;
			elseif (attackInfo.GetAttacker().GetRace() == DROW)
				return 0;
			elseif (attackInfo.GetAttacker().GetRace() == ELFO)
				return 0;
			elseif (attackInfo.GetAttacker().GetRace() == ORC)
				return 0;
			elseif (attackInfo.GetAttacker().GetRace() == HUMANO)
				return 0;
			elseif (attackInfo.GetAttacker().GetRace() == POLSKI)
				return 0;
			endif
		endif
	endif
endfunction

function GetDefenderSFX(byRef attackInfo)
	if (attackInfo.GetDefender().IsNPC())
		if (attackInfo.GetDefender().IsDead())
			return attackInfo.GetDefender().GetNPCTemplate().DeathSound;
		else
			return attackInfo.GetDefender().GetNPCTemplate().DamagedSound;
		endif
	else
		if (attackInfo.GetDefender().IsDead())
			if (attackInfo.GetDefender().GetRace() == ANAO)
				return 0;
			elseif (attackInfo.GetDefender().GetRace() == DROW)
				return 0;
			elseif (attackInfo.GetDefender().GetRace() == ELFO)
				return 0;
			elseif (attackInfo.GetDefender().GetRace() == ORC)
				return 0;
			elseif (attackInfo.GetDefender().GetRace() == HUMANO)
				return 0;
			elseif (attackInfo.GetDefender().GetRace() == POLSKI)
				return 0;
			endif
		else
			// TODO: Add hurt sounds
		endif
	endif
endfunction