/*
 * $Id$
 *
 */

use uo;
use cfgfile;
use os;

CONST NO_COOLDOWNS			:= 0x1;

// Event flags
CONST EVENT_ADD_TIMER			:= 0x1;
CONST EVENT_LOWER_TIMER			:= 0x2;
CONST EVENT_PLAYER_DIED			:= 0x4;
CONST EVENT_NPC_DIED			:= 0x8;
CONST EVENT_LOGOFF			:= 0x16;

CONST EVENT_TYPE_COOLDOWN		:= 0x32;

// Constants
CONST NOSTART_CONTROLLER		:= 0x0;
CONST START_CONTROLLER			:= 0x1;

/*
 * TS_GetCfgFile()
 *
 * Purpose
 * Retrieves timedScripts.cfg
 *
 * Parameters
 * None
 *
 * Return Value
 * Config file
 *
 */
function TS_GetCfgFile()
	var cfg := ReadConfigFile(":timedscripts:timedScripts");

	if ( cfg.errortext )
		print("Error::TS_GetSettingsCfgFile() - Unable to open [:timedscripts:timedScripts] ->"+cfg.errortext);
	endif

	return cfg;
endfunction

/*
 * TS_GetCfgElem(elem_name, cfg_file)
 *
 * Purpose
 * Retrieves an elem from a config file.
 *
 * Parameters
 * elem_name:	A string matching the elem name to be retrieved.
 * cfg_file:	Optional parameter - reference to a config already read in by TS_GetCfgFile()
 *
 * Return value
 * A config file elem.
 *
 */
function TS_GetCfgElem(elem_name, byref cfg_file:=0)
	if ( !cfg_file )
		cfg_file := TS_GetCfgFile();
	endif

	var elem := cfg_file[elem_name];

	if ( elem.errortext )
		print("Error::TS_GetCfgElem - Unable to find elem ["+elem_name+"] ->"+elem.errortext);
	endif

	return elem;
endfunction

/*
 * TS_GetControllerProcess(mobile, start)
 *
 * Purpose
 * Retrieves a script reference to the timer controller for a mobile.
 *
 * Parameters
 * mobile:	Mobile to get the timer controll process for.
 * start:	Optional parameter. Default is NOSTART_CONTROLLER.
 *		NOSTART_CONTROLLER - Will only try to get a reference to the script.
 *		START_CONTROLLER - Will start the controller if it is not running.
 *
 * Return value
 * Returns a script reference.
 * Returns an error the controller is not running.
 *
 */
function TS_GetControllerProcess(mobile, start:=NOSTART_CONTROLLER)
	var pid := CInt(GetObjProperty(mobile, "#TimerControlPID"));
	var process := GetProcess(pid);
	if ( !process )
		if ( start )
			process := TS_StartController(mobile);
		endif
	endif

	return process;
endfunction

/*
 * TS_StartController(mobile)
 *
 * Purpose
 * Starts the timer controller for a mobile.
 *
 * Parameters
 * mobile:	Mobile to start the controller on.
 *
 * Return value
 * Returns a script reference.
 * Returns error on failure.
 *
 */
function TS_StartController(mobile)
	if ( TS_GetControllerProcess(mobile, NOSTART_CONTROLLER) )
		return error{"errortext":="Error::TS_StartController() - Controller is already running."};
	endif

	var script := Start_Script(":timedscripts:timerControl", mobile);
	if ( script.errortext )
		print("Error::TS_StartController() ->"+script.errortext);
	endif

	return script;
endfunction

/*
 * TS_GetTimerList(mobile)
 *
 * Purpose
 * Retrieves the timer_list dictionary on the mobile.
 *
 * Parameters
 * mobile:	Mobile to retrieve the timer list from.
 *
 * Return value
 * Returns a dictionary
 *
 */
function TS_GetTimerList(mobile)
	var timer_list := GetObjProperty(mobile, "%TimerList%");
	if ( Lower(TypeOf(timer_list)) != "dictionary" )
		timer_list := dictionary;
	endif

	return timer_list;
endfunction

function TS_GetCooldownList(mobile)
	var timer_list := GetObjProperty(mobile, "%CooldownList%");
	if ( Lower(TypeOf(timer_list)) != "dictionary" )
		timer_list := dictionary;
	endif

	return timer_list;
endfunction


/*
 * TS_SaveTimerList(mobile, timer_list)
 *
 * Purpose
 * Saves the timer_list dictionary on the mobile.
 *
 * Parameters
 * mobile:	Mobile to save the list on.
 * timer_list:	Dictionary timer_list to save.
 *
 */
function TS_SaveTimerList(mobile, byref timer_list)
	return SetObjProperty(mobile, "%TimerList%", timer_list);
endfunction

