use uo;
use util;
use vitals;

include ":charactercreation:resists";
include ":timedScripts:timedScripts";
include ":combat:damage";
include ":nature:nature";

function EfeitosTremor(item, danobase, danoale, dif, range)
    //  if( who.cmdlevel > 0)
    //    if(!GetObjProperty (who, "lastpossessedserial"))
    //      return;
    //    endif
    //  endif

    var chars := ListmobilesNearLocation(item.x, item.y, item.z, range, item.realm);
    foreach char in chars
        PlaySoundEffect(char, 0x222);
        SendSysMessageEx(char, "O solo nas proximidades treme violentamente.", SSM_INFO);
        if (char.npctemplate == "Zombie")
            DamageFLS(char, 1000);
        endif
        if (CheckResist(char, REFLEXOS, dif)) //note: colocar uma variavel de chance de resistir ao terremoto
            PrintTextAbove(char, "*Atraves de movimentos acrobaticos habeis, resiste ao tremor*");
            PerformAction(char, 20);
        else
            TS_StartTimer(char, "paralysis", 2);
            PerformAction(char, 21);
            DamageFLS(char, danobase + (RandomInt(danoale)));

            var pedra;

            for (pedra := 1; pedra <= (RandomInt(5) + 1); pedra := pedra + 1)
                var tempx := char.x + RandomIntMinMax(-4, 4);
                var tempy := char.y + RandomIntMinMax(-4, 4);

                //SendSysMessageEx(who, "p2", SSM_INFO);

                if (CheckLosAt(char, tempx, tempy, char.z) != 0 || !char.dead)
                    PlayMovingEffectXYZ(tempx, tempy, char.z + 60, tempx, tempy, char.z, 0x1773, 0x9, 0xA1);
                    CreateItemAtLocation(tempx, tempy, char.z, 0xBD11);
                endif
            endfor
        endif
    endforeach
endfunction

function EfeitosClima(item, options, duration, range)
    var type;
    //var duration := 20;
    var som := 0;
    var bolt := 0;
    var ventoForte := 0;
    if (options == 1) //chuva
        type := 0;
    elseif (options == 2) //neve
        type := 2;
    elseif (options == 3) //tempestade
        type := 1;
    elseif (options == 4) //tempestade de raios
        type := 1;
        som := 1;
        bolt := 1;
    elseif (options == 5) //vento
        som := 2;
    elseif (options == 6) //vento forte
        som := 2;
        ventoForte := 1;
    endif
    var lever_opts := GetObjProperty(item, "lever_opts");
    var move := lever_opts."movefinal";
    var origx := cint(move[1]);
    var origy := cint(move[2]);
    var origz := cint(move[3]);
    var i := 0;
    var chars;
    if (!move)
        chars := (ListmobilesNearLocation(item.x, item.y, item.z, range, item.realm));
    else
        chars := (ListmobilesNearLocation(item.x - origx, item.y - origy, item.z - origz, range, item.realm));
    endif
    while (i < duration)
        // se tiver polvora, destruir
        var items := ListItemsNearLocation(item.x, item.y, item.z, 20, item.realm);
        foreach item in items
            if (item.objtype == 0x9092)
                PrintTextAbove(item, "*Molhada*");
                SubtractAmount(item, RandomInt(5) + 1);
            endif
        endforeach
        
        if (som)
            windsound(item);
        endif
        var fuga;
        var bolts := RandomInt(5);
        SetWeatherForPlayer(chars, type, 50);
        foreach char in chars
            if (distance(char, item) > range + 10)
                SendSysMessageEx(char, "Voce escapou da intemperie climatica.", SSM_INFO);
                fuga := 1;
                break;
            endif
			if (!char.isUnderRoof())
				wetPowder(char);
            	SetWeatherForPlayer(char, type, 50);
			endif

            SetWeatherForPlayer(char, type, 50);
            if (distance(char, item) < 20)
                SetWeatherForPlayer(char, type, 50);
                if (bolt)
                    if (RandomInt(100) < 5)
                        PlayLightningBoltEffect(char);
                        ApplyDamageEX(char, RandomDiceRoll("1d10") + 40, DMG_ENERGY);
                        SendSysMessageEx(char, "Voce foi atingido por um raio!", SSM_INFO);
                    endif
                endif

                if (ventoForte)
                    if (RandomInt(100) < 5)
                        SendSysMessageEx(char, "O vento forte impede seus movimentos!", SSM_INFO);
                        TS_StartTimer(char, "paralysis", 5);
                    endif
                endif
            endif
        endforeach

        if (fuga == 1)
            break;
        endif
        if (bolt)
            var k := 0;
            while (k < bolts)
                if (RandomInt(100) < 15)
                    DoBolt(item);
                endif
                k := k + 1;
            endwhile
        endif

        sleep(5);
        i := i + 1;
    endwhile

    foreach char in chars
        EraseObjProperty(char, "molhado");
        SetWeatherForPlayer(char, 0xFF, 1);
        moveobjecttolocation(char, char.x, char.y, char.z, char.realm);
        SetWeatherForPlayer(char, 0xFF, 1);
        moveobjecttolocation(char, char.x, char.y, char.z, char.realm);
    endforeach
endfunction

function windsound(person)
    var wind := {21, 22, 23};
    if (randomdiceroll("1d4") == 2)
        playsoundeffect(person, wind[randomint(3) + 1]);
    endif
endfunction