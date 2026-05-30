//////////////////////////////////////////////////////////////////////////
//
// Arrays.inc
//
// Contains some useful functions for arrays.
//
// Originally uploaded by Shilohen.
// One function added by Madman Across the Water.
//
//////////////////////////////////////////////////////////////////////////

use basic;
use math;
use os;
use uo;

/*
 * ImplodeArray(glue, array)
 *
 * Purpose
 * Returns a string containing the array elements in the same order with
 * the 'glue' string in between each array element.
 *
 * Parameters
 * glue:	String to place between each array element.
 * array:	Array to turn into a string.
 *
 * Return Value
 * Returns a string on success.
 * Returns an error on failure.
 *
 */
function ImplodeArray(glue, byref the_array)
	if ( Lower(TypeOf(the_array)) != "array" )
		return error{"errortext":="Object passed was not an array."};
	endif

	var str_array := "";
	foreach element in ( the_array )
		str_array += (CStr(element)+glue);
		SleepMS(2);
	endforeach

	return str_array;
endfunction

/*
 * FindInArray(byref the_array, find_what)
 *
 * Purpose
 * Finds the first instance of something in an array.
 *
 * Parameters
 * the_array:	Array to search.
 * find_what:	What to look for.
 *
 * Return value
 * Returns > 1 (the index) if it is found.
 * Returns 0 if 'find_what' was not found.
 *
 */
function FindInArray(byref the_array, find_what)
	if ( Lower(TypeOf(the_array)) != "array" )
		return error{"errortext":="Object passed was not an array."};
	endif

	foreach index in ( the_array )
		if ( index == find_what )
			return _index_iter;
		endif
		sleepms(2);
	endforeach

	return 0;
endfunction

/*
 * GetArrayRange(the_array, start, stop)
 *
 * Purpose
 * Retrieves a range from the array.
 *
 * Parameters
 * the_array:	The array to extract data from.
 * start:	Start index to get data from.
 * stop:	End index to get data from.
 *		If stop is 0, will go until the end of the array is reached.
 *
 * Return value
 * Returns an array on success.
 * Returns an error on failure.
 *
 */
function GetArrayRange(byref the_array, start, stop:=0)
	if ( Lower(TypeOf(the_array)) != "array" )
		return error{"errortext":="Object passed was not an array."};
	endif

	start := CInt(start);
	stop := CInt(stop);

	if ( start < 1 )
		start := 1;
	endif
	if ( !stop )
		stop := the_array.Size();
	elseif ( stop > the_array.Size() )
		stop := the_array.Size();
	endif

	var new_array := array{};
	for ( start; start<=stop; start:=start+1 )
		new_array.Append(the_array[start]);
		SleepMS(2);
	endfor

	return new_array;
endfunction

/*
 * BreakArray(the_array, size)
 *
 * Purpose
 * Breaks an array up into smaller arrays.
 *
 * Parameters
 * the_array:	The array that will be broken up.
 * size:	Size of the new arrays.
 *
 * Return value
 * Returns an array of arrays
 *
 */
function BreakArray(byref the_array, size:=1)
	if ( Lower(TypeOf(the_array)) != "array" )
		return error{"errortext":="Object passed was not an array."};
	endif

	if ( the_array.Size() > size )
		var new_arrays := array();
		var arrays_needed := Ceil(CDbl(the_array.Size()) / CDbl(size));

		var i := 1;
		var start := i;
		var stop := size;
		for ( i; i<=arrays_needed; i:=i+1 )
			new_arrays.Append(GetArrayRange(the_array, start, stop));
			start := stop+1;
			stop := stop+size;
			SleepMS(2);
		endfor

		return new_arrays;
	else
		return array{the_array};
	endif
endfunction

////////////////////////////////////////////////////////////////
// Function	InsertElementInSortedArray(byref the_array, byref element)
//
// Purpose	Insert an element at the good index, must be the only one
//			instance of that element in the array, no dupe allowed.
//
// Parameters	the_array containing the element
//				what to insert
// Return		non-zero if the element could be inserted, error otherwise.
// Errortext	<Element <element> already exists in the array>
//				<Couldn't insert element <element>>
////////////////////////////////////////////////////////////////
function InsertInSortedArray( byref the_array, byref element )

	var max := the_array.Size();
	if( !max )
		return the_array.Append(element);
	endif

	var ctr;
	var min := 1;
	if( the_array[1].IsA(POLCLASS_UOBJECT) )
		while( min <= max )
			ctr := Cint((min+max)/2);
			if( the_array[ctr].serial == element.serial )
				var ret := error;
				ret.+errortext := "Element " + element + " already exists in array";
				return ret;
			elseif( the_array[ctr].serial < element.serial )
				min := ctr + 1;
			else
				max := ctr - 1;
			endif
		endwhile
	else
		while( min <= max )
			ctr := Cint((min+max)/2);
			if( the_array[ctr] == element )
				var ret := error;
				ret.+errortext := "Element " + element + " already exists in array";
				return ret;
			elseif( the_array[ctr] < element )
				min := ctr + 1;
			else
				max := ctr - 1;
			endif
		endwhile
	endif

	return the_array.Insert( min, element );

