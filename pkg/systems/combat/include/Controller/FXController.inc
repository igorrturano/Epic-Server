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

enum class WEAPON_FX
	SOUND := "SOUND",
	ANIM := "ANIMATION"
endenum

function PlayAttackAnimation(byRef attackInfo)
	var attackAnim;

	if (attackInfo.GetAttacker().IsNPC() && !attackInfo.GetAttacker().GetEquipBySlot(WEAPON))
		attackAnim := 0x0c;
	else
		attackAnim := getWeaponFX(WEAPON_FX::ANIM, attackInfo.GetAttacker().GetEquipBySlot(WEAPON), attackInfo.GetDamage().IsCritical());
	endif

	attackInfo.GetAttacker().PlayAnimation(attackAnim);
endfunction

function PlayAttackSound(byRef attackInfo)
	var weaponSound;
	var attackerSound;
	var defenderSound; // TODO: Buscar o som de gemido/grito do defensor, ao ser atingido

	if (attackInfo.GetAttacker().GetEquipBySlot(WEAPON))
		if (attackInfo.Succeded())
			if (attackInfo.GetDamage().IsCritical())
				weaponSound := GetWeaponFX(WEAPON_FX::SOUND, attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetObjRef(), true); // Passa true para indicar que é CRÍTICO
				if (attackInfo.GetDefender().IsDead())
					if (attackInfo.GetAttacker().GetRace() == ANAO)
						attackerSound := 0; // Som de grito Anão
					elseif (attackInfo.GetAttacker().GetRace() == DROW)
						attackerSound := 0; // Som de grito Drow
					elseif (attackInfo.GetAttacker().GetRace() == ELFO)
						attackerSound := 0; // Som de grito Elfo
					elseif (attackInfo.GetAttacker().GetRace() == ORC)
						attackerSound := 0; // Som de grito Orc
					elseif (attackInfo.GetAttacker().GetRace() == HUMANO)
						attackerSound := 0; // Som de grito Humano
					elseif (attackInfo.GetAttacker().GetRace() == POLSKI)
						attackerSound := 0; // Som de grito Polski
					endif
				endif
			else
				weaponSound := GetWeaponFX(WEAPON_FX::SOUND, attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetObjRef());
			endif
		else // Se errou o golpe
			weaponSound := GetWeaponFX(WEAPON_FX::SOUND, false); // Passa falso para indicar que é MISS
		endif
	else
		if (attackInfo.Succeded())
			if (attackInfo.GetAttacker().IsNPC())
				weaponSound := attackInfo.GetAttacker().GetNPCTemplateCfg().AttackSound;
			else
				weaponSound := GetWeaponFX(WEAPON_FX::SOUND, attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetObjRef());
			endif
		else
			weaponSound := GetWeaponFX(WEAPON_FX::SOUND, false);
		endif
	endif

	if (attackInfo.GetDefender().IsNPC())
		defenderSound := attackInfo.GetDefender().GetNPCTemplateCfg().DamagedSound;
	else
		if (attackInfo.GetDefender().GetGender() == MOBILE_GENDER::MALE)
			defenderSound := array{341, 342, 343, 345, 346};
		elseif (attackInfo.GetDefender().GetGender() == MOBILE_GENDER::FEMALE)
			defenderSound := array{332, 333, 334, 335, 336};
		endif
	endif

	weaponSound := RandomIntMinMax(1, weaponSound.Size() + 1);
	attackInfo.GetAttacker().PlaySound(weaponSound);
	
	if (attackerSound)
		attackerSound := RandomIntMinMax(1, attackerSound.Size() + 1);
		attackInfo.GetAttacker().PlaySound(attackerSound);
	endif
	
	if (defenderSound)
		defenderSound := RandomIntMinMax(1, defenderSound.Size() + 1);
		attackInfo.GetDefender().PlaySound(defenderSound);
	endif
endfunction

