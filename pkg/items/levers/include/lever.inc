const ONLY_ON := 0x11;
use uo;
use os;
use cfgfile;
include ":traps:traps";
include "include/arrays";
include ":tn:boss";
include ":combat:hitscripts";
include ":levers:efeitos";
include ":tn:cooldown";
include "include/say";
include ":destroyable:include/destroyItem";

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
	    // Para desativação com ordem: apenas verificar se a primeira alavanca da ordem está sendo desativada
         if (GetObjProperty(lever, "status") == "off")
            var first_lever_serial := order[1];
            if (lever.serial == first_lever_serial)
               // Se a primeira alavanca da ordem foi desativada, reseta todas
               foreach serial in order
                  var item := SystemFindObjectBySerial(serial);
                  if (GetObjProperty(item, "status") == "on")
                     ChangeLeverStatus(serial);
                  endif
                  sleepms(2);
               endforeach
               return 1; // Permite que os efeitos sejam executados para reset
            else
               // Se não é a primeira alavanca, apenas desativa ela sem executar efeitos
               return 0;
            endif
         endif

         // Verificar ordem específica
         foreach serial in order
            var item := SystemFindObjectBySerial(serial);
            if (item.serial == lever.serial)
               if (_serial_iter == order.size())
                  break;
               endif
               return 0;
            endif

            if (GetObjProperty(item, "status") != "on")
               ChangeLeverStatus(lever.serial);
               PrintText(lever, "*desativa*");
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

   if (opts.all_levers)
      if (!CheckAllLevers(item, lever_list, opts)) 
         SendSysMessageEx(who, "Você não percebe nada de diferente", SSM_FAIL);
         if (RunTraps(item, who)) //se estiver errado, vai cair no outro runtraps
            SetObjProperty(item, "trapSpoted", 1);
            PrintText(who, "*dispara armadilha*"); 
         endif
         return;
      endif
      lever_list := array{item.serial};
   else
      ChangeLeverStatus(lever_list);
   endif
   
   if (RunTraps(item, who))
      SetObjProperty(item, "trapSpoted", 1);
      PrintText(who, "*dispara armadilha*");
      ChangeLeverStatus(lever_list);
      return;
   endif

   var msg := opts.ontext;
   if(!msg)
      msg := "Você ouve algo se movendo...";
   endif
   var chars := ListmobilesNearLocation(item.x, item.y, item.z, 10, item.realm);
	foreach char in chars
		sleepms(2);
		SendSysMessageEx(char, msg, SSM_INFO);
	endforeach

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
        PrintText(item, "=== INICIO DEBUG ===");
        
        var tp_all_raw := GetObjProperty(item, "teleport_all");
        PrintText(item, "tp_all raw: " + tp_all_raw);
        
        var tp_all := Cint(tp_all_raw);
        PrintText(item, "tp_all convertido: " + tp_all);
        
        if (tp_all != 0)
            PrintText(item, "Entrando no tp_all, range: " + tp_all);
            
            var mobiles_list := ListMobilesNearLocation(item.x, item.y, item.z, tp_all);
            PrintText(item, "Total de mobiles: " + mobiles_list.size());
            
            foreach mobile in mobiles_list
                PrintText(item, "=== MOBILE DEBUG ===");
                PrintText(item, "Nome: " + mobile.name);
                PrintText(item, "Account: " + mobile.acctname);
                PrintText(item, "Account valido: " + (mobile.acctname != error));
                
                if(!mobile.acctname)
                    PrintText(item, "ACCOUNT INVALIDO, PULANDO");
                    continue;
                endif
                
                PrintText(item, "TELEPORTANDO: " + mobile.name);
                teleporter(mobile, item, opts);
                sleepms(2);
            endforeach
        else
            PrintText(item, "tp_all = 0, teleportando só o usuário");
            teleporter(who, item, opts);
        endif
        
        PrintText(item, "=== FIM DEBUG ===");
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
 var cfg := GetObjProperty(item, "configboss");
 var text_above := GetObjProperty(item, "txtabove");
 var creature;
		if (GetObjProperty(item, "loc") ==1 )
			creature := Createboss(cfg.boss, item.x, item.y, item.z, item.realm);
			SetObjProperty(creature, "anchor", array{item.x, item.y, item.z});
			sleepms(20);
			if (!text_above)
			else
			PrintTextAbove(creature,""+text_above);
			endif
		else
			var x := GetObjProperty(item, "x");
			var y := GetObjProperty(item, "y");
			var z := GetObjProperty(item, "z");
			creature := Createboss(cfg.boss, cint(x), cint(y), cint(z), item.realm);
			SetObjProperty(creature, "anchor", array{item.x, item.y, item.z});
			sleepms(20);
			if (!text_above)
			else
				PrintTextAbove(creature,""+text_above);
			endif
		endif
 SetObjProperty(item, "champ", creature.serial);
 EraseObjProperty(item, "#inuse");
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

