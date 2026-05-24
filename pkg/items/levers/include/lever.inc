const ONLY_ON := 0x11;
use uo;
use os;
use cfgfile;
include ":traps:traps";
include "include/arrays";

include ":combat:hitscripts";
include ":levers:efeitos";
include ":tn:cooldown";
include "include/say";
include ":destroyable:include/destroyItem";
include ":epicspawn:epicBoss";
include ":message:include/model/Message";
include ":propagacao:include/efeitosPatogenicos";
include "include/epicCombatUtils";
include "mockmultiUtils";

function ChangeLeverStatus(la, only_on_levers := 0)
   if (typeof(la) != "Array")
      la := array{la};
   endif
    
   var mid := 0;
   var fin, snd, obj, gfx;
   foreach entry in la
      obj := SystemFindObjectBySerial(CInt(entry));
      var status := GetObjProperty(obj, "status");
      if (only_on_levers && status != "on")
         continue;
      endif
      gfx := obj.graphic;
      mid := 0;
      case(gfx)
         0x108c: mid := 0x108d;
               fin := 0x108e;
               snd := 0x4c;
         0x108e: mid := 0x108d;
               fin := 0x108c;
               snd := 0x4c;
         0x1093: mid := 0x1094;
               fin := 0x1095;
               snd := 0x4c;
         0x1095: mid := 0x1094;
               fin := 0x1093;
               snd := 0x4c;
         0x108f: fin := 0x1090;
               snd := 0x4b;
         0x1090: fin := 0x108f;
               snd := 0x4b;
         0x1091: fin := 0x1092;
               snd := 0x4b;
         0x1092: fin := 0x1091;
               snd := 0x4b;
         default:
            fin := gfx;
            mid := gfx;
      endcase

      if(mid == 0)
         mid := fin;
      endif
var som := Cint(GetObjProperty(obj, "som_inicial"));
	if(som) //ADICIONADO SOM 1
		snd := som;
	endif

      sleepms(500);
      PlaySoundEffect( obj, snd);
      obj.graphic := mid;
      sleepms(500);
      obj.graphic := fin;

      if (!status || status == "off")
         SetObjProperty(obj, "status", "on");
         SetObjProperty(obj, "reset_cd", polcore().systime+2400);
      else
         SetObjProperty(obj, "status", "off");
         EraseObjProperty(obj, "reset_cd");
      endif
   endforeach
endfunction

function SetRandomorder(item)
   var opts := item.GetLeverOpts();
   if (opts.linked_to && opts.linked_to.size() && opts.all_levers_order)
      var randomorder := array{};
      var leverarray := array{item.serial} + opts.linked_to;
      for i := 1 to (leverarray.size())
         var choosen_one := leverarray.randomentry();
         randomorder.append(choosen_one);
         var index := FindInArray(leverarray, choosen_one);
         leverarray.erase(index);
         sleepms(2);
      endfor
      SetObjProperty(item, "leverorder", randomorder);
   endif
endfunction

