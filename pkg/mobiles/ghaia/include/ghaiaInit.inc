use uo;
use cfgfile;

include ":mounts:mounts";

var couleurs;
var vetements;
var templt;


function SetupAmmo(mobile)

	if ( npccfgfile[ParseTemplateName(mobile.npctemplate).template].MissileWeapon )
		var weapon := CreateItemAtLocation(mobile.x, mobile.y, mobile.z, npccfgfile[ParseTemplateName(mobile.npctemplate).template].MissileWeapon, 1, mobile.realm);
		if ( !EquipItem(mobile, weapon) )
			DestroyItem(weapon);
		endif
	endif

	if ( npccfgfile[ParseTemplateName(mobile.npctemplate).template].AmmoType )
		if ( CInt(npccfgfile[ParseTemplateName(mobile.npctemplate).template].AmmoAmount) > 0 )
			var ammo := CreateItemInBackpack(mobile, npccfgfile[ParseTemplateName(mobile.npctemplate).template].AmmoType, CInt(npccfgfile[ParseTemplateName(mobile.npctemplate).template].AmmoAmount));
			if ( !ammo )
				return ammo;
			endif
			return 1;
		endif
	endif

	return 1;

endfunction

function SetupBackPack(mobile)
	if ( !mobile.backpack )
		var backpack := CreateItemAtLocation(mobile.x, mobile.y, mobile.z, "BackPack", 1);
		var result := EquipItem(mobile, backpack);
		if ( !result )
			DestroyItem(backpack);
			return result;
		else
			return backpack;
		endif
	endif

	return mobile.backpack;
endfunction

function SetupColor(who)
  var colors := npccfgfile[ParseTemplateName(who.npctemplate).template].colors;
  if(colors)
    colors := SplitWords(colors);
    var clr := Cint(colors[RandomInt(Len(colors)) + 1]);
    who.color := clr;
    who.truecolor := clr;
  endif
endfunction

function SetupName(mobile)

	if (!mobile.name["<random>"])
		return 1;
	endif

	var npc_names := ReadConfigFile(":ghaia:config/names");
  var npcelem :=  FindConfigElem(npccfgfile, ParseTemplateName(mobile.npctemplate).template);
  var nametemplate := GetConfigString(npcelem, "NameTemplate");
	var name_elem := npc_names[nametemplate];
	if ( !name_elem )
    //printtextabove(mobile, "npcdesc: " + npccfgfile);
    //printtextabove(mobile, "npc template " + ParseTemplateName(mobile.npctemplate).template);
    //printtextabove(mobile, "npc_name: " + npc_names);
    //printtextabove(mobile, "nametemplate: " + nametemplate);
    //printtextabove(mobile, "name_elem: " + name_elem);
		return 0;
	endif

	var name_list := GetConfigStringArray(name_elem, "Name");
	if ( !name_list )
    //printtextabove(mobile, "elem name not found");
    //printtextabove(mobile, "name_elem " + name_elem);
    //printtextabove(mobile, "name_list " + name_list);
		return 0;
	endif

	var my_name := mobile.name;
	//while ( my_name == "<random>")
	my_name["<random>"] := name_list[RandomInt(Len(name_list))+1];
	SetName(mobile, my_name);
	//my_name := mobile.name;
		sleepms(2);
	//endwhile


	return 1;
endfunction

