
use uo;
use os;

include ":timedScripts:timedScripts";
include ":loot:include/lootParser";
include ":ghaia:ghaiaInit";
include ":tn:recipes";
include ":tn:cooldown";
include "include/facings";
include ":fls_core:packets";
include "include/client";
include "include/sounds";
include "include/shapes";
include "include/say";
include ":tn:npcutil";
include ":tn:cooldown";
include ":brainAI:npcUtil";
include ":awareness:awareness";
include ":tn:boss";
include ":combat:armorZones";
include ":combat:include/combat";
include ":charactercreation:resists";

// estados do npc, todos comecam no INIT, depois podem ir pro WANDER, apos algum tempo no wander se nao for interrompido eles vao pro SLEEP. do WANDER e do SLEEP podem ir pro FIGHT e desse pro VICTORY
enum STATES
 INITIALIZING := 1,
 WANDERING:= 2,
 FIGHTING := 3,
 SLEEPING := 4,
 VICTORY := 5,
 FLEEING := 6
endenum

//alinhamento pra determinar se o player é inimigo ou nao pra IA.
//outros inimigos pra IA é definido com a tag enemy
const ALIGN_EVIL := "evil";
const ALIGN_GOOD := "good";
const ALIGN_NEUTRAL := "neutral";

//novos eventos

//const EVID_ALERT_ALLIES := 0x00000013;

