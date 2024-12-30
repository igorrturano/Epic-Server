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
    elseif (taxonomy == "canidae" || taxonomy == "felidae" || taxonomy == "chiroptera" || taxonomy == "rodentia"|| taxonomy == "reptilia" || taxonomy == "crustacean" || taxonomy == "scarabaeidae")
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