use uo;

// Single source of truth for default trigger radius
const DEFAULT_TRIGGER_RADIUS := 8;

class TriggerConfig()
	var type;       // "SPEECH", "ENTEREDAREA", "LEFTAREA", "ITEM_GIVEN", "DOUBLECLICKED"
	var params;     // Type-specific parameters struct

	function TriggerConfig(this, triggerType)
		this.type := triggerType;
		this.params := struct{};
	endfunction

	function GetKeyword(this)
		// For SPEECH triggers
		if (this.params.keyword)
			return this.params.keyword;
		endif
		return "";
	endfunction

	function GetRadius(this)
		// For ENTEREDAREA/LEFTAREA triggers
		if (this.params.radius)
			return CInt(this.params.radius);
		endif
		return DEFAULT_TRIGGER_RADIUS;
	endfunction

	function GetRequiredItems(this)
		// For ITEM_GIVEN triggers
		if (this.params.required_items)
			return this.params.required_items;
		endif
		return array{};
	endfunction

	function ToStruct(this)
		var data := struct;
		data.+type := this.type;
		data.+params := this.params;
		return data;
	endfunction

	function FromStruct(this, data)
		if (!data)
			return 0;
		endif
		this.type := data.type;
		this.params := data.params;
		return 1;
	endfunction
endclass
