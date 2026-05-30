use os;

include ":message:Controller/CachedTranslator";
include ":text:TranslatableText";

const TOOLTIP_PREFIX := "Tooltip 1114057 @";
const TOOLTIP_SUFFIX := "@'";

function _TranslateGumpTooltips(mobile, byref gump)
	if (!mobile || mobile == error)
		return;
	endif
	var acct := mobile.acct;
	if (!acct || acct == error)
		return;
	endif
	var targetLang := acct.GetProp("original_language");
	if (targetLang == error || !targetLang || targetLang == LANGUAGE::PORTUGUESE)
		return;
	endif

	var translator := CachedTranslator();
	for i := 1 to gump.layout.Size()
		var line := gump.layout[i];
		var text := _ExtractTooltipText(line);
		if (text == "")
			continue;
		endif
		var translated := translator.Translate(text, LANGUAGE::PORTUGUESE, targetLang);
		if (translated == text)
			continue;
		endif
		gump.layout[i] := TOOLTIP_PREFIX + translated + TOOLTIP_SUFFIX;
	endfor
endfunction

function _ExtractTooltipText(line)
	var prefixLen := Len(TOOLTIP_PREFIX);
	var suffixLen := Len(TOOLTIP_SUFFIX);
	if (Len(line) < prefixLen + suffixLen)
		return "";
	endif
	if (line[1, prefixLen] != TOOLTIP_PREFIX)
		return "";
	endif
	if (line[Len(line) - suffixLen + 1, Len(line)] != TOOLTIP_SUFFIX)
		return "";
	endif
	return line[prefixLen + 1, Len(line) - suffixLen];
endfunction
