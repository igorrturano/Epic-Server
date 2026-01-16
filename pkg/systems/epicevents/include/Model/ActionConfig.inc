use uo;

class ActionConfig()
	var type;       // "SPAWN", "EMOTE", "TELEPORT", etc.
	var params;     // Action-specific parameters
	var condition;  // Optional condition struct
	var delay;      // Optional delay in seconds

	function ActionConfig(this, actionType)
		this.type := actionType;
		this.params := struct{};
		this.condition := 0;
		this.delay := 0;
	endfunction

	function ToStruct(this)
		var data := struct;
		data.+type := this.type;
		data.+params := this.params;
		data.+condition := this.condition;
		data.+delay := this.delay;
		return data;
	endfunction

	function FromStruct(this, data)
		if (!data)
			return 0;
		endif
		this.type := data.type;
		this.params := data.params;
		this.condition := data.condition;
		this.delay := data.delay;
		return 1;
	endfunction
endclass
