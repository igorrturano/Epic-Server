
use uo;

include ":gumps:gumps";
include ":gumps:gumps_ex";
include "include/client";
include "include/utils";

const HIDDEN_CANCEL := 1;

function AutoCheckBox(mobile, prompt, options, returnKeys := 0)

	var sizex;
	var sizey;

	var columns := options.size()/20;
	if ( columns <= 1 )
		sizey := 40 + options.size()*20;
		if (sizey > 540)
			sizey := 540;
		endif
		if (sizey < 160)
			sizey := 160;
		endif
		sizex := 200;
		if ( (options.size() % 20) > 0)
			sizex := sizex + 150;
		endif
	else
		sizey := 540;
		sizex := 40 + cint(columns) * 150;
		if ( (options.size() % 20) > 0)
			sizex := sizex + 150;
		endif
		if (sizex > 800)
			sizex := 800;
		endif
		
	endif

	var gump := GFCreateGump(40, 40);
	GFClosable(gump, 0);
    GFSetID(gump, GUMPID_TNGUMPS);
	GFResizePic(gump, 0, 0, 3600, sizex, sizey);
	GFResizePic(gump, 15, 15, 0x13BE, sizex-30, sizey-30);
	var y_pos := 20;
	foreach line in (GFWordWrap(prompt, 360))
		GFTextLine(gump, 20, y_pos, 1153, line);
		y_pos := y_pos+20;
		SleepMS(2);
	endforeach

	GFSetRadioGroup(gump, 1);
	var indice := 1;
	y_pos := 70;
	var first := 1;
	
	var y := 0;
	var x := 0;
	while(y <= columns)
		var maxx := 20;
		if (y == columns)
			maxx := options.size() % 20;
		endif
		while (x < maxx)
			GFTextLine(gump, 60 + y*150, y_pos + x*20, 1153, options[indice]);
			GFCheckBox(gump, 30 + y*150, y_pos + x*20, 210, 211, first, indice);
			first := 0;
			indice := indice+1;
			x := x + 1;
		endwhile
		x := 0;
		y := y + 1;
		indice := indice + 1;
	endwhile

	//foreach text in options
	//	GFTextLine(gump, 60, y_pos + indice*20, 1153, options[indice]);
	//	GFCheckBox(gump, 30, y_pos + indice*20, 210, 211, first, indice);
	//	first := 0;
	//	indice := indice+1;
	//endforeach
	
	GFAddButton(gump, sizex-70, sizey-60, 4502, 4502, 1, 999 );
	GFTextLine(gump, sizex-110, sizey-50, 1153, "Próximo");
  
	var ret := array;
  
	var input := GFSendGump(mobile, gump);
	if ( input[0] == 999 )
		 foreach key in (input.keys)
        		if (options[key] )
					if (returnKeys)
						ret.append(key);
					else
						ret.append(options[key]);
					endif
        		endif
     		endforeach
     		return ret;
  	else
      return 0;
	endif	

endfunction

/* CheckBoxGump cria um gump com varias checkbox onde pode retornar varios valores clicados
   RadioGump cria um gump com varias radiosbuttons, mas apenas um botao pode ser retornado */

function CheckBoxGump(mobile, sizex, sizey, prompt:="", options, opts := 0)
   if (!sizex) sizex := 200; endif;
   if (!sizey) sizey := 125 + (options.size() * 20); endif;
   
    
	var gump := GFCreateGump(250, 200);
	GFClosable(gump, 0);
    GFSetID(gump, GUMPID_TNGUMPS);
	GFResizePic(gump, 0, 0, 3600, sizex, sizey);
	GFResizePic(gump, 15, 15, 0x13BE, sizex-30, sizey-30);
	
	var y_pos := 20;
	foreach line in (GFWordWrap(prompt, 360))
		GFTextLine(gump, 20, y_pos, 1153, line);
		y_pos := y_pos+20;
		SleepMS(2);
	endforeach
	
  GFSetRadioGroup(gump, 1);
  var indice := 1;
  y_pos := 30;
  var first := 0;
  foreach text in options
    GFTextLine(gump, 60, y_pos + indice*20, 1153, text);
    if (opts)
      first := opts[indice];
    endif
    GFCheckBox(gump, 30, y_pos + indice*20, 210, 211, first, indice);
    first := 0;
    indice := indice+1;
  endforeach
	
    GFAddButton(gump, sizex-70, sizey-60, 4502, 4502, 1, 999 );
    GFTextLine(gump, sizex-110, sizey-50, 1153, "Próximo");
    
  var ret := array;
  
	var input := GFSendGump(mobile, gump);
	if ( input[0] == 999 )
		 foreach key in (input.keys)
        if (options[key]  )
          ret.append(options[key]);
        endif
     endforeach
     return ret;
  else
      return 0;
	endif	

