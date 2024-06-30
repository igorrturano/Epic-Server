


function getSwordSpellName(id)

	var cfg := ReadconfigFile(":spells:config/allspells");
	var elem := FindConfigElem(cfg, id);

	var element := GetConfigString(elem, "Name");

	return element;

endfunction

function Elementalista(who, spellinfo)
	var elementalist := GetObjProperty(who, "elemental");
	if (elementalist == error)
		return 0;
	endif

	var elemento := getElemental(spellinfo.id);
	
	if (elemento == elementalist)
		return 1;
	endif

endfunction

//retorna 1 se tiver com a arma escolhida equipada, 0 se nao tiver
function Swordmageweapon(who)
		if (GetObjProperty(who.weapon, "swordmage") ==  who.serial)
			return 1;
		else
			return 0;
		endif
endfunction

function FundamentalAtiva(who)

	var elemental := GetObjProperty(who.weapon, "#elementalweapon");

	if (GetObjProperty(who.weapon, "armaespiritual") != error)
		return 1;
	endif

	if (elemental != error)
		return 1;
	endif

	return 0;

endfunction

function GetElementalType(who)

	if (!GetObjProperty(who.weapon, "armaespiritual") != error)
		var info := GetObjProperty(who.weapon, "armaespiritual");
		return info.type;
	endif


	var elemental := GetObjProperty(who.weapon, "#elementalweapon");
	if (elemental != error)
		return elemental.type;
	endif

	if ( GetCooldown(who.weapon, "blessweapon") > 0 )
		return 0;
	endif

	return 0;

endfunction

function CriticoElemental(attacker, defender, armor, basedamage, rawdamage)

	if (Swordmageweapon(attacker))
		if (FundamentalAtiva(attacker))			
			var type := GetElementalType(attacker);
	
			if (type == DMG_FIRE)
				start_script(":combat:weaponHitScripts/FireHit", {attacker, defender, attacker.weapon, armor, basedamage, rawdamage*3});
			elseif (type == DMG_COLD)
				start_script(":combat:weaponHitScripts/IceHit", {attacker, defender, attacker.weapon, armor, basedamage, rawdamage*3});
			elseif( type == DMG_ENERGY)
				start_script(":combat:weaponHitScripts/LightningHit", {attacker, defender, attacker.weapon, armor, basedamage, rawdamage*3});
			endif
		endif
	endif

endfunction

function releasegems(who, sword)

	var gems := GetObjProperty(sword, "gems");
	foreach gem in (gems.keys())
		createiteminbackpack(who, gems[gem].objtype);
	endforeach

endfunction 

function getElemental(id)

	var cfg := ReadconfigFile(":spells:config/allspells");
	var elem := FindConfigElem(cfg, id);

	var elemental := GetConfigString(elem, "Elemental");

	return elemental;

endfunction
