use uo;
use math;

include "include/client";
include "include/sounds";
include ":attributes:attributes";
include "include/damage";
include ":timedScripts:timedScripts";
include "spellSounds";
include ":charactercreation:resists";
include ":charactercreation:habilidades";

function spelllearn(who)

	var spell := GetObjProperty(who, "spellcraft");

	if (spell == error)
		SendSysMessage(who, "Voce nao esta pesquisando nenhuma magia para aprende-la agora.");
		return;
	endif

	var hoje := Cint(GetGlobalProperty("daynumber"));
	var ano := GetGlobalProperty("current_year");
	if (spell.year == ano)
		if (  (hoje - spell.start) >= spell.end  )
			SendSysMessage(who, "Selecione seu grimorio para escrever o novo encantamento.");
			CopyToSpellbook(who);
		else
			SendSysMessage(who, "Ainda faltam " + (spell.end - (hoje - spell.start)) + " dias para aprender esse encantamento.");
			return;
		endif
	else
		var diaspassados := (365 - spell.start) + hoje;
		if (diaspassados >= spell.end)
			SendSysMessage(who, "Selecione seu grimorio para escrever o novo encantamento.");
			CopyToSpellbook(who);
		else
			SendSysMessage(who, "Ainda faltam " + (spell.end - diaspassados) + " dias para aprender esse encantamento.");
			return;
		endif
	endif


endfunction

function CopyToSpellbook(who)

	var targ := Target(who);

	if (!targ)
		SendSysMessage(who, "Cancelado.");
		return;
	endif

	if (targ.objtype != 0x6100)
		SendSysMessage(who, "Isto nao e um grimorio.");
		return;
	endif

	var spells := GetObjProperty(targ, "spells");

	if (!spells)
		spells := array;
	endif

	spells.append( GetObjProperty(who, "spellcraft") );
	setobjproperty(targ, "spells", spells);

	EraseObjProperty(who, "spellcraft");

	SendSysMessage(who, "Voce escreveu o encantamento no seu grimorio.");


endfunction

