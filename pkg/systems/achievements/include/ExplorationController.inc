


function AchievementCombat_Advance(player, combatInfo)
endfunction

function IncrementStepCounter(player)
    var steps := player.GetProp("#steps");
	if (!steps)
		steps := 0;
	endif

    steps += 1;

    player.SetProp("#steps", steps);
endfunction