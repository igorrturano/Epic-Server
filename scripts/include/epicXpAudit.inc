// Append-only CSV ledger for every player-level XP grant (and suppressed
// would-be grants). Authoritative trace for tuning a fair XP system.
// Mirrors epicEconomyAudit.inc. Log path:
//   ::data/log/xp/<DD-MM-YYYY>/xp.log
// Columns (no header line, append-only, daily-dir rotation):
//   timestamp,source,acct,char,level,amount,x,y,z,realm,extra

use uo;
use os;
use file;
use util;
use basic;
use polsys;

const XP_NPC_KILL          := "NPC_KILL";
const XP_CRAFT             := "CRAFT";
const XP_CRAFT_SUPPRESSED  := "CRAFT_SUPPRESSED";
const XP_GM                := "GM";
const XP_UNKNOWN           := "UNKNOWN";

function XP_CsvEsc(value)
	if (value == error || !value)
		return "";
	endif
	return StrReplace(CStr(value), ",", ";");
endfunction

function XP_Record(who, amount, source, extra)
	if (!who || who == error || who.acctname == error || !who.acctname)
		return 0;
	endif

	amount := CInt(amount);

	var char_name := GetObjProperty(who, "nome_inicial");
	if (char_name == error || !char_name)
		char_name := who.name;
	endif

	var level := GetObjProperty(who, "Level");
	if (level == error || !level)
		level := 0;
	endif

	var timestamp := StrFormatTime("%d-%m-%Y %H:%M:%S", POLCore().systime);
	var today := StrFormatTime("%d-%m-%Y", POLCore().systime);

	var log_dir := "::data/log/xp/{}".format(today);
	if (!FileExists(log_dir))
		CreateDirectory(log_dir);
	endif
	var log_file := "{}/xp.log".format(log_dir);

	var line := "{},{},{},{},{},{},{},{},{},{},{}".format(
		timestamp,
		source,
		XP_CsvEsc(who.acctname),
		XP_CsvEsc(char_name),
		CInt(level),
		amount,
		who.x,
		who.y,
		who.z,
		XP_CsvEsc(who.realm),
		XP_CsvEsc(extra)
	);

	LogToFile(log_file, line, 0);
	return 1;
endfunction
