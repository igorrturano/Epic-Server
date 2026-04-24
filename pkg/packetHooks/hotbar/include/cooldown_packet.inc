use uo;

include ":hotbar:include/hotbar_write";

const COOLDOWN_TYPE_ABILITY := 0x01;
const COOLDOWN_TYPE_SPECIAL := 0x02;

function SendCooldownPacket(who, name, seconds, iconId, cooldownType)
    var name_len := len(name);
    // PacketID(1) + Length(2) + SubCmd(1) + Type(1) + Seconds(2) + IconId(2) + NameLen(2) + Name(N)
    var packet_size := 11 + name_len;

    var packet := CreatePacket(0xFF, MSGLEN_VARIABLE);
    packet.SetInt8(0, 0xFF);
    // Bytes 1-2: total packet length (required for variable-length packets)
    SetInt16BE(packet, 1, packet_size);
    packet.SetInt8(3, 0x04);
    packet.SetInt8(4, cooldownType);

    var offset := 5;
    offset := SetInt16BE(packet, offset, CInt(seconds));
    offset := SetInt16BE(packet, offset, CInt(iconId));
    offset := SetInt16BE(packet, offset, name_len);
    packet.SetString(offset, name, 0);

    packet.SendPacket(who);
endfunction
