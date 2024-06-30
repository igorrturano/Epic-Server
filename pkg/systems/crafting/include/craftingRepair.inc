// //$Id: craftingRepair.inc 916 2005-11-05 23:44:32Z austin $

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  name:    repair.inc
//  version: 2.0
//  author:  MuadDib
//
//  Purpose: Contains all the central Repair functions involving repairing items, using repair
//           deeds, etc.
//
//  Changes: Sept 14, 2005 - Updated repair_item() for swing counter (new crafting ready)
//                         - Added CreateRepairDeed() Function with support for the following skills:
//                           Smithy, Bowcraft, Carpenter, Tailor (new crafting ready)
//                         - Moved repair_this() function to the include (new crafting ready)
//                         - Moved s_weapon_armor() to the include (new crafting ready)
//                         - Moved resmelt() to include (New crafting ready)
//
////////////////////////////////////////////////////////////////////////////////////////////////////

//include "include/OLD/sound";
use os;
include "include/imersao";

function repair_item(character, what, craft_skill)

var cfg;
var reparos:=0;
var ver_reparos:=0;
var repairdelay:=3;


if (craft_skill == METALWORKING)
	cfg := ReadConfigFile(":metalworking:blacksmithy");
elseif (craft_skill == DomesticWork)
	cfg := ReadConfigFile(":DomesticWork:tailoring");
elseif (craft_skill == WOODWORKING)
	cfg := ReadConfigFile(":woodworking:carpentry");
else
	return 0;
endif


var elem := FindConfigElem(cfg, Hex(cint(what.objtype)));

var item_skill:= GetConfigString(elem,"skill");

var hisskill := AP_GetSkill(character, craft_skill);

ver_reparos:=(GetObjProperty(what,"reparos"));

if (ver_reparos == error)
	reparos:=(hisskill/2);
	SetObjProperty(what,"reparos",reparos);
endif

ver_reparos:=(GetObjProperty(what,"reparos"));

if (ver_reparos >= (11 - (hisskill / 11)));

if (cint(hisskill)>=cint(item_skill))
  var maxhp;
  maxhp := what.maxhp;
  SetObjProperty(what, "broken", 0);
  SetName(what, GetObjProperty(what, "realdesc"));
  maxhp := cint(maxhp * what.quality);
  var thehp := what.hp;
  var damaged := maxhp - thehp;
  var damageamount := 0.0;
  damageamount := (125 -  hisskill)/3000.0;

  if(what.hp == what.maxhp)
    SendSysMessage(character, "O item nao esta danificado.");
    return;
  endif

  if((what.hp <= 0) || (what.quality <= 0.1) || (!what.quality))
    SendSysMessage(character, "O fragil material quebrou quando voce manipulou ele.");
    Imersao(character, "susto");
    Imersao(what, "vidro");
    DestroyItem(what);
    return;
  endif

  if(what.quality <= damageamount)
    SendSysMessage(character, "O fragil material quebrou quando voce manipulou ele.");
    Imersao(character, "susto");
    Imersao(what, "vidro");
    DestroyItem(what);
    return;
  endif
  what.quality := what.quality - damageamount;

  if((what.hp <= 0) || (what.quality <= 0.1) || (!what.quality))
    SendSysMessage(character, "O fragil material quebrou quando voce manipulou ele.");
    Imersao(character, "susto");
    Imersao(what, "vidro");
    DestroyItem(what);
    return;
  endif

 if (SkillCheck(character, craft_skill, cint(damaged / 3), 0) > 0 )
    var repaired_amount := cint((hisskill + 1)/2);
    //SendSysMessage(character, "rep"+repaired_amount);
    //SendSysMessage(character, "hisskill: "+((randomint(hisskill) + 1))/2);

    if (damaged == 1)
      repaired_amount := 1;
    endif

    if (thehp + repaired_amount > what.maxhp)
      what.hp := what.maxhp;
      EraseObjProperty(what, "swings");
      AnimRepair(character, craft_skill, repairdelay);
      SendSysMessage(character, "Voce consertou o item completamente.");
    else
      what.hp := thehp + repaired_amount;
      AnimRepair(character, craft_skill, repairdelay);
      SendSysMessage(character, "Voce consertou o item.");
    endif

    if ((what.hp <= 0) || (what.quality <= 0.1) || (!what.quality))
      SendSysMessage(character, "O fragil material quebrou quando voce manipulou ele.");
      DestroyItem(what);
      Imersao(character, "susto", 1,0,1);
      Imersao(character, "vidro");
    return;
    endif

  else
    var damage_amount := randomint(10);

    if (thehp <= damage_amount)
      DestroyItem(what);
      SendSysMessage(character, "Voce falhou e destruiu o item.");
      Imersao(character, "susto", 1,0,1);
      Imersao(character, "vidro");
    endif

    thehp := thehp - damage_amount;
    what.hp := thehp;

    if(randomint(2) == 1)
      what.quality := what.quality - (damaged/100);
    endif

    if(thehp <= 0)
      DestroyItem(what);
      SendSysMessage(character, "Voce falhou e destruiu o item.");
      Imersao(character, "susto", 1,0,1);
      Imersao(character, "vidro");
    else
      SendSysMessage(character, "Voce falhou e danificou o item.");
    endif

    if ((what.hp <= 0) || (what.quality <= 0.1) || (!what.quality))
      SendSysMessage(character, "O fragil material quebrou quando voce manipulou ele.");
      DestroyItem(what);
      Imersao(character, "susto", 1,0,1);
      Imersao(character, "vidro");
      return;
    endif
