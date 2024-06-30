

function Boss1Fight(opponent, alerted := 0)

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

	var speech := GetConfigStringArray( npccfgfile[mytemplate], "InitCombatSpeech" );
	if (speech.size() > 0)
		if (RandomInt(100)  < 30) //sempre 30% de chance de falar
			Say(speech[RandomInt(speech.size()+1)]);
		endif
	endif

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

		if (hasSpecial(me))
			DoSpecial(me);
		endif

		if (!GetCooldown(me, "flamewave"))
			MoveObjectToLocation(me,  4862, 883, 0, me.realm);
			me.facing:= 4;
      sleep(1);
      Say("*fazendo gestos ritualisticos*");
			sleep(3);

				var fire1 := CreateITemAtLocation( 4867, 887, 0, 0x3996,1, me.realm);
				fire1.SetDuration(60);
				var fire2 := CreateITemAtLocation( 4868, 887, 0, 0x3996,1, me.realm);
				fire2.SetDuration(60);
				var fire3 := CreateITemAtLocation( 4869, 887, 0, 0x3996,1, me.realm);
				fire3.SetDuration(60);
				var fire4 := CreateITemAtLocation( 4870, 887, 0, 0x3996,1, me.realm);
				fire4.SetDuration(60);

			var queue := array;
			//esquerda
			var y := 879;
			for (y := 879; y <= 888;  y := y+1)
				var fire := CreateITemAtLocation( 4857, y, 0, 0x3996,1, me.realm);
				queue.append(fire);
				fire.SetDuration(30);
				sleepms(100);
			endfor

			//baixo
			var x := 4855;
			for (x := 4857; x <= 4866;  x := x+1)
				var fire := CreateITemAtLocation( x, 888, 0, 0x398C, 1, me.realm);
				queue.append(fire);
				//printtextabove(me, " fire " + fire);
				fire.SetDuration(30);
				sleepms(100);
			endfor

			//direita
			 y := 888;
			for (y := 888; y >= 879;  y := y-1)
				var fire := CreateITemAtLocation(4866, y, 0, 0x3996, 1, me.realm);
				queue.append(fire);
				fire.SetDuration(30);
				sleepms(100);
			endfor

			var size := queue.size();
			var locx, locy;
			var gfx ;
			var n;
			for  ( n := 0; n < 12;   n := n +1)
        var sleeptime := 500;
        if (n >= 8)
          sleeptime := 200;
        elseif (n >= 4)
          sleeptime := 300;
        endif
				if (queue[queue.size()].x == 4866 && queue[queue.size()].y == 879) //desenha em cima
					locx := 4866;
					locy := 879;
					var kk := 0;
					for ( kk := 0; kk <= 9; kk := kk + 1)
						destroyItem(queue[1]);
						queue.erase(1);
						var fire := CreateITemAtLocation(locx-kk, locy, 0, 0x398C, 1, me.realm);
						queue.append(fire);
						fire.setDuration(30);
						sleepms(sleeptime);
					endfor
				elseif ( queue[queue.size()].x == 4857 && queue[queue.size()].y == 879) //desenha esquerda
					locx := 4857;
					locy := 879;
					var kk := 0;
					for ( kk := 0; kk <=9; kk := kk + 1)
						destroyItem(queue[1]);
						queue.erase(1);
						var fire := CreateITemAtLocation(locx, locy+kk, 0, 0x3996, 1, me.realm);
						queue.append(fire);
						fire.setDuration(30);
						sleepms(sleeptime);
					endfor
				elseif ( queue[queue.size()].x == 4857 && queue[queue.size()].y == 888) //desenha baixo
					locx := 4857;
					locy := 888;
					var kk := 0;
					for ( kk := 0; kk <=9; kk := kk + 1)
						destroyItem(queue[1]);
						queue.erase(1);
						var fire := CreateITemAtLocation(locx+kk, locy, 0, 0x398c, 1, me.realm);
						queue.append(fire);
						fire.setDuration(30);
						sleepms(sleeptime);
					endfor
				elseif ( queue[queue.size()].x == 4866 && queue[queue.size()].y == 888) //desenha baixo
					locx := 4866;
					locy := 888;
					var kk := 0;
					for ( kk := 0; kk <=9; kk := kk + 1)
						destroyItem(queue[1]);
						queue.erase(1);
						var fire := CreateITemAtLocation(locx, locy-kk, 0, 0x3996, 1, me.realm);
						queue.append(fire);
						fire.setDuration(30);
						sleepms(sleeptime);
					endfor
				else
					//printtextabove(me, " x " + queue[queue.size()].x  + " y " + queue[queue.size()].y);
				endif
			endfor

			SetCooldown(me,  "flamewave", 60);
//			say("burn");

    if (!opponent.dead)
        MoveObjectToLocation(me,  opponent.x, opponent.y, opponent.z, me.realm);
    else
        MoveObjectToLocation(me,  4866, 888,  0, me.realm);
    endif

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
			endcase
		endif
	endwhile

	DisableCombatEvents();
	EnableMainEvents();
	SetWarMode(0);
	SetOpponent(0);
	sleep(1);
endfunction
