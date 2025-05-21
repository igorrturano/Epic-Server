
use uo;
use util;
use cfgfile;

include ":combat:damage";
include ":commands:debugsystem";

// Calendar constants
var MONTHS := array{
    struct{ "name" := "Duwus", "days" := 30 }, // Mês dedicado ao Deus Sol - Björska
    struct{ "name" := "Alorae", "days" := 30 }, // Mês dedicado à magia, Alora, elfos.
    struct{ "name" := "Galgrumirius", "days" := 30 }, // Mês dedicado aos anões e a forja
    struct{ "name" := "Kaius", "days" := 30 }, // Mês dedicado ao Deus Dragão oriental
    struct{ "name" := "Idrithae", "days" := 30 }, // Deus dos drows, mês com noites mais longas e dias mais curtos
    struct{ "name" := "Lleuadae", "days" := 30 }, // Mês dedicado à Deusa Lua - Björska
    struct{ "name" := "Adarae", "days" := 30}, // Mês dedicado à Adar Kor Doom, Gulthrak, orcs
    struct{ "name" := "Crassus", "days" := 31 }, // Mês dedicado ao primeiro imperador, Crassus
    struct{ "name" := "Demiurgus", "days" := 33 }, // Mês dedicado ao Deus da Criação aiglano, Demiurgus. O Mês com mais dias
    struct{ "name" := "Phrugum", "days" := 30 }, // Mês dedicado aos polski e à colheita
    struct{ "name" := "Praetorus", "days" := 31 }, // Mês dedicado ao tribunal arcano e a justiça
    struct{ "name" := "Aeternum", "days" := 31 } // Mês dedicado à vitória eterna Aiglana, o recomeço
};

var WEEKDAYS := array{
    "Solaris", // Agricultura, masculinidade e descanso
    "Bellumis",  // Guerra, força e honra
    "Artisium",  // Criação, habilidade e arte
    "Aetheria", // Magia, conhecimento e sabedoria
    "Mercatus",  // Comércio, riqueza e prosperidade
    "Lunaris" // Vigília, feminilidade e espiritualidade
};

var MOON_PHASES := array{
    "Kalendae", // Lua nova
    "Nonae", // Crescente
    "Idus", // Cheia
    "Decrescens" // Minguante
};

var daysInYear := CalcDaysInYear();

// Season Id based on UO packet
const SUMMER := 1;
const AUTUMN := 2;
const WINTER := 3;
const SPRING := 5;

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
			if (!person.isUnderRoof())
				wetPowder(person);
            	SetWeatherForPlayer(person, type, 50);
			endif
            
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
	DebugMsgEvent(DEBUG_CLIMA, "Stopping weather effects.");
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
            Broadcast("O sol é mais forte nesta época do ano...");
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
// function output(msgtext)
// 	foreach player in enumerateonlinecharacters()
// 		if (player.cmdlevel)
// 			SendSysMessageEx(player, "Weather debug:" + msgtext, 3, 87);
// 		endif
// 	endforeach
// 	print("Weather debug:" + msgtext);
// endfunction

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

function wetPowder(who)
    SetObjProperty(who, "molhado", 1);
    SendSysMessageEx(who, "Você está molhado!", SSM_INFO);
    var fire_weapons := Array{0xC5FD, 0xC5FE, 0xC5FF, 0xC600, 0x89A7};

    var itemsinbag := EnumerateItemsInContainer(who.backpack);
    foreach item in itemsinbag
        var container := item.container;
        if ((item.objtype == 0x9092 && container.objtype != 0xDE31))
            SendSysMessageEx(who, "A chuva danificou parte de sua pólvora!", SSM_FAIL);
            SubtractAmount(item, RandomInt(5) + 1);
        endif
        if (item.objtype in fire_weapons)
            SetObjProperty(item, "balas", 0);
            SendSysMessageEx(who, "A pólvora da sua arma molhou!", SSM_FAIL);
        endif
    endforeach

    var items := ListItemsNearLocation(who.x, who.y, who.z, 20, who.realm);
    foreach item in items
        if (item.objtype == 0x9092)
            PrintTextAbove(item, "*Molhada*");
            SubtractAmount(item, RandomInt(5) + 1);
        endif
    endforeach
endfunction

