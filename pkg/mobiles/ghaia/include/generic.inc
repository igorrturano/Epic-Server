use os;
use util;
use vitals;

include "include/sysEvent";
include ":attributes:attributes";
include ":tn:boss";
include ":ghaia:ghaia";


function hasSpecial(me)
  if (npccfgfile[mytemplate].Special)
    if (!GetCooldown(me, npccfgfile[mytemplate].Special))
      return 1;
    endif
  endif
  return 0;
endfunction


function doSpecial(me)

  //checa as condicoes
  var conds :=  GetConfigStringArray( npccfgfile[mytemplate], "SpecialCondition" );

  foreach cond in  conds
    if (cond == "enemies")

    endif

    if (cond == "closedist")
      if  (distance(me, me.opponent)  >= 2)
        me.concealed :=0;
        return;
      endif
    endif
  endforeach

  SetCooldown(me, npccfgfile[mytemplate].Special, npccfgfile[mytemplate].SpecialCooldown);

  var says := GetConfigStringArray( npccfgfile[mytemplate], "SpecialSpeak" );
  var text := says[RandomInt(says.size())+1];
  if (says != error)
    Say(text);
  endif

  if (npccfgfile[mytemplate].SpecialAnimation)
    var e := PerformAction(me, cint(npccfgfile[mytemplate].SpecialAnimation));
  endif

  var effect := SplitWords(GetconfigString(npccfgfile[mytemplate], "SpecialEffect" ));
  if (effect)
    if (effect[1] == "timer")
      if (effect[2] == "area")
        foreach  m in (ListMobilesNearLocation( me.x, me.y, me.z, 3, me.realm))
          if (!m.npctemplate)
          TS_StartTimer(m, effect[3], cint(effect[4]), cint(effect[5]), me);
          endif
        endforeach
      elseif (effect[2] == "me")
		TS_StartTimer(me, effect[3], cint(effect[4]), cint(effect[5]));
	  else
        TS_StartTimer(me.opponent, effect[3], cint(effect[4]), cint(effect[5]), me);
      endif
    endif
  endif

endfunction

//construir uma array de oponentes?
//incluir algum criterio de escolha?
function EnableMainEvents()
  DisableEvents(/*SYSEVENT_SPEECH +*/ SYSEVENT_LEFTAREA + SYSEVENT_DISENGAGED /*+ SYSEVENT_OPPONENT_MOVED*/);
  EnableEvents(SYSEVENT_ENGAGED + SYSEVENT_DAMAGED + SYSEVENT_ENTEREDAREA, areasize);
endfunction

function DisableMainEvents()
  DisableEvents(SYSEVENT_ENGAGED + SYSEVENT_DAMAGED + SYSEVENT_ENTEREDAREA);
endfunction


function CloseIn(me, lx, ly, mode := 0 )
  var d := coordinatedistance(me.x, me.y, lx, ly);
  case (d)
    0:       return 1;
    default:
	     if (mode == 0)
	      	WalkTowardLocation(lx, ly);
	     else
		RunTowardLocation(lx, ly);
    //Quebraquebra(me);
	     endif
             return 0;
  endcase
endfunction

function Fight(opponent, alerted := 0)


 // mystate := FIGHTING;
  if(opponent.serial == me.serial)
    SetWarMode(0);
    opponent := 0;
    return;
  endif

  var oldprio := set_priority(50);
  SetOpponent(opponent);
  DisableMainEvents();
  EnableCombatEvents();
  TurnToward(opponent);
  var loops := 0;
  var ev;
  var waittime := 0;

//	printtextabove(me, " " + alert_allies);
  if (!alerted) //se ele ja nao foi alertado por outro aliado
	if (alert_allies) //se esse npc eh do tipo que alerta
	//	printtextabove(me, "vou alertar");
		AlertAllies(me, opponent);
	endif
 // else