endfunction

function RadioGump(mobile, sizex, sizey, prompt:="", options, returnkeys := 0, first := 1)
   if (!sizex) sizex := 200; endif;
   if (!sizey) sizey := 120 + (options.size() * 20); endif;

	var gump := GFCreateGump(250, 200);
	GFClosable(gump, 0);
	GFResizePic(gump, 0, 0, 3600, sizex, sizey);
	GFResizePic(gump, 15, 15, 0x13BE, sizex-30, sizey-30);
	
	var y_pos := 20;
	foreach line in (GFWordWrap(prompt, 360))
		GFTextLine(gump, 20, y_pos, 1153, line);
		y_pos := y_pos+20;
		SleepMS(2);
	endforeach
	
  GFSetRadioGroup(gump, 1);
  var indice := 1;
  y_pos := 30;
  foreach text in options
    GFTextLine(gump, 60, y_pos + indice*20, 1153, text);
    GFRadioButton(gump, 30, y_pos + indice*20, 210, 211, (indice == first), indice);
    indice := indice+1;
  endforeach
	
  GFAddButton(gump, sizex-70, sizey-60, 4502, 4502, 1, 999 );
	GFTextLine(gump, sizex-110, sizey-50, 1153, "Próximo");
  
	var input := GFSendGump(mobile, gump);
	if ( input[0] == 999 )
		 foreach key in (input.keys)
        if (options[key])
		    if (returnkeys)
				return key;
			 endif
          return options[key];
        endif
     	endforeach
  else
      return 0;
	endif	
  
endfunction

  
function BigRequestGump(who, question := "")
	var gump := GFCreateGump();
   GFSetID(gump, GUMPID_TNGUMPS);
	GFDisposable(gump, 0);
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
	GFResizePic(gump, 15, 431, GFCfgConst("Defaults", "ForeGround"), 310, 30);

	GFAddButton(gump, 285, 441, 4011, 4030, GF_CLOSE_BTN, 20);
	GFTextLine(gump, 225, 441, 2100, "Terminar");
	GFTextLine(gump, 34, 23, 2100, question);

	var input := GFSendGump(who, gump);

	if (input[0] == 20)
		var i;
		for ( i:=1; i<=msg_array.Size(); i:=i+1 )
			var line := GFExtractData(input, msg_array[i]);
      //sendsysmessage(who, "extraindo linha " + line);
			if ( line )
				line := line + " ";
				msg_array[i] := line;
			else
				msg_array[i] := "";
			endif
      //sendsysmessage(who, "linha final " + line);
		endfor
	
		//Clear off empty lines.
		for ( i := msg_array.Size(); i>=1; i:=i-1 )
			if ( Len(msg_array[i]) <= 0 )
				msg_array.Erase(i);
			endif
			SleepMS(2);
		endfor
		
		var text := "";
		for ( i:=1; i<=msg_array.Size(); i:=i+1 )
			text := text + msg_array[i];
			SleepMS(2);
		endfor
		
		return text;
	endif
endfunction


function SmallRequestGump(who, question := "", addBR := 0)
	var gump := GFCreateGump();
   GFSetID(gump, GUMPID_TNGUMPS);
	GFResizePic(gump, 15, 50, 2620, 150, 100);
	
	var num_lines := 5;
	var msg_array := array;
	var y_pos := 55;
	var i;
	for ( i:=1; i<=num_lines; i:=i+1 )
		msg_array.Append(GFTextEntryLimited(gump, 20, y_pos, 595, 20, 2100, "", 0, 22));
		y_pos := y_pos+19;
	endfor

	GFResizePic(gump, 15, 20, GFCfgConst("Defaults", "ForeGround"), 150, 30);
	GFResizePic(gump, 15, 150, GFCfgConst("Defaults", "ForeGround"), 150, 30);

	GFAddButton(gump, 130, 150, 4011, 4030, GF_CLOSE_BTN, 20);
	GFTextLine(gump, 75, 150, 2100, "Termina");
	GFTextLine(gump, 35, 20, 2100, question);

	var input := GFSendGump(who, gump);

	if (input[0] == 20)
		var i;
		for ( i:=1; i<=msg_array.Size(); i:=i+1 )
			var line := GFExtractData(input, msg_array[i]);
			if ( line )
				line := line + " ";
				if (addBR)
					msg_array[i] := line + "<br> ";
				else
					msg_array[i] := line;
				endif
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
		for ( i:=1; i<=msg_array.Size(); i:=i+1 )
			text := text + msg_array[i];
			//sendsysmessage(who, " " + text);
			SleepMS(2);
		endfor
		
		return text;
	endif
