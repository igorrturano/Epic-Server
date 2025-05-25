print("INSTALLING: Methods for Character -> Mastery System... OK!");

exported function GetMasteryProp(who)
	var prop := who.GetProp("Masteries") ?: dictionary{};

	return prop;
endfunction

exported function SetMasteryProp(who, prop)
	who.SetProp("Masteries", prop);
endfunction