function spellcraft(who)

	var aprendendo := GetObjProperty(who, "spellcraft");
	if (aprendendo != error)
		var cancelar := YesNo(who, "Voce ainda esta pesquisando um encantamento. Deseja cancelar e pesquisar outro?", "Sim", "Nao");
		if (!cancelar)
			SendSysMessage(who, "Cancelado.");
			return;
		else
			EraseObjProperty(who, "spellcraft");
		endif
	endif

	var spellinfo := struct;
	var spellname := RequestGump(who, "Digite o nome do encantamento:", "", "", 1);
	if (spellname == error)
		SendSysMessage(who, "Cancelado.");
		return;
	endif

	spellinfo.+name := spellname;

	var powerwords := RequestGump(who, "Digite o compoente verbal do encantamento:", "", "", 1);
	if (powerwords == error)
		SendSysMessage(who, "Cancelado.");
		return;
	endif

	spellinfo.+powerwords := powerwords;

	WriteSpellDescription(who, spellinfo);

	//escolhe o tipo: enfraquecer,  fortalecer,  destruir, curar, invocar, controlar
	var type := RadioGump(who, 400, 380, "Escolha o tipo de encantamento.", {"Enfraquecer", "Fortalecer", "Destruir", "Curar"/*,  "Controlar", "Entender"*/});
	spellinfo.+type := type;

	var alvo := RadioGump(who, 400, 205, "Escolha se o encantamento tera efeito sobre um unico alvo ou todos ao redor do feiticeiro.", {"Unico", "Area"});
	if (alvo == "Unico")
		var range := RadioGump(who, 400, 205, "Escolha o alcance do encantamento.", {"Distancia", "Toque"});
		spellinfo.+range := range;
	endif
	var targtype := "Animados";
	spellinfo.+targ := alvo;
	spellinfo.+targtype := "Animados";

	if (  type == "Fortalecer")
		targtype := RadioGump(who, 400, 205, "Escolha se o alvo do encantamento sao seres vivos ou inanimados.", {"Animados", "Inanimados"});
		spellinfo.targtype := targtype;
	endif
	if (targtype == "Inanimados")
		spellinfo.+inanimado := RadioGump(who, 400, 205, "Escolha que tipo de objeto a magia alveja.", {"Armas", "Armaduras", "Comida", "Curativos", "Fechaduras"});
	endif


	//se for enfraquecer ou fortalecer escolhe o atributo que vai ser atingido
	if (spellinfo.type == "Enfraquecer" || spellinfo.type == "Fortalecer")
		if (targtype == "Animados" )
			spellinfo.+atributo := CheckBoxGump(who, 400, 305, "Que caracteristica voce quer " + spellinfo.type +" . Pode escolher mais de uma.", {"Forca", "Destreza", "Inteligencia",  "Resistencia"});
			if ("Resistencia" in spellinfo.atributo)
				spellinfo.+resistencia := CheckBoxGump(who, 400, 305, "Que resistencia voce quer " + spellinfo.type + ". Pode escolher mais de uma.", {"Fisica", "Fogo", "Frio", "Veneno", "Energia"});
			endif
		endif
	elseif ( spellinfo.type == "Destruir" )
		if (targtype == "Animados")
			spellinfo.+dmgtarg := CheckBoxGump(who, 400, 205, "Que caracteristica voce quer "+spellinfo.type+". Pode escolher mais de uma.", {"Vida", "Alma", "Fadiga"});
		elseif (targtype == "Inanimados")
			spellinfo.+dmgtarg := "Vida";
		endif
		spellinfo.+dmgtype := RadioGump(who, 400, 305, "Com o que voce deseja destruir o alvo.", {"Fogo", "Relampago", "Gelo"});
	elseif ( spellinfo.type == "Curar")
		if (targtype == "Animados")
			spellinfo.+dmgtarg := CheckBoxGump(who, 400, 305, "Que caracteristica voce quer " + spellinfo.type +" . Pode escolher mais de uma.", {"Vida", "Alma", "Fadiga"});
		elseif (targtype == "Inanimados")
			spellinfo.+dmgtarg := "Vida";
		endif
	elseif ( spellinfo.type == "Controlar" )
		if (targtype == "Animados")
			spellinfo.+atributo := RadioGump(who, 400, 205, "Que caracteristica voce quer "+spellinfo.type+". ", {"Movimento", "Forma", "Cor"});
		elseif (targtype == "Inanimado")
			spellinfo.+atributo := RadioGump(who, 400, 205, "Que caracteristica voce quer "+spellinfo.type+". ", {"Movimento", "Forma", "Abrir/Fechar"});
		endif
	elseif ( spellinfo.type == "Entender")
	endif

	spellinfo.+power := RadioGump(who, 400, 305, "Defina o quao forte sera o encantamento. ", {"1", "2", "3", "4", "5"});
	CalculateResearchTime(spellinfo);
	spellinfo.+mana := Cint(spellinfo.power)*10;
	spellinfo.+delay := Cint(spellinfo.power)*300;
	if (Cint(spellinfo.power) == 1)
		spellinfo.+difficulty := 5;
	elseif (Cint(spellinfo.power) == 2 )
		spellinfo.+difficulty := 10;
	elseif (Cint(spellinfo.power) == 3 )
		spellinfo.+difficulty := 30;
	elseif (Cint(spellinfo.power) == 4 )
		spellinfo.+difficulty := 50;
	elseif (Cint(spellinfo.power) == 5 )
		spellinfo.+difficulty := 70;
	endif

	var aprender := YesNo(who, "Voce deseja pesquisar este encantamento? Ele vai requerer " + spellinfo.end + " dias de estudo. ", "Sim", "Nao");
	if (aprender)
		spellinfo.+start := GetGlobalProperty("daynumber");
		spellinfo.+year := GetGlobalProperty("current_year");
		if(who.cmdlevel > 1)
			var gm := YesNo(who, "Voce eh gm, deseja aprender essa magia automaticamente?", "Sim", "Nao");
			if (gm)
				spellinfo.end := 0;
			endif
		endif
		SetObjProperty(who, "spellcraft", spellinfo);
	else
		SendSysMessage(who, "Cancelado.");
		return;
	endif
endfunction