// Calculate the game time using direct mapping
function getGameTime()
    var current_timestamp := ReadGameClock();
    var period_start_timestamp := CInt(GetGlobalProperty("halfdaytime"));
    var is_day := GetGlobalProperty("dia") == "dia";

    // Get the actual period durations from Global Properties
    var day_seconds_prop := CInt(GetGlobalProperty("CurrentActualDaySeconds"));
    var night_seconds_prop := CInt(GetGlobalProperty("CurrentActualNightSeconds"));

    var day_seconds_calc; // Will hold the value used for calculation
    var night_seconds_calc; // Will hold the value used for calculation

    // Determine if fallbacks are needed for the current period
    if (is_day)
        if (!day_seconds_prop)
            print("Warning: getGameTime using fallback for DAY seconds.");
            var config := ReadConfigFile(":nature:config/daynight");
            day_seconds_calc := CInt(config.settings.daylength) * 60;
        else
            day_seconds_calc := day_seconds_prop;
        endif
        // night_seconds_calc is not strictly needed for day period calculation logic below,
        // but ensure it's initialized if used elsewhere within this scope.
        night_seconds_calc := night_seconds_prop; // Or a fallback if it's also needed for some reason
    else // is_night
        if (!night_seconds_prop)
            print("Warning: getGameTime using fallback for NIGHT seconds.");
            var config := ReadConfigFile(":nature:config/daynight");
            if (config.settings.exists("nightlength"))
                night_seconds_calc := CInt(config.settings.nightlength) * 60;
            else
                // Fallback to daylength for night if nightlength is not specified in config
                night_seconds_calc := CInt(config.settings.daylength) * 60;
            endif
        else
            night_seconds_calc := night_seconds_prop;
        endif
        // day_seconds_calc is not strictly needed for night period calculation logic below.
        day_seconds_calc := day_seconds_prop; // Or a fallback
    endif
    
    // Time elapsed since period started
    var elapsed_seconds := current_timestamp - period_start_timestamp;
    if (elapsed_seconds < 0) 
        elapsed_seconds := 0; 
    endif
    
    // Each hour spans exactly 1/12 of the total period
    var seconds_per_hour;
    var start_hour;
    
    if (is_day)
        // Day period: 6:00 to 18:00
        // Ensure day_seconds_calc has a valid value (even if it's a fallback from above)
        if (!day_seconds_calc) // Should ideally not happen if fallback worked
            print("Critical Error: day_seconds_calc is zero during day in getGameTime!");
            day_seconds_calc := 3600 * 12; // A long default to avoid division by zero
        endif
        seconds_per_hour := day_seconds_calc / 12.0;
        start_hour := 6;
    else // is_night
        // Night period: 18:00 to 6:00
        // Ensure night_seconds_calc has a valid value
        if (!night_seconds_calc) // Should ideally not happen if fallback worked
            print("Critical Error: night_seconds_calc is zero during night in getGameTime!");
            night_seconds_calc := 3600*12; // A long default
        endif
        seconds_per_hour := night_seconds_calc / 12.0;
        start_hour := 18;
    endif
    
    // Calculate how many hours have passed since the period started
    var hours_elapsed := 0.0; // Default to float
    if (seconds_per_hour > 0) // Avoid division by zero
        hours_elapsed := CDbl(elapsed_seconds) / seconds_per_hour;
    else
        print("Critical Error: seconds_per_hour is zero in getGameTime!");
    endif
    
    // Calculate the current hour and minute
    var total_hours := start_hour + CInt(hours_elapsed);
    var hour_fraction := hours_elapsed - CInt(hours_elapsed);
    var minute := CInt(hour_fraction * 60.0);
    
    // Handle wrapping past midnight for night period
    if (total_hours >= 24)
        total_hours := total_hours - 24;
    endif
    
    return struct{
        "hour" := total_hours,
        "minute" := minute
    };
endfunction

function getCurrentHour()
    var current_time := getGameTime();
    return current_time.hour;
endfunction

function getCurrentMinute()
    var current_time := getGameTime();
    var current_minute := current_time.minute;
    return current_minute;
endfunction

// Format minute to always show two digits (e.g., "05" instead of "5")
function formatMinute(minute)
    if (minute < 10)
        return "0" + minute;
    else
        return "" + minute;
    endif
endfunction

function isDaylight()
    // Only check if the game system says it's day, don't rely on calculated hour
    return (GetGlobalProperty("dia") == "dia");
endfunction

// Get descriptive text for sun position based on hour
function getSunPosition(tempestade := 0)
    var hour := getCurrentHour();
    var is_day := GetGlobalProperty("dia");
    
    if (is_day != "dia")
        return "O sol não está visível.";
    endif
    
    if (hour >= 6 && hour < 9)
        return "Parece ter nascido há pouco";
    elseif (hour >= 9 && hour < 12)
        return "Caminhando ao topo do céu";
    elseif (hour >= 12 && hour < 16)
        return "A Pino";
    elseif (hour >= 16 && hour < 18)
        return "Caminhando ao horizonte";
    elseif (hour == 18)
        return "Quase se pondo";
    elseif ((tempestade == 0) || (tempestade == 1) || (tempestade == 2))
        return "A tempestade atrapalha a visão do sol";
    else
        return "O sol não está visível.";
    endif
endfunction

// Get the appropriate sundial graphic based on time of day
function getSundialGraphic()
    var is_day := GetGlobalProperty("dia");
    var hour := getCurrentHour();
    if (is_day == "dia")
        // Day graphics - map 6:00-17:59 to graphics 0x9BBF-0x9BC4, 0x9BBA-0x9BBE
        var offset := (hour - 6 + 5) % 12;
        var sundial_graphic := 0x9BBA + offset;
        
        // Safety check
        if (offset >= 12)
            sundial_graphic := 0x9BC5;
        endif
        
        return sundial_graphic;
    else
        // Nighttime - use night graphic (always 0x9BC6 during night)
        return 0x9BC6;
    endif
endfunction

function GetSeasonName(season_id)
    case (season_id)
        SUMMER: return "Verão";
        AUTUMN: return "Outono";
        WINTER: return "Inverno";
        SPRING: return "Primavera";
        default: return "Desconhecida";
    endcase
endfunction