use uo;
use os;

include ":attributes:attributeConstants";
include ":attributes:attributes";
include ":attributes:skillCheck";
include ":crafting:toolWear";
include ":gathering:mining";
include "include/say";
include ":wrappers:Objects/Model/MobileWrapper";
include ":abilities:models/Ability";

const METEORITE_ANIMATION := 0xB;
const METEORITE_SOUND     := 0x126;
const METEORITE_FX_SMOKE  := 0x3728;

function IsMeteorite(item)
	if (!item || item == error)
		return 0;
	endif
	return CInt(GetObjProperty(item, "IsMeteorite"));
endfunction

function MeteoriteOreType(meteorite)
	return CInt(GetObjProperty(meteorite, "OreType"));
endfunction

function MeteoriteOreLeft(meteorite)
	return CInt(GetObjProperty(meteorite, "OreLeft"));
endfunction

function DepetrifyMeteorite(meteorite)
	if (!meteorite || meteorite == error)
		return 0;
	endif
	PlayObjectCenteredEffect(meteorite, METEORITE_FX_SMOKE, 10, 16);
	DestroyItem(meteorite);
	return 1;
endfunction

function MeteoriteSwingAndExtract(who, tool, meteorite)
	if (!meteorite || meteorite == error)
		return 0;
	endif

	var i;
	for (i := 0; i < 3; i := i + 1)
		if (CoordinateDistance(who.x, who.y, meteorite.x, meteorite.y) > 2)
			SendSysMessage(who, "Muito longe.");
			return 0;
		endif
		PerformAction(who, METEORITE_ANIMATION);
		PlaySoundEffect(who, METEORITE_SOUND);
		sleep(1);
	endfor

	var ore_type := MeteoriteOreType(meteorite);
	if (!ore_type)
		SendSysMessage(who, "Este meteorito não tem minério configurado.");
		return 0;
	endif

	var ore_left := MeteoriteOreLeft(meteorite);
	if (ore_left <= 0)
		DepetrifyMeteorite(meteorite);
		return 0;
	endif

	var difficulty := DetermineMiningDifficulty(ore_type);
	if (MobileWrapper(who).HasAbility(ABILITY_ID::PROSPECTOR))
		difficulty := difficulty - 5;
	endif

	if (SkillCheck(who, GATHERING, difficulty) <= 0)
		SendSysMessage(who, "Você falhou em extrair minério.");
		return 1;
	endif

	var amt := CInt(AP_GetSkill(who, GATHERING) / 20);
	var chardata := GetObjProperty(who, "chardata");
	if (MobileWrapper(who).HasAbility(ABILITY_ID::TALENTED_MINER) || chardata.trait == "Cavar Buracos")
		amt := amt + 2;
	endif
	if (amt <= 0)
		amt := 1;
	endif
	if (amt > ore_left)
		amt := ore_left;
	endif

	ore_left := ore_left - amt;
	SetObjProperty(meteorite, "OreLeft", ore_left);

	var minerio := CreateItemInBackPack(who, ore_type, amt);
	if (minerio && minerio != error)
		minerio.movable := 1;
	endif
	SendSysMessage(who, "Você extraiu " + amt + " minério(s) do meteorito.");

	if (ore_left <= 0)
		SendSysMessage(who, "O meteorito se desfaz após a última extração.");
		DepetrifyMeteorite(meteorite);
		return 0;
	endif

	return 1;
endfunction

function MeteoriteExtractFlow(who, tool, meteorite)
	if (!IsMeteorite(meteorite))
		return 0;
	endif
	if (CoordinateDistance(who.x, who.y, meteorite.x, meteorite.y) > 2)
		SendSysMessage(who, "Muito longe.");
		return 0;
	endif

	while (MeteoriteSwingAndExtract(who, tool, meteorite))
		CheckToolWear(who, tool, GATHERING);

		var stam_cost := 5;
		if (MobileWrapper(who).HasAbility(ABILITY_ID::EXTRACTOR_VIGOR))
			stam_cost := 3;
		endif

		if (AP_GetVital(who, "Stamina") < stam_cost)
			SendSysMessage(who, "Voce esta muito cansado para minerar.");
			break;
		endif
		AP_ConsumeVital(who, "Stamina", stam_cost);
		sleep(3);
	endwhile

	SendSysMessage(who, "Você parou de minerar.");
	return 1;
endfunction
