////////////////////////////////////////////////
//packets.inc
//  v1.1
//
//  Miscellaneous packet functions,
//    S1tH
//    Thanks to Maximilian Scherr for the following functions:
//      SetPlayerWarMode( who, value )
//      EnableChatButton( who )
//      SendIdleWarning( who )
//
//    sithilius@hotmail.com
//    38288137
////////////////////////////////////////////////

use uo;
use os;
use polsys;

////////////////////////////////////////////////
//Function Listing
////////////////////////////////////////////////
//SetPlayerWarMode( who, value )                   Toggle a players war mode.
//EnableChatButton( who )                          Enable the "chat" button in a player paperdoll. WARNING! See function definition.
//SendIdleWarning( who )                           Sends an idle warning telling the player they will be kicked in 1 minute if they dont do anything.
//UpdatePlayer( who, forwho, graphic )             Shows another player onscreen as having different body, until their next movement.
//PlayCharAnim( who, anim )                        Look at the definition of this one... more options you can pass (speed, forwards/backwards, etc).
//PlayCharAnimPrivate( who, anim, forwho )         See definition for more details and explanation.
//PlayCharAnimWithRepitition( ... )		   See definition for explanation.
//PlayCharAnimWithRepititionPrivate( ... )         See definition for explanation
//PlayMidiMusic( who, musicID )                    Plays some MIDI.
//SetWeatherForAll( type, num )                    Sets weather conditions for all online clients.
//SetWeatherForPlayer( who, type, num )            Sets a players weather conditions.
//DrawGamePlayer( who, body, colour )              Draws the clients character with whatever body graphic you want, without showing other clients.
//NormaliseGamePlayer( who )                       Return what the client sees of themselves to normal.
//SetOverallLightForAll( lightlevel )		   Sets the 'overall' lightlevel for all online characters.
//SetOverallLightForPlayer( who, lightlevel )      Sets the 'overall' lightlevel for a specific character.
//SetCursorHue( who, hue )                         Changes cursor colour between gold (trammel) and normal (felucca?).
//SetSeasonForAll( season )                        Sets the weather season for all online characters.
//SetSeasonForPlayer( who, season )                Sets the weather season for a specific character.
//SetQuestArrow( who, active, xLoc, yLoc )         Sets if the player has a 'quest arrow', and where its pointing to.
////////////////////////////////////////////////

Const CURSOR_NORM := 0; //Felucca / Old UO
Const CURSOR_GOLD := 1; //Trammel i think...

Const SEASON_SPRING := 0;     //Trammel.
Const SEASON_SUMMER := 1;     //Old Britannia
Const SEASON_FALL := 2;       //Fall
Const SEASON_WINTER := 3;     //Screwed attempt by OSI for snow. Snow has no edges and looks fucked.
Const SEASON_DESOLATION := 4; //Felucca - no leaves on trees, gravestones everywhere...

Const WEATHER_STARTS_TO_RAIN := 0;
Const WEATHER_FIERCE_STORM   := 1;
Const WEATHER_BEGINS_SNOW    := 2;
Const WEATHER_STORM_BREWING  := 3;
Const WEATHER_DISABLE        := 0xFF;

Const ANIM_DIR_FORWARDS  := 0; //The animation will run the frames in normal order.
Const ANIM_DIR_BACKWARDS := 1; //The animation will run the frames in reverse.
Const ANIM_DEF_SPEED     := 0; //0 = fastest, 30 = way too fucken slow, 70 = wtf are you doing?

enum PACKET_ID
    PACKET_FEATURES     := 0xB9,
    PACKET_CLILOC       := 0xC1
endenum

enum FEATURE_BITS
    FEATURE_CHAT        := 0x0001,
    FEATURE_LBR         := 0x0002,
    FEATURE_AOS         := 0x0004
endenum

/*
 * SendFeatures(who, features)
 *
 * Purpose
 * Enable OSI client features like the chat button or LBR graphics.
 *
 * Parameters
 * who:         The character on which to enable the features
 * features:    The features to enable, see enumeration above for possible features
 *
 * Return value
 * No return value
 *
 * TODO:
 * Make sure the feature bits are right
 *
 */
