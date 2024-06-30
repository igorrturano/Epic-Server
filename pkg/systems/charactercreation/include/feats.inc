include ":attributes:attributes";
include "include/colormap";
include "include/utils";

const RETURN_ACTIVE_ONLY := 1;

function getFeatById(feat_id, get_class := 0)
   var featCfg := ReadConfigFile(":charactercreation:config/feats");
   var elem := FindConfigElem(featCfg, feat_id);

   if (get_class)
      var classCfg := ReadConfigFile(":charactercreation:config/classes");
      var class_elem := FindConfigElem(classCfg, elem.Class);
      if (class_elem.Type == "Main")
         get_class := "classes";
      else
         get_class := "subclasses";
      endif
   endif

   return struct{
      FeatID := GetConfigInt(elem, "FeatID"),
      Name := GetConfigString(elem, "Name"),
      Class := _ifOr(GetConfigString(elem, "Class"), GetConfigString(elem, "God")),
      Description := GetConfigString(elem, "Description"),
      Mana := GetConfigInt(elem, "Mana"),
      God := GetConfigString(elem, "God"),
      Vigor := GetConfigInt(elem, "Vigor"),
      Cooldown := GetConfigInt(elem, "Cooldown"),
      Icon := GetConfigInt(elem, "Icon"),
      CastTime := GetConfigInt(elem, "CastTime"),
      Script := GetConfigString(elem, "Script"),
      Range := GetConfigInt(elem, "Range"),
      Targ := GetConfigString(elem, "Target"),
      TargetAlign := GetConfigString(elem, "TargetAlign"),
      Group := GetConfigInt(elem, "Group"),
      Level := GetConfigInt(elem, "Level"),
      RequireMagicWeapon := GetConfigInt(elem, "RequireMagicWeapon"),
      IgnoreDodge := GetConfigInt(elem, "IgnoreDodge"),
      IgnoreBlock := GetConfigInt(elem, "IgnoreBlock"),
      Experience := GetConfigInt(elem, "Experience"),
      PointCost := GetConfigInt(elem, "PointCost"),
      PowerMod := GetConfigStringArray(elem, "PowerMod"),
      ClassType := get_class
   };
endfunction

function GetClassFeats(class_id, elem := 0)
   if (!elem)
      var classCfg := ReadConfigFile(":charactercreation:config/classes");
      elem := FindConfigElem(classCfg, class_id);
   endif

   if (!elem) return; endif

   var hab_ids := GetConfigStringArray(elem, "Habilidade");

   var feat_list := array{};
   foreach id in hab_ids
      feat_list.append(getFeatById(id));
      sleepms(2);
   endforeach

   return feat_list;
endfunction

function GetRaceFeats(race, return_id := 0)
   var raceCfg := ReadConfigFile(":charactercreation:config/races");
   var elem := FindConfigElem(raceCfg, race);

   if (!elem) return; endif

   var hab_ids := GetConfigStringArray(elem, "Habilidade");
   if (return_id) return hab_ids; endif

   var feat_list := array{};
   foreach id in hab_ids
      feat_list.append(getFeatById(id));
      sleepms(2);
   endforeach

   return feat_list;
endfunction

function GetGodFeats(god, return_id := 0)
   var classCfg := ReadConfigFile(":charactercreation:config/deuses");
   var godelem := FindConfigElem(classCfg, god);
   if (!godelem) return; endif

   var hab_ids := GetConfigStringArray(godelem, "Habilidade");
   if (return_id) return hab_ids; endif

   var feat_list := array{};
   foreach id in hab_ids
      feat_list.append(getFeatById(id));
      sleepms(2);
   endforeach

   return feat_list;
endfunction

function HaveFeatSecondary(who, feat)
	var chardata := GetObjProperty(who, "chardata");
   if (!chardata) return 0; endif

   if (typeOf(feat) != "Integer")
      return 0;
   endif
   
   var feat_list := array{};
   foreach classe in (chardata.subclasses)
      feat_list := feat_list + classe.habs;
      sleepms(2);
   endforeach

	if (feat in feat_list)
		return 1;
	endif

	return 0;
endfunction

function GetFeatList(who, only_active := 0)
	var chardata := GetObjProperty(who, "chardata");
   var feat_list := array{};
   if (!chardata) return array{}; endif
   
   if (chardata.current_class && chardata.classes)
      var classe := FindElemInArray(chardata.classes, struct{ id := chardata.current_class});
      if (classe)
         feat_list := feat_list + classe.selected_habs;
      endif
      
      sleepms(2);
   endif

   foreach classe in (chardata.subclasses)
      feat_list := feat_list + classe.habs;
      sleepms(2);
   endforeach

   feat_list := feat_list + GetRaceFeats(chardata.raca, 1);
   feat_list := feat_list + GetGodFeats(chardata.god, 1);

	if (!feat_list)
		feat_list := array{};
	endif

   var return_list := array{};
   foreach feat in feat_list
      sleepms(2);
      feat := GetFeatByid(feat);
      if ((only_active && !feat.Script) || (chardata.level < feat.Level && !who.cmdlevel))
         continue;
      endif

      return_list.append(feat);
   endforeach
	
	return return_list;
