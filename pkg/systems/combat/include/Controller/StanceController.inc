/*
 * StanceController.inc - Combat Stance Management
 *
 * Centralizes all combat stance logic following OOP principles and separation of concerns.
 * Handles stance application, removal, monitoring, and presentation.
 *
 * Responsibilities:
 * - Apply/remove stance modifiers (hitchance, evasion)
 * - Start/stop background monitoring for auto-reset
 * - Provide stance metadata (names, emotes)
 *
 * Author: Epic Shard Team
 */

use uo;

include ":wrappers:Objects/Model/MobileWrapper";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":combat:Model/CombatTraits";

/**
 * Applies stance modifiers to a mobile's combat stats
 *
 * @param mobile MobileWrapper - The mobile to apply modifiers to
 * @param stance STANCE_TYPE - The stance type to apply
 */
function ApplyStanceModifiers(mobile, stance)
	var hitchance_bonus := 0;
	var evasion_penalty := 0; // Positive value means easier to hit

	case (stance)
		STANCE_TYPE::AGGRESSIVE:
			hitchance_bonus := 5;
			evasion_penalty := 5;
		STANCE_TYPE::VERY_AGGRESSIVE:
			hitchance_bonus := 10;
			evasion_penalty := 10;
		STANCE_TYPE::DEFENSIVE:
			hitchance_bonus := -5;
			evasion_penalty := -5;
		STANCE_TYPE::VERY_DEFENSIVE:
			hitchance_bonus := -10;
			evasion_penalty := -10;
		STANCE_TYPE::NEUTRAL:
			// No modifiers
			return;
	endcase

	var mobileRef := mobile.GetObjRef();

	// Apply modifiers
	mobileRef.hitchance_mod := mobileRef.hitchance_mod + hitchance_bonus;
	mobileRef.evasionchance_mod := mobileRef.evasionchance_mod - evasion_penalty;

	// Debug message
	if (!mobileRef.npctemplate)
		DebugMsg(mobileRef, DEBUG_HIT, Message($"[STANCE] Aplicado: hitchance {hitchance_bonus:+}, evasion {-evasion_penalty:+}"));
	endif

	// Store for cleanup
	mobileRef.SetProp("#stance_hitchance_mod", hitchance_bonus);
	mobileRef.SetProp("#stance_evasion_mod", evasion_penalty);
endfunction

/**
 * Removes stance modifiers from a mobile
 * Reads stored modifiers and reverses them
 *
 * @param mobile MobileWrapper - The mobile to remove modifiers from
 */
function RemoveStanceModifiers(mobile)
	var mobileRef := mobile.GetObjRef();

	// Retrieve stored modifiers
	var hitchance_bonus := CInt(mobileRef.GetProp("#stance_hitchance_mod"));
	var evasion_penalty := CInt(mobileRef.GetProp("#stance_evasion_mod"));

	if (!hitchance_bonus && !evasion_penalty)
		return; // No modifiers to remove
	endif

	// Reverse the modifiers
	mobileRef.hitchance_mod := mobileRef.hitchance_mod - hitchance_bonus;
	mobileRef.evasionchance_mod := mobileRef.evasionchance_mod + evasion_penalty;

	// Debug message
	if (!mobileRef.npctemplate)
		DebugMsg(mobileRef, DEBUG_HIT, Message($"[STANCE] Removido: hitchance {hitchance_bonus}, evasion {evasion_penalty}"));
	endif

	// Clean up properties
	mobileRef.EraseProp("#stance_hitchance_mod");
	mobileRef.EraseProp("#stance_evasion_mod");
endfunction

/**
 * Resets a mobile's stance to neutral
 * Removes modifiers, updates CombatTraits, and notifies player
 *
 * @param mobile MobileWrapper - The mobile to reset
 */
function ResetStanceToNeutral(mobile)
	var traits := mobile.GetCombatTraits();
	if (!traits || traits.errortext)
		return;
	endif

	var currentStance := traits.GetStance();
	if (!currentStance || currentStance == STANCE_TYPE::NEUTRAL)
		return; // Already neutral
	endif

	// Remove stance modifiers
	RemoveStanceModifiers(mobile);

	// Reset stance to neutral
	mobile.SetCombatTraits(traits.SetStance(STANCE_TYPE::NEUTRAL));

	// Notify player
	Message("Sua postura de combate foi resetada para Neutra (fora de perigo).", Color(SYSTEM_COLOR::INFO))
		.SendSys(mobile.GetObjRef());
endfunction

/**
 * Starts background monitoring script for auto-reset
 * Prevents multiple monitors from running for the same player
 *
 * @param mobile MobileWrapper - The mobile to monitor
 */
function StartStanceMonitor(mobile)
	var mobileRef := mobile.GetObjRef();

	// Check if monitor is already running for this player
	if (mobileRef.GetProp("#stance_monitor_running"))
		return; // Already running
	endif

	// Mark as running
	mobileRef.SetProp("#stance_monitor_running", 1);

	// Start background script
	start_script(":charactercreation:stanceMonitor", mobileRef);
endfunction

/**
 * Stops background monitoring for a mobile
 *
 * @param mobile MobileWrapper - The mobile to stop monitoring
 */
function StopStanceMonitor(mobile)
	var mobileRef := mobile.GetObjRef();
	mobileRef.EraseProp("#stance_monitor_running");
endfunction

/**
 * Returns the localized name of a stance
 *
 * @param stance STANCE_TYPE - The stance type
 * @return string - Localized stance name
 */
function GetStanceName(stance)
	case (stance)
		STANCE_TYPE::NEUTRAL:
			return "Neutra";
		STANCE_TYPE::AGGRESSIVE:
			return "Agressiva";
		STANCE_TYPE::VERY_AGGRESSIVE:
			return "Muito Agressiva";
		STANCE_TYPE::DEFENSIVE:
			return "Defensiva";
		STANCE_TYPE::VERY_DEFENSIVE:
			return "Muito Defensiva";
		default:
			return "Neutra";
	endcase
endfunction

/**
 * Returns the emote message for a stance
 *
 * @param stance STANCE_TYPE - The stance type
 * @return string - Emote message
 */
function GetStanceEmote(stance)
	case (stance)
		STANCE_TYPE::NEUTRAL:
			return "assume uma postura equilibrada";
		STANCE_TYPE::AGGRESSIVE:
			return "assume uma postura agressiva";
		STANCE_TYPE::VERY_AGGRESSIVE:
			return "assume uma postura extremamente agressiva";
		STANCE_TYPE::DEFENSIVE:
			return "assume uma postura defensiva";
		STANCE_TYPE::VERY_DEFENSIVE:
			return "assume uma postura extremamente defensiva";
		default:
			return "assume uma postura equilibrada";
	endcase
endfunction
