use uo;
use os;
use cfgfile;

function FarmingGetSettingsCfgFile()
	var cfg := ReadConfigFile(":farming:config/settings");
	
	if ( cfg.errortext )
		SysLog("Error - Unable to open :farming:config/settings.cfg ->"+cfg.errortext);
	endif
	
	return cfg;
endfunction

function FarmingGetSettingsCfgElem(elem_name)
	var cfg := FarmingGetSettingsCfgFile();
	var elem := cfg[elem_name];
	
	if ( elem.errortext )
		SysLog("Error - Unable to find elem ["+elem_name+"] in :farming:config/settings.cfg ->"+elem.errortext);
	endif
	
	return elem;
endfunction
