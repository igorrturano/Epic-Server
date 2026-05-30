// use uo;
use os;
include "StackFrame";

class StackTrace()
	var frames;
	var frame_count;
	
	function StackTrace(this)
		this.frames := array{};
		this.frame_count := 0;
		this._CaptureStackTrace();
	endfunction
	
	function _CaptureStackTrace(this)
		var stack_info := array{};
		stack_info := GetProcess().stacktrace(struct{ format := "array" });
		
		if (stack_info)
			stack_info.erase(1); // Remove a primeira frame relacionada à captura da stack trace (CaptureStackTrace)
			stack_info.erase(2); // Remove a segunda frame relacionada à captura da stack trace (GetStackTrace)

			foreach s in stack_info
				if (s.name == "CaptureStackTrace" || s.name == "GetStackTrace" || s.name == "StackTrace")
					continue;
				endif

				var frame := StackFrame();
				
				frame.SetScriptName(s.file);
				frame.SetMethodName(s.name);
				frame.SetLineNumber(s.line);
				frame.SetInstruction(s.pc);
				
				this.frames.append(frame);
				this.frame_count++;
			endforeach
		endif
	endfunction
	
	function GetFrame(this, index)
		if (index >= 0 && index < this.frames.size())
			return this.frames[index + 1];
		endif

		return 0;
	endfunction
	
	function GetFrameCount(this)
		return this.frame_count;
	endfunction
	
	function ToString(this)
		var result := "\n";
		if (!this.frames[1].GetMethodName() || !this.frames[1].GetLineNumber())
			result += "[!!!] To a more detailed stack trace, compile with debug mode enabled: \"ecompile -r <path> -f -x\"\n";
		endif

		result += $"Stack Trace ({this.frame_count} frames):\n";

		foreach frame in (this.frames)
			result += "   at " + frame.ToString() + "\n";
		endforeach

		return result;
	endfunction
	
	function GetFrames(this)
		return this.frames;
	endfunction
endclass
