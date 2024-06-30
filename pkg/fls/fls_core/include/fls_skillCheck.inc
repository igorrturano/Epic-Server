
/* skillcheck do fls. apenas 1 pequena modificação, eh passado um novo parametro MOD,
* que inclui outros modificadores que reduzem a skill antes do check e sem ser uma
* skillmod. Por exemplo, se usar disarm trap com luva reduz sua skill efetiva em 5%
* eh so passar direto no check. (assimevita o bug do perma boost se usar cprop)
*/

use uo;
use os;
use util;
use cfgfile;

include ":attributes:skillCheck";


function SkillCheckMod(mobile, attrib_name, difficulty, checkmod:=0, award_diff:=0, advance_flags:=ADV_SKILL)

	if ( mobile.dead )
		SendSysMessage(mobile, "Voce nao pode usar skills nesse estado.");
		return -100;
	elseif ( GetObjProperty(mobile, "DisableSkills") )
		return -100;
	elseif ( GetObjProperty(mobile, "NoGains") )
		advance_flags := ADV_DISABLE;
	endif

	var effective_skill := AP_GetTrueSkill(mobile, attrib_name) + checkmod;

	if ( difficulty != -1 )
		difficulty := CDbl(difficulty);
	endif

	var roll := RandomInt(100)+1;
	var pass_success;
	var chance;
	
	if (difficulty == -1 )
		pass_success := (effective_skill - roll ) + 7;
	else
		if (difficulty > effective_skill + 25) //se for mto dificil
			pass_success :=  -100;
		else
			chance :=  ( ((effective_skill + 20)*100) / ((difficulty + 20)*200) ) * 100;
			if (chance < 7)
				chance := 7;
			endif
			pass_success := chance - roll;
		endif
	endif

	if ( difficulty == -1 )
		difficulty := effective_skill;
	endif

	if ( !award_diff )
		award_diff := difficulty;
	endif

	if ( (advance_flags & ADV_SKILL) )
		CheckSkillAdvance(mobile, attrib_name, award_diff, difficulty);
	endif

	return pass_success;
endfunction
