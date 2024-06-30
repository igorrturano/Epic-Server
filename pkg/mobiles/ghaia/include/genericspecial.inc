use os;
use util;
use vitals;

include "include/sysEvent";
include ":attributes:attributes";
include ":tn:boss";


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

/*   var speech := GetConfigStringArray( npccfgfile[mytemplate], "InitCombatSpeech" );
   if (speech.size() > 0)
	if (RandomInt(100)  < 30) //sempre 30% de chance de falar
		Say(speech[RandomInt(speech.size()+1)]);
	endif
   endif
*/

  var dist;
  while( me )

    if (me.dead)
       break;
    endif

    if (!opponent)
      break;
    endif

    if (opponent.dead)
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
	/*if ( isBloodied(opponent) && TemHabilidade(opponent, "Furia") )
		LogToFile(":fls_core:berserker.log", opponent.name+" ("+opponent.acctname+") peidou pra "+me.name+" em "+opponent.x+" "+opponent.y, LOG_DATETIME);
		foreach x in EnumerateOnlineCharacters()
			if (x.cmdlevel > 0)
				SendSysMessageEx(x, opponent.name+" BERSERKER PEIDAO!", SSM_REQUEST);
			endif
		endforeach
	endif
*/
      break;
    endif

/*  if (hasSpecial(me))
    DoSpecial(me); // ANTIGO SISTEMA
  endif
*/


  //se eh boss e tem eventos



  // if (!TS_GetCooldown(me, "temspecial"))
  //   if ( GetBossStruct(me) != error )
  //   var unique := GetBossStruct(me);
  //   foreach ce in (unique.combatevent)
  //     sleepms(10);
  //     printtextabove(me, " "  + ce);
  //     if (!TS_GetCooldown(me, ce.name))
  //       var e := Start_Script(":ghaia:special/genericspecial" , array{me, ce});
  //     endif
  //     TS_StartCooldown(me,  ce.name, 30);
  //   endforeach
  //      TS_StartCooldown(me, "temspecial", 30);

  //   endif
  // endif
/*
  if (hasCombat(me))
    DoCombatSpecial(me); ////////////// SPECIALL
    sleepms(10);
  endif
*/
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

    if(!CloseDistance(opponent))
      loops := loops + 1;
      waittime := 0;
    else
      loops := 0;
      waittime := 1;
    endif

    checkFlee(opponent);

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
				if (coordinateDistance(me.x, me.y, ev.x, ev.y) <= 1)
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
    while((Distance(me, opponent) < 15) && (runs < 50) && (CheckLineOfSight(me, opponent)))
      ev := wait_for_event(1);
      //chk := process_flee_event(opponent);
	  RunAwayFrom(opponent);
	  runs := runs +1;
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
    //gambiarra para os NPCs darem awareness a cada 30 segundos (talvez se tirar o comentario do look_around na linha 360 funcione tb)
    if (CInt( AP_GetSkill(me, AWARENESS) ) > 0)
        procura_em_volta(me);
    endif

    ev := os::wait_for_event(30);

    //gambiarra para os NPCs darem awareness a cada 30 segundos
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

//Gambiarra para NPC dar awareness
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
