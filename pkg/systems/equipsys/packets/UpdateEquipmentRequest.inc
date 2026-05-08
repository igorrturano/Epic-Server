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

// Emits a per-slot 0xBF:0x63 packet so the client's StatusGumpEpic can
// display armor + spell resist on the right region.
//
// Armor branch:
//   - `equip` may be null/error (empty slot). Needed so spell buffs like
//     barkskin/escudodefe show on every region, not just slots with armor.
//   - `layer` must be set.
//   - isEquipping=0 treats the slot as if the item is gone (armor_resist=0),
//     but still emits the active spell resist so buffs remain visible after
//     an unequip.
//
// Weapon branch: unchanged; requires a real weapon item.
function SendUpdateEquipmentRequest(quem, equip, isEquipping, layer := 0)
	if (!quem || quem.IsNpc())
		return 0;
	endif

	if (quem.IsA(POLCLASS_MOBILE))
		quem := MobileWrapper(quem);
	endif

	var wrappedEquip;
	if (equip && !(equip is @Exception))
		if (equip.IsA(POLCLASS_ARMOR))
			wrappedEquip := ArmorWrapper(equip);
		elseif (equip.IsA(POLCLASS_WEAPON))
			wrappedEquip := WeaponWrapper(equip);
		endif
	endif

	var packet := Packet(PACKET_ID_GENERIC_COMMANDS);
	packet.AppendBytes(PACKET_BYTE_TYPE::INT, 0) //valorTemporario, é alterado ao final.
		.AppendBytes(PACKET_BYTE_TYPE::INT, PACKET_ID_GENERIC_COMMANDS_SUBCMD_UPDT_EQUIP);

	if (wrappedEquip is @WeaponWrapper)
		packet.AppendBytes(PACKET_BYTE_TYPE::BYTE, WEAPON_UPDATE_FLAG);

		if (isEquipping)
			var attackInfo := AttackInfo(quem, 0);
			attackInfo.SetWeapon(wrappedEquip);
			var mainHandDmg := CalculateEstimatedMinMaxDamage(attackInfo);
			var offHandDmg := struct{ min := 0, max := 0 };

			if (attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetAttribute() == DUALWIELD)
				attackInfo.SetDamageHand(DAMAGE_HAND::OFFHAND);
				offHandDmg := CalculateEstimatedMinMaxDamage(attackInfo);
			endif

			packet
				.AppendBytes(PACKET_BYTE_TYPE::INT, mainHandDmg.min)
				.AppendBytes(PACKET_BYTE_TYPE::INT, mainHandDmg.max)
				.AppendBytes(PACKET_BYTE_TYPE::INT, offHandDmg.min)
				.AppendBytes(PACKET_BYTE_TYPE::INT, offHandDmg.max)
				.AppendBytes(PACKET_BYTE_TYPE::INT, mainHandDmg.Damage.GetFireRawDamage())
				.AppendBytes(PACKET_BYTE_TYPE::INT, mainHandDmg.Damage.GetColdRawDamage())
				.AppendBytes(PACKET_BYTE_TYPE::INT, mainHandDmg.Damage.GetPoisonRawDamage())
				.AppendBytes(PACKET_BYTE_TYPE::INT, mainHandDmg.Damage.GetEnergyRawDamage());
		else
			packet.FillStrip(PACKET_BYTE_TYPE::INT, 8);
		endif
	elseif (layer)
		packet.AppendBytes(PACKET_BYTE_TYPE::BYTE, ARMOR_UPDATE_FLAG)
			  .AppendBytes(PACKET_BYTE_TYPE::BYTE, layer);

		var spellResist := quem.GetSpellResist();
		var aSlash  := 0;
		var aPierce := 0;
		var aBlunt  := 0;
		var aFire   := 0;
		var aCold   := 0;
		var aPoison := 0;
		var aEnergy := 0;

		if (isEquipping && wrappedEquip is @ArmorWrapper)
			var r := wrappedEquip.GetResists();
			aSlash  := CInt(r.GetSlashResist());
			aPierce := CInt(r.GetPiercingResist());
			aBlunt  := CInt(r.GetBluntResist());
			aFire   := CInt(r.GetFireResist());
			aCold   := CInt(r.GetColdResist());
			aPoison := CInt(r.GetPoisonResist());
			aEnergy := CInt(r.GetEnergyResist());
		endif

		// Combinacao multiplicativa armor + spell por tipo de dano. Soma simples
		// estaria errada matematicamente ja que ambos sao % e podem passar de
		// 100. Formula: 1 - (1-armor) * (1-spell), em escala 0-100.
		packet
			.AppendBytes(PACKET_BYTE_TYPE::BYTE, _CombineResistDisplay(aSlash,  CInt(spellResist.GetSlashResist())))
			.AppendBytes(PACKET_BYTE_TYPE::BYTE, _CombineResistDisplay(aPierce, CInt(spellResist.GetPiercingResist())))
			.AppendBytes(PACKET_BYTE_TYPE::BYTE, _CombineResistDisplay(aBlunt,  CInt(spellResist.GetBluntResist())))
			.AppendBytes(PACKET_BYTE_TYPE::BYTE, _CombineResistDisplay(aFire,   CInt(spellResist.GetFireResist())))
			.AppendBytes(PACKET_BYTE_TYPE::BYTE, _CombineResistDisplay(aCold,   CInt(spellResist.GetColdResist())))
			.AppendBytes(PACKET_BYTE_TYPE::BYTE, _CombineResistDisplay(aPoison, CInt(spellResist.GetPoisonResist())))
			.AppendBytes(PACKET_BYTE_TYPE::BYTE, _CombineResistDisplay(aEnergy, CInt(spellResist.GetEnergyResist())));
	else
		return 0;
	endif

	packet.SetInt(PACKET_SIZE_INDEX, packet.Length());
	packet.Send(quem.GetObjRef());

	return 1;
endfunction

// Combina dois valores de % (0-100) multiplicativamente. Usado para o display
// no StatusGump: cada layer mostra o valor combinado da peca + spell resist
// global, refletindo o que combat realmente faz. Sem cap aqui — display e'
// informativo, gameplay aplica cap em CalculateDefenderResist se armored.
function _CombineResistDisplay(armorPct, spellPct)
	if (armorPct <= 0) return CInt(spellPct); endif
	if (spellPct <= 0) return CInt(armorPct); endif
	return CInt(100 - ((100 - armorPct) * (100 - spellPct) / 100));
endfunction
