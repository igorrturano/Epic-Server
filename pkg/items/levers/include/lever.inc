const ONLY_ON := 0x11;

include ":traps:traps";
include "include/arrays";
include ":tn:boss";
include ":combat:hitscripts";
include ":levers:efeitos";

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
   if (opts.all_levers_order)
      var order := GetObjProperty(opts.item_ref, "leverorder");
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
   else
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
   if (opts.moveitems)
      moveObjects(item, opts);
   endif
   if (opts.teleport_to)
      teleporter(who, item, opts);
   endif
   	if (opts.graphicarraysitem)
		changegraphici(item, opts);
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
     var stat :=  GetObjProperty(item, "status");
     if ( (stat == error) || (stat == "on"))
        serial.graphic := GetObjProperty(serial, "graphicto");
        //setobjproperty(item, "status", "unreleased");
     else
        serial.graphic := GetObjProperty(serial, "originalgraphic");
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
   
   MoveObjectToLocation(who, Cint(x), Cint(y), Cint(z), item.realm, MOVEOBJECT_FORCELOCATION);
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
   
   var snd_effect := 0x1f5;
   if (GetObjProperty(item, "status") == "on")
      snd_effect := 0x200;
   endif

   foreach item in (opts.opendoors)
      item := SystemFindObjectBySerial(cint(item));
      item.doToggle();
      PlaySoundEffect(item, snd_effect);
      sleepms(2);
   endforeach

   EraseObjProperty(item, "#inuse");
endfunction

function lightswitch(item, opts)
   SetObjProperty(item, "#inuse", 1);
   Detach();
   
   EraseObjProperty(item, "#inuse");
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
   if (opts.moveitems)
      moveObjects(item, opts);
   endif
//   	if (opts.spawn_boss)
	//	spawnBoss(item, opts);
	//endif
	if (opts.graphicarraysitem)
		  var serial;
  foreach serial in  (opts.graphicarraysitem)
     serial := SystemFindObjectBySerial(CInt(serial));
         serial.graphic := GetObjProperty(serial, "originalgraphic");
endforeach
	endif
	if (opts.spawn_boss)
		  var boss :=  SystemFindObjectBySerial( GetObjProperty(item, "champ") );
			ApplyDamage( boss, 20000);
			EraseObjProperty(item, "champ");
	endif
   SetRandomOrder(item);
endfunction
