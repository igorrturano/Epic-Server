use cfgfile;

include ":loot:include/lootParser";
include ":tn:recipes";
include ":mounts:mounts";
include "npcUtils";
include ":fls_core:fls_characters";

const MAX_Z_DIFF := 18;

function InitializeEpicNPC(mobile)
    var npc_already_initiliazed := GetObjProperty(mobile, "started");
    if (npc_already_initiliazed)
        return 0;
    endif

	var npc_elem := NPC_GetNPCConfig(mobile);
    var npccfgfile := ReadConfigFile( ":epicai:npcdesc" );
    var mytemplate := ParseTemplateName(mobile.npctemplate).template;
    var elem := FindConfigElem(npccfgfile, mytemplate);
    var sounds := GetConfigStringArray(elem, "IdleSound");

    var isDarknessCreature := GetObjProperty(mobile, "Mobnoturno") || Lower(npc_elem.Category) == "undead";
    if ((isDarknessCreature) && !InDarkness(mobile))
        ApplyRawDamage(mobile, CInt(AP_GetVitalMaximumValue(mobile, "Hits")));
    endif

    setupAnchorNPC(mobile);
    setupChampspawnNPC(mobile);
    setupNPCBackPack(mobile);
    GenerateRecipeLoot(mobile);
    SetupAmmo(mobile, npccfgfile);
    var loot := npccfgfile[mytemplate].Loot;
    if (loot)
        Loot_Generate(mobile.backpack, loot);
    endif

    var throwItem := CInt(npc_elem.throwItem);
    if (throwItem != error)
        SetObjProperty(mobile, "throwItem", throwItem);
    endif

    var brain_type := GetConfigString(npc_elem, "BrainType");
    // Type-specific initialization
    case (brain_type)
        "monster":  InitializeMonsterNPC(mobile, npc_elem, elem, sounds);
        "animal":   InitializeAnimalNPC(mobile, npc_elem, elem, sounds);
        "humanoid": InitializeHumanoidNPC(mobile, npc_elem, elem, sounds);
        default:    InitializeGenericNPC(mobile, npc_elem, elem, sounds);
    endcase

    SetObjProperty(mobile, "started", 1);
endfunction

function InitializeMonsterNPC(mobile, npc_elem, elem, sounds)
    var has_colors := npc_elem.colors;
    if (has_colors)
        setupNPCColor(mobile, has_colors);
    endif
    SetupNPCEquip(mobile, npc_elem);

    // Monstros falam uma língua específica
    var known_languages := SplitWords(npc_elem.Languages);
    var chosen_language := known_languages[RandomInt(Len(known_languages)) + 1];
    SetObjProperty(mobile, "linguas_conhecidas", known_languages);
    SetObjProperty(mobile, "lingua_escolhida", chosen_language);

    // Add any other monster-specific setup here
endfunction

function InitializeAnimalNPC(mobile, npc_elem, elem, sounds)
    var has_colors := npc_elem.colors;
    if (has_colors)
        setupNPCColor(mobile, has_colors);
    endif

    SetupAnimalGender(mobile, npc_elem);
    SetupAnimalCriticalHit(mobile, npc_elem);

    // Animal-specific initialization
endfunction

function InitializeHumanoidNPC(mobile, npc_elem, elem, sounds)
    var has_colors := npc_elem.colors;
    if (has_colors)
        setupNPCColor(mobile, has_colors);
    endif

    setupNPCGender(mobile);
    SetupNPCEquip(mobile, npc_elem);
    SetupBaseLanguages(mobile);

    if (mobile.isSage())
        SetupWizardClass(mobile, npc_elem);
    endif
endfunction

function SetupWizardClass(mobile, npc_elem)
    if (!mobile.isMage())
        return 0;
    endif

    var spellschool := GetConfigString(npc_elem, "SpellSchool");
    var base_ability;
    
    case (spellschool)
        "Kaijin": base_ability := "Sacrificio da Carne";
        "Ahtaleloreth": base_ability := "Vínculo Elemental";
        "Aradalore": base_ability := "Elo Natural";
        "Evocatium": base_ability := "Determinacao Arcana";
        "Velkyn": base_ability := "Danca das Sombras";
        default: return 0;
    endcase

    // Build progression based on base ability
    var habcfg := ReadConfigFile(":charactercreation:config/habilidades");
    var progression := GetClassProgression(habcfg, base_ability);
    SetObjProperty(mobile, "myhabs", progression);

    return 1;
endfunction

function SetupWarriorClass(mobile, npc_elem)
    if (!npc.isWarrior())
        return 0;
    endif
    var subclass;
    var speed_type := SplitWords(GetConfigString(npc_elem, "EpicAttackSpeed"), "_");
    var isArcher := (speed_type[1] == "RANGED");
    if (isArcher)
      subclass := "Tiro Poderoso";
    endif

    var progression := GetClassProgression(habcfg, subclass);
    SetObjProperty(mobile, "myhabs", progression);

    return 1;
endfunction

