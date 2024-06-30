
use uo;
use os;
use cfgfile;


function DP_GetSettingsCfgFile()
	var cfg := ReadConfigFile(":death:config/playerdeath");

	if ( cfg.errortext )
		SysLog("Error::DP_GetSettingsCfgFile() - Unable to open [:death:config/playerdeath.cfg] ->"+cfg.errortext);
	endif

	return cfg;
endfunction

function DP_GetSettingsCfgElem(elem_name, byref cfg_file:=0)

	if ( !cfg_file )
		cfg_file := DP_GetSettingsCfgFile();
	endif
	
	var elem := cfg_file[elem_name];

	if ( elem.errortext )
		SysLog("Error::DP_GetSettingsCfgElem() - Unable to find elem ["+elem_name+"] ->"+elem.errortext);
	endif

	return elem;
endfunction

function DP_GetMaxTime()

	var elem := DP_GetSettingsCfgElem("Settings");

	return elem.MaxTime;

endfunction

function DP_GetVitalsRegen()

	var elem := DP_GetSettingsCfgElem("Settings");

	var tegen := GetConfigReal(elem, "VitalsRegen");

	return tegen;

endfunction

function DP_GetDeathZone()

	var elem := DP_GetSettingsCfgElem("Settings");

	var tegen := GetConfigString(elem, "MandosHall");

	return tegen;

endfunction
