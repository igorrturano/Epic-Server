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
include ":wrappers:Objects/Model/WeaponWrapper";
include ":wrappers:Objects/Model/MobileWrapper";
include ":blood:blood";

function PlayAttackFX(byRef attackInfo)
	var attackerAnim := GetAttackerFX(attackInfo);
	var defenderAnim := GetDefenderFX(attackInfo);
	PlayWeaponFX(attackInfo);

	attackInfo.GetAttacker().PlayAnimation(attackerAnim);
	if (defenderAnim) attackInfo.GetDefender().PlayAnimation(defenderAnim); endif

	var hpPercent := (attackInfo.GetDefender().GetCurrentHP() / attackInfo.GetDefender().GetMaxHP()) * 100;
	if (hpPercent < 20 && attackInfo.GetDefender() is @MobileWrapper)
		MakeBleed(attackInfo.GetDefender().GetObjRef());
	endif

	PlayElementalHitFX(attackInfo);
endfunction

function PlayAttackSFX(byRef attackInfo)
	var attackerSound := GetAttackerSFX(attackInfo);
	var defenderSound := GetDefenderSFX(attackInfo);
	var weaponSound   := GetWeaponSFX(attackInfo);

	if (attackerSound) attackInfo.GetAttacker().PlaySound(attackerSound); endif
	if (defenderSound) attackInfo.GetDefender().PlaySound(defenderSound); endif
	attackInfo.GetAttacker().PlaySound(weaponSound);
endfunction

function PlayWeaponFX(byRef attackInfo)
	if (attackInfo.GetAttacker().GetProperty("throwItem") ||
		attackInfo.GetWeapon().IsThrowable())
		PlayThrowingFX(attackInfo);
	elseif (Upper(attackInfo.GetWeapon().GetCategory()) == Upper(WEAPON_CATEGORY::BOW) ||
			Upper(attackInfo.GetWeapon().GetCategory()) == Upper(WEAPON_CATEGORY::CROSSBOW) ||
			attackInfo.GetWeapon().isFireWeapon())
		PlayShootingFX(attackInfo);
	endif
endfunction

function PlayParryFX(byRef attackInfo)
	var parryAnim := GetParryFX(attackInfo);

	attackInfo.GetDefender().PlayAnimation(parryAnim);
endfunction

function PlayParrySFX(byRef attackInfo)
	var parrySound := GetParrySFX(attackInfo);

	attackInfo.GetDefender().PlaySound(parrySound);
endfunction

