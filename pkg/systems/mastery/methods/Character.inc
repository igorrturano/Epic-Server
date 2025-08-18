include ":mastery:PropTools";

print("INSTALLING: Methods for Character -> Mastery System... OK!");

exported function GetMastery(who, key)
	return LoadMastery(who, key);
endfunction

exported function GetMasteryProp(who)
	return who.GetProp("Masteries");
endfunction

exported function SetMastery(who, object)
	return SaveMastery(who, object);
endfunction