function SetupBaseLanguages(mobile)
    SetObjProperty(mobile, "linguas_conhecidas", {"Comum"});
    SetObjProperty(mobile, "lingua_escolhida", "Comum");
endfunction

function InitializeGenericNPC(mobile, npc_elem, elem, sounds)
   // Chamar o initializeNPC do ghaia para fazer uma migração não destrutiva, mesmo que
   // eu deva só adicionar mobs que eu tenha criado
endfunction

function SetupAmmo(mobile, npccfgfile)

	if ( npccfgfile[ParseTemplateName(mobile.npctemplate).template].MissileWeapon )
		var weapon := CreateItemAtLocation(mobile.x, mobile.y, mobile.z, npccfgfile[ParseTemplateName(mobile.npctemplate).template].MissileWeapon, 1, mobile.realm);
		if ( !EquipItem(mobile, weapon) )
			DestroyItem(weapon);
		endif
	endif

	if ( npccfgfile[ParseTemplateName(mobile.npctemplate).template].AmmoType )
		if ( CInt(npccfgfile[ParseTemplateName(mobile.npctemplate).template].AmmoAmount) > 0 )
			var ammo := CreateItemInBackpack(mobile, npccfgfile[ParseTemplateName(mobile.npctemplate).template].AmmoType, CInt(npccfgfile[ParseTemplateName(mobile.npctemplate).template].AmmoAmount));
			if ( !ammo )
				return ammo;
			endif
			return 1;
		endif
	endif

	return 1;

endfunction

function SetupNPCEquip(who, npc_elem)
    var categ := GetConfigString(npc_elem, "Equip");
    categ := categ;
    if(isMaleGraphic(who))
        categ := categ + "M";
    elseif (who.graphic == 0x191)
        categ := categ + "F";
    endif
    var template := ReadConfigFile(":ghaia:config/equip2");
    var group := FindConfigElem(template, categ);
    if(group)
        var str;
        var lastc := 0;
        foreach str in GetConfigStringArray(group, "Equip")
            lastc := EquipNPCItemString(who, str, lastc);
        endforeach
        foreach str in GetConfigStringArray(group, "Armor")
            lastc := EquipNPCItemString(who, str, lastc );
        endforeach
        foreach str in GetConfigStringArray(group, "Weapon")
            lastc := EquipNPCItemString(who, str, lastc);
        endforeach
    endif
    
  var montaria := Getobjproperty(who, "Montaria");
  if (!montaria)
  else
	var graphic := MP_GetMountGraphic(montaria);
	var item := CreateItemAtLocation(who.x, who.y, who.z, MOUNT_OBJTYPE, 1);
	item.graphic := graphic;
	Equipitem (who, item);
  endif
endfunction

function SetupAnimalCriticalHit(mobile, npc_elem)
    var gender_set := npc_elem.Gender;
    var taxonomy := GetConfigString(npc_elem, "Taxonomy");
    var evil := (mobile.alignment == 1);
    if (!evil)
        return;
    endif

    if (taxonomy == "bovidae" || taxonomy == "cervidae")
        case (RandomInt(1))
            0: SetObjProperty(mobile, "Critic", "Coice" );
            1: SetObjProperty(mobile, "Critic", "Chifrada");
        endcase
    elseif (taxonomy == "suidae" || taxonomy == "camelidae" || taxonomy == "equus")
        SetObjProperty(mobile, "Critic", "Coice");
    elseif (taxonomy == "anura" || taxonomy == "serpentes")
        case (RandomInt(1))
            0: SetObjProperty(mobile, "Critic", "Bote" );
            1: SetObjProperty(mobile, "Critic", "Glandulas_concentradas" );
        endcase
    elseif (taxonomy == "primatomorpha" || taxonomy == "ursidae")
        case (RandomInt(2))
            0: SetObjProperty(mobile, "Critic", "Frenesi" );
            1: SetObjProperty(mobile, "Critic", "Rugir" );
        endcase
    elseif (taxonomy == "canidae" || taxonomy == "felidae" || taxonomy == "chiroptera" || taxonomy == "reptilia" || taxonomy == "crustacean" || taxonomy == "scarabaeidae")
        case (RandomInt(2))
            0: SetObjProperty(mobile, "Critic", "Presas_profundas" );
            1: SetObjProperty(mobile, "Critic", "Garras_afiadas" );
        endcase
    elseif (taxonomy == "immortuos" || taxonomy == "draco")
        SetObjProperty(mobile, "Critic", "Baforada" );
    elseif (taxonomy == "aves" || taxonomy == "phasianidae")
        SetObjProperty(mobile, "Critic", "Garras_afiadas");
    endif

    SetObjProperty(mobile, "Selvagem", 1);

endfunction

function SetupAnimalGender(mobile, npc_elem)
  var gender_set := npc_elem.Gender;

  if ( gender_set == 0)
    //var gender_new := Cint(randomint(2) + 1);
    case(randomint(2))
          0: Setobjproperty(mobile, "Sexo", 1);
          1: Setobjproperty(mobile, "Sexo", 2);
        endcase
  else
    Setobjproperty(mobile, "Sexo", Cint(mobile.Gender) );
  endif

