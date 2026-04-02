use uo;

include ":epicevents:Model/ActionConfig";

class ConversationNode()
	var id;
	var text;
	var answers;  // array of answer structs

	function ConversationNode(this, node_id := "")
		this.id := node_id;
		this.text := "";
		this.answers := array{};
	endfunction

	function AddAnswer(this, answer_text, next_node := "", conditions := 0, actions := 0)
		var answer := struct{};
		answer.+id := "ans_" + CStr(polcore().systime) + "_" + CStr(RandomInt(9999));
		answer.+text := answer_text;
		answer.+next_node := next_node;
		answer.+conditions := conditions;
		answer.+actions := actions ? actions : array{};
		this.answers.append(answer);
		return answer;
	endfunction

	function RemoveAnswer(this, index)
		if (index < 1 || index > this.answers.size())
			return 0;
		endif
		this.answers.erase(index);
		return 1;
	endfunction

	function ToStruct(this)
		var data := struct{};
		data.+id := this.id;
		data.+text := this.text;

		var answers_data := array{};
		foreach answer in (this.answers)
			var ans := struct{};
			ans.+id := answer.id;
			ans.+text := answer.text;
			ans.+next_node := answer.next_node;
			ans.+conditions := answer.conditions;

			// Serialize actions array - always create clean structs
			var acts := array{};
			if (answer.actions)
				foreach action in (answer.actions)
					var act_struct := struct{};
					act_struct.+type := action.type;
					act_struct.+params := action.params;
					act_struct.+condition := action.condition;
					act_struct.+delay := action.delay;
					acts.append(act_struct);
				endforeach
			endif
			ans.+actions := acts;

			answers_data.append(ans);
		endforeach
		data.+answers := answers_data;

		return data;
	endfunction

	function FromStruct(this, data)
		if (!data)
			return 0;
		endif

		this.id := data.id;
		this.text := data.text;
		this.answers := array{};

		if (data.answers)
			foreach ans_data in (data.answers)
				var answer := struct{};
				answer.+id := ans_data.id;
				answer.+text := ans_data.text;
				answer.+next_node := ans_data.next_node;
				answer.+conditions := ans_data.conditions;

				// Deserialize actions array
				var acts := array{};
				if (ans_data.actions)
					foreach act_data in (ans_data.actions)
						var action := ActionConfig("");
						action.FromStruct(act_data);
						acts.append(action);
					endforeach
				endif
				answer.+actions := acts;

				this.answers.append(answer);
			endforeach
		endif

		return 1;
	endfunction
endclass
