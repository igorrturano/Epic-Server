/*
 * $Id*
 *
 */
 
use uo;
use os;
use math;

/*
 * GetCoordsInSquare(mid_x, mid_y, radius)
 *
 * Purpose
 * Retrieves the coordinates in the area of a square.
 *
 * Parameters
 * mid_x:	X coordinate of the middle of the square.
 * mid_y:	Y coordinate of the middle of the square.
 * radius:	Size of the square
 *
 * Return value
 * Array of structs (.x, .y)
 * 
 */
function GetCoordsInSquare(mid_x, mid_y, radius)
	var x_row := CInt(mid_x)-CInt(radius);
	var y_col := CInt(mid_y)-CInt(radius);
	var stop_x := CInt(mid_x)+CInt(radius);
	var stop_y := CInt(mid_y)+CInt(radius);

	var point_list := array{};
	var coord := struct{"x", "y"};
	
	for ( x_row:=(mid_x-radius); x_row<=stop_x; x_row:=x_row+1 )
		for ( y_col:=(mid_y-radius); y_col<=stop_y; y_col:=y_col+1 )
			coord.x := x_row;
			coord.y := y_col;
			point_list.Append(coord);
			sleepms(2);
		endfor
		sleepms(2);
	endfor
	
	return point_list;
endfunction

/*
 * GetCoordsInRect(x1, y1, x2, y2)
 *
 * Purpose
 * Retrieves the coordinates in the area of a rectangle.
 *
 * Parameters
 * x1:	X coordinate
 * y1:	Y coordinate
 * x2:	X coordinate
 * y2:	Y coordinate
 *
 * Return value
 * Array of structs (.x, .y)
 * 
 */
function GetCoordsInRect(x1, y1, x2, y2)
	var point_list := array{};
	var coord := struct{"x", "y"};
	
	x1 := CInt(x1);
	y1 := CInt(y1);
	x2 := CInt(x2);
	y2 := CInt(y2);
	
	if ( x1 > x2 )
		x1 := x1*-1;
		x2 := x2*-1;
	endif
	if ( y1 > y2 )
		y1 := y1*-1;
		y2 := y2*-1;
	endif
	
	var sx, sy;
	for ( sx:=x1; sx<=x2; sx:=sx+1 )
		for ( sy:=y1; sy<=y2; sy:=sy+1 )
			coord.x := Abs(sx);
			coord.y := ABs(sy);
			point_list.Append(coord);
			sleepms(2);
		endfor
		sleepms(2);
	endfor
	
	return point_list;
endfunction

/*
 * GetCoordsInCircle(mid_x, mid_y, radius)
 *
 * Purpose
 * Retrieves the coordinates in the area of a circle.
 *
 * Parameters
 * mid_x:	X coordinate of the middle of the square.
 * mid_y:	Y coordinate of the middle of the square.
 * radius:	Size of the circle
 *
 * Return value
 * Array of structs (.x, .y)
 * 
 */
function GetCoordsInCircle(mid_x, mid_y, radius)
	mid_x := CInt(mid_x);
	mid_y := CInt(mid_y);
	radius := CInt(radius);
	return error{"errortext":="Hasn't been coded yet!"};
endfunction

/* 
 * GetCoordsInPolygon(coord_list)
 *
 * Purpose
 * Takes in an array of structs containing .x, .y coordinates.
 * Each point is a segment that builds the shape.
 * Finds the coordinates inside of that shape.
 *
 *
 * Parameters
 * coord_list:	Array of structs. Each struct contains an x and y member.
 *
 * Return value
 * Returns an array of structs containing a .x and .y member.
 *
 */
function GetCoordsInPolygon(coord_list)
	var num_coords := coord_list.Size();
	if ( num_coords < 3 )
		return error{"errortext":="Needs atleast three coordinates to form a polygon."};
	endif
	
	//var start_coord := coord_list[1];
	//var end_coord := coord_list[num_coords];
	
	return error{"errortext":="Hasn't been coded yet!"};
endfunction