function GetAttackerFX(byRef attackInfo)
	var attackerFX := struct{
		hit := array{},
		critical := array{}
	};
	
	if (attackInfo.GetAttacker().IsNPC() && attackInfo.GetWeapon().GetCategory() == WEAPON_CATEGORY::UNARMED)
		attackerFX.hit := array{4, 5, 6};
		attackerFX.critical := array{4, 5, 6};
	else
		if (attackInfo.GetWeapon().GetCategory() == WEAPON_CATEGORY::UNARMED)
			attackerFX.hit := array{HUMAN_ANIM::ATTACK_1HAND_SLASH, HUMAN_ANIM::ATTACK_1HAND_PIERCING, HUMAN_ANIM::ATTACK_1HAND_BLUNT};
			attackerFX.critical := array{HUMAN_ANIM::ATTACK_PUNCH};
		elseif (attackInfo.GetWeapon().GetCategory() == WEAPON_CATEGORY::BOW ||
				attackInfo.GetWeapon().GetCategory() == WEAPON_CATEGORY::MAGICBOW)
			attackerFX.hit := array{HUMAN_ANIM::ATTACK_BOW};
			attackerFX.critical := array{HUMAN_ANIM::ATTACK_BOW};
		elseif (attackInfo.GetWeapon().GetCategory() == WEAPON_CATEGORY::CROSSBOW)
			attackerFX.hit := array{HUMAN_ANIM::ATTACK_XBOW};
			attackerFX.critical := array{HUMAN_ANIM::ATTACK_XBOW};
		elseif (attackInfo.GetWeapon().isFireWeapon())
			attackerFX.hit := array{HUMAN_ANIM::ATTACK_XBOW};
			attackerFX.critical := array{HUMAN_ANIM::ATTACK_XBOW};
		elseif (attackInfo.GetWeapon().GetCategory() == WEAPON_CATEGORY::THROWING)
			attackerFX.hit := array{HUMAN_ANIM::ATTACK_2HAND_BLUNT};
			attackerFX.critical := array{HUMAN_ANIM::ATTACK_2HAND_BLUNT};
		else
			if (Upper(attackInfo.GetWeapon().GetCategory()) == Upper(WEAPON_CATEGORY::SPIRITUALBLADE))
			// Use absorbed weapon's objtype for animation lookup (stored when spiritual blade is summoned)
				var cfg := ReadConfigFile(":*:itemdesc");
				var weaponRef := attackInfo.GetWeapon().GetObjRef();
				// The spiritual blade's graphic IS the absorbed weapon's objtype (set by ElementalBond)
				var absorbedObjtype := weaponRef.graphic;
				var attribute := cfg[absorbedObjtype].Attribute;
				if (Upper(attribute) == Upper(TWOHANDED))
					if (Upper(attackInfo.GetAttackType()) == Upper(DAMAGE_TYPE::SLASH))
						attackerFX.hit := array{HUMAN_ANIM::ATTACK_2HAND_SLASH};
						attackerFX.critical := array{HUMAN_ANIM::ATTACK_2HAND_SLASH};
					elseif (Upper(attackInfo.GetAttackType()) == Upper(DAMAGE_TYPE::PIERCING))
						attackerFX.hit := array{HUMAN_ANIM::ATTACK_2HAND_PIERCING};
						attackerFX.critical := array{HUMAN_ANIM::ATTACK_2HAND_PIERCING};
					elseif (Upper(attackInfo.GetAttackType()) == Upper(DAMAGE_TYPE::BLUNT))
						attackerFX.hit := array{HUMAN_ANIM::ATTACK_2HAND_BLUNT};
						attackerFX.critical := array{HUMAN_ANIM::ATTACK_2HAND_BLUNT};
					endif
				elseif (Upper(attribute) == Upper(ONEHANDED) || Upper(attribute) == Upper(DUALWIELD))
					if (Upper(attackInfo.GetAttackType()) == Upper(DAMAGE_TYPE::SLASH))
						attackerFX.hit := array{HUMAN_ANIM::ATTACK_1HAND_SLASH};
						attackerFX.critical := array{HUMAN_ANIM::ATTACK_1HAND_SLASH};
					elseif (Upper(attackInfo.GetAttackType()) == Upper(DAMAGE_TYPE::PIERCING))
						attackerFX.hit := array{HUMAN_ANIM::ATTACK_1HAND_PIERCING};
						attackerFX.critical := array{HUMAN_ANIM::ATTACK_1HAND_PIERCING};
					elseif (Upper(attackInfo.GetAttackType()) == Upper(DAMAGE_TYPE::BLUNT))
						attackerFX.hit := array{HUMAN_ANIM::ATTACK_1HAND_BLUNT};
						attackerFX.critical := array{HUMAN_ANIM::ATTACK_1HAND_BLUNT};
					endif
				endif
			elseif (Upper(attackInfo.GetWeapon().GetAttribute()) == Upper(TWOHANDED)
					|| Upper(attackInfo.GetWeapon().GetAttribute()) == Upper(WRESTLING))
				if (Upper(attackInfo.GetAttackType()) == Upper(DAMAGE_TYPE::SLASH))
					attackerFX.hit := array{HUMAN_ANIM::ATTACK_2HAND_SLASH};
					attackerFX.critical := array{HUMAN_ANIM::ATTACK_2HAND_SLASH};
				elseif (Upper(attackInfo.GetAttackType()) == Upper(DAMAGE_TYPE::PIERCING))
					attackerFX.hit := array{HUMAN_ANIM::ATTACK_2HAND_PIERCING};
					attackerFX.critical := array{HUMAN_ANIM::ATTACK_2HAND_PIERCING};
				elseif (Upper(attackInfo.GetAttackType()) == Upper(DAMAGE_TYPE::BLUNT))
					attackerFX.hit := array{HUMAN_ANIM::ATTACK_2HAND_BLUNT};
					attackerFX.critical := array{HUMAN_ANIM::ATTACK_2HAND_BLUNT};
				endif
			elseif (Upper(attackInfo.GetWeapon().GetAttribute()) == Upper(ONEHANDED) ||
					Upper(attackInfo.GetWeapon().GetAttribute()) == Upper(DUALWIELD))
				if (Upper(attackInfo.GetAttackType()) == Upper(DAMAGE_TYPE::SLASH))
					attackerFX.hit := array{HUMAN_ANIM::ATTACK_1HAND_SLASH};
					attackerFX.critical := array{HUMAN_ANIM::ATTACK_1HAND_SLASH};
				elseif (Upper(attackInfo.GetAttackType()) == Upper(DAMAGE_TYPE::PIERCING))
					attackerFX.hit := array{HUMAN_ANIM::ATTACK_1HAND_PIERCING};
					attackerFX.critical := array{HUMAN_ANIM::ATTACK_1HAND_PIERCING};
				elseif (Upper(attackInfo.GetAttackType()) == Upper(DAMAGE_TYPE::BLUNT))
					attackerFX.hit := array{HUMAN_ANIM::ATTACK_1HAND_BLUNT};
					attackerFX.critical := array{HUMAN_ANIM::ATTACK_1HAND_BLUNT};
				endif
			endif
		endif
	endif

	if (attackInfo.GetDamage().IsCritical())
		attackerFX := attackerFX.critical;
	else
		attackerFX := attackerFX.hit;
	endif

	attackerFX := attackerFX.Size() > 1 ? attackerFX[RandomInt(attackerFX.Size()) + 1] : attackerFX[1];

	return CInt(attackerFX);