endfunction

function setupNPCBackPack(mobile)
	if ( !mobile.backpack )
		var backpack := CreateItemAtLocation(mobile.x, mobile.y, mobile.z, "BackPack", 1);
		var result := EquipItem(mobile, backpack);
		if ( !result )
			DestroyItem(backpack);
			return result;
		else
			return backpack;
		endif
	endif

	return mobile.backpack;
endfunction

function setupNPCColor(who, colors)
    colors := SplitWords(colors);

    var color := Cint(colors[RandomInt(Len(colors)) + 1]);
    who.color := color;
    who.truecolor := color;
endfunction

function setupNPCGender(mobile)
  var female := GetObjProperty(mobile, "female");

  if (female)
    case (mobile.graphic)
        400: mobile.graphic := 401;
        1830: mobile.graphic := 1831;
        1832: mobile.graphic := 1833;
        1828: mobile.graphic := 1829;
    endcase
    mobile.gender := 1;
  endif
endfunction

function setupAnchorNPC(mobile)
    var spawnpointserial := GetObjProperty(mobile, "spawnpoint");
        if (spawnpointserial)
            var spawnpoint := SystemFindObjectBySerial(spawnpointserial);
            if (spawnpoint)
                var range := cint(GetObjProperty(spawnpoint, "Range"));
            if (!range)
                range := 6;
            endif
            SetObjProperty(mobile, "anchor", array{ spawnpoint.x , spawnpoint.y , spawnpoint.z + 2 });
            SetObjProperty(mobile, "range", range);
            var e := SetAnchor(spawnpoint.x, spawnpoint.y, range, 90);
        endif
    endif
endfunction

function setupChampspawnNPC(mobile)
    if (GetObjPRoperty(mobile, "championspawn") == 2)
        SetObjProperty(mobile, "#championspawn", 1);
        ApplyDamage( mobile, 20000);
    elseif (GetObjPRoperty(mobile, "championspawn") == 1)
        SetObjPRoperty(mobile, "championspawn", 2);
    endif
endfunction

function ParseTemplateName(template_name)

  if ( template_name.IsA(POLCLASS_NPC) )
    template_name := template_name.npctemplate;
  endif

  //Find positions of ":" characters
  var colon_a := Find(template_name, ":", 1);
  var colon_b := Find(template_name, ":", 2);

  var parsed := struct{"package", "template"};

  if ( !colon_a || !colon_b )
    //Not a complete package name - use default npcdesc.cfg
    parsed.package := "";
    parsed.template := template_name;
  else
    parsed.package := template_name[colon_a+1, colon_b-2];
    parsed.template := template_name[colon_b+1, Len(template_name)];
  endif

  return parsed;
endfunction

function EquipNPCItemString(who, str, lastcolor)
    var params := SplitWords(str);
    var objname;
    var color;
    var chance;
    var lastc := 0;
    var colors := ReadConfigFile(":ghaia:config/colors");
    var vestiments := ReadConfigFile(":ghaia:config/cloths");
    objname := params[1];
    if(CInt(objname))
        objname := CInt(objname);
    endif
    if(CInt(params[2]))
        color := CInt(params[2]);
    else
        color := 0;
    if(params[2] == "lastcolor" || params[2] == "lastcolour")
        color := lastcolor;
    else
        var colorg := FindConfigElem(color, params[2]);
        if(colorg)
            var str := GetConfigStringArray(colorg, "Color");
            var ar := array{};
            var rndint := RandomInt(len(str)) + 1;
            ar := SplitWords(str[rndint]);
        if(len(ar) > 1)
            if(CInt(ar[2]) > 0)
                color := RandomInt(CInt(ar[2]) - CInt(ar[1]))+1;
             else
                color := CInt(str[RandomInt(len(str))+1]);
            endif
            else
                color := CInt(str[RandomInt(len(str))+1]);
            endif
        if(color == 1)
            color := 0;
        endif
      endif
    endif
  endif
  lastc := color;
  if(CInt(params[3]))
    chance := CInt( params[3] );
  else
    chance := 100;
  endif
  if(CInt(RandomInt(99)+1) <= CInt(chance))
    var item := CreateItemAtLocation(5852, 1163, 0, objname, 1);
    if(!item)
      var itemg := FindConfigElem(vestiments, objname);
      if(itemg)
        var str := GetConfigStringArray(itemg, "Item");
        var str3;
        var newarray := array{};
        var i := 1;
        var sex := "";
        foreach str3 in str
          var str2 := SplitWords(str3);
          sex := CStr(str2[2]);
          if(sex != error)
            if(sex == "m" || sex == "f")
              if(sex == "m" && who.graphic == 400)
                newarray[i] := str2[1];
                i := i + 1;
              elseif(sex == "f" && who.graphic == 401)
                newarray[i] := str2[1];
                i := i + 1;
              endif
            else
              newarray[i] := str2[1];
              i := i + 1;
            endif
          else
            newarray[i] := str2[1];
            i := i + 1;
          endif
        endforeach
        objname := CInt(newarray[RandomInt(len(newarray))+1]);
        item := CreateItemAtLocation(5852, 1163, 0, objname, 1);
      endif
    endif
    item.color := color;
    if(CInt(params[4]) == 1)
      MoveItemToContainer(item, who.backpack);
    else
      if(!EquipItem(who, item))
        MoveItemToContainer(item, who.backpack);
      endif
    endif
  endif
  return lastc;
