// use os;
// use cfgfile;
use math;

include ":equipsys:utils/consts";
include ":combat:Model/Damage";
include "ArmorController";

function GetDurabilityByType(type)
	var maxhp := 0;
	
	if (Upper(type) == Upper(DAMAGE_TYPE::SLASH))
		maxhp := 45;
	elseif (Upper(type) == Upper(DAMAGE_TYPE::PIERCING))
		maxhp := 60;
	elseif (Upper(type) == Upper(DAMAGE_TYPE::BLUNT))
		maxhp := 75;
	elseif (Upper(type) == Upper(SHIELD_BUCKLER))
		maxhp := 40;
	elseif (Upper(type) == Upper(SHIELD_SMALL))
		maxhp := 60;
	elseif (Upper(type) == Upper(SHIELD_MEDIUM))
		maxhp := 80;
	elseif (Upper(type) == Upper(SHIELD_LARGE))
		maxhp := 100;
	elseif (Upper(type) == Upper(SHIELD_TOWER))
		maxhp := 120;
	else
		var resists := GetResistPhysicalByProtection(type);
		maxhp := Max(array{resists.GetSlashResist(), resists.GetPiercingResist(), resists.GetBluntResist()});
	endif

	return maxhp;
endfunction

function GetDurabilityMultiplierByMaterial(material)
	var modifier := 0;

	case (material)
		0x6309: modifier := 1; break; // Ferro

		default: modifier := 0.5;
	endcase

	return CDbl(modifier);
endfunction

function GetWeightMultiplierByMaterial(material)
	var modifier := 0;

	case (material)
		0x6994: modifier := 1.3; break; // Bronze
		0x6309: modifier := 1.1; break; // Ferro
		0x6315: modifier := 0.9; break; // Illyra
		0x6311: modifier := 0.8; break; // Mithril

		default: modifier := 1;
	endcase

	return CDbl(modifier);
endfunction
