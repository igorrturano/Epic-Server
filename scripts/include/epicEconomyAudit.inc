// Append-only CSV ledger for coin (0xBA63 / 0xBA64 / 0xBA65) mint and transfer events.
// Authoritative source of truth; no CProps are written to coin stacks (would break POL's
// native stack-merge, which compares CProps).
// Log path: ::data/log/economy/<DD-MM-YYYY>/economy.log

use uo;
use os;
use file;
use util;
use basic;
use polsys;

include "include/client";

const EA_COIN_COBRE  := 0xBA63;
const EA_COIN_PRATA  := 0xBA64;
const EA_COIN_OURO   := 0xBA65;

const EA_MINT_LOOT             := "MINT_LOOT";
const EA_MINT_GM               := "MINT_GM";
const EA_MINT_CHARGEN          := "MINT_CHARGEN";
const EA_MINT_GUILD_PAYOUT     := "MINT_GUILD_PAYOUT";
const EA_MINT_GUILD_WITHDRAW   := "MINT_GUILD_WITHDRAW";
const EA_MINT_ESTALAGEM        := "MINT_ESTALAGEM";
const EA_TRANSFER_CORPSE_LOOT  := "TRANSFER_CORPSE_LOOT";
const EA_TRANSFER_PICKUP       := "TRANSFER_PICKUP";
const EA_TRANSFER_VENDOR_REFUND := "TRANSFER_VENDOR_REFUND";
const EA_TRANSFER_VENDOR_BUY   := "TRANSFER_VENDOR_BUY";
const EA_TRANSFER_TRADE        := "TRANSFER_TRADE";

function EA_IsCoin(objtype)
	return (objtype == EA_COIN_COBRE || objtype == EA_COIN_PRATA || objtype == EA_COIN_OURO);
endfunction

function EA_CsvEsc(value)
	if (value == error || !value)
		return "";
	endif
	return StrReplace(CStr(value), ",", ";");
endfunction

function EA_ResolveActor(actor, fallback_label)
	var info := struct{
		"acct" := "",
		"label" := "",
		"realm" := "",
		"x" := 0,
		"y" := 0
	};

	if (!actor || actor == error)
		if (fallback_label != error && fallback_label)
			info.label := CStr(fallback_label);
		endif
		return info;
	endif

	// Strings (pre-built labels like "GM:acct", "CHEST:loot_index")
	if (TypeOf(actor) == "String")
		info.label := actor;
		return info;
	endif

	// Mobile or item — both expose x/y/realm
	if (actor.x != error)
		info.x := actor.x;
	endif
	if (actor.y != error)
		info.y := actor.y;
	endif
	if (actor.realm != error && actor.realm)
		info.realm := actor.realm;
	endif

	// Player
	if (actor.acctname != error && actor.acctname)
		info.acct := actor.acctname;
		var char_name := GetObjProperty(actor, "nome_inicial");
		if (char_name == error || !char_name)
			char_name := actor.name;
		endif
		info.label := char_name;
		return info;
	endif

	// NPC
	if (actor.npctemplate != error && actor.npctemplate)
		info.label := actor.npctemplate;
		return info;
	endif

	// Generic item / container
	if (actor.serial != error && actor.serial)
		info.label := actor.name;
		return info;
	endif

	return info;
endfunction

function EA_Record(event_type, item, amount, src_actor, dst_actor, extra)
	if (!item || item == error)
		return 0;
	endif
	if (!EA_IsCoin(item.objtype))
		return 0;
	endif

	amount := CInt(amount);
	if (!amount)
		return 0;
	endif

	// Filter TRANSFER_* events where the recipient is not a player (drops
	// corpse-decay/mannequin noise). MINT_* bypasses this since mint dsts are containers.
	if (Find(event_type, "TRANSFER_", 1) == 1)
		if (!dst_actor || dst_actor == error || dst_actor.acctname == error || !dst_actor.acctname)
			return 0;
		endif
	endif

	var timestamp := StrFormatTime("%d-%m-%Y %H:%M:%S", POLCore().systime);
	var today := StrFormatTime("%d-%m-%Y", POLCore().systime);

	var log_dir := "::data/log/economy/{}".format(today);
	if (!FileExists(log_dir))
		CreateDirectory(log_dir);
	endif
	var log_file := "{}/economy.log".format(log_dir);

	var src := EA_ResolveActor(src_actor, "");
	var dst := EA_ResolveActor(dst_actor, "");

	var realm := dst.realm;
	if (!realm)
		realm := src.realm;
	endif
	if (!realm && item.realm != error)
		realm := item.realm;
	endif

	var rec_x := dst.x;
	var rec_y := dst.y;
	if (!rec_x && !rec_y && item.x != error)
		rec_x := item.x;
		rec_y := item.y;
	endif

	// timestamp,event,objtype,amount,src_acct,src_label,dst_acct,dst_label,realm,x,y,extra
	var line := "{},{},{},{},{},{},{},{},{},{},{},{}".format(
		timestamp,
		event_type,
		Hex(item.objtype),
		amount,
		EA_CsvEsc(src.acct),
		EA_CsvEsc(src.label),
		EA_CsvEsc(dst.acct),
		EA_CsvEsc(dst.label),
		EA_CsvEsc(realm),
		rec_x,
		rec_y,
		EA_CsvEsc(extra)
	);

	LogToFile(log_file, line, 0);
	return 1;
endfunction
