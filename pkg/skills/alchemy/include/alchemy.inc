use cfgfile;

include ":timedScripts:timedScripts";
include ":tn:cooldown";
include "include/say";
include ":fls_core:fls_characters";
include ":charactercreation:resists";
include "include/epicConstants";

CONST DRINK_SOUND := 0x32;

function checkifcandrink(character, type)
  // Check to make sure they have at least one hand free to drink the potion
  var equippedright := GetEquipmentByLayer(character, 0x01);
  var equippedleft := GetEquipmentByLayer(character, 0x02);
  var cfg := ReadConfigFile(":*:itemdesc");
  var twohanded := Cint(cfg[equippedleft.objtype].Two_Handed);
  if (equippedright and equippedleft || twohanded)
    SendSysMessage(character, "Voce precisa de pelo menos uma mao livre!");
    return 0;
  endif
  if((type != "red") and (type != "green_potion"))
    if(GetCooldown(character, type))
        SendSysMessage(character, "Voce deve esperar um pouco antes de beber outra pocao!");
        return 0;
     else
        SetCooldown(character, type, 10);
        return 1;
     endif
  else
    return 1;
  endif
endfunction

function empty_bottle(character, potion, potiontype := 0, drinkSound:= 1)

  if (drinkSound)
    PlaySoundEffect(character, DRINK_SOUND);
  endif
  PrintText(character, "*usa uma pocao*");
  var goles;
  var objtype := GetObjType(potion);
  
	if (!GetObjProperty(potion, "goles"))
		goles := cint(GetObjProperty(potion, "crafterskill") / 20);
		SetObjProperty(potion, "goles", goles);
	else
		goles := cint(GetObjProperty(potion, "goles"));
	endif

	if(potiontype == 1)  //eh uma poção de 1 so gole
		goles := 0;
	endif

  if (goles > 1)
		goles := goles - 1;
    var props := getobjpropertynames(potion);
    var pot2 := SubtractAmount( potion, 1 );
    var pot := CreateItemInBackpack(character, objtype, 1);
    foreach key in props
      var prop_value := getobjproperty(potion, key);
      SetObjProperty(pot, key, prop_value);
  	endforeach
    SetObjProperty(pot, "goles", goles);
  else
    var pot2 := SubtractAmount( potion, 1 );
	  var stacknow := GetAmount(0x0F0E);
		if (stacknow > 0)
			Addamount(0x0F0E, 1);
		else
			var pot := CreateItemInBackpack(character, 0x0F0E, 1);
		endif
	return 1;
	endif

endfunction

function isotherreagent(what) //essa funcao so se mantem pra checar o que não é reagent do próprio UO. se nao quiser checar isso use .isReagent();
	case(Lower(what.objtype))
    ///// GRAVETOS /////
		0xf7a:  
		0xf7b:  
		0xf84:  
		0xf85:  
		0xf86:  
		0xf88:  
		0xf8c:  
		0xf8d:  
		0xef1a: 
		0xef1b: 
		0xef1c: 
		0xef1d: 
		0xf7d: 
		0xef1e: 
		0xf87: 
		0xf79: 
		0xf91: 
		0xf89: 
		0xf82: 
		0xf81: 
		0xf78: 
		0xf8f: 
		0xf90: 

		0xee25: 
		0xee26: 
		0xee27: 
		0xee28: 
		0xfc5:  
		0x0dea: 

    0xee29: 
    0xee2a: 
    0xee2b: 
    0xee2c: 
    0xee2d: 
    0xee2e: 

		0x1f91: //COPO D'ÁGUA
    0x1f9e: //JARRA D'ÁGUA

    ///// INGOTS /////
		0x6993: //BARRA DE PRATA LUNAR
		0x630e: //BARRA DE OURO
      return 1;
	endcase
	return 0;
endfunction

