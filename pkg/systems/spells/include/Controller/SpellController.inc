use uo;
use os;

include ":wrappers:Objects/Model/MobileWrapper";
include ":timedscripts:timedScripts";
include ":hotbar:include/custombuff";
include ":attributes:stats";
include ":fls_core:packets";
include ":equipsys:packets/UpdateEquipmentRequest";

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
	_SC_ApplyOpts(targ, opts);

	opts.+spellId := spellId;
	opts.+totalDuration := duration;
	MobileWrapper(targ).SetProperty($"#SpellOpts_{spellId}", opts);

	if (spellId)
		var desc := _SC_BuildDescription(opts);
		SendSpellBuff(caster, targ, spellId, duration, desc);
	endif

	TS_StartTimer(targ, $"{timerName}_{spellId}", duration, spellId, caster, 0);
endfunction

/**
 * Removes a spell effect by spellId. Called by endscript or dispel.
 *
 * @param targ    Mobile - the targ
 * @param spellId Integer - spell ID to remove
 */
function SC_RemoveEffect(targ, spellId)
	var mw := MobileWrapper(targ);
	var opts := mw.GetProperty($"#SpellOpts_{spellId}");
	if (!opts || opts.errortext)
		return;
	endif

	_SC_RemoveOpts(targ, opts);
	mw.EraseProperty($"#SpellOpts_{spellId}");
	RemoveSpellBuff(targ, spellId);
endfunction

/**
 * Centralizes silent magic handling.
 * Call this in spell scripts INSTEAD of handleSilentMagic().
 *
 * @param caster  Mobile - the caster
 * @param targ    Mobile - the targ (for sound playback location)
 * @param sound   Integer - sound effect ID (0 = no sound)
 */
function SC_HandleSound(caster, targ, sound := 0)
	if (GetObjProperty(caster, "#magiasilenciosa"))
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

// Re-emit armor packets for all worn armor so the client's resist HUD
// reflects the updated #SpellResist. Spells are a peer event source
// alongside equipitem.src's async update on equip/unequip.
function _SC_RefreshArmorPackets(targ)
	if (targ.npctemplate)
		return;
	endif
	Start_Script(":equipsys:packets/AssyncUpdateEquipment", array{MobileWrapper(targ), 1, 0, 0});
endfunction

/**
 * Re-syncs spell effects on login.
 * Re-sends buff icons for active spell timers, or cleans up
 * orphaned properties if the timer expired during offline.
 *
 * @param mobile Mobile - the player logging in
 */
function SC_ResyncOnLogin(mobile)
	var timer_list := TS_GetTimerList(mobile);
	var found := 0;

	foreach timer in (timer_list)
		if (_timer_iter[1, 12] != "spelleffect_")
			continue;
		endif

		found := 1;
		var spellId := CInt(timer.level);
		if (!spellId)
			continue;
		endif

		var remaining := CInt(timer.endtime - ReadGameClock());
		if (remaining <= 0)
			SC_RemoveEffect(mobile, spellId);
			continue;
		endif

		var mw := MobileWrapper(mobile);
		var opts := mw.GetProperty($"#SpellOpts_{spellId}");
		if (opts && !opts.errortext)
			_SC_ApplyOpts(mobile, opts);
			var desc := _SC_BuildDescription(opts);
			var totalDur := CInt(opts.totalDuration);
			SendSpellBuff(mobile, mobile, spellId, remaining, desc, totalDur);
		endif
	endforeach

	if (!found)
		_SC_CleanupOrphans(mobile);
	endif

	_SC_ResyncMetamorphosisResist(mobile);
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