endfunction


function LearnFeat(who, feat, minus_point := 0, gives := 1)
	if (typeof(feat) != "Struct")
		feat := FeatInfo(feat, 1);
	endif

	if (typeof(feat) != "Struct")
		Print(">>>>> ERROR OCURRED, NO FEAT FOUND {}".format(feat));
		return 0;
	endif

	var chardata := GetObjProperty(who, "chardata");
	var feat_list := chardata.Feats;
   if (!feat_list)
      feat_list := array{};
   endif
   
	if (feat.name in feat_list)
		return 0;
	endif

   if (feat.type == "Classe")
      var class_feats := chardata.class_feats;
      if (!class_feats || typeof(class_feats) != "Array" )
         class_feats := array{};
      endif
      class_feats.append(feat.level);
      chardata.class_feats := class_feats;
   endif

	feat_list.append(feat.name);
	chardata.Feats := feat_list;
	SetObjProperty(who, "chardata", chardata);
	
	// if (minus_point)
	// 	who.SetTalentPoints(-1);
	// endif

   if (gives)
      foreach give_parse in (feat.give)
         give_parse := SplitWords(give_parse);
         if (give_parse[1] == "item")
            createiteminBackpack(who, give_parse[2], give_parse[3]);
         elseif (give_parse[1] == "stat")
            AP_SetTrueStat(who, give_parse[2], AP_GetTrueStat(who, give_parse[2]) + cint(give_parse[3]));
         // else
         //    var oldskill := AP_GetTrueSkill(who, give_parse[2]);
         //    AP_SetTrueSkill(who, give_parse[2], oldskill+1);
         endif
         sleepms(2);
      endforeach
   endif

   if (chardata.subclasses)
      foreach classe in (chardata.subclasses)
         all_feats := all_feats + classe.habs;
         sleepms(2);
      endforeach
   endif

	return feat_list;
endfunction

function HaveFeat(who, feat)
	var feat_list := GetFeatList(who);

   if (typeOf(feat) != "Integer")
      return 0;
   endif

	if (feat in feat_list)
		return 1;
	endif

	return 0;
endfunction

function HaveGodFeat(who, feat)
	var chardata := GetObjProperty(who, "chardata");
   if (!chardata) return 0; endif

   if (typeOf(feat) != "Integer")
      return 0;
   endif

   feat := GetFeatByid(feat);
	if (feat.God == chardata.god && (chardata.level >= feat.Level && !who.cmdlevel))
		return 1;
	endif

	return 0;
endfunction


function HaveFeatPrimary(who, feat)
	var chardata := GetObjProperty(who, "chardata");
   if (!chardata) return 0; endif

   if (typeOf(feat) != "Integer")
      return 0;
   endif

	if (feat in (chardata.classes[1].selected_habs))
		return 1;
	endif

	return 0;
endfunction

function BuildFeatToolTip(feat, return_points := 0, power := 0)
   var desc := "";
   if (feat.Script) desc += "{}Habilidade Ativável<br>".format(ORANGE, feat.PointCost);
   else desc += "{}Habilidade Passiva<br>".format(ORANGE, feat.PointCost); endif
   if (return_points && feat.PointCost) desc += "{}Custo: {} ponto(s)<br>".format(LIGHTGRAY, feat.PointCost); endif
   if (return_points && feat.Level) desc += "{}Level do Personagem: {}<br>".format(LIGHTGRAY, feat.Level); endif
   if (return_points && feat.Group) desc += "{}Level de Classe: {}<br>".format(LIGHTGRAY, feat.Group); endif
   if (feat.Mana) desc += "{}Mana: {}<br>".format(BLUE, feat.Mana); endif
   if (feat.Vigor) desc += "{}Vigor: {}<br>".format(YELLOW, feat.Vigor); endif
   if (feat.Cooldown) desc += "{}Cooldown: {}<br>".format(LIGHTGRAY, feat.Cooldown); endif
   if (feat.Description) desc += "{}{}<br>".format(WHITE, feat.Description); endif
   var tooltip := "<center><basefont color=#7FFFD4><b>{}</b></basefont><br>{}".format(feat.Name, desc);

   foreach mod in (feat.PowerMod)
      if (power)
         mod := cint(cdbl(power) * (cdbl(mod) / 100));
         tooltip["$"] := "{}".format(mod);
      else
         tooltip["$"] := "{}%".format(mod);
      endif
      sleepms(2);
   endforeach

   return tooltip;