endfunction

function Flee(byref opponent, sleepdelay := 0, change_opponent := 0)
  //  if ("FleeStealth" in custombehaviors)
  //     if (!GetCooldown(me, "fleestealth"))
  //        PrintText(me, "*esconde*");
  //        sleep(1);
  //        me.hidden := 1;
  //        me.stealthsteps := 30;
  //        SetCooldown(me, "fleestealth", 30);
  //     endif
  //  endif

   var numflees := cint(GetObjProperty(me,"#flees"));
   if(numflees > 10)
      EraseObjProperty(me, "#flees");
      RestartScript(me);
   else
      numflees := numflees + 1;
      SetObjProperty(me,"#flees", numflees);
   endif
   
   var runs := 0;
   if (!sleepdelay)
      sleepdelay := 275 - me.run_speed;
   endif
   var last_loc := array{opponent.x, opponent.y, opponent.z};
   while(Distance(me, opponent) < 30 && runs < 50 && CoordinateDistance(opponent.x, opponent.y, last_loc[1], last_loc[2]) <= 3)
      sleepms(sleepdelay);
      var ev := wait_for_event(0);
      RunAwayFrom(opponent);
      runs := runs +1;

      if (change_opponent && randomint(6) <= 2)
         me.setAggro(opponent.serial, -100);
         if (ChooseHighestOpponent(me, opponent))
            break;
         endif
      endif

      if (ev.type == SYSEVENT_DAMAGED)
         if (((ev.source.npctemplate) and (CheckLineOfSight(me, ev.source))) 
               || (CoordinateDistance(me.x, me.y, ev.source.x, ev.source.y) < CoordinateDistance(me.x, me.y ,opponent.x, opponent.y)) 
               || (!CheckLineOfSight(me, opponent)) || (!opponent) || (RandomInt(3)==1))
            opponent := ev.source;
            SetOpponent(opponent);
            me.setAggro(opponent, 300);
         endif
      endif

      findAnotherPath();
   endwhile

   SetObjProperty(me, "#noback", polcore().systime+15);
endfunction

function ChooseHighestOpponent(me, byref opponent, new_opponent := 0, force_change := 0)
   if (!opponent && new_opponent)
      opponent := new_opponent;
      SetOpponent(new_opponent);
      TurnToward(new_opponent);
      return 1;
   elseif (opponent && new_opponent)
      SetOpponent(opponent);
      TurnToward(opponent);
      return 1;
   elseif (opponent && !CheckLineOfSight(me, opponent))
      me.setAggro(opponent.serial, -50);
   endif

   if (GetCooldown(me, "#blockaggro"))
      return 0;
   endif

   var highest_aggro := getHighestAggro(me);
   if (highest_aggro && (!opponent || highest_aggro != opponent.serial))
      highest_aggro := SystemFindObjectBySerial(highest_aggro);
      opponent := highest_aggro;
      SetOpponent(highest_aggro);
      TurnToward(highest_aggro);
      return 1;
   endif
endfunction

function getHighestAggro(creature)
   var aggros := creature.aggroProp();
   var highest := array{0, -9999999};
   foreach serial in (aggros.keys())
      if (aggros[serial] >= highest[2])
         highest := array{serial, aggros[serial]};
      endif
      sleepms(2);
   endforeach

   return highest[1];
endfunction

function TacticalReposition(opponent)
    var runs := 0;
    var max_dist := 12; // Only run 5 tiles away instead of 30

    while(Distance(me, opponent) < max_dist && runs < 10) // Reduced from 50 to 10 attempts
        RunAwayFrom(opponent);
        runs += 1;
        sleepms(200);

        // Try to find a path after each reposition
        var test_path := FindPath(me.x, me.y, me.z, opponent.x, opponent.y, opponent.z, me.realm, FP_IGNORE_MOBILES);
        if (test_path)
            foreach coord in test_path
                if (Distance(me, opponent) <= 1)
                    break;
                endif
                RunTowardLocation(coord.x, coord.y);
                sleepms(200);
            endforeach
            return 1;
        else
          Flee(opponent, 0, 1);
        endif

        // If we haven't moved, try a different direction
        findAnotherPath();
    endwhile

    return 1;
endfunction

