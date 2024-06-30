var cfg := ReadConfigFile("alchemy");

include "include/attributes";

function is_reagent(what)
	case(what.objtype)
		0xf7a:  return 1;
		0xf7b:  return 1;
		0xf84:  return 1;
		0xf85:  return 1;
		0xf86:  return 1;
		0xf88:  return 1;
		0xf8c:  return 1;
		0xf8d:  return 1;
		0xef1a: return 1;
		0xef1b: return 1;
		0xef1c: return 1;
		0xef1d: return 1;
		0xf7d: return 1;
		0xef1e: return 1;
		0xf87: return 1;
		0xf79: return 1;
		0xf91: return 1;
		0xf89: return 1;
		0xf82: return 1;
		0xF81: return 1;
		0xF78: return 1;
		0xF8F: return 1;
    
		0xee25: return 1;
		0xee26: return 1;
		0xee27: return 1;
		0xee28: return 1;
		0xfc5:  return 1;
		0x0dea: return 1;

		0x1F91: return 1;
	endcase
	return 0;
endfunction

function GetReagName(item)
  if (item.objtype ==  0xf7a)
     return "perola negra";
  elseif (item.objtype == 0xf7b)
    return "musgo do sangue";
  elseif (item.objtype == 0xf84)
    return "alho";
  elseif (item.objtype == 0xf85)
    return "ginseng";
  elseif ( item.objtype ==  0xf86)
    return "mandrake";
  elseif ( item.objtype == 0xf88)
    return "erva da noite";
  elseif ( item.objtype == 0xf8c)
    return "enxofre";
  elseif (item.objtype == 0xf8d)
    return "teia";
  elseif ( item.objtype == 0xef1a)
   return "ferrao de wyvern";
  elseif (item.objtype == 0xef1b)
   return "ferrao de escorpiao";
  elseif (item.objtype == 0xef1c)
   return "glandula de aranha";
  elseif (item.objtype == 0xef1d)
   return "glandula de cobra";
  elseif (item.objtype == 0xF7D)
   return "sangue fresco";
  elseif (item.objtype == 0xef1e)
   return "cranio de liche";
  elseif (item.objtype == 0xf87 )
   return "olho de beholder";
  elseif (item.objtype == 0xf79 )
   return "blackmoor";
  elseif (item.objtype == 0xf91 )
   return "coracao de wyrm";
  elseif (item.objtype == 0xf89 )
   return "obsidiana";
  elseif (item.objtype == 0xf82 )
   return "sangue de dragao";
  elseif (item.objtype == 0xF81 )
   return "terra fertil";
  elseif (item.objtype == 0xF78 )
   return "asa de morcego";
  elseif (item.objtype == 0xF8F )
   return "po de tumulo";
   
  elseif (item.objtype == 0xee27 )
   return "cascas vermelhas";
  elseif (item.objtype == 0xee28 )
   return "cascas verdes";
  elseif (item.objtype == 0xee26 )
   return "cascas marrom";
  elseif (item.objtype == 0xee25 )
   return "cascas amarelas";
  elseif (item.objtype == 0xfc5 )
   return "conchas";
  elseif (item.objtype == 0x0dea )
   return "cinzas";
  elseif (item.objtype == 0x1F91 )
   return "copo de agua";

  endif
endfunction

function GetReagObj(item)
  if (item == "perola negra" )
     return 0xf7a;
  elseif (item == "musgo do sangue" )
    return 0xf7b;
  elseif (item == "alho")
    return 0xf84;
  elseif (item == "ginseng")
    return 0xf85;
  elseif ( item == "mandrake" )
    return 0xf86;
  elseif ( item == "erva da noite")
    return 0xf88;
  elseif ( item == "enxofre")
    return 0xf8c;
  elseif (item == "teia")
    return 0xf8d;
  elseif ( item == "ferrao de wyvern")
   return 0xef1a;
  elseif (item == "ferrao de escorpiao")
   return 0xef1b;
  elseif (item ==  "glandula de aranha")
   return 0xef1c;
  elseif (item == "glandula de cobra")
   return 0xef1d;
  elseif (item.objtype == "sangue fresco")
   return 0xF7D;
  elseif (item == "cranio de liche")
   return 0xef1e;
  elseif (item == "olho de beholder" )
   return 0xf87;
  elseif (item == "blackmoor" )
   return 0xf79;
  elseif (item == "coracao de wyrm" )
   return 0xf91;
  elseif (item == "obsidiana")
   return 0xf89;
  elseif (item == "sangue de dragao")
   return 0xf82;
  elseif (item == "terra fertil" )
   return 0xF81;
  elseif (item == "asa de morcego")
   return 0xF78;
  elseif (item == "po de tumulo" )
   return 0xF8F;
   
  elseif (item.objtype == "cascas vermelhas" )
   return 0xee27;
  elseif (item.objtype == "cascas verdes" )
   return 0xee28;
  elseif (item.objtype == "cascas marrom" )
   return 0xee26;
  elseif (item.objtype == "cascas amarelas" )
   return 0xee25;
  elseif (item.objtype == "conchas" )
   return 0xfc5;
  elseif (item.objtype == "cinzas" )
   return 0x0dea;

  elseif (item.objtype == "copo de agua" )
   return 0x1F91;
  endif