endfunction

   // var questions := array{ 
   //    struct{"title" := "Entre com texto", "marked" := "Seu texto"}, 
   //    struct{"title" := "Entre com texto"},
   //    struct{"title" := "Select an Option?", "marked" := 3, "radio" := array{ "Dia", "Noite", "feliz"}},
   //    struct{"title":= "Is a checkbox", "checkbox" := array{"teste", "teste2"}, "marked":= array{1}},
   //    };
   // var res:= QuestionsGump(who, questions);
   // var answer_1 := res[1];
function QuestionsGump(who, questions := array{}, hidden_cancel_btn := 0)
	var total_height := (questions.size() * 27);
   foreach quest in questions
      if (quest.radio)
         total_height += Floor((quest.radio.size()+3) / 4) * 25;
      elseif (quest.checkbox)
         total_height += Floor((quest.checkbox.size()+3) / 4) * 25;
      elseif (quest.lines)
         total_height += cint(quest.lines+1) * 25;
      else
         total_height += 25;
      endif
      sleepms(2);
   endforeach

	var gump := GFCreateGump(250, 250);
   GFSetID(gump, GUMPID_TNGUMPS);
	GFResizePic(gump, 250, 250, cint(0x9C40), 540, 70+total_height);

   if (hidden_cancel_btn)
      GFClosable(gump, 0);
   endif

	var entries := dictionary{};
   var previousiter := 0;
	var x := 270;
	var y := 270;
	foreach quest in questions
		var x_plus := x;
		GFTextLine(gump, x, y, 2100, quest.title);
      if (quest.helper)
         GFTooltipText(gump, quest.helper);
      endif
		y += 18;
		if (quest.radio)
  			if (!quest.continueradio) 
            GFSetRadioGroup(gump, _quest_iter);
         else
            previousiter := quest.radio.size();
         endif

			foreach entry in (quest.radio)
				GFTextLine(gump, x_plus+25, y+3, 1153, entry);
            GFRadioButton(gump, x_plus, y+3, 210, 211, (_entry_iter == cint(quest.marked)), _entry_iter+(_quest_iter * 100));
				x_plus += 130;
            if (_entry_iter % 4 == 0 && quest.radio[_entry_iter+1])
               y += 25;
               x_plus := x;
            endif
			endforeach
		elseif(quest.checkbox)
			foreach entry in (quest.checkbox)
				GFTextLine(gump, x_plus+25, y+3, 1153, entry);
				GFCheckBox(gump, x_plus, y+3, 210, 211, (_entry_iter in quest.marked), _entry_iter+(_quest_iter * 500));
				x_plus += 130;
           if (_entry_iter % 4 == 0 && quest.checkbox[_entry_iter+1])
               y += 25;
               x_plus := x;
            endif
			endforeach
		elseif(quest.multipleanswer)
         if (!quest.marked) quest.marked := array{}; endif

         entries[_quest_iter] := array{};
         var i := 1;
         for (i; i <= quest.multipleanswer;i+=1)
            if (!quest.marked[i]) quest.marked[i] := ""; endif
            var x_plus := x + ((i-1) * 115);
            GFResizePic(gump, x_plus, y+3, 0x7752, 200-100, 28);
            entries[_quest_iter].append(GFTextEntry(gump, x_plus+5, y+5, 200-100, 20, 2100, quest.marked[i]));

            sleepms(2);
         endfor
		else
         quest.lines := cint(quest.lines);
         if (!quest.marked) quest.marked := ""; endif
         
         GFResizePic(gump, x, y+3, 0x7752, 500, 28 + (quest.lines * 18));
         var texts := quest.marked;
         if (quest.lines > 1) 
            texts := GFWordWrap(quest.marked, 570);
         else
            texts := array{quest.marked};
         endif

         entries[_quest_iter] := array{};
         for lines := 0 to (quest.lines)
            if (lines != 0) y += 18; endif
            var text := texts[lines+1];
            if (!text) text := ""; endif
            entries[_quest_iter].append(GFTextEntry(gump, x+5, y+5, 500, 20, 2100, text));
         endfor
		endif

		y += 36;
		sleepms(2);
	endforeach

	GFAddButton(gump, x+150, y, 4011, 4030, GF_CLOSE_BTN, 5); //confirm
   GFTextLine(gump, x+185, y+3, 1153, "Confirmar");
   if (!hidden_cancel_btn)
      GFAddButton(gump, x+260, y, 4011, 4030, GF_CLOSE_BTN, 1); //cancel;
      GFTextLine(gump, x+295, y+3, 1153, "Cancelar");
   endif

	var input := GFSendGump(who, gump);
	if (input[0] == 1) return 0; endif

	var res := array{};
	foreach quest in questions
		if (quest.radio)
			var result;
			foreach item in (quest.radio)
				if (_item_iter + (_quest_iter * 100) in input.keys)
					result := _ifElse(quest.return_number, _item_iter, item);
					break;
				endif
			endforeach
			res[_quest_iter] := result;
		elseif (quest.checkbox)
			var result := array{};
			foreach item in (quest.checkbox)
				if (_item_iter + (_quest_iter * 500) in input.keys)
               if (quest.return_number)
                  result.append(_item_iter);
               else
					   result.append(item);
               endif
				endif
			endforeach
			res[_quest_iter] := result;
      elseif (quest.multipleanswer)
         res[_quest_iter] := array{};
         foreach i in (entries[_quest_iter])
            var output :=  GFExtractData(input, i);
            if (output && output != " ") res[_quest_iter].append(output); endif
            sleepms(2);
         endforeach
		else
         res[_quest_iter] := array{};
         foreach i in (entries[_quest_iter])
            var output := GFExtractData(input, i);
            if (output && output != " " && output != "") res[_quest_iter].append(output); endif
            sleepms(2);
         endforeach
         res[_quest_iter] := " ".join(res[_quest_iter]);
		endif
		sleepms(2);
	endforeach

	return res;
