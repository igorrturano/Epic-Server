use uo;
use os;
use file;
include "include/client";
include "include/sysEvent";
include "include/say";
include "include/epicUtils";

function LogPlayerSpeech( who, text )
  if ( text == "..." || text == " . . . " || text == ". . ." )
    return 0;
  endif

  var account_name := who.acctname;
  var character_name := "";
  var position := "{}:{}:{}".format( who.x, who.y, who.z );
  var timestamp := StrFormatTime( "%d-%m-%Y %H:%M:%S", POLCore().systime );

  if ( GetObjProperty( who, "realname" ) )
    character_name := GetObjProperty( who, "realname" );
  else
    character_name := who.name;
  endif

  var listeners_names := GetPlayersInRange( who, 12 );

  var listeners_str := "None";
  if ( listeners_names.size() >= 1 )
    listeners_str := "\"{}\"".format( join( listeners_names, ", " ) );
  endif

  // Formato do diretório baseado na data atual
  var today := StrFormatTime( "%d-%m-%Y", POLCore().systime );
  var log_dir := "::data/log/players/{}".format( today );

  if ( !FileExists( log_dir ) )
    var dir := CreateDirectory( log_dir );
  endif

  var log_file := "{}/{}_{}.log".format( log_dir, account_name, character_name );

  // Formato da linha de log
  // timestamp,          account_name, character_name, position,   text,      listeners
  // 09-09-2024 20:04:40,admin,        Drow,           3484:1203:0,"asdsadas",None
  var log_entry := "{},{},{},{},\"{}\",{}\n".format( timestamp, account_name, character_name,
                                                     position, text, listeners_str );
  LogToFile( log_file, log_entry, 0 );
endfunction

function GetPlayersInRange( who, range )
  var players_in_range := array{};
  var players := ListMobilesNearLocationEx( who.x, who.y, LIST_IGNORE_Z, range, LISTEX_FLAG_PLAYERS_ONLY,
                                            who.realm );
  foreach player in players
    if ( !CheckLineOfSight( who, player ) )
      if ( temHabilidade( player, "Mestre Espiao" ) )
        players_in_range.append( player.name );
      elseif ( temHabilidade( player, "Espionagem" ) )
        if ( Distance( who, player ) < 12 && abs( who.z - player.z ) < 19 )
          players_in_range.append( player.name );
        endif
      endif
    else
      players_in_range.append( player.name );
    endif

  endforeach
  return players_in_range;
endfunction