//essa funcao determina se o npc vai enfrentar o alvo ou nao
function CanFight(me, mobile, align)

  if (mobile.hidden )
  var dif := CalculateDhDiff(me, mobile);
  if ( SkillCheck(me, AWARENESS, dif) > 0 )
    mobile.hidden := 0;
    PrintText(mobile, "*aparece*");
    SendSysMessage(me, "Voce encontrou "+mobile.name+".");
    SendSysMessage(mobile, me.name+" te encontrou!");

    var last_loc := struct{ "x" := mobile.x, "z" := mobile.z, "y" := mobile.y };
    foreach monster in ListMobilesNearLocationEx(mobile.x, mobile.y, mobile.z, 12, LISTEX_FLAG_NORMAL|LISTEX_FLAG_HIDDEN|LISTEX_FLAG_NPC_ONLY)
      if (GetObjProperty(monster, "guild"))
        SendEvent(monster, struct{ "type" := 0x78460, source := mobile, "last_loc" := last_loc});
      endif
      sleepms(2);
    endforeach

    if ( GetObjProperty(me, "attackhiddenenemy" ))
      return 1;
    endif
  else
    return 0;
  endif
  elseif  ( mobile.concealed) // kra ta intocado
  return 0;
  elseif (mobile.cmdlevel && GetObjProperty(mobile, "ignorenpcs") == 1) //kra eh gm
        return 0;
  elseif ( GetCooldown(me, "peacemade") > 0 )
  //printtextabove(me, "nao");
  return 0;
  elseif ( GetObjProperty(mobile, "#fantasma") == 1)
  return 0;
    endif
  //printtextabove(me, "nao vou atacar " + mobile.name + " " + GetObjProperty(mobile, "#deathtime" ) );
  if ( Cint(GetCooldown(mobile, "deathtime" )) > 0  )
  //printtextabove(me, "nao vou atacar" + mobile.name);
  return 0;
  endif

  var mycategory := lower(getnpccategory( mytemplate ));
  var chardata := GetObjProperty(mobile, "chardata");
  var povo := chardata.povo;

  if (!mobile.npctemplate) //se for player olha a tendencia do npc
    if (align == ALIGN_EVIL) //npcs malignos atacam todos os players
      if (GetObjProperty(me, "AtacarNoite") && (GetGlobalProperty( "dia") != "noite")) //checa se s� � hostil de noite
        if(Distance(me, mobile) > 3) //se for de dia e o alvo n�o estiver a 3 tiles de distancia, eles n�o atacam
          return 0;
        endif
      endif
      if (povo == "Numajene" )
        if ( (mytemplate == "spiritbear") || (mytemplate == "spiritpanther") || (mytemplate == "spiritserpent")
          || (mytemplate == "spiritwolf") || (mytemplate == "spiritualbear") || (mytemplate == "phasespider")
          || (mytemplate == "eruva") || (mytemplate == "luine")
          || (mytemplate == "centaur") || (mytemplate == "dryad")|| (mytemplate == "fadinha") || (mytemplate == "najassur")
          || (mytemplate == "pytnas") || (mytemplate == "carniceiro") || (mytemplate == "dragonete") || (mytemplate == "criafuria"))
          return 0;
        endif
      endif

      if ( GetObjProperty(mobile, "kit") == "Bandido")
          if ( (mytemplate == "MercenarioRecruta") || (mytemplate == "MercenarioBesteiro") || (mytemplate == "Roguearcher") || (mytemplate == "Mercenario") || (mytemplate == "Rogue") || (mytemplate == "MercenarioLider") || (mytemplate == "MercenarioExperiente") )
            return 0;
          endif
      endif

      //printtextabove(me, "mytemplate " + mytemplate + " " + cint(GetObjProperty(mobile, "empatiacomanimais")));
      if ( (GetObjProperty(mobile, "kit") == "criado por lobos") || (cint(GetObjProperty(mobile, "empatiacomanimais")) == 1) || TemHabilidade(mobile, "Senso da Natureza" ) || TemHabilidade(mobile, "Campestre" ) )//verifica se tem empatia por animais
        if ( (mytemplate == "giantbat") || (mytemplate == "giantrat") || (mytemplate == "gianttoad")
        || (mytemplate == "giantserpent") || (mytemplate == "snake") || (mytemplate == "mongbat") || (mytemplate == "naja")
        || (mytemplate == "warg") || (mytemplate == "polarbear") || (mytemplate == "lynx") ||  (mytemplate == "cougar")
        ||  (mytemplate == "alligator")  || (mytemplate == "panther") || (mytemplate == "bull") || (mytemplate == "grizzlybear") || (mytemplate == "wolf") || (mytemplate == "sapop"))
            if (isBoss(mobile)) //verifica se � boss
                return 1;
            else
                return 0;
            endif
        endif
      elseif ( (mycategory == "undead") && (GetObjProperty(mobile, "aparenciacadaverica") == 1) )
          return 0;

      elseif (GetObjProperty(mobile, "unattack"))
        return 0;

      elseif ( propallies != error )
        foreach key in (propallies.keys())
          if ( GetObjProperty(mobile, key) == propallies[key] ) //eh meu chapa, entao nao ataco.
            return 0;
          endif
        endforeach
      endif

      return 1;
    else //npcs bons ou neutrals nao atacam
      return 0;
    endif
  else //se for um npc verifica se eh inimigo
    if (align == ALIGN_EVIL)
      if (mobile.master) //se for bicho tamado, é atacado!
        var povo_owner := mobile.master;
        var chardata := GetObjProperty(povo_owner,"chardata");
        if ( (GetObjProperty(mobile.master, "kit") == "criado por lobos") || (cint(GetObjProperty(mobile.master, "empatiacomanimais")) == 1) || TemHabilidade(mobile.master, "Senso da Natureza" ) || TemHabilidade(mobile.master, "Campestre") )
          if ( (mytemplate == "giantbat") || (mytemplate == "giantrat") || (mytemplate == "gianttoad")
              || (mytemplate == "giantserpent") || (mytemplate == "snake") || (mytemplate == "mongbat") || (mytemplate == "naja")
              || (mytemplate == "warg") || (mytemplate == "wolf") || (mytemplate == "polarbear") || (mytemplate == "lynx") ||  (mytemplate == "cougar")
              ||  (mytemplate == "alligator")  || (mytemplate == "panther") || (mytemplate == "grizzlybear") || (mytemplate == "sapop"))
            return 0;
          endif
        endif
        if  (chardata["povo"] == "Numajene" )
          if ( (mytemplate == "spiritbear") || (mytemplate == "spiritpanther") || (mytemplate == "spiritserpent")
                || (mytemplate == "spiritwolf") || (mytemplate == "spiritualbear") || (mytemplate == "phasespider")
                || (mytemplate == "eruva") || (mytemplate == "luine")
                || (mytemplate == "criafuria")
                || (mytemplate == "centaur") || (mytemplate == "dryad")  || (mytemplate == "fadinha") || (mytemplate == "najassur") || (mytemplate == "pytnas") || (mytemplate == "carniceiro") || (mytemplate == "dragonete"))
             return 0;
          endif
        endif
        if ( GetObjProperty(mobile.master, "kit") == "Bandido")
          if ( (mytemplate == "MercenarioRecruta") || (mytemplate == "MercenarioBesteiro") || (mytemplate == "Mercenario") || (mytemplate == "MercenarioLider") || (mytemplate == "MercenarioExperiente") )
            return 0;
          endif
        endif
        return 1;
      endif
    endif
    var enemies := GetConfigStringArray( npccfgfile[mytemplate], "enemy" );
    if (enemies)
      foreach enemy in enemies
        if ( enemy == ParseTemplateName(mobile.npctemplate).template)
          return 1;
        endif
      endforeach
    endif
  endif

