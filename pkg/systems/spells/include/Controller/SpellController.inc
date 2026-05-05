use uo;
use os;

include ":wrappers:Objects/Model/MobileWrapper";
include ":timedscripts:timedScripts";
include ":hotbar:include/custombuff";
include ":attributes:stats";
include ":fls_core:packets";
include ":equipsys:packets/UpdateEquipmentRequest";
include "include/say";
include "MagicItemBonus";
include ":spells:SpellRegistry";

/**
 * SpellController — Unified spell effect orchestrator.
 *
 * Centralizes application, removal, and lifecycle of spell effects.
 * All spell effects are described by a struct (opts) and delegated to
 * timedScripts for duration management.
 *
 * Usage in spell scripts:
 *   var opts := struct{};
 *   opts.+slash_resist := bonus;
 *   opts.+str_mod := 5;
 *   opts.+color := 2826;
 *   SC_ApplyEffect(caster, targ, opts, duration, CInt(spell.id));
 */

/**
 * Applies a spell effect described by opts, sends buff icon,
 * and starts a timedScript for automatic cleanup.
 *
 * @param caster      Mobile - the caster
 * @param targ        Mobile - the targ receiving the effect
 * @param opts        Struct - mechanical effects to apply (resist, stats, color, speed)
 * @param duration    Integer - duration in seconds
 * @param spellId     Integer - spell ID for buff icon and tracking
 * @param timerName   String - timedScript timer name (default: "spelleffect")
 */
function SC_ApplyEffect(caster, targ, opts, duration, spellId, timerName := "spelleffect")
	// Fail closed if duration is invalid — TS_StartTimer rejects duration < 1,
	// so applying opts without a cleanup timer would leave permanent state
	// (e.g. frozen := 1 with no timer to undo it).
	if (CInt(duration) < 1)
		return error{"errortext":=$"SC_ApplyEffect: duration must be > 0 (spellId={spellId})"};
	endif

	// Velkyn focus bonus (Cogumelo staff): extend duration on Velkyn-school
	// effects. Gated on caster having a non-zero bonus to avoid the config
	// read on every spell apply — only Velkyn-focus casters pay the lookup.
	if (caster)
		var velkynBonus := GetMagicItemVelkynDurationBonus(caster);
		if (velkynBonus > 0)
			var spellinfo := ResolveSpell(spellId);
			if (spellinfo && spellinfo != error && CStr(spellinfo.School) == "Velkyn")
				duration := duration + velkynBonus;
			endif
		endif
	endif

	// Mutual exclusion: if this spell has a category, displace any other active
	// spell in the same category first (e.g. barkskin replaces escudodefe).
	var replacedName := "";
	if (opts.buff_category)
		replacedName := _SC_RemoveConflictingBuffs(targ, opts.buff_category, spellId);
		var mwIdx := MobileWrapper(targ);
		var idx := mwIdx.GetProperty("BuffCategoryIndex");
		if (!idx || idx.errortext)
			idx := dictionary;
		endif
		idx[opts.buff_category] := spellId;
		mwIdx.SetProperty("BuffCategoryIndex", idx);
	endif

	_SC_ApplyOpts(targ, opts);

	opts.+spellId := spellId;
	opts.+totalDuration := duration;
	// Unique per-apply token: lets a tick driver detect that its opts were
	// replaced by a subsequent apply (e.g. via maintain re-cast) and
	// self-terminate instead of firing against the new effect's state.
	opts.+instance_id := ReadGameClock();
	// Store caster serial so resync + tick scripts can recover the caster
	// after a server restart (the driver's caster arg is lost on shutdown).
	if (caster)
		opts.+caster_serial := caster.serial;
	endif
	MobileWrapper(targ).SetProperty($"SpellOpts_{spellId}", opts);

	if (spellId)
		var desc := _SC_BuildDescription(opts);
		SendSpellBuff(caster, targ, spellId, duration, desc);
	endif

	TS_StartTimer(targ, $"{timerName}_{spellId}", duration, spellId, caster, 0);

	if (opts.on_apply_script)
		start_script(opts.on_apply_script, array{ caster, targ, spellId, opts.on_apply_params });
	endif

	if ((opts.tick_script || opts.tick_damage) && opts.tick_interval)
		_SC_LaunchTickDriver(caster, targ, spellId, opts);
	endif

	// Tell the target when one buff replaced another in the same category.
	if (replacedName != "" && opts.displayname)
		SendSysMessageEx(targ, $"{replacedName} foi substituído por {opts.displayname}.", SSM_INFO);
	endif