function CheckAllLevers(lever, lever_list, opts)
   ChangeLeverStatus(lever.serial);
   // Se há ordem específica (tanto selecionada quanto random)
   if (opts.all_levers_order == 1 || opts.all_levers_order == 2)
      var order := GetObjProperty(opts.item_ref, "leverorder");
      if (!order)
         // Se não há ordem definida, tratar como sem ordem
         var all_levers := 0;
         foreach item in (lever_list)
            item := SystemFindObjectBySerial(item);
            if (GetObjProperty(item, "status") == "on")
               all_levers += 1;
            endif
            sleepms(2);
         endforeach

         if (GetObjProperty(lever, "status") == "off" && all_levers == lever_list.size()-1)
            return 1;
         elseif (all_levers != lever_list.size())
            return 0;
         endif
      else
         // Desativação: qualquer alavanca desativada reseta TODAS e não executa efeitos
         if (GetObjProperty(lever, "status") == "off")
            foreach serial in order
               var item := SystemFindObjectBySerial(serial);
               if (item && GetObjProperty(item, "status") == "on")
                  ChangeLeverStatus(serial);
               endif
               sleepms(2);
            endforeach
            return 0;
         endif

         // Verificar ordem específica (ativação)
         foreach serial in order
            var item := SystemFindObjectBySerial(serial);
            if (item.serial == lever.serial)
               if (_serial_iter == order.size())
                  break;
               endif
               return 0;
            endif

            if (GetObjProperty(item, "status") != "on")
               // Ordem errada! Desativar TODAS as alavancas ativadas
               ChangeLeverStatus(lever.serial);
               foreach reset_serial in order
                  var reset_item := SystemFindObjectBySerial(reset_serial);
                  if (reset_item && GetObjProperty(reset_item, "status") == "on")
                     ChangeLeverStatus(reset_serial);
                  endif
                  sleepms(2);
               endforeach
               PrintTextAbove(lever, "*desativa*");
               return 0;
            endif
            sleepms(2);
         endforeach
      endif
   else
      // Sem ordem específica - funcionalidade original
      var all_levers := 0;
      foreach item in (lever_list)
         item := SystemFindObjectBySerial(item);
         if (GetObjProperty(item, "status") == "on")
            all_levers += 1;
         endif
         sleepms(2);
      endforeach

      if (GetObjProperty(lever, "status") == "off" && all_levers == lever_list.size()-1)
         return 1;
      elseif (all_levers != lever_list.size())
         return 0;
      endif
   endif

   return 1;
endfunction

function normalusage(who, item)
   if (GetObjProperty(item, "#inuse"))
      return;
   endif

   // Check boss respawn cooldown (GMs with cmdlevel > 1 can bypass)
   if (who.cmdlevel <= 1)
      var cooldown_remaining := getCooldown(item, "boss_respawn");
      if (cooldown_remaining > 0)
         Message("Voce nao consegue mover a alavanca").SetColor(SSM_FAIL).SendSys(who);
         return;
      endif
   endif

   var status := GetObjProperty(item, "status");
   if(!status)
      status := "off";
      SetObjProperty( item, "status", status);
   endif

   var opts  := item.GetLeverOpts();
   if(!opts || !opts.keys().size())
      SendSysMessage(who,"esta alavanca parece quebrada");
      return;
   endif

   // Verificar se a alavanca requer entrega de itens
   var requires_items := GetObjProperty(item, "requires_item_delivery");
   if (requires_items && status == "off")
      if (!CheckItemDelivery(who, item))
         return; // Entrega falhou ou foi cancelada
      endif
   endif

   if (status == "off")
      PrintTextAbove(who, "*Ativa*");
   else
      PrintTextAbove(who, "*Desativa*");
			if ( GetObjProperty(item, "powerword"))
				item.invisible := 1;
			endif
   endif

   var lever_list := array{item.serial};
   if (opts.linked_to)
      lever_list := opts.linked_to;
      if (!lever_list) lever_list := array{}; endif
      lever_list.append(item.serial);
      opts.item_ref := item;
   elseif (opts.is_linked)
      var origin := SystemFindObjectBySerial(CInt(opts.is_linked));
      opts := origin.GetLeverOpts();
      opts.item_ref := origin;
      lever_list := opts.linked_to;
      if (!lever_list) lever_list := array{}; endif
      lever_list.append(origin.serial);
   endif

   // Handle sync_toggle mode - all levers act as one switch
   if (opts.sync_toggle)
      // Get the master lever to check current state
      var master := item;
      if (opts.is_linked)
         master := SystemFindObjectBySerial(CInt(opts.is_linked));
         opts := master.GetLeverOpts();
      endif

      var master_status := GetObjProperty(master, "status");

      // Toggle ALL levers in the group to the opposite state
      foreach lever_serial in lever_list
         var lever_item := SystemFindObjectBySerial(lever_serial);
         if (lever_item)
            // Set all to same state based on master
            if (!master_status || master_status == "off")
               SetObjProperty(lever_item, "status", "on");
            else
               SetObjProperty(lever_item, "status", "off");
            endif
         endif
         sleepms(2);
      endforeach

      // Change graphic of the activated lever
      ChangeLeverStatus(item.serial);

      // Use master's opts for effects
      opts := master.GetLeverOpts();

   elseif (opts.all_levers)
      if (!CheckAllLevers(item, lever_list, opts))
         SendSysMessageEx(who, "Você não percebe nada de diferente", SSM_FAIL);
         if (RunTraps(item, who)) //se estiver errado, vai cair no outro runtraps
            SetObjProperty(item, "trapSpoted", 1);
            PrintTextAbove(who, "*dispara armadilha*");
         endif
         return;
      endif
      lever_list := array{item.serial};
   else
      ChangeLeverStatus(lever_list);
   endif
   
   if (RunTraps(item, who))
      SetObjProperty(item, "trapSpoted", 1);
      PrintTextAbove(who, "*dispara armadilha*");
      ChangeLeverStatus(lever_list);
      return;
   endif