endfunction

function GetDefenderFX(byRef attackInfo)
	var defenderFX := struct{
		hit := array{},
		miss := array{}
	};

	if (attackInfo.GetDefender().IsNPC())
		defenderFX.hit := array{10};
		defenderFX.miss := array{15, 16};
	else
		defenderFX.hit := array{20};
		defenderFX.miss := array{30};
	endif

	if (attackInfo.Succeeded())
		defenderFX := defenderFX.hit;
	else
		defenderFX := defenderFX.miss;
	endif

	defenderFX := defenderFX.Size() > 1 ? defenderFX[RandomIntMinMax(1, defenderFX.Size() + 1)] : defenderFX[1];

	return CInt(defenderFX);
endfunction

function GetParryFX(byRef attackInfo)
	var parryFX := struct{
		hit := array{},
		critical := array{}
	};
	
	if (attackInfo.GetDefender().IsNPC())
		parryFX.hit := array{15, 16};
		parryFX.critical := array{15, 16};
	else
		parryFX.hit := array{30};
		parryFX.critical := array{30};
	endif

	if (attackInfo.GetDamage().IsCritical())
		parryFX := parryFX.critical;
	else
		parryFX := parryFX.hit;
	endif

	parryFX := parryFX.Size() > 1 ? parryFX[RandomIntMinMax(1, parryFX.Size() + 1)] : parryFX[1];

	return CInt(parryFX);
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
			elseif (attackInfo.GetAttacker().GetRace() == PEQUILHO)
				return 0;
			endif
		endif
	endif
endfunction

function GetDefenderSFX(byRef attackInfo)
	var defenderSFX := struct {
		miss := array{},
		hit := array{},
		criticalHit := array{},
		death := array{},
		criticalDeath := array{}
	};

	if (attackInfo.Succeeded())
		if (attackInfo.GetDefender().IsNPC())
			defenderSFX.death := array{attackInfo.GetDefender().GetTemplate().DeathSound};
			defenderSFX.hit := array{attackInfo.GetDefender().GetTemplate().DamagedSound};
		else
			if (attackInfo.GetDefender().GetGender == MOBILE_GENDER::MALE)
				defenderSFX.hit := array{0x154, 0x155, 0x156, 0x157, 0x158, 0x159};
				defenderSFX.death := array{0x15A, 0x15B, 0x15C, 0x15D};
			elseif (attackInfo.GetDefender().GetGender == MOBILE_GENDER::FEMALE)
				defenderSFX.hit := array{0x14B, 0x14C, 0x14D, 0x14E, 0x14F};
				defenderSFX.death := array{0x150, 0x151, 0x152, 0x153};
			endif

			if (attackInfo.GetDefender().GetRace() == ANAO)
			elseif (attackInfo.GetDefender().GetRace() == DROW)
			elseif (attackInfo.GetDefender().GetRace() == ELFO)
			elseif (attackInfo.GetDefender().GetRace() == ORC)
			elseif (attackInfo.GetDefender().GetRace() == HUMANO)
			elseif (attackInfo.GetDefender().GetRace() == PEQUILHO)
			endif
		endif
	endif

	if (!attackInfo.Succeeded())
		defenderSFX := defenderSFX.miss;
	elseif (attackInfo.GetDefender().IsDead() && attackInfo.GetDamage().IsCritical())
		if (attackInfo.GetDamage().IsCritical())
			defenderSFX := defenderSFX.critical;
		endif
	else
		defenderSFX := defenderSFX.hit;
	endif

	defenderSFX := defenderSFX.Size() > 1 ? defenderSFX[RandomIntMinMax(1, defenderSFX.Size() + 1)] : defenderSFX[1];

	return 0;