function GetWeaponFX(FX, weapon, isCriticalAttack := false)
	var attackFX;
	weapon := weapon ? WeaponWrapper(weapon) : error{ errortext := "No weapon provided!" };
	var attackType := weapon.GetAttackType().Upper();

	if (FX != WEAPON_FX::ANIM && FX != WEAPON_FX::SOUND)
		return error{ errortext := "Invalid FX type!" };
	endif

	if (weapon == error)
		// TODO: Pensar se separa o som de MISS de acordo com a arma ou deixa genérico mesmo
		attackFX := array{0x0239, 0x023A, 0x023B, 0x539, 0x53A, 0x53B};
	elseif (weapon.GetCategory() == UNARMED)
		if (FX == WEAPON_FX::ANIM)
			attackFX := !isCriticalAttack ? array{0xA, 0x1F} : array{}; // Ajeitar essa animação do 0xA, tá diferente do Fiddler e não estou achando o soco
		elseif (FX == WEAPON_FX::SOUND)
			attackFX := !isCriticalAttack ? array{0x136, 0x138, 0x13C, 0x13D, 0x13E, 0x142, 0x145, 0x146, 0x149, 0x14A, 0x14B} : array{};
		endif
	elseif (weapon.GetCategory() == BOW)
		if (FX == WEAPON_FX::ANIM)
			attackFX := !isCriticalAttack ? array{0x12} : array{};
		else
			attackFX := !isCriticalAttack ? array{0x235, 0x4C9, 0x4CA} : array{};
		endif
	elseif (weapon.GetCategory() == CROSSBOW)
		if (FX == WEAPON_FX::ANIM)
			attackFX := !isCriticalAttack ? array{0x13} : array{};
		else
			attackFX := !isCriticalAttack ? array{0x235, 0x4C9, 0x4CA} : array{};
		endif
	else
		if (Upper(weapon.GetAttribute()) == Upper(TWOHANDED))
			if (attackType == Upper(SLASHDAMAGE))
				if (FX == WEAPON_FX::ANIM)
					attackFX := !isCriticalAttack ? array{0xC} : array{};
				else
					attackFX := !isCriticalAttack ? array{0x237, 0x238} : array{};
				endif
			elseif (attackType == Upper(PIERCINGDAMAGE))
				if (FX == WEAPON_FX::ANIM)
					attackFX := !isCriticalAttack ? array{0xE} : array{};
				else
					attackFX := !isCriticalAttack ? array{0x23D, 0x23D} : array{};
				endif
			elseif (attackType == Upper(BLUNTDAMAGE))
				if (FX == WEAPON_FX::ANIM)
					attackFX := !isCriticalAttack ? array{0xB} : array{};
				else
					attackFX := !isCriticalAttack ? array{0x3AE} : array{};
				endif
			endif
		else
			if (attackType == Upper(SLASHDAMAGE))
				if (FX == WEAPON_FX::ANIM)
					attackFX := !isCriticalAttack ? array{0xD} : array{};
				else
					attackFX := !isCriticalAttack ? array{0x23C, 0x23D} : array{};
				endif
			elseif (attackType == Upper(PIERCINGDAMAGE))
				if (FX == WEAPON_FX::ANIM)
					attackFX := !isCriticalAttack ? array{0xA} : array{};
				else
					attackFX := !isCriticalAttack ? array{0x23C, 0x23D} : array{};
				endif
			elseif (attackType == Upper(BLUNTDAMAGE))
				if (FX == WEAPON_FX::ANIM)
					attackFX := !isCriticalAttack ? array{0xB} : array{};
				else
					attackFX := !isCriticalAttack ? array{0x3AC} : array{};
				endif
			endif
		endif
	endif

	return attackFX;
endfunction

function _GetAnim(fxs, isCriticalAttack := 0)
	return !isCriticalAttack ? fxs.ANIM[RandomInt(fxs.ANIM.Size()) + 1] : fxs.ANIM_CRITICAL[RandomInt(fxs.ANIM_CRITICAL.Size()) + 1];
endfunction

function _GetSound(fxs, isCriticalAttack := 0)
	return !isCriticalAttack ? fxs.SFX[RandomInt(fxs.SFX.Size()) + 1] : fxs.SFX_CRITICAL[RandomInt(fxs.SFX_CRITICAL.Size()) + 1];
endfunction