// =====================================================
// EFEITO: NUVEM PATOGÊNICA (PROPAGAÇÃO)
// =====================================================
if (GetObjProperty(item, "LEVER_PATOGENICO") && status == "off")

   // parâmetros mínimos e seguros
   var alcance := GetObjProperty(item, "patog_alcance");
   var ciclosAteExpancao := GetObjProperty(item, "patog_ciclos");
   var duracaoNuvem := GetObjProperty(item, "patog_duracao_nuvem");
   var gravidadeInfeccao := GetObjProperty(item, "patog_gravidade");
   var arraySintomas := GetObjProperty(item, "patog_sintomas");
   var duracaoPatogenico := GetObjProperty(item, "patog_duracao");
   var nivelInfeccao := GetObjProperty(item, "patog_nivel");

   CriaMarcadorPatogenico(
      item.x,
      item.y,
      item.z,
      item.realm,
      who.serial,
      alcance,
      ciclosAteExpancao,
      duracaoNuvem,
      gravidadeInfeccao,
      arraySintomas,
      duracaoPatogenico,
      nivelInfeccao
   );
endif
// =====================================================


   var msg := opts.ontext;
   if(!msg)
      msg := "Você ouve algo se movendo...";
   endif
   var chars := ListmobilesNearLocation(item.x, item.y, item.z, 10, item.realm);
	foreach char in chars
		sleepms(2);
		SendSysMessageEx(char, msg, SSM_INFO);
	endforeach

   // Para levers linkadas com all_levers, usar a alavanca mestre para efeitos
   // (as configurações de efeitos ficam na mestre, não nas escravas)
   if (opts.item_ref)
      item := opts.item_ref;
   endif

   if (opts.lights)
      lightswitch(item, opts);
   endif

   if (opts.graphicarrays)
      changegraphiclever(item, opts);
   endif
   if (opts.opendoors)
      openthedoors(item, opts);
   endif
   if (opts.doorstatus)
      changedoorstatuses(item, opts);
   endif
   
   var sequence_gr_mov_items := Cint(GetObjProperty(item, "sequence"));
	if (sequence_gr_mov_items == 2)
		if (opts.moveitems)
			moveObjects(item, opts);
		endif
		if (opts.graphicarraysitem)
			changegraphici(item, opts);
		endif
	else
		if (opts.graphicarraysitem)
			changegraphici(item, opts);
		endif
		if (opts.moveitems)
			moveObjects(item, opts);
		endif
	endif
   
	if (opts.mockmulti)
		moverMockMulti(item, opts);
	endif

	if(opts.somfim)//ADICIONADO PARA SIM 2 (FINAL)
		var snd := Cint(GetObjProperty(item, "Som_fim"));
		foreach char in chars
			sleepms(2);
			PlaySoundEffect( char, snd);
		endforeach
	endif

	if (opts.spawn_boss)
		if (status == "off")
			spawnBoss(item, opts);
		else
		  var boss :=  SystemFindObjectBySerial( GetObjProperty(item, "champ") );
			ApplyDamage( boss, 20000);
			EraseObjProperty(item, "champ");
		endif
	endif
	if (opts.tremor)
		if (status == "off")
			var config_tremor := GetObjProperty(item, "tremor_cfg");
			var danobase := config_tremor[1];
			var danoale := config_tremor[2];
			var dif := config_tremor[3];
			var range := config_tremor[4];
			EfeitosTremor(item,danobase,danoale,dif,range);
		endif
	endif
   if (opts.teleport_to)
      if (status == "off")
	  var tp_all:= Cint(GetObjProperty(item, "teleport_all"));
		if (tp_all != 0)
			foreach mobile in ListMobilesNearLocation(item.x, item.y, item.z, tp_all, item.realm)
				if(!mobile.acctname)
					continue;
				endif
				teleporter(mobile, item, opts);
				sleepms(2);
			endforeach
		else
			teleporter(who, item, opts);
		endif
	  endif
   endif
   if (opts.clima) //SEMPRE deixar essa opção para ativar por último, ela é mais longa
		if (status == "off")
			var config_clima := GetObjProperty(item, "clima_cfg");
			var duration := config_clima[1];
			var range := config_clima[2];
			var options := GetObjProperty(item, "clima");
			EfeitosClima (item, options, duration, range);
		endif
   endif