endfunction

function GetWeaponSFX(byRef attackInfo)
	var weaponSFX := struct{
		hit := array{},
		miss := array{}
	};

	if (attackInfo.GetAttacker().IsNPC() && attackInfo.GetWeapon().GetCategory() == WEAPON_CATEGORY::UNARMED)
		var npcTemplate := attackInfo.GetAttacker().GetTemplate();
		weaponSFX.hit := array{npcTemplate.AttackHitSound};
		weaponSFX.miss := array{npcTemplate.AttackMissSound};
	elseif (attackInfo.GetWeapon().GetCategory() == WEAPON_CATEGORY::UNARMED)
		weaponSFX.hit := array{0x136, 0x138, 0x13C, 0x13D, 0x13E, 0x142, 0x145, 0x146, 0x149, 0x14A, 0x14B};
		weaponSFX.miss := array{0x0239, 0x023A, 0x023B, 0x539, 0x53A, 0x53B};
	elseif (attackInfo.GetWeapon().GetCategory() == WEAPON_CATEGORY::MAGICBOW)
		if (attackInfo.GetWeapon().GetObjRef().GetProp("fire_arrow"))
			weaponSFX.hit := array{0x5D0};
		elseif (attackInfo.GetWeapon().GetObjRef().GetProp("energy_arrow"))
			weaponSFX.hit := array{0x029};
		elseif (attackInfo.GetWeapon().GetObjRef().GetProp("cold_arrow"))
			weaponSFX.hit := array{0x22B};
		else
			weaponSFX.hit := array{0x235};
		endif
		weaponSFX.miss := array{0x526};
	elseif (attackInfo.GetWeapon().GetCategory() == WEAPON_CATEGORY::BOW)
		weaponSFX.hit := array{0x235, 0x4C9, 0x4CA};
		weaponSFX.miss := array{0x0239, 0x023A, 0x023B, 0x539, 0x53A, 0x53B};
	elseif (attackInfo.GetWeapon().GetCategory() == WEAPON_CATEGORY::CROSSBOW)
		weaponSFX.hit := array{0x235, 0x4C9, 0x4CA};
		weaponSFX.miss := array{0x0239, 0x023A, 0x023B, 0x539, 0x53A, 0x53B};
	elseif (attackInfo.GetWeapon().GetCategory() == WEAPON_CATEGORY::PISTOL)
		weaponSFX.hit := array{0x11C};
		weaponSFX.miss := array{0x11C};
	elseif (attackInfo.GetWeapon().GetCategory() == WEAPON_CATEGORY::HANDCANNON)
		weaponSFX.hit := array{0x308};
		weaponSFX.miss := array{0x308};
	elseif (attackInfo.GetWeapon().GetCategory() == WEAPON_CATEGORY::CARABINE)
		weaponSFX.hit := array{0x309};
		weaponSFX.miss := array{0x309};
	elseif (attackInfo.GetWeapon().GetCategory() == WEAPON_CATEGORY::MUSKET)
		weaponSFX.hit := array{0x11E};
		weaponSFX.miss := array{0x11E};
	elseif (attackInfo.GetWeapon().GetCategory() == WEAPON_CATEGORY::WHIP)
		weaponSFX.hit := array{0x67F};
		weaponSFX.miss := array{0x67F};
	elseif (attackInfo.GetWeapon().GetCategory() == WEAPON_CATEGORY::THROWING)
		weaponSFX.hit := array{0x531};
		weaponSFX.miss := array{0x532};
	else
		if (Upper(attackInfo.GetWeapon().GetAttribute()) == Upper(TWOHANDED))
			if (Upper(attackInfo.GetAttackType()) == Upper(DAMAGE_TYPE::SLASH))
				weaponSFX.hit := array{0x237, 0x238};
				weaponSFX.miss := array{0x0239, 0x023A, 0x023B, 0x539, 0x53A, 0x53B};
			elseif (Upper(attackInfo.GetAttackType()) == Upper(DAMAGE_TYPE::PIERCING))
				weaponSFX.hit := array{0x23D, 0x23D};
				weaponSFX.miss := array{0x0239, 0x023A, 0x023B, 0x539, 0x53A, 0x53B};
			elseif (Upper(attackInfo.GetAttackType()) == Upper(DAMAGE_TYPE::BLUNT))
				weaponSFX.hit := array{0x3AE};
				weaponSFX.miss := array{0x0239, 0x023A, 0x023B, 0x539, 0x53A, 0x53B};
			endif
		else
			if (Upper(attackInfo.GetAttackType()) == Upper(DAMAGE_TYPE::SLASH))
				weaponSFX.hit := array{0x23C, 0x23D};
				weaponSFX.miss := array{0x0239, 0x023A, 0x023B, 0x539, 0x53A, 0x53B};
			elseif (Upper(attackInfo.GetAttackType()) == Upper(DAMAGE_TYPE::PIERCING))
				weaponSFX.hit := array{0x23C, 0x23D};
				weaponSFX.miss := array{0x0239, 0x023A, 0x023B, 0x539, 0x53A, 0x53B};
			elseif (Upper(attackInfo.GetAttackType()) == Upper(DAMAGE_TYPE::BLUNT))
				weaponSFX.hit := array{0x3AC};
				weaponSFX.miss := array{0x0239, 0x023A, 0x023B, 0x539, 0x53A, 0x53B};
			endif
		endif
	endif

	if (!attackInfo.Succeeded())
		weaponSFX := weaponSFX.miss;
	else
		weaponSFX := weaponSFX.hit;
	endif

	weaponSFX := weaponSFX.Size() > 1 ? weaponSFX[RandomIntMinMax(1, weaponSFX.Size() + 1)] : weaponSFX[1];

	return CInt(weaponSFX);