endfunction

function _SC_LaunchTickDriver(caster, targ, spellId, opts)
	start_script(":spells:scTickDriver",
		array{ caster, targ, spellId, opts.tick_script, CInt(opts.tick_interval), opts.tick_params, CInt(opts.instance_id) });
endfunction

/**
 * Removes a spell effect by spellId. Called by endscript or dispel.
 *
 * @param targ    Mobile - the targ
 * @param spellId Integer - spell ID to remove
 */
function SC_RemoveEffect(targ, spellId)
	var mw := MobileWrapper(targ);
	var opts := mw.GetProperty($"SpellOpts_{spellId}");
	if (!opts || opts.errortext)
		return;
	endif

	// Fire on_expire *before* restoring state so the script can still read the
	// pre-removal opts if it needs them (e.g. to clean up spawned items).
	if (opts.on_expire_script)
		start_script(opts.on_expire_script, array{ 0, targ, spellId, opts.on_expire_params });
	endif

	_SC_RemoveOpts(targ, opts);

	// Clean the category index only if *this* spell still owns the slot.
	// The ownership guard matters during replacement: the new spell has already
	// overwritten the index entry before we get here, and we must not clobber it.
	if (opts.buff_category)
		var idx := mw.GetProperty("BuffCategoryIndex");
		if (idx && !idx.errortext && CInt(idx[opts.buff_category]) == spellId)
			idx.erase(opts.buff_category);
			mw.SetProperty("BuffCategoryIndex", idx);
		endif
	endif

	mw.EraseProperty($"SpellOpts_{spellId}");
	RemoveSpellBuff(targ, spellId);
endfunction

// Displaces the active spell in a given category, if any. Returns the
// displayname of the removed spell (empty string if nothing displaced or
// the old spell didn't advertise a name).
function _SC_RemoveConflictingBuffs(targ, category, exceptSpellId)
	if (!category)
		return "";
	endif
	var mw := MobileWrapper(targ);
	var idx := mw.GetProperty("BuffCategoryIndex");
	if (!idx || idx.errortext)
		return "";
	endif
	var existingId := CInt(idx[category]);
	if (!existingId || existingId == exceptSpellId)
		return "";
	endif
	var oldName := "";
	var existingOpts := mw.GetProperty($"SpellOpts_{existingId}");
	if (existingOpts && !existingOpts.errortext && existingOpts.displayname)
		oldName := existingOpts.displayname;
	endif
	SC_RemoveEffect(targ, existingId);
	return oldName;
endfunction

/**
 * Centralizes silent magic handling.
 * Call this in spell scripts INSTEAD of handleSilentMagic().
 *
 * @param caster  Mobile - the caster
 * @param targ    Mobile - the targ (for sound playback location)
 * @param sound   Integer - sound effect ID (0 = no sound)
 */
function SC_HandleSound(caster, targ := 0, sound := 0)
	if (GetObjProperty(caster, "#magiasilenciosa"))
		SendSysMessageEx(caster, "O efeito da magia silenciosa acabou.", SSM_INFO);
		EraseObjProperty(caster, "#magiasilenciosa");
		return;
	endif

	if (sound && targ)
		PlaySoundEffect(targ, sound);
	endif
endfunction

// ============================================================
// Internal: Apply/Remove opts
// ============================================================

