use uo;
use cfgfile;

function getnpccategory(template)

	var npccfgfile := ReadConfigFile(":brainai:npcdesc");
	
	var category := npccfgfile[template].Category;	
	
	if (!category)
		return "sem categoria";
	else
		return category;
	endif
	
	unloadConfigFile(":brainai:npcdesc");

endfunction

function getnpcname(who, template)

	var npccfgfile := ReadConfigFile(":brainai:npcdesc");
	
	var name := CStr(npccfgfile[template].Name);	
	
	return name;

	
	unloadConfigFile(":brainai:npcdesc");

endfunction