function SetupEquip(who)

  var group;
  var equipt;
  var npcelem := FindConfigElem(npccfgfile, ParseTemplateName(me.npctemplate).template);
  var mobile_taxonomy := GetConfigString(npcelem, "Taxonomy");
  var categ := GetConfigString(npcelem, "Equip");
  categ := categ;

  if(who.graphic == 0x190)
        //case(randomint(2))
          //0:
              who.graphic := 0x190;
             who.gender := 0;
             categ := categ + "M";
  elseif (who.graphic == 0x191)
          //1: who.graphic := 0x191;
           //  who.gender := 1;
             categ := categ + "F";
        //endcase
   endif
  couleurs := ReadConfigFile(":ghaia:config/colors");
  vetements := ReadConfigFile(":ghaia:config/cloths");
  templt := ReadConfigFile(":ghaia:config/equip2");
  //printtextabove(me, " " + couleurs + " " + vetements + " " + templt);
  if(couleurs && vetements && templt)
    group := FindConfigElem(templt, categ);
    //printtextabove(who, " " +templt + " " + categ + " " + group);
    if(group)
      var str;
      var lastc := 0;
      foreach str in GetConfigStringArray(group, "Equip")
            lastc := EquipItemString(who, str, lastc);
      endforeach
      foreach str in GetConfigStringArray(group, "Armor")
        lastc := EquipItemString(who, str, lastc );
      endforeach
      foreach str in GetConfigStringArray(group, "Weapon")
        //printtextabove(who, " " + str);
        lastc := EquipItemString(who, str, lastc);
      endforeach
    endif
  endif
  var montaria := Getobjproperty(who, "Montaria");
  if (!montaria)
  else
	var graphic := MP_GetMountGraphic(montaria);
	print("graphic eh" +graphic);
	var item := CreateItemAtLocation(who.x, who.y, who.z, MOUNT_OBJTYPE, 1);
	item.graphic := graphic;
	print("item eh "+item.graphic);
	Equipitem (who, item);
  endif

  if (!mobile_taxonomy)
  else
    SetupGender(who);
    SetupAnimalCritic(who);
  endif
endfunction

function EquipItemString(who, str, lastcolor)
  var params := SplitWords(str);
  var objname;
  var color;
  var chance;
  var lastc := 0;
  objname := params[1];
  if(CInt(objname))
    objname := CInt(objname);
  endif
  if(CInt(params[2]))
    color := CInt(params[2]);
  else
    color := 0;
    if(params[2] == "lastcolor" || params[2] == "lastcolour")
      color := lastcolor;
    else
      var colorg := FindConfigElem(couleurs, params[2]);
      if(colorg)
        var str := GetConfigStringArray(colorg, "Color");
        var ar := array{};
        var rndint := RandomInt(len(str)) + 1;
        ar := SplitWords(str[rndint]);
        if(len(ar) > 1)
          if(CInt(ar[2]) > 0)
            color := RandomInt(CInt(ar[2]) - CInt(ar[1]))+1;
          else
            color := CInt(str[RandomInt(len(str))+1]);
          endif
        else
          color := CInt(str[RandomInt(len(str))+1]);
        endif
        if(color == 1)
          color := 0;
        endif
      endif
    endif
  endif
  lastc := color;
  if(CInt(params[3]))
    chance := CInt( params[3] );
  else
    chance := 100;
  endif
  if(CInt(RandomInt(99)+1) <= CInt(chance))
    var item := CreateItemAtLocation(5852, 1163, 0, objname, 1);
    if(!item)
      var itemg := FindConfigElem(vetements, objname);
      if(itemg)
        var str := GetConfigStringArray(itemg, "Item");
        var str3;
        var newarray := array{};
        var i := 1;
        var sex := "";
        foreach str3 in str
          var str2 := SplitWords(str3);
          sex := CStr(str2[2]);
          if(sex != error)
            if(sex == "m" || sex == "f")
              if(sex == "m" && who.graphic == 400)
                newarray[i] := str2[1];
                i := i + 1;
              elseif(sex == "f" && who.graphic == 401)
                newarray[i] := str2[1];
                i := i + 1;
              elseif(sex == "Anão M" && who.graphic == 1828)
                newarray[i] := str2[1];
                i := i + 1;
              elseif(sex == "Anão F" && who.graphic == 1829)
                newarray[i] := str2[1];
                i := i + 1;
              elseif(sex == "Elfo M" && who.graphic == 1830)
                newarray[i] := str2[1];
                i := i + 1;
              elseif(sex == "Elfo F" && who.graphic == 1831)
                newarray[i] := str2[1];
                i := i + 1;
              elseif(sex == "Orc M" && who.graphic == 1832)
                newarray[i] := str2[1];
                i := i + 1;  
              elseif(sex == "Orc F" && who.graphic == 1833)
                newarray[i] := str2[1];
                i := i + 1;
              elseif(sex == "Wolfkin M" && who.graphic == 1834)
                newarray[i] := str2[1];
                i := i + 1;  
              elseif(sex == "Wolfkin F" && who.graphic == 1835)
                newarray[i] := str2[1];
                i := i + 1;
              endif
            else
              newarray[i] := str2[1];
              i := i + 1;
            endif
          else
            newarray[i] := str2[1];
            i := i + 1;
          endif
        endforeach
        objname := CInt(newarray[RandomInt(len(newarray))+1]);
        item := CreateItemAtLocation(5852, 1163, 0, objname, 1);
      endif
    endif
    item.color := color;
    if(CInt(params[4]) == 1)
      MoveItemToContainer(item, who.backpack);
    else
      if(!EquipItem(who, item))
        MoveItemToContainer(item, who.backpack);
      endif
    endif
  endif
  return lastc;