/*
 * GetCoordsInTriangle(x1, y1, x2, y2, x3, y3)
 *
 * Purpose
 * Retrieves the coordinates in the area of a triangle.
 *
 * Parameters
 * x1, y1: Coordinates of the first vertex of the triangle.
 * x2, y2: Coordinates of the second vertex of the triangle.
 * x3, y3: Coordinates of the third vertex of the triangle.
 *
 * Return value
 * Array of structs (.x, .y)
 * 
 */
function GetCoordsInTriangle(x1, y1, x2, y2, x3, y3)
    var point_list := array{};
    var coord := struct{"x", "y"};

    // Assume the vertices are in counter-clockwise order
    var minX := min(x1, min(x2, x3));
    var minY := min(y1, min(y2, y3));
    var maxX := max(x1, max(x2, x3));
    var maxY := max(y1, max(y2, y3));

    var x, y;
    for (x := minX; x <= maxX; x := x + 1) 
        for (y := minY; y <= maxY; y := y + 1) 
            // Check if the point (x, y) is inside the triangle
            var b1 := ((x2 - x1) * (y - y1) - (x - x1) * (y2 - y1)) / 2.0;
            var b2 := ((x3 - x2) * (y - y2) - (x - x2) * (y3 - y2)) / 2.0;
            var b3 := ((x1 - x3) * (y - y3) - (x - x3) * (y1 - y3)) / 2.0;

            // Check if the point is inside the triangle
            if ((b1 >= 0 && b2 >= 0 && b3 >= 0) || (b1 <= 0 && b2 <= 0 && b3 <= 0)) 
                coord.x := Cint(x);
                coord.y := Cint(y);
                point_list.Append(coord);
                sleepms(2);
            endif
        endfor
        sleepms(2);
    endfor

    return point_list;
endfunction


/*
 * AdjustTriangleCoordinates(caster_x, caster_y, target_x, target_y, range)
 *
 * Purpose
 * Adjusts the coordinates of a triangle based on the target direction.
 *
 * Parameters
 * caster_x, caster_y: Current position of the character casting the spell.
 * target_x, target_y: Position or direction of the target.
 * range: Range or radius of the spell.
 *
 * Return value
 * Array of structs (.x, .y) representing adjusted triangle coordinates.
 * 
 */
function AdjustTriangleCoordinates(caster_x, caster_y, target_x, target_y, range)
    var triangle_coords := array{};
    var coord := struct{"x", "y"};

    // Calculate angle and adjust positions based on the target direction
    var angle := arcotangente(target_y - caster_y, target_x - caster_x);

    // Adjust the starting point of the triangle
    var x1 := caster_x + range * cos(angle);
    var y1 := caster_y + range * sin(angle);

    // Adjust the other two vertices based on a fixed angle increment
    var angle_increment := 2 * ConstPi() / 3.0;

    var x2 := x1 + range * cos(angle + angle_increment);
    var y2 := y1 + range * sin(angle + angle_increment);

    var x3 := x1 + range * cos(angle - angle_increment);
    var y3 := y1 + range * sin(angle - angle_increment);

  // Append adjusted coordinates to the array
    coord.x := CInt(x1);
    coord.y := CInt(y1);
    triangle_coords.Append(coord);

    coord.x := CInt(x2);
    coord.y := CInt(y2);
    triangle_coords.Append(coord);

    coord.x := CInt(x3);
    coord.y := CInt(y3);
    triangle_coords.Append(coord);


    return triangle_coords;
endfunction

/*
 * arcotangente2(y, x)
 *
 * Purpose
 * Computes the arctangent of y/x.
 *
 * Parameters
 * y: Vertical coordinate.
 * x: Horizontal coordinate.
 *
 * Return value
 * Real number in radians.
 * 
 */
function arcotangente(y, x)
    if (x == 0.0)
        return ConstPi() / 2.0; // Handle division by zero case
    endif
    var angle := ATan(y / x);

    if (x < 0.0)
        angle := angle + ConstPi(); // Adjust for the correct quadrant when x is negative
    endif
    return angle;
endfunction
