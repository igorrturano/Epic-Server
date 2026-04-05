use uo;
use math;

include "include/client";
include "include/sounds";
include ":attributes:attributes";
include ":combat:Controller/DamageController_legacy";
include ":timedScripts:timedScripts";
include "spellSounds";
include ":charactercreation:resists";
include ":charactercreation:habilidades";
include ":entropy:entropy";
include ":wrappers:Objects/Model/MobileWrapper";
include ":abilities:models/Ability";
include ":spells:SpellRegistry";

function CastSpell(mobile, spellinfo, targ)
    if (GetCooldown(targ, "spellthief"))
        targ := mobile;
    endif

	if (spellinfo.script != error)
		Start_Script(spellinfo.script, {mobile, targ, spellinfo});
	endif
endfunction

function TNCheckReagents(who, byref spellinfo, scroll)
	//sendsysmessage(who, " " + scroll + spellinfo.type);
	if ( who.npctemplate )
		return 1;
	elseif (scroll == 1)
		return 1;
	elseif ( scroll.isa(POLCLASS_ITEM) )
	//	sendsysmessage(who, " " + scroll);
		return 1;
	endif

	var reagent_costs := dictionary;
	var custo := Cint(spellinfo.power)/2;
	if (custo < 1)
		custo := 1;
	endif
	if (spellinfo.type == "Curar")
		reagent_costs["Garlic"] := custo;
		reagent_costs["Ginseng"] := custo;
	elseif (spellinfo.type == "Destruir")
		reagent_costs["SulphurousAsh"] := custo*2;
	elseif (spellinfo.type == "Fortalecer")
		reagent_costs["MandrakeRoot"] := custo;
		reagent_costs["Garlic"] := custo;
	elseif (spellinfo.type == "Enfraquecer")
		reagent_costs["BloodMoss"] := custo;
		reagent_costs["NightShade"] := custo;
	elseif ( spellinfo.type == "Controlar")
		reagent_costs["BloodMoss"] := custo;
		reagent_costs["SpiderSilk"] := custo;
	elseif ( spellinfo.type == "Entender" )
		reagent_costs["MandrakeRoot"] := 1;
	elseif ( spellinfo.type == "Outro")
		var elem := ResolveSpell(spellinfo.id);
		if (!elem)
			SendSysMessageEx(who, "spellid nao encontrado " + spellinfo.id, SSM_FAIL);
			return 0;
		endif
		reagent_costs := GetConfigStringDictionary( elem, "RegCost" );
	//	sendsysmessage(who, " " + cfg + " " + elem + " " + reagent_costs);
	endif

	//sendsysmessage(who,  " " + reagent_costs);
	var keys := reagent_costs.keys();
	var amount;
	foreach reag in keys

		amount := CInt( reagent_costs[reag] );
		if (!amount)
			amount := 1;
		endif
		var objtype := GetObjTypeByName(reag);
		var dictToAmount := dictionary; //variavel pra ser usada na funcao abaixo
		dictToAmount["objtype"] := objtype;
		var amountinpack := AmountInContainer(who.backpack, dictToAmount);
		//sendsysmessage(who, "reag " + reag + " objtype " + objtype + " pack " + amountinpack);
		if (amountinpack  < amount )
			SendSysMessage(who, "Voce nao possui "+GetObjTypeDesc(objtype, (amount>1)));
			return 0;
		endif
		sleepms(2);
	endforeach

	foreach reag in ( keys )
		amount := CInt( reagent_costs[reag] );
		if (!amount)
			amount := 1;
		endif
		ConsumeSubstance(who.backpack, GetObjTypeByName(reag), amount);
		sleepms(2);
	endforeach

	return 1;
endfunction

function GetSpellDuration(skill)
	return cint(skill/10)*60 + 300;
endfunction

function WriteCantripSpellInfo(who, byref spellinfo)
	var cfg := ReadConfigFile(":spells:config/cantrips");

	var elem := FindConfigElem(cfg, spellinfo.id);
	spellinfo.+name := GetConfigString( elem, "Name" );
    spellinfo.+circle := GetConfigStringArray( elem, "Circle" );
	spellinfo.+alvo := GetConfigString( elem, "Target" );
	spellinfo.+type := GetConfigString( elem, "Type" );
	spellinfo.+range := GetConfigString( elem, "Range" );
	spellinfo.+mana := GetConfigInt( elem, "Mana" );
	spellinfo.+delay := GetConfigInt( elem, "Delay" );
	spellinfo.+difficulty := GetConfigInt( elem, "Difficulty" );
	spellinfo.+description := GetConfigString( elem, "Desc" );
	spellinfo.+script := GetConfigString( elem, "SpellScript" );
	spellinfo.+spelltype := GetConfigString(elem, "Type");
	spellinfo.+flavortext := GetConfigString( elem, "FlavorText" );
	spellinfo.+cycles := GetConfigInt( elem, "Cycles" );
	if (spellinfo.flavortext == error)
		spellinfo.flavortext := "";
	else
		spellinfo.flavortext := "<I> "  + spellinfo.flavortext + " </I>";
	endif
