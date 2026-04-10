use uo;
use os;
use polsys;

include "system/exception/ParameterException/InvalidParameterException";
include "include/Packets/packetConstants";


class Packet()
	function Packet(this, type, packet_size := MSGLEN_VARIABLE)
		this.internalPacket := CreatePacket(type,packet_size);
	endfunction

	function Assign(this, newPacket)
		if(!(newPacket is @Packet))
			return InvalidParameterException();
		endif
		
		var retorno := this.internalPacket.Assign(newPacket);
		if(retorno)
			return this;
		endif

		return retorno;		
	endfunction

	//ATENÇÂO! Não use AppendBytes em scripts Criticos
	function AppendBytes(this, type, content, endian := 0, nullterminate := 0)
		case(type)
			PACKET_BYTE_TYPE::BYTE:
			this.SettBytte(this.PacketSize(), content);
				break;

			PACKET_BYTE_TYPE::INT:
				if(endian == PACKET_ENDIAN::LOW)
					this.internalPacket.SetInt16Flipped(this.PacketSize(), content);
				else
					this.SettIntt(this.PacketSize(), content);
				endif
				break;

			PACKET_BYTE_TYPE::DBL:
				if(endian == PACKET_ENDIAN::LOW)
					this.internalPacket.SetInt32Flipped(this.PacketSize(), content);
				else
					this.SetDbl(this.PacketSize(), content);
				endif
				break;

			PACKET_BYTE_TYPE::STR:
				this.SetStr(this.PacketSize(), content, nullterminate);
				break;

			PACKET_BYTE_TYPE::UNI_STR:
				if(endian == PACKET_ENDIAN::LOW)
					this.internalPacket.SetUnicodeStringFlipped(this.PacketSize(), content, nullterminate);
				else
					this.SetUniStr(this.PacketSize(), content, nullterminate);
				endif
				break;
			default:
				return InvalidParameterException();
		endcase

		return this;
	endfunction

	function FillStrip(this, type, times,  endian := 0)
		for i := 1 to times
			this.AppendBytes(type, 0, endian);
		endfor
	endfunction

	function GettBytte(this, offset)
		return this.internalPacket.GetInt8(offset);
	endfunction

	function GettIntt(this, offset)
		return this.internalPacket.GetInt16(offset);
	endfunction

	function GetDbl(this, offset)
		return this.internalPacket.GetInt32(offset);
	endfunction

	function GettStr(this, offset, newLength)
		return this.internalPacket.GetString(offset, newLength);
	endfunction

	function GettUniStr(this, offset, newLength)
		return this.internalPacket.GetUnicodeString(offset, newLength);
	endfunction

	function GettPacket(this)
		return this.internalPacket;
	endfunction
	
	function PacketSize(this)
		return this.internalPacket.GetSize();
	endfunction

	function Send(this, character)
		return this.internalPacket.SendPacket(character);
	endfunction

	function SendArea(this, x, y, range, realm)
		return this.internalPacket.SendAreaPacket(x, y, range, realm);
	endfunction

	function SettBytte(this, offset, value)
		return this.internalPacket.SetInt8(offset, value);
	endfunction

	function SettIntt(this, offset, value)
		return this.internalPacket.SetInt16(offset, value);
	endfunction

	function SettDbll(this, offset, value)
		return this.internalPacket.SetInt32(offset,value);
	endfunction

	function SettStr(this, offset, newString, nullterminate)
		return this.internalPacket.SetString(offset, newString, nullterminate);
	endfunction

	function SettUniStr(this, offset, unicodeArray, nullterminate)
		return this.internalPacket.SetUnicodeString(offset, unicodeArray, nullterminate);
	endfunction

	function SettPacket(this, newPacket)
		this.internalPacket := newPacket;
	endfunction

	function SettSizee(this, newsize)
		return this.internalPacket.SetSize(newsize);
	endfunction

	function PrintPacket(this)
		Print("Packet String: [{}]".format(this.internalPacket));
	endfunction
endclass