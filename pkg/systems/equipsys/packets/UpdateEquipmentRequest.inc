include "include/Packets/packetConstants";
include "include/Packets/Model/Packet";
include ":wrappers:Objects/Model/MobileWrapper";
include ":wrappers:Objects/Model/ArmorWrapper";
include ":wrappers:Objects/Model/WeaponWrapper";
include ":combat:Controller/DamageController";
include ":combat:Model/AttackInfo";
include "include/math";

const QTD_RESISTENCIAS := 7;
const WEAPON_UPDATE_FLAG := 0x1;
const ARMOR_UPDATE_FLAG := 0x2;
const PACKET_SIZE_INDEX := 1;
const OACKET := 1;

var ARMOR_RESISTENCE_PACKET_ARRAY := array{LAYER_SHOES,LAYER_HELM,LAYER_GLOVES,LAYER_NECK,LAYER_CHEST,LAYER_ARMS,LAYER_PANTS};

function SendUpdateEquipmentRequest(quem, equip, isEquipping, layer := 0)
	if(!quem || !equip || quem.IsNpc())
		return 0;
	endif

	if(quem.IsA(POLCLASS_MOBILE))
		quem := MobileWrapper(quem);
	endif

	var wrappedEquip;
	if (equip.IsA(POLCLASS_ARMOR))
		wrappedEquip := ArmorWrapper(equip);
	elseif (equip.IsA(POLCLASS_WEAPON))
		wrappedEquip := WeaponWrapper(equip);
	endif

	if(!(wrappedEquip is @EquipWrapper))
		return 0;
	endif

	var packet := Packet(PACKET_ID_GENERIC_COMMANDS);

	packet.AppendBytes(PACKET_BYTE_TYPE::INT,0) //valorTemporario, é alterado ao final.
		.AppendBytes(PACKET_BYTE_TYPE::INT,PACKET_ID_GENERIC_COMMANDS_SUBCMD_UPDT_EQUIP);

	if(wrappedEquip is @ArmorWrapper)
		packet.AppendBytes(PACKET_BYTE_TYPE::BYTE,ARMOR_UPDATE_FLAG)
			  .AppendBytes(PACKET_BYTE_TYPE::BYTE,layer);

		if(isEquipping)
			var spellResist := quem.GetSpellResist();
			var resistencias := wrappedEquip.GetResists();
				packet.AppendBytes(PACKET_BYTE_TYPE::BYTE, (resistencias.GetSlashResist() ? resistencias.GetSlashResist() : 0) + spellResist.GetSlashResist())
					.AppendBytes(PACKET_BYTE_TYPE::BYTE, (resistencias.GetPiercingResist() ? resistencias.GetPiercingResist() : 0) + spellResist.GetPiercingResist())
					.AppendBytes(PACKET_BYTE_TYPE::BYTE, (resistencias.GetBluntResist() ? resistencias.GetBluntResist() : 0) + spellResist.GetBluntResist())
					.AppendBytes(PACKET_BYTE_TYPE::BYTE, (resistencias.GetFireResist() ? resistencias.GetFireResist() : 0) + spellResist.GetFireResist())
					.AppendBytes(PACKET_BYTE_TYPE::BYTE, (resistencias.GetColdResist() ? resistencias.GetColdResist() : 0) + spellResist.GetColdResist())
					.AppendBytes(PACKET_BYTE_TYPE::BYTE, (resistencias.GetPoisonResist() ? resistencias.GetPoisonResist() : 0) + spellResist.GetPoisonResist())
					.AppendBytes(PACKET_BYTE_TYPE::BYTE, (resistencias.GetEnergyResist() ? resistencias.GetEnergyResist() : 0) + spellResist.GetEnergyResist());
		else
			packet.FillStrip(PACKET_BYTE_TYPE::BYTE, 7);

		endif
	elseif (wrappedEquip is @WeaponWrapper)
		
		packet.AppendBytes(PACKET_BYTE_TYPE::BYTE,WEAPON_UPDATE_FLAG);
		
		if(isEquipping)
			
			var attackInfo := AttackInfo(quem, 0);
			attackInfo.SetWeapon(wrappedEquip);
			var mainHandDmg := CalculateEstimatedMinMaxDamage(attackInfo);
			var offHandDmg := struct{ min := 0, max := 0 };

			if (attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetAttribute() == DUALWIELD)
				attackInfo.SetDamageHand(DAMAGE_HAND::OFFHAND);
				offHandDmg := CalculateEstimatedMinMaxDamage(attackInfo);
			endif

			packet
				.AppendBytes(PACKET_BYTE_TYPE::INT,mainHandDmg.min)
				.AppendBytes(PACKET_BYTE_TYPE::INT,mainHandDmg.max)
				.AppendBytes(PACKET_BYTE_TYPE::INT,offHandDmg.min)
				.AppendBytes(PACKET_BYTE_TYPE::INT,offHandDmg.max)
				.AppendBytes(PACKET_BYTE_TYPE::INT,mainHandDmg.Damage.GetFireRawDamage())
				.AppendBytes(PACKET_BYTE_TYPE::INT,mainHandDmg.Damage.GetColdRawDamage())
				.AppendBytes(PACKET_BYTE_TYPE::INT,mainHandDmg.Damage.GetPoisonRawDamage())
				.AppendBytes(PACKET_BYTE_TYPE::INT,mainHandDmg.Damage.GetEnergyRawDamage());
		else
			packet.FillStrip(PACKET_BYTE_TYPE::INT, 8);
		endif
	else
		return 0;
	endif
	packet.SetInt(PACKET_SIZE_INDEX,packet.Length());
	packet.Send(quem.GetObjRef());

	return 1;
endfunction