function SendFeatures(who, features)
    var packet;

    packet := CreatePacket(PACKET_FEATURES, 0x03);
    packet.SetInt16(1, features);
    packet.SendPacket(who);

    return 1;
endfunction

//////////////////////////////////////////////////////////////////////////////
//Sets a characters war mode. Value:
//  1     War
//  0     Peace
//////////////////////////////////////////////////////////////////////////////
function SetPlayerWarMode( who, value )
  if( !who.acctname )
    if( who.IsA(POLCLASS_NPC) )
      return who.SetWarMode(value);
    endif
  else
    var packetString := "72" + fixPacketLength(value, 1);
    packetString := packetString + "003200";
    return SendPacket(who, packetString);
  endif
endfunction

//////////////////////////////////////////////////////////////////////////////
//Allows the player to click the "chat" button on their paperdoll.
//WARNING! If the player clicks this, POL doesn't recognize it, and will
//complain about it (quite a bit) in the console. Could be useful if you
//have an aux package listening in to enable chat though.
//////////////////////////////////////////////////////////////////////////////
function EnableChatButton( who )
  return SendPacket(who, "B90001");
endfunction

//////////////////////////////////////////////////////////////////////////////
//Sends the client a little gump saying
//"You have been idle for too long. If you do not do anything
// within the next minute, you will be logged out."
//////////////////////////////////////////////////////////////////////////////
function SendIdleWarning( who )
  return SendPacket(who, "5307");
endfunction

//////////////////////////////////////////////////////////////////////////////
//Changes another characters graphic for a specific client
//  who        Character whos graphic to change
//  forwho     Character to change for
//  graphic    New graphic to show
//////////////////////////////////////////////////////////////////////////////
function UpdatePlayer( who, forwho, graphic )
    if( !graphic )
        graphic := who.graphic;
    endif

    var packetString := "77";
    packetString := packetString + fixPacketLength(hex(who.serial),4);
    packetString := packetString + fixPacketLength(hex(graphic),2);
    packetString := packetString + fixPacketLength(hex(who.x),2);
    packetString := packetString + fixPacketLength(hex(who.y),2);
    packetString := packetString + fixPacketLength(hex(who.z),1);
    packetString := packetString + fixPacketLength(hex(who.facing),1);
    packetString := packetString + fixPacketLength(hex(0),2); //Colour (doesnt work)
    packetString := packetString + fixPacketLength(hex(0),1); //Flag... dunno?
    packetString := packetString + fixPacketLength(hex(0),1); //Highlight colour (doesnt work)
    return SendPacket( forwho, packetString );
endfunction

//////////////////////////////////////////////////////////////////////////////
//Animates a character
//  who    Character to animate
//  anim   Animation ID (constant from client.inc)
//  speed  Frame delay (lesser = faster)
//  dir    Play frames backwards or forwards? (use constants at header!!!)
//////////////////////////////////////////////////////////////////////////////
function PlayCharAnim( who, anim, speed := ANIM_DEF_SPEED, dir := ANIM_DIR_FORWARDS )
    var packetString := "6E";
    packetString := packetString + fixPacketLength(hex(who.serial),4);
    packetString := packetString + fixPacketLength(hex(anim),2);
    packetString := packetString + "00";
    packetString := packetString + fixPacketLength(hex(who.facing),1);
    packetString := packetString + "0001"; //Repitition
    packetString := packetString + fixPacketLength(hex(dir),1);
    packetString := packetString + "00"; //Does this animation repeat?
    packetString := packetString + fixPacketLength(hex(speed),1);
    foreach chr in ListMobilesNearLocation( who.x, who.y, who.z, 16 );
        SendPacket( chr, packetString );
    endforeach
endfunction

