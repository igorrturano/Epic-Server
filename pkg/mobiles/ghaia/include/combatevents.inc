
include ":tn:tngumps";
//include ":yggdrasil:boss";

function LoadCombatEventData()
    var df := DFOpenDataFile( ":ghaia:combatevents", DF_CREATE, DF_KEYTYPE_INTEGER);
    return df;
endfunction

function GetAllVents(filter := 0)
   var df := LoadCombatEventData();
   var events := array{};
   foreach event in (DFGetElemNames(df))
      var df_event := DFFindElementEx(df, event);
      var monsters := DFGetProp(df_event, "monsters", DF_CREATE, "");
      if (!filter || monsters[filter])
         events.append(struct{
            "number" := event,
            "name" := DFGetProp(df_event, "name", DF_CREATE, "No Name"),
            "cd" := DFGetProp(df_event, "cd", DF_CREATE, 0),
            "dist" := DFGetProp(df_event, "dist", DF_CREATE, 0),
            "distm" := DFGetProp(df_event, "distm", DF_CREATE, 0),
            "chance" := DFGetProp(df_event, "chance", DF_CREATE, 0),
            "icon" := DFGetProp(df_event, "icon", DF_CREATE, 0),
            "description" := DFGetProp(df_event, "description", DF_CREATE, ""),
            "hp_percent" := DFGetProp(df_event, "hp_percent", DF_CREATE, 0),
            "steps" := DFGetProp(df_event, "steps", DF_CREATE, array{}),
            monsters := monsters
         });
      endif
      sleepms(10);
   endforeach

   return events;
endfunction

function UpdateCombatEvent(combatevent, index := 0)
   var df := LoadCombatEventData();
   if (!index)
      index := DFGetElemNames(df).size() +1;
   endif

   var elem := DFFindElementEx(df, index, DF_CREATE);
   elem.SetProp("name", combatevent.name);
   elem.SetProp("cd", combatevent.cd);
   elem.SetProp("dist", combatevent.dist);
   elem.SetProp("distm", combatevent.distm);
   elem.SetProp("chance", combatevent.chance);
   elem.SetProp("icon", combatevent.icon);
   elem.SetProp("hp_percent", combatevent.hp_percent);
   elem.SetProp("monsters", combatevent.monsters);
   elem.SetProp("description", combatevent.description);
   elem.SetProp("steps", combatevent.steps);
endfunction

function RemoveCombatEvent(index)
   if (!index)
      return; 
   endif

   var df := LoadCombatEventData();
   df.DeleteElement(index);
endfunction

function CopyCombatEvent(who, what)
   if (what.isA(POCLASS_MOBILE))

   endif
endfunction

/*

function ConfigureCombatEvent(who, event := 0, change_event := 0, no_mobile := 0)
	if (!event || change_event == 1)
      if (!change_event)
         var choice := RadioGump(who, 0, 0, "O que deseja fazer?", array{"Nova Habilidade", "Copiar de um Boss","Copiar da lista"}, 0, 1);
         if (choice["Boss"])
            SendSysMessageEx(who, "Selecione o NPC", SSM_REQUEST);
            var targ := Target(who);
            if (targ && targ.isA(POLCLASS_NPC))
               var boss_struct := GetBossStruct(targ);
               if (!boss_struct || !boss_struct.combatevent || !boss_struct.combatevent.size())
                  SendSysMessageEx(who, "Esse monstro n�o tem eventos de combate", SSM_FAIL);
                  return event;
               endif

               var opts := array{};
               foreach opt in (boss_struct.combatevent)
                  opts.append(opt.name);
                  sleepms(2);
               endforeach

               var input_list := array{};
               input_list.append(struct{ "title" := "Qual habilidade deseja copiar?", "marked" := 1, "radio" := opts});
               var output := QuestionsGump(who, input_list);
               if (!output) return event; endif

               return boss_struct.combatevent[output[1] in opts];
            endif

            return event;
         elseif (choice["lista"])
            var all_events := GetAllVents();
         
            var opts := array{};
            foreach opt in (all_events)
               opts.append(opt.name);
               sleepms(2);
            endforeach

            var input_list := array{};
            input_list.append(struct{ "title" := "Qual habilidade deseja copiar?", "marked" := 1, "radio" := opts});
            var output := QuestionsGump(who, input_list);
               if (!output) return event; endif

               return all_events[output[1] in opts];
         endif
      endif

      if (!event)
         event := struct{
            name := "",
            cd := 10,
            dist := 0,
            distm := 0,
            chance := 100,
            hp_percent := 100,
            steps := array{}
         };
      endif
      var input_list := array{};
      input_list.append(struct{ "title" := "Digite o nome desse evento de combate?. Ex: Ataque Multiplo", "marked" := event.name});
      input_list.append(struct{ "title" := "De quanto em quanto tempo o npc realiza esse evento? (segundos)", "marked" := ""+event.cd});
      input_list.append(struct{ "title" := "Qual a distancia minima entre o npc e o alvo? (0 para n�o checar)", "marked" := ""+event.dist});
      input_list.append(struct{ "title" := "Qual a distancia m�xima entre o npc e o alvo? (0 para n�o checar)", "marked" := ""+cint(event.distm)});
      input_list.append(struct{ "title" := "Qual a chance do npc realizar esta a��o? (100 para sempre)", "marked" := ""+event.chance});
      input_list.append(struct{ "title" := "Quanto % de hp o npc deve ter para realizar este evento? (100 para sempre)", "marked" := ""+event.hp_percent});
      input_list.append(struct{ "title" := "Icone para posses (opcional)", "marked" := ""+cint(event.icon)});

      var output := QuestionsGump(who, input_list);
      if (!output) return; endif
      
		event.name := output[1];
		event.cd := cint(output[2]);
		event.onlybloodied := 0;
		event.dist := cint(output[3]);
		event.distm := cint(output[4]);
		event.chance := cint(output[5]);
		event.hp_percent := cint(output[6]);
		event.icon := cint(output[7]);
	endif
	var continua := 1;

	while (continua)
      var gumpcombat := GFCreateGump();

      GFDisposable(gumpcombat, 0);
      GFClosable(gumpcombat, 1);
      GFResizePic(gumpcombat, 0, 25, 3700, 250, 430);	
      GFResizePic(gumpcombat, 15, 40, 0x13BE, 220, 400);
      GFTextMid(gumpcombat, 20, 50, 250, 1051, "Combat Events");
      
      var y := 64;
      GFTextLine(gumpcombat, 35, y, 1051, "Adicionar Novo ");
      GFAddButton(gumpcombat, 20, y+2, 2117, 2118, 1, 100);
      
      var i := 1;
      foreach state in (event.steps)
         y := y+ 15;
         GFTextLine(gumpcombat, 35, y, 1051, "{}. {}".format(_state_iter, state.type));
         GFAddButton(gumpcombat, 20, y+2, 2117, 2118, 1, 400+i);
         i := i + 1;
      endforeach

      GFAddButton(gumpcombat, 180, 387, 4502, 4502, 1, 1000 );
      GFTextLine(gumpcombat, 140, 400, 1051, "Aplicar");
      
      var input := GFSendGump(who, gumpcombat);

      if (input[0] == 100)
         // if (choice["Novo"])
         var state := CombatEventsOptions(who, struct{}, no_mobile);
         if (state && state.keys().size()) event.steps.append(state); endif
         // elseif (choice["Boss"])
         // elseif (choice["lista"])
         // endif
      elseif (input[0] == 1000)
         continua := 0;
      elseif (input[0] >= 400)
         input := input[0] - 400;
         var oldstate := event.steps[input];
         var choice := RadioGump(who, 0, 0, "O que deseja fazer?", array{"Alterar", "Mudar Posi��o", "Remover", "Cancelar"}, 0, 1);
         if (choice == "Alterar")
            var state := CombatEventsOptions(who, oldstate, no_mobile);
            if (state) 
               event.steps.Erase(input);
               event.steps.Insert(input, state);
            endif
         elseif (choice["Mudar Posi"])
            var position := cint(RequestGump(who, "Qual posi��o??", "Ex: 1", "1", 1));
            if (position == 0)
               position := 1;
            elseif (position > event.steps.size())
               position := event.steps.size();
            endif
            event.steps.Erase(input);
            event.steps.Insert(position, oldstate);
         elseif(choice["Remover"])
            event.steps.Erase(input);
         endif
      else
         continua := 0;
      endif
	endwhile

	return event;
endfunction

*/