endfunction

function AlertAllies(me, opponent)
/*
  var my_spot := GetObjProperty(me, "spawnpoint");
   var allies_near := GetAlliesNear(me);
   var myallies := array{};
   foreach mob in allies_near
      if (CheckLineOfSight(me, mob))
         myallies.append(mob);
      endif
      sleepms(2);
   endforeach

   if (myallies.size())
      var enmies := GetEnemiesNear(me);
      foreach mob in myallies
         var ev := struct;
         ev.type := EVID_ALERT_ALLIES;
         ev.source := me;
         ev.opponent := enmies.randomentry();
         SendEvent(mob, ev);
         sleepms(2);
      endforeach
   endif
*/
  /*var ev := struct;
  ev.type := EVID_ALERT_ALLIES;
  ev.source := me;
  ev.opponent := opponent;
  */
  /*var allies := GetConfigStringArray( npccfgfile[mytemplate], "allie" );

 // printtextabove(me, " " + allies);

  foreach mobile in ListMobilesNearLocation(me.x, me.y, me.z, 8)
    if (mobile.npctemplate)
        if ( ParseTemplateName(mobile.npctemplate).template in allies)
    if (mobile.opponent == error)
    //  printtextabove(mobile, "fui alertado");
            SendEvent(mobile, ev);
    endif
  endif
    endif
  endforeach*/


  var spawnpointserial := GetObjProperty(me, "spawnpoint");
  var spawnpoint := SystemFindObjectBySerial(spawnpointserial);
  var spawns := GetObjProperty(spawnpoint, "Children");

  var aliadosdoplayer := array;
  foreach mobile in (ListMobilesNearLocation(opponent.x, opponent.y, opponent.z, 12))
    if (!mobile.npctemplate)
  //if (CheckLineOfSight(mobile, me))
    aliadosdoplayer.append(mobile);
  //endif
    endif
  endforeach

  var myallies := array;
  foreach mob in spawns
    if (mob != me.serial)
      var realmob := SystemFindObjectBySerial(mob);
      if (realmob)
        myallies.append(realmob);
      endif
    endif
  endforeach

  foreach mob in myallies
      var ev := struct;
      ev.type := EVID_ALERT_ALLIES;
      ev.source := me;
      ev.opponent := aliadosdoplayer[RandomInt(aliadosdoplayer.size())+1];
      SendEvent(mob, ev);
  endforeach


endfunction

//pega a lista de aliados proximos. se o npc for good, os players serao allies pra ele tbm
function GetAlliesNear(me)
  var allies := GetConfigStringArray( npccfgfile[mytemplate], "allie" );
  var myallies := array{};
  foreach mobile in ListMobilesNearLocation(me.x, me.y, me.z, 5)
    if (mobile.npctemplate)
      if ( ParseTemplateName(mobile.npctemplate).template in allies)
        myallies.append(mobile);
      endif
    else
      if (align == ALIGN_GOOD)
        myallies.append(mobile);
      endif
    endif
  endforeach

  return myallies;

endfunction

//pega a lista de inimigos proximo, se tiver player perto e a tendencia for evil ele vira inimigo
function GetEnemiesNear(me)
  var enemies := GetConfigStringArray( npccfgfile[mytemplate], "enemy" );
  var myenemies := ListHostiles(me, 8);
  foreach mobile in ListMobilesNearLocation(me.x, me.y, me.z, 8)
    if (mobile IN myenemies)
       continue;
    endif
    if (mobile.npctemplate)
      if ( ParseTemplateName(mobile.npctemplate).template in enemies)
        myenemies.append(mobile);
      endif
    else
      if (align == ALIGN_EVIL)
        myenemies.append(mobile);
      endif
    endif
  endforeach

  return myenemies;