function CalculateResearchTime(byref spellinfo)

	var basictime := 0;

	if (spellinfo.targtype == "Animados")
		basictime := basictime + 4;
	else
		basictime := basictime + 2;
	endif

	if (spellinfo.range == "Distancia")
		basictime := basictime + 2;
	endif

	if (spellinfo.targ == "Area")
		basictime := basictime + 4;
	endif

	if (spellinfo.type == "Enfraquecer" || spellinfo.type == "Fortalecer" )
		if (spellinfo.targtype == "Animados")
			basictime := basictime + ((spellinfo.atributo.size())*5);
			if ( "Resistencia" in spellinfo.atributo)
				basictime := basictime + ((spellinfo.resistencia.size())*5);
			endif
		else
			basictime := basictime + 1;
		endif
	elseif (spellinfo.type == "Destruir")
		if ( spellinfo.targtype == "Animados")
			basictime := basictime + ((spellinfo.dmgtarg.size())*5);
		else
			basictime := basictime + 1;
		endif
	elseif (spellinfo.type == "Curar")
		if (spellinfo.targtype := "Animados")
			basictime := basictime + ((spellinfo.dmgtarg.size())*5);
		else
			basictime := basictime + 1;
		endif
	elseif (spellinfo.type == "Entender");
		basictime := basictime -2;
	else
		basictime := basictime + 1;
	endif

	spellinfo.+end := (basictime*cint(spellinfo.power))*2;

endfunction

function CastSpell(mobile, spellinfo, targ)
	//sendsysmessage(mobile, " " + spellinfo.script);
	//se for um spell com script (ou seja o novo sistema) executa o script e retorna, caso o contrario executa o de magias custom
    if (GetCooldown(targ, "spellthief"))
        targ := mobile;
    endif
        
	if (spellinfo.script != error)
		var e := Start_Script(spellinfo.script, {mobile, targ, spellinfo.id});
		//sendsysmessage(mobile, " " );
		return;
	else
		SendSysMessage(mobile, " " + spellinfo);
	endif

	var alvos := array;

	if (spellinfo.targ == "Unico")
		SendSysMessage(mobile, "Escolha o alvo.");
		var targ := Target(mobile, TGTOPT_CHECK_LOS);
		if (!targ )
			SendSysMessage(mobile, "Cancelado.");
			return;
		endif
		if (spellinfo.range == "Toque")
			if (Distance(mobile, targ) > 1)
				SendSysMessage(mobile, "O alvo esta muito distante e este encantamento precisa que voce o toque.");
				return;
			endif
		else
			if (Distance(mobile, targ) > 8 )
				SendSysMessage(mobile, "O alvo esta fora do alcance do encantamento.");
				return;
			endif
		endif

		if ( (spellinfo.targtype == "Animados") && (!targ.isa(POLCLASS_MOBILE)) )
			SendSysMessage(mobile, "Este encanto so afeta seres animados.");
			return;
		elseif ( (spellinfo.targtype == "Inanimados") && (targ.isa(POLCLASS_MOBILE)) )
			SendSysMessage(mobile, "Este encanto so afeta seres inanimados.");
			return;
		endif
		alvos.append(targ);
	else
		SendSysMessage(mobile, "Onde voce deseja invocar o encantamento?");
		var targ := TargetCoordinates(mobile);
		if (!CheckLosAt(mobile, targ.x, targ.y, targ.z))
			SendSysMessage(mobile, "Voce nao enxerga o alvo.");
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
         //if (spellinfo.name == "Teia")
         //  EncantoTeia(mobile, alvos, spellinfo);
         //endif
      endif


endfunction

