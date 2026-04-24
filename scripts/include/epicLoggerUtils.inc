use uo;
use os;
use file;
include "include/client";
include "include/sysEvent";
include "include/say";
include "include/epicUtils";

// function TranslateText(text, target_language, original_language)
//   var apiURL := "localhost:5000/epic-brain/translate";
//   var jsonData := PackJSON(struct{
//     "message" := text,
//     "target_language" := Lower(target_language),
//     "original_language" := Lower(original_language)
//   });

//   var apiResponse := HTTPRequest(
//         apiURL,
//         "POST",
//         struct{
//           data := jsonData,
//           headers := struct{
//               "Content-Type" := "application/json"
//           }
//         },
//         HTTPREQUEST_EXTENDED_RESPONSE
//     );

//   if (apiResponse.status == 200)
//     var unpackedJSON := UnpackJSON(apiResponse.body);
//     return unpackedJSON["message"];
//   else
//     return text; // Return original text if translation fails
//   endif
// endfunction

function LogPlayerSpeech( who, text )
  if ( text == "..." || text == " . . . " || text == ". . ." )
    return 0;
  endif

  if ( !who || who == error )
    return 0;
  endif

  var account_name := who.acctname;
  if ( account_name == error || !account_name )
    return 0;
  endif

  if (who.npctemplate || who.npcdesc)
    return 0;
  endif

  var character_name := "";
  var position := "{}:{}:{}".format( who.x, who.y, who.z );
  var timestamp := StrFormatTime( "%d-%m-%Y %H:%M:%S", POLCore().systime );

  if ( GetObjProperty( who, "nome_inicial" ) )
    character_name := GetObjProperty( who, "nome_inicial" );
  else
    character_name := who.name;
  endif

  if ( character_name == error || !character_name )
    return 0;
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
      if ( MobileWrapper( player ).HasAbility(ABILITY_ID::STEALTH_RUN) )
        players_in_range.append( player.name );
      elseif ( MobileWrapper( player ).HasAbility(ABILITY_ID::NINJA_PATH) )
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

function LogCombatAction(npc, state, ability, success)
  var npc_elem := NPC_GetNPCConfig(npc);
  var class_type := npc_elem.Classe;
  var log_dir := "::data/log/npc_combat/{}/".format(class_type);
  
  if (!FileExists(log_dir))
     CreateDirectory(log_dir);
  endif

  var npc_template := NPC_ParseTemplateName(npc.npctemplate).template;
  var log_file := "{}{}.log".format(log_dir, npc_template);

  // Get opponent info
  var opponent := me.opponent;
  var opponent_class := opponent.acctname ? GetObjProperty(opponent, "chardata").classe : NPC_GetNPCConfig(opponent).Classe;
  var opponent_type := opponent.acctname ? opponent.acctname : NPC_ParseTemplateName(opponent.npctemplate).template;

  var log_entry := "{},{},{},{},{},{},{},{},{},{},{},{}\n".format(
     FormatState(state),
     ability,
     CInt(success),
     StrFormatTime( "%d-%m-%Y %H:%M:%S", POLCore().systime ),
     opponent_class,
     opponent_type, 
     AP_GetVital(npc, HITS),
     AP_GetVitalMaximumValue(npc, HITS),
     AP_GetVital(opponent, HITS),
     AP_GetVitalMaximumValue(opponent, HITS),
     opponent.weapon.isRanged(),
     state.distance
  );

  LogToFile(log_file, log_entry, 0);
endfunction

function LoadCombatHistory(npc_template)
   var log_file := "::data/log/npc_combat/{}.log".format(npc_template);
   if (!FileExists(log_file))
      return dictionary;
   endif

   var success_rates := dictionary;
   var lines := ReadFile(log_file);
    print("LineS: " + lines);
   foreach line in lines
      print("line: " + line);
      var data := SplitWords(line, ",");
      var state := data[2];
      var ability := data[3];
      var success := CInt(data[4]);
      print("state: " + state);
      
      var key := state + "_" + ability;
      if (!success_rates[key])
         success_rates[key] := struct{
            "successes" := 0,
            "total" := 0
         };
      endif
      
      success_rates[key].total += 1;
      if (success)
         success_rates[key].successes += 1;
      endif
      sleepms(2);
   endforeach

   return success_rates;
endfunction

function FormatState(state)
   return "{}|{}|{}|{}|{}".format(
      state.health,  // "dying", "hurt", "injured", "healthy"
      state.distance,
      state.stamina,
      CInt(state.opponent_casting),
      CInt(state.has_advantage)
   );
endfunction

function LogAttackRequest( attacker, defender )
  if ( !attacker || attacker == error )
    return 0;
  endif

  var acctname := attacker.acctname;
  if ( acctname == error || !acctname )
    return 0;
  endif

  if ( !defender || defender == error )
    return 0;
  endif

  // Dedup: mesmo atacante → mesmo alvo dentro de 2s = skip (client spam / re-engage)
  var now := POLCore().systime;
  var last_target := GetObjProperty( attacker, "#lastAtkSerial" );
  var last_time := GetObjProperty( attacker, "#lastAtkTime" );
  if ( last_target == defender.serial && last_time != error && last_time && (now - last_time) < 2 )
    SetObjProperty( attacker, "#lastAtkTime", now );
    return 0;
  endif
  SetObjProperty( attacker, "#lastAtkSerial", defender.serial );
  SetObjProperty( attacker, "#lastAtkTime", now );

  var atk_char := "";
  if ( GetObjProperty( attacker, "nome_inicial" ) )
    atk_char := GetObjProperty( attacker, "nome_inicial" );
  else
    atk_char := attacker.name;
  endif
  if ( atk_char == error || !atk_char )
    atk_char := "?";
  endif

  var atk_coord := "{}:{}:{}".format( attacker.x, attacker.y, attacker.z );

  var tgt_serial := Hex( defender.serial );
  var tgt_name := defender.name;
  if ( tgt_name == error || !tgt_name )
    tgt_name := "?";
  endif
  // Sanitiza vírgula no nome pra não quebrar o parse CSV
  tgt_name := StrReplace( tgt_name, ",", ";" );

  var tgt_type := "Player";
  var tgt_master_acct := "";
  var tgt_template := "";
  if ( defender.npctemplate )
    tgt_template := defender.npctemplate;
    if ( defender.master )
      tgt_type := "Pet";
      if ( defender.master.acctname && defender.master.acctname != error )
        tgt_master_acct := defender.master.acctname;
      endif
    else
      tgt_type := "NPC";
    endif
  endif

  var timestamp := StrFormatTime( "%d-%m-%Y %H:%M:%S", POLCore().systime );
  var today := StrFormatTime( "%d-%m-%Y", POLCore().systime );

  var log_dir := "::data/log/combat/{}".format( today );
  if ( !FileExists( log_dir ) )
    CreateDirectory( log_dir );
  endif

  var log_file := "{}/attacks.log".format( log_dir );

  var log_entry := "{},{},{},{},{},{},\"{}\",{},{},{}\n".format(
    timestamp, acctname, atk_char, atk_coord, attacker.realm,
    tgt_serial, tgt_name, tgt_type, tgt_template, tgt_master_acct
  );

  LogToFile( log_file, log_entry, 0 );
  return 1;
endfunction