endfunction

function spawnBoss(item, opts)
   SetObjProperty(item, "#inuse", 1);
   Detach();

   // Get the boss configuration saved by epicboss command
   var form_data := GetObjProperty(item, "form_data");
   if (!form_data)
      // Legacy fallback: old levers stored just a template name in `configboss.boss`
      // via the deprecated `Createboss` GM command (now removed). Wrap it in the
      // minimal struct CreateEpicBoss expects (just npctemplate) so existing
      // world data still spawns. New levers use the full form_data path below.
      var cfg := GetObjProperty(item, "configboss");
      if (cfg && cfg.boss)
         var text_above := GetObjProperty(item, "txtabove");
         var spawn_x, spawn_y, spawn_z;
         if (GetObjProperty(item, "loc") == 1)
            spawn_x := item.x; spawn_y := item.y; spawn_z := item.z;
         else
            spawn_x := CInt(GetObjProperty(item, "x"));
            spawn_y := CInt(GetObjProperty(item, "y"));
            spawn_z := CInt(GetObjProperty(item, "z"));
         endif

         var creature := CreateEpicBoss(struct{ "npctemplate" := cfg.boss }, spawn_x, spawn_y, spawn_z, item.realm);

         if (creature && creature != error)
            SetObjProperty(creature, "anchor", array{item.x, item.y, item.z});
            SetObjProperty(creature, "spawned_by_lever", item.serial);
            if (text_above)
               Message(""+text_above).SendAbove(creature);
            endif
            SetObjProperty(item, "champ", creature.serial);
            // Keep #inuse - don't erase it
         else
            // Spawn failed, unlock
            EraseObjProperty(item, "#inuse");
         endif
      else
         // No config at all, unlock
         EraseObjProperty(item, "#inuse");
      endif
      return;
   endif

   // Determine spawn location
   var spawn_x, spawn_y, spawn_z;
   if (GetObjProperty(item, "spawn_at_lever"))
      spawn_x := item.x;
      spawn_y := item.y;
      spawn_z := item.z;
   else
      spawn_x := CInt(GetObjProperty(item, "spawn_x"));
      spawn_y := CInt(GetObjProperty(item, "spawn_y"));
      spawn_z := CInt(GetObjProperty(item, "spawn_z"));

      // Fallback to lever position if no spawn coordinates
      if (!spawn_x || !spawn_y)
         spawn_x := item.x;
         spawn_y := item.y;
         spawn_z := item.z;
      endif
   endif

   // Create the epic boss using the modern system
   var creature := CreateEpicBoss(form_data, spawn_x, spawn_y, spawn_z, item.realm);

   if (creature && creature != error)
      // Set anchor to lever location
      SetObjProperty(creature, "anchor", array{item.x, item.y, item.z});

      // Link boss to lever for death cleanup
      SetObjProperty(creature, "spawned_by_lever", item.serial);

      // Show text above if configured
      var text_above := GetObjProperty(item, "spawn_textabove");
      if (text_above && text_above != "")
         Message(""+text_above).SendAbove(creature);
      endif

      // Save creature serial for reset/deactivation
      SetObjProperty(item, "champ", creature.serial);

      // Keep #inuse set - lever stays locked until boss dies
      // This prevents spam spawning and instant kill via lever toggle
   else
      // Spawn failed, unlock the lever
      EraseObjProperty(item, "#inuse");
   endif
