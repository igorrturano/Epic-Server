/* Misc Champion activAltar functions
 *
 * Functions in the Inc:
 * get_spawnarray(altar)		returns an npc template to be spawned
 * get_subLevel(altar)			returns the current sub-level
 * activateAltar(altar)			activates the altar
 * deactivateAltar(altar)		deactivates the altar
 * getRedSkullLocation(altar, index)	gets the index'th red skull location
 * getWhiteSkullLocation(altar, index)  gets the index'th white skull location
 * createNextWhiteSkull(altar)		creates the next white skull
 * createNextRedSkull(altar)		creates the next red skull
 * destroyLastRedSkull(altar)		destroys the last red skull
 * wipeItemsArray(Items)		destroy all items from an Items serial array
 * getSpawnLocation(altar)		gets an random spawn location 20 tiles around the altar
 *
 */


function get_spawn(altar)
	
  var lvl := get_subLevel(altar);
  var cfg := GetObjProperty(altar, "config");
  var spawns;
  
  if (lvl == 1 || lvl == 0)
    spawns := SplitWords(cfg.lvl1mobs);
  elseif (lvl == 2)
    spawns := SplitWords(cfg.lvl2mobs);
  else
    spawns := SplitWords(cfg.lvl3mobs);
  endif
  
	if ( !spawns )
		PrintTextAbove(altar, "ERRO: " + spawns);
		return error;
	endif

	return (spawns[RandomInt(spawns.size())+1]);

endfunction


function spawnChampion(altar)
	var loc := getSpawnLocation(altar);
  var cfg := GetObjProperty(altar, "config");
  var creature := CreateBoss(cfg.boss, loc[1], loc[2], loc[3], altar.realm);
  SetObjPRoperty(creature, "#championspawn", altar.serial); //deixa corpo se tiver essa prop
  if (creature == error)
    printtextabove(altar, " " + creature);
  endif
	SetObjProperty(altar, "champ", creature.serial);
endfunction

function get_subLevel(altar)
	var redSkulls := CInt(GetObjProperty(altar, "lrskull"));
	
	if(redSkulls<6)
		return 0;
	elseif(redSkulls<10)
		return 1;
	elseif(redSkulls<14)
		return 2;
	else
		return 3;
	endif
endfunction

function get_maxMonsters(altar)
	return (50-get_subLevel(altar)*12);
endfunction

function activateAltar(altar)
	var pentagram := GetObjProperty(altar, "pentagram");
	foreach piece in pentagram
		piece:=SystemFindObjectBySerial(piece);
		piece.invisible:=0;
		piece.color:=0;
	endforeach
	SetObjProperty(altar, "#active", 1);
  
//  printtextabove(altar, "activate");
  if (GetObjProperty(altar,  "status") == "on")
   // printtextabove(altar, "status on, preparando");
    var configuracao := GetObjProperty(altar, "config");
    PrepareEvent(configuracao.evento);
    SetObjProperty(altar, "status", "off");
  endif
endfunction

function deactivateAltar(altar)
	var pentagram := GetObjProperty(altar, "pentagram");
	foreach piece in pentagram
		piece:=SystemFindObjectBySerial(piece);
		piece.color:=980;
		piece.invisible:=1;
	endforeach
	SetObjProperty(Altar, "#NextActiv", ReadGameClock()+ACTIVE_EACH);
	EraseObjProperty(altar, "#completed");
	EraseObjProperty(altar, "#active");
  var boss :=  SystemFindObjectBySerial( GetObjProperty(altar, "champ") );
  ApplyDamage( boss, 20000);
	EraseObjProperty(altar, "champ");
  var configuracao := GetObjProperty(altar, "config");
  //printtextabove(altar, "desativando");
  if (GetObjProperty(altar, "status") == "off")
    //printtextabove(altar,"status off, executando");
    ExecuteEvent(configuracao.evento);
    SetObjProperty(altar,  "status", "on");
  endif
endfunction

function getRedSkullLocation(altar, index)
	var x, y;
	if(index<5)
		x := index - 2;
		y := -2;
	elseif(index<9)
		x := 2;
		y := index-6;
	elseif(index<13)
		x := 10-index;
		y := 2;
	else
		x := -2;
		y := 14-index;
	endif

	return {altar.x+x, altar.y+y, altar.z};
