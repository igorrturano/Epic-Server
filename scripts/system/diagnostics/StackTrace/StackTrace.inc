// use uo;
use os;
include "StackFrame";

class StackTrace()
	var frames;
	var frame_count;
	var _seconds;
	
	function StackTrace(this, seconds)
		this.frames := array{};
		this.frame_count := 0;
		this._seconds := seconds;
		this.CaptureStackTrace();
	endfunction
	
	function CaptureStackTrace(this)
		// Captura informações da pilha atual usando GetStackTrace()
		var stack_info := this.GetStackTrace(this._seconds);
		
		if (stack_info)
			foreach line in stack_info
				var frame := StackFrame();
				
				frame.SetScriptName(line);
				
				this.frames.append(frame);
				this.frame_count++;
			endforeach
		endif
	endfunction

	function GetStackTrace(this, seconds)
		var arr := array{};
		
		arr.append($"PID: {GetPid()}");
		arr.append($"Process: {GetProcess()}");
		arr.append($"EventsWaiting: {Events_Waiting()}");
		var performance := PerformanceMeasure(seconds);
		arr.append($"Ticks: {performance.total_number_observed}");
		arr.append($"Instructions: {performance.total_instructions}");
		foreach script in (performance.scripts)
			arr.append($"Script: pid={script.pid}, instructions={script.instructions}, percent={script.percent}, name={script.name}");
		endforeach

		return arr;
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
		var result := $"Stack Trace ({this.frame_count} frames in {this._seconds} seconds):\n";

		foreach frame in (this.frames)
			result += "   at " + frame.ToString() + "\n";
		endforeach

		return result;
	endfunction
	
	function GetFrames(this)
		return this.frames;
	endfunction
endclass

function GetCurrentStackTrace(seconds := 1)
	return StackTrace(seconds);
endfunction