//////////////////////////////////////////////////////////////////////////////
//Animates a character privately
//  who    Character to animate
//  anim   Animation ID (constant from client.inc)
//  forwho Character that sees the animation
//  speed  Frame delay (lesser = faster)
//  dir    Play frames backwards or forwards? (use constants at header!!!)
//////////////////////////////////////////////////////////////////////////////
function PlayCharAnimPrivate( who, anim, forwho, speed := ANIM_DEF_SPEED, dir := ANIM_DIR_FORWARDS )
    var packetString := "6E";
    packetString := packetString + fixPacketLength(hex(who.serial),4);
    packetString := packetString + fixPacketLength(hex(anim),2);
    packetString := packetString + "00";
    packetString := packetString + fixPacketLength(hex(who.facing),1);
    packetString := packetString + "0001"; //Repitition
    packetString := packetString + fixPacketLength(hex(dir),1);
    packetString := packetString + "00"; //Does this animation repeat?
    packetString := packetString + fixPacketLength(hex(speed),1);
    SendPacket( forwho, packetString );
endfunction

/////////////////////////////////////////////////////////////////////////////////////////
//Repitition is totally wacked mega weird shit...
//It doesnt repeat the same animation that number of times but rather cycles through that number of following animations,
//then at the end i think it plays the original in reverse... why? NFI... test coz im not sure...
//Anyway its handy if you pass "reptimes" as "1", because it will play an animation, then the same in reverse.
// eg. A person falling over, then flying back up again :D
/////////////////////////////////////////////////////////////////////////////////////////
function PlayCharAnimWithRepitition( who, anim, reptimes, speed := ANIM_DEF_SPEED, dir := ANIM_DIR_FORWARDS )
    var packetString := "6E";
    packetString := packetString + fixPacketLength(hex(who.serial),4);
    packetString := packetString + fixPacketLength(hex(anim),2);
    packetString := packetString + "00";
    packetString := packetString + fixPacketLength(hex(who.facing),1);
    packetString := packetString + fixPacketLength(hex(reptimes),2);
    packetString := packetString + fixPacketLength(hex(dir),1);
    packetString := packetString + "01"; //Does this animation repeat?
    packetString := packetString + fixPacketLength(hex(speed),1);
    foreach chr in ListMobilesNearLocation( who.x, who.y, who.z, 16 );
        SendPacket( chr, packetString );
    endforeach
endfunction

/////////////////////////////////////////////////////////////////////////////////////////
//Above, but only seen by 'forwho'
/////////////////////////////////////////////////////////////////////////////////////////
function PlayCharAnimWithRepititionPrivate( who, anim, reptimes, forwho, speed := ANIM_DEF_SPEED, dir := ANIM_DIR_FORWARDS )
    var packetString := "6E";
    packetString := packetString + fixPacketLength(hex(who.serial),4);
    packetString := packetString + fixPacketLength(hex(anim),2);
    packetString := packetString + "00";
    packetString := packetString + fixPacketLength(hex(who.facing),1);
    packetString := packetString + fixPacketLength(hex(reptimes),2);
    packetString := packetString + fixPacketLength(hex(dir),1);
    packetString := packetString + "01"; //Does this animation repeat?
    packetString := packetString + fixPacketLength(hex(speed),1);
    SendPacket( forwho, packetString );
endfunction

//////////////////////////////////////////////////
//Plays MIDI music (untested)
//  who      Person who hears the music
//  musicID  MIDI to play, same as "playmusic.inc"
//////////////////////////////////////////////////
function PlayMidiMusic( who, musicID )
    var packetString := "6D" + fixPacketLength(hex(musicID),2);
    return SendPacket(who,packetString);
endfunction

///////////////////////////////////////////////////////
//Sets weather for all online clients:
//  type   Weather type. Constants at header.
//  num    Number of weather effects (dots?) onscreen. Max is 70.
//  temp   Temperature. Currently has no effect...
///////////////////////////////////////////////////////
function SetWeatherForAll( type, num, temp := 0 )
    if( num > 70 )
        num := 70;
    endif

    var packetString := "65";
    packetString := packetString + fixPacketLength(hex(type),1);
    packetString := packetString + fixPacketLength(hex(num),1);
    packetString := packetString + fixPacketLength(hex(temp),1);

    foreach chr in EnumerateOnlineCharacters()
        SendPacket( chr, packetString );
    endforeach
endfunction