endfunction

function PourPotion(user,mortar)
  try_to_make_potion(user, "spacefill", 1, mortar, 0);
endfunction

function make_the_potion(user, mortar)
  var use_on := Target( user );
  if(!use_on)
    SendSysMessage(user, "Targetting Cancelled");
    return;
  endif
  if(!can_access(user, use_on))
    return;
  endif
  if(is_reagent(use_on))
    var cfg := ReadConfigFile("alchemy");
    if(!cfg)
      return;
    endif
    var elem := cfg[use_on.objtype];
    if(!cfg)
      return;
    endif
    var possible := array;
    var tmenu := CreateMenu("Select a potion to make");
    var num_possibilities := 0;
    var selections := array;
    possible := GetConfigStringArray( elem, "potion");
    foreach thingie in possible
      if(can_make(user, thingie, use_on))
        var titem := cfg[thingie].objtype;
        if(titem)
          thingie["_"] := " ";
          AddMenuItem( tmenu, titem, thingie );
          num_possibilities := num_possibilities + 1;
        endif
      endif
    endforeach
    if(num_possibilities == 0)
      SendSysMessage(user, "You can't make any potions!");
      return;
    endif
    var citem := SelectMenuItem2(user, tmenu);
    if(!citem.objtype)
      SendSysMessage(user, "Targetting canceled");
      return -1;
    endif
    try_to_make_potion(user, possible[citem.index], 0, mortar, use_on);
  elseif(use_on == mortar)
    var last := GetObjProperty(mortar, "LastMade");
    if(last)
  	  try_to_make_last(user, last, mortar);
  	else
  	  SendSysMessage(user, "You cannot grind that.");
  	  return;
  	endif
  else
    SendSysMessage(user, "That's not a reagent!");
  endif
endfunction

function can_make(user, potion, reg)
  var skillneeded:= CInt(cfg[potion].skill);
  if(skillneeded > 20)
    skillneeded := skillneeded - 20;
  else
    skillneeded := 0;
  endif
  if(GetEffectiveSkill(user, SKILLID_ALCHEMY) >= skillneeded)
    return 1;
  else
    return 0;
  endif
endfunction

function take_regs(user, potion, reg)
  var amount_needed:= cfg[potion].reagent_amount;
  set_critical(1);
  if(CInt(amount_needed) <= reg.amount)
    SubtractAmount(reg, amount_needed);
    set_critical(0);
    return 1;
  else
    SendSysMessage(user, "You ran out of reagents!");
    set_critical(0);
    return 0;
  endif
endfunction

function try_to_make_potion(user, potion, in_mortar, mortar, reg)
  if(!in_mortar)
    potion[" "] := "_";
    var difficulty := cfg[potion].skill;
    var points := cfg[potion].points;
    var product := cfg[potion].objtype;
    var cntr;
    SetObjProperty(mortar, "LastMade", potion);
    var loop := 0;
    while(loop < 3)
      PlaySoundEffect(user, SOUND_EFFECT_GRIND);
      PrintTextAbovePrivate(user, "You grind the reagent",user);
      sleep(1);
      loop := loop + 1;
    endwhile
    if(!take_regs(user, potion, reg))
      return;
    endif
    if(CheckSkill(user, SKILLID_ALCHEMY, difficulty, points))
      var hasbottle:= 0;
      set_critical(1);
      foreach item in EnumerateItemsInContainer(user.backpack)
        if((item.objtype == UOBJECT_EMPTY_BOTTLE) && (item.container.objtype != 0x1940))
          if(SubtractAmount(item, 1))
            hasbottle := 1;
            break;
          endif
        endif
      endforeach
      set_critical(0);
      if(RandomInt(10) > 6)
        var skill := GetEffectiveSkill(user, SKILLID_ALCHEMY);
        ExceptionalFameMod(user, skill, difficulty, Cint(points / 4));
      endif
      if(hasbottle)
        var thing := CreateItemInBackpack(user, product, 1);
        SetObjProperty(thing, "pocao", product);
        PlaySoundEffect(user, SOUND_EFFECT_POUR);
        PrintTextAbovePrivate(user, "You create the potion and put it in your backpack", user);
        return;
      else
        PrintTextAbovePrivate(user, "You dont have any bottles.", user);
        SetObjProperty(mortar, "contents", product);
        return;
      endif
    else
      PlaySoundEffect(user, SOUND_EFFECT_POUR);
      PrintTextAbovePrivate(user, "You pour out the useless mixture", user);
      return;
    endif
  else
    var hasbottle:= 0;
    set_critical(1);
    foreach item in EnumerateItemsInContainer(user.backpack)
      if(item.objtype == UOBJECT_EMPTY_BOTTLE)
        SubtractAmount(item, 1);
        hasbottle := 1;
        break;
      endif
    endforeach
    set_critical(0);
    if(hasbottle)
      var product := GetObjProperty(mortar, "contents");
      var thing := CreateItemInBackpack(user, product, 1);
      SetObjProperty(thing, "pocao", product);
      PlaySoundEffect(user, SOUND_EFFECT_POUR);
      PrintTextAbovePrivate(user, "You create the item and put it in your backpack", user);
      EraseObjProperty( mortar, "contents" );
      return;
    else
      PrintTextAbovePrivate(user, "You dont have any bottles.", user);
      return;
    endif
  endif