endif

if (GetObjProperty(what, "reparos") == error)
	reparos:=hisskill/2;
	reparos:= (reparos  -(11 - (hisskill / 11)));
	SetObjProperty(what,"reparos",reparos);
else
	reparos:=GetObjProperty(what, "reparos");
	reparos:= (reparos  -(11 - (hisskill / 11)));
	SetObjProperty(what,"reparos",reparos);
endif

else
	SendSysMessage(character, "Voce nao possui habilidades para isto.");
endif

else
	SendSysMessage(character, "Este item nao pode ser reparado: " + GetObjProperty(what,"reparos") + " ponto(s) de reparo restante(s).");
endif

endfunction

function AnimRepair(who, skill, delay)
  for i:=1 to delay
    case (craft_skill)
      METALWORKING: Imersao(who, "mw"); break;
      DomesticWork: Imersao(who, "dw"); break;
      WOODWORKING: Imersao(who, "ww"); break;
    endcase
    sleep(2);
  endfor
endfunction

function CreateRepairDeed(who, scroll, craft_skill)

  var type, title, color;
  case(CInt(craft_skill))
     METALWORKING: 		type := "Blacksmith"; color := CInt(2107);
     WOODWORKING: 	type := "Bowyer"; color := CInt(2116);
     //CARPENTRY: 		type := "Carpenter"; color := CInt(2113);
     DomesticWork: 		type := "Tailor"; color := CInt(2128);
  endcase
  var ramt := AP_GetSkill(who, craft_skill);
  if(ramt < 50)
    SendSysMessage(who, "You must be at least apprentice level to create a repair service contract.");
    return;
  endif
  if(ramt < 60 )
    title := "A repair service contract from an Apprentice " + type;
  elseif(ramt < 70 )
    title := "A repair service contract from a Journeyman " + type;
  elseif(ramt < 80 )
    title := "A repair service contract from an Expert " + type;
  elseif(ramt < 90 )
    title := "A repair service contract from an Adept " + type;
  elseif(ramt < 100 )
    title := "A repair service contract from a Master " + type;
  elseif(ramt < 110 )
    title := "A repair service contract from a Grandmaster " + type;
  elseif(ramt < 120 )
    title := "A repair service contract from an Elder " + type;
  elseif(ramt < 130 )
    title := "A repair service contract from a Legendary " + type;
  endif
  type := lower(type);
  var ret := GetItemDescriptor(0xee09);
  ret.Color := color;
  ret.CProps.insert("type", type);
  ret.CProps.insert("skillid", craft_skill);
  ret.CProps.insert("ramt", ramt);
  var theitem := CreateItemInBackpack(who, ret, 1);
  if(theitem)
    SubtractAmount(scroll, 1);
  endif
  SetName(theitem, title);
  return;
endfunction



