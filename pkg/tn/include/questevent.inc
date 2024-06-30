use uo;

include "include/say";
include ":merchants:item_template";
include ":tn:boss";
include ":timedscripts:timedScripts";

function ConfigureEvent(who)
	var evento := struct;
	evento.+type := RadioGump(who, 400, 220, "Escolha o tipo de evento.", array{ "Criar Itens", "Abrir Porta", "Criar Npcs", "Abrir Passagem", "Ultimas Palavras"});
	evento.+realm := Cstr(who.realm);
	print("evento real eh"+evento.realm);

	if ( (evento.type == "Abrir Porta") || (evento.type == "Abrir Passagem") )
		var items := array;

                SendSysMessage(who, "Escolha os itens. ESC para cancelar.");
                var targ := Target(who);
                if (targ)
                  evento.+items := array;
                endif
                while(targ)
                    evento.items.append(targ.serial);
                    targ := Target(who);
                endwhile
	elseif  (evento.type == "Criar Npcs" )
               /* evento.+amount := Cint(RequestGump(who, "Quantos npcs vao ser spawnados:", "", "", 1));
                var loc := RequestGump(who, "Digite o x y z onde eles vao aparecer:", "Escreva 6 6 6 se for para aparecer onde um npc morrer (bosses)", "", 1);
                loc := splitwords(loc);
                evento.+x :=  loc[1];
                evento.+y :=  loc[2];
                evento.+z :=  loc[3];
                evento.+range := Cint(RequestGump(who, "Digite o raio de spawn:", "", "", 1));
                evento.+npctemplate := RequestGump(who, "Digite o npctemplate:", "", "", 1);*/
		evento.+amount := Cint(RequestGump(who, "Quantos npcs vao ser spawnados:", "", "", 1));
		var loc := RequestGump(who, "Digite o x y z onde eles vao aparecer:", "Escreva 6 6 6 se for para aparecer onde um npc morrer (bosses)", "", 1);
		loc := splitwords(loc);
		evento.+x :=  loc[1];
		evento.+y :=  loc[2];
		evento.+z :=  loc[3];
		evento.+range := Cint(RequestGump(who, "Digite o raio de spawn:", "", "", 1));
		evento.+npctemplate := RequestGump(who, "Digite o npctemplate:", "", "", 1);
		var boss := YesNo( who, "Deseja adicionar um npc boss?" );
		if (boss)
			var bossinfo := struct;
			CreateBossStructGump(who, bossinfo);
			evento.+bossinfo := bossinfo;
		endif
		evento.+summontext := RequestGump(who, "Deseja adicionar algum texto nos mobs?", "(voce pode adicionar so no boss)", "", 1);
		evento.+summonbosstext :=  RequestGump(who, "Deseja adicionar algum texto no Boss?", "", "", 1);

	elseif ( evento.type == "Criar Itens")
		SendSysMessage(who, "Crie os itens e coloque eles no lugar que quer que aparecam. Quando o evento for disparado eles vao aparecer la.");
                var targ := Target(who);
                if (targ)
                  evento.+items := array;
                endif
                while(targ)
		    var iteminfo := struct;
		    iteminfo.+item := CreateItemTemplate(targ);
		    iteminfo.+x := targ.x;
		    iteminfo.+y := targ.y;
		    iteminfo.+z := targ.z;
                    evento.items.append(iteminfo);
                    targ := Target(who);
                endwhile
	elseif ( evento.type == "Ultimas Palavras")
		evento.+text := RequestGump(who, "Digite as ultimas palavras do NPC (caso seja um npc quem executa esse evento)", "", "", 1);
	endif
	return evento;
endfunction

function ExecuteEvent(evento)

//	print( "EVENTO " + evento.type);

	if (evento.type == "Abrir Porta" )
		foreach serial in (evento.items)
			var  item := SystemFindObjectBySerial(serial);
			item.locked := 0;
		endforeach
	elseif (evento.type == "Abrir Passagem")
		var i := 0;
		while ( i < 20)
			foreach serial in (evento.items)
				var item := SystemFindObjectBySerial(serial);
				item.movable := 1;
				var e := MoveObjectToLocation(item, item.x, item.y, item.z + 1, item.realm, MOVEOBJECT_FORCELOCATION);
        //printtextabove(item, "move object " + e );
				item.movable := 0;
			endforeach
			i := i + 1;
		endwhile
	elseif (evento.type == "Criar Npcs")
            var amt := 0;
           print("criar npcs  2 " + evento.x + " " + evento.y + " " + evento.z +  " amt " + evento.amount + " realm " + evento.realm);
            while (amt < evento.amount)
                var npc := CreateNpcFromTemplate( cstr(evento.npctemplate), Cint(evento.x) + RandomInt(2 * evento.range) - evento.range, Cint(evento.y) + RandomInt(2 * evento.range) - evento.range, Cint(evento.z), 0, Cstr(evento.realm) );
		print ( "NPC spawn " + npc);
                if (npc != error)
					var duracaonpc := evento.duracao;
					if (!duracaonpc)
						duracaonpc := 120;
					endif
					TS_StartTimer(npc, "summon", duracaonpc);
                  amt := amt + 1;
			if (evento.summontext)
				printtext(npc, evento.summontext);
			endif
                endif
              sleepms(2);
            endwhile
	    if (evento.bossinfo)
		var success := 0;

         	 //print("tem boss");

          	while ( !success)

            		var boss := CreateBoss(evento.bossinfo, Cint(evento.x), Cint(evento.y), Cint(evento.z), Cstr(evento.realm));

            		//print( "boss " + boss);

            		if (boss)

                		SetObjProperty(boss, "anchor", array{boss.x, boss.y});

                		success := 1;

				if (evento.summonbosstext)
					PrintText(boss, evento.summonbosstext);
				endif
            		endif

            		sleepms(2);


	    	endwhile
	endif
	elseif (evento.type == "Criar Itens")
		print("CRIAR ITEMS " + evento.items);
		foreach iteminfo in (evento.items)
			var item := CreateItemAtLocationFromTemplate( cint(iteminfo.x), cint(iteminfo.y), cint(iteminfo.z), Cstr(evento.realm) ,iteminfo.item );
			print("ITEM " + item);
		endforeach
	endif

endfunction

function PrepareEvent(evento)
	if (evento.type == "Abrir Porta" )
		foreach serial in (evento.items)
			var item := SystemFindObjectBySerial(serial);
			item.locked := 1;
		endforeach
	elseif (evento.type == "Abrir Passagem")
		var i := 0;
		while ( i < 20)
			foreach serial in (evento.items)
				var item := SystemFindObjectBySerial(serial);
				item.movable := 1;
				MoveObjectToLocation(item, item.x, item.y, item.z - 1, item.realm, MOVEOBJECT_FORCELOCATION);
				item.movable := 0;
			endforeach
			i := i + 1;
		endwhile
	endif
endfunction