endfunction

function try_to_make_last(user, potion, mortar)
  var reg;
  case(potion)
    "Refresh":            reg := FindRegs(user, 0xf7a);
    "Total_Refresh":      reg := FindRegs(user, 0xf7a);
    "Agility":            reg := FindRegs(user, 0xf7b);
    "Greater_Agility":    reg := FindRegs(user, 0xf7b);
    "Lesser_Cure":        reg := FindRegs(user, 0xf84);
    "Cure":               reg := FindRegs(user, 0xf84);
    "Greater_Cure":       reg := FindRegs(user, 0xf84);
    "Lesser_Heal":        reg := FindRegs(user, 0xf85);
    "Heal":               reg := FindRegs(user, 0xf85);
    "Greater_Heal":       reg := FindRegs(user, 0xf85);
    "Strength":           reg := FindRegs(user, 0xf86);
    "Greater_Strength":   reg := FindRegs(user, 0xf86);
    "Lesser_Poison":      reg := FindRegs(user, 0xf88);
    "Poison":             reg := FindRegs(user, 0xf88);
    "Greater_Poison":     reg := FindRegs(user, 0xf88);
    "Deadly_Poison":      reg := FindRegs(user, 0xf88);
    "Lesser_Explosion":   reg := FindRegs(user, 0xf8c);
    "Explosion":          reg := FindRegs(user, 0xf8c);
    "Greater_Explosion":  reg := FindRegs(user, 0xf8c);
    "Nightsight":         reg := FindRegs(user, 0xf8d);
    "TintaVermelha":      reg := FindRegs(user, 0xee27);
    "TintaVerde":         reg := FindRegs(user, 0xee28);
    "TintaMarrom":        reg := FindRegs(user, 0xee26);
    "TintaBege":          reg := FindRegs(user, 0xee25);
    "TintaAzul":          reg := FindRegs(user, 0xfc5);
    "TintaPreta":         reg := FindRegs(user, 0x0dea);
  endcase
  if(reg == "none")
    SendSysMessage(user, "You do not have enough reagents to make that.");
    return;
  endif
  potion[" "] := "_";
  var difficulty := cfg[potion].skill;
  var points := cfg[potion].points;
  var product := cfg[potion].objtype;
  var cntr;
    if(!take_regs(user, potion, reg))
    SendSysMessage(user, "You do not have enough reagents to make that.");
    return;
  endif
  PlaySoundEffect(user, SOUND_EFFECT_GRIND);
  PrintTextAbovePrivate(user, "You grind the reagent",user);
  sleep(2);
  PlaySoundEffect(user, SOUND_EFFECT_GRIND);
  PrintTextAbovePrivate(user, "You grind the reagent",user);
  sleep(2);
  PlaySoundEffect(user, SOUND_EFFECT_GRIND);
  PrintTextAbovePrivate(user, "You grind the reagent",user);
  sleep(2);
  if(CheckSkill(user, SKILLID_ALCHEMY, difficulty, points))
    var hasbottle:= 0;
    set_critical(1);
    foreach item in EnumerateItemsInContainer(user.backpack)
      if(item.objtype == UOBJECT_EMPTY_BOTTLE)
        if(SubtractAmount(item, 1))
          hasbottle := 1;
          break;
        endif
      endif
    endforeach
    set_critical(0);
    if(hasbottle)
      var thing := CreateItemInBackpack(user, product, 1);
      SetObjProperty(thing, "pocao", product);
      PlaySoundEffect(user, SOUND_EFFECT_POUR);
      SetObjProperty(mortar, "LastMade", potion);
      PrintTextAbovePrivate(user, "You create the potion and put it in your backpack", user);
      return;
    else
      PrintTextAbovePrivate(user, "You dont have any bottles.", user);
      SetObjProperty(mortar, "contents", product);
      return;
    endif
  else
    PlaySoundEffect(user, SOUND_EFFECT_POUR);
    PrintTextAbovePrivate(user, "You pour out the useless mixture", user);
    return;
  endif
endfunction

function FindRegs(who, type)
  var holder := "none";
  foreach thing in EnumerateItemsInContainer(who.backpack)
    if(thing.objtype == type)
      holder := thing;
      break;
    endif
  endforeach
  return holder;
endfunction
