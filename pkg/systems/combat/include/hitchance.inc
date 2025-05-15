/*
 * Refatorado por EvilLink
 * Se der merda, pode reclamar comigo...
 *
 * Criado para organizar a lógica de cálculo de chance de acerto do combatHook
 * o cálculo precisa ser feito em alguns scripts fora do .src.
 * Portanto, foi separado pra cá.
 */

// include "combat";
include "habilidades";

function CalculateHitChance(attacker, defender)
	var skill_a; // Não sei quem teve essa ideia de merda de nomear assim...
	var skill_d; // Vou deixar pra não ficar mais complexo o Refactor.

	if ( !attacker.npctemplate )
		skill_a := CDbl(AP_GetSkill(attacker, attacker.weapon.attribute));

		if ( GetEquipmentByLayer(attacker, LAYER_MOUNT) && GetObjProperty(attacker, "chardata").trait != "Cavalaria de Escaramuça")
			skill_a := skill_a / 2;
		endif

		if (attacker.weapon.attribute == WRESTLING) // Ajustar se for soco pra melhor melee combat
			if (AP_GetSkill(attacker, ONEHANDED) > AP_GetSkill(attacker, TWOHANDED))
				skill_a := AP_GetSkill(attacker, ONEHANDED);
			else
				skill_a := AP_GetSkill(attacker, TWOHANDED);
			endif
		endif

	else
		skill_a := CDbl(AP_GetSkill(attacker, WRESTLING));
	endif

	if ( GetObjProperty(attacker, "metamorfose") != error)
		skill_a := cint(GetObjProperty(attacker, "skill_a"));
	endif

	// Piloto de Golem
	if ( GetObjProperty(attacker, "pilotando") != error)
		skill_a := cint(GetObjProperty(attacker, "skill_a"));
	endif

	if ( !defender.npctemplate )
		skill_d := CDbl(AP_GetSkill(defender, defender.weapon.attribute));
		if ( GetEquipmentByLayer(defender, LAYER_MOUNT) && GetObjProperty(defender, "chardata").trait != "Cavalaria de Escaramuça")
			skill_d := skill_d / 2;
		endif
		if (defender.weapon.attribute == WRESTLING)
			if (AP_GetSkill(defender, ONEHANDED) > AP_GetSkill(defender, TWOHANDED))
				skill_d := AP_GetSkill(defender, ONEHANDED);
			else
				skill_d := AP_GetSkill(defender, TWOHANDED);
			endif
		endif
	else
		skill_d := CDbl(AP_GetSkill(defender, WRESTLING));
	endif

	if ( GetObjProperty(defender, "metamorfose") != error)
		skill_d := CInt(GetObjProperty(defender, "skill_d"));
	endif

	// Piloto de Golem
	if ( GetObjProperty(defender, "pilotando") != error)
		skill_d := CInt(GetObjProperty(defender, "skill_d"));
	endif

	var hit_chance := 50.0 + ((skill_a - skill_d) / 1.5);

	hit_chance += CDbl( GetObjProperty(attacker, "hitbonus") ) + attacker.hitchance_mod - defender.evasionchance_mod;
	hit_chance += CInt( GetObjProperty(attacker, "#attackbonus" ));


	if (getcooldown(defender, "evasion_chance"))
		hit_chance -= 10;
	endif

    if (getcooldown(attacker, "blinded") > 0)
		hit_chance -= 20;
	endif

	if (TemHabilidade(attacker, "Postura Ofensiva"))
		hit_chance += 10;
	endif
	if (TemHabilidade(defender, "Postura Ofensiva"))
		hit_chance += 10;
	endif
	if (TemHabilidade(attacker, "Postura Defensiva"))
		hit_chance -= 10;
	endif
	if (TemHabilidade(defender, "Postura Defensiva"))
		hit_chance -= 10;
	endif
	if (TemHabilidade(defender, "Preservar Orgaos"))
		hit_chance -= 10;
	endif
	if (GetCooldown(attacker, "posturadecombate") > 0)
		hit_chance += 10;
	endif

    var challenged_by := GetObjProperty(attacker, "#DuelChallenge");
    if (challenged_by && attacker.acctname) // Only affects players
        if (challenged_by.time < ReadGameClock()) // Challenge expired
            EraseObjProperty(attacker, "#DuelChallenge");
        else
            var challenger := SystemFindObjectBySerial(challenged_by.challenger);
            if (challenger && challenger.connected)
                if (defender.serial != challenger.serial) // If attacking someone other than the challenger
                    hit_chance -= 10;
                    if (RandomInt(100) < 20) // 20% chance to remind
                        SendSysMessageEx(attacker, "Você está em duelo! Seus ataques são menos efetivos contra outros alvos.", SSM_FAIL);
                    endif
                endif
            else
                EraseObjProperty(attacker, "#DuelChallenge"); // Clean up if challenger disconnected
            endif
        endif
    endif

	if (TemHabilidade(attacker, "Aura de Fe"))
		hit_chance += attacker.party.members.size()/*  * 2 */;
	endif

	if ( GetObjProperty(attacker, "armapreferida") == attacker.weapon.objtype )
		hit_chance += 5;
	endif

	var balanced_weapon := CInt(GetObjProperty(attacker.weapon, "Aprimoramentos")["Arma Balanceada"]);
	if (balanced_weapon)
		hit_chance += balanced_weapon;

		DebugMsg(attacker, DEBUG_HIT, "Arma Balanceada: " + CInt(balanced_weapon));
	endif

	if (GetCooldown(attacker, "furia" && TemHabilidade(attacker, "Fúria Indomável")))
		if (!TemHabilidade(defender, "Fortitudo Impia"))
			hit_chance += 10;
		endif
	endif

	if ( cint(GetObjProperty(attacker, "#attackbonus")) != 0 )
		EraseObjProperty(attacker, "#attackbonus");
	endif

	if ( IsRaging(attacker) )
		if (TemHabilidade(defender, "Iludir o Fraco") )
			hit_chance -= 10;
		endif
		if (GetCooldown(attacker, "Ataques Enlouquecidos") > 0 )
			hit_chance += 100;
		endif
	endif
    
	if ( IsRaging(defender) )
		if (TemHabilidade(defender, "Esquiva Instintiva") )
			hit_chance -= 10;
		endif
		if (GetCooldown(defender, "Ataques Enlouquecidos") > 0 )
			hit_chance += 100;
		endif
	endif

    var swash_debuff := cint(AP_GetSkill(defender, TACTICS)/5);
	if (GetCooldown(attacker, "desafioheroico") > 0 )
        hit_chance -= swash_debuff;
	endif

    
	if (GetCooldown(attacker, "defesapersistente") > 0) //penaliza ataque
        var parry_bonus_sem_dor := Cint(Ap_GetSkill(attacker, PARRY) / 5);
		hit_chance -= parry_bonus_sem_dor;
	endif

	if (GetCooldown(defender, "defesapersistente") > 0) //beneficia defesa
        var parry_bonus_sem_dor := Cint(Ap_GetSkill(defender, PARRY) / 5);
		hit_chance -= parry_bonus_sem_dor;
	endif

	if (GetCooldown(defender, "resistenciaheroica") > 0)
		var hostiles := ListHostiles(defender, 3);
		if ( hostiles.size() >= 5)
			hit_chance := hit_chance  - 15;
        elseif ( hostiles.size() >= 3)
            hit_chance := hit_chance  - 10;
        elseif ( hostiles.size() >= 1)
            hit_chance := hit_chance  - 5;
		endif
	endif

	if (temhabilidade(attacker, "Lobo Solitario") )
		var hostiles := ListHostiles(attacker, 3);
		if ( hostiles.size() > 1)
			hit_chance := hit_chance + 15;
		endif
	endif

	var tiroCerteiro := Cint(getObjProperty(attacker.weapon, "tirocerteiro"));
	if ( tiroCerteiro > 0)
		tiroCerteiro := tiroCerteiro - 1;
		if (tiroCerteiro == 0)
			EraseObjProperty(attacker.weapon, "tirocerteiro");
		else
			SetObjProperty(attacker.weapon, "tirocerteiro", tiroCerteiro);
		endif
		hit_chance := hit_chance + 100.0;
	endif

	if (TemHabilidade(defender, "Budōka"))
		var monk_weapons := array{ 0x0df0, 0x13f9, 0x0e8a, 0x0e81, 0x1F020, 0x27AD, 0x27AE, 0x27A7};
        var tactics_bonus := CInt(AP_GetSkill(defender, TACTICS) / 5);
		if ( defender.weapon.objtype in monk_weapons )
			hit_chance := hit_chance - tactics_bonus;
		endif
	endif

    var dancer_bonus := CInt(AP_GetSkill(attacker, ARCANA) / 5);
    if (TemHabilidade(attacker, "Danca das Sombras"))
        hit_chance := hit_chance + dancer_bonus;
	endif
    if (GetCooldown(defender, "shadowdancer_blur") > 0)
        hit_chance := hit_chance - dancer_bonus;
    endif

    if (TemHabilidade(attacker, "Movimentacao Graciosa"))
	    var swash_bonus := CInt(AP_GetSkill(attacker, TACTICS) / 10);
        hit_chance := hit_chance + swash_bonus;
    endif

    if (TemHabilidade(defender, "Movimentacao Graciosa"))
		var swash_bonus := CInt(AP_GetSkill(defender, TACTICS) / 10);
        hit_chance := hit_chance - swash_bonus;
    endif

	if (TemHabilidade(attacker, "Mancha da Corrupcao"))
		var demon_hunter_bonus := CInt(AP_GetSkill(attacker, THEOLOGY) / 10);
		hit_chance := hit_chance + demon_hunter_bonus;
	endif

	if (TemHabilidade(defender, "Mancha da Corrupcao"))
		var demon_hunter_bonus := CInt(AP_GetSkill(defender, THEOLOGY) / 10);
		hit_chance := hit_chance - demon_hunter_bonus;
	endif

	var witcher_bonus := Cint(AP_GetSkill(attacker, SCIENCE) / 5);
	if (TemHabilidade(attacker, "Oleo do Cacador"))
		hit_chance := hit_chance + witcher_bonus;
	endif

    var attacker_weapon := attacker.weapon.objtype;
    var ranged_weapons := array{ 0x13B2, 0xe137, 0x26CC, 0xe139, 0x2d2b, 0x2d2a, 0x27A5, 0x0f4f, 0x13fd };

    if (GetCooldown(defender, "Desviar Disparos") > 0 && attacker_weapon in ranged_weapons)
        printtext(defender, "*desviar disparos*");
        hit_chance := 0;
    endif

	if (defender.acctname  )
		if (!defender.warmode)
			hit_chance := 100;
		endif
	endif

	if (GetObjProperty(attacker, "#attackfail") == 1)
		EraseObjProperty(attacker, "#attackfail");
		hit_chance := 0;
	endif

	if (GetObjProperty(defender, "#defensefail") == 1)
		EraseObjProperty(defender, "#defensefail");
		hit_chance := 100;
	endif

	if (GetCooldown(attacker, "ataquetotal"))
		hit_chance := 100;
	endif
	if (GetCooldown(defender, "ataquetotal"))
		hit_chance := 100;
	endif

	// Verifica acerto na escuridão
	if (!attacker.npctemplate)
		DarknessHitModifier(attacker, defender, hit_chance);
	endif
	
	if ( cint( GetObjProperty(attacker, "#ataqueinesperado") ) == 1 )
		if ( GetObjProperty(defender, "lastHit") != error)
			var def := GetObjProperty(defender, "lastHit");
			def := def.serial;
			if (def.serial != attacker.serial)// se o defensor nao acertou o atacante
				hit_chance := hit_chance + 50;
				printtext(attacker, "*ataque inesperado*");
			endif
		endif
		EraseObjPRoperty(attacker, "#ataqueinesperado");
	endif

	if ( cint(GetObjProperty(attacker, "#bomcomeco")) == 1)
		var attackers := GetObjProperty(defender, "#bomcomecoserials");
		if (attackers == error)
			attackers := array;
		endif
		if ( !(attacker.serial in attackers))
			SendSysMessageEx(attacker, "Voce comecou bem.", SSM_INFO);
			hit_chance := hit_chance + 50;
			attackers.append(attacker.serial);
			SetObjProperty(defender,  "#bomcomecoserials", attackers);
		endif
		EraseObjProperty(attacker, "#bomcomeco");
	endif

	if (GetObjProperty(defender, "#mobilidade") == 1)
		hit_chance -= 15.0;
		AP_ConsumeVital(defender, STAMINA, 3);
	endif
	if ( GetObjProperty(attacker, "#mobilidade") == 1)
		hit_chance -= 15.0;
	endif

	if (defender.npctemplate == ":epicai:spiderling")
		hit_chance -= 20;
	endif

	EraseObjProperty(attacker, "insistencia");

	if (TemHabilidade(attacker, "Tirothan"))
		var mlore_bonus := CInt(AP_GetSkill(attacker, ARCANA) / 10);
		hit_chance += mlore_bonus;
	endif

	if (hit_chance < 5.0)
		hit_chance := 5.0;
	elseif (hit_chance > 100.0)
		hit_chance := 100.0;
	endif

	return hit_chance;
