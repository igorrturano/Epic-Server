function getSwordSpellName(id)

	var cfg := ReadconfigFile(":spells:config/swordmage");
	var elem := FindConfigElem(cfg, id);

	var element := GetConfigString(elem, "Name");

	return element;

endfunction

function releasegems(who, sword)

	var gems := GetObjProperty(sword, "gems");
	foreach gem in (gems.keys())
		createiteminbackpack(who, gems[gem].objtype);
	endforeach

endfunction

function getElemental(id)

	var cfg := ReadconfigFile(":spells:config/swordmage");
	var elem := FindConfigElem(cfg, id);

	var elemental := GetConfigString(elem, "Elemental");

	return elemental;

endfunction
