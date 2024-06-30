
use uo;
use util;
use cfgfile;

include "include/damage";

function playweathersound(type, person)
	var wind := {21, 22, 23};
	var lightning := {41, 42, 519};
 
	if (type == 0 || type == 1)
		if (randomdiceroll("1d8") < (type + 5))
			playsoundeffect(person, lightning[randomint(3)+1]);
		endif
	endif
 
	if (randomdiceroll("1d4") == 2)
		playsoundeffect(person, wind[randomint(3)+1]);
	endif
endfunction

function DoBolt(who)

  if (who.multi)
    return;
  endif
	var lightning := {41, 42, 519};
  var xpos := who.x;
  var ypos := who.y;
  var xrand := RandomInt(20);
  var yrand := RandomInt(20);
  xpos := ((xpos - xrand) + 10);
  ypos := ((ypos - yrand) + 10);
  var zpos := GetMapInfo( xpos, ypos ).z;
  var mob := CreateNpcFromTemplate( ":brainai:rabbit", xpos, ypos, zpos);
  mob.cmdlevel := 2;
  mob.concealed := 1;
  setobjproperty(mob, "guardkill", 1);
  PlayLightningBoltEffect( mob );
  playsoundeffect(mob, lightning[randomint(3)+1]);
  MoveObjectToLocation(mob,10,10,10, mob.realm, MOVEOBJECT_FORCELOCATION);
  mob.kill();

endfunction


function DruidWeather(who, type, duration, radius, som, bolt := 0, ventoForte := 0 )

	var originalx, originaly;
	originalx := who.x;
	originaly := who.y;
	var i;
	for(i := 0; i <= duration; i := i+1)

		if ( (who.x != originalx) || (who.y != originaly) )
		endif

		var bolts := 0;
		
		var weatherid := CStr(type);
		var id_len := len(weatherid);

		if( id_len == 0 )
			weatherid := "00";
		elseif( id_len == 1)
 			weatherid := "0" + weatherid;
 		endif 
	 	
	 	var packet := "65"+weatherid+"7000";
	 
		foreach person in EnumerateOnlineCharacters()
			//if (person.x < 5120)
				if (distance(who, person) < radius)
					playweathersound(som, person);
					SendPacket(person, packet);
				endif
			//endif
		endforeach

		if (bolt)
			foreach mobile in (ListMobilesNearLocation( who.x, who.y, who.z, radius, who.realm ))
				if (RandomInt(100) < 5)
					PlayLightningBoltEffect( mobile );
					ApplyDamageEX(mobile, RandomDiceRoll("1d10")+40, DMG_ENERGY);
				elseif (RandomInt(100) < 15 )
					DoBolt(who);
				endif
			endforeach		
		endif

		sleep(5);
	endfor
	
endfunction


//Send a number for the weather type. Duration is duration * 5 minutes.
//Weather list from Mad Man's weather include.
//   0 - light rain ("It starts to rain")
//   1 - rain/thunder ("A fierce storm approaches")
//   2 - snow ("It begins to snow")
//   3 - brewing storm - not entirely sure the effect ("A storm is brewing")
//   255 - None (Turns sound effects off?)
//   254 - Unknown effect...

function weathereffects(type, duration, severity := 5)

	var i;
	for(i := 0; i <= duration*30; i := i+1)
		
		var weatherid := CStr(type);
		var id_len := len(weatherid);

		if( id_len == 0 )
			weatherid := "00";
		elseif( id_len == 1)
 			weatherid := "0" + weatherid;
 		endif 
	 	
	 	var packet := "65"+weatherid+"7000";
	 
        var fire_weapons := Array{0xC5FD, 0xC5FE, 0xC5FF, 0xC600, 0x89A7};
		foreach person in EnumerateOnlineCharacters()
            if (!GetObjProperty(person, "molhado"))
                SetObjProperty(person, "molhado", 1);
            endif
            var itemsinbag := EnumerateItemsInContainer(person.backpack);
            foreach item in itemsinbag
                if (item.objtype in fire_weapons)
                    SetObjProperty(item, "balas", 0);
                    SendSysMessageEx(person, "A polvora da sua arma molhou!", SSM_INFO);
                endif
            endforeach
			if (person.x < 4174)
				playweathersound(type, person);
				SendPacket(person, packet);
			endif
		endforeach
		
		sleep(5);
	endfor
	foreach person in EnumerateOnlineCharacters()
        EraseObjProperty(person, "molhado");
    endforeach