endfunction

//toca um som random do bixo
function PlayIdleSound()
  var rint := RandomInt(4) + 1;
  if(rint == 4)
    PlaySoundEffect(me, idlesnd1);
  elseif(rint == 3)
    PlaySoundEffect(me, idlesnd2);
  endif
endfunction

function InitializeNpc(mobile)

  //inicializa algumas variaveis que todos os npcs tem
  mytemplate := ParseTemplateName(me.npctemplate).template;
  var elem := FindConfigElem(npccfgfile, mytemplate);
  var sounds := GetConfigStringArray(elem, "IdleSound");
  idlesnd1 := cint(sounds[1]); //CInt(npccfgfile[mytemplate].idlesound1);
  idlesnd2 :=  cint(sounds[2]); //CInt(npccfgfile[mytemplate].idlesound2);
  flee_point := CInt(npccfgfile[mytemplate].FleeLevel)/100.0;
  alert_allies := CInt(npccfgfile[mytemplate].AlertAllies);
  align := cstr(npccfgfile[mytemplate].Alignment);
  areasize := CInt(npccfgfile[mytemplate].AreaSize);
  throwinfo := struct;
  throwinfo.canthrow := 0;
  if (npccfgfile[mytemplate].throwItem != error)
    var donotthrowitem_floor := npccfgfile[mytemplate].notcreateitemfloor;
	throwinfo.canthrow := 1;
    throwinfo.+misssound := CInt(npccfgfile[mytemplate].throwMiss);
    throwinfo.+hitsound := CInt(npccfgfile[mytemplate].throwHit);
    throwinfo.+delay := CInt(npccfgfile[mytemplate].throwDelay);
    throwinfo.+anim := CInt(npccfgfile[mytemplate].throwAnim);
    throwinfo.+item := CInt(npccfgfile[mytemplate].throwItem);
    throwinfo.+damage := Cstr(npccfgfile[mytemplate].throwDamage);
    throwinfo.+range :=   CInt(npccfgfile[mytemplate].throwRange);
	if (!donotthrowitem_floor)
		throwinfo.+notcreateitemfloor := 0;
	else
		throwinfo.+notcreateitemfloor := 1;
	endif
  endif


  if (GetObjPRoperty(mobile, "championspawn") == 2)
    SetObjProperty(mobile, "#championspawn", 1);
    ApplyDamage( mobile, 20000);
  elseif (GetObjPRoperty(mobile, "championspawn") == 1)
    SetObjPRoperty(mobile, "championspawn", 2);
  endif

  //elem := FindConfigElem(npccfgfile, mytemplate);
  propallies := GetConfigStringDictionary( elem, "allyprop");
  //printtextabove(mobile, " prop " + propallies + " " + elem + " " + propallies);

  var spawnpointserial := GetObjProperty(mobile, "spawnpoint");
  //printtextabove(mobile, " " + spawnpointserial);
  //print("spawnpoint " + spawnpointserial);
  if (spawnpointserial)
    var spawnpoint := SystemFindObjectBySerial(spawnpointserial);
    //printtextabove(mobile, " " + spawnpoint);
    if (spawnpoint)
      var range := cint(GetObjProperty(spawnpoint, "Range"));
      if (!range)
        range := 6;
      endif
      SetObjProperty(mobile, "anchor", array{ spawnpoint.x , spawnpoint.y , spawnpoint.z + 2 });
      SetObjProperty(mobile, "range", range);
      var e := SetAnchor(spawnpoint.x, spawnpoint.y, range, 90);
      //print("drop anchor " + e);
    endif
  endif