function TS_SaveCooldownList(mobile, byref timer_list)
	return SetObjProperty(mobile, "%CooldownList%", timer_list);
endfunction

/*
 * TS_GetTimerNames(mobile, type:="")
 *
 * Purpose
 * Retrieves a list of names of timed scripts running on a mobile.
 *
 * Parameters
 * mobile:	Mobile to get the list from.
 * type:	Optional parameter - Will only list timers of a certain type.
 *		If left unset, all will be retuned.
 *
 * Return value
 * Array of strings
 *
 */
function TS_GetTimerNames(mobile, type:="")
	type := Lower(type);
	var timer_list := TS_GetTimerList(mobile);
	var name_list := array{};

	foreach timer_name in ( timer_list.Keys() )
		if ( type )
			var elem_info := TS_GetCfgElem(timer_name);
			if ( elem_info.errortext )
				print("Error::TS_GetTimerNames() - Unable to find config elem ["+timer_name+"] - "+elem_info.errortext);
				return 0;
			elseif ( Lower(elem_info.Type) == type )
				name_list.Append(timer_name);
			endif
		else
			name_list.Append(timer_name);
		endif
		SleepMS(2);
	endforeach

	return name_list;
endfunction


/*
 * Retorna o nome em portugues do buff e apenas das magias
 */
function GetMagicTimersSpellName(mobile)

	var timer_list := TS_GetTimerList(mobile);
	var names := array;

	foreach timer in ( timer_list.keys() )
		timer := lower(timer);
		var t := struct;
		if ( timer == "hallucination")
			t.+nome := "alucinacoes";
		elseif (timer == "paralysis")
			t.+nome := "paralisia";
		elseif (timer == "weaken" )
			t.+nome := "enfraquecer";
		elseif (timer == "agility" )
			t.+nome := "agilidade";
		elseif ( timer == "strength");
			t.+nome := "forca";
		elseif ( timer == "intelligence")
			t.+nome := "inteligencia";
		elseif ( timer == "velocidade")
			t.+nome := "velocidade";
		elseif ( timer == "lentidao")
			t.+nome := "lentidao";
		elseif ( timer == "PhysicalResist")
			t.+nome := "protecao fisica";
		elseif ( timer == "bless")
			t.+nome := "bencao";
		elseif ( timer == "curse")
			t.+nome := "maldicao";
		elseif ( timer == "phmod")
			t.+nome := "protecao fisica";
		elseif (timer == "fimod")
			t.+nome := "protecao ao fogo";
		elseif (timer == "comod")
			t.+nome := "protecao ao frio";
		elseif (timer == "enmod")
			t.+nome := "protecao a energia";
		elseif ( timer == "pomod")
			t.+nome := "resistencia a veneno";
		elseif ( timer == "disfarce")
			t.+nome := "disfarce";
		elseif ( timer == "gloria")
			t.+nome := "gloria";
		elseif (timer == "desarmar")
			t.+nome := "desarmar";
		elseif (timer == "desviarprojetil")
			t.+nome := "desviar projetil";
		elseif (timer == "dmgreduction")
			t.+nome := "pacifismo";
        elseif (timer == "dmgreturn")
			t.+nome := "Retribuicao Profana";
		endif
		if (t.nome != error)
			names.append(t);
		endif
	endforeach

	if (GetObjProperty(mobile, "tirocerteiro") )
		var t := struct;
		t.+nome := "tiro certeiro";
		names.append(t);
	endif

	if ( GetObjProperty(mobile, "evasao"))
		var t := struct;
		t.+nome := "evasao";
		names.append(t);
	endif

	return names;

endfunction

/*
 * TS_GetTimerInfo(mobile, timer_name)
 *
 * Purpose
 * Retrieves information on a *running* timer on a mobile.
 *
 * Parameters
 * mobile:	Mobile to find the timer on.
 * timer_name:	Name of the timer (elem name from timedScripts.cfg) to find.
 *
 * Return value
 * Returns a struct or error
 *
 */
function TS_GetTimerInfo(mobile, timer_name)
	var timer_list := TS_GetTimerList(mobile);
	if ( timer_list.Exists(timer_name) )
		return timer_list[timer_name];
	else
		return error{"errortext":="Error::TS_GetTimerInfo() - No timer exists by that name."};
	endif
endfunction

/*
 * TS_GetTimerDuration(mobile, timer_name)
 *
 * Purpose
 * Gets the number of seconds a timer has left to run.
 *
 * Parameters
 * mobile:	Mobile to find the timer on.
 * timer_name:	Name of the timer (elem name from timedScripts.cfg) to find.
 *
 * Return value
 * Returns an integer on success
 * Returns an error on failure
 *
 */