function _SC_ApplyOpts(targ, opts)
	// Resist mods (virtual armor layer)
	if (opts.slash_resist || opts.piercing_resist || opts.blunt_resist ||
	    opts.fire_resist || opts.cold_resist || opts.poison_resist || opts.energy_resist)
		_SC_ApplyResist(targ, opts);
	endif

	// Stat mods
	if (opts.str_mod)
		AP_ModifyStatMod(targ, STRENGTH, opts.str_mod);
	endif
	if (opts.dex_mod)
		AP_ModifyStatMod(targ, DEXTERITY, opts.dex_mod);
	endif
	if (opts.int_mod)
		AP_ModifyStatMod(targ, INTELLIGENCE, opts.int_mod);
	endif

	// Color change
	if (opts.color)
		targ.color := opts.color;
	endif

	// Speed
	if (opts.speed_mod)
		targ.run_speed := targ.run_speed + opts.speed_mod;
	endif

	// Hit chance
	if (opts.hitchance_mod)
		targ.hitchance_mod := targ.hitchance_mod + opts.hitchance_mod;
	endif

	// Frozen
	if (opts.frozen)
		targ.frozen := 1;
	endif

	// Graphic change
	if (opts.graphic)
		targ.graphic := opts.graphic;
	endif

	// Armor hit scripts (append to array)
	if (opts.armor_hit_scripts)
		var armorscript := GetObjProperty(targ, "ArmorHitScripts");
		if (!armorscript)
			armorscript := array{};
		endif
		foreach script_path in (opts.armor_hit_scripts)
			armorscript.append(script_path);
		endforeach
		SetObjProperty(targ, "ArmorHitScripts", armorscript);
	endif

	// Hidden (invisibility)
	if (opts.hidden)
		targ.hidden := 1;
	endif

	// Evasion chance
	if (opts.evasionchance_mod)
		targ.evasionchance_mod := targ.evasionchance_mod + opts.evasionchance_mod;
	endif

	// Speed boost (PlayerSpeedBoost packet for players, run_speed for NPCs)
	if (opts.speed_boost)
		if (targ.npctemplate)
			SetObjProperty(targ, "#sc_runspeed", targ.run_speed);
			targ.run_speed := 250;
		else
			PlayerSpeedBoost(targ, 1);
		endif
	endif

	// Custom ObjProperties
	if (opts.properties)
		foreach val in (opts.properties)
			SetObjProperty(targ, _val_iter, val);
		endforeach
	endif
endfunction

function _SC_RemoveOpts(targ, opts)
	// Resist mods
	if (opts.slash_resist || opts.piercing_resist || opts.blunt_resist ||
	    opts.fire_resist || opts.cold_resist || opts.poison_resist || opts.energy_resist)
		_SC_RemoveResist(targ, opts);
	endif

	// Stat mods (reverse)
	if (opts.str_mod)
		AP_ModifyStatMod(targ, STRENGTH, -opts.str_mod);
	endif
	if (opts.dex_mod)
		AP_ModifyStatMod(targ, DEXTERITY, -opts.dex_mod);
	endif
	if (opts.int_mod)
		AP_ModifyStatMod(targ, INTELLIGENCE, -opts.int_mod);
	endif

	// Restore color
	if (opts.color)
		targ.color := targ.truecolor;
	endif

	// Restore speed
	if (opts.speed_mod)
		targ.run_speed := targ.run_speed - opts.speed_mod;
	endif

	// Hit chance (reverse)
	if (opts.hitchance_mod)
		targ.hitchance_mod := targ.hitchance_mod - opts.hitchance_mod;
	endif

	// Frozen (reverse)
	if (opts.frozen)
		targ.frozen := 0;
	endif

	// Graphic (restore)
	if (opts.graphic)
		targ.graphic := targ.trueobjtype;
	endif

	// Armor hit scripts (remove)
	if (opts.armor_hit_scripts)
		EraseObjProperty(targ, "ArmorHitScripts");
	endif

	// Hidden (reverse)
	if (opts.hidden)
		targ.hidden := 0;
	endif

	// Evasion chance (reverse)
	if (opts.evasionchance_mod)
		targ.evasionchance_mod := targ.evasionchance_mod - opts.evasionchance_mod;
	endif

	// Speed boost (reverse)
	if (opts.speed_boost)
		if (targ.npctemplate)
			var normal_speed := CInt(GetObjProperty(targ, "#sc_runspeed"));
			if (normal_speed)
				targ.run_speed := normal_speed;
			endif
			EraseObjProperty(targ, "#sc_runspeed");
		else
			PlayerSpeedBoost(targ, 0);
		endif
	endif

	// Custom ObjProperties
	if (opts.properties)
		foreach val in (opts.properties)
			EraseObjProperty(targ, _val_iter);
		endforeach
	endif
