use uo;
use cfgfile;

function getnpccategory(template)

	var npccfgfile := ReadConfigFile(":*:npcdesc");
	
	var category := npccfgfile[template].Category;	
	
	if (!category)
		return "sem categoria";
	else
		return category;
	endif
	
	unloadConfigFile(":*:npcdesc");

endfunction

function getnpcname(who, template)

	var npccfgfile := ReadConfigFile(":*:npcdesc");
	
	var name := CStr(npccfgfile[template].Name);	
	
	return name;

	
	unloadConfigFile(":*:npcdesc");

endfunction