//	printtextabove(me, "fui alertado, nao vou alertar");
  endif

   var speech := GetConfigStringArray( npccfgfile[mytemplate], "InitCombatSpeech" );
   if (speech.size() > 0)
	if (RandomInt(100)  < 50) //sempre 50% de chance de falar
		Say(speech[RandomInt(speech.size()+1)]);
	endif
   endif


  var dist := CoordinateDistance(me.x, me.y, opponent.x, opponent.y);
  var range := 1;
  var minrange := 0;

  while( me )

    if (me.dead)
       break;
    endif

    if (!opponent)
      break;
    endif

    if (opponent.dead)
      var corpo := opponent.GetCorpse();
      var temnego := 0;
      foreach npc in ListMobilesInLineOfSight(me, areasize + 2)
        if( CanFight(me, npc, align) )
          temnego := 1;
          break;
        endif
        sleepms(2);
      endforeach
	    if (!temnego)
		    var corpo := opponent.GetCorpse();
		    var end := ReadGameClock() + 10;
    		while (CoordinateDistance(me.x, me.y, corpo.x, corpo.y) < areasize + 5 && end > ReadGameClock())
        //arrastaporai(me, corpo);
          RunAwayFromLocation(corpo.x, corpo.y);
          sleepms(3);
		    endwhile
	    endif
      break;
    endif
    if (opponent.concealed)
      break;
    endif
    if (opponent.hidden)
    	break;
    endif

    dist := CoordinateDistance(me.x, me.y, opponent.x, opponent.y);
    if ( dist > 20)
    	if ( isBloodied(opponent) && TemHabilidade(opponent, "Baixo Limiar de Furia") )
    		LogToFile(":fls_core:berserker.log", opponent.name+" ("+opponent.acctname+") peidou pra "+me.name+" em "+opponent.x+" "+opponent.y, LOG_DATETIME);
        var tipo := "thuriao";
        var region := GetRegionName(opponent);
        var coords := {opponent.x, opponent.y};
        var nome := GetObjProperty(opponent, "nome_inicial");
        Start_script(":DiscordWebhook:discord", {tipo, nome, opponent.acctname, me.name, region, coords});
    		foreach x in EnumerateOnlineCharacters()
    			if (x.cmdlevel > 0)
    				SendSysMessageEx(x, opponent.name+" BERSERKER PEIDAO!", SSM_REQUEST);
    			endif
    		endforeach
    	endif
      break;
    endif

/*  if (hasSpecial(me))
    DoSpecial(me); // ANTIGO SISTEMA
  endif
*/


  //se eh boss e tem eventos
    if ( GetBossStruct(me) != error )
      var unique := GetBossStruct(me);
      foreach ce in (unique.combatevent)
        sleepms(10);
    //		printtextabove(me, " "  + ce);
        if (!TS_GetCooldown(me, ce.name))
          var e := Start_Script(":ghaia:special/genericspecial" , array{me, ce});
        endif
        TS_StartCooldown(me, ce.name, ce.cd);
      endforeach
    endif
    if(isBoss(me)) //Faz os SpecialCombats (p/ boss) e o SpecialAttacks (p/ mobs)
      if (hasCombat(me))
        DoCombatSpecial(me); ////////////// SPECIALL para boss
      endif
      if (hasSpecial(me))
        DoSpecial(me); // ANTIGO SISTEMA
      endif
    else
      if (hasAttack(me))
        var chance := CInt(npccfgfile[mytemplate].SpecialAttackChance); //para mobs do npcdesc
        if (!chance)
          chance := 10;
        endif
        if ((RandomInt(500) <= chance) && (!TS_GetCooldown(me, "npc_special")) )// Acertou!
          DoCombatSpecialNotBoss (me, opponent);
        endif
      endif
      if (hasSpecial(me))
        DoSpecial(me); // ANTIGO SISTEMA
      endif
    endif
    var armor :=  CS_GetEquipmentInArmorZone(opponent, CS_GetRandomArmorZone());
    if (hasCombat(me))
      DoCombatSpecial(me); ////////////// SPECIALL
      sleepms(10);
    endif

    if ( (dist > throwinfo.range) && (throwinfo.canthrow == 1) )
      throwItem(me, opponent, dist);
    endif

    if ( (dist > 6) && ( CInt(npccfgfile[mytemplate].teleport) ) )
      teleport(me, opponent);
    endif

    if ( (dist >  CInt(npccfgfile[mytemplate].BreathRange)) && CInt(npccfgfile[mytemplate].BreathAttack) == 1 )
      FireBreath(me, opponent, dist);
    endif

    if ( (npccfgfile[mytemplate].WebRange) && (dist > npccfgfile[mytemplate].WebRange) )
      SpitWeb(me, opponent,dist);
    endif


     if (dist > range || !CheckLineOfSight(me, opponent))
        CalcPathAndMove(me, opponent, 275 - me.run_speed);
     //elseif (dist < minrange)
     //   checkFlee(opponent);
     endif

     checkFlee(opponent);

  /*
    if(!CloseDistance(opponent))
      loops := loops + 1;
      waittime := 0;
    else
      loops := 0;
      waittime := 1;
    endif
	*/

   // checkFlee(opponent);

    ev := wait_for_event(waittime);