endfunction

function GetParrySFX(byRef attackInfo)
	var parrySFX := 0;

	if (attackInfo.GetDefender().IsNPC() && !attackInfo.GetDefender().GetEquipBySlot(SHIELD).GetCategory())
		// TODO: Adicionar sons de bloqueio para NPCs sem escudo
	else
		// TODO: Adicionar sons de bloqueio para NPCs/Players com escudo
			//TODO: Diferenciar sons de bloqueio por tipo de escudo
		
		parrySFX := array{0x136, 0x138, 0x139, 0x140, 0x143, 0x536, 0x537};
	endif

	parrySFX := parrySFX.Size() > 1 ? parrySFX[RandomIntMinMax(1, parrySFX.Size() + 1)] : parrySFX[1];

	return parrySFX;
endfunction

/**************************/
/* Weapon Custom FX & SFX */
/**************************/
function PlayThrowingFX(byRef attackInfo)
	if (attackInfo.GetAttacker().IsNPC())
		var effect := attackInfo.GetAttacker().GetObjRef().GetProp("throwItem");
		attackInfo.GetAttacker().PlayEffectTo(attackInfo.GetDefender(), effect);
	else
		var effect := attackInfo.GetWeapon().GetObjType();
		attackInfo.GetAttacker().PlayEffectTo(attackInfo.GetDefender(), effect);
		attackInfo.GetWeapon().MoveToLocation(attackInfo.GetDefender().GetPosition());
	endif
endfunction

function PlayShootingFX(byRef attackInfo)
	attackInfo.GetAttacker().PlayEffectTo(
		attackInfo.GetDefender(),
		attackInfo.GetWeapon().GetAmmo().GetAnim()
	);
endfunction

function PlayElementalHitFX(byRef attackInfo)
	if (attackInfo.GetWeapon().GetCategory() != WEAPON_CATEGORY::MAGICBOW)
		return;
	endif

	if (!attackInfo.Succeeded())
		return;
	endif

	var defender := attackInfo.GetDefender().GetObjRef();
	var attacker := attackInfo.GetAttacker().GetObjRef();
	var weapon := attackInfo.GetWeapon().GetObjRef();

	var element := "";
	if (weapon.GetProp("fire_arrow"))
		element := "fire";
	elseif (weapon.GetProp("cold_arrow"))
		element := "cold";
	elseif (weapon.GetProp("energy_arrow"))
		element := "energy";
	endif

	// Third consecutive hit FX (Afinidade Elemental) — reads flag set by ElementalAffinity.inc
	var triggered := attacker.GetProp("#elemental_affinity_triggered");
	if (triggered)
		attacker.EraseProp("#elemental_affinity_triggered");
		if (triggered == "fire_arrow")
			PlayObjectCenteredEffect(defender, FX_FIRE_PROTECT, 0, 30);
		elseif (triggered == "cold_arrow")
			PlayObjectCenteredEffect(defender, FX_SPEED_BEAM_MAGIC, 0, 30);
		elseif (triggered == "energy_arrow")
			PlayObjectCenteredEffect(defender, FX_RAIO_PROTECTOR, 0, 30);
			PlayObjectCenteredEffect(defender, FX_RAIO_PEQUENO, 0, 30);
		endif
	endif
endfunction
