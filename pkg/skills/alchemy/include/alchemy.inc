use cfgfile;

include ":timedScripts:timedScripts";
include ":tn:cooldown";
include "include/say";
include ":fls_core:fls_characters";
include ":charactercreation:resists";
include "include/epicConstants";
include ":wrappers:Objects/Model/MobileWrapper";
include ":wrappers:Objects/Model/WeaponWrapper";
include ":attributes:attributes";
include ":abilities:models/Ability";
include ":crafting:/include/caracteristicasFabricacao";
include ":crafting:/include/fabricacaoUtil";

CONST DRINK_SOUND := 0x32;
CONST GATILHO_PAVIO := 11;       // legacy single 3s fuse (back-compat; treated as Pavio Normal)
CONST GATILHO_IMPACTO := 12;     // legacy impact trigger (removed from configurator; treated as Pavio Normal)
CONST GATILHO_MINA := 13;
CONST GATILHO_PAVIO_CURTO := 14;  // 2s
CONST GATILHO_PAVIO_NORMAL := 15; // 3s
CONST GATILHO_PAVIO_LONGO := 16;  // 5s

CONST ELEMENTO_FOGO := 21;
CONST ELEMENTO_ENERGIA := 22;
CONST ELEMENTO_GELO := 23;

CONST INVOLUCRO_BASE := 31;
CONST INVOLUCRO_FRAGMENTACAO := 32;
CONST INVOLUCRO_CONCUSSAO := 33;

CONST CARGA_BALANCEADA := 41;
CONST CARGA_CONCENTRADA := 42;
CONST CARGA_DISPERSA := 43;

CONST REFORCO_COMUM := 51;
CONST REFORCO_CARGA_DUPLA := 52;
CONST REFORCO_POTENCIA_ELEM := 53;

// Zona de Pressao (L3) — tipo pre-comprometido no momento da ativacao
CONST ZONA_TIPO_PRESSAO  := 61;  // +50% raio de explosao
CONST ZONA_TIPO_LENTIDAO := 62;  // vitimas slowed por Science/10s
CONST ZONA_TIPO_PRECISAO := 63;  // +50% dano

// Facade de mensagem sobre MobileWrapper.HasFreeHand(). A regra real
// (ocupacao de layer + Two_Handed/Dual_Wield) vive no wrapper; aqui so
// emitimos a mensagem de pocao. Toda pocao consumida passa por aqui.
function hasFreeHandToDrink(character)
  if (!MobileWrapper(character).HasFreeHand())
    SendSysMessage(character, "Voce precisa de pelo menos uma mao livre!");
    return 0;
  endif
  return 1;
endfunction

function checkifcandrink(character, potion)
  if (!hasFreeHandToDrink(character))
    return 0;
  endif
  var cfg := ReadConfigFile(":*:itemdesc");
  var type := cfg[potion.objtype].Type;
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

function empty_bottle(character, potion, drinkSound:= 1, silent:= 0)

  if (drinkSound)
    PlaySoundEffect(character, DRINK_SOUND);
  endif
  if (!silent)
    PrintTextAbove(character, "*usa uma pocao*");
  endif
  var doses;
  var objtype := GetObjType(potion);
  
	if (!GetObjProperty(potion, "doses"))
		doses := 1;
	else
		doses := cint(GetObjProperty(potion, "doses"));
	endif
  
  if (doses > 1)
		doses := doses - 1;
    if (potion.amount > 1)
      var props := getobjpropertynames(potion);
      var pot := CreateItemInBackpack(character, objtype, 1);
      foreach key in props
        var prop_value := getobjproperty(potion, key);
        SetObjProperty(pot, key, prop_value);
  	  endforeach
      SetObjProperty(pot, "doses", doses);
      pot.color := potion.color;
      sleepms(100);
      subtractamount(potion, 1);
    else
      setobjproperty(potion, "doses", doses);
    endif
  else
    var props := PegaComponenteFabricacao(potion, "doses");
    if(!props)
      props := PegaComponenteFabricacao(potion, "vazio");
    endif

    SubtractAmount( potion, 1 );
		var pot := CreateItemInBackpack(character, 0x0F0E, 1);
    setobjproperty(pot, "encaixeComponente", "frasco");
    setobjproperty(pot, "material", 7139);
    setobjproperty(pot, "tierComponente", props.tier.id ? props.tier.id : 1);
    pot.color := 1328;
	  return 1;
	endif
  IncRevision(potion);

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
		0x6993: //BARRA DE PRATA
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
    0x6993: return "barras de prata";
    0x630e: return "barras de ouro";

    ///// GRAVETOS NUMAJA /////
    0xee29: return "gravetos preto metálico";
    0xee2a: return "gravetos verde musgo metálico";
    0xee2b: return "gravetos dourado";
    0xee2c: return "gravetos azul metálico";
    0xee2d: return "gravetos azul lunar";
    0xee2e: return "gravetos azul penumbra";
    
    //pizza dentro do EvilLink
    0xc020: return "graveto teixo";
    0xEE62: return "graveto imbuia";
    0xc026: return "graveto mogno";
    0xEE60: return "graveto cogumelo";
    0xEE61: return "graveto ebano";

    ///// OTHERS /////
    0x1f9e: return "Jarra de Água";
    0x1852: return "Óleo Exótico";
  endcase

endfunction