//    printtextabove(self(), "event " + ev);
  	if(ev.type != SYSEVENT_SPEECH)
  		case (ev.type)
  			SYSEVENT_DAMAGED:

  			//se ele receber dano de alguem mais proximo que o oponente ou se receber dano de alguem e o oponente nao estiver acessivel
  			if( /*(CoordinateDistance(me.x, me.y, ev.source.x, ev.source.y) < CoordinateDistance(me.x, me.y, opponent.x, opponent.y)) ||*/ (!CheckLineOfSight(me, opponent)) || (!opponent))
  				if ( (opponent) && RandomInt(4) == 0 ) //se ainda existir o oponente so tem 25% de chance de mudar de alvo
  					if (ev.source)
  						opponent := ev.source;
  						SetOpponent(opponent);
  						TurnToward(opponent);
  					endif
  				elseif ( (!opponent) || (!CheckLineOfSight(me, opponent))) //mas se nao tiver mais o oponent entao 100% de chance de mudar de alvo
  					if (ev.source)
  						opponent := ev.source;
  						SetOpponent(opponent);
  			      TurnToward(opponent);
  					endif
  				endif
  			endif

  			SYSEVENT_ENGAGED:
  				if(ev.source)
  					if(RandomInt(6) == 1)
  						opponent := ev.source;
  						SetOpponent(opponent);
  						TurnToward(opponent);
  					endif
  				endif
  			EVID_TAUNT:
  				if(ev.source)
  					opponent := ev.source;
  					SetOpponent(opponent);
  				        TurnToward(opponent);
  				endif
  			EVID_FLEE:
  				if (ev.source)
  					flee(ev.source);
  				endif
  			EVID_PEACEMADE:
  			//	sendsysmessage(ev.source, "peacemade");
  				SetWarMode( 0 );
         	SetOpponent( 0 );
         	sleep(1);
         	DisableCombatEvents();
         	EnableMainEvents();
  				return;
  			EVID_COMBAT_MOVE:
  				RunTowardLocation( ev.x, ev.y );
          //Quebraquebra(me);
  				if (coordinateDistance(me.x, me.y, ev.x, ev.y) <= 1)
            me.concealed :=0;
  					clear_event_queue();
  				endif
  		endcase
  	endif

  endwhile

  DisableCombatEvents();
  EnableMainEvents();
  SetWarMode(0);
  SetOpponent(0);
  sleep(1);
endfunction

function EnableCombatEvents()
  EnableEvents(SYSEVENT_ENGAGED + SYSEVENT_DAMAGED );
  DisableEvents(SYSEVENT_SPEECH);
endfunction