function findAnotherPath()
    var start_pos := struct{ "x" := me.x, "y" := me.y };
    var reposition_tries := 0;

    if (me.x == start_pos.x && me.y == start_pos.y)
        reposition_tries += 1;
        if (reposition_tries >= 3)
            // Try to move laterally
            var directions := array{
                struct{ "x" := 2, "y" := 0 },
                struct{ "x" := -2, "y" := 0 },
                struct{ "x" := 0, "y" := 2 },
                struct{ "x" := 0, "y" := -2 }
            };
            foreach dir in directions
                var new_x := me.x + dir.x;
                var new_y := me.y + dir.y;
                if (CheckLosAt(me, new_x, new_y, me.z))
                    RunTowardLocation(new_x, new_y);
                    sleepms(200);
                    break;
                endif
                sleepms(2);
            endforeach
            reposition_tries := 0;
        endif
    endif
endfunction

function CheckForIdealTarget()
   // If we already have an opponent, stick with them most of the time
   if (me.opponent && CanFight(me, me.opponent))
      // Only 5% chance to even consider switching targets
      if (RandomInt(100) < 95)
         return me.opponent;
      endif
   endif

   var potential_opponents := ListMobilesNearLocationEx(me.x, me.y, me.z, AREA_SIZE, LISTEX_FLAG_NORMAL);
   if (potential_opponents.size() < 1)
      return 0;
   endif

   var best_opponent := struct{
      "mobile" := 0,
      "priority" := -1
   };

   foreach opponent in potential_opponents
      // Skip invalid opponents
      if (!CanFight(me, opponent))
         continue;
      endif

      var opponent_class := opponent.getClass();
      var opponent_is_ranged := opponent.weapon.isRanged() || opponent_class == SABIO;
      var priority := 0;

       // Current opponent gets huge priority bonus to maintain aggro
      if (opponent == me.opponent)
         priority += 15;
      endif

      // Fast NPC prioritizes ranged players
      if (opponent_is_ranged)
         priority += 2;
      endif

      // Class-based opponenting
      if (me.isSage()) // NPC mage prefers warriors
         if (opponent_class == GUERREIRO)
            priority += 2;
         endif
      elseif (me.isWarrior()) // NPC warrior prefers rogues
         if (opponent_class == LADINO)
            priority += 2;
         endif
      elseif (me.isRogue()) // NPC rogue prefers mages
         if (opponent_class == SABIO)
            priority += 2;
         endif
      endif

      // Prioritize close opponents slightly
      var dist := Distance(me, opponent);
      if (dist <= 4)
         priority += 1;
      endif

      // Check if this is a better opponent
      if (priority > best_opponent.priority)
         best_opponent.mobile := opponent;
         best_opponent.priority := priority;
      endif

      sleepms(2);
   endforeach

   return best_opponent.mobile;
endfunction

// New function to handle ally behavior
function AlertAllies(invasor, me)
    if (isAlly(invasor))
        return 0;
    endif

    if (invasor.hidden || invasor.concealed)
        return 0;
    endif

    if (GetCooldown(me, "alert_allies"))
        return 0;
    endif
    
    foreach mobile in ListMobilesNearLocation(me.x, me.y, me.z, 8)
        if (mobile.npctemplate && mobile != me)
            if (isAlly(mobile))
                SetObjProperty(mobile, "#taunt_opponent", invasor.serial);
                SendEvent(mobile, struct{ "type" := EVID_ALERT_ALLIES, "source" := invasor });
            endif
        endif
        sleepms(2);
    endforeach

    SetCooldown(me, "alert_allies", 15);
endfunction

function isAlly(mobile)
    if (!mobile || mobile.dead) return 0; endif

    if (mobile.master || GetObjProperty(mobile, "summoned")) return 0; endif

    if (mobile.hidden || mobile.concealed) return 0; endif

    if (mobile.acctname || mobile.alignment != 1) return 0; endif
    
    return 1;
endfunction

function CanFight(attacker, defender)
    if (!defender || defender.dead || defender.hidden || defender.concealed)
        return 0;
    endif

    // Cooldown to avoid ress kill
    if (Cint(GetCooldown(defender, "deathtime" )) > 0) 
        // Allow combat if defender initiated it
        if (me.opponent == defender) return 1; endif
        // Otherwise prevent attacking during cooldown
        return 0;
    endif

    // If defender is a player, normal combat rules apply
    if (!defender.npctemplate)
        return 1;
    endif

    // NPC vs NPC combat only happens if:
    if (defender.npctemplate)
        // Check if forced to attack by GM command
        var forced_target := GetObjProperty(attacker, "#ForcedTarget");
        if (forced_target == defender.serial)
            return 1;
        endif

        if (defender.master)
            return 1;
        endif

        // Check enemy categories
        var attacker_elem := NPC_GetNPCConfig(attacker);
        var defender_elem := NPC_GetNPCConfig(defender);
        var enemy_cats := GetConfigStringArray(attacker_elem, "EnemyCategories");
        
        if (enemy_cats && defender_elem.Category in enemy_cats)
            return 1;
        endif

        return 0; // No valid reason for NPCs to fight
    endif

    return 1;
endfunction