endfunction

function DualWieldPenalty(attacker)
	var weaponElem := GetWpnCfgInfo(attacker, attacker.weapon);
	var penalty := (100-AP_GetSkill(attacker, weaponElem.Attribute))/2;
		if (penalty < 10)
			penalty := 10;
		endif
	
	return penalty;
endfunction

function DarknessHitModifier(attacker, defender, byref hit_chance)
    var attacker_race := GetObjProperty(attacker, "chardata").raca;
    var in_darkness := InDarkness(attacker);
	var darkness_intensity := "muita";
	var metamorfose := GetObjProperty(attacker, "metamorfose");
	if (metamorfose)
		return;
	endif

    if (in_darkness && attacker_race != DROW)
        if (!HasLightNearby(defender))
			if (GetCooldown(attacker, "nightvision") > 0)
            	hit_chance := hit_chance - 10;
            	SendSysMessageEx(attacker, "Você luta com dificuldade.", SSM_FAIL);
			else
				hit_chance := hit_chance - 20;
				SendSysMessageEx(attacker, "Você luta com muita dificuldade.", SSM_FAIL);
			endif
        endif
    elseif (!in_darkness && attacker_race == DROW)
		SendSysMessageEx(attacker, "Você luta com dificuldade.", SSM_FAIL);
		if (GetCooldown(attacker, "lightvision") > 0)
			hit_chance := hit_chance - 10;
			SendSysMessageEx(attacker, "Você luta com dificuldade.", SSM_FAIL);
		else
			hit_chance := hit_chance - 20;
			SendSysMessageEx(attacker, "Você luta com muita dificuldade.", SSM_FAIL);
		endif
    endif