function EncantoCurar(mobile, alvos, spellinfo)

	var dmg := CalculateSpellDamage(mobile, cint(spellinfo.power));

	if (spellinfo.targtype == "Animados")
		foreach mob in alvos
			foreach att in (spellinfo.dmgtarg)
				if (att == "Vida")
					HealDamage(mob, dmg);
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
			PlayObjectCenteredEffect(mob, GFX_BLUE_SPARKLE_SWIRL, 7, 16);
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
				PlayObjectCenteredEffect(mob, GFX_EXPLODE_1, 7, 16);
				PlaySoundEffect(mob, SFX_EXPLOSION_11);
			elseif ( spellinfo.dmgtype == "Gelo")
				PlayObjectCenteredEffect(mob, GFX_BLUE_SPARKLE_GROUND, 7, 16);
				PlaySoundEffect(mob, SFX_GLASS_BREAK_3);
			elseif ( spellinfo.dmgtype == "Relampago")
				PlayMovingEffectXYZ(mob.x, mob.y, mob.z+15, mob.x-1, mob.y, mob.z+15, GFX_ENERGY, 5, 25);
				PlayMovingEffectXYZ(mob.x, mob.y, mob.z+15, mob.x+1, mob.y, mob.z+15, GFX_ENERGY, 5, 25);
				PlayMovingEffectXYZ(mob.x, mob.y, mob.z+15, mob.x, mob.y+1, mob.z+15, GFX_ENERGY, 5, 25);
				PlayMovingEffectXYZ(mob.x, mob.y, mob.z+15, mob.x, mob.y-1, mob.z+15, GFX_ENERGY, 5, 25);
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
			PlayObjectCenteredEffect(mob, GFX_BLUE_SPARKLES, 7, 7);
		endforeach

	else

		foreach mob in alvos
			if ( spellinfo.inanimado == "Armas")
				if (mob.isa(POLCLASS_WEAPON ))
					SetObjProperty(mob, "armaafiada", (CINT(AP_GetSkill(mobile, MAGERY)/10)+1) );
					PlaySoundEffect(mob, SFX_SPELL_BLESS);
					PlayStationaryEffect(mob.x, mob.y, mob.z, GFX_BLUE_SPARKLES, 7, 7);
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
		PlayObjectCenteredEffect(mob, GFX_RED_SPARKLES, 7, 7);
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
		var cfg;
		cfg :=  ReadConfigFile(":spells:config/allspells"); //ReadConfigFile(GetBookScripts(scroll));
		//if (scroll == "mage")
		//	cfg := ReadConfigFile(":spells:config/tnspells");
		//elseif (scroll == "necro")
		//	cfg := ReadConfigFile(":spells:config/necrospells");
		//elseif (scroll == "swordmage")
		//	cfg := ReadConfigFile(":spells:config/swordmagespells");
		//endif


		if (!cfg)
			SendSysMessageEx(who, "spell.cfg nao encontrado", SSM_FAIL);
			return 0;
		endif
		var elem := FindConfigElem(cfg, spellinfo.id);
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

function WritespellInfo(who, byref spellinfo)
	var cfg;
	var myrace := Lower(GetObjProperty(who, "raca"));

    if (GetObjProperty(who, "chardata").magia == "Shaman")
        cfg := ReadConfigFile(":spells:config/shamanspells");
        spellinfo.used := 1;
	elseif ((myrace == "elfo" || myrace == "drow") && spellinfo.magic_path == "cantripspells")
        cfg := ReadConfigFile(":spells:config/cantripspells");
	elseif(spellinfo.magic_path == "Sangria")
        cfg := ReadConfigFile(":spells:config/sangriaspells");
	elseif (GetObjProperty(who, "chardata").magia == "spellcaster" && spellinfo.magic_path == "spellcaster")
        cfg := ReadConfigFile(":spells:config/spellcaster");
	elseif(spellinfo.magic_path == "Trickster" || TemHabilidade(who, "Sensibilidade Arcana"))
        cfg := ReadConfigFile(":spells:config/tricksterspells");
	else
		cfg := ReadConfigFile(":spells:config/allspells");
    endif

	//sendsysmessage(who, "cfg" + cfg + " " + spellinfo.id);

	if (!cfg)
//		SendSysMessageEx(who, "spell.cfg nao encontrado", SSM_FAIL);
		return;
	endif
	var elem := FindConfigElem(cfg, spellinfo.id);
	//sendsysmessage(who, "elem " + elem);
	if (elem == error)
