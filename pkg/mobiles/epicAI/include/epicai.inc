use cfgfile;

include ":loot:include/lootParser";
include ":tn:recipes";
include ":mounts:mounts";
include "npcUtils";

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

    // Humanoid-specific initialization
    SetObjProperty(mobile, "linguas_conhecidas", {"Comum"});
    SetObjProperty(mobile, "lingua_escolhida", "Comum");

    // Add any other humanoid-specific setup here
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
    var reposition_tries := 0;
    var start_pos := struct{ "x" := me.x, "y" := me.y };

    while(Distance(me, opponent) < max_dist && runs < 10) // Reduced from 50 to 10 attempts
        RunAwayFrom(opponent);
        runs += 1;
        sleepms(200);

        // Try to find a path after each reposition
        var test_path := FindPath(me.x, me.y, me.z, opponent.x, opponent.y, opponent.z, me.realm, FP_IGNORE_MOBILES);
        if (test_path)
            PrintTextAbove(me, "*encontrou novo caminho*");
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
        if (me.x == start_pos.x && me.y == start_pos.y)
            reposition_tries += 1;
            if (reposition_tries >= 3)
                PrintTextAbove(me, "*mudando direção*");
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
    endwhile

    PrintTextAbove(me, "*reposição completa*");
    return 1;
endfunction