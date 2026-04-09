class StackFrame()
	var method_name;
	var script_name;
	var line_number;
	var instruction;
	
	function StackFrame(this, method := "", script := "", line := 0, instr := "")
		this.method_name := method;
		this.script_name := script;
		this.line_number := line;
		this.instruction := instr;
	endfunction

	function GetMethodName(this)
		return this.method_name;
	endfunction

	function GetScriptName(this)
		return this.script_name;
	endfunction

	function GetLineNumber(this)
		return this.line_number;
	endfunction

	function GetInstruction(this)
		return this.instruction;
	endfunction

	function SetMethodName(this, method)
		this.method_name := method;
	endfunction

	function SetScriptName(this, script)
		this.script_name := script;
	endfunction

	function SetLineNumber(this, line)
		this.line_number := line;
	endfunction

	function SetInstruction(this, instr)
		this.instruction := instr;
	endfunction
	
	function ToString(this)
		var result := "";

		if (this.script_name)
			result += this.script_name;
		endif

		if (this.method_name)
			result += "::" + this.method_name;
		endif

		if (this.line_number)
			result += " (line " + CStr(this.line_number) + ")";
		endif

		if (this.instruction)
			result += " [instr: " + this.instruction + "]";
		endif

		return result;
	endfunction
endclass
