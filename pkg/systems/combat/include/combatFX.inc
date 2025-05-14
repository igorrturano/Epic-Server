// use uo;
// use util;
// use cfgfile;

include "include/client";
include "include/epicConstants";
include ":attributes:attributeConstants";
// include ":equipsys:tags";
include ":equipsys:utils";

enum WEAPONFX
	WEAPON_FX_SOUND, WEAPON_FX_ANIM
endenum

var MISS_FX := struct {
	SFX := array{0x0239, 0x023A, 0x023B, 0x539, 0x53A, 0x53B}
};
var UNARMED_FX := struct {
	ANIM := array{0x9, 0xA},
	SFX := array{0x136, 0x138, 0x13C, 0x13D, 0x13E, 0x142, 0x145, 0x146, 0x149, 0x14A, 0x14B},
	ANIM_CRITICAL := array{},
	SFX_CRITICAL := array{}
};
var BOW_FX := struct {
	ANIM := array{0x12},
	SFX := array{0x235, 0x4C9, 0x4CA},
	ANIM_CRITICAL := array{},
	SFX_CRITICAL := array{}
};
var CROSSBOW_FX := struct {
	ANIM := array{0x13},
	SFX := array{0x235, 0x4C9, 0x4CA},
	ANIM_CRITICAL := array{},
	SFX_CRITICAL := array{}
};
var SLASH_1H_FX := struct {
	ANIM := array{0xD},
	SFX := array{0x23C,0x23D},
	ANIM_CRITICAL := array{},
	SFX_CRITICAL := array{}
};
var SLASH_2H_FX := struct {
	ANIM := array{0xC},
	SFX := array{0x237,0x238},
	ANIM_CRITICAL := array{},
	SFX_CRITICAL := array{}
};
var PIERCING_1H_FX := struct {
	ANIM := array{0xA},
	SFX := array{0x23C,0x23D},
	ANIM_CRITICAL := array{},
	SFX_CRITICAL := array{}
};
var PIERCING_2H_FX := struct {
	ANIM := array{0xE},
	SFX := array{0x23D, 0x23D},
	ANIM_CRITICAL := array{},
	SFX_CRITICAL := array{}
};
var BLUNT_1H_FX := struct {
	ANIM := array{0xB},
	SFX := array{0x3AC},
	ANIM_CRITICAL := array{},
	SFX_CRITICAL := array{}
};
var BLUNT_2H_FX := struct {
	ANIM := array{0xB},
	SFX := array{0x3AE},
	ANIM_CRITICAL := array{},
	SFX_CRITICAL := array{}
};

function PlayAttackAnimation(who)
	var attackAnim;

	if (who.weapon)
		attackAnim := getWeaponFX(WEAPON_FX_ANIM, who.weapon);
	endif

	if (!who.acctname)
		attackAnim := 0x0c;
	endif

	PerformAction(who, attackAnim);
endfunction

function getWeaponFX(FX, weapon, isCriticalAttack := 0)
	var attackFX;
	var weaponAttr := weapon.getAttrBase();

	if (weapon.objtype == FIST)
		if (FX == WEAPON_FX_ANIM)
			attackFX := getAnim(UNARMED_FX, isCriticalAttack);
		else
			attackFX := getSound(UNARMED_FX, isCriticalAttack);
		endif
	else
		if (weapon.isRanged())
			if (weapon.isBow())
				if (FX == WEAPON_FX_ANIM)
					attackFX := getAnim(BOW_FX, isCriticalAttack);
				else
					attackFX := getSound(BOW_FX, isCriticalAttack);
				endif
			else
				if (FX == WEAPON_FX_ANIM)
					attackFX := getAnim(CROSSBOW_FX, isCriticalAttack);
				else
					attackFX := getSound(CROSSBOW_FX, isCriticalAttack);
				endif
			endif
		else
			if (weapon.getType().Upper() == TWOHANDED.Upper())
				if (weaponAttr.AttackType.Upper() == SLASHDAMAGE.Upper())
					if (FX == WEAPON_FX_ANIM)
						attackFX := getAnim(SLASH_2H_FX, isCriticalAttack);
					else
						attackFX := getSound(SLASH_2H_FX, isCriticalAttack);
					endif
				elseif (weaponAttr.AttackType.Upper() == PIERCINGDAMAGE.Upper())
					if (FX == WEAPON_FX_ANIM)
						attackFX := getAnim(PIERCING_2H_FX, isCriticalAttack);
					else
						attackFX := getSound(PIERCING_2H_FX, isCriticalAttack);
					endif
				elseif (weaponAttr.AttackType.Upper() == BLUNTDAMAGE.Upper())
					if (FX == WEAPON_FX_ANIM)
						attackFX := getAnim(BLUNT_2H_FX, isCriticalAttack);
					else
						attackFX := getSound(BLUNT_2H_FX, isCriticalAttack);
					endif
				endif
			else
				if (weaponAttr.AttackType.Upper() == SLASHDAMAGE.Upper())
					if (FX == WEAPON_FX_ANIM)
						attackFX := getAnim(SLASH_1H_FX, isCriticalAttack);
					else
						attackFX := getSound(SLASH_1H_FX, isCriticalAttack);
					endif
				elseif (weaponAttr.AttackType.Upper() == PIERCINGDAMAGE.Upper())
					if (FX == WEAPON_FX_ANIM)
						attackFX := getAnim(PIERCING_1H_FX, isCriticalAttack);
					else
						attackFX := getSound(PIERCING_1H_FX, isCriticalAttack);
					endif
				elseif (weaponAttr.AttackType.Upper() == BLUNTDAMAGE.Upper())
					if (FX == WEAPON_FX_ANIM)
						attackFX := getAnim(BLUNT_1H_FX, isCriticalAttack);
					else
						attackFX := getSound(BLUNT_1H_FX, isCriticalAttack);
					endif
				endif
			endif
		endif
	endif

	return attackFX;
endfunction

function getAnim(fxs, isCriticalAttack := 0)
	return !isCriticalAttack ? fxs.ANIM[RandomInt(fxs.ANIM.Size())+1] : fxs.ANIM_CRITICAL[RandomInt(fxs.ANIM_CRITICAL.Size())+1];
endfunction

function getSound(fxs, isCriticalAttack := 0)
	return !isCriticalAttack ? fxs.SFX[RandomInt(fxs.SFX.Size())+1] : fxs.SFX_CRITICAL[RandomInt(fxs.SFX_CRITICAL.Size())+1];
endfunction