/*

function CombatEventsOptions(who, state, no_mobile := 0)
   var opt;
   if (!state.type)
      var input_list := array{struct{
         "title" := "Novo estado:",
         "marked" := 1, 
         "radio" := array{
            "Create Mine",
            "Create Item",
            "Consume Creature",
            "Dmg", 
            "Frozen Until End",
            "Graphic Effect", 
            "Heal",
            // "Custom HitScript", 
            "If Block/Dodge",
            "Delay",
            "Hidden",
            "Move",
            "Metamorfose",
            "Magicball",
            "Pushback", 
            "Pull Target",
            "Play Sound", 
            // "Resist Test", 
            "Summon Npcs", 
            "Simple Target",
            "Set Cooldown", 
            // "Stop Monster",
            "Spell Trilha",
            "Teleport", 
            "TimedScript", 
            "Throw Bomb",
            "Broken Equips",
            "Text"
            }
         }
      };

      if (!no_mobile)
         input_list[1].radio += array{ "Steal Player", "Perform Action", "Play Cast Anim", "Dash" };
      endif

      opt := QuestionsGump(who, input_list);
      if (!opt) return; endif
      opt := opt[1];
      state.type := opt;
   else
      opt := state.type;
   endif

   var dmgs := array{DMG_SLASH, DMG_BLUDGE, DMG_PIERCE, DMG_FIRE, DMG_COLD, DMG_POISON,DMG_ENERGY, "Sem Resist�ncia" };
   var resists := array{"block", "dodge"};
   var targ_opts := array{"Self", "Cone in front", "Cone back", "Cone left", "Cone right", "Around Self", "Opponent", "Around Opponent", "All Opponents", "Random Opponent", "Points to Opponent", "In Line", "All Allies", "Random Ally", "Previous Targets"};
   if (opt == "Move")
      SendSysMessageEx(who, "Para onde o npc deve andar?");
      var loc := TargetCoordinates(who);
      state.x := loc.x;
      state.y := loc.y;
      state.z := loc.z;
   elseif ( opt == "Pull Target")
      if(!state.text) state.text := ""; endif

      var input_list := array{};
      input_list.append(struct{ "title" := "Quem vai ser puxado?", "marked" := _ifElse(state.target, state.target in targ_opts, 1), "radio" := targ_opts});
      input_list.append(struct{ "title" := "Quantas vezes ser� puxado?", "marked" := ""+cint(state.times) });
      input_list.append(struct{ "title" := "Qual mensagem ser� reproduzida?", "marked" := ""+state.text});
      input_list.append(struct{ "title" := "Qual a��o o monstro executar�?", "marked" := ""+cint(state.action)});
      if (!state.conedist) state.conedist := 3; endif
      input_list.append(struct{ "title" := "Dist�ncia para efeito target (all, around, cone, inline...) ?", "marked" := ""+cint(state.conedist)});
      var output := QuestionsGump(who, input_list, HIDDEN_CANCEL);

      state.target := output[1];
      state.times := cint(output[2]);
      state.text := output[3];
      state.action := cint(output[4]);
      state.conedist := cint(output[5]);
   elseif ( opt == "Play Cast Anim" )
      var input_list := array{};
      input_list.append(struct{ "title" := "Quantos cycles de cast? (0 para o sistema fazer autom�tico", "marked" := ""+cint(state.times)});
      var output := QuestionsGump(who, input_list, HIDDEN_CANCEL);

      state.times := cint(output[1]);
   elseif ( opt == "Steal Player")
      var input_list := array{};
      input_list.append(struct{ "title" := "Quem vai ser roubado?", "marked" := _ifElse(state.target, state.target in targ_opts, 1), "radio" := targ_opts});
      input_list.append(struct{ "title" := "Apenas roubar se estiver atr�s do alvo?", "marked" := 1+cint(state.onlybehind), "radio" := array{"Sim", "N�o"}});
      if (!state.conedist) state.conedist := 3; endif
      input_list.append(struct{ "title" := "Dist�ncia para efeito target (all, around, cone, inline...) ?", "marked" := ""+cint(state.conedist)});
      var output := QuestionsGump(who, input_list, HIDDEN_CANCEL);

      state.target := output[1];
      state.onlybehind := output[2] == "N�o";
      state.conedist := cint(output[3]);
   elseif ( opt == "Broken Equips")
      var input_list := array{};
      input_list.append(struct{ "title" := "Quais alvos?", "marked" := _ifElse(state.target, state.target in targ_opts, 1), "radio" := targ_opts});
      input_list.append(struct{ "title" := "Quantos equipamentos ser�o danificados?", "marked" := _ifOr(state.amount, 1)});
      if (!state.conedist) state.conedist := 3; endif
      input_list.append(struct{ "title" := "Dist�ncia para efeito target (all, around, cone, inline...) ?", "marked" := ""+cint(state.conedist)});
      var output := QuestionsGump(who, input_list, HIDDEN_CANCEL);

      state.target := output[1];
      state.amount := cint(output[2]);
      state.conedist := cint(output[3]);
   elseif ( opt == "Simple Target")
      var input_list := array{};
      input_list.append(struct{ "title" := "Quais alvos?", "marked" := _ifElse(state.target, state.target in targ_opts, 1), "radio" := targ_opts});
      input_list.append(struct{ "title" := "Checar linha de vis�o?", "marked" := 1+cint(state.nochecklos), "radio" := array{"N�o", "Sim"}});
      input_list.append(struct{ "title" := "Trancar Previous Target? (Outros targs n�o substituir�o esse valor)", "marked" := 1+cint(state.nolocktargs), "radio" := array{"N�o", "Sim"}});
      input_list.append(struct{ "title" := "Inclur boss nos targets?", "marked" := 1+cint(state.noincludeself), "radio" := array{"N�o", "Sim"}});
      input_list.append(struct{ "title" := "Parar habilidade se n�o houver targets?", "marked" := 1+cint(state.nostopifnotarg), "radio" := array{"N�o", "Sim"}});
      if (!state.conedist) state.conedist := 3; endif
      input_list.append(struct{ "title" := "Dist�ncia para efeito target (all, around, cone, inline...) ?", "marked" := ""+cint(state.conedist)});
      var output := QuestionsGump(who, input_list, HIDDEN_CANCEL);

      state.target := output[1];
      state.nochecklos := output[2] == "Sim";
      state.nolocktargs := output[3] == "Sim";
      state.noincludeself := output[4] == "Sim";
      state.nostopifnotarg := output[5] == "Sim";
      state.conedist := cint(output[6]);
   elseif ( opt == "Pushback")
      var behaviours := array{"Para frente", "Para tr�s", "Para frente relacional", "Para tr�s relacional"};
      var input_list := array{};
      input_list.append(struct{ "title" := "Quem vai sofrer a pushback?", "marked" := _ifElse(state.target, state.target in targ_opts, 1), "radio" := targ_opts});
      input_list.append(struct{ "title" := "Qual o comportamento?", "marked" := state.behaviour in behaviours, "radio" := behaviours});
      input_list.append(struct{ "title" := "Quantas vezes ser� aplicado?", "marked" := ""+cint(state.times)});
      if (!state.conedist) state.conedist := 3; endif
      input_list.append(struct{ "title" := "Dist�ncia para efeito target (all, around, cone, inline...) ?", "marked" := ""+cint(state.conedist)});
      var output := QuestionsGump(who, input_list, HIDDEN_CANCEL);

      state.target := output[1];
      state.behaviour := output[2];
      state.times := cint(output[3]);
      state.conedist := cint(output[4]);
   elseif ( opt == "Consume Creature")
      var input_list := array{};
      input_list.append(struct{ "title" := "Onde vai procurar pelo alvo?", "marked" := _ifElse(state.target, state.target in targ_opts, 1), "radio" := targ_opts});
      input_list.append(struct{ "title" := "O que vai comer?", "marked" := cint(state.what_type == "Item")+1, "radio" := array{"Criatura", "Item"}});
      input_list.append(struct{ "title" := "Template da criatura ou ID do item:", "marked" := cint(state.what) });
      input_list.append(struct{ "title" := "Quanto % vai se curar?", "marked" := cint(state.hp_percent) });
      input_list.append(struct{ "title" := "Parar script se alvo n�o for encontrado?", "marked" := cint(state.stopscript == "Sim")+1, "radio" := array{"N�o", "Sim"} });
      if (!state.conedist) state.conedist := 3; endif
      input_list.append(struct{ "title" := "Dist�ncia para efeito target (all, around, cone, inline...) ?", "marked" := ""+cint(state.conedist)});
      var output := QuestionsGump(who, input_list, HIDDEN_CANCEL);
      
      state.target := output[1];
      state.what_type := output[2];
      state.what := output[3];
      state.hp_percent := cint(output[4]);
      state.stopscript := cint(output[5] == "Sim");
      state.conedist := cint(output[6]);
   elseif ( opt == "Teleport")
      var xyz := "";
      if (state.x || state.y || state.z)
         xyz := "{} {} {}".format(state.x, state.y, state.z);
      endif
      var tele_opts := array{"Teleportar X,Y,Z", "Alvo (DO NPC)", "Tele no NPC", "Aplicar +-X,Y,Z", "Ao redor (Alvo)", "Ao redor (NPC)"};
      var input_list := array{};
      input_list.append(struct{ "title" := "Quem vai sofrer a teleporte?", "marked" := _ifElse(state.target, state.target in targ_opts, 1), "radio" := targ_opts});
      input_list.append(struct{ "title" := "Escolha o comportamento", "marked" := state.behaviour in tele_opts, "radio" := tele_opts });
      input_list.append(struct{ "title" := "Qual o X Y Z? (Apenas para primeira e ultima op��o)", "marked" := xyz});
      if (!state.conedist) state.conedist := 3; endif
      input_list.append(struct{ "title" := "Dist�ncia para efeito target (all, around, cone, inline...) ?", "marked" := ""+cint(state.conedist)});

      var output := QuestionsGump(who, input_list, HIDDEN_CANCEL);
      
      state.target := output[1];
      state.behaviour := output[2];
      state.conedist := cint(output[4]);

      var loc := SplitWords(output[3]);
      state.x := cint(loc[1]);
      state.y := cint(loc[2]);
      state.z := cint(loc[3]);
   elseif ( opt == "If Block/Dodge")
      var resist_choice := state.resist_test;
      if (!resist_choice) resist_choice := array{"block", "dodge"}; endif
      foreach resist in resist_choice
         resist := resist in resists;
      endforeach
      if (!state.resist_diff) state.resist_diff := 14; endif
      var input_list := array{};
      input_list.append(struct{ "title" := "Quem vai ser checado?", "marked" := _ifElse(state.target, state.target in targ_opts, 1), "radio" := targ_opts});
      input_list.append(struct{ "title" := "Qual tipo de defesa ser� checado?", "marked" := resist_choice, "checkbox" :=  resists });
      input_list.append(struct{ "title" := "Passar para 'Previous Target'?", "marked" := cint(state.successonly)+1, "radio" :=  array{"Se Def N�o Ativa", "Se Def Ativa"} });
      if (!state.conedist) state.conedist := 3; endif
      input_list.append(struct{ "title" := "Dist�ncia para efeito target (all, around, cone, inline...) ?", "marked" := ""+cint(state.conedist)});
      var output := QuestionsGump(who, input_list, HIDDEN_CANCEL);
      
      state.target        := output[1];
      state.resist_test := output[2];
      state.successonly := output[3] == "Se Def Ativa";
      state.conedist    := cint(output[4]);
   elseif ( opt == "Dash")
      var input_list := array{};
      input_list.append(struct{ "title" := "Qual alvo do dash?", "marked" := _ifElse(state.target, state.target in targ_opts, 1), "radio" := targ_opts});
      input_list.append(struct{ "title" := "Qual dist�ncia a percorrer? [at� 120s]", "marked" := ""+cint(state.dist)});
      input_list.append(struct{ "title" := "Lan�ar mobiles no Previous Target?", "marked" := ""+cint(state.nosetprevious)+1, radio := array{"Sim", "N�o"}});
      if (!state.conedist) state.conedist := 3; endif
      input_list.append(struct{ "title" := "Dist�ncia para efeito target (all, around, cone, inline...) ?", "marked" := ""+cint(state.conedist)});

      var output := QuestionsGump(who, input_list, HIDDEN_CANCEL);
      state.target        := output[1];
      state.dist        := cint(output[2]);
      state.nosetprevious := cint(output[3] == "N�o");
      state.conedist      := cint(output[4]);
   elseif ( opt == "TimedScript")
      if (!state.resist_diff) state.resist_diff := 14; endif

      var timedcfg := TS_GetCfgFile();
      var timerlist := GetConfigStringKeys(timedcfg);

      var input_list := array{};
      input_list.append(struct{ "title" := "Quem vai sofrer a action?", "marked" := _ifElse(state.target, state.target in targ_opts, 1), "radio" := targ_opts});
      input_list.append(struct{ "title" := "Escolha o timedscript", "marked" := state.timername in timerlist, "radio" := timerlist });
      input_list.append(struct{ "title" := "Qual a dura��o do timer? [at� 120s]", "marked" := ""+cint(state.timerduration)});
      input_list.append(struct{ "title" := "Qual o level do timer?", "marked" := ""+cint(state.level)});
      if (!state.conedist) state.conedist := 3; endif
      input_list.append(struct{ "title" := "Dist�ncia para efeito target (all, around, cone, inline...) ?", "marked" := ""+cint(state.conedist)});

      var output := QuestionsGump(who, input_list, HIDDEN_CANCEL);
      
      state.target        := output[1];
      state.timername     := output[2];
      state.timerduration := cint(output[3]);
      state.level         := cint(output[4]);
      state.conedist      := cint(output[5]);
   elseif ( opt == "Perform Action" )
      var input_list := array{}; 
      input_list.append(struct{ "title" := "Quem vai realizar a action?", "marked" := _ifElse(state.target, state.target in targ_opts, 1), "radio" := targ_opts});
      input_list.append(struct{ "title" := "Qual o ID da anima��o?", "marked" := ""+cint(state.id)});
      if (!state.conedist) state.conedist := 3; endif
      input_list.append(struct{ "title" := "Dist�ncia para efeito target (all, around, cone, inline...) ?", "marked" := ""+cint(state.conedist)});
      var output := QuestionsGump(who, input_list, HIDDEN_CANCEL);

      state.target := output[1];
      state.id := cint(output[2]);
      state.conedist      := cint(output[3]);
   elseif (opt == "Text")
      var input_list := array{};
      if (!state.color) state.color := 1000; endif
      if (!state.text) state.text := ""; endif
      input_list.append(struct{ "title" := "Quem vai falar?", "marked" := _ifElse(state.target, state.target in targ_opts, 1), "radio" := targ_opts});
      input_list.append(struct{ "title" := "Qual o texto?", "marked" := ""+cstr(state.text)});
      input_list.append(struct{ "title" := "Qual a cor do texto?", "marked" := ""+cint(state.color)});
      if (!state.conedist) state.conedist := 3; endif
      input_list.append(struct{ "title" := "Dist�ncia para efeito target (all, around, cone, inline...) ?", "marked" := ""+cint(state.conedist)});
      var output := QuestionsGump(who, input_list, HIDDEN_CANCEL);

      state.target := output[1];
      state.text := output[2];
      state.color := cint(output[3]);
      state.conedist := cint(output[4]);
   elseif (opt == "Play Sound")
      state.id := cint(RequestGump(who, "Qual o ID do som?", "", ""+cint(state.id), 1));
   elseif (opt == "Delay")
      var input_list := array{};
      input_list.append(struct{ "title" := "Especificar em:", "marked" := cint(state.delaytype == "milissegundos")+1, radio := array{"segundos", "milissegundos"} });
      input_list.append(struct{ "title" := "Quanto tempo?", "marked" := ""+cint(state.delay) });
      var output := QuestionsGump(who, input_list, HIDDEN_CANCEL);

      state.delaytype := output[1];
      state.delay := cint(output[2]);
   elseif (opt == "Heal")
      var vital_list := array{"Stam", "Hits", "Mana"};
      var input_list := array{};
      input_list.append(struct{ "title" := "Quem vai ser curado?", "marked" := _ifElse(state.target, state.target in targ_opts, 1), "radio" := targ_opts + array{"Fixed Loc"}});
      input_list.append(struct{ "title" := "Qual porcentagem do poder em dano?", "marked" := ""+state.healamt});
      input_list.append(struct{ "title" := "Qual tipo de vital sera afetado pelo heal?", "marked" := state.vital in vital_list, "radio":= vital_list});
      if (!state.conedist) state.conedist := 3; endif
      input_list.append(struct{ "title" := "Dist�ncia para efeito target (all, around, cone, inline...) ?", "marked" := ""+cint(state.conedist)});
      var output := QuestionsGump(who, input_list, HIDDEN_CANCEL);
      
      state.target := output[1];
      state.healamt := output[2];
      state.vital := output[3];
      state.conedist := cint(output[4]);
   elseif (opt == "Graphic Effect")
      var effect_opts := array{"Lightning", "Red Sparkles", "Blue Sparkles", "Blue and Swirl", "Swirl Sparkles", "Blue Sparkles on Ground", "Fire Column", "Small Fireball", "Large Fireball", "Wavy Fireball", "Electric", "Outro"};
      var effect_output := array{"Lightning", GFX_RED_SPARKLES, GFX_BLUE_SPARKLES, GFX_BLUE_SPARKLE_SWIRL, GFX_BLUE_SPARKLE_BORDER, GFX_BLUE_SPARKLE_GROUND, GFX_FIRE_COLUMN, GFX_SMALL_FIREBALL, GFX_LARGE_FIREBALL, GFX_WAVY_FIREBALL};
      var effect_types := array{"Por pontos", "Por oponentes"};

      var effect_pos := state.fxid in effect_output;
      if (effect_pos <= 0 || !effect_pos) effect_pos := 12; endif
      var input_list := array{};
      var effects := array{"Stationary", "Moving", "Circle In", "Circle Out"};
      input_list.append(struct{ "title" := "Qual estilo de alvo?", "marked" := _ifOr(state.style in effect_types, 1), "radio" := effect_types });
      input_list.append(struct{ "title" := "Onde vai surgir o efeito?", "marked" := _ifElse(state.target, state.target in targ_opts, 1), "radio" := targ_opts});
      input_list.append(struct{ "title" := "Qual o tipo de efeito?", "marked" := _ifOr(state.graphictype in effects, 1), "radio" := effects });
      input_list.append(struct{ "title" := "Qual o efeito?", "marked" := effect_pos, "radio" := effect_opts });
      input_list.append(struct{ "title" := "(Outro) Qual o ID, SPEED e LOOPS do efeito?", multipleanswer := 3, "marked" := array{cint(state.fxid), ""+cint(state.speed), ""+cint(state.loops)}});
      input_list.append(struct{ "title" := "Qual a dire��o? (Moving, ignore for Cone)", "marked" := (state.direction == "Player->Mob")+1, "radio" := array{"Mob->Player", "Player->Mob"}});
      // input_list.append(struct{ "title" := "Digite o x y z onde o efeito vai aparecer: (stationary, ignore for Moving/cone)", "marked" := "{} {} {}".format(cint(state.startx), cint(state.starty), cint(state.startz))});
      // input_list.append(struct{ "title" := "Digite o x y z da posi��o final do efeito: (stationary, ignore for Moving/cone)", "marked" := "{} {} {}".format(cint(state.endx), cint(state.endy), cint(state.endz))});
      input_list.append(struct{ "title" := "Qual a cor do efeito?", "marked" := ""+cint(state.color)});
      if (!state.conedist) state.conedist := 3; endif
      input_list.append(struct{ "title" := "Dist�ncia para efeito target (all, around, cone, inline...) e Cone In/Out?", multipleanswer := 2, "marked" := array{cint(state.conedist), cint(state.circledist) } });
      var output := QuestionsGump(who, input_list, HIDDEN_CANCEL);


      state.style := output[1];
      state.target := output[2];
      state.graphictype := output[3];
      if ( output[4] == "Lightning" )
         state.fxid := "Lightning";
      elseif (output[4] == "Electric")
         state.fxid := "Electric";
      elseif ( output[4] == "Red Sparkles")
         state.fxid := GFX_RED_SPARKLES;
         state.speed := 5;
         state.loops := 7;
      elseif ( output[4] == "Blue Sparkles")
         state.fxid := GFX_BLUE_SPARKLES;
         state.speed := 10;
         state.loops := 10;
      elseif (output[4] == "Blue and Swirl")
         state.fxid := GFX_BLUE_SPARKLE_SWIRL;
         state.speed := 5;
         state.loops := 16;
      elseif (output[4] == "Swirl Sparkles")
         state.fxid := GFX_BLUE_SPARKLE_BORDER;
         state.speed := 5;
         state.loops := 0;
      elseif (output[4] == "Blue Sparkles on Ground")
         state.fxid := GFX_BLUE_SPARKLE_GROUND;
         state.speed := 5;
         state.loops := 10;
      elseif (output[4] == "Fire Column");
         state.fxid := GFX_FIRE_COLUMN;
         state.speed := 5;
         state.loops := 0;
      elseif (output[4] == "Small Fireball")
         state.fxid := GFX_SMALL_FIREBALL;
         state.speed := 5;
         state.loops := 0;
      elseif (output[4] == "Large Fireball")
         state.fxid := GFX_LARGE_FIREBALL; 
         state.speed := 5;
         state.loops := 0;
      elseif (output[4] == "Wavy Fireball")
         state.fxid := GFX_WAVY_FIREBALL;
         state.speed := 5;
         state.loops := 0;
      elseif (output[4] == "Outro")
         state.fxid := cint(output[5][1]);
         state.speed := cint(output[5][2]);
         state.loops := cint(output[5][3]);
      endif

      state.direction := output[6];
      state.color     := cint(output[7]);
      state.conedist  := cint(output[8][1]);
      state.circledist  := cint(output[8][2]);

      // if (state.target == "Fixed Loc")
      //    var loc := splitwords(output[6]);
      //    state.startx :=  loc[1];
      //    state.starty :=  loc[2];
      //    state.startz :=  loc[3];
      //    state.startradius := 0;

      //    if (state.graphictype == "Moving")
      //       var loc2 := splitwords(output[7]);
      //       state.endx :=  loc2[1];
      //       state.endy :=  loc2[2];
      //       state.endz :=  loc2[3];
      //       // var radius :=   RequestGump(who, "Qual o raio de onde o efeito vai terminar:", "0 se for sempre no mesmo lugar", "0", 1);
      //       state.endradius := 0;
      //    endif
      // endif
   elseif ( opt == "Dmg")
      if (!state.dmg) state.dmg := "100"; endif
      if (!state.conedist) state.conedist := 3; endif
      var dmg_types := array{"Stam", "Hits", "Mana"};
      var state_vitals := array{};
      if (state.vitals) state_vitals := state.vitals; endif
      foreach vit in state_vitals
         vit := vit in dmg_types;
      endforeach

      var input_list := array{};
      input_list.append(struct{ "title" := "Quem vai sofrer?", "marked" := _ifElse(state.target, state.target in targ_opts, 1), "radio" := targ_opts});
      input_list.append(struct{ "title" := "Qual porcentagem de poder em dano? Ex: 100", "marked" := ""+state.dmg});
      input_list.append(struct{ "title" := "Qual o tipo de dano?", "marked" := state.dmgtype in dmgs, "radio" := dmgs});
      input_list.append(struct{ "title" := "Qual tipo de vital sera afetado pelo dano?", "marked" := state_vitals, "checkbox":= dmg_types});
      if (!state.conedist) state.conedist := 3; endif
      input_list.append(struct{ "title" := "Dist�ncia para efeito target (all, around, cone, inline...) ?", "marked" := ""+cint(state.conedist)});
      var output := QuestionsGump(who, input_list, HIDDEN_CANCEL);

      state.target := output[1];
      state.dmg := cint(output[2]);
      state.dmgtype := output[3];
      state.conedist := cint(output[5]);
      if (state.dmgtype == "Sem Resist�ncia")
         state.dmgtype := -1;
      endif
      state.vitals := output[4];
   elseif ( opt == "Create Item")
      var xyz := "";
      if (state.target)
         if (!(state.target in targ_opts))
            xyz := " ".join(state.target);
         endif
      endif

      var p1 := "";
      var p2 := "";
      var p3 := "";
      if (state.p1) p1 := " ".join(state.p1); endif
      if (state.p2) p2 := " ".join(state.p2); endif
      if (state.p3) p3 := " ".join(state.p3); endif

      var input_list := array{};
      input_list.append(struct{ "title" := "Onde ser� criado?", "marked" := _ifElse(state.target, state.target in targ_opts, 1), "radio" := targ_opts});
      input_list.append(struct{ "title" := "Criar em X Y Z (deixa em branco para usar target)", "marked" := xyz });
      input_list.append(struct{ "title" := "Qual ID ou nome do item a ser criado?", "marked" := state.item });
      input_list.append(struct{ "title" := "Qual cor o item ter�?", "marked" := state.color });
      input_list.append(struct{ "title" := "O item pode ser pego por players?", "marked" := cint(state.movable)+1, "radio" := array{"N�o", "Sim"} });
      if (!state.conedist) state.conedist := 3; endif
      input_list.append(struct{ "title" := "Dist�ncia para efeito target (all, around, cone, inline...) ?", "marked" := ""+cint(state.conedist)});
      input_list.append(struct{ "title" := "Custom property (key value) integer", "marked" := p1 });
      input_list.append(struct{ "title" := "Custom property (key value) integer", "marked" := p2 });
      input_list.append(struct{ "title" := "Custom property (key value) string", "marked" := p3 });

      var output := QuestionsGump(who, input_list, HIDDEN_CANCEL);
      state.target := output[1];

      if (output[2]) state.target := Splitwords(output[2]); endif
      if (cint(output[3])) 
         state.item := cint(output[3]);
      else
         state.item := output[3];
      endif

      state.color := cint(output[4]);
      state.movable := cint(output[5] == "Sim");
      state.conedist := cint(output[6]);
      if (output[7]) state.p1 := SplitWords(output[7]); endif
      if (output[8]) state.p2 := SplitWords(output[8]); endif
      if (output[9]) state.p3 := SplitWords(output[9]); endif
   elseif (opt == "Summon Npcs")
      if (!state.npctemplate) state.npctemplate := ":ghaia:"; endif
      if (!state.amount) state.amount := "2"; endif
      if (!state.range) state.range := "5"; endif
      if (!state.tempo) state.tempo := "120"; endif
      var loc := "";
      if (state.x || state.y || state.z) loc := "{} {} {}".format(state.x, state.y, state.z); endif

      var walk_to := "6 6 6";
      if (state.walk_to) walk_to := " ".join(state.walk_to); endif

      var input_list := array{};
      input_list.append(struct{ "title" := "Onde ser� spawnado?", "marked" := _ifElse(state.target, state.target in targ_opts, 1), "radio" := targ_opts});
      input_list.append(struct{ "title" := "Qual o npctemplate?", "marked" := ""+state.npctemplate});
      input_list.append(struct{ "title" := "Quantos npcs vao ser spawnados?", "marked" := ""+cint(state.amount)});
      input_list.append(struct{ "title" := "Qual o raio de spawn?", "marked" := ""+cint(state.range)});
      input_list.append(struct{ "title" := "Qual o tempo de vida dos monstros? (segundos)", "marked" := ""+cint(state.tempo)});
      input_list.append(struct{ "title" := "Digite o x y z onde eles vao aparecer: (6 6 6 para usar o target)", "marked" := ""+loc});
      input_list.append(struct{ "title" := "Digite o x y z para onde eles v�o andar: (6 6 6 para onde o boss esta)", "marked" := walk_to});
      if (!state.conedist) state.conedist := 3; endif
      input_list.append(struct{ "title" := "Dist�ncia para efeito target (all, around, cone, inline...) ?", "marked" := ""+cint(state.conedist)});
      
      var output := QuestionsGump(who, input_list, HIDDEN_CANCEL);
      if (!NPC_Checktemplate(output[2])) SendSysMessageEx(who, "Template {} n�o existe!!!".format(output[2]), SSM_FAIL); endif
      state.target := output[1];
      state.npctemplate := output[2];
      state.amount := output[3];
      state.range := cint(output[4]);
      state.tempo := cint(output[5]);

      state.conedist := cint(output[8]);
      
      loc := output[6];
      if (!loc)
         SendSysMessageEx(who, "Onde v�o ser spawnados? (Esc para target)");
         loc := TargetCoordinates(who);
         if (!loc)
            loc := array{6, 6, 6};
         else
            loc := array{loc.x, loc.y, loc.z};
         endif
      else
         loc := splitwords(loc);
      endif
      
      state.x := cint(loc[1]);
      state.y := cint(loc[2]);
      state.z := cint(loc[3]);

      walk_to := SplitWords(output[7]);
      if (walk_to.size() == 3)
         state.walk_to := walk_to;
      endif
      
   elseif (opt == "Custom HitScript")
      SendSysMessage(who, "N�o implementado");
   elseif (opt == "Set Cooldown")
      var input_list := array{};
      input_list.append(struct{ "title" := "Qual o cooldown?", "marked" := "lockaggro" });
      input_list.append(struct{ "title" := "Quanto tempo vai durar?", "marked" := "10"});
      var output := QuestionsGump(who, input_list, HIDDEN_CANCEL);
      state.prop := output[1];
      state.time := cint(output[2]);
   elseif (opt == "Create Mine")
      var dmg := "100";
      if (state.dmg) dmg := state.dmg; endif

      var input_list := array{};
      input_list.append(struct{ "title" := "Onde ser� criado?", "marked" := _ifElse(state.target, _ifElse(state.target, state.target in targ_opts, 1), 1), "radio" := targ_opts});
      input_list.append(struct{ "title" := "Quantas minas?", "marked" := ""+cint(state.amount)});
      input_list.append(struct{ "title" := "Qual tipo da mina?", "marked" := 1+(state.mine_type == "Gelo"), radio := array{ "Fogo", "Gelo" }});
      input_list.append(struct{ "title" := "Qual porcentagem de poder em dano?", "marked" := dmg});
      if (!state.conedist) state.conedist := 3; endif
      input_list.append(struct{ "title" := "Dist�ncia para efeito target (all, around, cone, inline...) ?", "marked" := ""+cint(state.conedist)});

      var output := QuestionsGump(who, input_list, HIDDEN_CANCEL);

      state.target := output[1];
      state.amount := cint(output[2]);
      state.mine_type := output[3];
      state.dmg := cint(output[4]);
      state.conedist := cint(output[5]);
   elseif (opt == "Hidden")
      var stealthsteps := 15;
      if (state.stealthsteps) stealthsteps := state.stealthsteps; endif 
      var input_list := array{};
      input_list.append(struct{ "title" := "Quantos passos?", "marked" := ""+stealthsteps });
      input_list.append(struct{ "title" := "Ataca sem sair do modo furtivo?", "marked" := 1+cint(state.attackhidden), radio := array{"N�o", "Sim"} });
      input_list.append(struct{ "title" := "Fugir do alvo enquanto furtivo?", "marked" := 1+cint(state.flee), radio := array{"N�o", "Sim"} });
      var output := QuestionsGump(who, input_list, HIDDEN_CANCEL);
      state.stealthsteps := cint(output[1]);
      state.attackhidden := output[2] == "Sim";
      state.flee := output[3] == "Sim";
   elseif (opt == "Spell Trilha")
      var dmg := "100";
      var resist := 12;
      var duration := 10;
      if (state.dmg) dmg := state.dmg; endif
      if (state.diff) resist := state.diff; endif
      if (state.duration) duration := state.duration; endif

      var trilha_types := array{"Fogo", "Veneno", "Eletrecidade"};

      var input_list := array{};
      input_list.append(struct{ "title" := "Qual o alvo?", "marked" := _ifElse(state.target, state.target in targ_opts, 1), "radio" := targ_opts});
      input_list.append(struct{ "title" := "Qual tipo de trilha?", "marked" := state.trilha_type in trilha_types, radio := trilha_types });
      input_list.append(struct{ "title" := "Qual porcentagem de poder em dano?", "marked" := dmg });
      input_list.append(struct{ "title" := "Qual a dificuldade para esquivar?", "marked" := resist });
      input_list.append(struct{ "title" := "Qual o tempo de dura��o da mina?", "marked" := duration });
      if (!state.conedist) state.conedist := 3; endif
      input_list.append(struct{ "title" := "Dist�ncia para efeito target (all, around, cone, inline...) ?", "marked" := ""+cint(state.conedist)});

      var output := QuestionsGump(who, input_list, HIDDEN_CANCEL);

      state.target      := output[1];
      state.trilha_type := output[2];
      state.dice_dmg    := cint(output[3]);
      state.diff        := cint(output[4]);
      state.duration    := cint(output[5]);
      state.conedist := cint(output[6]);
   elseif (opt == "Throw Bomb")
      var bomb_opts := array{"FogoGregoLeve", "FogoGrego", "FogoGregoMedio", "FogoGregoForte", "pocaodecola", "gasvenenofraco","gasvenenomedio", "gasvenenoforte", "pocaoflamejantefraca", "pocaoflamejanteforte", "gaslucinogeno"};
      var marked := array{};
      if (state.bombs) marked := state.bombs; endif
      foreach b in marked
         b := b in bomb_opts;
         sleepms(2);
      endforeach

      var input_list := array{};
      input_list.append(struct{ "title" := "Qual o alvo?", "marked" := _ifElse(state.target, state.target in targ_opts, 1), "radio" := targ_opts});
      input_list.append(struct{ "title" := "Quais bombas pode lan�ar?", "marked" := marked, "checkbox" := bomb_opts});
      if (!state.conedist) state.conedist := 3; endif
      input_list.append(struct{ "title" := "Dist�ncia para efeito target (all, around, cone, inline...) ?", "marked" := ""+cint(state.conedist)});
      var output := QuestionsGump(who, input_list, HIDDEN_CANCEL);

      state.target := output[1];
      state.bombs := output[2];
      state.conedist := cint(output[3]);
   elseif (opt == "Metamorfose")
      var metamorf := ":ghaia:";
      if (state.monsters) metamorf := " ".join(state.monsters); endif
      var input_list := array{};
      input_list.append(struct{ "title" := "Para qual npctemplate? (separar por espa�o para mais op��es, sem :ghaia:)", "marked" := metamorf });
      input_list.append(struct{ "title" := "Quanto tempo de dura��o?", "marked" := ""+cint(state.time) });
      var output := QuestionsGump(who, input_list, HIDDEN_CANCEL);

      state.monsters := Splitwords(output[1]);
      state.time := cint(output[2]);
   elseif (opt == "Magicball")
      var dmg_dice := 100;
      var duration := 6;
      if (state.dmg_dice) dmg_dice := state.dmg_dice; endif
      if (state.duration) duration := state.duration; endif

      var input_list := array{};
      input_list.append(struct{ "title" := "Qual o alvo?", "marked" := _ifElse(state.target, state.target in targ_opts, 1), "radio" := targ_opts});
      input_list.append(struct{ "title" := "Criar em +X +Y +Z? (vazio para lan�ar do npc)", marked := "" });
      input_list.append(struct{ "title" := "Enviar na dire��o ou seguir target?", "marked" := 1+cint(state.to_where == "Target"), radio := array{"Local", "Target"} });
      input_list.append(struct{ "title" := "Qual type?", "marked" := 1+cint(state.ball_type), radio := array{"0 - Single DMG", "1 - Area DMG", "2 - Token Balls"} });
      input_list.append(struct{ "title" := "Qual dura��o?", "marked" := duration });
      input_list.append(struct{ "title" := "Qual a porcentagem de poder em dano?", "marked" := dmg_dice });
      input_list.append(struct{ "title" := "Qual o tipo de dano?", "marked" := state.dmg_type in dmgs, "radio" := dmgs });
      input_list.append(struct{ "title" := "Qual gr�fico de ball?", "marked" := ""+cint(state.graphic) });
      input_list.append(struct{ "title" := "Qual cor do ball?", "marked" := ""+cint(state.color) });
      input_list.append(struct{ "title" := "Qual o som de impacto?", "marked" := ""+cint(state.sound) });
      input_list.append(struct{ "title" := "Qual o efeito da ball", "marked" := ""+cint(state.effect) });
      input_list.append(struct{ "title" := "Qual gr�fico de sub ball? (type 3)", "marked" := ""+cint(state.subgraphic) });
      input_list.append(struct{ "title" := "Qual cor do sub ball? (type 3)", "marked" := ""+cint(state.subcolor) });
      input_list.append(struct{ "title" := "Dist�ncia para efeito target (all, around, cone, inline...) ?", "marked" := ""+cint(state.conedist)});
      var output := QuestionsGump(who, input_list, HIDDEN_CANCEL);

      state.target     := output[1];
      state.create_at  := output[2];
      state.to_where   := output[3];
      state.ball_type  := cint(SplitWords(output[4])[1]);
      state.duration   := cint(output[5]);
      state.dmg_dice   := cint(output[6]);
      state.dmg_type   := output[7];
      state.graphic    := cint(output[8]);
      state.color      := cint(output[9]);
      state.sound      := cint(output[10]);
      state.effect   := cint(output[11]);
      state.subgraphic := cint(output[12]);
      state.subcolor   := cint(output[13]);
      state.conedist   := cint(output[14]);
   endif

   return state;
endfunction

*/

function DFFindElementEx(byref file_ref, elem_name, create:=DF_NO_CREATE)
	var temp := file_ref.FindElement(elem_name);
	if ( !temp && create )
		DF_OutPut("Debug::DFFindElement() - Creating elem: "+CStr(elem_name));
		file_ref.CreateElement(elem_name);
		temp := file_ref.FindElement(elem_name);
	endif

	if ( temp )
		return temp;
	elseif (create)
		var errmsg := error{"errortext":="Error::DFFindElement() - Could not open data elem ["+elem_name+"] - "+temp.errortext};
		DF_OutPut(errmsg.errortext);
		return errmsg;
	endif
endfunction