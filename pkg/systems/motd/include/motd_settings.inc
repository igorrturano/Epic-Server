/* MOTD per-player login behavior.
 * Shared by the MOTD gump (motd.src), the .motd command, and the logon hook.
 * Only touches the player's datafile element, so it is safe to include from
 * core scripts and package scripts alike (no file/path assumptions). */

const MOTD_MODE_ALWAYS := "always";
const MOTD_MODE_UNREAD := "unread";
const MOTD_MODE_OFF    := "off";

// Player's MOTD login mode. Migrates the legacy binary "noshow" prop:
// noshow == 1 meant "don't auto-open" -> off; anything else -> the new
// default of "only when there's something new".
function MOTD_GetMode(data_elem)
	var mode := data_elem.GetProp("motd_mode");
	if (mode == MOTD_MODE_ALWAYS || mode == MOTD_MODE_UNREAD || mode == MOTD_MODE_OFF)
		return mode;
	endif
	if (CInt(data_elem.GetProp("noshow")) == 1)
		return MOTD_MODE_OFF;
	endif
	return MOTD_MODE_UNREAD;
endfunction

// Persists the mode and drops the legacy prop so the two can't disagree.
function MOTD_SetMode(data_elem, mode)
	if (mode != MOTD_MODE_ALWAYS && mode != MOTD_MODE_UNREAD && mode != MOTD_MODE_OFF)
		mode := MOTD_MODE_UNREAD;
	endif
	data_elem.SetProp("motd_mode", mode);
	data_elem.EraseProp("noshow");
	return mode;
endfunction