endfunction

// function IsBloodied(mobile)

// 	//if (mobile.npctemplate)
// 		if (AP_GetVital(mobile, HITS) < CInt(AP_GetVitalMaximumValue(mobile, HITS)*0.3))
// 			return 1;
// 		endif
// 	/*elseif ( AP_GetVital(mobile, HITS) < GetHP(mobile) )
// 		return 1;
// 	endif*/

// 	return 0;

// endfunction


function ChooseTarget(mobile, feat)
   var move_script := start_script(":magery:canceltarget_if_move", mobile);
   var targ;
   
   if (feat.Targ == "Self")
      targ := mobile;
   elseif (feat.Targ == "Opponent")
      targ := mobile.opponent;
      var rang := _ifElse(feat.Range, feat.Range, 1);

      if (!targ)
         targ := ListHostiles(mobile, rang);
         targ := targ[1];
      endif

      if (!targ)
         targ := 0;
      elseif (rang && CoordinateDistance(mobile.x, mobile.y, targ.x, targ.y) > rang)
         targ := 0;
      endif
   elseif (feat.Targ == "Mobile" || feat.Targ == "Item")
      while (mobile.connected)
         SendSysMessageEx(mobile, "Escolha o alvo da habilidade.", SSM_REQUEST);
         var noto := TGTOPT_NEUTRAL;
         if (feat.TargetAlign == "Harmful")
            noto := TGTOPT_HARMFUL;
         elseif (feat.TargetAlign == "Helpful")
            noto := TGTOPT_HELPFUL;
         endif

         targ := Target(mobile, noto);
         if (!targ)
            break;
         elseif (GetCooldown(targ, "repeal") > 0)
            SendSysMessageEx(mobile, "O alvo esta invulneravel a habilidades.", SSM_FAIL);
         elseif (feat.Targ == "Mobile" && !targ.isA(POLCLASS_MOBILE))
            SendSysMessageEx(mobile, "Você deve escolher uma criatura como alvo.", SSM_FAIL);
         elseif (feat.Targ == "Item" && !targ.isA(POLCLASS_ITEM) && !targ.isA(POLCLASS_CONTAINER))
            SendSysMessageEx(mobile, "Você deve escolher um objeto como alvo", SSM_FAIL);
         elseif (!feat.NoLos && !CheckLineOfSight(mobile, targ));
            SendSysMessageEx(mobile, "Você não tem visão do alvo.", SSM_FAIL);
         elseif (!feat.Range)
            break;
         elseif (feat.Range && CoordinateDistance(mobile.x, mobile.y, targ.x, targ.y) > feat.Range)
            SendSysMessageEx(mobile, "Muito distante!", SSM_FAIL);
         else
            break;
         endif
      endwhile
      if (mobile != targ && targ)
         TurnObjectToward(mobile, targ.x, targ.y);
      endif
   elseif (feat.Targ == "Area")
      while (mobile.connected)
         PrintTextPrivate(mobile, "'Escolha o ponto alvo.'", mobile, SSM_INFO);
         targ := TargetCoordinates(mobile);
         if (!targ)
            break;
         endif

         TurnObjectToward(mobile, targ.x, targ.y);

         if (!feat.NoLos && !CheckLosAt( mobile, targ.x, targ.y, targ.z ));
            SendSysMessageEx(mobile, "Você não tem visão da area escolhida.", SSM_FAIL);
         elseif(!feat.Range)
            break;
         elseif (CoordinateDistance(mobile.x, mobile.y, targ.x, targ.y) > feat.Range)
            SendSysMessageEx(mobile, "Muito distante!", SSM_FAIL);
         else
            break;
         endif
      endwhile
   endif

   move_script.kill();
   return targ;
endfunction


