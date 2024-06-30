
use datafile;
use uo;
use util;
use os;

include ":tn:boss";
include ":gumps:gumps";
include ":gumps:yesNo";
include ":gumps:requestGump";
include ":tn:tngumps";
include ":spells:tnmagery";
include ":merchants:item_template";
include ":attributes:attributes";
include ":tn:questevent";

/* pega o array de quests ja criadas. cada entrada do array eh o nome da quest
*/
function GetQuestsKeys()

	var questdata := OpenDataFile( ":tn:questdata" );
	if (questdata == error)
		return questdata; //retorna o erro
	endif

	return questdata.Keys();

endfunction

function DeleteQuest(questname)

	var questdata := OpenDataFile( ":tn:questdata" );
	questdata.DeleteElement(questname);

endfunction


/* metodo mais simples que retorna o elem dado o nome da quest */
function GetQuestElem(questname)

	var questdata := OpenDataFile( ":tn:questdata" );
	if (questdata == error)
		return questdata; //retorna o erro
	endif

  return questdata.findelement(questname);

endfunction

/*cria um gump pra facilitar o gm criar a quest. a quest eh uma struct
que eh passada byref. Depois da struct criada ela tem que ser salva no datafile.
esse menu ta meio bugado nos botoes de add evento */

function CreateQuestStructGump(who, byref quest)

	var chargump := GFCreateGump();
	var input;


	//GFClosable(chargump, 0);
	GFDisposable(chargump, 0);

	GFResizePic(chargump, 0, 25, 3600, 300, 500);
	GFResizePic(chargump, 15, 40, 0x13BE, 270, 470);

	GFTextMid(chargump, 15, 50, 300, 1153, "Quest Menu");
	GFTextLine(chargump, 50, 65, 1153,  "Tempo p/ fazer (minutos):");
	GFTextLine(chargump, 50, 80, 1153,  "Nome:");
	GFTextLine(chargump, 50, 100, 1153, "Objetivo: ");
	GFAddButton( chargump, 30, 103, 2103, 2104, 1, 2);


	var nome;
	var time;

	if ( quest.time )
		time := GFTextEntry( chargump, 100, 65, 150, 20, 1150, quest.time);
	else
		time := GFTextEntry( chargump, 220, 65, 300, 20, 1150, "60");
	endif

	if ( quest.nome )
		nome := GFTextEntry( chargump, 100, 80, 150, 20, 1150, quest.nome);
	else
		nome := GFTextEntry( chargump, 100, 80, 150, 20, 1150, "-nome-");
	endif

	if(quest.type)
		GFTextLine(chargump, 100, 100, 1153, " " + quest.type);
	else
		GFTextLine(chargump, 100, 100, 1153, " ");
	endif


	GFTextLine(chargump, 50, 120, 1153, "Recompensas: ");
	GFAddButton( chargump, 50, 143, 2103, 2104, 1, 10);
	GFAddButton( chargump, 50, 163, 2103, 2104, 1, 11);
	GFAddButton( chargump, 50, 183, 2103, 2104, 1, 12);
	GFAddButton( chargump, 50, 203, 2103, 2104, 1, 13);
	GFAddButton( chargump, 50, 223, 2103, 2104, 1, 14);

	if (quest.recompensas)
		var y := 0;
		foreach lootinfo in (quest.recompensas)
			GFTextLine(chargump, 60, 140+y, 1150, " " + lootinfo.type );
			y := y + 20;
		endforeach
	endif

	var num_lines := 10;
	var msg_array := array;
	var y_pos := 260;
	var i;


	GFResizePic(chargump, 15, 240, 2620, 270, 240);
	for ( i:=1; i<=num_lines; i:=i+1 )
		if (!quest.description)
			msg_array.Append(GFTextEntry(chargump, 20, y_pos, 300, 20, 2100, ""));
		else
			msg_array.Append(GFTextEntry(chargump, 20, y_pos, 300, 20, 2100, cstr(quest.description[i])));
		endif
		y_pos := y_pos+19;
	endfor

	GFTextLine(chargump, 35, 240, 2100, "Descreva a quest:");
	GFAddButton(chargump, 230, 467, 4502, 4502, 1, 1 );
	GFTextLine(chargump, 190, 480, 1153, "Fim");
	input := GFSendGump(who, chargump);

	if (quest.nome == error)
		quest.+nome := Cstr(GFExtractData(input, nome));

	else
		quest.nome := Cstr(GFExtractData(input, nome));
	endif

	if (quest.time == error)
		quest.+time := Cint(GFExtractData(input, time));

	else
		quest.time := Cint(GFExtractData(input, time));
	endif


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
	/*for ( i:=msg_array.Size(); i>=1; i:=i-1 )
		if ( Len(msg_array[i]) <= 0 )
			msg_array.Erase(i);
		endif
		SleepMS(2);
	endfor*/

	var text := "";
	for ( i:=1; i<msg_array.Size(); i:=i+1 )
		text := text + msg_array[i];
		SleepMS(2);
	endfor

	sendsysmessage(who,"msg " + msg_array);

	if (quest.description == error)
		quest.+description := msg_array;
	else
		quest.description := msg_array;
	endif
	sendsysmessage(who, "desc " + quest.description);

	if (input[0] == 1 )
		return;
	elseif( input[0] == 2)
		quest.+type := RadioGump(who, 400, 380, "Escolha o tipo de quest.", array{"Trazer X itens", "Trazer Item Guardado"/*, "Levar item para local X", "Levar item para pessoa X"*/});
		ConfigureQuest(who, quest);
		CreateQuestStructGump(who, quest);
	elseif (input[0] == 10 || input[0] == 11 || input[0] == 12 || input[0] == 13 || input[0] == 14)

		var nevent := ConfigureEvent(who);
		if (nevent)
			if (!quest.recompensas)
				quest.+recompensas := array;
			endif
			(quest.recompensas).append(nevent);
		endif
		CreateQuestStructGump(who, quest);

	endif

