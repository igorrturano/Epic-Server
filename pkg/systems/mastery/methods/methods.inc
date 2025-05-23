print("INSTALLING: Methods for Mastery System... OK!");

exported function GetMasteryProp(who)
	var prop := who.GetProp("Masteries");

	if (!prop) 
		prop := dictionary;
	endif

	return prop;
endfunction

exported function SetMasteryProp(who, prop)
	who.SetProp("Masteries", prop);
endfunction