///////////////////////////////////////////////////////
//Sets weather for a specific client:
//  type   Weather type. Constants at header.
//  num    Number of weather effects (dots?) onscreen. Max is 70.
//  temp   Temperature. Currently has no effect...
///////////////////////////////////////////////////////
function SetWeatherForPlayer( who, type, num, temp := 0 )
    if( num > 70 )
        num := 70;
    endif

    var packetString := "65";
    packetString := packetString + fixPacketLength(hex(type),1);
    packetString := packetString + fixPacketLength(hex(num),1);
    packetString := packetString + fixPacketLength(hex(temp),1);
    return SendPacket(who,packetString);
endfunction

////////////////////////////////////////////////////
//Changes how the clients character is drawn, like
//them polymorphing but no-one else sees it :)
//  who     Client whos character will change
//  body    Body graphic to change to
//  colour  Colour for the clients body
////////////////////////////////////////////////////
function DrawGamePlayer( who, body, colour )
    var packetString := "20";
    packetString := packetString + fixPacketLength(hex(who.serial),4);
    packetString := packetString + fixPacketLength(hex(body),2);
    packetString := packetString + "00";
    packetString := packetString + fixPacketLength(hex(colour),2);
    packetString := packetString + "00";
    packetString := packetString + fixPacketLength(hex(who.x),2);
    packetString := packetString + fixPacketLength(hex(who.y),2);
    packetString := packetString + "0000";
    packetString := packetString + fixPacketLength(hex(who.facing),1);
    packetString := packetString + fixPacketLength(hex(who.z),1);
    return SendPacket( who, packetString );
endfunction

///////////////////////////////////////////////////////
//Tells a client what their character really looks like
//  who    Person to send to
///////////////////////////////////////////////////////
function NormaliseGamePlayer( who )
    return DrawGamePlayer( who, who.graphic, who.color );
endfunction

///////////////////////////////////////////////////////
//Sets the 'overall' (world) light for everyone online
//  light    Light level. 0 = daylight, 30 = black.
///////////////////////////////////////////////////////
function SetOverallLightForAll( light )
    var packetString := "4F" + fixPacketLength(hex(light),1);
    foreach chr in EnumerateOnlineCharacters()
        SendPacket(chr,packetString);
    endforeach
endfunction

///////////////////////////////////////////////////////////
//Sets the 'overall' (world) light for a specific character
//  who     Person whos light to change.
//  light   Light level. 0 = daylight, 30 = black.
///////////////////////////////////////////////////////////
function SetOverallLightForPlayer( who, light )
    var packetString := "4F" + fixPacketLength(hex(light),1);
    return SendPacket(who,packetString);
endfunction

///////////////////////////////////////////////////////////
//Change a clients cursor colour, I think this is 2.0.0+
//  who      Clients cursor to change.
//  hue      Hue type, see constants at header.
///////////////////////////////////////////////////////////
function SetCursorHue( who, hue )
    var packetString := "BF";
    packetString := packetString + fixPacketLength(6,2);
    packetString := packetString + fixPacketLength("8",2);
    packetString := packetString + fixPacketLength(hue,1);
    return SendPacket(who,packetString);
endfunction

///////////////////////////////////////////////////////////
//Changes season for all online characters...
//  season    Season type. Constants at header.
///////////////////////////////////////////////////////////
function SetSeasonForAll( season )
    var packetString := "BC" + fixPacketLength(season,1) + "01";
    foreach chr in EnumerateOnlineCharacters()
        SendPacket(chr,packetString);
    endforeach
endfunction

///////////////////////////////////////////////////////////
//Changes season for a specific character
//  who      Character to change for
//  season   Season type. Constants at header.
///////////////////////////////////////////////////////////
function SetSeasonForPlayer( who, season )
    var packetString := "BC" + fixPacketLength(season,1) + "01";
    SendPacket(who,packetString);
    return;
endfunction

///////////////////////////////////////////////////////////
//Quest arrow pointing to a location. Must be disabled too!
//  who       Person who sees the quest arrow
//  active    Arrow Status. 0 = off, non-zero = on.
//  xLoc      X Location to point to.
//  yLoc      Y Location to point to.
///////////////////////////////////////////////////////////
function SetQuestArrow( who, active, xLoc, yLoc )
    var packetString := "BA" + fixPacketLength(active,1);
    packetString := packetString + fixPacketLength(hex(xLoc),2);
    packetString := packetString + fixPacketLength(hex(yLoc),2);
    print( packetString );
    return SendPacket(who,packetString);
