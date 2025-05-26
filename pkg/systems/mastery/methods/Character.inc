print("INSTALLING: Methods for Character -> Mastery System... OK!");

exported function GetMastery(who)
	var prop := who.GetProp("Masteries") ?: dictionary{};

	return prop;
endfunction

exported function SetMastery(who, masteryObj)
	var prop := masteryObj.Serialize();
	who.SetProp("Masteries", prop);
endfunction