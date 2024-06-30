use cfgfile;

function CreateItemFromRecipe(who, index)

	var cfg := ReadConfigFile(":recipes:recipes");

	var elem := FindConfigElem(cfg, index);
	if (!elem)
		return 0;
	endif

	var objtype := GetConfigInt(elem, "objtype");
	var item := CreateItemInBackpack(who, objtype, 1);
	if (item == error)
		SendSysMessage(who, "error " + error);
		return item;
	endif

	var name := GetConfigString(elem, "name");
	if (name != error)
		SetName(item, name);
	endif

	var color := GetconfigInt(elem, "color");
	if (color != error)
		item.color := color;
	endif

	var graphic := GetConfigInt(elem, "graphic");
	if (graphic != error)
		item.graphic := graphic;
	endif

	var equips := GetConfigStringDictionary(elem, "equip");
	if (equips != error)
		SetObjProperty(item, "equip", equips);
	endif

	item.resist_cold_mod := item.resist_cold_mod + cint(GetConfigInt(elem, "ColdResistMod"));
	item.resist_fire_mod := item.resist_fire_mod + cint(GetConfigInt(elem, "FireResistMod"));
	item.resist_poison_mod := item.resist_poison_mod + cint(GetConfigInt(elem, "PoisonResistMod"));
	item.resist_energy_mod := item.resist_energy_mod + cint(GetConfigInt(elem, "EnergyResistMod"));
	item.resist_physical_mod := item.resist_physical_mod + cint(GetConfigInt(elem, "PhysicalResistMod"));

	var dmg_mod := GetConfigInt(elem, "dmg_mod");
	item.dmg_mod := dmg_mod;

	var hp_mod := GetConfigInt(elem, "hp_mod");
	item.hp_mod := hp_mod;

	return item;

endfunction

