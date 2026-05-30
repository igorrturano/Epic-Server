
use uo;
use math;
use os;
use util;

const FACING_NORTH	:= 0x0;
const FACING_NORTH_EAST	:= 0x1;
const FACING_EAST	:= 0x2;
const FACING_SOUTH_EAST	:= 0x3;
const FACING_SOUTH	:= 0x4;
const FACING_SOUTH_WEST	:= 0x5;
const FACING_WEST	:= 0x6;
const FACING_NORTH_WEST	:= 0x7;


function IsBehind( tgt_mob, locx, locy )
    var frot := Abs(((tgt_mob.facing+4)%8)-GetFacing(tgt_mob.x,tgt_mob.y,locx,locy));
    if ( frot > 4 ) 
    	frot := 8-frot; 
    endif
    
    return ( frot < 2 );
endfunction

function IsFacing(mobile, x, y)
	return (mobile.facing == GetFacing(mobile.x,mobile.y, x,y));
endfunction
