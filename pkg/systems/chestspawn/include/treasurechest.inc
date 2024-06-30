use uo;
use os;

include ":loot:lootParser";
include ":traps:traps";
include "include/say";

function CreateTreasureChests(lvl, amt, rng, x, y, z, realm, lootname)
	var newx, newy, newz, checker, type, gfx, lootgroup, chest;
	var arraychest := array{};
    if (x == error)
    return  arraychest;
    endif
	if(rng > 10)
		rng := 10;
	endif
	var total := 0;
	while(total < amt)
		chest := 0;
		checker  := 0;
		var tries := 0;
		while((checker == 0) )
			checker := 0;
			newx := cint(x + RandomInt(rng) - RandomInt(rng));
			newy := cint(y + RandomInt(rng) - RandomInt(rng));
			newz := GetWorldHeight(x, y, realm);
			if (z > newz+5)
				newz := z;
			elseif(z < newz-5)
				newz := z;
			endif
			if(len(ListItemsNearLocation(newx, newy, newz, 0, realm)) == 0)
				type := RandomInt(4)+1;
				case(type)
					1: gfx := 3650;
					2: gfx := 3651;
					3: gfx := 3703;
					4: gfx := 3648;
					5: gfx := 3708;
				endcase
				chest := CreateItemAtLocation(newx, newy, newz, gfx, 1, realm);
				if(chest)
					checker := 1;
					chest.movable := 0;
				else
					checker := 0;
					tries := tries + 1;
				endif
			endif
			if (tries > 30)
				var players := EnumerateOnlineCharacters();
				foreach player in players
					if (player.cmdlevel > 2 )
						SendSysMessageEx(player, "Chestspawner em x " + x + " y " + y + " z " + z + " esta consumindo muito a CPU, delete-o ou recrie-o". SSM_FAIL);
					endif
				endforeach
			endif
			sleep(10);
		endwhile

		arraychest.append(chest);
		chest.locked :=  1;
		chest.movable := 0;
		//chest.usescript := ":tinkering:tinkerTraps";
		SetObjProperty(chest, "level", lvl);

		var trap := RandomInt(3)+1;
		case(trap)
			1: AddTrap(chest, "Needle", lvl);
			2: AddTrap(chest, "Explosion", lvl);
			3: AddTrap(chest, "PoisonGas", lvl);
		endcase

		lootgroup := lootname;
		Loot_Generate(chest, lootgroup);

		total := total + 1;
		sleep(10);
	endwhile

	return arraychest;
endfunction
