include "include/string";

function execgenerichit(attacker, defender, hitopt, rawdamage)

//printtextabove(attacker, " " + hitopt.vitals);
	//slayer entra aqui
	foreach vital in (hitopt.vitals)
		var dmg := rawdamage*hitopt.dmg;
		if (hitopt.area)
			foreach mob in (ListMobilesNearLocation( attacker.x, attacker.y, attacker.z, hitopt.area, attacker.realm))
				if ( !(mob in attacker.party.members) )
					if (vital == "Hits")
						DamageFLS(defender, dmg, hitopt.dmgtype, attacker);
					elseif (vital == "Stam")
						AP_ConsumeVital(defender, STAMINA, dmg);
					elseif (vital == "Mana")
						AP_ConsumeVital(defender, MANA, dmg);
					endif
				endif
			endforeach
		else

   //   printtextabove(attacker, " " + vital);

			if (vital == "Hits")
				var hitdmg := dmg;
				if (defender.npctemplate)
					var npccfg := NPC_GetNPCConfig(defender.npctemplate);
					var category := GetConfigString(npccfg, "Category");
      //    printtextabove(attacker, " " + " " + hitopt.slayer + " "+ hitdmg);
					if (hitopt.slayer ==  category)
						hitdmg := hitdmg * 2;
					endif
				endif
        //printtextabove(attacker, " "  + " " + hitopt.slayer + " "+ hitdmg);
				DamageFLS(defender, hitdmg, DMG_FORCED, attacker);
			elseif (vital == "Stam")
				AP_ConsumeVital(defender, STAMINA, dmg);
			elseif (vital == "Mana")
				AP_ConsumeVital(defender, MANA, dmg);
			endif
		endif
	endforeach

	if (hitopt.som != error)
		PlaySoundEffect(defender, hitopt.som);
	endif

	if (hitopt.textatacante != error)
		PrintText(attacker, hitopt.textatacante);
	endif

	if (hitopt.textdefensor != error)
		PrintText(defender, hitopt.textdefensor);
	endif

	if (hitopt.fxatacante != error)
		PlayObjectCenteredEffect(attacker, hitopt.fxatacante[1], hitopt.fxatacante[2], hitopt.fxatacante[3]);
	endif

	if (hitopt.fxdefensor != error)
		PlayObjectCenteredEffect(defender, hitopt.fxdefensor[1], hitopt.fxdefensor[2], hitopt.fxdefensor[3]);
	endif

	//POL DE MERDA
	var tname := "";
	var i := 1;
	var j := 1;
	while(i <= len(hitopt.timername))
		if (hitopt.timername != " ")
			tname[j] := hitopt.timername[i];
			j := j + 1;
		endif
		i := i + 1;
	endwhile
	if(hitopt.timername != error)
//		var timername := RemoveSpaces(hitopt.timername, CLR_ALL_SPACES);
		//sendsysmessage(attacker, "a" + tname  + "defender " + defender);
		if (tname == "bleed")
			TS_StartTimer(defender, "bleed", 40, 5, attacker);
		else
			var e := TS_StartTimer(defender, tname, cint(hitopt.timerduration), cint(hitopt.level) );
		endif
	//	sendsysmessage(attacker, " " + e);
	endif

	if (hitopt.createitem != error)
		var x , y, z;
		if (hitopt.itemloc == "Atacante")
			x := attacker.x;
			y := attacker.y;
			z := attacker.z;
		elseif (hitopt.itemloc == "Defensor")
			x := defender.x;
			y := defender.y;
			z := defender.z;
		elseif (hitopt == "Random")
			x :=  (attacker.x - 3 )+ RandomInt(7 );
			y :=  (attacker.y - 3 )+ RandomInt(7 );
			z := GetWorldHeight(x, y, attacker.realm);
		endif
		CreateItemAtLocation(x, y, z, hitopt.createitem, 1, attacker.realm);
	endif

endfunction

function execonkill(attacker, defender)

  var weapon := attacker.weapon;
  var opts := GetObjProperty(weapon, "generichit");
  if (opts == error)//ve no npc
    opts := GetObjProperty(attacker, "generichit");
  endif
  if (opts == error)
    return;
  endif

  var onkill := opts.onkill;


  if (onkill.text != "")
    printtext(defender, onkill.text);
  endif
  if (onkill.fxmorto != error)
    PlayObjectCenteredEffect(defender, onkill.fxmorto[1], onkill.fxmorto[2], onkill.fxmorto[3]);
  endif
  if (onkill.sound != error)
    PlaySoundEffect(defender, onkill.som);
  endif
  if (cint(onkill.mana) > 0)
    HealMana(attacker, onkill.mana);
  endif
  if (onkill.stamina != error)
    HealStamina(attacker, onkill.stamina);
  endif
  if (onkill.hits != error)
    HealDamage(attacker, onkill.hits);
    RecalcVitals(attacker);
  endif
  if (onkill.itemcreate != error)
    CreateItemAtLocation(defender.x, defender.y, defender.z, onkill.itemcreate, onkill.itemamt, attacker.realm);
  endif
  if (onkill.lootcreate != error)
    var itm := CreateItemInContainer(defender, onkill.lootcreate, onkill.lootamt);
    if (itm  == error)
      printText(defender, " " + itm);
    endif
  endif
  if (onkill.destroy != error)
    DestroyItem(defender);
  endif

endfunction