function DisableCombatEvents()
  DisableEvents(SYSEVENT_ENGAGED + SYSEVENT_DAMAGED );
endfunction


function CheckFlee(opponent)
  if((AP_GetVital(me, HITS) /AP_GetVitalMaximumValue(me, HITS)) <= flee_point)
	EraseObjProperty(me,"#flees");
    flee(opponent);
  endif
endfunction

function Flee(opponent)
  if((me.script == "immobile") || (me.script == "immobilespell"))
	sleep(2);
	return;
  else
   var speech := GetConfigStringArray( npccfgfile[mytemplate], "FleeSpeech" );
   if (speech.size() > 0)
	if (RandomInt(100)  < 30) //sempre 30% de chance de falar
		Say(speech[RandomInt(speech.size()+1)]);
	endif
   endif

    var numflees := GetObjProperty(me,"#flees");
	if(numflees > 10)
      RestartScript(me);
	else
	  numflees := numflees + 1;
	endif
    SetObjProperty(me,"#flees", numflees);
    var runs := 0;
    var chk := 0;
    var ev;
    var furtivo := GetConfigStringArray( npccfgfile[mytemplate], "Furtivo" );

    while( opponent || (Distance(me, opponent) < 20) && (runs < 50) && (CheckLineOfSight(me, opponent)))
      ev := wait_for_event(1);
      //chk := process_flee_event(opponent);
    if (furtivo == 1)
      WalkAwayFrom(opponent);
    else
	    RunAwayFrom(opponent);
    endif
	  runs := runs +1;

    if (furtivo == 1)
      if (!GetCooldown(me, "fleestealth"))
         PrintText(me, "*esconde*");
         sleep(1);
         me.hidden := 1;
         me.stealthsteps := 60;
         SetCooldown(me, "fleestealth", 30);
      endif
    endif

      if (Distance(me, opponent) > 20)
        opponent:=0;
      endif

	  case (ev.type)
        SYSEVENT_DAMAGED: if(((ev.source.npctemplate) and (CheckLineOfSight(me, ev.source))) ||(CoordinateDistance(me.x, me.y, ev.source.x, ev.source.y) < CoordinateDistance(me.x, me.y ,opponent.x, opponent.y)) || (!CheckLineOfSight(me, opponent)) || (!opponent) || (RandomInt(3)==1))
					        opponent := ev.source;
			  		        SetOpponent(opponent);
			              endif
	  endcase
	  if(chk)
	    break;
	  endif
    endwhile
  endif
endfunction




//no sleep mode ele o waitfor event fica 30 secs e ele nao fica mais fazendo lookarounds o tempo todo
function sleepmode()


//  foreach critter in ListMobilesNearLocation( me.x,me.y,me.z, Cint(npccfgfile[mytemplate].AreaSize);
//	look_around(); //da um lookaround pra ver se nao tem um inimigo antes do sleep
 //     return;