endfunction

function changegraphiclever(item, opts)
  SetObjProperty(item, "#inuse", 1);
  EraseObjProperty(item, "#inuse");
endfunction

function changegraphici(item, opts)
  SetObjProperty(item, "#inuse", 1);

  var serial;
  foreach serial in  (opts.graphicarraysitem)
     serial := SystemFindObjectBySerial(CInt(serial));
   var name_novo := GetObjProperty(serial, "new_name");
  var name_antigo := GetObjProperty(serial, "old_name");
     var stat :=  GetObjProperty(item, "status");
     if ( (stat == error) || (stat == "on"))
        serial.graphic := GetObjProperty(serial, "graphicto");
        if (name_novo)
			serial.name := name_novo;
		endif
		//setobjproperty(item, "status", "unreleased");
     else
        serial.graphic := GetObjProperty(serial, "originalgraphic");
		if (name_antigo)
			serial.name := name_antigo;
		endif
        //setobjproperty(item, "status", "released");
     endif
    
  endforeach

  EraseObjProperty(item, "#inuse");
endfunction

function moveObjects(item, opts)
   SetObjProperty(item, "#inuse", 1);
   Detach();
   
   var move := opts.movefinal;
   move[1] := cint(move[1]);
   move[2] := cint(move[2]);
   move[3] := cint(move[3]);
   if (GetObjProperty(item, "status") == "off")
      move[1] *= -1;
      move[2] *= -1;
      move[3] *= -1;
   endif

   for i := 1 to (opts.movetime)
      foreach item in (opts.moveitems)
         if (!item.isA(POLCLASS_UOBJECT))
            item := SystemFindObjectBySerial(cint(item));
         endif
		   MoveObjectToLocation(item, item.x+move[1], item.y+move[2], item.z+move[3], item.realm, MOVEOBJECT_FORCELOCATION+MOVEITEM_IGNOREMOVABLE);
         sleepms(2);
      endforeach
      sleep(1);
   endfor

   EraseObjProperty(item, "#inuse");
endfunction

function teleporter(who, item, opts)
   SetObjProperty(item, "#inuse", 1);

   var x := Cint(GetObjProperty(item, "x_tp"));
   var y := Cint(GetObjProperty(item, "y_tp"));
   var z := Cint(GetObjProperty(item, "z_tp"));
   var realm := GetObjProperty(item, "realm_tp");

   var pets := ListAllPetsAndSummonedPetsNear(who, 4);

   Detach();

   SetObjProperty(who, "#Teleported", ReadMillisecondClock());
   MoveObjectToLocation(who, x, y, z, realm, MOVEOBJECT_FORCELOCATION);

   foreach pet in pets
      SetObjProperty(pet, "#Teleported", ReadMillisecondClock());
      MoveObjectToLocation(pet, x, y, z, realm, MOVEOBJECT_FORCELOCATION);
      sleepms(10);
   endforeach

   //ChangeLeverStatus(opts.linked_to);
   EraseObjProperty(item, "#inuse");
endfunction

function changedoorstatuses(item, opts)
   SetObjProperty(item, "#inuse", 1);
   Detach();

   var set_prop := 1;
   var snd_effect := 0x1f5;
   if (GetObjProperty(item, "status") == "on")
      set_prop := 0;
      snd_effect := 0x200;
   endif

   foreach item in (opts.doorstatus)
      item := SystemFindObjectBySerial(cint(item));
      item.locked := set_prop;
      PlaySoundEffect(item, snd_effect);
      sleepms(2);
   endforeach
   
   EraseObjProperty(item, "#inuse");
endfunction

