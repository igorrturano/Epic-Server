use uo;
use cfgfile;

include "include/dungeons";
include ":attributes:attributes";

function GetPlayerNameBySerial(serial)
   var player := SystemFindObjectBySerial(serial, SYSFIND_SEARCH_OFFLINE_MOBILES);
   if (!player) return 0; endif
   var player_name := player.name;
   if (GetObjProperty(player, "realname"))
      player_name := GetObjProperty(player, "realname");
   endif
	return player_name;
endfunction