//		SendSysMessageEx(who, "spellid nao encontrado " + spellinfo.id, SSM_FAIL);
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
	spellinfo.+align := GetConfigStringDictionary(elem, "Align");
	spellinfo.+flavortext := GetConfigString( elem, "FlavorText" );
	spellinfo.+cycles := GetConfigInt( elem, "Cycles" );
	spellinfo.+corruption := GetConfigInt( elem, "Corruption" );
	if(spellinfo.spelltype == "spellcaster")
		spellinfo.+School := GetConfigString( elem, "School" );
	endif
	if (spellinfo.flavortext == error)
		spellinfo.flavortext := "";
	else
		spellinfo.flavortext := "<I> "  + spellinfo.flavortext + " </I>";
	endif

	//sendsysmessage(who, " " + spellinfo.script);

endfunction

function WriteSpelldescription(who, byref spellinfo)

	var gump := GFCreateGump();
	GFResizePic(gump, 15, 50, 2620, 310, 380);

	var num_lines := 19;
	var msg_array := array;
	var y_pos := 55;
	var i;
	for ( i:=1; i<=num_lines; i:=i+1 )
		msg_array.Append(GFTextEntry(gump, 20, y_pos, 595, 20, 2100, ""));
		y_pos := y_pos+19;
	endfor

	GFResizePic(gump, 15, 20, GFCfgConst("Defaults", "ForeGround"), 310, 30);
	GFResizePic(gump, 15, 430, GFCfgConst("Defaults", "ForeGround"), 310, 30);

	GFAddButton(gump, 285, 440, 4011, 4030, GF_CLOSE_BTN, 20);
	GFTextLine(gump, 225, 440, 2100, "Proximo");
	GFTextLine(gump, 35, 20, 2100, "Descreva o encantamento:");

	var input := GFSendGump(who, gump);

	if (input[0] == 20)
		var i;
		for ( i:=1; i<=msg_array.Size(); i:=i+1 )
			var line := GFExtractData(input, msg_array[i]);
			if ( line )
				line := line + " ";
				msg_array[i] := line;
			else
				msg_array[i] := "";
			endif
		endfor

		//Clear off empty lines.
		for ( i:=msg_array.Size(); i>=1; i:=i-1 )
			if ( Len(msg_array[i]) <= 0 )
				msg_array.Erase(i);
			endif
			SleepMS(2);
		endfor

		var text := "";
		for ( i:=1; i<msg_array.Size(); i:=i+1 )
			text := text + msg_array[i];
			SleepMS(2);
		endfor

		spellinfo.+description := text;
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

function MantainSpell(byref info)

	//sendsysmessage(info.mage, " " + info);


	if (info.mage.connected)
		start_script( ":spells:mantainSpell", { info.mage, info.mobserial, info.name, info.duration, info.level  }  );
	endif

endfunction