function openthedoors(item, opts)
   SetObjProperty(item, "#inuse", 1);
   Detach();    
   var set_prop := 1;
   var snd_effect := 0x1f5;
   var door_open := 0;
   if (GetObjProperty(item, "status") == "on")
      set_prop := 0;
	  door_open := 1;
	  snd_effect := 0x200;
   endif

   foreach item in (opts.opendoors)
      item := SystemFindObjectBySerial(cint(item));
	  if (door_open == 1)
		item.Open();
	  else
		item.close();
	  endif
	  item.locked := set_prop;
      PlaySoundEffect(item, snd_effect);
      sleepms(2);
   endforeach

   EraseObjProperty(item, "#inuse");
endfunction

function lightswitch(item, opts)
  SetObjProperty(item, "#inuse", 1);
   Detach();


  foreach serial in (opts.lights)
  serial := SystemFindObjectBySerial(cint(serial));
    var cfg := ReadConfigFile(":*:itemdesc");
  var elem := FindConfigElem(cfg, serial.objtype);
  if(serial.graphic == serial.objtype)
	var chan := elem.change;
    serial.graphic := chan;
    setobjproperty(serial, "graphic", chan);
  else
	serial.graphic := serial.objtype;
    setobjproperty(serial, "graphic", serial.graphic);
  endif
  endforeach
  
   EraseObjProperty(item, "#inuse");
endfunction

function CheckItemDelivery(who, item)
   var required_items := GetObjProperty(item, "required_items");
   if (!required_items || !required_items.size())
      return 0;
   endif

   // Mapear itens encontrados na bag para consumir depois
   var items_to_consume := array{};  // {item, amount}
   var missing_items := array{};

   foreach req_item in required_items
      sleepms(2);
      var amount_needed := req_item.amount;
      var items_for_this_req := array{};

      // Procurar todos os itens desse tipo na bag
      foreach backpack_item in EnumerateItemsInContainer(who.backpack)
         if (backpack_item.objtype == req_item.objtype && amount_needed > 0)
            var amount_from_this := backpack_item.amount;
            if (amount_from_this > amount_needed)
               amount_from_this := amount_needed;
            endif

            items_for_this_req.append(struct{
               "item" := backpack_item,
               "amount" := amount_from_this
            });
            amount_needed -= amount_from_this;
         endif
         sleepms(2);
      endforeach

      if (amount_needed > 0)
         var found_amount := req_item.amount - amount_needed;
         missing_items.append("" + req_item.amount + "x " + req_item.desc + " (você tem " + found_amount + ")");
      else
         // Adicionar todos os itens encontrados para esse requisito
         foreach item_entry in items_for_this_req
            items_to_consume.append(item_entry);
            sleepms(2);
         endforeach
      endif
   endforeach

   // Se faltam itens, mostra o que falta
   if (missing_items.size() > 0)
      SendSysMessageEx(who, "Você não possui todos os itens necessários:", SSM_FAIL);
      foreach missing in missing_items
         SendSysMessage(who, "- " + missing);
         sleepms(2);
      endforeach
      return 0;
   endif

   // Mostrar lista de itens que serão consumidos e pedir confirmação
   var item_list_text := "Itens que serão entregues:\n";
   foreach req_item in required_items
      item_list_text += "- " + req_item.amount + "x " + req_item.desc + "\n";
      sleepms(2);
   endforeach

   var confirm := YesNo(who, item_list_text + "\nDeseja entregar esses itens?");
   if (!confirm)
      SendSysMessageEx(who, "Entrega cancelada.", SSM_FAIL);
      return 0;
   endif

   // Consumir os itens automaticamente
   foreach consume_entry in items_to_consume
      SubtractAmount(consume_entry.item, consume_entry.amount);
      sleepms(2);
   endforeach

   SendSysMessageEx(who, "Itens entregues com sucesso!", SSM_INFO);
   return 1;
endfunction

