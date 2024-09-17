
use uo;
use util;
use cfgfile;

include "include/damage";

// Calendar constants
var MONTHS := array{
    struct{ "name" := "Duwus", "days" := 31 },
    struct{ "name" := "Alorae", "days" := 30 },
    struct{ "name" := "Galgrumirius", "days" := 30 },
    struct{ "name" := "Kaius", "days" := 30 },
    struct{ "name" := "Idrithae", "days" := 30 },
    struct{ "name" := "Adarae", "days" := 30 },
    struct{ "name" := "Demiurgus", "days" := 33 },
    struct{ "name" := "Crassus", "days" := 32 },
    struct{ "name" := "Lleuadae", "days" := 31 },
    struct{ "name" := "Phrugum", "days" := 31 },
    struct{ "name" := "Igorsmonth", "days" := 31 },
    struct{ "name" := "Enzosmonth", "days" := 31 }
};

var WEEKDAYS := array{
    "Solaris", "Madredis", "Bellonis", "Magicae", "Fabricae", "Mercatus", "Lunaris"
};

var MOON_PHASES := array{
    "Kalendae", "Nonae", "Idus", "Decrescens"
};

var daysInYear := CalcDaysInYear();

// Season Id based on UO packet
const SUMMER := 1;
const AUTUMN := 2;
const WINTER := 3;
const SPRING := 4;

function getSeason()
    return CInt(GetGlobalProperty("season"));
endfunction

function CalcDaysInYear()
    var totalDays := 0;
    foreach month in MONTHS
        totalDays += month.days;
    endforeach
    return totalDays;
endfunction

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

function WhatSeason()
    var daynumber := getDay();
    if (daynumber > daysInYear*0)
        if (daynumber > daysInYear*0.25)
            if (daynumber > daysInYear*0.50)
                if (daynumber > daysInYear*0.75)
                    return SPRING;
                endif
                return WINTER;
            endif
            return AUTUMN;
        endif
        return SUMMER;
    endif
endfunction

function SeasonRotate()
    getCurrentDate(); // Usado aqui para checar se as props estão corretas e setar padrão caso não tenha.

    case (WhatSeason())
        
        SUMMER:
            Broadcast("O sol é mais forte que o comum...");
            SetGlobalProperty("season", SUMMER);
            break;

        AUTUMN:
            Broadcast("As folhas das arvores começam a cair...");
            SetGlobalProperty("season", AUTUMN);
            break;

        WINTER:
            Broadcast("O vento sopra gelado e a neve comeca a cair...");
            SetGlobalProperty("season", WINTER);
            break;

        SPRING:
            Broadcast("Os pássaros cantam felizes e a vida selvagem é abundante...");
            SetGlobalProperty("season", SPRING);
            break;
    endcase
endfunction

function MoonFase()
    var currentDate := getCurrentDate();
    SetGlobalProperty("moonfase", currentDate.moonPhase);
endfunction

function GetMoonLightMod()
    var fase := GetGlobalProperty("moonfase");

    case (fase)
       "Kalendae":    return 24;
       "Nonae":       return 21;
       "Idus":        return 16;
       "Decrescens":  return 21;
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
			SendSysMessageEx(player, "Weather debug:" + msgtext, 3, 87);
		endif
	endforeach
	print("Weather debug:" + msgtext);
endfunction

function getDay()
    var day := GetGlobalProperty("daynumber");

    if (!day)
        resetCalendar();
        day := GetGlobalProperty("daynumber");
    endif

    return CInt(day);
endfunction

function setDay(day)
    SetGlobalProperty("daynumber", day);
endfunction

function getDayOfMonth()
    var day := getDay();

    var dayCount := 0;
    foreach month in MONTHS
        if (day <= (dayCount + month.days))
            return (day - dayCount);
        endif
        dayCount += month.days;
    endforeach

    return "Error: Day out of range";
endfunction

function getMonth()
    var day := getDay();

    var dayCount := 0;
    foreach month in MONTHS
        dayCount += month.days;
        if (day <= dayCount)
            return month.name;
        endif
    endforeach

    return "Error: Day out of range";
endfunction

function getYear()
    var current_year := GetGlobalProperty("current_year");

    if (!current_year)
        resetCalendar();
        current_year := GetGlobalProperty("current_year");
    endif

    return CInt(current_year);
endfunction

function setYear(year)
    SetGlobalProperty("current_year", year);
endfunction

function advanceDay()
    var day := getDay();
    var year := getYear();

    day++;
    if (day > daysInYear)
        day := 1;
        year++;
    endif

    SetGlobalProperty("daynumber", day);
    SetGlobalProperty("current_year", year);
endfunction

function getWeekday()
    var day := getDay();
    if (!day)
        resetCalendar();
        day := getDay();
    endif

    return WEEKDAYS[(day - 1) % 7 + 1];
endfunction

function getMoonPhase()
    var day := getDay();
    var weeklength := WEEKDAYS.Size();
    var moonphases := MOON_PHASES.Size();

    var fase := day / weeklength;
    while (fase >= moonphases)
        fase := fase - moonphases;
    endwhile
    fase++;
    
    case (fase)
        1: return MOON_PHASES[1];
        2: return MOON_PHASES[2];
        3: return MOON_PHASES[3];
        4: return MOON_PHASES[4];
    endcase
endfunction

function resetCalendar()
    SetGlobalProperty("daynumber", 1);
    SetGlobalProperty("current_year", 456); // E.A. year - Dois anos antes da fase começar
endfunction

function getCurrentDate()
    var dayYear := getDay();
    var day := getDayOfMonth();
    var month := getMonth();
    var year := getYear();
    var weekday := getWeekday();
    var moonPhase := getMoonPhase();

    return struct{
        "month" := month,
        "day" := day,
        "weekday" := weekday,
        "moonPhase" := moonPhase
    };
endfunction

function updateCharacterAge(who)
    var chardata := GetObjProperty(who, "chardata");
    if (!chardata || !chardata.birthdate)
        return;
    endif

    var current_year := CINT(GetGlobalProperty("current_year"));
    var years_passed := current_year - chardata.birthdate.year;
    var new_age := chardata.initial_age + years_passed;

    if (new_age != chardata.current_age)
        chardata.current_age := new_age;
        SetObjProperty(who, "chardata", chardata);
    endif
endfunction