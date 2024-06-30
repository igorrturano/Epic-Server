var GP_MOVABLE	:= 0;


function GumpPrompt(mobile, text)
	OldGFInitGump(180, 100);
	var layout := array();
	
	if ( !GP_MOVABLE )
		layout.append("nomove");
	endif	
	layout := layout+array
	(
		"noclose",
		"resizepic 180 100 "+BKGND_BLUESLAB+" 300 160",
		"resizepic 195 115 "+BKGND_STONESLAB+" 270 130",
		"button 300 220 2128 2129 1 1"
	);
	var data := array();

	text := WordWrap(text, 270);
	var y_pos := 125;
	
	foreach line in text
		data.append(line);
		layout.append("text 200 " + y_pos + " " + GF_STDCOLOR + " " + (data.size()-1));
		y_pos := y_pos+20;
		sleepms(2);
	endforeach

	var gump_input := SendDialogGump(mobile, layout, data);
	return gump_input[0];
endfunction
