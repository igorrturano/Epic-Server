
use uo;

function DoObjective();
  
  look_around();
  
	var dest := GetObjProperty(me, "objloc");
	var path;
  if (dest == error)
		EraseObjProperty(me, "objloc");
  endif
  
var xy := lineBresenham(me.x, me.y, dest[1], dest[2]);
  
  // sendsysmessage(me, "Distance: "+ CoordinateDistance( me.x, me.y, cint(xy.x), cint(xy.y) ) );
  var i;
  while (CoordinateDistance( me.x, me.y, cint(xy.x), cint(xy.y) ) > 0)
    if (i >= 25)
      Print(""+me+": abortando DoObjective por uso exagerado de CPU;");
      EraseObjProperty(me, "objloc");
      break;
    endif
    TurnTowardLocation( xy.x, xy.y );
    WalkTowardLocation( xy.x, xy.y );
    i := i + 1;
    sleepms(50);
  endwhile
  
	if ( CoordinateDistance(me.x, me.y, cint(dest[1]), cint(dest[2])) <= 2 ) //se chegou no objetivo, não tem mais objetivo.
		EraseObjProperty(me, "objloc");
    // SetObjProperty(me, "anchor", array{me.x, me.y});
	endif
	
  sleepms(50);

endfunction

//Bresenham's line drawing algorithm. determines a line between two waypoints.
function lineBresenham(x0, y0, x1, y1)
    var dy;
    dy := y1 - y0;
    var dx;
    dx := x1 - x0;
    var stepx, stepy;

    if (dy < 0)
      	dy := -dy;  
       	stepy := -1;
    else
      	stepy := 1;
    endif
        
    if (dx < 0) 
       	dx := -dx;
       	stepx := -1;
    else 
      	stepx := 1;
    endif
        
    dy := 2*dy;   
    dx := 2*dx;    

    if (dx > dy)
        var fraction;
        fraction := 2*dy - dx;
        while (x0 != x1)
        	if (fraction >= 0)
            	y0 := y0 + stepy;
                fraction := fraction - dx;
           	endif
            x0 := x0 + stepx;
            fraction := fraction + dy;
            return array{x0, y0};
        endwhile
    else
        var fraction;
        fraction := 2*dx - dy;
        while (y0 != y1)
            if (fraction >= 0)
                x0 := x0 + stepx;
                fraction := fraction - dy;
            endif
            y0 := y0 + stepy;
            fraction := fraction + dx;
            return array{x0, y0};
        endwhile
    endif
endfunction

function DoObjectiveGuard();
	var dest := GetObjProperty(me, "objloc");
	var path;
  if (dest == error)
		EraseObjProperty(me, "objloc");
  endif
  
var xy := lineBresenham(me.x, me.y, dest[1], dest[2]);
  
  // sendsysmessage(me, "Distance: "+ CoordinateDistance( me.x, me.y, cint(xy.x), cint(xy.y) ) );
  var i;
  while (CoordinateDistance( me.x, me.y, cint(xy.x), cint(xy.y) ) > 0)
    if (i >= 25)
      Print(""+me+": abortando DoObjective por uso exagerado de CPU;");
      EraseObjProperty(me, "objloc");
      break;
    endif
    TurnTowardLocation( xy.x, xy.y );
    WalkTowardLocation( xy.x, xy.y );
    i := i + 1;
    sleepms(50);
  endwhile
  
	if ( CoordinateDistance(me.x, me.y, cint(dest[1]), cint(dest[2])) <= 2 ) //se chegou no objetivo, não tem mais objetivo.
		EraseObjProperty(me, "objloc");
    // SetObjProperty(me, "anchor", array{me.x, me.y});
	endif
	
  sleepms(50);

endfunction