//////////////////////////////////////////////////////////////
//////// Corromper(jogador, alvo, mana, circulo) - By Duttones
////////
////////
////////
//////////////////////////////////////////////////////////////
function Corromper(who, spell, targ)
	if (GetObjProperty(who, "corruption") == 20000) //se o jogador tiver atingido o maximo de corrupcao, ele nem inicia o script
		return 0;
	endif
	//escreve o spellinfo
	var cfg := ReadConfigFile(":spells:config/allspells");
	var elem := FindConfigElem(cfg, spell);
	var spellinfo := dictionary;
	spellinfo.+name := GetConfigString( elem, "Name" );
	spellinfo.+mana := GetConfigInt( elem, "Mana" );
	spellinfo.+cycles := GetConfigInt( elem, "Circle" );
	spellinfo.+corruption := GetConfigInt( elem, "Corruption" );
	var variavel; var corrupcao;

	//testa resistencia de vontade
	if (temHabilidade(who, "Pacto com Shimoria"))
		if (CheckResist(who, VONTADE, 20*spellinfo.cycles))
		return 0;
		endif
	elseif (CheckResist(who, VONTADE, 30*spellinfo.cycles))
		return 0;
	endif

	SendSysMessageEx(who, "Voce falhou em resistir a corrupcao e Asan se apoderou mais um pouco de sua alma.", SSM_FAIL);
	//fim de checagem

	//checa se e magia banida e define a variavel
	if (spellinfo.corruption != 1)
		variavel := spellinfo.corruption/10;
	endif

	//checa se é player
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
	//fim de checagem

	//calculo de corrupcao
	corrupcao := (spellinfo.mana) /5 * spellinfo.cycles;
	variavel := corrupcao * variavel;
	corrupcao := Cint(corrupcao + variavel);

	if (temHabilidade(who, "Alma Forte"))
		corrupcao := corrupcao /2;
	endif
	//fim do calculo de corrupcao

	//adiciona corrupcao no personagem
	if (GetObjProperty(who, "corruption"))
		corrupcao := corrupcao + GetObjProperty(who, "corruption");
	endif

	if (corrupcao > 20000)
		corrupcao := 20000;
	endif

	SetObjProperty(who, "corruption", corrupcao);
	//fim
	if (GetObjProperty(who, "corruptionalert") == error)
		SetObjProperty(who, "corruptionalert", 0);
	endif

	//checa se o nivel de corrupcao aumentou, avisa o jogador e adiciona os efeitos
	if ((corrupcao >= 5000) && (corrupcao < 9999) )
		if (GetObjProperty(who, "corruptionalert") < 1)
			SendSysMessageEx(who, "Sua corrupcao agora aumentou de 'muito baixa' para 'baixa'.", SSM_FAIL);
			SetObjProperty(who, "corruptionalert", 1);
		endif
	elseif ((corrupcao >= 10000) && (corrupcao < 14999) )
		if (GetObjProperty(who, "corruptionalert") < 2)
			SendSysMessageEx(who, "Sua corrupcao agora aumentou de 'baixa' para 'razoavel'.", SSM_FAIL);
			//SendSysMessageEx(who, "Voce comeca a sentir os efeitos da insanidade.", SSM_INFO);
			SetObjProperty(who, "corruptionalert", 2);
		endif
	elseif ((corrupcao >= 15000) && (corrupcao < 19999) )
		if (GetObjProperty(who, "corruptionalert") < 3)
			SendSysMessageEx(who, "Sua corrupcao agora aumentou de 'razoavel' para 'alta'.", SSM_FAIL);
			//SendSysMessageEx(who, "Voce esta ficando insano.", SSM_INFO);
			SetObjProperty(who, "corruptionalert", 3);
/*
			var originalcorruptionbody := struct;
			var mybeard := GetEquipmentByLayer(who, LAYER_BEARD);
			var myhair := GetEquipmentByLayer(who, LAYER_HAIR);
			originalcorruptionbody.+corcabelo := myhair.color;
			originalcorruptionbody.+corbarba := mybeard.color;
			originalcorruptionbody.+corpele := who.color;
			SetObjProperty(who, "originalcorruptionbody", originalcorruptionbody);

			case ( RandomIntMinMax(1,3) )
				1:
					who.color := 1873;
					SendSysMessageEx(who, "Sua pele ficou mais palida!!", SSM_FAIL);
				2:
					mybeard.color := 0;
					myhair.color := 0;
					SendSysMessageEx(who, "Seus cabelos perderam a cor!!", SSM_FAIL);
			endcase
*/
		endif
	elseif (corrupcao >= 20000)
		if (GetObjProperty(who, "corruptionalert") < 4)
			SendSysMessageEx(who, "Sua corrupcao agora aumentou de 'alta' para 'muito alta'.", SSM_FAIL);
			//SendSysMessageEx(who, "A insanidade agora faz parte do seu dia-a-dia.", SSM_INFO);
			SetObjProperty(who, "corruptionalert", 4);
/*
			case ( RandomIntMinMax(1,3) )
				1:
					SetObjProperty(who, "BloodColor", 72);
					SendSysMessageEx(who, "Seu sangue agora é verde!!", SSM_FAIL);
				2:
					var str := "Veias visiveis sob a pele";
					if (GetObjProperty(who, "description"))
						str := GetObjProperty(who, "description") + " <br> " + str;
					endif
					SetObjProperty(who, "description", str);
					SendSysMessageEx(who, "As veias por baixo de sua pele parece estar mais escuras!!", SSM_FAIL);
			endcase
*/
		endif
	endif
	//fim da checagem

endfunction