function repair_this(who, craft_skill := METALWORKING)

  SendSysMessage(who, "Escolha o item para consertar..");
  var use_on := Target(who);
  if(!Accessible(who, use_on))
    return;
  endif

  //SendSysMessage(who, "craft_skill: "+craft_skill);
  //SendSysMessage(who, "movable: "+use_on.movable);
  if(use_on.movable == 0)
    SendSysMessage(who, "Voce nao pode consertar isto.");
    return;
  endif

  if((!Accessible(who, use_on)) || (distance(who, use_on) > 2))
    SendSysMessage(who, "Voce nao alcanca o item.");
    return;
  endif

  if(CInt(use_on.objtype) == CInt(0xe34))
    case(CInt(craft_skill))
      METALWORKING:            CreateRepairDeed(who, use_on, craft_skill);
//    BOWCRAFT_FLETCHING:    CreateRepairDeed(who, use_on, craft_skill);
  //  CARPENTRY:   		 CreateRepairDeed(who, use_on, craft_skill);
    //TAILORING:   		 CreateRepairDeed(who, use_on, craft_skill);
    endcase
    return;
  endif
  var forge := 0;
  case(craft_skill)
    METALWORKING:
      forge := 1;
      break;
    DomesticWork:
      if (is_tailor_item(use_on.objtype))
        repair_item( who, use_on, craft_skill);
      else
        SendSysMessage(who, "Voce nao pode consertar isso.");
      endif
      return;
    WOODWORKING:
      if (is_carpentry_item(use_on.objtype))
        repair_item( who, use_on, craft_skill);
      else
        SendSysMessage(who, "Voce nao pode consertar isto.");
      endif
      return;
  endcase

  if(!forge)
   if(is_weapon_armor(use_on.objtype))
    repair_item( who, use_on, craft_skill);
   else
    SendSysMessage(who, "Voce so pode concertar armas ou armaduras.");
   endif
  else
    if(is_weapon_armor(use_on.objtype))
      var near_items := ListItemsNearLocation(who.x, who.y, who.z,2, who.realm);
      foreach item in near_items
        if ((item.objtype == UOBJ_ANVIL1) || (item.objtype == UOBJ_ANVIL2))
          repair_item( who, use_on, craft_skill);
        return;
      endif
    endforeach
    SendSysMessage( who, "Voce precisa estar proximo de uma forja!" );
    return;
    else
      SendSysMessage(who, "Voce nao pode consertar isto.");
    endif
  endif
  return;
//SendSysMessage(character,"Você não possui habilidades para isto");
endfunction

function is_weapon_armor(what)
  var combatcfg := ReadConfigFile(":combat:itemdesc");
  var elem := FindConfigElem(combatcfg, what);
  var repairable := GetConfigInt(elem, "repairable");
  if (repairable == 1)
    return 1;
  else
    return 0;
  endif
endfunction

function is_tailor_item(what)
  var tailorcfg := ReadConfigFile(":DomesticWork:tailoring");
  var elem := FindConfigElem(tailorcfg, what);
  //var repairable := GetConfigInt(elem, "repairable");
  if (elem )
    return 1;
  else
    return 0;
  endif
endfunction

function is_carpentry_item(what)
  var carpentrycfg := ReadConfigFile(":woodwork:carpentry");
  var elem := FindConfigElem(carpentrycfg, what);
  //var repairable := GetConfigInt(elem, "repairable");
  if (elem )
    return 1;
  endif

  var carpentrycfg2 := ReadConfigFile(":combat:itemdesc");
  elem := FindConfigElem(carpentrycfg2, what);
  if (elem)
   return 1;
  endif

  return 0;

endfunction