function TS_GetTimerDuration(mobile, timer_name)
	var timer := TS_GetTimerInfo(mobile, timer_name);
	if ( !timer )
		return timer; // Returns the error
	endif

	return (timer.endtime - ReadGameClock());
endfunction

/*
 * TS_StartTimer(mobile, timer_name, duration, level, attacker)
 *
 * Purpose
 * Starts a timed script on a mobile. If it is already running, it adds to the time it runs.
 *
 * Parameters
 * mobile:	Mobile to start the timed script on.
 * timer_name:	Name of the timer (elem name from timedScripts.cfg) to start.
 * duration:	Number of seconds to add to the timer's run time.
 * attacker:	Optional - Sets an attacker.
 * level:	Optional - Sets the level of the timer (for poisons)
 *
 * Return value
 * Returns 1 on success.
 * Returns error on failure.
 *
 */
function TS_StartTimer(mobile, timer_name, duration, level:=0, attacker:=0, mantain := 0)
	duration := CInt(duration);
	if ( duration < 1 )
		return error{"errortext":="Error::TS_StartTimer() - Duration must be greater than 0."};
	//elseif ( duration > 250 )
	//	duration := 250;
	endif

	var event := struct;
	event.+type := EVENT_ADD_TIMER;
	event.+name := CStr(timer_name);
	event.+amount := duration;
	event.+level := level;
	if (mantain)
	//	sendsysmessage(mobile, " attacker" + attacker );
		event.+attacker := attacker;
		event.+mage := attacker;
	else
		event.+attacker := attacker;
	endif
	
	//SendSysMessage(mobile, "attacker " + attacker);

	event.+mantain := mantain;

	var process := TS_GetControllerProcess(mobile, START_CONTROLLER);
	return process.SendEvent(event);
endfunction

function TS_StartCooldown(mobile, timer_name, duration, attacker := 0)
	duration := CInt(duration);
	if ( duration < 1 )
		return error{"errortext":="Error::TS_StartTimer() - Duration must be greater than 0."};
	endif

	var event := struct;
	event.+type := EVENT_ADD_TIMER;
	event.+timerType := EVENT_TYPE_COOLDOWN;
	event.+name := CStr(timer_name);
	event.+amount := duration;
	event.+attacker := attacker;

	var process := TS_GetControllerProcess(mobile, START_CONTROLLER);
	return process.SendEvent(event);
endfunction

function TS_GetCooldown(mobile, cooldown)

	var timers := TS_GetCooldownList(mobile);

	return timers[cooldown];

endfunction

/*
 * TS_LowerDuration(mobile, timer_name, amount)
 *
 * Purpose
 * Lowers the amount of time a timer will run.
 * If remaining time is <= 0, it will turn the timer off.
 * If -1 is passed for amount, it will turn the timer off.
 *
 * Parameters
 * mobile:	Mobile to lower the timed script on.
 * timer_name:	Name of the timer (elem name from timedScripts.cfg).
 * duration:	Number of seconds lower the run time of the timer.
 *		-1 will stop it completely.
 *
 * Return value
 * Returns 1 on success.
 * Returns error on failure.
 *
 */
function TS_LowerDuration(mobile, timer_name, amount, cooldown := 0)
	var event := struct;
	event.+type := EVENT_LOWER_TIMER;
	event.+name := CStr(timer_name);
	event.+amount := CInt(amount);
	if (cooldown)
		event.timerType := EVENT_TYPE_COOLDOWN;
	endif

	var process := TS_GetControllerProcess(mobile);
	return process.SendEvent(event);
endfunction

/*
 * TS_LogOff(mobile)
 *
 * Purpose
 * Informs the timer controller that a player logged out.
 *
 * Parameters
 * mobile:	Mobile that logged off
 *
 * Return value
 * Returns 1 on success
 * Returns 0 on failure
 *
 */
function TS_LogOff(mobile)
	if ( mobile.IsA(POLCLASS_NPC) )
		return error{"errortext":="NPCs can't log off."};
	endif

	var event := struct;
	event.+type := EVENT_LOGOFF;

	var process := TS_GetControllerProcess(mobile);
	return process.SendEvent(event);
endfunction

/*
 * TS_Death(mobile)
 *
 * Purpose
 * Informs the timer controller that a moble died.
 *
 * Parameters
 * mobile:	Mobile that died.
 *
 * Return value
 * Returns 1 on success
 * Returns 0 on failure
 *
 */
function TS_Death(mobile)
	var event := struct;
	if ( mobile.IsA(POLCLASS_MOBILE) )
		event.+type := EVENT_PLAYER_DIED;
	elseif ( mobile.IsA(POLCLASS_CORPSE) )
		event.+type := EVENT_NPC_DIED;
	endif

	var process := TS_GetControllerProcess(mobile);
	return process.SendEvent(event);
endfunction