endfunction


function endweather()
	foreach player in enumerateonlinecharacters()
		if (player.x < 5120)
			moveobjecttolocation(player, player.x, player.y, player.z, player.realm);
		endif
	endforeach
	output("Stopping weather effects.");
endfunction


function SeasonRotate()
	var season := cint(getglobalproperty("season"));
	var daynumber := cint(getglobalproperty("daynumber"));
        var current_year := CINT(GetGlobalProperty("current_year"));
        
	if (!season)
		Season := 1;
		SetGlobalProperty("season", Season);
	endif
 
	if (!daynumber)
		daynumber := 1;
	endif
	
        if (!current_year)
           current_year := 230;
        endif
 
	daynumber := daynumber+1;
	if (daynumber > 360)
		daynumber := 1;
                current_year := current_year + 1;
	endif
	
	setglobalproperty("daynumber", daynumber);
        SetGlobalProperty("current_year", current_year);
	
	if (daynumber == 1)
		Broadcast("E verao.");
		setglobalproperty("season", 1);
		//start_script("seasonaleffects");
	elseif (daynumber == 91)
		Broadcast("As folhas das arvores comecam a cair.");
		setglobalproperty("season", 2);
		//start_script("seasonaleffects");
	elseif (daynumber == 180)
		Broadcast("O vento sopra gelado e a neve comeca a cair..");
		setglobalproperty("season", 3);
		//start_script("seasonaleffects");
	elseif (daynumber == 272)
		Broadcast("Os passaros cantam felizes e a vida selvagem e ambudante.");
		setglobalproperty("season", 5);
		//start_script("seasonaleffects");
	endif
endfunction

function moonfase()

  var fase := GetGlobalProperty("moonfase");
  var moon_period := GetGlobalProperty("moon_period");
  
  if (!moon_period)
    moon_period := 1;
  endif
  
  if (!fase)
     SetGlobalProperty("moonfase", "nova");
  endif

  moon_period := moon_period + 1;

  if (moon_period > 10)
    moon_period := 1;
    case (fase)
       "nova":      SetGlobalProperty("moonfase", "crescente");
       "crescente": SetGlobalProperty("moonfase", "cheia");
       "cheia":     SetGlobalProperty("moonfase", "minguante");
       "minguante": SetGlobalProperty("moonfase", "nova");
    endcase
  endif
  
  SetGlobalProperty("moon_period", moon_period);
  
endfunction

function GetMoonLightMod()

 var fase := GetGlobalProperty("moonfase");

    case (fase)
       "nova":      return 28;
       "crescente": return 25;
       "cheia":     return 20;
       "minguante": return 25;
    endcase

endfunction

function temperatura()
   var basetemp := CINT(GetGlobalproperty("basetemp"));
   if (basetemp < -20)
      basetemp := 20;
   endif
   if (basetemp > 40)
      basetemp := 20;
   endif
   if (!basetemp)
      basetemp := 0;
   endif
   var temp;
   var season := cint(getglobalproperty("season"));
   if ( season == 1)
      temp := 15 + randomint(20);
      if (basetemp == 0)
        basetemp := temp;
      endif
      if (basetemp < temp)
        basetemp := basetemp - 3;
      else
        basetemp := basetemp + 3;
      endif
   elseif ( season == 2)
      temp := 0 + randomint(25);
      if (basetemp == 0)
        basetemp := temp;
      endif
      if (basetemp < temp)
        basetemp := basetemp - 4;
      else
        basetemp := basetemp + 2;
      endif
   elseif (season == 3 )
      temp := -20 + randomint(40);
      if (basetemp == 0)
        basetemp := temp;
      endif
      if (basetemp < temp)
        basetemp := basetemp - 8;
      else
        basetemp := basetemp + 1;
      endif
   else
      temp := 0 + randomint(25);
      if (basetemp == 0)
        basetemp := basetemp;
      endif
      if (basetemp < temp)
        basetemp := basetemp - 4;
      else
        basetemp := temp + 2;
      endif
   endif
   
   setglobalproperty("basetemp", basetemp);

endfunction

function output(msgtext)
	foreach player in enumerateonlinecharacters()
		if (player.cmdlevel)
			SendSysMessage(player, "Weather debug:" + msgtext, 3, 87);
		endif
	endforeach
	print("Weather debug:" + msgtext);
endfunction