//printtextabove(mobile, "initialize npc");
  if (GetObjProperty(mobile, "started"))
  return 0;
  endif

  var female := GetObjProperty(mobile, "female");
  if (female)
  mobile.graphic := 401;
  mobile.gender := 1;
  endif

  //setup nos equips, loots, nome e cor
  SetupBackPack(mobile);
  SetupColor(mobile);
  SetupName(mobile);
  SetupEquip(mobile);
  if ((GetObjProperty(mobile, "Mobnoturno") == 1) && (GetGlobalProperty( "dia") != "noite"))
    ApplyRawDamage(mobile, CInt(AP_GetVitalMaximumValue(mobile, "Hits")));
  endif
  var loot := npccfgfile[mytemplate].Loot;
  if (loot)
    Loot_Generate(mobile.backpack, loot);
  endif
  GenerateRecipeLoot(mobile);
  SetupAmmo(mobile);
  SetObjProperty(mobile, "started", 1);


endfunction

//substitui o antigo wander() basicamente checa se tem anchor
function MoveAround()
      var anchor := GetObjProperty(me, "anchor");
    var range := GetObjProperty(me, "range");
    if (GetObjProperty(me, "owner"))
    anchor := 0;
    endif
    if (!range)
    range := 6;
    endif
      var cfg := ReadConfigFile(":brainai:npcdesc");
      var npctemplate := NPC_ParseTemplateName(me.npctemplate);
    if (cfg[npctemplate.template].Category != "Animal")
      if (anchor == error)
        wander();
      else
        if (CoordinateDistance(me.x, me.y, anchor[1], anchor[2]) > range)
          TurnTowardLocation( anchor[1], anchor[2] );
          WalkTowardLocation( anchor[1], anchor[2] );
      if (CoordinateDistance(me.x, me.y, anchor[1], anchor[2]) > 35 && !GetObjProperty(me, "#peacemode"))
      PrintText(me, "*Retorna correndo* [Sistema Anti-Exploits]");
      sleep(3);
      MoveObjectToLocation(me, anchor[1], anchor[2], anchor[3], me.realm, MOVEOBJECT_FORCELOCATION);
      endif
        else
          wander();
        endif
      endif

    else
        wander();
    endif
     PlayIdleSound();
endfunction

/////////////////////////////////
// Funcoes Especiais dos Npcs
////////////////////////////////

function hasCombat(me)
  var str := GetConfigString(npccfgfile[mytemplate],  "SpecialCombat");
  str := splitwords(str);
  if (TS_GetCooldown(me, str[1]))
    return;
  endif
  //printtextabove(me, " " + str);
  if (str.size() >= 1)
      return 1;
  endif
  return 0;
endfunction

function doCombatSpecial(me)

  var str := GetConfigString(npccfgfile[mytemplate],  "SpecialCombat");
  str := splitwords(str);
  
  if(TS_GetCooldown(me, str[1]) || !str)
      return 0; //O especial já foi dado, tem que esperar ou não existe especial
  endif

  var delay := str[2];
  if (!delay)
    delay := 80;
  endif

  if (str[1] != error && delay != error && !TS_GetCooldown(me, str[1]))
    sleepms(10);
    TS_StartCooldown(me, cstr(str[1]), cint(delay));
      var e := Start_Script(str[1], array{me, cint(delay), str[1]});
    sleepms(10);
      //printtextabove(me, " " + e +  " " + str[1]);
  endif

endfunction

function hasAttack(me)
  var str := GetConfigString(npccfgfile[mytemplate],  "SpecialAttack");
  //str := splitwords(str);
  //printtextabove(me, " " + str);
  if (!str)//.size() >= 1)
      return 0;
  endif
  return 1;
endfunction

function hasSpecialCombat(me)
  var str := GetConfigString(npccfgfile[mytemplate],  "SpecialCombat");
  //str := splitwords(str);
  //printtextabove(me, " " + str);
  if (!str)//.size() >= 1)
      return 0;
  endif
  return 1;
endfunction


function doCombatSpecialNotBoss(me, opponent)

  var diff_aleatorio := RandomInt(100);
  var script := GetConfigString(npccfgfile[mytemplate],  "SpecialAttack");
  var delay := GetConfigInt(npccfgfile[mytemplate],  "SpecialAttackDelay");
    if (!delay)
      delay := 80;
    endif
  var armor :=  CS_GetEquipmentInArmorZone(opponent, CS_GetRandomArmorZone());

  if(TS_GetCooldown(me, "npc_special") || !script || script == "" || script == error)
    return 0; //O especial já foi dado, tem que esperar ou não tem especial
  endif

  TS_StartCooldown(me, "npc_special", delay);
   if (CheckResist(opponent, REFLEXOS, diff_aleatorio))
      printtextabove(opponent, "*Evitou o ataque especial do inimigo!*");
    return 0;
   else
    var e := Start_Script(script, array{me, opponent, 0, armor, GetRawDmg(me)});
    return 1;
  endif
