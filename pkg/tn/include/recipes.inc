use uo;
use datafile;

/* procura no datafile se tem uma entrada igual o npctemplate. pega a chance do item dropar, a forma q vai dropar e o recipename , joga em uma struct
e coloca no elem referente ao npctemplate. */
function CreateRecipeLoot(who, npcloot, recipename)

	var elem := FindRecipeLoot(npcloot);
	if (!elem)
		var df := OpenDataFile( ":tn:lootrecipes" );
		elem := df.Createelement(npcloot);
	endif
	
	var lootinfo := struct;
	
	var chance := Cint(RequestGump(who, "Qual a chance desse loot aparecer no npc: " + npcloot));
	lootinfo.+chance := chance;
	
	var recipeoritem := RadioGump(who, 200, 200, "O item vai aparecer na forma de recipe ou de item.", {"recipe", "item"});
	lootinfo.+type := recipeoritem;
	lootinfo.+item := recipename;
	
	var lista := elem.GetProp("loots");
	if (lista)
		lista.append(lootinfo);
	else
		lista := {};
		lista.append(lootinfo);
	endif
	
	elem.SetProp("loots", lista);

endfunction
	
function GenerateRecipeLoot(npc)
  
  var elem := FindRecipeLoot( npc.npctemplate);
  if (elem)
    var loots := elem.GetProp("loots");
    foreach loot in loots
      if ( (RandomInt(100)+1) < loot.chance) //se passou na chance pode criar
        if (loot.type == "recipe")
          CreateRecipeItem(npc.backpack, loot.item);
        else
          CreateRecipe(npc.backpack, loot.item);
        endif
      endif
    endforeach
  endif
  
endfunction
  
function EraseRecipeLoot(who, npcloot, recipename)
	
	var elem := FindRecipeLoot(npcloot);
	
	if (elem)
		var lista := elem.GetProp("loots");
		var i := 1;
		foreach x in lista
			if (x.item == recipename)
				lista.erase(i);
				break;
			endif
			i := i + 1;
		endforeach
		elem.SetProp("loots", lista);
	endif
	
	
endfunction

function DeleteRecipe(recipename)

	var df := OpenDataFile( ":tn:recipes" );
	if (!df)
		df := CreateDataFile(":tn:recipes");
	endif
	
	var r := df.DeleteElement(recipename);
	
	return r;

endfunction

function CreateRecipeItem(container, recipename)

	var elem := FindRecipe(recipename);
	
	if (elem)
		var itm := elem.GetProp("recipe");
		var item := CreateItemInContainer(container, itm, 1);
		SetName(item, lower(itm.desc));
		return item;
	else
		return 0;
	endif

endfunction

function CreateRecipe(container, recipe)

	var itm := CreateItemInContainer(container, "recipe1", 1);
	
	if (itm)
		SetObjProperty(itm, "recipe", recipe);
	endif

	return itm;
endfunction

function GetRecipeMaterials( recipename)
  
  var elem := FindRecipe(recipename);
  if (elem)
    var materiais := elem.getprop("craftmat");
    return materiais;
  endif
  
  return 0;
  
endfunction

function GetRecipeCraftSkills( recipename)
  
  var elem := FindRecipe(recipename);
  if (elem)
    var materiais := elem.getprop("craftskills");
    return materiais;
  endif
  
  return 0;
  
endfunction

function FindRecipeLoot( npcloot)

	var df := OpenDataFile( ":tn:lootrecipes" );
	if (!df)
		df := CreateDataFile(":tn:lootrecipes");
	endif

	var elem := df.FindElement( npcloot );	

	return elem;

endfunction

function FindRecipe(recipename)

	recipename := lower(recipename);
	var df := OpenDataFile( ":tn:recipes" );
	if (!df)
		df := CreateDataFile(":tn:recipes");
	endif

	var elem := df.FindElement( recipename );

	return elem;
endfunction