//  endforeach

   var speech := GetConfigStringArray( npccfgfile[mytemplate], "SleepSpeech" );
   if (speech.size() > 0)
	if (RandomInt(100)  < 30) //sempre 30% de chance de falar
		Say(speech[RandomInt(speech.size()+1)]);
	endif
   endif

  EraseObjProperty(me,"#flees");
  DisableMainEvents();

  //TODO se tiver skill hiding, usar
  EnableEvents(SYSEVENT_ENTEREDAREA, areasize);
  EnableEvents(SYSEVENT_ENGAGED);
  EnableEvents(SYSEVENT_DAMAGED);
  var ev;
  while (1)

    if (CInt( AP_GetSkill(me, AWARENESS) ) > 0)
        procura_em_volta(me);
    endif

    ev := os::wait_for_event(30);

    if (CInt( AP_GetSkill(me, AWARENESS) ) > 0)
        procura_em_volta(me);
    endif

    repeat
    //npcs criam ovos ou outras coisas
    if ( (GetCooldown(me, "dropitem" ) == 0) && (CInt(npccfgfile[mytemplate].DropDelay) > 0) )
       CreateItemAtLocation(me.x, me.y, me.z, CStr(npccfgfile[mytemplate].DropItem), 1, me.realm);
       SetCooldown(me, "dropitem", CInt(npccfgfile[mytemplate].DropDelay));
    endif
    //se for carneiro
    if ( me.graphic == 223)
	if (GetCooldown(me, "wool") == 0)
		me.graphic := 207;
	endif
    endif
    PlayIdleSound();
    case (ev.type)
      SYSEVENT_ENGAGED:
                             if (me.hidden)
                               me.hidden := 0;
                             endif
                             EnableMainEvents();
                             return ev;

      SYSEVENT_DAMAGED:
                             if (me.hidden)
                               me.hidden := 0;
                             endif
                             EnableMainEvents();
                             return ev;

      SYSEVENT_ENTEREDAREA:
                          if (CanFight(me, ev.source, align))
                             if(me.hidden)
                               me.hidden := 0;
                             endif
                             EnableMainEvents();
                             return ev;
                          endif

	EVID_TAUNT:
                             EnableMainEvents();
                             return ev;
	EVID_ALERT_ALLIES:	EnableMainEvents();
                             return ev;
	EVID_HERDING:
				EnableMainEvents();
				return  ev;
	EVID_FLEE:
			EnableMainEvents();
			return ev;

    endcase
    until(!(ev := os::wait_for_event(30)));
  endwhile
endfunction

function Quebraquebra(me)
  foreach item in (ListItemsNearLocation(me.x, me.y, me.z, 1, me.realm))
		if (GetObjProperty(item, "crafteracc"))
			var roll := RandomInt(100)+1;
			if (roll > 99)
				if (!GetObjProperty(item, "LockId"))
					createitematlocation(item.x, item.y, item.z, 3119, 1, item.realm);
					sleepms(10);
					DestroyItem(item);
				endif
			endif
		endif
	endforeach
endfunction

