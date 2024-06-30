/* $Id: fishingFunctions.inc 917 2005-11-05 23:51:11Z austin $
 *
 */

function is_fish(theobj)

	if((theobj >= UOBJ_FISH_START) && (theobj <= UOBJ_FISH_END) )
		return 1;
	elseif ((theobj >= UOBJ_SFISH_START) && (theobj <= UOBJ_SFISH_END))
		return 1;
	else
		return 0;
	endif

	return 0;

endfunction


function CarveFish(who, blade, use_on)
	var peso;
	var pesostack;
	var steak;
	var num_steaks;
	if( (!Accessible(who, use_on)) or (!CheckLineOfSight(who, use_on)) or (distance(who, use_on) > 2) )
		SendSysMessage(who, "Voce nao pode usar isto.");
		return 0;
	endif
	if( (!ReserveItem(use_on)) || (use_on.movable == 0) )
		SendSysMessage(who, "Voce nao pode usar isto.");
		return 0;
	endif
	
	var nomepeixe := use_on.name;
	nomepeixe := splitwords(nomepeixe);
	peso := cint(nomepeixe[4]);
	num_steaks := Cint((peso / 2));	

	//Verificacao do tipo de peixe para o tipo certo de filé, Alem do peso stack, que concerta o peso.
	if (use_on.objtype == 0x9987) pesostack := (2 + RandomInt(1)); steak := "0x9972";
 	elseif (use_on.objtype == 0xdd8)  pesostack := (60 + RandomInt(30)); steak :=  "0x9973";
 	elseif (use_on.objtype == 0x9cf) pesostack := (40 + RandomInt(20)) ; steak :=  "0x9978";
 	elseif (use_on.objtype == 0x9ce) pesostack := (25 + RandomInt(10));  steak :=  "0x9975";
 	elseif (use_on.objtype == 0xdd7) pesostack := (6 + RandomInt(1));  steak :=  "0x9971";
	elseif (use_on.objtype == 0x9cd) pesostack := (30 + RandomInt(15)); steak :=  "0x9974";
	elseif (use_on.objtype == 0x9cc) pesostack := (15 + RandomInt(1)); steak :=  "0x9970";
 	elseif (use_on.graphic == 0xdd9) pesostack := (30 + RandomInt(15)); steak :=  "0x9977";
	elseif (use_on.graphic == 0xdd6) pesostack := (2 + RandomInt(1)); steak :=  "0x9976";
	endif

	//IF para nao dar erro se stackarem os peixes (perdem o peso no nome)
	if (use_on.amount > 1)
		num_steaks := Cint(((pesostack / 2) * use_on.amount));
	endif

	steak := cint(steak);
	var peixe:= use_on;
	if( DestroyItem(use_on) )
		CreateItemInBackpack(who, steak, num_steaks);
		return 1;
	endif
	sleep(1);

	return 0;

endfunction

function IsInRange(who, x, y, z)

	z := z;
	var xd, yd;
	var retval := 1;
	xd := Abs(who.x - x);
	yd := Abs(who.y - y);
	if((xd > 6)||(yd > 6))
		retval := 0;
	endif

	return retval;

endfunction


function IsWater(thistile, locinfo)

	if(thistile.objtype)
		if((thistile.objtype >= 0x1797)&&(thistile.objtype <= 0x179c))
			return 1;
		elseif((thistile.objtype >= 0x346e)&&(thistile.objtype <= 0x3485))
			return 1;
		elseif((thistile.objtype >= 0x3490)&&(thistile.objtype <= 0x34ab))
			return 1;
		elseif((thistile.objtype >= 0x34b5)&&(thistile.objtype <= 0x34d5))
			return 1;
		elseif (thistile.objtype IN {20739, 2881, 2882, 2883, 2884, 5453})
			return 1;
		endif
	else
		if((locinfo.landtile >= 0x00a8)&&(locinfo.landtile <= 0x00ab))
			return 1;
		elseif((locinfo.landtile >= 0x0136)&&(locinfo.landtile <= 0x0137))
			return 1;
		elseif((locinfo.landtile >= 0x3ff0)&&(locinfo.landtile <= 0x3ff3))
			return 1;
		endif
	endif

	return 0;