function TriggerHabilitie(who, hab)
	if ( TS_GetCooldown(who, "hab_{}".format(hab.FeatID)) != error)
		return SendSystemReport(who, "'Habilidade em Cooldown!'", SSM_FAIL);
   elseif (hab.RequireMagicWeapon && !who.weapon.isMagicWeapon())
		return SendSystemReport(who, "'Você precisa estar equipado com uma arma mágica para esta habilidade.'", SSM_FAIL);
	endif
   
   if (TS_GetCooldown(who, "dodge") || TS_GetCooldown(who, "block"))
		return SendSysMessageEx(who, "'Você não pode usar habilidades quando estiver esquivando ou bloqueado.'", SSM_FAIL);
   endif
   SetObjProperty(who, "#Casting", 1);
   
   if (hab.Mana)
      var mana := hab.Mana + cint(cdbl(hab.Mana) * (AP_GetTrueSkill(who, "FeatConsume") / 100));
      if (!AP_ConsumeVital(who, MANA, mana))
         SendSystemReport(who, "'Mana Insuficiente!'", SSM_FAIL);
         EraseObjProperty(who, "#Casting");
         return SendSysMessageEx(who, "Você não tem mana suficiente para executar esta habilidade.", SSM_FAIL);
      endif
   endif

   if (hab.Vigor)
      var vigor := hab.Vigor + cint(cdbl(hab.Vigor) * (AP_GetTrueSkill(who, "FeatConsume") / 100));
      if (!AP_ConsumeVital(who, STAMINA, vigor))
         SendSystemReport(who, "'Vigor Insuficiente!'", SSM_FAIL);
         EraseObjProperty(who, "#Casting");
         return SendSysMessageEx(who, "Você não tem vigor suficiente para executar esta habilidade.", SSM_FAIL);
      endif
   endif
   TS_StartCooldown(who, "HabUsed", 300);
   who.hidden := 0;

   var weapon := who.weapon;
   var power := CalculateRawDamage(who, 0, weapon);
   
   if (hab.CastTime)
      var cycles := hab.CastTime;
      if (HaveGodFeat(who, 83) && Randomint(100) <= 20)
         cycles := 0;
         SendSystemReport(who, "Conjuração Rápida", SSM_INFO);
      endif
      
      var moveCheck := Start_Script(":magery:moveCheck", array{who, Getpid()});

      while (cycles)
         if ( Events_Waiting() || TS_GetTimerInfo(who, "tombar") || who.frozen || who.paralyzed )
            EraseObjProperty(who, "#Casting");
            SendSysMessageEx(who, "Você não conseguiu se concentrar o suficiente para usar esta habilidade.", SSM_FAIL);
            // FailSpell(who);
            return 0;
         endif

         case ( hab.Target )
            "Mobile":
               PerformAction(who, ANIM_CAST_DIR);
               break;
            "Area":
               PerformAction(who, ANIM_CAST_AREA);
               break;
            default:
               PerformAction(who, ANIM_CAST_DIR);
         endcase

         cycles -= 1;
         sleepms(800);
      endwhile

      moveCheck.kill();
   endif

   var targ := ChooseTarget(who, hab);
   if (!targ) //verifica se o ponto ¿ v¿sivel
      SendsysMessageEx(who, "Habilidade cancelada", SSM_FAIL);
      FailFeat(who, hab);
      return;
   endif

	if (hab.Cooldown && !GetObjProperty(who, "no_cd"))
      TS_StartCooldown(who, "hab_{}".format(hab.FeatID), hab.Cooldown);
   endif

   if (hab.Target != "Self")
      if (TS_GetCooldown(who, "hallucination") && targ.isA(POLCLASS_NPC))
         var mobile_around := ListMobilesInLineOfSight( who, 10);
         targ := mobile_around.randomentry();
      endif
   endif

   if (targ.isA(POLCLASS_NPC) && hab.TargetAlign == "Harmful")
      targ.setAggro(who.serial, 25);
   endif

   calcPowerMod(hab, power);

   var script := ":charactercreation:feats/{}";
   if (hab.script[":"])
      script := hab.script;
   else
      script := script.format(hab.script);
   endif

   var e := start_script(script, array{who, hab, targ, power, weapon});
   if (e == error)
      FailFeat(who, hab);
      return SendSysMessageEx(who, "Script da Habilidade não encontrado.", SSM_FAIL);
   endif

   EraseObjProperty(who, "#magiasilenciosa");
   EraseObjProperty(who, "#Casting");
endfunction

function calcPowerMod(byref hab, power)
   foreach mod in (hab.PowerMod)
      mod := CalculatePowerPercent(power, mod);
      sleepms(2);
   endforeach

   return hab;
endfunction

function FailFeat(who, hab)
   if (hab.Mana)
      var mana := hab.Mana + cint(cdbl(hab.Mana) * (AP_GetTrueSkill(who, "FeatConsume") / 100));
      HealMana(who, mana);
   endif
   
   if (hab.Vigor)
      var vigor := hab.Vigor + cint(cdbl(hab.Vigor) * (AP_GetTrueSkill(who, "FeatConsume") / 100));
      HealStamina(who, vigor);
   endif

   TS_LowerDuration(who, "hab_{}".format(hab.FeatID), -1, 1);

   EraseObjProperty(who, "#Casting");
endfunction