/*
 * $Id: death.inc 664 2005-10-27 03:36:13Z muaddiblsd $
 *
 */


use uo;
use os;
use util;

include ":brainAI:npcUtil";
include ":attributes:attributes";
include ":timeutils:time";

/*
 * DP_PlayDeathSound(corpse, ghost:=0)
 *
 * Purpose
 * Plays a sound effect when something dies.
 *
 * Parameters
 * object:	Either a corpse or a ghost to play the sound on.
 *
 * Return value
 * Sound number that was played
 */
function DP_PlayDeathSound(object)
	var death_sound := 0;

	if ( object.IsA(POLCLASS_CORPSE) )
		var template := GetObjProperty(object, "npctemplate"); // Set by core
		if ( template )
			var cfg_info := NPC_GetNPCConfig(template);
			var sounds := GetConfigStringArray(cfg_info, "DeathSound");
			death_sound := CInt(sounds[RandomInt(sounds.Size())+1]);
		endif
	endif
	if ( !death_sound || object.corpsetype == 401 )
		// No sound selected yet.
		if ( object.graphic == 402 || object.corpsetype == 400 || object.corpsetype == 1828  || object.corpsetype == 1830 || object.corpsetype == 1832  || object.corpsetype == 1834  )
			 // Male ghost / corpse
			var sounds := array{347, 348, 349, 350};
			death_sound := sounds[RandomInt(sounds.Size())+1];
		elseif ( (GetObjProperty(object, "female") == 1) || object.graphic == 401 ||  object.graphic == 403 || object.corpsetype == 401 || object.corpsetype == 1829 || object.corpsetype == 1831 || object.corpsetype == 1833 || object.corpsetype == 1835 )
			 // Female ghost / corpse
			var sounds := array{337, 338, 339, 340};
			death_sound := sounds[RandomInt(sounds.Size())+1];
		endif
	endif

	if ( death_sound )
		PlaySoundEffect(object, death_sound);
	endif

	return death_sound;
endfunction

/*check the deathpoints. if its 0 the player is dead and return 0 else return 1 */

function CheckDeathPoints(who)

	if (!who.npctemplate)
		//if (who.cmdlevel < 1)

			var deathpoints := RegenerateDeathPoints(who);

			deathpoints += 1;

			if( deathpoints >= 4 )
				SendSysMessage(who, "Voce ja desmaiou muitas vezes. Seus Stats foram penalizados.");
				AP_ModifyStatMod(who, DEXTERITY, -10);
				AP_ModifyStatMod(who, STRENGTH, -10);
				AP_ModifyStatMod(who, INTELLIGENCE, -10);
			endif

			SetObjProperty(who, "deathpoints", deathpoints);
			SetObjProperty(who, "LastFaint", polcore().systime);
			RecalcVitals(who);
			return 1;

		//endif
	endif

	return 0;

endfunction

function GetNPCDeathpointsDmg(template)

	var npcelem := NPC_GetNPCConfig(template);

	var DeathPoints := npcelem.DeathPoints;

	if (!DeathPoints)
		return 0;
	endif

	return DeathPoints;

endfunction

function RegenerateDeathPoints(who)

	var deathpoints := GetObjProperty(who, "deathpoints");
	if (deathpoints == error)
		deathpoints := 0;
	else
		var ratio := CInt( (GetTime() - GetObjProperty(who, "LastFaint"))/SEC_HOUR );
		if(  ratio > 0 )

			//if( deathpoints > 3 )
			//	RegenDpStats(who, ratio);
			//endif

			var deathPointsToHeal := deathpoints - 3; //calcula quanto de stat tem q regenerar
			if (deathPointsToHeal > 0)
				RegenDpStats(who, deathPointsToHeal);
			endif


			if (deathpoints > 0)
				deathpoints -= ratio;
			endif
			if( deathpoints < 0 )
				deathpoints := 0;
			endif

			SetObjProperty(who, "LastFaint", GetTime());
		endif
	endif

	SetObjProperty(who, "deathpoints", deathpoints);

	return deathpoints;

endfunction

function RegenDpStats(who, ratio)
	var bonus := ratio*10;

	AP_ModifyStatMod(who, INTELLIGENCE, bonus);
	AP_ModifyStatMod(who, DEXTERITY, bonus);
	AP_ModifyStatMod(who, STRENGTH, bonus);


	/*if( bonus > CInt(AP_GetStatMod(who, DEXTERITY)*(-1)) )
		bonus := CInt(AP_GetStatMod(who, DEXTERITY)*(-1));
	endif
	AP_ModifyStatMod(who, DEXTERITY, bonus);

	bonus := ratio*10;
	if( bonus > CInt(AP_GetStatMod(who, STRENGTH)*(-1)) )
		bonus := CInt(AP_GetStatMod(who, STRENGTH)*(-1));
	endif
	AP_ModifyStatMod(who, STRENGTH, bonus);

	bonus := ratio*10;
	if( bonus > CInt(AP_GetStatMod(who, INTELLIGENCE)*(-1)) )
		bonus := CInt(AP_GetStatMod(who, INTELLIGENCE)*(-1));
	endif
	AP_ModifyStatMod(who, INTELLIGENCE, bonus);*/
endfunction
