
function customizeNpc(mob)
	var npc_elem := NPC_GetNPCConfig(mob);

	var Hits := npc_elem.HITS;
	var classe := npc_elem.Classe;
	var tier := npc_elem.Tier;
    var npc_wrestling := npc_elem.WRESTLING;
    var npc_tactics := npc_elem.TACTICS;

    var calculate_level_hp := Cint(npc_wrestling + npc_tactics - 80);

	var hp := 25;
	if (classe == SABIO)
		hp := 15;
	elseif (classe == LADINO)
		hp := 20;
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
        else
            new_hp := 6 * multiplier;
        endif
        hp += Cint(new_hp);
    endif

	SetObjProperty(mob, "hitsmod", hitsmod + hp);

	RecalcVitals(mob);
    HealDamage(mob, hitsmod + hp);

	var mob_skill := 0;
	if (tier == 1)
		mob_skill += RandomIntMinMax(20, 40);
	elseif (tier == 2)
		mob_skill += RandomIntMinMax(40, 60);
	elseif (tier == 3)
		mob_skill += RandomIntMinMax(60, 80);
	endif
    var tactics := RandomIntMinMax(tier * 20, tier * 30);

	AP_SetTrueSkill(mob, WRESTLING, mob_skill);
	AP_SetTrueSkill(mob, TACTICS, tactics);

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