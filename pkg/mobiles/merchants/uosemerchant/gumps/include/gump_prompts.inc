
use uo;

include ":gumps:gumps";
include ":gumps:gumps_ex";
include "include/client";
include "include/utils";

const HIDDEN_CANCEL := 1;
const DEFAULT_FONT_COLOR := 1153;

function QuestionsGumpUOSE(who, questions := array{}, hidden_cancel_btn := 0)
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
            if (quest.helper)
               if (quest.helper[_entry_iter]) GFTooltipText(gump, quest.helper[_entry_iter]); endif 
            endif
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
            if (quest.helper) if (quest.helper[_entry_iter]) GFTooltipText(gump, quest.helper[_entry_iter]); endif endif
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

/* 
 * YesNoUOSE(mobile, prompt, yes_text, no_text)
 *
 * Purpose
 * Template to create a yes / no gump.
 *
 * Parameters
 * mobile:	Mobile to send the gump to.
 * prompt:	Primary text field, instructing the mobile on what the choices are about.
 * yes_text:	Text to be displayed next to the 'YES' button.
 * no_text:	The text to be displayed next to the 'NO' button.
 *
 * Return value
 * Returns '0' if no is selected.
 * Returns '1' if yes is selected.
 *
 */
function YesNoUOSE(mobile, prompt:="", yes_text:="Sim", no_text:="N�o")
	var yn_gump := GFCreateGump(140, 100);

	var wraps := struct;
	wraps.+prompt := GFWordWrap(prompt, 385);
	wraps.+yes := GFWordWrap(yes_text, 355);
	wraps.+no := GFWordWrap(no_text, 355);
	var wraps_size := len(wraps.prompt) + len(wraps.yes) + len(wraps.no);

	GFClosable(yn_gump, 0);
	GFResizePic(yn_gump, 0, 0, DEFAULT_BACKGROUND, 400, wraps_size*20 +80 ); //205);
	GFResizePic(yn_gump, 15, 15, DEFAULT_FOREGROUND, 370, wraps_size*20 + 50); //175);
	
	var y_pos := 20;
	foreach line in (wraps.prompt)
		GFTextLine(yn_gump, 20, y_pos, DEFAULT_FONT_COLOR, line);
		y_pos := y_pos+20;
		SleepMS(2);
	endforeach
	
	//y_pos := 90;
	y_pos += 20;
	GFAddButton(yn_gump, 20, (y_pos+3), 2117, 2118, GF_CLOSE_BTN, 100);
	foreach line in (wraps.yes);
		GFTextLine(yn_gump, 40, y_pos, DEFAULT_FONT_COLOR, line);
		y_pos := y_pos+20;
		SleepMS(2);
	endforeach
		
	//y_pos := 140;
	y_pos += 20;
	GFAddButton(yn_gump, 20, (y_pos+3), 2117, 2118, GF_CLOSE_BTN);
	foreach line in (wraps.no)
		GFTextLine(yn_gump, 40, y_pos, DEFAULT_FONT_COLOR, line);
		y_pos := y_pos+20;
		SleepMS(2);
	endforeach
	
	var input := GFSendGump(mobile, yn_gump);
	input := input[0];
	if ( input == 100 )
		return 1;
	else
		return 0;
	endif	
endfunction