function GetReagName(objtype)

  if (objtype ==  0xf7a)
    return "perola negra";
  elseif (objtype == 0xF78 )
   return "asa de morcego";
  elseif (objtype == 0xf79 )
   return "pedra negra";
  elseif (objtype == 0xf7b )
    return "musgo vermelho";
  elseif (objtype == 0xf7c )
    return "cria do sangue";
  elseif (objtype == 0xF7D)
   return "sangue fresco";
  elseif (objtype == 0xF7E)
   return "ossos";
  elseif (objtype == 0xF7F)
   return "enxofre";
  elseif (objtype == 0xF80)
   return "ossos demoniacos";
  elseif (objtype == 0xF81 )
   return "terra fertil";
  elseif (objtype == 0xf82 )
   return "sangue de dragao";
  elseif (objtype == 0xf83 )
   return "cogumelo negro";
  elseif (objtype == 0xf84)
    return "alho";
  elseif (objtype == 0xf85)
    return "ginseng";
  elseif ( objtype ==  0xf86)
    return "raiz de mandragora";
  elseif (objtype == 0xf87 )
   return "olho da salamandra";
  elseif ( objtype == 0xf88)
    return "solanaceae";
  elseif (objtype == 0xf89 )
   return "obsidiana";
  elseif ( objtype == 0xf8c)
    return "cinzas de enxofre";
  elseif (objtype == 0xf8d)
    return "teia";
  elseif (objtype == 0xf8a)
    return "gusa";
  elseif (objtype == 0xf8b)
    return "pedra pomes";
  elseif (objtype == 0xf8e)
    return "escamas de cobra";
  elseif (objtype == 0xf8f)
    return "cinzas vulcanicas";
  elseif (objtype == 0xF90 )
   return "madeira morta";
  elseif (objtype == 0xf91 )
   return "coracao do verme";

  elseif ( objtype == 0xef1a)
   return "glandula de wyvern";
  elseif (objtype == 0xef1b)
   return "glandula de escorpiao";
  elseif (objtype == 0xef1c)
   return "glandula de aranha";
  elseif (objtype == 0xef1d)
   return "glandula de cobra";
  elseif (objtype == 0xF8F)
   return "po de tumulo";
  elseif (objtype == 0xee27 )
   return "cascas vermelhas";
  elseif (objtype == 0xee28 )
   return "cascas verdes";
  elseif (objtype == 0xee26 )
   return "cascas marrom";
  elseif (objtype == 0xee25 )
   return "cascas amarelas";
  elseif (objtype == 0xfc5 )
   return "conchas";
  elseif (objtype == 0x0dea )
   return "cinzas";
  elseif (objtype == 0x1F91 )
   return "copo de agua";
  endif

  case(Lower(objtype)) //EvilLink
    ///// INGOTS /////
    0x6993: return "barras de prata lunar";
    0x630e: return "barras de ouro";

    ///// GRAVETOS NUMAJA /////
    0xee29: return "gravetos numaja preto metálico";
    0xee2a: return "gravetos numaja verde musgo metálico";
    0xee2b: return "gravetos numaja dourado";
    0xee2c: return "gravetos numaja azul metálico";
    0xee2d: return "gravetos numaja azul lunar";
    0xee2e: return "gravetos numaja azul penumbra";
    
    //pizza dentro do EvilLink
    0xc020: return "graveto teixo";
    0xc023: return "graveto imbuia";
    0xc026: return "graveto mogno";
    0xc02c: return "graveto cogumelo";
    0xc02f: return "graveto ebano";

 

    ///// OTHERS /////
    0x1f9e: return "Jarra de Água";
    0x1852: return "Óleo Exótico";
  endcase

endfunction


function is_poison(potion)
  var poison_level := GetObjProperty(potion, "tier");
  return poison_level;
endfunction

