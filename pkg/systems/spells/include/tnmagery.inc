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
	//se for um spell com script (ou seja o novo sistema) executa o script e retorna, caso o contrario executa o de magias custom
    if (GetCooldown(targ, "spellthief"))
        targ := mobile;
    endif

	if (spellinfo.script != error)
		var e := Start_Script(spellinfo.script, {mobile, targ, spellinfo});
		return;
	else
		PrintTextAbove(mobile, " " + spellinfo);
	endif

	var alvos := array;

	if (spellinfo.targ == "Unico")
		SendSysMessageEx(mobile, "Escolha o alvo.");
		var targ := Target(mobile, TGTOPT_CHECK_LOS);
		if (!targ )
			SendSysMessageEx(mobile, "Cancelado.");
			return;
		endif
		if (spellinfo.range == "Toque")
			if (Distance(mobile, targ) > 1)
				SendSysMessageEx(mobile, "O alvo esta muito distante e este encantamento precisa que voce o toque.");
				return;
			endif
		else
			if (Distance(mobile, targ) > 8 )
				SendSysMessageEx(mobile, "O alvo esta fora do alcance do encantamento.");
				return;
			endif
		endif

		if ( (spellinfo.targtype == "Animados") && (!targ.isa(POLCLASS_MOBILE)) )
			SendSysMessageEx(mobile, "Este encanto so afeta seres animados.");
			return;
		elseif ( (spellinfo.targtype == "Inanimados") && (targ.isa(POLCLASS_MOBILE)) )
			SendSysMessageEx(mobile, "Este encanto so afeta seres inanimados.");
			return;
		endif
		alvos.append(targ);
	else
		SendSysMessageEx(mobile, "Onde voce deseja invocar o encantamento?");
		if (mobile.acctname)
			var targ := TargetCoordinates(mobile);
		endif
		if (!CheckLosAt(mobile, targ.x, targ.y, targ.z))
			SendSysMessageEx(mobile, "Voce nao enxerga o alvo.");
			return;
		endif
		if (spellinfo.targtype == "Animados")
			foreach mob in ListMobilesNearLocation(targ.x, targ.y, targ.z, 8, mobile.realm)
				if (mob.serial != mobile.serial)
					alvos.append(mob);
				endif
			endforeach
		else
			foreach mob in ListItemsNearLocation(targ.x, targ.y, targ.z, 8, mobile.realm)
				if (mob.serial != mobile.serial)
					alvos.append(mob);
				endif
			endforeach
		endif
	endif

	if (spellinfo.type == "Fortalecer")
		EncantoFortalecer(mobile, alvos, spellinfo);
	elseif (spellinfo.type == "Destruir")
		EncantoDestruir(mobile, alvos, spellinfo);
	elseif (spellinfo.type == "Curar")
		EncantoCurar(mobile, alvos, spellinfo);
	elseif (spellinfo.type == "Enfraquecer")
		EncantoEnfraquecer(mobile, alvos, spellinfo);
       elseif (spellinfo.type == "Outro")
      endif
endfunction

function EncantoCurar(mobile, alvos, spellinfo)

	var dmg := CalculateSpellDamage(mobile, cint(spellinfo.power));

	if (spellinfo.targtype == "Animados")
		foreach mob in alvos
			foreach att in (spellinfo.dmgtarg)
				if (att == "Vida")
					ApplyEpicHealing(mob, dmg);
				elseif (att == "Alma")
					var dmg2;
					if (dmg > AP_GetVitalMaximumValue(mob, MANA))
						dmg2 := AP_GetVitalMaximumValue(mob, MANA);
					else
						dmg2 := dmg;
					endif
					AP_SetVital(mob, MANA, dmg2);
				elseif (att == "Fadiga")
					var dmg2;
					if (dmg > AP_GetVitalMaximumValue(mob, STAMINA))
						dmg2 := AP_GetVitalMaximumValue(mob, STAMINA);
					else
						dmg2 := dmg;
					endif
					AP_SetVital(mob, STAMINA, dmg2);
				endif
			endforeach
			PlayObjectCenteredEffect(mob, FX_BLUE_SPARKLE_SWIRL, 7, 16);
			PlaySoundEffect(mob, SFX_HEAL);
		endforeach
	else
		foreach mob in alvos
			mob.hits := mob.hits + dmg;
			PrintTextAbove(mob, "*consertado*");
		endforeach
	endif