function reset(item, force_reset := 0)
   var opts := item.GetLeverOpts();
   if ( GetObjProperty(item, "status") != "on" || GetObjProperty(item, "#inuse"))
      return;
   endif
   var reset_cd := GetObjProperty(item, "reset_cd");
   if (reset_cd && reset_cd > polcore().systime && !force_reset)
      return;
   endif
   
	if ( cint(GetObjProperty(item, "level")) > 0 || GetObjProperty(item, "powerword"))
		item.invisible := 1;
	elseif (GetObjProperty(item, "nature_event"))
		CheckNatureEvent(item);
	endif

	if ( GetObjProperty(item, "HaveTrap") && !ExistTrap(item))
      var trapstrength := CInt(GetObjProperty(item,"trap_strength"));
      if (!trapstrength) trapstrength := 2; endif
      AddTrap(item, GetObjProperty(item, "HaveTrap"), trapstrength);
	endif

   var lever_list := array{item.serial};
   if (opts.linked_to)
      lever_list := opts.linked_to;
      if (!lever_list) lever_list := array{}; endif
      lever_list.append(item.serial);
   elseif (opts.is_linked)
      var original := SystemFindObjectBySerial(opts.is_linked);
      if (GetObjProperty(original, "status") == "off")
         ChangeLeverStatus(item.serial);
      endif
      return;
   endif

   if (opts.all_levers)
      if (!CheckAllLevers(item, lever_list, opts))
         return;
      endif
      lever_list.Erase(lever_list.size());
      ChangeLeverStatus(lever_list, ONLY_ON);
   else
      ChangeLeverStatus(lever_list, ONLY_ON);
   endif


   if (opts.lights)
      lightswitch(item, opts);
   endif
   if (opts.graphicarrays)
      changegraphiclever(item, opts);
   endif
   if (opts.opendoors)
      openthedoors(item, opts);
   endif
   if (opts.doorstatus)
      changedoorstatuses(item, opts);
   endif
	if (opts.graphicarraysitem)
		  var serial;
  foreach serial in  (opts.graphicarraysitem)
     serial := SystemFindObjectBySerial(CInt(serial));
         serial.graphic := GetObjProperty(serial, "originalgraphic");
         var name_antigo := GetObjProperty(serial, "old_name");
         if (name_antigo)
            serial.name := name_antigo;
         endif
endforeach
	endif
   if (opts.moveitems)
      moveObjects(item, opts);
   endif
   if (opts.mockmulti)
      moverMockMulti(item, opts);
   endif
	if (opts.spawn_boss)
		  var boss :=  SystemFindObjectBySerial( GetObjProperty(item, "champ") );
			ApplyDamage( boss, 20000);
			EraseObjProperty(item, "champ");
	endif
   // Resetar ordem se for random
   if (opts.all_levers_order == 2)
      SetRandomOrder(item);
   endif
endfunction

function moverMockMulti(item, opts)
   var mm := opts.mockmulti;
   var gdata := GetGlobalProperty("mmGroup_" + mm.groupId);
   if (!gdata || gdata.errortext) return; endif

   var serials := gdata.serials;
   if (!serials || serials.size() == 0) return; endif

   var newStatus := GetObjProperty(item, "status");
   var z_destino := newStatus == "on" ? mm.z_up : mm.z_down;
   var shift     := z_destino - BaseZAtual(serials);
   if (shift == 0) return; endif

   var cloud_serials := gdata.cloud_serials ? gdata.cloud_serials : array{};
   if (z_destino > CLOUD_Z_THRESHOLD && cloud_serials.size() == 0)
      cloud_serials := _CriarNuvens(serials);
   endif
   var extras := cloud_serials + (mm.corda_volta ? array{mm.corda_volta} : array{});

   Detach();
   _AnimarSerials(serials + extras, shift, VelStep(mm.vel), VelDelay(mm.vel));

   if (z_destino <= CLOUD_Z_THRESHOLD)
      _DestruirNuvens(cloud_serials);
      cloud_serials := array{};
   endif
   _AtualizarDestino(mm.corda_subir, shift);
   gdata.cloud_serials := cloud_serials.size() > 0 ? cloud_serials : 0;
   SetGlobalProperty("mmGroup_" + mm.groupId, gdata);
endfunction

function CheckNatureEvent(item)
   var nature_event := GetObjProperty(item, "nature_event");
   
   var current_day := GetGlobalProperty("dia");
   var current_moon := getMoonPhase();
   var current_season := GetSeasonName(getSeason());
   
   var nature_global := {current_day, current_moon, current_season};
	if (nature_event in nature_global)
		item.invisible := 0;
	else
		item.invisible := 1;
	endif
endfunction