endfunction

/*
 * SetupGender(mobile)
 *
 * Purpose
 * Set the gender of the npc  based on its 'Sex'
 * settings in its npcdesc.cfg template.
 *
 * mobile:  The npc that will be given the gender.
 *
 * Return value
 * Returns 1
 *
 */
function SetupGender(mobile)
  var gender_set := npccfgfile[ParseTemplateName(mobile.npctemplate).template].Gender;

  if ( gender_set == 0)
    //var gender_new := Cint(randomint(2) + 1);
    case(randomint(2))
          0: Setobjproperty(mobile, "Sexo", 1);
          1: Setobjproperty(mobile, "Sexo", 2);
        endcase
  else
    Setobjproperty(mobile, "Sexo", Cint(mobile.Gender) );
  endif

endfunction

/*
 * SetupAnimalCritic(mobile)
 *
 * Purpose
 * Set the critic of the savage annimal npc  based on its category 'Critics'
 * settings in its npcdesc.cfg template.
 *
 * mobile:  The npc that will be given the critic.
 *
 * Return value
 * Returns 1
 *
 */
function SetupAnimalCritic(mobile)
  var gender_set := npccfgfile[ParseTemplateName(mobile.npctemplate).template].Gender;
  var npcelem :=  FindConfigElem(npccfgfile, ParseTemplateName(mobile.npctemplate).template);
  var taxonomy := GetConfigString(npcelem, "Taxonomy");
  var evil := (mobile.alignment == 1);
if (!evil)
return;
endif

if (taxonomy == "bovidae" || taxonomy == "cervidae")
  case (RandomInt(1))
  0:
    SetObjProperty(mobile, "Critic", "Coice" );
  1:
    SetObjProperty(mobile, "Critic", "Chifrada");
  endcase
elseif (taxonomy == "suidae" || taxonomy == "camelidae" || taxonomy == "equus")
  SetObjProperty(mobile, "Critic", "Coice");
elseif (taxonomy == "anura" || taxonomy == "serpentes")
  case (RandomInt(1))
  0:
    SetObjProperty(mobile, "Critic", "Bote" );
  1:
    SetObjProperty(mobile, "Critic", "Glandulas_concentradas" );
  endcase
elseif (taxonomy == "primatomorpha" || taxonomy == "ursidae")
  case (RandomInt(2))
  0:
    SetObjProperty(mobile, "Critic", "Frenesi" );
  1:
    SetObjProperty(mobile, "Critic", "Rugir" );
  endcase
elseif (taxonomy == "canidae" || taxonomy == "felidae" || taxonomy == "chiroptera" || taxonomy == "rodentia"|| taxonomy == "reptilia" || taxonomy == "crustacean" || taxonomy == "scarabaeidae")
  case (RandomInt(2))
  0:
    SetObjProperty(mobile, "Critic", "Presas_profundas" );
  1:
    SetObjProperty(mobile, "Critic", "Garras_afiadas" );
  endcase
elseif (taxonomy == "immortuos" || taxonomy == "draco")
  SetObjProperty(mobile, "Critic", "Baforada" );
elseif (taxonomy == "aves" || taxonomy == "phasianidae")
  SetObjProperty(mobile, "Critic", "Garras_afiadas");
endif

  SetObjProperty(mobile, "Selvagem", 1);

endfunction