endfunction

function WritespellInfo(who, byref spellinfo)
	// Use SpellRegistry to resolve spell by ID - no more class-based guessing
	var cfg_path := GetConfigFromId(spellinfo.id);
	if (!cfg_path)
		return;
	endif

	var cfg := ReadConfigFile(cfg_path);
	if (!cfg)
		return;
	endif

	var elem := FindConfigElem(cfg, spellinfo.id);
	if (!elem)
		return;
	endif

	spellinfo.+name := GetConfigString( elem, "Name" );
	spellinfo.+powerwords := GetConfigString( elem, "PowerWords" );
	spellinfo.+prayer := GetConfigStringArray( elem, "Prayer" );
    spellinfo.+circle := GetConfigStringArray( elem, "Circle" );
	spellinfo.+alvo := GetConfigString( elem, "Target" );
	spellinfo.+type := GetConfigString( elem, "Type" );
	spellinfo.+range := GetConfigString( elem, "Range" );
	spellinfo.+targtype := GetConfigString( elem, "TargetType" );
	spellinfo.+mana := GetConfigInt( elem, "Mana" );
	spellinfo.+delay := GetConfigInt( elem, "Delay" );
	spellinfo.+difficulty := GetConfigInt( elem, "Difficulty" );
	spellinfo.+description := GetConfigString( elem, "Desc" );
	spellinfo.+script := GetConfigString( elem, "SpellScript" );
	spellinfo.+reagents := GetConfigStringDictionary(elem, "RegCost");
	spellinfo.+spelltype := GetConfigString(elem, "Type");
	spellinfo.+align := GetConfigString(elem, "Align");
	spellinfo.+flavortext := GetConfigString( elem, "FlavorText" );
	spellinfo.+cycles := GetConfigInt( elem, "Cycles" );
	spellinfo.+corruption := GetConfigInt( elem, "Corruption" );
	spellinfo.+School := GetConfigString( elem, "School" );
	if (spellinfo.flavortext == error)
		spellinfo.flavortext := "";
	else
		spellinfo.flavortext := "<I> "  + spellinfo.flavortext + " </I>";
	endif

endfunction

function Spell_Target(who, opts)
	var targ := Target(who, opts);
	if (Distance(targ, who) > 8)
		SendSysMessageEx(who, "O alvo esta muito distante.", SSM_FAIL);
		return 0;
	endif
	if (!targ)
		SendSysMessageEx(who, "Cancelado.", SSM_FAIL);
		return 0;
	endif
	return targ;
endfunction

//////////////////////////////////////////////////////////////
//////// Corromper(jogador, alvo, mana, circulo) - By Duttones
//////// Adapted to use entropy system - corruption decreases entropy
//////////////////////////////////////////////////////////////
function Corromper(who, spell, targ)
	// Check if player has reached maximum corruption (-20000 entropy)
	var wrapper := MobileWrapper(who);
	if (wrapper.GetObjRef().errortext)
		return 0;
	endif

	var current_entropy := wrapper.GetEntropy();
	if (current_entropy <= -20000)
		return 0;
	endif

	// Read spell configuration via SpellRegistry
	var elem := ResolveSpell(spell);
	if (!elem)
		return 0;
	endif
	var spellinfo := dictionary;
	spellinfo.+name := GetConfigString( elem, "Name" );
	spellinfo.+mana := GetConfigInt( elem, "Mana" );
	spellinfo.+cycles := GetConfigInt( elem, "Circle" );
	spellinfo.+corruption := GetConfigInt( elem, "Corruption" );
	var variavel := 0.0;
	var corrupcao;

	// Test willpower resistance
	if (CheckResist(who, WILL, RESIST_HARD+spellinfo.cycles))
		return 0;
	endif

	SendSysMessageEx(who, "Você não resiste a corrupção inevitável da arte necromântica. Seu corpo começa a sentir.", SSM_FAIL);

	// Check if it's a banned spell and define the multiplier
	if (spellinfo.corruption != 1)
		variavel := CDbl(spellinfo.corruption) / 10.0;
	endif

	// Check if target is a player and increase corruption
	if (targ)
		if (targ.acctname)
			var chardata := GetObjProperty(targ, "chardata");
			if (chardata.magia == "Sacerdote")
				variavel := variavel + 0.8;
			else
				variavel := variavel + 0.3;
			endif
		endif
	endif

	// Calculate corruption gain
	corrupcao := CDbl(spellinfo.mana) / 5.0 * CDbl(spellinfo.cycles);
	variavel := corrupcao * variavel;
	corrupcao := Cint(corrupcao + variavel);

	// Apply corruption as negative entropy (corruption decreases entropy)
	var entropy_loss := corrupcao * -1;

	// Ensure we don't go below -20000 entropy
	if ((current_entropy + entropy_loss) < -20000)
		entropy_loss := -20000 - current_entropy;
	endif

	// Apply the entropy change
	if (entropy_loss != 0)
		UpdateMobileEntropy(who, entropy_loss);
	endif

	return 1;
endfunction