function stobj(who, item)

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
   Detach();
 
      var x := GetObjProperty(item, "x_tp");
		var y := GetObjProperty(item, "y_tp");
		var z := GetObjProperty(item, "z_tp");
      var realm := GetObjProperty(item, "realm_tp");
   
   MoveObjectToLocation(who, Cint(x), Cint(y), Cint(z), realm, MOVEOBJECT_FORCELOCATION);
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
      return 0; // Não tem itens configurados
   endif
   
   // MUDANÇA 1: Verificar se tem todos os itens ANTES de iniciar o processo
   var missing_items := array{};
   foreach req_item in required_items
      var found_amount := 0;
      foreach backpack_item in EnumerateItemsInContainer(who.backpack)
         if (backpack_item.objtype == req_item.objtype)
            found_amount += backpack_item.amount;
         endif
         sleepms(2);
      endforeach
      
      if (found_amount < req_item.amount)
         missing_items.append("" + req_item.amount + "x " + req_item.desc + " (você tem " + found_amount + ")");
      endif
   endforeach
   
   // Se faltam itens, para aqui
   if (missing_items.size() > 0)
      SendSysMessageEx(who, "Você não possui todos os itens necessários:", SSM_FAIL);
      foreach missing in missing_items
         SendSysMessage(who, "- " + missing);
         sleepms(2);
      endforeach
      return 0;
   endif
   
   var selected_items := array{}; // MUDANÇA 2: Guarda itens selecionados SEM consumir
   var required_copy := array{};
   
   // Criar uma cópia dos itens necessários para controle
   foreach req_item in required_items
      required_copy.append(struct{
         "objtype" := req_item.objtype,
         "amount" := req_item.amount,
         "desc" := req_item.desc,
         "delivered" := 0
      });
   endforeach
   
   var continue_delivery := 1;
   while (continue_delivery)
      SendSysMessage(who, "Selecione um item para entregar ou ESC para cancelar:");
      var target_item := Target(who);
      
      if (!target_item)
         SendSysMessage(who, "Entrega cancelada.");
         return 0;
      endif
      
      if (target_item.container != who.backpack)
         SendSysMessage(who, "O item deve estar no seu inventário.");
         continue;
      endif
      
      // Verificar se o item é um dos necessários
      var item_found := 0;
      foreach req_item in required_copy
         if (req_item.objtype == target_item.objtype && req_item.delivered < req_item.amount)
            var amount_needed := req_item.amount - req_item.delivered;
            var amount_to_take := target_item.amount;
            
            if (amount_to_take > amount_needed)
               amount_to_take := amount_needed;
            endif
            
            // Verificar se o jogador tem quantidade suficiente
            if (target_item.amount >= amount_to_take)
               // MUDANÇA 3: NÃO consome ainda, apenas guarda para consumir depois
               selected_items.append(struct{
                  "item" := target_item,
                  "amount" := amount_to_take
               });
               
               req_item.delivered += amount_to_take;
               SendSysMessage(who, "Selecionou " + amount_to_take + "x " + target_item.desc);
               item_found := 1;
               break;
            else
               SendSysMessage(who, "Quantidade insuficiente.");
               item_found := 1;
               break;
            endif
         endif
      endforeach
      
      if (!item_found)
         SendSysMessageEx(who, "Este item não parece funcionar aqui", SSM_FAIL);
         continue;
      endif
      
      // Verificar se todos os itens foram selecionados
      var all_selected := 1;
      foreach req_item in required_copy
         if (req_item.delivered < req_item.amount)
            all_selected := 0;
            SendSysMessage(who, "Ainda falta: " + (req_item.amount - req_item.delivered) + "x de determinado item");
         endif
      endforeach
      
      if (all_selected)
         // MUDANÇA 4: Confirma ANTES de consumir
         var confirm := YesNo(who, "Confirma a entrega de todos os itens selecionados?");
         if (confirm)
            // Só agora consome os itens
            foreach selected in selected_items
               SubtractAmount(selected.item, selected.amount);
            endforeach
            SendSysMessage(who, "Todos os itens foram usados com sucesso!");
            return 1;
         else
            SendSysMessage(who, "Entrega cancelada.");
            return 0;
         endif
      endif
   endwhile
   
   return 0; // Não deveria chegar aqui
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
endforeach
	endif
   if (opts.moveitems)
      moveObjects(item, opts);
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
