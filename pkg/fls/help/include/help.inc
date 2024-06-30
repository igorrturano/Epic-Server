
use uo;
use datafile;

include ":datafile:datafile";

function SendGmPage(who, msg)

	var df := DFOpenDataFile("gmpages", DF_CREATE);
	var elem := DFFindElement(df, who.serial, DF_CREATE);
	elem.SetProp( "page", msg);
	elem.SetProp( "date", polcore().systime);
	GMBcast("Nova page recebida!");
	GMBcast("Enviada por " + who.name);
	GMBcast("Texto " + msg);

endfunction

function SendCounselorPage(who, msg)

	var df := DFOpenDataFile("counpages", DF_CREATE);
	var elem := DFFindElement(df, who.serial, DF_CREATE);
	elem.SetProp( "page", msg  );
	GMBcast("Nova page para counselor recebida!");
	GMBcast("Enviada por " + who.name);
	GMBcast("Texto " + msg);
	CounBcast("Nova page para counselor recebida!");
	CounBcast("Enviada por " + who.name);
	CounBcast("Texto " + msg);
endfunction

function ReportBug(who, msg, title)

	var df := DFOpenDataFile("bugs", DF_CREATE, DF_KEYTYPE_INTEGER);
	var total := df.Keys();
	var num := cint(total.size()) + 1;
	var elem := df.CreateElement(num);
	elem.SetProp( "page", msg);
	elem.SetProp( "ReporterSerial", who.serial  );
	elem.SetProp( "ReporterCharacter", who.name  );
	elem.SetProp( "ReporterAccount", who.acctname  );
	elem.SetProp( "Title", title  );
	GMBcast("Novo bug reportado!");
endfunction

function GMBcast(msg)
	var ppl := EnumerateOnlineCharacters();
	foreach x in ppl
		if (x.cmdlevel > 0)
			SendSysMessage(x, ""+msg);
		endif
	endforeach
endfunction

function CounBcast(msg)
	var ppl := EnumerateOnlineCharacters();
	foreach x in ppl
		if (GetObjProperty(x, "counselor") == 1)
			SendSysMessage(x, ""+msg);
		endif
	endforeach
endfunction

//pagefille: gmpages, counpages, bugs
function GetPageIndexByPageNum(num, pagefile)

	var df := DFOpenDataFile(pagefile, DF_CREATE, DF_KEYTYPE_INTEGER);
	var pages := df.Keys();
	if (pages[num])
		return pages[num];
	else
		return 0;
	endif

endfunction

function PageDescGump(who, elem)

	var chargump := GFCreateGump();
	var input;

	GFDisposable(chargump, 0);
	GFResizePic(chargump, 0, 25, 3600, 300, 380);
	GFResizePic(chargump, 15, 40, 0xA3C, 270, 350);
	//GFTextMid(chargump, 15, 35, 600, 1153, "Descricao da page");

	var lines := elem.GetProp("page");

	GFHTMLArea(chargump, 22, 61, 259, 303, lines, 0, 1);
	input := GFSendGump(who, chargump);
	input := input[0];


endfunction

function DeletePage(serial, pagetype)

	if ( serial )
		var df := DFOpenDataFile(pagetype);
		df.DeleteElement(serial);
		return 1;
	endif
	return 0;
endfunction