endfunction

/* essa funcao determina o que deve ser feito para se concluir a quest.
*/

function ConfigureQuest(who, byref quest)

	if (quest.type == "Trazer X itens")
		quest.+itemamount := array;
		quest.+itemobj := array;
		quest.+itemdesc := array;
		SendSysMessage(who, "Escolha os itens que quer que tragam para quest. ESC para terminar.");
		var item := target(who);
		while ( item )
			var itemamount := RequestGump(who, "Quantidade de itens que devem ser entregues (se nao for stackable escreva 1):", "", "", 1);
			(quest.itemamount).append(itemamount);
			(quest.itemobj).append(item.objtype);
			(quest.itemdesc).append(item.desc);
			item := Target(who);
		endwhile

		if ((quest.itemobj).size() > 1)
			SendSysMessage(who, "Quest configurada.");
		else
			SendSysMessage(who, "Cancelado.");
		endif

		quest.+itemdiferentes := (quest.itemobj).size();
	elseif (quest.type == "Trazer Item Guardado")


		quest.+items := array;
		SendSysMessage(who, "Escolha os itens que quer que tragam para quest. ESC para terminar.");
		var item := target(who);
		while ( item )
			var itemtemplate := CreateItemTemplate(item);
			itemtemplate.desc := item.desc;
			quest.items.append(itemtemplate);
			item := target(who);
		endwhile

		if (quest.items.size() == 0)
			SendSysMessage(who, "cancelado.");
			return;
		else
			quest.+numItems := quest.items.size();
		endif

		var npcs := YesNo(who, "Deseja criar npcs para guardar os items?");
		if (npcs)
			quest.+numSpawns := Cint(RequestGump(who, "Quantos spawns de npcs voce deseja criar:", "", "", 1));
			var numSpawns2 := quest.numSpawns;
			quest.+npcspawns := array;
			while ( numSpawns2 > 0)
				var npcinfo := SetupNpcSpawn(who);
				quest.npcspawns.append(npcinfo);
				numSpawns2 := numSpawns2 - 1;
			endwhile
		endif

		var numItems := quest.numItems;
		var itens := 1;
		var spawnNames := array;
		var i := 1;
		for ( i := 1; i <= quest.numSpawns; i := i + 1)
			spawnNames.append(i);
		endfor
		while(itens <= numItems)

			var resp := RadioGump(who, 400, 380, "Em qual spawn deve ficar o item (na ordem que foram adicionados): " + quest.items[itens].desc, spawnNames );
			quest.npcspawns[resp].+item := itens; //o spawn numero resp (1, 2, 3... n) vai spawnar o item questinfo.items[item]
			itens := itens + 1;
		endwhile

	endif


endfunction


/* metodo simples pra configurar um spawn*/
function SetupNpcSpawn(who)

	var npcspawn := struct;
	npcspawn.+amount := Cint(RequestGump(who, "Quantos npcs vao ser spawnados:", "", "", 1));
	var loc := RequestGump(who, "Digite o x y z onde eles vao aparecer:", "", "", 1);
	loc := splitwords(loc);
	npcspawn.+x :=  loc[1];
	npcspawn.+y :=  loc[2];
	npcspawn.+z :=  loc[3];
	npcspawn.+range := Cint(RequestGump(who, "Digite o raio de spawn:", "", "", 1));
	npcspawn.+npctemplate := RequestGump(who, "Digite o npctemplate:", "", "", 1);
	var boss := YesNo( who, "Deseja adicionar um npc boss?" );
	if (boss)
		var bossinfo := struct;
		CreateBossStructGump(who, bossinfo);
		npcspawn.+bossinfo := bossinfo;
	endif

	return npcspawn;
endfunction

/*funcao que verifica se a quest foi concluida com sucesso*/