function is_poison(potion)
  var fabricado := getobjproperty(potion, "fabricado");
  var poison_level := fabricado.classe;
  return poison_level;
endfunction

function do_poison(who, potion)
  var level, difficulty, points, poisonType;
  var skill := Cint(GetObjProperty(potion, "crafterskill"));
  var bonus := Cint(GetObjProperty(potion, "bonus"));
  var tier := peganivelpocao(who, potion);
  level := skill;
  
  //se for uma poçao que nao a default a formula eh essa:
  var difficulty_mod := bonus;
  points := difficulty;
  case(potion.objtype)
    0xdc04: case(tier)
              1: difficulty := RESIST_EASY; points := 50; poisonType := "alchPoison";
              2: difficulty := RESIST_MEDIUM; points := 40; poisonType := "alchPoison";
              3: difficulty := RESIST_HARD; points := 60; poisonType := "alchPoison";
            endcase
    0xbf01: poisonType :="strPoison"; difficulty := RESIST_MEDIUM; points := 70; level := cint(skill/(2.5/tier));
    0xbf20: poisonType :="dexPoison"; difficulty := RESIST_MEDIUM; points := 70; level := cint(skill/(2.5/tier));
    0xef54: poisonType :="intPoison"; difficulty := RESIST_MEDIUM; points := 70; level := cint(skill/(2.5/tier));
    0xbf02: poisonType:="staminaPoison"; difficulty := RESIST_MEDIUM; points := 70; level := level/20;
    0xbf03: poisonType:="paralyzePoison"; difficulty := RESIST_HARD; points := 100;
    0xbf06: difficulty := RESIST_HARD; points := 70; poisonType :="manaPoison"; level := level/20;
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
    if (MobileWrapper(who).HasAbility(ABILITY_ID::VENENO_PURO))
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

		if (MobileWrapper(who).HasAbility(ABILITY_ID::CAMPO_MINADO) && science == BOMBA);
			bonus := Cint(bonus * spec);
		elseif (MobileWrapper(who).HasAbility(ABILITY_ID::VENENO_PURO) && science == VENENO);
			bonus := Cint(bonus * spec);
		elseif (MobileWrapper(who).HasAbility(ABILITY_ID::SINTESE_VELOZ) && science == BUFF);
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

Function AplicaVicio (who, potion, Atr)
  var tier := peganivelpocao(who, potion);
  var bonus := 0;
  var vicio := 0;
  var att := 0;
  var ult := 0;

  if (Atr == STRENGTH)
    vicio := AP_GetVital(who, "viciostr");
    att := "viciostr";
    ult := "ultimastr";
  elseif (Atr == DEXTERITY)
    vicio := AP_GetVital(who, "viciodex");
    att := "viciodex";
    ult := "ultimadex";
  elseif (atr == INTELLIGENCE)
    vicio := AP_GetVital(who, "vicioint");
    att := "vicioint";
    ult := "ultimaint";
  endif
  
  var dmg := epicPotionDamageRoll (tier, bonus);
  If(dmg < 0)
    dmg := 0;
  endif
  dmg := TesteVicio(who, potion, dmg);
  vicio := vicio-dmg;
  if (vicio < 0)
    vicio := 0;
  endif
  var ultimouso := readgameclock();
  AP_SetVital(who, att, vicio);
  SetObjProperty(who, ult, ultimouso);

Endfunction

function TesteVicio(who, potion, dmg)
  var tier := PegaNivelPocao(who, potion);
  var pureza := getobjproperty(potion, "bonus");
  if (MobileWrapper(who).HasAbility(ABILITY_ID::SINTESE_VELOZ))
      pureza := pureza * -1;
  endif
  var dif := 0;
  case (tier)
    1: dif := RESIST_EASY;
    2: dif := RESIST_MEDIUM;
    3: dif := RESIST_VERY_HARD;
    4: dif := RESIST_ALMOST_IMPOSSIBLE;
  endcase
  dif -= pureza;
  if (CheckResist(who, FORTITUDE, dif))
      dmg := 0;
      SendSysMessageEx(who, "Seu corpo absorve bem a substância.", SSM_INFO);
  else
      SendSysMessageEx(who, "Seu corpo cria tolerância à substância.", SSM_INFO);
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

function PegaNivelPocao(who, pocao)
	var fabricado := GetObjProperty(pocao, "fabricado");
	var tier := fabricado.classe;
  if(getobjProperty(who, "#powerpot"))
    if((getobjproperty(who, "#power_time"))+15 > readgameclock())
      tier := tier+1;
    else
      SendSysMessageEx(who, "Você demorou demais para usar sua poção.", SSM_FAIL);
    endif
  endif
  eraseobjproperty(who, "#powerpot");
  eraseobjproperty(who, "#power_time");

  return tier;
endfunction

function Pushback_alch(mobile, cx, cy, cz, realm, raio, forca, passos, delay_ms)
        var dx := mobile.x - cx;
        var dy := mobile.y - cy;

        if (dx == 0 && dy == 0)
            dx := 1;
        endif

        var dist := Sqrt(dx * dx + dy * dy);
        var nx := dx / dist;
        var ny := dy / dist;

        // Dispara um script separado para cada mobile
        // para que todos se movam simultaneamente
        Start_Script(":alchemy:pushback", array{ mobile, nx, ny, forca, passos, delay_ms, realm, cz });

endfunction