endfunction

///////////////////////////////////////////////////////////////
//Pads a string with a "0" prefix, until it is of proper length
//  packetString   value to pad.
//  byteLength     number of bytes. Byte counted as two chars.
///////////////////////////////////////////////////////////////
function fixPacketLength( packetString, byteLength )
    if( !packetString || packetString == error )
        packetString := 0;
    endif
    packetString := cStr(packetString);

    if( packetString[1,2] == "0x" )
        packetString := cStr(packetString[3,len(packetString)]);
    endif

    if( len(packetString) > byteLength * 2 )
//		var extraBytes := (byteLength*2) - len(packetString);
		var extraBytes := len(packetString) - (byteLength*2) + 1;
		return cStr(packetString[extraBytes,len(packetString)]);
    endif

    while( len(packetString) < byteLength * 2 )
        packetString := "0" + packetString;
    endwhile

    return packetString;
endfunction


function PlayObjectCenteredEffectHuefx( center, effect, speed, loop := 0, color := 0, translucent := 0 )
    var packetString := "C0";
    packetString := packetString + "03";
    packetString := packetString + fixPacketLength(hex(center.serial),4);
    packetString := packetString + fixPacketLength(hex(0),4);
    packetString := packetString + fixPacketLength(hex(effect),2);
    packetString := packetString + fixPacketLength(hex(0),10);
    packetString := packetString + fixPacketLength(hex(speed),1); //speed
    packetString := packetString + fixPacketLength(hex(loop),1); //loop
    packetString := packetString + "000000000000" ;
    packetString := packetString + fixPacketLength(hex(color),2); //color
    packetString := packetString + "000000";
    packetString := packetString + fixPacketLength(hex(translucent),1); //tran
    foreach chr in ListMobilesNearLocation( center.x, center.y, center.z, 16 );
        SendPacket( chr, packetString );
    endforeach
endfunction

function PlayMovingEffectHuefx( source, dest, effect, speed, loop :=0, explode := 0, color := 0, translucent := 0)
    var packetString := "C0";
    packetString := packetString + "00";
    packetString := packetString + fixPacketLength(hex(source.serial),4);
    packetString := packetString + fixPacketLength(hex(dest.serial),4);
    packetString := packetString + fixPacketLength(hex(effect),2);
    packetString := packetString + fixPacketLength(hex(0),10);
    packetString := packetString + fixPacketLength(hex(speed),1); //speed
    packetString := packetString + fixPacketLength(hex(loop),1); //loop
    packetString := packetString + "000000" ;
    packetString := packetString + fixPacketLength(hex(explode),1); //explode
    packetString := packetString + "0000";
    packetString := packetString + fixPacketLength(hex(color),2); //color
    packetString := packetString + "000000";
    packetString := packetString + fixPacketLength(hex(translucent),1); //tran
    foreach chr in ListMobilesNearLocation( source.x, source.y, source.z, 16 );
        SendPacket( chr, packetString );
    endforeach
endfunction

function PlayMovingEffectXYZHuefx(source, sourcex, sourcey, sourcez, destx, desty, destz, effect, speed, loop := 0, explode := 0, color := 0, translucent := 0)
    var packetString := "C0";
    packetString := packetString + "00";
    packetString := packetString + fixPacketLength(hex(0),8);
    packetString := packetString + fixPacketLength(hex(effect),2);
    packetString := packetString + fixPacketLength(hex(sourcex),2);
    packetString := packetString + fixPacketLength(hex(sourcey),2);
    packetString := packetString + fixPacketLength(hex(sourcez),1);
    packetString := packetString + fixPacketLength(hex(destx),2);
    packetString := packetString + fixPacketLength(hex(desty),2);
    packetString := packetString + fixPacketLength(hex(destz),1);
    packetString := packetString + fixPacketLength(hex(speed),1); //speed
    packetString := packetString + fixPacketLength(hex(loop),1); //loop
    packetString := packetString + "000000";
    packetString := packetString + fixPacketLength(hex(explode),1); //explode
    packetString := packetString + "0000";
    packetString := packetString + fixPacketLength(hex(color),2); //color
    packetString := packetString + "000000";
    packetString := packetString + fixPacketLength(hex(translucent),1); //tran
    foreach chr in ListMobilesNearLocation( source.x, source.y, source.z, 16 );
        SendPacket( chr, packetString );
    endforeach