function CraftRecipe(who, index)


	//SendSysMessage(who, "Fabricando um item.. ");
	if (index < 0)
		index := index*-1;
	endif


	var cfg := ReadConfigFile(":recipes:craft-recipes");

	var elem := FindConfigElem(cfg, index);
	if (!elem)
		SendSysMessage(who, "Cfg elem not found " + index);
		return 0;
	endif

	var craft_skill := lower(GetConfigString(elem, "mainskill"));
	var diff := GetConfigInt(elem, "difficulty");
	var points := GetConfigInt(elem, "points");
	var itemindex := GetconfigInt(elem, "itemindex");
	var materials := GetConfigStringDictionary(elem, "materials");
	var principalMaterial := GetConfigString(elem, "principalMaterial"); //o material q vai dar bonus
	var chosenPrincipalMaterial := 0;
	var theitem;

	var craft_sound  := menu_config[craft_skill].CraftSound;
	var craft_loops  := menu_config[craft_skill].CraftLoops;
	var craft_delay  := menu_config[craft_skill].CraftDelay;
	var craft_action := menu_config[craft_skill].CraftAnim;

	//SendSysMessage(who, index + " " + craft_skill + " " + diff + " " + points + " " + itemindex );

	var targMaterials := dictionary;
		//sendsysmessage(who,  " " + index);
		//sendsysmessage(who,  " " + materials);
	foreach thing in (materials.keys())
	//	sendsysmessage(who, " " + thing);
		if (thing == "metal" || thing == "wood" || thing == "glass"
			|| thing == "cloth" || thing == "leather" || thing == "stone" || thing == "ossos")
			var ret := MaterialSelection(who, craft_skill, thing, 0);
			if (!ret)
				return 0;
			else
				if (ret[1].amount < materials[thing] )
					SendSysMessageEx(who, "Voce nao possui os materiais necessarios." , SSM_FAIL);
					return 0;
				else
					targMaterials[ret[1].objtype] := materials[thing];
					if (thing == principalMaterial)
						chosenPrincipalMaterial := ret[1].objtype;
					endif
				endif
			endif
		else
			foreach item in (EnumerateItemsInContainer(who.backpack))
					//sendsysmessage(who, " " + item.objtype + " " + cint(thing));
				if (item.objtype == cint(thing))
					if (targMaterials[thing])
						targMaterials[thing] := targMaterials[thing] + item.amount;
					else
						targMaterials[thing] :=  item.amount;
					endif
				endif
			endforeach
			if ( targMaterials[thing] < materials[thing])
				SendSysMessageEx(who, "Voce nao possui os materiais necessarios." + targMaterials[thing] + " " + materials[thing] , SSM_FAIL);
				return 0;
			else
				targMaterials[thing] := materials[thing];
			endif
		endif

	endforeach

	//no loop anterior procurou os materiais, agora consome.
	foreach thing in (targMaterials.keys())
			foreach item in (EnumerateItemsInContainer(who.backpack))
				if (cint(item.objtype) == cint(thing))
					if (item.amount >= targMaterials[thing] )
						var chk := SubtractAmount(item, cint(targMaterials[thing]));
						if (!chk )
							SendSysMessage(who, "chk " + chk);
						endif
						targMaterials[thing] := 0;
					else
						targMaterials[thing] := targMaterials[thing] - item.amount;
						SubtractAmount(item, item.amount);
					endif
				endif
			endforeach
			if (targMaterials[thing] > 0)
				SendSysMessageEx(who, "Voce nao possui todos os materiais.", SSM_FAIL);
				return 0;
			endif

	endforeach

	var check := Skillcheck(who, craft_skill, diff, points) ;

	if(check > 0)
		playCraftWork(who, craft_sound, craft_loops, craft_delay, craft_action);
		theitem := CreateItemFromRecipe(who, itemindex);
		if(!theitem)
			SendSysMessageEx(who, "Voce nao tem espaco suficiente na sua mochila!", SSM_FAIL);
			return 0;
		endif
		//CheckToolWear (who, tool, craft_skill);
		theitem.movable := 1;
		SendSysMessageEx(who, "Voce criou o item o colocou na mochila.", SSM_INFO);
		if(theitem.isA(POLCLASS_CONTAINER))
			SetObjProperty(theitem, "trappable", 1);
		endif
	else
		SendSysMessageEx(who, "Voce falhou ao fazer o item e perdeu material." , SSM_FAIL);
		return 0;
	endif

	if (chosenPrincipalMaterial)
		var elem    := FindConfigElem(resource_config, chosenPrincipalMaterial);
    		if (elem)
      			var DmgMods := GetConfigString(elem, "AoSDamageMod");
      			DmgMods := SplitWords(DmgMods);
      			var ResistMods := GetConfigString(elem, "AoSResistMod");
      			ResistMods := SplitWords(ResistMods);
     	 		var hpmod := GetConfigInt(elem, "hpmod");
			if(theitem.isA(POLCLASS_WEAPON))
				var finaldmg := dictionary;
				finaldmg[DMG_PHYSICAL] := Cdbl(DmgMods[1]);
				finaldmg[DMG_FIRE] := Cdbl(DmgMods[2]);
				finaldmg[DMG_COLD] := Cdbl(DmgMods[3]);
				finaldmg[DMG_POISON] := Cdbl(DmgMods[4]);
				finaldmg[DMG_ENERGY] := Cdbl(DmgMods[5]);
				SetObjProperty(theitem, "DamageTypes", finaldmg );
			elseif(theitem.isA(POLCLASS_ARMOR))
			        var itemresist := GetObjProperty(theitem, "Resistances");
				var finalresist := dictionary;
				finalresist[DMG_PHYSICAL] := Cint(ResistMods[1])+ Cint(itemresist[DMG_PHYSICAL]);
				finalresist[DMG_FIRE] := Cint(ResistMods[2]) + Cint(itemresist[DMG_FIRE]);
				finalresist[DMG_COLD] := Cint(ResistMods[3]) + Cint(itemresist[DMG_COLD]);
				finalresist[DMG_POISON] := Cint(ResistMods[4]) +  Cint(itemresist[DMG_POISON]);
				finalresist[DMG_ENERGY] := Cint(ResistMods[5]) +  Cint(itemresist[DMG_ENERGY]);
				SetObjProperty(theitem, "Resistances", finalresist );
			endif
		endif
	endif

	return theitem;

endfunction

function GetRecipeList(who, skill)

    var itens := GetObjProperty(who,  "recipes"); //pega a lista de recipes
    var df := ReadConfigFile(":recipes:craft-recipes");

    var recipelist := array; //lista temp pra veriicar se os itens sao da skill do menu
    foreach x in itens
      var elem := FindConfigElem(df, x);
      var itemskill := GetConfigString(elem, "mainskill");
      if (itemskill)
        if (lower(itemskill) == lower(skill))
          recipelist.append(x);
        endif
      endif
    endforeach

    return recipelist;

endfunction