endfunction


function IsValidFishingSpot(who, targetsquare, locinfo)

	var retval := 1;
	if(!IsWater(targetsquare, locinfo))
		SendSysMessage(who, "Voce nao pode pescar aqui!");
		retval := 0;
	elseif(!IsInRange(who, targetsquare.x, targetsquare.y, targetsquare.z))
		SendSysMessage(who, "Isto esta muito longe.");
		retval := 0;
	elseif(!CheckLosAt(who, targetsquare.x, targetsquare.y, targetsquare.z))
		SendSysMessage(who, "Este lugar nao esta na sua linha de visao!");
		retval := 0;
	endif

	return retval;

endfunction


function farout(who)

	var x := who.x, y := who.y;
	var chk := 0;

	var theinfo := GetMapInfo( x, y );
	if((theinfo.landtile >= 0x00a8)&&(theinfo.landtile <= 0x00ab))
		chk := 1;
	elseif((theinfo.landtile >= 0x0136)&&(theinfo.landtile <= 0x0137))
		chk := 1;
	elseif((theinfo.landtile >= 0x3ff0)&&(theinfo.landtile <= 0x3ff3))
		chk := 1;
	else
		return 0;
	endif

	var distinfo := GetMapInfo( (x+15), (y+15) );
	if((distinfo.landtile >= 0x00a8)&&(distinfo.landtile <= 0x00ab))
		return 1;
	elseif((distinfo.landtile >= 0x0136)&&(distinfo.landtile <= 0x0137))
		return 1;
	elseif((distinfo.landtile >= 0x3ff0)&&(distinfo.landtile <= 0x3ff3))
		return 1;
	endif

	distinfo := GetMapInfo( (x-15), (y-15) );
	if((distinfo.landtile >= 0x00a8)&&(distinfo.landtile <= 0x00ab))
		return 1;
	elseif((distinfo.landtile >= 0x0136)&&(distinfo.landtile <= 0x0137))
		return 1;
	elseif((distinfo.landtile >= 0x3ff0)&&(distinfo.landtile <= 0x3ff3))
		return 1;
	endif

	distinfo := GetMapInfo( (x+15), (y-15) );
	if((distinfo.landtile >= 0x00a8)&&(distinfo.landtile <= 0x00ab))
		return 1;
	elseif((distinfo.landtile >= 0x0136)&&(distinfo.landtile <= 0x0137))
		return 1;
	elseif((distinfo.landtile >= 0x3ff0)&&(distinfo.landtile <= 0x3ff3))
		return 1;
	endif

	distinfo := GetMapInfo( (x-15), (y+15) );
	if((distinfo.landtile >= 0x00a8)&&(distinfo.landtile <= 0x00ab))
		return 1;
	elseif((distinfo.landtile >= 0x0136)&&(distinfo.landtile <= 0x0137))
		return 1;
	elseif((distinfo.landtile >= 0x3ff0)&&(distinfo.landtile <= 0x3ff3))
		return 1;
	endif

	return 0;

endfunction


function isSOS(who, targetsquare)

	var packitems;
	packitems := EnumerateItemsInContainer(who.backpack);
	var i, sosx, sosy;

	for(i := 1; i <= len(packitems); i := i + 1)
		if((packitems[i].objtype == 0xa363))
			sosx := GetObjProperty(packitems[i], "tileX");
			sosy := GetObjProperty(packitems[i], "tileY");
			var xd, yd;
			xd := Abs(targetsquare.x - sosx);
			yd := Abs(targetsquare.y - sosy);
			if((xd <= 6)&&(yd <= 6))
				GetSOSTreasure(who, targetsquare, packitems[i]);
				return 1;
			endif
		endif
	endfor

	return 0;