endfunction

function PlayStationaryEffectHuefx( x, y, z, effect, speed, loop := 0, explode := 0, color := 0,translucent := 0)
    var packetString := "C0";
    packetString := packetString + "02";
    packetString := packetString + fixPacketLength(hex(0),8);
    packetString := packetString + fixPacketLength(hex(effect),2);
    packetString := packetString + fixPacketLength(hex(x),2);
    packetString := packetString + fixPacketLength(hex(y),2);
    packetString := packetString + fixPacketLength(hex(z),1);
    packetString := packetString + fixPacketLength(hex(0),5);
    packetString := packetString + fixPacketLength(hex(speed),1); //speed
    packetString := packetString + fixPacketLength(hex(loop),1); //loop
    packetString := packetString + "000000";
    packetString := packetString + fixPacketLength(hex(explode),1); //explode
    packetString := packetString + "0000";
    packetString := packetString + fixPacketLength(hex(color),2); //color
    packetString := packetString + "000000";
    packetString := packetString + fixPacketLength(hex(translucent),1); //tran
    foreach chr in ListMobilesNearLocation( x, y, z, 16 );
        SendPacket( chr, packetString );
    endforeach
endfunction

function PlayMovingEffectXYZHued(source, dest, effect, speed, loop:=0, explode:=0, color:=0, translucent:=0)
    var packet := CreatePacket(0xC0, 36);
    packet.SetInt8(1, 0x00);
    packet.SetInt32(2, source.serial);
    packet.SetInt32(6, dest.serial);
    packet.SetInt16(10, effect);
    packet.SetInt16(12, source.x);
    packet.SetInt16(14, source.y);
    packet.SetInt8(16, source.z);
    packet.SetInt16(17, dest.x);
    packet.SetInt16(19, dest.y);
    packet.SetInt8(21, dest.z);
    packet.SetInt8(22, speed);
    packet.SetInt8(23, loop);
    packet.SetInt16(24, 0);
    packet.SetInt8(26, 0);
    packet.SetInt8(27, explode);
    packet.SetInt32(28, color);
    packet.SetInt32(32, translucent);
    packet.SendAreaPacket(source.x, source.y, 16, source.realm);
endfunction

function GetBuffIcon(name)

    if (name == "strength")
        return 1047;
    elseif (name == "intelligence")
        return 1046;
    elseif (name == "DefaultPoison")
        return 1038;
    elseif (name == "agility")
        return 1045;
    elseif (name == "bleed")
        return 1039;
    elseif ( name == "invisibilidade")
        return 1036;
    elseif ( name == "Hallucination")
        return 1017; //mindrot
    elseif ( name == "Paralysis")
        return 1037;
    elseif ( name == "PhysicalResist")
        return 1029;
    elseif ( name == "MagicResist")
        return 1031;
    elseif (name == "bless")
        return 1048;
    elseif (name == "curse")
        return 1043;
    elseif (name == "enmod")
        return 1022;
    elseif ( name == "phmod")
        return 1030;
    endif

endfunction

function SendBuff(mobile, show,  icon, duration := 0, cliloc1 := 0, cliloc2 := 0, str := 0 )

//  return 1;
   if ( show )
      show := CInt(show);
   else
      show := 0;
   endif

   if (str)
    str := " " + str;
   endif
   sendsysmessage(mobile, "s" + show +  " " + icon + " " + duration + " " + cliloc1 +  " " + cliloc2) ;
   // define the offsets to make it easier
   var   OFFSET_SERIAL := 3;
   var   OFFSET_ICONNUMBER1 := 7;
   var   OFFSET_SHOW1 := 9;
   var   OFFSET_ICONNUMBER2 := 15;
   var   OFFSET_SHOW2 := 17;
   var   OFFSET_TIME := 23;
   var   OFFSET_CLILOC1 := 28;
   var   OFFSET_CLILOC2 := 32;
   var   OFFSET_CLILOC2PARMS := 42;

