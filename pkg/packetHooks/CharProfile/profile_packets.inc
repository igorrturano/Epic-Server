// =============================================================================
// Profile URL Packet Functions
// Event-driven profile URL updates (like XP system)
// Subcommand 0x58 for packet 0xD7
// =============================================================================

use uo;
use polsys;
 
const SUBCOMMAND_PROFILE_URL := 0x58;

// Send profile URL to a specific client
// who: the client to send to
// target_serial: the serial of the entity whose profile this is
// url: the profile image URL
function UpdateProfileClient(who, target_serial, url)
    if (!url || url == "")
        return;
    endif

    var url_len := Len(url) + 1;  // +1 for null terminator
    var packet_len := 1 + 2 + 2 + 4 + url_len;  // ID + Len + Sub + Serial + URL

    var packet := CreatePacket(0xD7, MSGLEN_VARIABLE);
    packet.SetInt16(1, packet_len);
    packet.SetInt16(3, SUBCOMMAND_PROFILE_URL);  // 0x58
    packet.SetInt32(5, target_serial);
    packet.SetString(9, url, 1);  // null-terminated

    packet.SendPacket(who);
endfunction

// Broadcast profile update to all party members (excluding self)
function BroadcastProfileToParty(who)
    var party := who.party;
    if (!party)
        return;
    endif

    var url := GetObjProperty(who, "url");
    if (!url || url == "")
        return;
    endif

    foreach member in (party.members)
        if (member.serial != who.serial)
            UpdateProfileClient(member, who.serial, url);
        endif
    endforeach
endfunction

// Send all party member profiles to a player (used on login)
function SendPartyMemberProfiles(who)
    var party := who.party;
    if (!party)
        return;
    endif

    foreach member in (party.members)
        var url := GetObjProperty(member, "url");
        if (url && url != "")
            UpdateProfileClient(who, member.serial, url);
        endif
    endforeach
endfunction