endfunction

function throwItem(npc, opponent, dist)
  if ( dist < 3 || dist > 15 )
    return 0;
  elseif ( !CheckLineOfSight(npc, opponent) )
    return 0;
  elseif ( CInt( GetCooldown(npc, "throws" )) != 0 )
    return 0;
  endif

  npc.frozen := 1;
  sleepms(50);

  if (throwinfo.anim)
    PerformAction(npc, throwinfo.anim);
  else
    PerformAction(npc, 4);
  endif

  sleep(1);

  PlayMovingEffect(npc, opponent, throwinfo.item, 15);
  npc.frozen := 0;

if (throwinfo.notcreateitemfloor != 1)
  var item := CreateItemAtLocation(opponent.x, opponent.y, opponent.z, throwinfo.item, 1);
  item.movable := 1;
endif

var chance := RandomInt(100);
if (CheckResist(opponent, REFLEXOS, chance)) // Errou!
	PrintTextAbove(npc, "*Errou!*", _DEFAULT_TEXT_FONT, 38);
    if (throwinfo.misssound)
      PlaySoundEffect(npc, throwinfo.misssound);
    else
      PlaySoundEffect(npc, 0x137);
    endif
  else
    DamageFLS(opponent, RandomDiceRoll(throwinfo.damage), DMG_PHYSICAL, npc);
    PlaySoundEffect(npc, throwinfo.hitdamage);
  endif

  SetCooldown(npc, "throws", throwinfo.delay);


  return 1;
endfunction

function Firebreath(npc, opponent, byref dist)

  if ( dist < 5 || dist > 15 )
    return 0;
  elseif ( CInt( GetCooldown(npc, "breath" )) != 0 )
    return 0;
  elseif ( !CheckLineOfSight(npc, opponent) )
    return 0;
  endif

  TurnTowardLocation( opponent.x, opponent.y );
  //AI_Turn(npc, opponent, NETURN_TOWARD);
  npc.frozen := 1;
  PerformAction(npc, 12);
  PlaySoundEffect(npc, 0x16B);

  sleep(2);

  PlayMovingEffect(npc, opponent, GFX_LARGE_FIREBALL, 10, 10, 1);
  var coord_list := GetCoordsInLine(npc.x, npc.y, opponent.x, opponent.y);

  foreach coord in coord_list
    if ( _coord_iter < 4 ) // Dont make one where its standing
      continue;
    endif

    var field := CreateItemAtLocation(coord.x, coord.y, npc.z, "FireField_NS", 1);
    field.SetDuration(0);
    sleepms(105);

  endforeach



  foreach coord in GetCoordsInSquare(opponent.x, opponent.y, CInt(npccfgfile[mytemplate].BreathRadius))

    if ( RandomInt(2) != 1 ) // Don't place a fire field
      continue;
    endif

    case( RandomInt(3) )
      default:
        PlayStationaryEffect(coord.x, coord.y, npc.z, GFX_EXPLODE_1, 10, 10, 0);
        break;
      1:
        PlayStationaryEffect(coord.x, coord.y, npc.z, GFX_EXPLODE_2, 10, 10, 0);
        break;
      2:
        PlayStationaryEffect(coord.x, coord.y, npc.z, GFX_EXPLODE_3, 10, 10, 0);
        break;
    endcase

    var field;

    if ( RandomInt(2) )
      field := CreateItemAtLocation(coord.x, coord.y, opponent.z, "FireFieldNS", 1);
    else
      field := CreateItemAtLocation(coord.x, coord.y, opponent.z, "FireFieldEW", 1);

    endif
    field.SetDuration(20);
    sleepms(5);

  endforeach

  PlaySoundEffect(opponent, 0x208);
  var chars := ListMobilesNearLocation(opponent.x, opponent.y, opponent.z, CInt(npccfgfile[mytemplate].BreathRadius), npc.realm);

  foreach mobile in chars
    DamageFLS(mobile, RandomDiceRoll(CInt(npccfgfile[mytemplate].BreathDamage)), DMG_FIRE, npc);
  endforeach
  npc.frozen := 0;

  SetCooldown(npc, "breath", CInt(npccfgfile[mytemplate].BreathDelay));
  return 1;