//   var   BLANK_CLILOC := 1005007;   // a cliloc number with blank text
//   var   ERROR_CLILOC := 500267;      // an cliloc showing an error


   var packet := CreatePacket(0xDF, MSGLEN_VARIABLE);
   packet.setint32(OFFSET_SERIAL, mobile.serial);
   packet.setint16(OFFSET_ICONNUMBER1, icon);
   packet.setint16(OFFSET_SHOW1, show);
   if ( show )
      packet.setint16(OFFSET_ICONNUMBER2, icon);
      packet.setint16(OFFSET_SHOW2, show);
      packet.setint16(OFFSET_TIME, duration);
      packet.setint32(OFFSET_CLILOC1, cliloc1);
      packet.setint32(OFFSET_CLILOC2, cliloc2);
      packet.SetUnicodeStringFlipped(OFFSET_CLILOC2PARMS, CAscZ(str), 1);
      packet.setint16(1, packet.GetSize() );
   endif
   packet.sendpacket(mobile);
endfunction

function NewCreatePacket( id )

    var packet := cstr(id);
    return packet;

endfunction

function SetInt32( int32)

    var int1, int2, int3, int4;
    int1 := (int32 > 24) & 0xFF;
    int2 := (int32 > 16) & 0xFF;
    int3 := (int32 > 8)  & 0xFF;
    int4 := (int32 > 0)  & 0xFF;

    var str := cstr(int1) + cstr(int2) + cstr(int3) + cstr(int4);
    return str;

endfunction

function SetInt16 (int16)

    var int1, int2;
    int1 := (int16 > 8) & 0xFF;
    int2 := (int16 > 0) & 0xFF;

    var str := cstr(int1) + cstr(int2);

    return str;

endfunction

///////////////////////////////////////////////////////////
// Close Gump
///////////////////////////////////////////////////////////

function CloseGumpPacket(user, gump_pid, ret_val)
    var packet:="BF" + "000D" + "0004" + MyHex(gump_pid) + FixPacketLength(ret_val, 4);
    return SendPacket(user, packet);
endfunction

///////////////////////////////////////////////////////////
// Speed Boost
///////////////////////////////////////////////////////////
function PlayerSpeedBoost(player, toggle:=0)
       if ( toggle )
               return SendPacket(player, "BF0006002601");
       else
               return SendPacket(player, "BF0006002600");
       endif
endfunction

function SendSkillUpdate(who, Skill)
  Skill := GetSkillIdByAttributeId(Skill);
  var SkillLock:=GetObjProperty(who, "skill");
  var Packet:="";
  var SkillID:=Hex(Skill)-"0x";
  if(Skill<=0xF)
    SkillID:="0"+SkillID;
  endif
  var Value:=hex(GetBaseSkillBaseValue(who, Skill))-"0x";
  var theskillid:=GetAttributeIdBySkillId(Skill);
  var modval:=hex(GetAttributeTemporaryMod( who, theskillid )+GetBaseSkillBaseValue(who, Skill))-"0x";
  Fix(Value);
  Fix(SkillID);
  Fix(modval);
  if (SkillLock[Skill+1]==1)
    Packet:="3A000BFF"+SkillID+modval+Value+"00";
  elseif (SkillLock[Skill+1]==2)
    Packet:="3A000BFF"+SkillID+modval+Value+"01";
  else
    Packet:="3A000BFF"+SkillID+modval+Value+"02";
  endif

  SendPacket(who, Packet);

endfunction

