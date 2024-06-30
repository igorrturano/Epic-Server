include "include/sysEvent";
include ":brainAI:eventid";
include ":taming:taming";
include "include/say";

function ghaiaRessurect(targ, notame := 0)
   var template := GetObjProperty(targ, "npctemplate");
   var npctemplate := NPC_GetNPCConfig(template);
	var npc := CreateNpcFromTemplate( template, targ.x, targ.y, targ.z, 0, targ.realm);
	RestartScript(npc);
	var ownerserial := GetObjProperty(targ, "owner");
	var owner := SystemFindObjectBySerial(ownerserial, SYSFIND_SEARCH_OFFLINE_MOBILES);
	if (!owner)
		destama(npc.serial);
	else
      if (npctemplate.TamingCombat && GetObjProperty(owner, "combat_pet"))
         SendSysMessageEx(owner, "Você já possui um animal combatente. A criatura perde a lealdade a você.", SSM_FAIL);
         destama(npc.serial);
      else
         AddPet(owner, npc.serial, 1);
         SetObjProperty(owner, "combat_pet", npc.serial);
      endif
	endif

   if ( !npc.backpack )
      var new_pack := CreateItemAtLocation(npc.x, npc.y, npc.z, "backpack", 1);
      EquipItem(npc, new_pack);
   endif

   CopyProps(targ, npc);
   CopyAttributes(targ, npc);
   RestartScript(npc);
   SetName(npc, GetObjProperty(npc, "originalname"));

   Detach();
   var serialC := cint(GetObjProperty(targ, "serial"));
	foreach item in EnumerateItemsInContainer(targ)
		if (item.container == targ)
			MoveObjectToLocation(item, npc.x, npc.y, npc.z, npc.realm);
			MoveItemToContainer(item, npc.backpack);
			if ( CInt(GetObjProperty(item, "EquippedOn")) == serialC )
				if ( !EquipItem(npc, item) )
					EraseObjProperty(item, "EquippedOn");
				endif
			endif
		endif
	endforeach

   npc.color := targ.color;
   npc.truecolor := targ.color;
   npc.graphic := GetObjProperty(npc, "graphic");

	DestroyItem(targ);
	var skills := GetObjProperty(npc, "petskills");
	foreach skill in (skills.keys())
		AP_SetTrueSkill(npc, skill, skills[skill]);
	endforeach

   EraseObjProperty(npc, "GuardKill");
   EraseObjProperty(npc, "morto");
endfunction

function ghaiaAttack(who, npc)
	var event 		:= 	struct;
	event.type		:=	EVID_TAUNT;
	event.source	:=	who;

   npc.setAggro(who.serial, 150);
	SendEvent(npc, event);
endfunction

function ghaiaPeaceMade(who, npc)
	var ev 		:= struct;
	ev.type 	:= EVID_PEACEMADE;
	ev.source 	:= who;

	sendEvent( npc, ev);
endfunction

function ghaiaFlee(who, npc)
	var ev 		:= struct;
	ev.type 	:= EVID_FLEE;
	ev.source 	:= who;

	sendEvent( npc, ev);
endfunction

function ghaiaCheckNumber(npc)
	sendEvent( npc, struct{ type := EVID_CHECK_NUMBER });
endfunction

function CopyProps(origin, destiny)
	var propnames := GetObjPropertyNames(origin);
	foreach prop in propnames
		SetObjPRoperty(destiny, prop, GetObjProperty(origin, prop));
	endforeach
endfunction

function CopyAttributes(origin, destiny)
    var stats := GetObjProperty(origin, "stats");
    AP_SetTrueStat(destiny, "Strength", CInt(stats.str));
    AP_SetTrueStat(destiny, "Dexterity", CInt(stats.dex));
    AP_SetTrueStat(destiny, "Intelligence", CInt(stats.int));
    AP_SetTrueStat(destiny, "Wisdom", CInt(stats.wis));
    AP_SetTrueStat(destiny, "Constitution", CInt(stats.con));
endfunction

function ghaiaObjective(npc, destiny, move_type, can_stop := 0)
	var ev      := struct;
	ev.type     := EVID_DOOBJECTIVE;
	ev.destiny  := destiny;
	ev.move_type:= move_type;
	ev.can_stop := can_stop;
	// ev.move_direction:=

	sendEvent( npc, ev);
endfunction