function do_poison(who, potion)
  var level, difficulty, points, poisonType;
  var skill := Cint(GetObjProperty(potion, "crafterskill"));
  var bonus := Cint(GetObjProperty(potion, "bonus"));
  level := skill;
  var tier := getobjProperty (potion, "tier");

  //se for uma poçao que nao a default a formula eh essa:
  var difficulty_mod := bonus;
  points := difficulty;
  case(potion.objtype)
    0xdc04: difficulty := RESIST_EASY; points := 50; poisonType := "alchPoison";
    0xbf01: poisonType :="strPoison"; difficulty := RESIST_MEDIUM; points := 70;
    0xbf20: poisonType :="dexPoison"; difficulty := RESIST_MEDIUM; points := 70;
    0xef54: poisonType :="intPoison"; difficulty := RESIST_MEDIUM; points := 70;
    0xbf02: poisonType:="staminaPoison"; difficulty := RESIST_MEDIUM; points := 70;
    0xbf03: poisonType:="paralyzePoison"; difficulty := RESIST_HARD; points := 100;
    0xbf06: difficulty := RESIST_HARD; points := 70; poisonType :="manaPoison";
    0xbf04: difficulty := RESIST_HARD; points := 60; poisonType := "alchPoison";
    0x6072: difficulty := RESIST_MEDIUM; points := 40; poisonType := "alchPoison";
    0xbf05: difficulty := RESIST_VERY_HARD; points := 100; poisonType := "alchPoison";
    default: difficulty := 0; points := 0; poisonType := "invalid";
  endcase
  PlaySoundEffect(who, 0x248);
    SendSysMessageEx(who, "Voce aplicou o veneno com sucesso.", SSM_INFO);
    var poisonInfo := struct;
    poisonInfo.+name := poisonType;
    poisonInfo.+level := level;
    poisonInfo.+dosage := 20 - (tier*2);
    poisonInfo.+acc := who.acctname;
    poisonInfo.+time := 20 + level;
    poisonInfo.+difficulty_mod := difficulty_mod;
    if (temHabilidade(who, "Veneno Puro"))
      poisonInfo.dosage := CInt(poisonInfo.dosage * 1.5);
      poisonInfo.time := CInt(poisonInfo.time * 1.25);
    endif

    if (CheckResist(who, FORTITUDE, difficulty, -1*poisonInfo.difficulty_mod))
      SendSysMessageEx(who, "Voce resistiu ao veneno.", SSM_INFO);
    else
      if (TS_GetTimerInfo(who, "defaultPoison"))
        var defaultpoison := cint(TS_GetTimerLevel(who, "defaultPoison"));
        setobjproperty(who, "#poisontier", defaultpoison);
      endif
      if (getobjProperty(who, "#poisontier"))
        var poison := ts_gettimernames(who, type:="P");
        var teste := cint(getobjProperty(who, "#poisontier"));
        if (teste <= tier);
          TS_LowerDuration(who, poison, -1);
          TS_StartTimer(who, poisonInfo.Name, poisonInfo.time, poisonInfo.level, who);
        endif
      else
      setobjproperty(who, "#poisontier", tier);
      setobjproperty(who, "#poisonbonus", bonus);
      TS_StartTimer(who, poisonInfo.name, poisonInfo.time, poisonInfo.level, who);
      endif
    endif
endfunction

Function PotionSpecialistBonus (who, bonus, science)
	var spec := 2;

		if (TemHabilidade(who, "Bombardeiro Eficiente") && science == BOMBA);
			bonus := Cint(bonus * spec);
		elseif (TemHabilidade(who, "Veneno Puro") && science == VENENO);
			bonus := Cint(bonus * spec);
		elseif (TemHabilidade(who, "Impulso Metabolico") && science == BUFF);
			bonus := Cint(bonus * spec);
		endif

    return bonus;
endfunction

Function epicPotionDamageRoll(tier, bonus)
  var dice := cstr("1d4");
    if (tier == 2)
      dice := cstr("2d6");
    elseif (tier == 3)
      dice := cstr("3d8");
    elseif (tier == 4)
      dice := cstr("4d10");

    endif

  var dmg := RandomDiceRoll(dice);
  dmg := Cint(dmg + bonus);

  return dmg;
endfunction

Function AlchLabTransfer (item, alvo)
  SetObjProperty(item, "strength", cint(GetObjProperty(alvo, "strength")));
  SetObjProperty(item, "bonus", cint(GetObjProperty(alvo, "bonus")));
  SetObjProperty(item, "tier", cint(GetObjProperty(alvo, "tier")));
  SetObjProperty(item, "crafterskill", cint(GetObjProperty(alvo,"crafterskill")));
  SetObjProperty(item, "science", (GetObjProperty(alvo,"science")));
  
  EraseObjProperty(alvo, "extrato");
  EraseObjProperty(alvo, "strength");
  EraseObjProperty(alvo, "bonus");
  EraseObjProperty(alvo, "tier");
  EraseObjProperty(alvo, "crafterskill");
  EraseObjProperty(alvo, "science");
endfunction