function SendSkillArray(who)
  var SkillLock:=GetObjProperty(who, "skill");
  var Skills:="";
  var Packet:="";
  var Value;
  var Skill;
  for (Skill:=0; Skill<=51; Skill:=Skill+1)
    var SkillID:=Hex(Skill+1)-"0x";
    if(Skill<=0xF)
      SkillID:="0"+SkillID;
    endif
    Value:=hex(GetBaseSkillBaseValue(who, Skill))-"0x";
    var theskillid:=GetAttributeIdBySkillId(Skill);
    var modval:=hex(GetAttributeTemporaryMod( who, theskillid )+GetBaseSkillBaseValue(who, Skill))-"0x";
    Fix(Value);
    Fix(SkillID);
    Fix(modval);
    if (SkillLock[Skill+1]==1)
      Skills:=Skills+SkillID+modval+Value+"00";
    elseif (SkillLock[Skill+1]==2)
      Skills:=Skills+SkillID+modval+Value+"01";
    else
      Skills:=Skills+SkillID+modval+Value+"02";
    endif
  endfor
  var size:=len(Skills)/2;
  size:=size+6;
  size:=hex(size)-"0x";
  Fix(size);
  Packet:="3A"+size+"00"+Skills+"0000";
  SendPacket(who, Packet);
endfunction

function MostraDano(attacker, defender, dmg)
    var thepacket := "BF000B002201" + fixPacketLength(hex(defender.serial),4)+ fixPacketLength(hex(dmg),1);
    SendPacket(attacker, thepacket);
endfunction

function DisplayDamage(damaged, amount, source:=0)
    var dmg_packet := CreatePacket(0x0B, 0x07);

    if( amount <= 255 )
        dmg_packet.SetInt8(0x06, amount);
    else
        dmg_packet.SetInt16(0x05, amount);
    endif

    dmg_packet.SetInt32(0x01, damaged.serial);

    if ( source.acctname )
        dmg_packet.SendPacket(source);
    endif

    if( damaged.acctname )
        dmg_packet.SendPacket(damaged);
    endif

    return 1;
endfunction

function FixSize(byref value, fixamt)

  var i;
  for(i := 1; i <= CInt(fixamt); i := i+1)
    value := "0" + value;
  endfor

endfunction

function Fix(byref value)
  if(len(value)==1)
    value:="000"+value;
  elseif(len(value)==2)
    value:="00"+value;
  elseif(len(value)==3)
    value:="0"+value;
  elseif(len(value)==4)
  endif
endfunction

///////////////////////////////////////////////////////////////
// Taken from MyHex.inc by Assargadon on polserver.com
//////////////////////////////////////////////////////////////

function MyHex(i)
//convert i to 8-symbols hex form

    var dig:=i/(16*16*16*16*16*16*16);
    i:=i%(16*16*16*16*16*16*16);

    if (dig>=10)
        dig:=65+dig-10;
    else
        dig:=48+dig;
    endif

    var res:=CChr(dig);

    dig:=i/(16*16*16*16*16*16);
    i:=i%(16*16*16*16*16*16);
    if (dig>=10)
        dig:=65+dig-10;
    else
        dig:=48+dig;
    endif
    res:=res+CChr(dig);

    dig:=i/(16*16*16*16*16);
    i:=i%(16*16*16*16*16);
    if (dig>=10)
        dig:=65+dig-10;
    else
        dig:=48+dig;
    endif
    res:=res+CChr(dig);

    dig:=i/(16*16*16*16);
    i:=i%(16*16*16*16);
    if (dig>=10)
        dig:=65+dig-10;
    else
        dig:=48+dig;
    endif
    res:=res+CChr(dig);

    dig:=i/(16*16*16);
    i:=i%(16*16*16);
    if (dig>=10)
        dig:=65+dig-10;
    else
        dig:=48+dig;
    endif
    res:=res+CChr(dig);

    dig:=i/(16*16);
    i:=i%(16*16);
    if (dig>=10)
        dig:=65+dig-10;
    else
        dig:=48+dig;
    endif
    res:=res+CChr(dig);

    dig:=i/(16);
    i:=i%(16);
    if (dig>=10)
        dig:=65+dig-10;
    else
        dig:=48+dig;
    endif
    res:=res+CChr(dig);

    dig:=i;
    if (dig>=10)
        dig:=65+dig-10;
    else
        dig:=48+dig;
    endif
    res:=res+CChr(dig);

    return res;
endfunction

///////////////////////////////////////////////////////////
// Speed Slower
///////////////////////////////////////////////////////////
function PlayerSpeedSlower(player, toggle:=0)
       if ( toggle )
            return SendPacket(player, "BF0006002602");
       else
            return SendPacket(player, "BF0006002600");
       endif
endfunction