endfunction


function GetSOSTreasure(who, targetsquare, sosmsg)

	targetsquare := targetsquare;
	if( SkillCheck(who, DomesticWork, 80, 200) > 0 )
		var itemcfg := ReadConfigFile(":*:itemdesc");
		var it, oldname;
		if( RandomInt(100) < 75 )
			var newitem;
			var artarray;
			case(RandomInt(3))
				0:	newitem := RandomInt(0x0fcc - 0x0fc4)+ 0x0fc4;
				1:	artarray := array(0xea4, 0xea8, 0xea7, 0xe9f, 0xea0, 0xea3, 0xea6);
					newitem := artarray[RandomInt(7) + 1];
				2:	artarray := array(0x1397, 0x13a4, 0x13a5, 0x13a6, 0x13a7, 0x13a8, 0x13a9, 0x13ab, 0x13ac, 0x13ad, 0x13ae, 0x163a, 0x163b, 0x163c);
					newitem := artarray[RandomInt(14) + 1];
			endcase
			it := CreateItemInBackpack(who, newitem, 1);
			oldname := it.name;
			if( (!oldname) || (oldname == "") )
				oldname := itemcfg[newitem].desc;
			endif
			SetName(it, (oldname + " [Recuperado de um navio submerso]"));
			it.movable := 1;
			SendSysMessage(who, "Voce achou alguma coisa interessante!");
		else
			SendSysMessage(who, "Voce achou um tesouro submerso!");
			var chest := CreateItemInBackpack(who, 0x9e40, 1);
			case(RandomInt(4) + 1)
				1: chest.graphic := 0xe40;
				2: chest.graphic := 0xe41;
				3: chest.graphic := 0xe7c;
				4: chest.graphic := 0x9ab;
			endcase
			SetName(chest, ("bau [Recuperado de um navio submerso]"));
			chest.movable := 1;
			var lvl := RandomDiceRoll("1d5");
			var lootgroup;
			case(lvl)
			      1: lootgroup := "SpawnChest1";  
			      2: lootgroup := "SpawnChest2";  
			      3: lootgroup := "SpawnChest3";  
			      4: lootgroup := "SpawnChest4";  
			      5: lootgroup := "SpawnChest5";  
			endcase
			//MakeLoot(chest, lootgroup);
			DestroyItem(sosmsg);
		endif
	else
		SendSysMessage(who, "Alguma coisa grande caiu de seu anzol!");
	endif

	return 0;

endfunction

function FindPeixePretendido(who, isca)
	if (isca.objtype == 0x9980) return "Manjuba"; 
 	elseif (isca.objtype == 0x9981) return "Atum"; 
 	elseif (isca.objtype == 0x9982) return "Dourado"; 
 	elseif (isca.objtype == 0x9983) return "Anchova";  
 	elseif (isca.objtype == 0x9984) return "Sardinha";  
	elseif (isca.objtype == 0xdd7) return "Perca"; 
	elseif (isca.objtype == 0xdd6) return "Robalo"; 
 	elseif (isca.graphic == 0x0C82) return "PeixeGalo";
	elseif (isca.graphic == 0x1CEF) return "Lambari";
	endif
endfunction

function FindPeixeId(who, isca)
	if (isca.objtype == 0x9980) return "0x9987"; 
 	elseif (isca.objtype == 0x9981) return "0xdd8"; 
 	elseif (isca.objtype == 0x9982) return "0x9cf"; 
 	elseif (isca.objtype == 0x9983) return "0x9ce";  
 	elseif (isca.objtype == 0x9984) return "0xdd7";  
	elseif (isca.objtype == 0xdd7) return "0x9cd"; 
	elseif (isca.objtype == 0xdd6) return "0x9cc"; 
 	elseif (isca.graphic == 0x0C82) return "0xdd9";
	elseif (isca.graphic == 0x1CEF) return "0xdd6"; 
	endif
endfunction