function PursueTarget(opponent)
    var dist := Distance(me, opponent);
    if (!shouldKeepPursuing(opponent, dist)) return 0; endif

    if (dist >= 2 && dist <= 6)
        if (me.canSpitAcid())
            AcidSpray(me, opponent);
        endif

        if (me.canShootWeb())
            WebShot(me, opponent);
        endif 
    endif

    if (me.canThrow())
        if (dist >= 2 && dist <= 8)
            ThrowSpecial(me, opponent);
        endif
    endif

    // Track progress towards target
    var progress := GetObjProperty(me, "#pursue_progress");
    if (!progress)
        progress := struct{
            "last_dist" := dist,
            "last_pos" := struct{ "x" := me.x, "y" := me.y },
            "last_check_time" := ReadGameClock(),
            "no_progress_count" := 0,
            "start_time" := ReadGameClock()
        };
    endif

    // Only check progress every 2 seconds
    if (ReadGameClock() >= progress.last_check_time + 2)
        var moved := (me.x != progress.last_pos.x || me.y != progress.last_pos.y);
        var got_closer := (dist < progress.last_dist);
        
        // No progress if we haven't moved OR haven't gotten closer in 2 seconds
        if (!moved || !got_closer)
            progress.no_progress_count += 1;
        else
            progress.no_progress_count := 0;
        endif

        // Update tracking info
        progress.last_dist := dist;
        progress.last_pos.x := me.x;
        progress.last_pos.y := me.y;
        progress.last_check_time := ReadGameClock();
    endif

    // If we're not making progress after several checks, force a reposition
    if (progress.no_progress_count >= 1) // Zero tolerance for no progress
        EraseObjProperty(me, "path");
        EraseObjProperty(me, "#enemy_last_loc");
        TacticalReposition(opponent);
        progress.no_progress_count := 0;
        progress.last_check_time := ReadGameClock();
        SetObjProperty(me, "#pursue_progress", progress);
        return 1;
    endif

    SetObjProperty(me, "#pursue_progress", progress);

    // Normal pursuit logic
    if (CheckLineOfSight(me, opponent))
        if (Distance(me, opponent) > 1)
            RunToward(opponent);
        endif
        sleepms(100);
    else
        var path := FindPath(me.x, me.y, me.z, opponent.x, opponent.y, opponent.z, me.realm, FP_IGNORE_MOBILES);
        if (path)
            foreach coord in path
                if (Distance(me, opponent) <= 1)
                    break;
                endif
                if (!shouldKeepPursuing(opponent, dist)) break; endif
                RunTowardLocation(coord.x, coord.y);
                sleepms(100);
            endforeach
        else
            progress.no_progress_count += 1;
            if (me.hasTunnelTactics())
                TunnelTactics(me, opponent);
            endif
        endif
    endif

    return 1;
endfunction

function EnableMainEvents()
    DisableEvents(SYSEVENT_OPPONENT_MOVED + SYSEVENT_DISENGAGED);
    EnableEvents(SYSEVENT_ENGAGED + SYSEVENT_DAMAGED + SYSEVENT_ENTEREDAREA, AREA_SIZE);
    EnableEvents(SYSEVENT_DISENGAGED + SYSEVENT_LEFTAREA, AREA_SIZE);
endfunction

function DisableMainEvents()
    DisableEvents(SYSEVENT_ENGAGED + SYSEVENT_DAMAGED + SYSEVENT_ENTEREDAREA);
    DisableEvents(SYSEVENT_OPPONENT_MOVED + SYSEVENT_DISENGAGED);
endfunction

function HandleIdleState()
    DisableMainEvents();
    EnableEvents(SYSEVENT_ENTEREDAREA + SYSEVENT_ENGAGED + SYSEVENT_DAMAGED, AREA_SIZE);
    
    // Return to spawn point if we have one
    var spawn_point := SystemFindObjectBySerial(GetObjProperty(me, "spawnpoint"));
    var anchor := GetObjProperty(me, "anchor");
    var home_x, home_y;
    
    if (spawn_point)
        home_x := spawn_point.x;
        home_y := spawn_point.y;
    elseif (anchor)
        home_x := anchor[1];
        home_y := anchor[2];
    else
        return;
    endif

    while (!hasPlayerNear(me))    
        if (CoordinateDistance(me.x, me.y, home_x, home_y) > 5)
            WalkTowardLocation(home_x, home_y);
        endif
    endwhile
    
    // No need for event checking here since MonsterBrain already does it
    return 1;
endfunction

function FleeFrom(opponent)
    if (!opponent)
        return 0;
    endif

    var runs := 0;
    while (opponent && Distance(me, opponent) < 16 && runs < 20)
        RunAwayFrom(opponent);
        if(!AP_ConsumeVital(me, STAMINA, 5))
            PrintTextAbove(me, "*Exausto*");
            break;
        endif
        runs += 1;
        sleepms(100);
    endwhile
endfunction