endfunction

// ============================================================
// Internal: Spell Resist management via #SpellResist property
// ============================================================

function _SC_ApplyResist(targ, opts)
	var mw := MobileWrapper(targ);
	var sr := mw.GetProperty("#SpellResist");
	if (!sr || sr.errortext)
		sr := struct{
			slash := 0, piercing := 0, blunt := 0,
			fire := 0, cold := 0, poison := 0, energy := 0
		};
	endif

	if (opts.slash_resist)    sr.slash    += opts.slash_resist;    endif
	if (opts.piercing_resist) sr.piercing += opts.piercing_resist; endif
	if (opts.blunt_resist)    sr.blunt    += opts.blunt_resist;    endif
	if (opts.fire_resist)     sr.fire     += opts.fire_resist;     endif
	if (opts.cold_resist)     sr.cold     += opts.cold_resist;     endif
	if (opts.poison_resist)   sr.poison   += opts.poison_resist;   endif
	if (opts.energy_resist)   sr.energy   += opts.energy_resist;   endif

	mw.SetProperty("#SpellResist", sr);
	_SC_RefreshArmorPackets(targ);
endfunction

function _SC_RemoveResist(targ, opts)
	var mw := MobileWrapper(targ);
	var sr := mw.GetProperty("#SpellResist");
	if (!sr || sr.errortext)
		return;
	endif

	if (opts.slash_resist)    sr.slash    -= opts.slash_resist;    endif
	if (opts.piercing_resist) sr.piercing -= opts.piercing_resist; endif
	if (opts.blunt_resist)    sr.blunt    -= opts.blunt_resist;    endif
	if (opts.fire_resist)     sr.fire     -= opts.fire_resist;     endif
	if (opts.cold_resist)     sr.cold     -= opts.cold_resist;     endif
	if (opts.poison_resist)   sr.poison   -= opts.poison_resist;   endif
	if (opts.energy_resist)   sr.energy   -= opts.energy_resist;   endif

	mw.SetProperty("#SpellResist", sr);
	_SC_RefreshArmorPackets(targ);
endfunction

// Public wrapper: external systems (transformation revert, etc.) call this
// to force the client's ArmorResistances dict to re-sync with the current
// #SpellResist — e.g. after endmetamorfose runs, because the packets that
// fired while the druid was stripped don't reflect re-equipped gear state.
function SC_RefreshArmorPackets(targ)
	_SC_RefreshArmorPackets(targ);
endfunction

// Re-emit armor packets for every region slot so the client's StatusGumpEpic
// shows the active #SpellResist on all 7 slots — even empty ones. Must pass
// raw `targ` (not a wrapper): SendUpdateEquipmentRequest's internal
// MobileWrapper(quem) relies on the OT_MOBILEREF constructor branch, and a
// wrapper argument would fall through to the template-string branch and
// zero out the resulting ObjRef.
function _SC_RefreshArmorPackets(targ)
	if (targ.npctemplate)
		return;
	endif
	for i := 1 to ARMOR_RESISTENCE_PACKET_ARRAY.Size()
		var layer := ARMOR_RESISTENCE_PACKET_ARRAY[i];
		// Pass whatever GetEquipmentByLayer returns — SendUpdateEquipmentRequest
		// tolerates null/error equip and emits a 0+spellResist packet for empty slots.
		SendUpdateEquipmentRequest(targ, GetEquipmentByLayer(targ, layer), 1, layer);
	endfor
