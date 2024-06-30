use uo;
use os;
use math;
use cfgfile;

include "include/facings";

function _ifElse(a,b,c)
   if (a) return b; endif
   return c;
endfunction


function _IfOr(a,b)
   if (a != error && a != 0) return a; endif
   return b;
endfunction


function _resolveParams(byref a, byref b, byref c := 0)
   if (typeof(a) == "Array")
      c := a[3];
      b := a[2];
      a := a[1];
   endif
endfunction

function _CreateItemInBackpack(who, objtype, amount := 1)
   if (cint(amount) < 1) amount := 1; endif
   
   var item := CreateItemInContainer(who.backpack, objtype, amount);
   if (item) 
      // MoveObjectToLocation( item, who.x, who.y, who.z, who.realm, MOVEOBJECT_FORCELOCATION );
      MoveItemToContainer( item, who.backpack );
   endif

   return item;
endfunction

function _checkPid(who, pid, temp := 0)
   set_critical(1);
   pid := "{}pid_{}".format(_ifElse(temp, "#", ""), pid);
   var process := GetProcess( GetobjProperty(who, pid) );
   if (process) process.kill(); endif
   SetObjProperty(who, pid, GetPid());
   set_critical(0);
endfunction

function _getPid(who, pid, temp := 0)
   pid := "{}pid_{}".format(_ifElse(temp, "#", ""), pid);
   return GetProcess( GetobjProperty(who, pid) );
endfunction

function randomPlace(loc, radius := 1)
	var y, x;
   case (RandomInt(3)+1)
      1: y := (RandomInt(radius)+1) * -1; x := (RandomInt(radius)+1);
      2: y := (RandomInt(radius)+1); 		x := (RandomInt(radius)+1) * -1;
      3: y := (RandomInt(radius)+1) * -1; x := (RandomInt(radius)+1) * -1;
      4: y := (RandomInt(radius)+1); 		x := (RandomInt(radius)+1);
   endcase

	var new_loc := struct{
		"x" := loc.x+x,
		"y" := loc.y+y
	};

   return new_loc;
endfunction

function GetPointsInRadius(mid_x, mid_y, radius)
	var x_row := mid_x-radius;
	var y_col := mid_y-radius;
	var stop_x := mid_x+radius;
	var stop_y := mid_y+radius;

	var point_list := array();
	var coord := struct;
	coord.+x;
	coord.+y;

	for ( x_row:=(mid_x-radius); x_row<=stop_x; x_row:=x_row+1 )
		sleepms(2);
		for ( y_col:=(mid_y-radius); y_col<=stop_y; y_col:=y_col+1 )
			coord.x := x_row;
			coord.y := y_col;
			point_list.append(coord);
			sleepms(2);
		endfor
	endfor

	return point_list;
endfunction

function PushBack(who, dist, delay, invert := 0)
   var mod := GetModByFacing(who.facing);
	var x := mod[1];
	var y := mod[2];

	if (!invert)
		x := x * -1;
		y := y * -1;
	endif
   
   for i:= 1 to dist
		var finalx := (who.x + x);
		var finaly := (who.y + y);
		if (!CheckLosAt(who, finalx, finaly, who.z))
			return 0;
		endif
		var err := MoveObjectToLocation( who, finalx, finaly, who.z, who.realm, MOVEITEM_IGNOREMOVABLE );
		sleepms(delay);
		if (err == error)
		//	PrintText(who, "*bate no obstaculo*");
			return 0;
		endif
	endfor

   return 1;
endfunction

function PushMobile(who, dist, delay, targ, invert := 0)
   var mod := GetModByFacing(GetFacing(who.x, who.y, targ.x, targ.y));
	var x := mod[1];
	var y := mod[2];

   if (invert) 
      invert := -1;
   else
      invert := 1;
   endif

	var i := 0;
	for( i := 1; i <= dist; i := i + 1)
		var finalx := (targ.x + x) * invert;
		var finaly := (targ.y + y) * invert;
      
		if (!CanWalk("L", targ.x, targ.y, targ.z, finalx, finaly))
			break;
		endif

		var err := MoveObjectToLocation( targ, finalx, finaly, targ.z, targ.realm, MOVEITEM_IGNOREMOVABLE );
		sleepms(delay);
		if (err == error)
		//	PrintText(who, "*bate no obstaculo*");
			break;
		endif
	endfor
endfunction


function FLSCreateItem(who, item, amount := 1)
   var created := CreateItemAtLocation(who.x, who.y, who.z, item, amount, who.realm);
	if (who.cmdlevel < 4)
		SetObjProperty(created, "gm_createdby", who.acctname + " " + who.name);
	endif
   
	if ( !created )
		if ( amount > 10 )
			SendSysMessage(who, "Item is not stackable. No more than 10 will be created at a time.");
			amount := 10;
		endif
		
		for( amount; amount > 0; amount := amount-1 )
			created := CreateItemAtLocation(who.x, who.y, who.z, item, 1, who.realm);
			if (who.cmdlevel < 4)
				SetObjProperty(created, "gm_createdby", who.acctname + " " + who.name);
			endif
		endfor	
	endif
	
	if ( !created )
		SendSysMessage(who, "Error: Could not create "+Hex(item));
	endif
endfunction

function FLSCreateItemBackpack(who, item, amount := 1)
   var created := CreateItemInBackPack(who, item, amount);
	if (who.cmdlevel < 4)
		SetObjProperty(created, "gm_createdby", who.acctname + " " + who.name);
	endif
   
	if ( !created )
		if ( amount > 10 )
			SendSysMessage(who, "Item is not stackable. No more than 10 will be created at a time.");
			amount := 10;
		endif
		
		for( amount; amount > 0; amount := amount-1 )
			created := CreateItemInBackPack(who, item, amount);
			if (who.cmdlevel < 4)
				SetObjProperty(created, "gm_createdby", who.acctname + " " + who.name);
			endif
		endfor	
	endif
	
	if ( !created )
		SendSysMessage(who, "Error: Could not create "+Hex(item));
	endif

   return created;
endfunction

function convertToRoman(num)
  var roman :=  struct{ "M":=1000, "CM":=900, "D":=500, "CD":=400, "C":=100, "XC":=90, "L":=50, "XL":=40, "X":=10, "IX":=9, "V":=5, "IV":=4, "I":=1};
  var letters := array{ "M", "CM", "D", "CD", "C", "XC", "L", "XL", "X", "IX", "V", "IV", "I"};
  var str := "";
   foreach i in (letters)
      var q := Floor(num / roman[i]);
      num -= q * roman[i];
      for a := 1 to q
          str += i;
      endfor
   endforeach

   return str;
endfunction

function InverseFacing(facing, type := 0)
   if (!type)
      if (facing >= 4)
         facing -= 4;
      else
         facing += 4; 
      endif
   elseif( type == 1)
      if (facing >= 6)
         facing += 2;
      else
         facing -= 6; 
      endif
   elseif( type == 2)
      if (facing <= 2)
         facing += 6;
      else
         facing += 2;
      endif
   endif

   return facing;
endfunction

function GetItemFix(fix_cfg, objtype)
   var elem := FindConfigElem(fix_cfg, Hex(objtype));
   if (elem)
      return struct{
         x := GetConfigInt(elem, "x"),
         y := GetConfigInt(elem, "y")
      };
   endif

   return struct{ x:= 0, y := 0};
endfunction