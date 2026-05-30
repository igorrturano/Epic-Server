use uo;

function RemoveNpcFromSpawnpoint(npcserial, spawnpointserial)

	var spawnpoint := SystemFindObjectBySerial(spawnpointserial);
	
	var spawns := GetObjProperty(spawnpoint, "Children");
	
	var i := 1;
	foreach spawn in spawns
		if (spawn == npcserial)
			spawns[i] := "Empty";
		endif
		i := i + 1;
	endforeach

endfunction