endfunction

/**
 * Re-syncs spell effects on login / server restart.
 *
 * _SC_ApplyOpts is additive — calling it on top of already-applied state
 * doubles stat_mod, run_speed, hitchance_mod, etc. So we deliberately do
 * NOT call it here. Instead we:
 *   - Rebuild #SpellResist only if it was dropped (server restart — `#`
 *     props are ephemeral). Same-session reconnect keeps the property
 *     intact, so we skip the rebuild and avoid doubling.
 *   - Re-push client-session-only state: PlayerSpeedBoost, buff icons,
 *     and armor packets (client's ArmorResistances dict resets on
 *     disconnect).
 *   - Native server-side fields (color, stat_mod, run_speed, hitchance,
 *     graphic, armor_hit_scripts array) all persist across restart — no
 *     reapply needed.
 *
 * @param mobile Mobile - the player logging in
 */
function SC_ResyncOnLogin(mobile)
	var timer_list := TS_GetTimerList(mobile);
	var mw := MobileWrapper(mobile);
	var found := 0;
	var active_opts := array{};

	foreach timer in (timer_list)
		if (_timer_iter[1, 12] != "spelleffect_")
			continue;
		endif

		found := 1;
		var spellId := CInt(timer.level);
		if (!spellId)
			continue;
		endif

		// Use saved `duration` (remaining at last logoff), not endtime-based
		// math — spell buffs pause while offline/server-down (Phase 3).
		var remaining := CInt(timer.duration);
		if (remaining <= 0)
			SC_RemoveEffect(mobile, spellId);
			continue;
		endif

		var opts := mw.GetProperty($"SpellOpts_{spellId}");
		if (opts && !opts.errortext)
			active_opts.Append(opts);

			// Client-session-only state — lost on disconnect, must re-push.
			if (opts.speed_boost && !mobile.npctemplate)
				PlayerSpeedBoost(mobile, 1);
			endif

			// Relaunch tick driver — detached scripts die on server shutdown,
			// so any effect with a tick_script or tick_damage needs a fresh
			// driver to resume ticking from the saved remaining duration.
			if ((opts.tick_script || opts.tick_damage) && opts.tick_interval)
				var caster := 0;
				if (opts.caster_serial)
					caster := SystemFindObjectBySerial(CInt(opts.caster_serial), SYSFIND_SEARCH_OFFLINE_MOBILES);
					if (!caster || caster is @Exception)
						caster := 0;
					endif
				endif
				_SC_LaunchTickDriver(caster, mobile, spellId, opts);
			endif

			var desc := _SC_BuildDescription(opts);
			var totalDur := CInt(opts.totalDuration);
			SendSpellBuff(mobile, mobile, spellId, remaining, desc, totalDur);
		endif
	endforeach

	// Rebuild #SpellResist only if missing — the `#` prefix makes it ephemeral
	// (dropped on world save). Presence means same-session reconnect, so the
	// existing struct is the source of truth; rebuilding would double-count.
	if (active_opts.Size() > 0)
		var existing_sr := mw.GetProperty("#SpellResist");
		if (!existing_sr || existing_sr.errortext)
			var sr := struct{ slash := 0, piercing := 0, blunt := 0, fire := 0, cold := 0, poison := 0, energy := 0 };
			foreach op in (active_opts)
				if (op.slash_resist)    sr.slash    += CInt(op.slash_resist);    endif
				if (op.piercing_resist) sr.piercing += CInt(op.piercing_resist); endif
				if (op.blunt_resist)    sr.blunt    += CInt(op.blunt_resist);    endif
				if (op.fire_resist)     sr.fire     += CInt(op.fire_resist);     endif
				if (op.cold_resist)     sr.cold     += CInt(op.cold_resist);     endif
				if (op.poison_resist)   sr.poison   += CInt(op.poison_resist);   endif
				if (op.energy_resist)   sr.energy   += CInt(op.energy_resist);   endif
			endforeach
			mw.SetProperty("#SpellResist", sr);
		endif
	endif

	if (!found)
		_SC_CleanupOrphans(mobile);
	endif

	_SC_ResyncMetamorphosisResist(mobile);

	// Client's ArmorResistances dict is session-only — push updated values so
	// StatusGumpEpic reflects current #SpellResist after login.
	_SC_RefreshArmorPackets(mobile);