endfunction

function EncantoDestruir(mobile, alvos, spellinfo)

	var dmgtype := dictionary;

	if (spellinfo.dmgtype == "Fogo")
		dmgtype[DMG_FIRE] := 1.0;
	elseif (spellinfo.dmgtype == "Gelo")
		dmgtype[DMG_COLD] := 1.0;
	elseif (spellinfo.dmgtype == "Relampago")
		dmgtype[DMG_ENERGY] := 1.0;
	endif

	var dmg := CalculateSpellDamage(mobile, cint(spellinfo.power));

	if (spellinfo.targtype == "Animados");
		foreach mob in alvos
			foreach att in (spellinfo.dmgtarg)
				if (att == "Vida")
					//ApplyDamageWra(mob, dmg, dmgtype);
				elseif (att == "Alma")
					AP_ConsumeVital(mob, MANA, dmg);
				elseif (att == "Fadiga")
					AP_ConsumeVital(mob, STAMINA, dmg);
				endif
			endforeach
			if (spellinfo.dmgtype == "Fogo")
				PlayObjectCenteredEffect(mob, FX_EXPLODE_1, 7, 16);
				PlaySoundEffect(mob, SFX_EXPLOSION_11);
			elseif ( spellinfo.dmgtype == "Gelo")
				PlayObjectCenteredEffect(mob, FX_BLUE_SPARKLE_GROUND, 7, 16);
				PlaySoundEffect(mob, SFX_GLASS_BREAK_3);
			elseif ( spellinfo.dmgtype == "Relampago")
				PlayMovingEffectXYZ(mob.x, mob.y, mob.z+15, mob.x-1, mob.y, mob.z+15, FX_ENERGY, 5, 25);
				PlayMovingEffectXYZ(mob.x, mob.y, mob.z+15, mob.x+1, mob.y, mob.z+15, FX_ENERGY, 5, 25);
				PlayMovingEffectXYZ(mob.x, mob.y, mob.z+15, mob.x, mob.y+1, mob.z+15, FX_ENERGY, 5, 25);
				PlayMovingEffectXYZ(mob.x, mob.y, mob.z+15, mob.x, mob.y-1, mob.z+15, FX_ENERGY, 5, 25);
				PlaySoundEffect(mob, SFX_LIGHTNING);
			endif
		endforeach
	endif

endfunction

function EncantoFortalecer(mobile, byref alvos, byref spellinfo)

	if (spellinfo.targtype == "Animados")

		foreach mob in alvos
			foreach att in (spellinfo.atributo)
				if (att == "Forca")
					TS_StartTimer(mob, "strength", CINT(AP_GetSkill(mobile, MAGERY)*12)+180, cint(spellinfo.power)*10, mobile);
				elseif ( att == "Destreza")
					TS_StartTimer(mob, "agility", CINT(AP_GetSkill(mobile, MAGERY)*12)+180, cint(spellinfo.power)*10, mobile);
				elseif ( att == "Inteligencia")
					TS_StartTimer(mob, "int", CINT(AP_GetSkill(mobile, MAGERY)*12)+180, cint(spellinfo.power)*10, mobile);
				elseif ( att == "Resistencia")
					foreach resist in (spellinfo.resistencia)
						if (resist == "Fogo")
							TS_StartTimer(mob, "phmod", CINT(AP_GetSkill(mobile, MAGERY)*12)+180, 2, mobile);
						elseif ( resist == "Frio")
							TS_StartTimer(mob, "comod", CINT(AP_GetSkill(mobile, MAGERY)*12)+180, 2, mobile);
						elseif ( resist == "Energia")
							TS_StartTimer(mob, "enmod", CINT(AP_GetSkill(mobile, MAGERY)*12)+180, 2, mobile);
						elseif ( resist == "Fisico")
							TS_StartTimer(mob, "phmod", CINT(AP_GetSkill(mobile, MAGERY)*12)+180, 2, mobile);
						elseif ( resist == "Veneno")
							TS_StartTimer(mob, "pomod", CINT(AP_GetSkill(mobile, MAGERY)*12)+180, 2, mobile);
						endif
					endforeach
				endif
			endforeach
			PlaySoundEffect(mob, SFX_SPELL_BLESS);
			PlayObjectCenteredEffect(mob, FX_BLUE_SPARKLES, 7, 7);
		endforeach

	else

		foreach mob in alvos
			if ( spellinfo.inanimado == "Armas")
				if (mob.isa(POLCLASS_WEAPON ))
					SetObjProperty(mob, "armaafiada", (CINT(AP_GetSkill(mobile, MAGERY)/10)+1) );
					PlaySoundEffect(mob, SFX_SPELL_BLESS);
					PlayStationaryEffect(mob.x, mob.y, mob.z, FX_BLUE_SPARKLES, 7, 7);
				endif
			elseif (spellinfo.inanimado == "Curativos")
				if (mob.objtype  == 0xe21 )
					var amt := mob.amount;
					if (amt > 10)
						amt := 10;
					endif
					if (SubtractAmount(mob, 10))
						CreateItemInBackpack(mobile, 0xfdde, amt);
						PlaySoundEffect(mob, SFX_SPELL_BLESS);
					endif
				endif
			elseif (spellinfo.inanimado == "Comida")
				if (mob.usescript == "eat")
					SetObjProperty(mob, "blessedfood", 1);
					PlaySoundEffect(mob, SFX_SPELL_BLESS);
				endif
			endif
		endforeach

	endif