function MaintainRange(opponent)
    var sleepdelay := 275 - me.run_speed;
    if (sleepdelay < 0)
        sleepdelay := 50;
    endif

    var dist := CoordinateDistance(me.x, me.y, opponent.x, opponent.y);
    var z_diff := Abs(me.z - opponent.z);

    if (z_diff > 18)
        return 0;
    endif

    if (dist < 4)
        RunAwayFrom(opponent);
        sleepms(sleepdelay);
    elseif (dist > 10)
        RunToward(opponent);
        sleepms(sleepdelay);
    else
        // Try to stay closer to allies if possible
        var allies := NPC_GetNearbyAllies(me);
        if (allies.size() > 0)
            var closest_ally := allies[1];
            var ally_dist := Distance(me, closest_ally);
            
            if (ally_dist > 4)
                RunToward(closest_ally);
                sleepms(sleepdelay);
                return 0;
            endif
        endif
        
        TurnToward(opponent);
        return 1;
    endif

    return 0;
endfunction

function DoThiefCombatBehavior(opponent)
    if (Distance(me, opponent) < 6)
        return 0;
    endif

    // 50% chance to steal, 50% chance to sneak attack
    if (RandomInt(100) < 50)
        DoThiefSteal(opponent);
    else
        DoThiefSneakAttack(opponent);
    endif
endfunction

function ShamanBehavior(shaman)
    // First priority: Check allies health
    if (GetCooldown(shaman, "heal_check"))
        return 0;
    endif

    var npc_elem := NPC_GetNPCConfig(shaman);
    var caster_tier := CInt(npc_elem.Tier);
    if (!caster_tier)
        caster_tier := 1;
    endif

    // Get all allies nearby
    var allies := NPC_GetNearbyAllies(shaman);
    var wounded_allies := array{};

    // Check allies' health
    foreach ally in allies
        if (isHurt(ally)) // Is Hurt
            wounded_allies.append(ally);
        endif
    endforeach

    if (wounded_allies.size() == 0)
        DoHarmfulSpell();
    endif

    // If multiple allies need help, cast healing rain
    if (wounded_allies.size() >= 3 && !GetCooldown(shaman, "healing_rain"))
        PerformHealingRain(shaman, wounded_allies);
        SetCooldown(shaman, "healing_rain", 45);
        return 1;
    endif

    foreach ally in wounded_allies
        var most_wounded := struct{
            "mobile" := 0,
            "hp_percent" := 100
        };

        foreach ally in wounded_allies
            var hp_percent := CDbl(AP_GetVital(ally, HITS)) / CDbl(AP_GetVitalMaximumValue(ally, HITS)) * 100;
            if (hp_percent < most_wounded.hp_percent)
                most_wounded.mobile := ally;
                most_wounded.hp_percent := hp_percent;
            endif
            sleepms(2);
        endforeach

        if (most_wounded.mobile)
            PerformHeal(shaman, most_wounded.mobile, caster_tier);
            SetCooldown(shaman, "heal_check", 15);
            return 1;
        endif
        sleepms(2);
    endforeach

    return 0;
endfunction

function AskForHealing(me)
    // First look specifically for shamans
    if (!isHurt(me))
        return 0;
    endif

    foreach mobile in ListMobilesNearLocation(me.x, me.y, me.z, 12)
        if (mobile.npctemplate && mobile != me)
            if (mobile.isSage())
                SetObjProperty(mobile, "#healing_needed", me.serial);
                SendEvent(mobile, struct{ "type" := EVID_BUDDYATTACKED, "source" := me });
            endif
        endif
        sleepms(2);
    endforeach
endfunction 

function shouldKeepPursuing(opponent, dist)
    if (!opponent)
        return 0;
    endif

    // Don't chase too far
    if (dist > AREA_SIZE + 6)
        return 0;
    endif

    // If we're in melee range and can see opponent, stop pursuing
    if (dist <= 1 && CheckLineOfSight(me, opponent))
        return 0;
    endif

    // Prevent flee spam by adding a cooldown
    if (GetCooldown(me, "flee_check"))
        return 1; // Continue pursuing if on cooldown
    endif
    var npcState := getNPCState(me);
    var cd := 15;
    
    if (npcState == "dying")
        cd := 5;
    endif

    SetCooldown(me, "flee_check", cd);
    var allies := NPC_GetNearbyAllies(me);
    if (allies.size() > 0)
        if (!GetCooldown(me, "allies"))
            SetObjProperty(me, "initial_allies", allies.size());
            SetCooldown(me, "allies", 15);
        endif

        var initial_allies := GetObjProperty(me, "initial_allies");
        var already_fighting := GetObjProperty(me, "already_fighting");
        
        // Only consider fleeing if we've lost allies
        if (initial_allies && allies.size() <= CInt(initial_allies/2))
            // Don't flee if already engaged unless heavily outnumbered
            if (!already_fighting || ListHostiles(me, 3).size() >= allies.size() * 2)
                // Higher chance to flee when heavily damaged
                var flee_chance := (npcState == "dying") ? 40 : 15;
                
                if (RandomInt(100) < flee_chance)
                    PrintTextAbove(me, "*"+me.name+" recua*");
                    FleeFrom(opponent);
                    return 0;  // Added return 0 after fleeing
                endif
            endif
        endif
    else
        // If alone, don't pursue new targets but stay if already fighting
        if (!GetObjProperty(me, "already_fighting"))
            return 0;
        endif
    endif

    return 1;  // Continue pursuing if none of the above conditions are met
