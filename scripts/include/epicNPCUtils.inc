function customizeNpc(mob)
	var npc_elem := NPC_GetNPCConfig(mob);

	var classe := npc_elem.Classe;
	var tier := npc_elem.Tier;
	var tactics := RandomIntMinMax(tier * 20, tier * 30);
	var skill_combat;
	var skill_support;
	
	var mob_skill := 0;
	if (tier == 1)
		mob_skill += RandomIntMinMax(20, 40);
	elseif (tier == 2)
		mob_skill += RandomIntMinMax(40, 60);
	elseif (tier == 3)
		mob_skill += RandomIntMinMax(60, 80);
	elseif (tier == 4)
		mob_skill += RandomIntMinMax(80, 100);
	elseif (tier == 5)
		mob_skill += RandomIntMinMax(100, 120);
	else
		mob_skill += RandomIntMinMax(0, 20);
	endif

	if (classe == SABIO)
		AP_SetTrueSkill(mob, MAGERY, mob_skill);
		AP_SetTrueSkill(mob, MAGICLORE, mob_skill);
		skill_combat := MAGERY;
		skill_support := MAGICLORE;
	else
		skill_combat := WRESTLING;
		skill_support := TACTICS;
	endif

	AP_SetTrueSkill(mob, WRESTLING, mob_skill);
	AP_SetTrueSkill(mob, TACTICS, tactics);

    var calculate_level_hp := Cint(AP_GetSkill(mob, skill_combat) + AP_GetSkill(mob, skill_support)) - 80;

	var hp := 0;
	if (classe == SABIO)
		hp := 15;
	elseif (classe == LADINO)
		hp := 20;
	elseif (classe == GUERREIRO)
		hp := 25;
	endif

	var hitsmod := GetObjProperty(mob, "hitsmod");
	if (!hitsmod)
		hitsmod := 0;
	endif
    if ((calculate_level_hp / 6) >= 1 )
        var new_hp := 0;
        var multiplier := Cint((calculate_level_hp / 6));
        if (classe == GUERREIRO)
            new_hp := 10 * multiplier;
        elseif (classe == LADINO)
            new_hp := 8 * multiplier;
        elseif (classe == SABIO)
            new_hp := 6 * multiplier;
		else
			new_hp := 0;
        endif
        hp += Cint(new_hp);
    endif

	SetObjProperty(mob, "hitsmod", hitsmod + hp);

	RecalcVitals(mob);
    HealDamage(mob, hitsmod + hp);
endfunction

function HasPlayerNear(me)
    var nearby := ListMobilesNearLocationEx(me.x, me.y, LIST_IGNORE_Z, 30, LISTEX_FLAG_NORMAL + LISTEX_FLAG_HIDDEN + LISTEX_FLAG_CONCEALED, me.realm);
    foreach mobile in nearby
        if (mobile.acctname)
            return 1;
        endif
    endforeach
    return 0;
endfunction

function HasPlayerNearLocation(x, y, realm := 0)
	var mobs := ListMobilesNearLocationEx( x, y, LIST_IGNORE_Z, 15, LISTEX_FLAG_NORMAL|LISTEX_FLAG_HIDDEN|LISTEX_FLAG_PLAYERS_ONLY );
	foreach mob in mobs
		if ( !mob.cmdlevel && !mob.concealed ) //se for player e priv de player
			return 1;
		endif
      sleepms(2);
	endforeach

	return 0;
endfunction