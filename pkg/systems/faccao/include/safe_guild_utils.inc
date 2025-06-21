/*
 * Safe Guild Utilities
 * Purpose: Provide safe access methods for guild properties to prevent "uninitialized object" errors
 * Author: Assistant
 * Date: 2025-06-07
 */

/*
 * SafeGetGuildName(guild)
 * 
 * Purpose: Safely get guild name with proper fallback mechanism to avoid "uninitialized object" errors
 * Parameters: guild - Guild object reference
 * Returns: String - Guild name or "Unknown Guild" if all methods fail
 */
function SafeGetGuildName(guild)
    if (!guild)
        return "No Guild";
    endif
      // Method 1: Try GetProp(FACTION_PROP_NAME) - most reliable for faction guilds
    var guild_name := guild.GetProp(FACTION_PROP_NAME);
    if (guild_name && guild_name != error && guild_name != "<uninitialized object>")
        return guild_name;
    endif
    
    // Method 2: Try guildname property as fallback
    if (guild.guildname && guild.guildname != error && guild.guildname != "<uninitialized object>")
        return guild.guildname;
    endif
    
    // Method 3: Last resort - return guild ID with prefix
    if (guild.guildid)
        return "Guild #" + guild.guildid;
    endif
    
    return "Unknown Guild";
endfunction

/*
 * SafeGetGuildProperty(guild, prop_name, default_value)
 * 
 * Purpose: Safely get any guild property with fallback
 * Parameters: 
 *   guild - Guild object reference
 *   prop_name - Property name to retrieve
 *   default_value - Value to return if property not found (optional)
 * Returns: Property value or default_value
 */
function SafeGetGuildProperty(guild, prop_name, default_value := "")
    if (!guild)
        return default_value;
    endif
    
    var prop_value := guild.GetProp(prop_name);
    if (prop_value && prop_value != error && prop_value != "<uninitialized object>")
        return prop_value;
    endif
    
    return default_value;
endfunction

/*
 * SafeSetGuildProperty(guild, prop_name, value)
 * 
 * Purpose: Safely set guild property with validation
 * Parameters: 
 *   guild - Guild object reference
 *   prop_name - Property name to set
 *   value - Value to set
 * Returns: 1 if successful, 0 if failed
 */
function SafeSetGuildProperty(guild, prop_name, value)
    if (!guild)
        return 0;
    endif
    
    var result := guild.SetProp(prop_name, value);
    if (result == error)
        return 0;
    endif
    
    return 1;
endfunction