endfunction

function LootCorpse(corpse)
    if (!CorpseHasLoot(corpse))
        return 0;
    endif

    if (GetCooldown(me, "loot_cooldown") > 0)
        return 0;
    endif

    // Approach the corpse
    while (Distance(me, corpse) > 1)
        RunToward(corpse);
        sleepms(100);
        
        // Break if corpse disappeared or too far
        if (!corpse || Distance(me, corpse) > 20)
            return 0;
        endif
    endwhile

    // Calculate max items to loot based on intelligence
    var max_loot := 1/* CInt(AP_GetStat(me, INTELLIGENCE)/20) */;
    if (max_loot < 1)
        max_loot := 1;
    endif
    var looted := 0;
    
    // Ensure NPC has a backpack
    if (!me.backpack)
        var backpack := CreateItemAtLocation(me.x, me.y, me.z, "Backpack", 1);
        if (!EquipItem(me, backpack))
            return 0;
        endif
    endif
    
    // Loot items
    while (looted < max_loot)
        PrintTextAbove(me, "*revira o corpo*");
        if (!LootSomething(me, corpse))
            break;
        endif
        looted += 1;
        sleepms(1500);
    endwhile
    
    return (looted > 0);
endfunction

function CorpseHasLoot(corpse)
    if (!corpse || corpse.IsA(POLCLASS_CORPSE) == error)
        return 0;
    endif

    foreach item in EnumerateItemsInContainer(corpse)
        if (item.movable)
            return 1;
        endif
        sleepms(2);
    endforeach

    return 0;
endfunction

function LootSomething(npc, corpse)
    // Define priority items by objtype (coins)
    var coins := array{0xBA63, 0xBA64, 0xBA65}; // copper, silver, gold
    var resources := array{0x1bf2, 0x1bd7};     // ingots, wood

    if (GetCooldown(npc, "already_looted") > 0)
        return 0;
    endif

    // First try to loot coins
    var priority_loot := FindPriorityLoot(corpse, coins);
    if (priority_loot)
        return LootItem(npc, priority_loot);
    endif

    // Then try resources
    var resource_loot := FindPriorityLoot(corpse, resources);
    if (resource_loot)
        return LootItem(npc, resource_loot);
    endif

    // Then look for equipment by class
    foreach item in EnumerateItemsInContainer(corpse)
        if (!item.movable || item.newbie)
            continue;
        endif

        if (CInt(GetObjProperty(item, "#EquippedOn")) == corpse.serial && item.IsA(POLCLASS_ARMOR))
            continue;
        endif

        if (item.IsA(POLCLASS_WEAPON) || item.IsShield() || item.IsA(POLCLASS_ARMOR))
            return LootItem(npc, item);
        endif

        // Check containers
        if (item.IsA(POLCLASS_CONTAINER))
            var container_item := FindEquipmentInContainer(item);
            if (container_item)
                return LootItem(npc, container_item);
            endif
        endif
        
        // Adicionei pra meter o bicho em alguma coisa pelo menos (bandages, tesoura)
        return LootItem(npc, item);
        sleepms(2);
    endforeach

    return 1;
endfunction

function FindEquipmentInContainer(container)
    foreach item in EnumerateItemsInContainer(container)
        if (!item.movable)
            continue;
        endif

        if (item.IsA(POLCLASS_WEAPON) || item.IsShield() || item.IsA(POLCLASS_ARMOR))
            return item;
        endif

        if (item.IsA(POLCLASS_CONTAINER))
            var container_item := FindEquipmentInContainer(item);
            if (container_item)
                return container_item;
            endif
        endif
        
        sleepms(2);
    endforeach
    
    return 0;
endfunction

function LootItem(npc, item)
    if (!item)
        return 0;
    endif

    if (!npc.backpack)
        var backpack := CreateItemAtLocation(npc.x, npc.y, npc.z, "Backpack", 1);
        EquipItem(npc, backpack);
    endif

    PrintTextAbove(npc, "*pega " + item.desc + "*");
    SetCooldown(npc, "already_looted", 300);

    return MoveItemToContainer(item, npc.backpack);
endfunction

function FindPriorityLoot(corpse, graphics_array)
    foreach item in EnumerateItemsInContainer(corpse)
        if (!item.movable)
            continue;
        endif

        if (item.objtype in graphics_array || item.graphic in graphics_array)
            return item;
        endif

        // Check containers for these items
        if (item.IsA(POLCLASS_CONTAINER))
            var container_item := FindPriorityLoot(item, graphics_array);
            if (container_item)
                return container_item;
            endif
        endif
        
        sleepms(2);
    endforeach
    
    return 0;
endfunction

function ClearCombatState()
    SetWarMode(0);
    SetOpponent(0);
endfunction