/*
 * NPC_GetNPCConfig(template)
 *
 * Purpose
 * Retrieves a config elem for an npc template.
 *
 * Parameters
 * info:	Template info from NPC_ParseTemplateName()
 *
 * Return value
 * Config elem reference
 *
 */

 /*
function NPC_GetNPCConfig(template, ignore_metamorfose := 0)
	if (template.isA(POLCLASS_MOBILE))
      if (!ignore_metamorfose && GetObjProperty(template, "metamorfose"))
		   template := struct{ "package" := template.package(), "my_template" := GetObjProperty(template, "metamorfose")};
      else
		   template := struct{ "package" := template.package(), "my_template" := template.my_template()};
      endif
	else
		var splited := SplitWords(template, ":");
		template := struct{ "package" := splited[2], "my_template" := splited[3]};
	endif

	var npc_cfg := ReadConfigFile(":"+template.package+":npcdesc");
	if ( npc_cfg.errortext )
		return error{"errortext":="Error::NPC_GetNPCConfig() - Could not open :"+template.package+":npcdesc ->"+npc_cfg.errortext};
	endif

	var cfg_elem := npc_cfg[template.my_template];
	if ( cfg_elem.errortext )
		return error{"errortext":="Error::NPC_GetNPCConfig() - Could not find elem ["+template.my_template+"] ->"+cfg_elem.errortext};
	endif

	return cfg_elem;
endfunction
*/

function NPC_Checktemplate(template)
   if (!template["ghaia"]) template := ":ghaia:{}".format(template); endif
   if (!NPC_GetNPCConfig(template)) return 0; endif
   return 1;
endfunction
/*
 * NPC_GetAllTemplateNames()
 *
 * Purpose
 * Retrieves a list of all npc templates in the shard's npcdesc.cfg files
 *
 * Parameters
 *
 * Return value
 * Returns an array
 *
 */

 /*
function NPC_GetAllTemplateNames()
	var elem_names := array{};
	elem_names := elem_names + GetConfigStringKeys(ReadConfigFile("::npcdesc"));
	foreach package in (Packages())
		if ( package.npcdesc )
			var pkg_name := ":"+package.name+":";
			foreach template_name in (GetConfigStringKeys(ReadConfigFile(pkg_name+"npcdesc")))
				elem_names.Append(pkg_name+template_name);
				sleepms(2);
			endforeach
		endif
		sleepms(2);
	endforeach

	return elem_names;
endfunction
*/

/*
 * NPC_DefaultSettings(settings:=0)
 *
 * Purpose
 * Checks the settings dictionary to make sure required values are set.
 * If a dictionary is passed, it will check it for required parameters.
 * If they are not present, they will be set.
 *
 * Parameters
 * settings:	Optional parameter
 *		Dictionary read in from NPC_GetDefaultSettings()
 *
 * Return value
 * Returns a dictionary
 *
 */
function NPC_GetDefaultSettings(settings:=0)
	if ( Lower(TypeOf(settings)) != "dictionary" )
		settings := dictionary;
	endif

	if ( CInt(settings.exists("CycleWait")) <= 0 )
		settings["CycleWait"] := 10;
	endif

	if ( !settings.Exists("NoSleep") )
		if ( CInt(settings.exists("SleepWait")) <= 0 )
			settings["SleepWait"] := 1024;
		endif
		if ( CInt(settings.exists("IdleTicks")) <= 0 )
			settings["IdleTicks"] := 5;
		endif
	endif

	return settings;
endfunction

function AI_Speak(npc, text, text_type:=SPEAK_TEXTTYPE_DEFAULT, do_event:=SPEAK_DOEVENT_DISABLE)
	var ev      := struct;
	ev.+type     := NPCEVENT_SPEAK;
	ev.+text     := text;
	ev.+texttype := text_type;
	ev.+doevent  := do_event;
	ev.+source   := npc;
	ev.+process  := GetProcess(GetPid());

	return SendEvent(npc, ev);
endfunction

function GetNpcAttacks(npctemplate)
	npctemplate := NPC_GetNPCConfig(npctemplate);
	var attacks := GetConfigStringArray(npctemplate, "Attack");
	if (!attacks || !attacks[1])
		return array{};
	endif

	var result := array{};
	foreach attack in attacks
		attack := SplitWords(attack, " ");

		var attack_s := struct{
			"name"    := attack[1],
			"dicedmg"    := attack[2],
			"DamageType" := attack[3],
			"AttackAnim" := cint(attack[4]),
         "AttackDamage" := cint(npctemplate.AttackDamage),
			"MaxRange"   := npctemplate.MaxRange,
			"MinRange"   := npctemplate.MinRange
		};
		result.append(attack_s);
		sleepms(2);
	endforeach

	return result;
endfunction

function GetNpcRandomAttack(mobile)
	var attacks := GetObjProperty(mobile, "customAttacks");
	if (!attacks)
		attacks := GetNpcAttacks(mobile.npctemplate);
	endif

	if (!attacks || !attacks[1])
		return 0;
	endif

	var result := attacks.randomEntry();

	return result;
endfunction

function getHighestAggro(creature)
   var aggros := creature.aggroProp();
   var highest := array{0, -9999999};
   foreach serial in (aggros.keys())
      if (aggros[serial] >= highest[2])
         highest := array{serial, aggros[serial]};
      endif
      sleepms(2);
   endforeach

   return highest[1];
endfunction