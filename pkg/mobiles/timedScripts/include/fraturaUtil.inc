use uo;

include ":fls_core:packets";
include "include/say";

// Shared gump icon for the custom hotbar buff (packet 0xBF 0x0062).
const FRATURA_BUFF_ICON      := 2247;

// Buff IDs. Namespaced above spells (0-9999) and abilities (10000-19999).
const BUFF_ID_FRATURA_CABECA := 20001;
const BUFF_ID_FRATURA_MAOS   := 20002;
const BUFF_ID_FRATURA_BRACOS := 20003;
const BUFF_ID_FRATURA_TORSO  := 20004;
const BUFF_ID_FRATURA_PERNAS := 20005;

// Finalize one fratura's cleanup: drop the part from the forensic `Fractures`
// array, and — only when *all* fraturas have cleared — wipe deferred state and
// announce the heal. Skipped during timer-controller reconnect cleanup so the
// pre-restart cleanup pass doesn't spam "cicatrizaram" while the timer is
// about to be restarted by the next foreach in StartUp.
function EndFratura_Finalize(mobile, part_name)
	if (CInt(GetObjProperty(mobile, "#reconnecting")))
		return;
	endif

	var fractures := GetObjProperty(mobile, "Fractures");
	if (fractures != error)
		var remaining := array{};
		foreach entry in (fractures)
			if (entry != part_name)
				remaining.append(entry);
			endif
			sleepms(2);
		endforeach
		if (remaining.size() == 0)
			EraseObjProperty(mobile, "Fractures");
		else
			SetObjProperty(mobile, "Fractures", remaining);
		endif
	endif

	if (CInt(GetObjProperty(mobile, "#fratura_bracos_penalty"))) return; endif
	if (CInt(GetObjProperty(mobile, "#fratura_cabeca_penalty"))) return; endif
	if (CInt(GetObjProperty(mobile, "#fratura_maos_penalty")))   return; endif
	if (CInt(GetObjProperty(mobile, "#fratura_pernas_penalty"))) return; endif
	if (CInt(GetObjProperty(mobile, "#fratura_torso_penalty")))  return; endif

	EraseObjProperty(mobile, "#deferred_fratura");
	SendSysMessageEx(mobile, "Suas fraturas cicatrizaram.");
endfunction