endfunction

function getWhiteSkullLocation(altar, index)
	var x:=0, y:=0;
	case ( index )
		default:
		0: x := -1; y := -1; break;
		1: x :=  1; y := -1; break;
		2: x :=  1; y :=  1; break;
		3: x := -1; y :=  1; break;
	endcase
	return {altar.x+x, altar.y+y, altar.z};
endfunction

function createNextWhiteSkull(altar)
	var lwskull := CInt(GetObjProperty(altar, "lwskull"));
	var wskulls:=array;
	wskulls:=GetObjProperty(altar, "wskulls");
	if(lwskull==4)
		WipeItemsArray(wskulls);
		wskulls:= {};
		SetObjProperty(altar, "wskulls", wskulls);
		EraseObjProperty(altar, "lwskull");
		CreateNextRedSkull(altar);
		return;
	endif
	var location := GetWhiteSkullLocation(altar, lwskull+1);
	var wskull := CreateItemAtLocation(location[1], location[2], location[3], 0x1854, 1);

	wskull.movable := 0;

	if(!wskulls)
		wskulls:={wskull.serial};
	else
		wskulls.append(wskull.serial);
	endif
    
        PlaySoundEffect(wskull, 0x29);
	PlayStationaryEffect( wskull.x,wskull.y,wskull.z, 0x3728, 7, 0x10,0,wskull.realm );
	//PlayStationaryEffect( x, y, z, effect, speed, loop := 0, explode := 0, realm := _DEFAULT_REALM );

	SetObjProperty(altar, "lwskull", lwskull+1);
	SetObjProperty(altar, "wskulls", wskulls);

	return;
endfunction

function createNextRedSkull(altar)
	var lskull := CInt(GetObjProperty(altar, "lrskull"));
	var skulls:=array;
	skulls:=GetObjProperty(altar, "rskulls");

	var wskulls:=array;
	wskulls:=GetObjProperty(altar, "wskulls");
	WipeItemsArray(wskulls);
	wskulls:= {};
	SetObjProperty(altar, "wskulls", wskulls);
	EraseObjProperty(altar, "lwskull");

	if(lskull==16)
		WipeItemsArray(skulls);
		skulls:= {};
		SetObjProperty(altar, "rskulls", skulls);
		SetObjProperty(altar, "#completed", 1);
		EraseObjProperty(altar, "lrskull");
		return;
	endif
	var location := GetRedSkullLocation(altar, lskull+1);
	var skull := CreateItemAtLocation(location[1], location[2], location[3], 0x1854, 1);
	
	skull.color:=0x26;
	skull.movable := 0;

	if(!skulls)
		skulls:={skull.serial};
	else
		skulls.append(skull.serial);
	endif
        PlaySoundEffect(skull, 0x29);
	PlayStationaryEffect( skull.x,skull.y,skull.z, 0x3728, 7, 0x10,0,skull.realm );

	SetObjProperty(altar, "lrskull", lskull+1);
	SetObjProperty(altar, "rskulls", skulls);

	return;
endfunction

function destroyLastRedSkull(altar)
	var lskull := CInt(GetObjProperty(altar, "lrskull"));
	var skulls:=array;
	skulls:=GetObjProperty(altar, "rskulls");
	if(!lskull)
		return;
	endif
	var skull := skulls[lskull];
	DestroyItem(SystemFindObjectBySerial(skull));
	skulls.erase(lskull);

	SetObjProperty(altar, "lrskull", lskull-1);
	SetObjProperty(altar, "rskulls", skulls);

	return;
endfunction

function wipeItemsArray(Items)
	foreach item in Items
		item:=SystemFindObjectBySerial(item);
		if(!DestroyItem(item))
			DestroyItem(item);
		endif
	endforeach

	return;
endfunction

function getSpawnLocation(altar)
	var x := altar.x + ((RandomIntMinMax(-10,10)+1));
	var y := altar.y + ((RandomIntMinMax(-10,10)+1));
    var z := GetMapInfo(x,y).z;

	return {x, y, z};
endfunction
