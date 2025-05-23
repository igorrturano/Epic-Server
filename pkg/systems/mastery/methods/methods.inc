print("INSTALLING: Methods for MasterySystem... OK!");

exported function GetMasteryProp(who)
	print("GetMasteryProp called");
	var prop := who.GetProp("MasterySystem");

	if (!prop) 
		prop := dictionary;
	endif

	return prop;
endfunction

exported function SetMasteryProp(who, prop)
	print("SetMasteryProp called");
	who.SetProp("MasterySystem", prop);
endfunction