endfunction

function EncantoEnfraquecer(mobile, byref alvos, byref spellinfo)
	foreach mob in alvos
		foreach att in (spellinfo.atributo)
			if (att == "Forca")
				TS_StartTimer(mob, "weaken", CINT(AP_GetSkill(mobile, MAGERY)*12)+180, cint(spellinfo.power)*10, mobile);
			elseif ( att == "Destreza")
				TS_StartTimer(mob, "clumsy", CINT(AP_GetSkill(mobile, MAGERY)*12)+180, cint(spellinfo.power)*10, mobile);
			elseif ( att == "Inteligencia")
				TS_StartTimer(mob, "feeblemind", CINT(AP_GetSkill(mobile, MAGERY)*12)+180, cint(spellinfo.power)*10, mobile);
			elseif ( att == "Resistencia")
				foreach resist in (spellinfo.resistencia)
					if (resist == "Fogo")
						TS_StartTimer(mob, "phmod", CINT(AP_GetSkill(mobile, MAGERY)*12)+180, 1, mobile);
					elseif ( resist == "Frio")
						TS_StartTimer(mob, "comod", CINT(AP_GetSkill(mobile, MAGERY)*12)+180, 1, mobile);
					elseif ( resist == "Energia")
						TS_StartTimer(mob, "enmod", CINT(AP_GetSkill(mobile, MAGERY)*12)+180, 1, mobile);
					elseif ( resist == "Fisico")
						TS_StartTimer(mob, "phmod", CINT(AP_GetSkill(mobile, MAGERY)*12)+180, 1, mobile);
					elseif ( resist == "Veneno")
						TS_StartTimer(mob, "pomod", CINT(AP_GetSkill(mobile, MAGERY)*12)+180, 1, mobile);
					endif
				endforeach
			endif
		endforeach
		PlaySoundEffect(mob, SFX_WEAKEN);
		PlayObjectCenteredEffect(mob, FX_RED_SPARKLES, 7, 7);
	endforeach

endfunction

function CalculateSpellDamage(mobile, power)
	var magery := CINT(AP_GetSkill(mobile, MAGERY));
	magery := magery/3;

	var dmg := magery;

	if (power == 1)
		dmg := dmg + RandomDiceRoll("1d6") + 10;
	elseif ( power == 2)
		dmg := dmg + RandomDiceRoll("1d6") + 20;
	elseif ( power == 3 )
		dmg := dmg + RandomDiceRoll("2d6") + 30;
	elseif ( power == 4 )
		dmg := dmg + RandomDiceRoll("3d6") + 40;
	elseif (power == 5 )
		dmg := dmg + RandomDiceRoll("4d6") + 50;
	endif

	return dmg;

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