endfunction


////////////////////////////////////////////////////////////////
// Function	RemoveElement(byref the_array, byref element)
// Parameters	the_array: the array containing the element
//				element: element to remove
// Return 1 if the elment was successfully removed or error.
//
////////////////////////////////////////////////////////////////
function RemoveElement( byref the_array, byref element )

	var index := element in the_array;
	if( index )
		return the_array.erase(index);
	else
		return index;
	endif

endfunction

// FillArray(starts, ends, incr := 1)
//
// This function is handed start and end numbers, and increments. If you send it
// (1,5,1), for instance, it will return an array {1,2,3,4,5}. (5,1,-2) will return
// {5,3,1}. (1,5,3) will return {1,4}.
// Added by Madman Across the Water

function FillArray(starts, ends, incr := 1)

	var retval :="";
	if(TypeOf(starts) != "Integer")
		retval := error;
		retval.errortext := "starts is not an integer";
		return retval;
	endif
	if(TypeOf(ends) != "Integer")
		retval := error;
		retval.errortext := "ends is not an integer";
		return retval;
	endif
	if(TypeOf(incr) != "Integer")
		retval := error;
		retval.errortext := "incr is not an integer";
		return retval;
	endif

	var newarray := array;
	if (incr == 0)
	     return starts;
	elseif (incr > 0)
	     var i:= 1;
	     while (1)
	         newarray[i] := starts;
	         starts := starts + incr;
	         if (starts > ends)
	         	return newarray;
	         endif
	         i := i + 1;
	     endwhile
	else
	     var i := 1;
	     while (1)
	         newarray[i] := starts;
	         starts := starts + incr;
	         if (starts < ends)
	         	return newarray;
	         endif
	         i := i + 1;
	     endwhile
	endif

	return 0;
endfunction

function FindObjInModel(search_for, equal, to_search)
	foreach item in to_search
		var item_prop := item.get_member(search_for);
		if ( item_prop == equal)
			return item;
		endif
      sleepms(2);
	endforeach

	return 0;
endfunction

function removeIndexes(byref the_array, index_array)
   index_array.Reverse(); //cheat for pol
   foreach index in index_array
      the_array.erase(index);
      sleepms(2);
   endforeach
endfunction

/*
 * RemoveFromArray(byref the_array, find_what)
 *
 * Purpose
 * Finds the first instance of something in an array.
 *
 * Parameters
 * the_array:	Array to search.
 * find_what:	What to look for.
 *
 * Return value
 * Returns > 1 (the index) if it is found.
 * Returns 0 if 'find_what' was not found.
 *
 */
function RemoveFromArray(the_array, delete_what)
	if ( Lower(TypeOf(delete_what)) != "array")
		delete_what := array{delete_what};
	endif

	var iter_to_remove := array{};
	foreach item in (the_array)
		if (item in delete_what)
			iter_to_remove += _item_iter;
		endif
		sleepms(5);
	endforeach

	iter_to_remove.Reverse();
	foreach iter in iter_to_remove
		the_array.Erase(iter);
	endforeach

	return the_array;
endfunction

/*
 * FindAllElemInArray(byref the_array, find_what)
 *
 * Purpose
 * Finds the all instances of something in an array.
 *
 * Parameters
 * the_array:   Array to search.
 * find_what:   What to look for.
 *
 * Return value
 * Returns all elem of what its searching
 * Returns 0 if 'find_what' was not found.
 *
 */
function FindAllElemInArray(byref the_array, find_what, return_index := 0)
    if ( Lower(TypeOf(the_array)) != "array" )
        return error{"errortext":="Object passed was not an array."};
    endif

    var to_return := array{};
    foreach index in ( the_array )
        if ((TypeOf(find_what) == "Struct" || TypeOf(index) == "Struct") || (TypeOf(index) == "Dictionary" || TypeOf(find_what) == "Dictionary"))
            foreach key in (find_what.Keys())
                if (index.Exists(key) && find_what[key] == index[key])
					if (return_index)
                   		to_return.append(_index_iter);
					else
                   		to_return.append(index);
					endif
                endif
                sleepms(2);
            endforeach
        elseif ( index == find_what )
			   to_return.append(index);
        endif
        sleepms(2);
    endforeach

	if (to_return.size() >= 1)
		return to_return;
	endif
	
    return 0;
endfunction