endfunction

function HasLightNearby(defender)
    const SEARCH_RANGE := 2;
    
    // Check for items with light flag
    var flag_lightsources := ListItemsNearLocationWithFlag(defender.x, defender.y, defender.z, SEARCH_RANGE, TILEDATA_FLAG_LIGHTSOURCE, defender.realm);
    if (flag_lightsources.size() > 0)
        return 1;
    endif

	var statics_lightsources := ListStaticsInBox( defender.x+SEARCH_RANGE, defender.y+SEARCH_RANGE, defender.z+5, defender.x-SEARCH_RANGE, defender.y-SEARCH_RANGE, defender.z-5, 0, defender.realm);
    // Check for specific light source objects
	UnloadConfigFile(":combat:config/lightsources");
    var cfg := ReadConfigFile(":combat:config/lightsources");
    var elem := cfg["LightSources"];
    var objtype_lightsources := array{};

	foreach light in GetConfigStringArray(elem, "LightSource")
        objtype_lightsources.append(light);
	endforeach

	foreach light in objtype_lightsources
		foreach static in statics_lightsources
			if (CInt(static.objtype) == CInt(light))
				return 1;
			endif
		endforeach
	endforeach

    var nearby_items := ListItemsNearLocation(defender.x, defender.y, defender.z, SEARCH_RANGE, defender.realm);
    foreach obj in nearby_items
        if (obj.objtype in objtype_lightsources)
            return 1;
        endif
    endforeach

	var nearby_mobiles := ListMobilesNearLocation(defender.x, defender.y, defender.z, SEARCH_RANGE, defender.realm);
	var isEquippedLightSource := {0x0a12, 0x0a22, 0x0a23, 0x0a24, 0x0a15, 0xC626, 0xC620, 0xC622, 0xC628}; // tocha e lanterna acesas

	foreach mob in nearby_mobiles
		if (!mob.npctemplate)
			foreach item in (listequippeditems(mob))
				if (Cint(item.graphic) in isEquippedLightSource)
					return 1;
				endif
			endforeach
		endif
	endforeach
    
	return 0;
endfunction