function CalcPathAndMove(me, byref opponent, sleepdelay)

  if (!opponent)
    return 0;
  endif

   var face_str := getFacingString(GetFacing(me.x, me.y, opponent.x, opponent.y));
   var stam := AP_GetVital(me, STAMINA);
   var move_type := 1;

   if (stam < 5)
      move_type := 0;
   endif

	var path := GetObjProperty(me, "path"); //procura se ja calculou o path
	var last_loc := GetObjProperty(me, "#enemy_last_loc"); //procura se ja calculou o path
   if (last_loc && path)
      if (CoordinateDistance(opponent.x, opponent.y, last_loc[1], last_loc[2]) > 3)
        me.concealed :=0;
         path := 0;
         EraseObjProperty(me, "path");
         EraseObjProperty(me, "#enemy_last_loc");
         last_loc := 0;
      else
         last_loc := 1;
      endif
   endif

   if (stam <= 1)
      var last_msg := GetObjProperty(me, "#fatigue");
      if (!last_msg || last_msg < ReadGameClock())
         SetObjProperty(me, "#fatigue", ReadGameClock()+15);
         PrintText(me, "*fatigado*");
      endif
      sleep(1);
   elseif (CheckLosAt(me, opponent.x, opponent.y, opponent.z) && !path && CanMove(face_str))
      EraseObjProperty(me, "path");
      EraseObjProperty(me, "#enemy_last_loc");

      if (move_type)
         RunToward( opponent );
         //Quebraquebra(me);
      else
         WalkToward( opponent );
      endif

      var reachtry := GetObjProperty(me, "#reachtry");
      if (reachtry == error || (reachtry[1].x != opponent.x && reachtry[1].y != opponent.y))
         reachtry := array{struct{ x := opponent.x, y := opponent.y, z := opponent.z }, ReadGameClock()+5 };
         SetObjProperty(me, "#reachtry", reachtry);
      endif

      if (reachtry[2] > ReadGameClock())
         path := FindPath(  me.x, me.y, me.z, opponent.x, opponent.y, opponent.z, me.realm, 0, 15);
         if (!path)
            //Flee(opponent);;
            sleepms(sleepdelay);
         else
            SetObjProperty(me, "path", path);
            SetObjProperty(me, "#enemy_last_loc", array{opponent.x, opponent.y, opponent.z});
         endif
      endif

      sleepms(sleepdelay);
	else
      if (path == error || last_loc ) //se não calculou
         var flags := FP_IGNORE_MOBILES;
         if ( cint(GetObjProperty(me, "ignoredoor")) || cint(GetObjProperty(me, "opendoor")) )
            flags := flags| FP_IGNORE_DOORS;
         endif
         path := FindPath(  me.x, me.y, me.z, opponent.x, opponent.y, opponent.z, me.realm, flags, 15); //calcula o path

         if (!path[1] || !path || path == error)
            //Flee(opponent);
            sleepms(sleepdelay);
         else
            if (move_type) RunTowardLocation( path[1].x, path[1].y );
            //Quebraquebra(me);
            else WalkTowardLocation(path[1].x, path[1].y); endif

				path.erase(1); //deleta a coordenada e vai pra proxima
            SetObjProperty(me, "path", path);
            SetObjProperty(me, "#enemy_last_loc", array{opponent.x, opponent.y, opponent.z});
         endif
		endif

      var coord := path[1]; //pega a primeira coordenada
      if (coord);
         var coordtry := GetObjProperty(me, "#coordtry");
         if (coordtry == error || coordtry[1] != coord)
            coordtry := array{coord, ReadGameClock()+5 };
            SetObjProperty(me, "#coordtry", coordtry);
         endif
         if (move_type) RunTowardLocation( coord.x, coord.y );
         //Quebraquebra(me);
         else WalkTowardLocation(coord.x, coord.y); endif
         sleepms(sleepdelay);
         Opendoors(me); //tenta abrir possiveis portas
         if (CoordinateDistance(me.x, me.y, coord.x, coord.y) <= 1) //se chegou onde devia
            me.concealed :=0;
            path.erase(1); //deleta a coordenada e vai pra proxima
            if (path[1])
               SetObjProperty(me, "path", path);
            else
               EraseObjProperty(me, "path");
            endif
         else // se não consegue chegar na coord
            if ( coordtry[2] > ReadGameClock()) //se ta tentando chegar la a mais de 5 segundos
               EraseObjProperty(me, "path"); //deleta o path atual
               me.remAggro(opponent);
               var old_opponent := opponent;
               ChooseHighestOpponent(me, opponent);
               if (old_opponent == opponent) /*Flee(opponent);*/ endif
            endif
         endif
      else
         if (move_type) RunToward( opponent );
         //Quebraquebra(me);
         else WalkToward( opponent ); endif
         sleepms(sleepdelay);
      endif
	endif
endfunction

/*
function ChooseAnotherOpponent(me)

	var otherTarget := ListMobilesNearLocationEx( me.x, me.y, me.z, 8, LISTEX_FLAG_NORMAL, me.realm );
	if  (otherTarget.size() > 0)
		foreach mob in otherTarget
			if(CanFight(me, mob, align)) //verifica se eh inimigo
				Fight(mob);
			endif
		endforeach
	endif
endfunction
*/


function Opendoors(me)

  foreach door in (ListItemsNearLocation(me.x, me.y, me.z, 1))
    if  ((door.isa(POLCLASS_DOOR)) and (!door.locked)) //se nao ta trancada
		if ( cint(GetObjProperty(me, "ignoredoor")) == 1)
			PrintText(me, "*atravessa a porta*");
			MoveObjectToLocation(me, door.x, door.y, door.z, door.realm, MOVEOBJECT_FORCELOCATION);
		elseif (cint(GetObjProperty(me, "opendoor")) == 1)
			PrintText(me, "*abre a porta*");
			door.Open();
		endif
	endif
  endforeach