function VicioSTR (who, potion)
  var tier := getobjProperty(potion,"tier");
  var vicio := AP_GetVital(who, "viciostr");
  var bonus := 0;
  
  if (temhabilidade(who, "Impulso Metabolico"))
      bonus := (getobjProperty(potion, "bonus")*-1);
  endif
  
  var dmg := epicPotionDamageRoll (tier, bonus);
  If(dmg < 0)
    dmg := 0;
  endif
  dmg := TesteVicio(who, tier, dmg);
  vicio := vicio-dmg;
  if (vicio < 0)
    vicio := 0;
  endif
  
  AP_SetVital(who,"viciostr", vicio);
 	
  var ultimouso := readgameclock();
	SetObjProperty(who, "ultimastr", ultimouso);
endfunction

function VicioDEX (who, potion)
  var tier := getobjProperty(potion,"tier");
  var vicio := AP_GetVital(who, "viciodex");
  var bonus := 0;
  
  if (temhabilidade(who, "Impulso Metabolico"))
      bonus := (getobjProperty(potion, "bonus")*-1);
  endif
  
  var dmg := epicPotionDamageRoll (tier, bonus);
  If(dmg < 0)
    dmg := 0;
  endif
  dmg := TesteVicio(who, tier, dmg);
  vicio := vicio-dmg;
  if (vicio < 0)
    vicio := 0;
  endif
  
  AP_SetVital(who,"viciodex", vicio);
 	
  var ultimouso := readgameclock();
	SetObjProperty(who, "ultimadex", ultimouso);
endfunction

function VicioINT (who, potion)
  var tier := getobjProperty(potion,"tier");
  var vicio := AP_GetVital(who, "vicioint");
  var bonus := 0;
  
  if (temhabilidade(who, "Impulso Metabolico"))
      bonus := (getobjProperty(potion, "bonus")*-1);
  endif
  
  var dmg := epicPotionDamageRoll (tier, bonus);
  If(dmg < 0)
    dmg := 0;
  endif
  dmg := TesteVicio(who, tier, dmg);
  vicio := vicio-dmg;
  if (vicio < 0)
    vicio := 0;
  endif
  
  AP_SetVital(who,"vicioint", vicio);
 	
  var ultimouso := readgameclock();
	SetObjProperty(who, "ultimaint", ultimouso);
endfunction

function TesteVicio(who, tier, dmg)
  if (tier == 1)
    if (CheckResist(who, FORTITUDE, RESIST_EASY))
      dmg := 0;
    endif
  elseif (tier == 2)
    if (CheckResist(who, FORTITUDE, RESIST_MEDIUM))
      dmg := 0;
    endif
  elseif (tier == 3)
    if (CheckResist(who, FORTITUDE, RESIST_VERY_HARD))
      dmg := 0;
    endif
  elseif (tier == 4)
    if (CheckResist(who, FORTITUDE, RESIST_ALMOST_IMPOSSIBLE))
      dmg := 0;
    endif
  endif
  return dmg;
endfunction

function WriteAlchInfo(byref recipe)
	var cfg := ReadConfigFile(":alchemy:config/recipes");
	var elem := FindConfigElem(cfg, recipe.recipe);

  recipe.+id := GetConfigInt(elem, "id");
	recipe.+name := GetConfigString( elem, "Name" );
  recipe.+tier := GetConfigint( elem, "tier" );
	recipe.+type := GetConfigString( elem, "Type" );
	recipe.+difficulty := GetConfigInt( elem, "skill" );
	recipe.+description := GetConfigString( elem, "desc" );
	recipe.+science := GetConfigString(elem, "science");
	recipe.+level := GetConfigInt( elem, "level" );
endfunction

function WriteDyeInfo(byref recipe)
	var cfg := ReadConfigFile(":alchemy:config/recipestintas");
	var elem := FindConfigElem(cfg, recipe.recipe);

  recipe.+id := GetConfigInt(elem, "id");
	recipe.+name := GetConfigString( elem, "Name" );
	recipe.+type := GetConfigString( elem, "Type" );
	recipe.+difficulty := GetConfigInt( elem, "skill" );
	recipe.+description := GetConfigString( elem, "desc" );
	recipe.+level := GetConfigInt( elem, "level" );
endfunction