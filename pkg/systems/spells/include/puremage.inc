




function dispel(targ);

	var timer_list := TS_GetTimerList(targ);
	var names := array;

	foreach timer in ( timer_list.keys() )
		timer := lower(timer);
		if (timer == "agility" )
			TS_LowerDuration(targ, timer, -1);
		elseif ( timer == "strength");
			TS_LowerDuration(targ, timer, -1);
		elseif ( timer == "intelligence")
			TS_LowerDuration(targ, timer, -1);
		elseif ( timer == "velocidade")
			TS_LowerDuration(targ, timer, -1);
		elseif ( timer == "PhysicalResist")
			TS_LowerDuration(targ, timer, -1);
		elseif ( timer == "bless")
			TS_LowerDuration(targ, timer, -1);
		elseif ( timer == "phmod")
			TS_LowerDuration(targ, timer, -1);
		elseif (timer == "fimod")
			TS_LowerDuration(targ, timer, -1);
		elseif (timer == "comod")
			TS_LowerDuration(targ, timer, -1);
		elseif (timer == "enmod")
			TS_LowerDuration(targ, timer, -1);
		elseif ( timer == "pomod")
			TS_LowerDuration(targ, timer, -1);
		elseif ( timer == "disfarce")
			TS_LowerDuration(targ, timer, -1);
		elseif ( timer == "gloria")
			TS_LowerDuration(targ, timer, -1);
		elseif (timer == "desviarprojetil")
			TS_LowerDuration(targ, timer, -1);
		endif
		
	endforeach
	
	EraseObjProperty(targ, "tirocerteiro");
	EraseObjProperty(targ, "evasao");
endfunction 