function configurehitscript(who)

	var options := CheckBoxGump(who, 300, 400, "Escolha as configuracoes do hitscript", array{"TimedScript", "Bonus Dano", "Dano Area", "Efeito Grafico", "Slayer", "Som", "Texto", "Criar Item", "OnKill"});

	var  hitopt := struct;
	foreach option in options
		if (option == "Bonus Dano") //stamina, mana
			var dmg := cdbl(RequestGump(who, "Quanto do rawdmg  da arma vai ser aumentado (de 0.1 a 10.0)?", "", "", 1));
			var dmgtype := RequestGump(who, "Digite qual o tipo de dano: Phisical, Fire, Cold, Poison, Energy ", "", "", 1);
			hitopt.+dmg := dmg;
			hitopt.+dmgtype := dmgtype;
			var vitals := CheckBoxGump(who, 200, 300, "Qual tipo de vital sera afetado pelo dano?", array{"Stam", "Hits", "Mana"});
			hitopt.+vitals := vitals;
		elseif  (option == "Dano Area")
			var area := Cint(RequestGump(who, "Qual a area do dano?", "", "", 1));
			hitopt.+area := area;
		elseif (option == "Som")
			var som := cint(RequestGump(who, "Escreva o id do som?", "", "", 1));
			hitopt .+som := som;
		elseif (option == "Texto")
			var textatacante := RequestGump(who, "Qual msg  vai aparecer no atacante?", "", "", 1);
			var textdefensor := RequestGump(who, "Qual msg  vai aparecer no defensor?", "", "", 1);
			if (textatacante != "")
				hitopt.+textatacante := textatacante;
			endif
			if (textdefensor != "")
				hitopt.+textdefensor := textdefensor;
			endif
		elseif (option  == "Efeito Grafico")
			var fxatacante := RequestGump(who, "Qual o efeito no atacante (id, speed, loop?", "", "", 1);
			var fxdefensor := RequestGump(who, "Qual o efeito no defensor (id, speed, loop?", "", "", 1);
			if (fxatacante != "")
				hitopt.+fxatacante := splitwords(fxatacante);
			endif
			if (fxdefensor != "")
				hitopt.+fxdefensor := splitwords(fxdefensor);
			endif
		elseif (option == "Criar Item")
			var where :=  CheckBoxGump(who, 200, 400, "Onde o item sera criado", array{"Atacante", "Defensor", "Random"});
			var createitem := RequestGump(who, "Qual o item a ser criado?", "", "", 1);
			hitopt.+createitem := createitem;
			hitopt.+itemloc := splitwords(where);
		elseif (option == "TimedScript")
			var timername := CheckBoxGump(who, 300, 500, "Escolha o timedscript", array{"defaultPoison", "hallucination", "Paralyze", "clumsy", "feeblemind", "weaken", "atrasarmagias", "fugademana", "staminaPoison", "manaPoison", "bleed", "darkborer"});

			hitopt.+timername := timername;
			hitopt.+timerduration := cint(RequestGump(who, "Qual a duracao do timer?", "Nunca mais de 120s", "", 1));
			hitopt.+level := cint(RequestGump(who, "Qual o level do timer?", "", "", 1));
		elseif (option == "OnKill")

      var onkillopts := CheckBoxGump(who, 200, 400, "Escolha as configuracoes para onkill", array{"Texto no corpo", "Som", "Efeito no corpo", "Destruir corpo", "Curar Mana", "Curar Stamina", "Curar Hits", "Criar Item", "Criar Item Loot"});
          var killopts := struct;
          foreach op in onkillopts
            if (op == "Texto no corpo")
              killopts.+text := RequestGump(who, "Qual a msg no corpo?", "", "", 1);
            elseif (op == "Som")
              killopts.+sound := cint(RequestGump(who, "Qual o id do som no corpo?", "", "", 1));
            elseif (op == "Efeito no corpo")
              var fxdefensor := splitwords(RequestGump(who, "Qual o efeito no corpo (id, speed, loop?", "", "", 1));
              killopts.+fxmorto := fxdefensor;
            elseif (op == "Destruir corpo")
              killopts.+destroy := 1;
            elseif (op == "Curar Mana")
              killopts.+mana := cint(RequestGump(who, "Quanto de mana sera curado?", "", "", 1));
            elseif (op == "Curar Stamina")
              killopts.+stamina := cint(RequestGump(who, "Quanto de stamina sera curado?", "", "", 1));
            elseif (op == "Curar Hits")
              killopts.+hits := cint(RequestGump(who, "Quanto de hits sera curado?", "", "", 1));
            elseif (op == "Criar Item")
              killopts.+itemcreate := cint(RequestGump(who, "Qual o id do item?", "", "", 1));
              killopts.+itemamt := cint(RequestGump(who, "Qual a quantidade do item?", "", "", 1));
            elseif (op == "Criar Item Loot")
              killopts.+lootcreate := cint(RequestGump(who, "Qual o id do item?", "", "", 1));
              killopts.+lootamt := cint(RequestGump(who, "Qual a quantidade do item?", "", "", 1));
            endif
          endforeach
          hitopt.+onkill := killopts;
		elseif (option == "Slayer")
			hitopt.+slayer := RequestGump(who, "Qual o tipo de monstro (veja as aterogias no .createnpc)?", "", "", 1);
		endif
	endforeach

	hitopt.+chance:= cint(RequestGump(who, "Digite a chance  do hitscript acontecer (1 a 100)?", "", "", 1));

	return hitopt;
endfunction