function resmelt(who, the_attribute)

	var near_items := ListItemsNearLocation(who.x, who.y, who.z, 3, who.realm);
	var forge_near := 0;
	foreach forge in near_items
		if((forge.objtype == 0xfb1) || ((forge.objtype >= 0x197a) && (forge.objtype <= 0x19a9)))
			forge_near := 1;
			break;
		endif
	endforeach
	if(forge_near == 0)
		SendSysMessage(who, "You must be near a forge.");
		return 0;
	endif

	SendSysMessage(who, "Select the item you wish to melt.");
	var use_on := Target(who, TGTOPT_CHECK_LOS);
	if(!use_on || !accessible(who, use_on))
		return 0;
	endif
	if(use_on.container.serial != who.backpack.serial)
		SendSysMessage(who, "That is not in your backpack.");
		return 0;
	endif
	if(!Accessible(who, use_on) || !ReserveItem( use_on ))
		SendSysMessage(who, "You cannot use that.");
		return 0;
	endif
	if(use_on.movable == 0)
		SendSysMessage(who, "You cannot use that while it is locked down.");
		return 0;
	endif
	if(use_on in ListEquippedItems(who))
		SendSysMessage(who, "You cannot use that.");
		return 0;
	endif

	var cfgarray := GetConfigStringKeys(resource_config);
	var ingot := -1;
	if(use_on.isA(POLCLASS_CONTAINER))
		var contitems := EnumerateItemsInContainer(use_on);
		var skill, amt;
		var loop_counter := 3;
		foreach thing in contitems
			loop_counter := loop_counter + 1;
			if( loop_counter == 4 )
				SendSysMessage(who, "Smelting items in container......");
				loop_counter := 0;
			endif
			if(craft_config[thing.objtype])
				skill := craft_config[thing.objtype].skill;
				amt := CInt(craft_config[thing.objtype].material);
				if((!skill) || (!amt))
					return 0;
				endif
				skill := CInt(skill/2);
				if(skill < 5)
					skill := 5;
				endif
				amt := CInt((amt * AP_GetSkill(who, the_attribute) * 75 ) / 10000);
				if(amt < 1)
					amt := 1;
				endif
				ingot := -1;
				var mattype := CInt(GetObjProperty(use_on, "material"));
				if(mattype > 0)
					if(mattype != error)
						ingot := mattype;
					endif
				endif
				if(ingot == -1)
					foreach entry in cfgarray
						if(lower(resource_config[entry].Type) == "metal")
							if(CInt(resource_config[entry].Color) == CInt(thing.color))
								ingot := entry;
							endif
						endif
					endforeach
				endif
				if(ingot == -1)
					SendSysMessage(who, "You have no idea how to smelt this strange ore!");
					return 0;
				endif
				PlaySoundEffect(who, 0x2c);
				sleep(1);
				PlaySoundEffect(who, 0x22);
				if(DestroyItem(thing))
					if(SkillCheck(who, the_attribute, skill, 10))
						CreateItemInContainer(who.backpack, ingot, amt);
					else
						SendSysMessage(who, "You destroy the item.");
					endif
				endif
			endif
			sleepms(500);
		endforeach
	else
		if(!craft_config[use_on.objtype])
			return 0;
		endif
		ingot := -1;
		var skill := craft_config[use_on.objtype].skill;
		var amt := CInt(craft_config[use_on.objtype].material);
		if((!skill) || (!amt))
			return 0;
		endif
		skill := CInt(skill/2);
		if(skill < 5)
			skill := 5;
		endif
		amt := CInt((amt * AP_GetSkill(who, the_attribute) * 75 ) / 10000);
		if(amt < 1)
			amt := 1;
		endif
		var mattype := CInt(GetObjProperty(use_on, "material"));
		if(mattype > 0)
			if(mattype != error)
				ingot := mattype;
			endif
		endif
		if(ingot == -1)
			foreach entry in cfgarray
				if(lower(resource_config[entry].Type) == "metal")
					if(CInt(resource_config[entry].Color) == CInt(use_on.color))
						ingot := entry;
					endif
				endif
			endforeach
		endif
		if(ingot == -1)
			SendSysMessage(who, "You have no idea how to smelt this strange ore!");
			return 0;
		endif
		PlaySoundEffect(who, 0x2c);
		sleep(1);
		if( DestroyItem(use_on) )
			PlaySoundEffect(who, 0x22);
			if(SkillCheck(who, the_attribute, skill, 10))
				CreateItemInContainer(who.backpack, ingot, amt);
			else
				SendSysMessage(who, "You destroy the item.");
			endif
		else
			SendSysMessage(who, "Couldn't destroy "+ use_on.name);
		endif
	endif

endfunction
