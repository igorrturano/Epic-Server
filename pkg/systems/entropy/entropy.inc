use uo;
use polsys;

include ":wrappers:Objects/Model/MobileWrapper";
include ":tn:npcutil";
include ":message:include/model/Message";

/**
 * Updates both individual mobile entropy and global world entropy.
 * This is the main function to use when entropy changes need to affect both the mobile and the world.
 *
 * @param who Mobile reference - The character whose entropy is being updated
 * @param amt Integer - Amount to add/subtract from entropy (can be negative)
 * @return Integer - The new entropy value of the mobile, or error if wrapper fails
 */
function UpdateMobileEntropy(who, amt)
    var wrapper := MobileWrapper(who);
    if (wrapper.GetObjRef().errortext)
        return wrapper.GetObjRef();
    endif

    // Update individual mobile entropy
    wrapper.UpdateEntropy(amt);

   // Update global world entropy
    updateGlobalEntropy(amt);

    // Check and update entropy milestone bonuses (dynamic)
    var current_entropy := wrapper.GetEntropy();
    updateDPEntropyBonus(who, current_entropy);
    // updateEntropyDescription(who, current_entropy);
    
    if (amt > 0)
        Message("Você ganhou {} pontos de entropia".format(amt), Color(SYSTEM_COLOR::SUCCESS))
            .SendSys(who);
    elseif (amt < 0)
        Message("Você perdeu {} pontos de entropia".format(Abs(amt)), Color(SYSTEM_COLOR::FAIL))
            .SendSys(who);
    endif
    return current_entropy;
endfunction

/**
 * Updates only the global world entropy.
 * Use this when you need to update world entropy without affecting individual mobiles.
 *
 * @param amt Integer - Amount to add/subtract from global entropy
 */
function updateGlobalEntropy(amt)
    var global_entropy := CInt(GetGlobalProperty("GlobalEntropy"));
    SetGlobalProperty("GlobalEntropy", global_entropy + amt);
endfunction


function updateDPEntropyBonus(byref who, current_entropy)
    var dpmod := GetObjProperty(who, "dpmod");
    if (!dpmod)
        dpmod := 0;
    endif

    // Get the currently active bonus (if any)
    var active_bonus := GetObjProperty(who, "dp_entropy_bonus_active");
    if (!active_bonus)
        active_bonus := 0;
    endif

    var new_bonus := 0;

    // Determine what bonus should be active based on current entropy
    if (current_entropy > 16000)
        new_bonus := 10;  // High entropy bonus
    elseif (current_entropy < -12000)
        new_bonus := -10; // Low entropy penalty
    else
        new_bonus := 0;   // Normal range, no bonus
    endif

    // Only update if the bonus changed
    if (new_bonus != active_bonus)
        // Remove old bonus
        dpmod := dpmod - active_bonus;

        // Apply new bonus
        dpmod := dpmod + new_bonus;

        SetObjProperty(who, "dpmod", dpmod);
        SetObjProperty(who, "dp_entropy_bonus_active", new_bonus);
    endif
endfunction

function updateEntropyDescription(byref who, current_entropy)
    var desc := "";
    var active_tier := GetObjProperty(who, "entropy_description_tier");
    var new_tier := 0;

    // Tier system for immersion
    if (current_entropy <= -20000)
        desc := "Aura Macabra";  // Extremely corrupted (Palpatine-level)
        new_tier := -2;
    elseif (current_entropy <= -16000)
        desc := "Olhos Negros";  // Very corrupted
        new_tier := -1;
    elseif (current_entropy >= 20000)
        desc := "Aura Radiante";  // Extremely virtuous
        new_tier := 2;
    elseif (current_entropy >= 16000)
        desc := "Olhos Dourados";  // Very virtuous
        new_tier := 1;
    else
        // Normal range, remove description
        EraseObjProperty(who, "descmodentropy");
        EraseObjProperty(who, "entropy_description_tier");
        IncRevision(who);  // Force client to refresh tooltip
        return;
    endif

    // Only update if tier changed
    if (new_tier != active_tier)
        SetObjProperty(who, "descmodentropy", desc);
        SetObjProperty(who, "entropy_description_tier", new_tier);
        IncRevision(who);  // Force client to refresh tooltip
    endif
endfunction

/**
 * Handles entropy gain when killing NPCs.
 * Awards entropy for killing undead, demons, and void creatures.
 *
 * @param corpse - The corpse of the killed NPC
 */
function handleEntropyGain(corpse)
	var nearby := ListMobilesNearLocationEx(corpse.x, corpse.y, LIST_IGNORE_Z, 16, LISTEX_FLAG_HIDDEN + LISTEX_FLAG_NORMAL, corpse.realm);
	var killers := array{};
	foreach killer in nearby
		if (killer.acctname)
			killers.append(killer);
		endif
	endforeach
	AwardEntropyFromKill(killers, corpse);
endfunction

/**
 * Awards entropy to killers based on NPC category and tier.
 * Awards entropy for killing Undead, Demons, and Void creatures.
 *
 * @param killers - Array of player mobiles that participated in the kill
 * @param corpse - The corpse of the killed NPC
 * @return 1 on success, 0 if no entropy awarded
 */
function AwardEntropyFromKill(killers, corpse)
    var template := GetObjProperty(corpse, "npctemplate"); // Set by the core.
	var npc_cfg := NPC_GetNPCConfig(template);

	var category := Lower(npc_cfg.Category);
	var tier := npc_cfg.Tier;

	// Only award entropy for Undead, Demons, and Void creatures
	if (!(category in array{"undead", "demons", "void"}))
		return 0;
	endif

	// Default tier to 1 if not set
	if (!tier)
		tier := 1;
	endif

	// Calculate entropy gain: tier * 5
	var entropy_gain := tier * 5;

	// Divide between killers
	if (killers.size() > 0)
		entropy_gain := CInt(entropy_gain / killers.size());
	else
		return 0;
	endif

	// Award entropy to each killer
	foreach killer in killers
		if (killer.acctname)
			UpdateMobileEntropy(killer, entropy_gain);
		endif
	endforeach

	return 1;
endfunction