endfunction

function _SC_ResyncMetamorphosisResist(mobile)
	if (GetObjProperty(mobile, "metamorfose") == error)
		return;
	endif

	var resist := CInt(GetObjProperty(mobile, "polymorph_resistmod"));
	if (resist <= 0)
		return;
	endif

	// Rebuild runtime engine resist mods that may reset across restart.
	if (mobile.resist_physical_mod < resist)
		mobile.resist_physical_mod := resist;
	endif
	if (mobile.resist_fire_mod < resist)
		mobile.resist_fire_mod := resist;
	endif
	if (mobile.resist_cold_mod < resist)
		mobile.resist_cold_mod := resist;
	endif
	if (mobile.resist_poison_mod < resist)
		mobile.resist_poison_mod := resist;
	endif
	if (mobile.resist_energy_mod < resist)
		mobile.resist_energy_mod := resist;
	endif

	// Ensure virtual spell resist layer also contains the polymorph resist value.
	var mw := MobileWrapper(mobile);
	var sr := mw.GetProperty("#SpellResist");
	if (!sr || sr.errortext)
		sr := struct{slash := 0, piercing := 0, blunt := 0, fire := 0, cold := 0, poison := 0, energy := 0};
	endif

	if (CInt(sr.slash) < resist)    sr.slash := resist; endif
	if (CInt(sr.piercing) < resist) sr.piercing := resist; endif
	if (CInt(sr.blunt) < resist)    sr.blunt := resist; endif
	if (CInt(sr.fire) < resist)     sr.fire := resist; endif
	if (CInt(sr.cold) < resist)     sr.cold := resist; endif
	if (CInt(sr.poison) < resist)   sr.poison := resist; endif
	if (CInt(sr.energy) < resist)   sr.energy := resist; endif

	mw.SetProperty("#SpellResist", sr);
endfunction

function _SC_CleanupOrphans(mobile)
	// Do not erase #SpellResist while player is in wildshape — it holds transformation resistances
	if (GetObjProperty(mobile, "metamorfose") != error)
		return;
	endif

	var mw := MobileWrapper(mobile);
	var sr := mw.GetProperty("#SpellResist");
	if (sr && !sr.errortext)
		mw.EraseProperty("#SpellResist");
	endif
endfunction

// ============================================================
// Internal: Build description for buff icon
// ============================================================

function _SC_BuildDescription(opts)
	var parts := array{};

	if (opts.slash_resist)    parts.Append($"+{opts.slash_resist} Corte");       endif
	if (opts.piercing_resist) parts.Append($"+{opts.piercing_resist} Perf.");    endif
	if (opts.blunt_resist)    parts.Append($"+{opts.blunt_resist} Cont.");       endif
	if (opts.fire_resist)     parts.Append($"+{opts.fire_resist} Fogo");         endif
	if (opts.cold_resist)     parts.Append($"+{opts.cold_resist} Gelo");         endif
	if (opts.poison_resist)   parts.Append($"+{opts.poison_resist} Veneno");     endif
	if (opts.energy_resist)   parts.Append($"+{opts.energy_resist} Energia");    endif
	if (opts.str_mod)         parts.Append($"{opts.str_mod} FOR");               endif
	if (opts.dex_mod)         parts.Append($"{opts.dex_mod} DES");               endif
	if (opts.int_mod)         parts.Append($"{opts.int_mod} INT");               endif

	if (opts.desc)
		parts.Append(opts.desc);
	endif

	if (parts.size() == 0)
		return "";
	endif

	return ", ".join(parts);
endfunction
