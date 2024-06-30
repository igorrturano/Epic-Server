use uo;
use os;

include ":attributes:attributes";
include "include/shapes";
include "include/client";
include "include/sounds";
include "include/facings";
include "include/damage";

function PlayLightningSoundEffect(mobile)

	var lightning := {41, 42, 519};

	playsoundeffect(mobile, lightning[(randomint(3)+1)]);

endfunction

function FireballEffect(npc, opponent, skill)

	PlayMovingEffect(npc, opponent, GFX_LARGE_FIREBALL, 10, 10, 1);

	var coord_list := GetCoordsInLine(npc.x, npc.y, opponent.x, opponent.y);
	foreach coord in coord_list
		if ( _coord_iter == 1 ) // Dont make one where its standing
			continue;
		endif
		
		var field := CreateItemAtLocation(coord.x, coord.y, npc.z, "FireField_NS", 1);
		field.SetDuration(2);
		sleepms(105);
	endforeach

	PlaySoundEffect(opponent, 0x208);
	var dice := cstr(skill/10) + "d4";

	var range := 3;
	if (skill > 80)
		range := 4;
	endif
	var coords := GetCoordsInSquare(opponent.x, opponent.y, range);
	foreach coord in coords;
		if ( RandomInt(2) != 1 ) // Don't place a fire field
			continue;
		endif
		case( RandomInt(3) )
			default:
				PlayStationaryEffect(coord.x, coord.y, npc.z, GFX_EXPLODE_1, 10, 10, 0);
				break;
			1:
				PlayStationaryEffect(coord.x, coord.y, npc.z, GFX_EXPLODE_2, 10, 10, 0);
				break;
			2:
				PlayStationaryEffect(coord.x, coord.y, npc.z, GFX_EXPLODE_3, 10, 10, 0);
				break;
		endcase
		var field := CreateItemAtLocation(coord.x, coord.y, opponent.z, "FireFieldNS", 1);
		field.SetDuration(5);
		sleepms(5);
	endforeach

	ApplyDamageEX(opponent, (RandomDiceRoll(dice) + skill/2), DMG_FIRE);

	foreach mobile in ListMobilesInLineOfSight(opponent, range)
		ApplyDamageEX(mobile, (RandomDiceRoll(dice) + skill/2), DMG_FIRE);
	endforeach


endfunction

function HaveMetalArmor(who)

	foreach thing in ListEquippedItems( who )
		if ( (thing.objtype == 0x13ec) || (thing.objtype == 0x13ed) || (thing.objtype == 0x13BF) || (thing.objtype == 0x13C4) || (thing.objtype == 0x1415) || (thing.objtype == 0x1416) )
			return 1;
		endif
	endforeach

	return 0;

endfunction

function BoltJump(byref mobilelist, who, dice)

	foreach mobile in ListMobilesInLineOfSight(who, 5)
		var canBolt := 1;
		foreach oldtarget in mobilelist
			if (oldtarget == mobile)
				canBolt := 0;
			endif
		endforeach
		if (canBolt)
			if (HaveMetalArmor(mobile) )
				ApplyDamageEX(mobile, (RandomDiceRoll(dice) + 100/10), DMG_ENERGY);
				PlayLightningSoundEffect(mobile);
				PlayLightningBoltEffect(mobile );
				mobilelist.append(mobile);
				BoltJump(mobilelist, mobile, "1d10");
			endif
		endif
		sleepms(200);
	endforeach

endfunction

function LightningEffect(npc, opponent, skill)


	PlayLightningSoundEffect(opponent);
	PlayLightningBoltEffect(opponent );
	var dice := cstr(skill/10) + "d10";
	ApplyDamageEX(opponent, (RandomDiceRoll(dice) + skill/10), DMG_ENERGY);
	var range := skill / 20 ;
	if (range < 2)
		range := 2;
	endif

	var mobilelist := {};
	mobilelist.append(opponent);

	BoltJump(mobilelist, opponent, dice);


endfunction