endfunction

////////
// options = [
//    { 
//       text = Essa Opção,
//       Icon = 7748, 
//       Item = item.graphic,
//       color = 0
//    }
// ]
function ChoiceSelectorGump(who, question := "", options := array{}, allow_cancel := 0)
   var fix_cfg := ReadConfigFile(":fls_core:item_fix");
	var gump := GFCreateGump(250, 250);
   GFSetID(gump, GUMPID_TNGUMPS);
	
	GFResizePic(gump, 10, 8, 9380, 392, 143+(options.size() * 69)); // background
   
	GFGumpPicTiled(gump, 37, 46, 338, 10, 50); // background
	GFGumpPicTiled(gump, 39, 108+(options.size() * 67), 338, 10, 50); // background

   var y := 87;
   foreach btn in options
      GFAddButton(gump, 51, y+9, 5151, 248, GF_CLOSE_BTN, _btn_iter); // button
      GFAddButton(gump, 195, y+9, 5151, 248, GF_CLOSE_BTN, _btn_iter); // button
      GFResizePic(gump, 44, y, 5120, 327, 58); // button
      GFTextLine(gump, 118, y+18, 2212, btn.text);

      GFResizePic(gump, 43, y, 9270, 63, 58); // behind icon

      if (btn.item)
         var fx := GetItemFix(fix_cfg, btn.item);
         GFResizePic(gump, 50, y+7, 9350, 50, 43);
         GFTilePic(gump, 55+fx.x, y+16+fx.y, btn.item, cint(btn.color)); //item
      elseif (btn.icon)
         GFGumpPic(gump, 52, y+7, btn.icon, cint(btn.color)); //gump
      endif
      y += 62;
   endforeach

   if (!allow_cancel)
	   GFAddButton(gump, 179, 85+(options.size() * 64), 2072, 2073, GF_CLOSE_BTN, 50); // cancel
   else
	   GFClosable(gump, 0);
   endif

   GFHtmlShadowed(gump, 42, 59, 326, 20, "#FFFFFF", "<center>{}".format(question), 5);

	var res := GFSendGump(who, gump)[0];
   if (res[0] == 50) return 0; endif
   return res;

endfunction

function PositionGump(who, cur_x := 0, cur_y := 0)
   var gump := GetGlobalProperty("#resize_gump");
   var positions := array{};
   if (!gump)
      gump := GFCreateGump();
	   GFClosable(gump, 0);
	   GFMovable(gump, 0);

      GFResizePic(gump, 0, 0, 9200, 1920, 1080);
      GFAddAlphaRegion(gump, 1, 2, 1920, 1080);
      GFSetRadioGroup(gump, 0);

      var i := 1;
      var y;
      for ( y := 3; y < 1080; y += 45)
         var x;
         for (x := 1; x < 1920; x += 52)
            positions.append(array{x, y});
            GFAddButton(gump, x, y, 1210, 1210, GF_CLOSE_BTN, positions.size());
            i += 1;
            sleepms(2);
         endfor
      endfor

      GFTextLine(gump, 433, 336, 2212, "Clique em um botão para reposicionar");
      GFTextLine(gump, 765, 558, 2212, "Clique em um botão para reposicionar");
      SetGlobalProperty("#resize_gump", struct{ gump := gump, positions := positions});
   else
      positions := gump.positions;
      gump := gump.gump;
   endif

   if (cur_x || cur_y)
      GFGumpPic(gump, cur_x, cur_y, 1210, 2133);
   endif
   
	var res := GFSendGump(who, gump)[0];
   return positions[res];
endfunction