include ":equipsys:utils";

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