endfunction

function SpitWeb(npc, opponent, byref dist)

  if ( dist < 3 || dist > 10 )
    return 0;
  elseif (Abs(npc.z - opponent.z) >= 15)
    return 0;
  elseif ( !CheckLineOfSight(npc, opponent) )
    return 0;
  elseif ( opponent.frozen )
    return 0;
  elseif (GetEquipmentByLayer(opponent, 0x19))
    return 0;
  elseif ( CInt( GetCooldown(npc, "web" )) != 0 )
    return 0;
  endif


  npc.frozen := 1;
  sleepms(50);
  var web;

  case( RandomInt(4) )

    0: web := 3811; break;
    1: web := 3812; break;
    2: web := 3813; break;
    3: web := 3814; break;

  endcase

  PlayMovingEffect(npc, opponent, web, 10);

  npc.frozen := 0;
  TS_StartTimer(opponent, "paralysis", 5);

  var preso1 := CreateItemAtLocation(opponent.x, opponent.y, opponent.z+1, 4317, 1);
  var preso2 := CreateItemAtLocation(opponent.x, opponent.y, opponent.z+1, 4314, 1);
  sleepms(5);

  //AI_Move(npc, opponent, NEMOVE_TOWARD, NEMOVE_RUN, WAKEUP, dist);
  RunTowardLocation( opponent.x, opponent.y );
  DestroyItem(preso1);
  DestroyItem(preso2);
  sleepms(50);
  SetCooldown(npc, "web", 6);

  return 1;

endfunction

function teleport(npc, opponent)

  var phase_coords := GetCoordsInLine(npc.x, npc.y, opponent.x, opponent.y);
  phase_coords.Erase(phase_coords.Size());
  TurnObjectToward(npc, opponent.x, opponent.y);
  PlaySoundEffect(npc, CInt(npccfgfile[mytemplate].teleportSound));

  foreach coord in phase_coords
    PlayStationaryEffect(npc.x, npc.y, npc.z, CInt(npccfgfile[mytemplate].teleportEffect), 5, 5, 0);
    //PlayStationaryEffectHuefx( npc.x, npc.y, npc.z, CInt(npccfgfile[mytemplate].teleportEffect), 5, 5,  0, CInt(npccfgfile[mytemplate].teleportColor), CInt(npccfgfile[mytemplate].teleportAlpha));
    var z := GetWorldHeight(coord.x, coord.y, npc.realm);
    MoveObjectToLocation(npc, coord.x, coord.y, z, npc.realm, MOVEOBJECT_NORMAL);
    sleepms(20);
  endforeach

  return 1;

endfunction

//funcao tirada dos includes da brain pra remover o nome do pkg do template do npc
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

function herd(ev)
  var holder := ev.data;
  var lx := holder[1];
  var ly := holder[2];
  var loops := 0;
  var walkmode := cint(holder[3]);
  var saytrigger := cint(holder[4]);
  var idlesound := cint(holder[5]);
  var opponent;
  var waittime := 0;
  var nome := ev.source.name;
  while(1)
        if( !CloseIn(me, lx, ly, walkmode))
          loops := loops + 1;
          waittime := 0;
        else
          loops := 0;
          waittime := 1;
        endif
        if((loops >= 30) or (coordinatedistance(me.x, me.y, lx, ly) <= 1))
          break;
        endif
        ev := wait_for_event(waittime);
        case (ev.type)
      SYSEVENT_DAMAGED: opponent := ev.source;
                                            SetOpponent(opponent);
                                            TurnToward(opponent);
          SYSEVENT_ENGAGED: opponent := ev.source;
                                            SetOpponent(opponent);
                                            TurnToward(opponent);
        endcase
  endwhile
  if (saytrigger)
    printtext(me, "*atende o chamado de " + nome + "*");
  endif

  if (idlesound)
        PlayIdleSound();
  endif
  return;
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