function CheckQuestRequisites(questinfo, who)


	//sendsysmessage(who, " " + questinfo);

  if (questinfo.type == "Trazer X itens")

	//sendsysmessage(who, "trazer x itens");
    var items := questinfo.itemobj;
    var amount := questinfo.itemamount;
    var descs := questinfo.itemdesc;

    var i := 1;
    foreach item in ( questinfo.itemobj)
      SendSysMessage(who, "Mostre o[s] " + amount[i] + " item " + descs[i] );
      var targ := Target(who);
	//sendsysmessage(who, " " + targ.amount + " " + amount[i]);
      if (targ.objtype == items[i])
        if (targ.amount != cint(amount[i]))
          SendSysMessage(who, "Essa nao parece ser a quantidade certa.");
          return 0;
        else
          DestroyItem(targ);
        endif
      else
        SendSysMessage(who, "Esse nao parece ser o item certo.");
        return 0;
      endif
      i := i + 1;
    endforeach

    return 1;

  elseif ( questinfo.type == "Trazer Item Guardado")

    var num := questinfo.numitems;
    var i := 1;
    var items := questinfo.items;
    while( i <= num )

      SendSysMessage(who, "Mostre o(a) " + items[i].desc);
      var targ := Target(who);

        if (targ.objtype != items[i].objtype)
          SendSysMessage(who, "Este nao parece ser o item certo.");
          return 0;
        else
            DestroyItem(targ);
        endif

      i := i + 1;
    endwhile

      return 1;
  endif

endfunction

function ResetQuest(questname)

  var questdata := OpenDataFile(":tn:questdata" );

  var elem := questdata.findelement(questname);

  var questinfo := elem.getprop("questinfo");

	print("RESETANDO QUEST " + questinfo.type);

  if (questinfo.type == "Trazer Item Guardado")

    //Reseta os npcs, caso tenha criado eles antes.

	print("criaturas" + questinfo.creatures);
    if (questinfo.creatures == error)
	questinfo.creatures := array;
    else
	foreach mobserial in (questinfo.creatures)
		var mob := SystemFindObjectBySerial(mobserial);
		print(" mob " + mob);
		SetObjProperty(mob, "summoned", 1);
		SetObjProperty(mob, "spawnkill", 1);
		RevokePrivilege(mob, "invul" );
		ApplyRawDamage(mob, GetVital(mob, HITS)+3);
	endforeach
    endif
  endif

endfunction

/* essa funcao faz o necessario para a quest existir. ou seja, prepara os eventos
de recompensas, cria os npcs, cria os bosses, os itens que devem ser trazidos, etc
*/
function SetupQuest(questname)



  var questdata := OpenDataFile(":tn:questdata" );

  var elem := questdata.findelement(questname);

  var questinfo := elem.getprop("questinfo");

  //print(" " + questdata + " " + elem + " " + questinfo);

  foreach premio in (questinfo.recompensas)
	PrepareEvent(premio);
  endforeach




  if (questinfo.type == "Trazer Item Guardado")


    var numspawns := questinfo.numSpawns;

    var items := questinfo.items;

    var i := 1;

    var spawns := questinfo.npcspawns;
        var npcs := array;
           var boss := 0;
    if (!questinfo.creatures)
	questinfo.+creatures := array;
     endif

    while( i <= numspawns)

      foreach spawn in spawns

        var amt := 0;

        while (amt < spawn.amount)

            //print("criando npc");

            //print( " " + spawn.npctemplate + " x " + spawn.x + " y " + spawn.y + " z " + spawn.z + " amount " + spawn.amount);

            var npc := CreateNpcFromTemplate( cstr(spawn.npctemplate), Cint(spawn.x) + RandomInt(2 * spawn.range) - spawn.range, Cint(spawn.y) + RandomInt(2 * spawn.range) - spawn.range, Cint(spawn.z));

            //print(" " + npc);

            if (npc != error)

              amt := amt + 1;

              SetObjProperty(npc, "anchor", array{npc.x, npc.y});

              npcs.append(npc);

            endif

          sleepms(2);

        endwhile



        if (spawn.bossinfo)

          var success := 0;

          //print("tem boss");

          while ( !success)

            boss := CreateBoss(spawn.bossinfo, Cint(spawn.x), Cint(spawn.y), Cint(spawn.z), REALM_BRITANNIA);

            print( "boss " + boss);

            if (boss)

                SetObjProperty(boss, "anchor", array{boss.x, boss.y});

                success := 1;

                if (spawn.item)

                  var amt := items[spawn.item].Amount;

                  if (!amt)

                    amt := 1;

                  endif

                  var item := CreateItemInBackpack(boss, items[spawn.item], amt);

                  item.movable := 1;

                endif

            endif

            sleepms(2);

          endwhile

        else

          print(" " + items[spawn.item]);

          if (spawn.item)

            var selectnpc := npcs[RandomInt(npcs.size())+1];

            if (!selectnpc.backpack)

              var backpack := CreateItemAtLocation(selectnpc.x, selectnpc.y, selectnpc.z,  2482,  1, selectnpc.realm);

              EquipItem(selectnpc, backpack);

            endif

            var ret := CreateItemInBackpack(selectnpc, items[spawn.item], items[spawn.item].Amount);

            ret.movable := 1;

            print("item criado " + ret);

          endif

        endif



      endforeach

        i := i + 1;

    endwhile

    //salva o que foi spawnado
    foreach mob in npcs
	questinfo.creatures.append(mob.serial);
    endforeach
    if (boss)
    	questinfo.creatures.append(boss.serial);
    endif
    elem.SetProp("questinfo", questinfo);

  endif




endfunction