endfunction


function procura_em_volta(me)
    var range := CInt( AP_GetSkill(me, AWARENESS) / 10 ) + 1;
    foreach mobile in ListMobilesNearLocationEX(me.x, me.y, me.z, range, LISTEX_FLAG_HIDDEN)
        if ( mobile == me ); // Do Nothing
        elseif ( CInt(GetObjProperty(me, "IgnoreHidden")) ); // Do Nothing
        else
            if ( CheckLineOfSight(me, mobile) )
                var difficulty := CalculateDHDiff(me, mobile);
                if ( SkillCheck(me, AWARENESS, difficulty) > 0 )
                    mobile.hidden := 0;
                    PrintText(mobile, "*aparece*");
                    SendSysMessageEx(mobile, me.name+" te encontrou!", SSM_FAIL);
                endif
            endif
        endif
        sleepms(2);
    endforeach
endfunction

function LootSomething(me)

	if ( !me.backpack )
		EquipItem(me, CreateItemAtLocation(me.x, me.y, me.z, "backpack", 1));
	endif

  var contents := array;

  foreach corpo in ListItemsNearLocation(me.x, me.y, me.z, 2, me.realm);
    if (corpo.IsA(POLCLASS_CORPSE))
      foreach item in EnumerateItemsInContainer(corpo)
        var inside := 0;
        if ( item.IsA(POLCLASS_CONTAINER) )
          inside := EnumerateItemsInContainer(item).size();
        endif

        if ( inside > 0 );
          //PrintTextAbove(corpse, item.desc+" is a container.. skipping it");
          //Do nothing
        elseif ( !item.movable );
          //PrintTextAbove(corpse, item.desc+" cant be moved.. skipping it");
          //Do nothing
        else
          //PrintTextAbove(corpse, item.desc+" is lootable.");
          contents.append(item);
        endif
        sleepms(5);
      endforeach
    endif
  endforeach

	if ( contents.size() > 0 )
		var item := contents[RandomInt(contents.size())+1];
		contents := 0;

		if ( item )
			//PrintTextAbove(me, "*revirando o corpo*");
			MoveItemToContainer(item, me.backpack);

			if ( item.layer )
				var equip := GetEquipmentByLayer(me, item.layer);
				if ( !equip.IsA(POLCLASS_WEAPON) )
					if ( item.ar > equip.ar )
						MoveItemToContainer(equip, me.backpack);
						if ( !EquipItem(me, item) )
							EquipItem(me, equip);
						endif
					endif
				endif
			elseif ( item.IsA(POLCLASS_EQUIPMENT) )
				EquipItem(me, item);
      elseif ( item.IsA(POLCLASS_ARMOR) )
        EquipItem(me, item);
			endif
		endif
	endif
endfunction

function arrastaporai(me, corpo)
  if (!GetObjProperty(me, "canibal"))
    return;
  endif

  if (GetObjProperty(corpo, "moving"))
    return 0;
  endif

  SetObjProperty(corpo, "moving", 1);

  if (corpo.isa(POLCLASS_CORPSE))
      corpo.movable := 1;
      var i := 1;
	    while ( i < 5)
        if (!(me.hidden || me.concealed))
          PrintText(me, "*arrastando "+corpo.name+"*");
        endif

        MoveObjectToLocation(corpo, me.x, me.y, me.z);
        var mob := SystemFindObjectBySerial(corpo.ownerserial);
        sleep(2);
		    i := i + 1;
      endwhile
      PrintText(me, "*Solta "+corpo.desc+"*");
      EraseObjPRoperty(corpo, "moving");
      corpo.movable := 0;
  endif
endfunction


function getFacingString(me_face)
   case (cint(me_face))
      0:default: return "N";
      1: return "S";
      2: return "E";
      3: return "W";
      4: return "NW";
      5: return "NE";
      6: return "SW";
      7: return "SE";
   endcase
endfunction
