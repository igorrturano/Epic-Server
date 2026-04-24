use uo;
use os;
use polsys;
use basic;
use file;

// ============================================================================
// BUDGET TRACKING FUNCTIONS
// ============================================================================

const AI_DAILY_BUDGET := 500; // calls per day (~$15-20/month)

function GetDateString()
	var time_info := polcore().systime;
	return CStr(time_info.year) + "-" + CStr(time_info.month) + "-" + CStr(time_info.mday);
endfunction

function CheckAIBudget()
	var usage := GetGlobalProperty("ai_daily_usage");
	if (!usage)
		usage := struct{
			"date" := GetDateString(),
			"count" := 0
		};
		SetGlobalProperty("ai_daily_usage", usage);
	endif

	// Reset if new day
	if (usage.date != GetDateString())
		usage.date := GetDateString();
		usage.count := 0;
		SetGlobalProperty("ai_daily_usage", usage);
	endif

	return (usage.count < AI_DAILY_BUDGET);
endfunction

function IncrementAIUsage()
	var usage := GetGlobalProperty("ai_daily_usage");
	if (!usage)
		usage := struct{
			"date" := GetDateString(),
			"count" := 0
		};
	endif

	usage.count += 1;
	SetGlobalProperty("ai_daily_usage", usage);

	// Log for monitoring
	LogToFile("ai_usage.log", "[" + GetDateString() + " " + polcore().systime.hour + ":" + polcore().systime.min + "] Count: " + usage.count + "/" + AI_DAILY_BUDGET);
endfunction

// ============================================================================
// INPUT VALIDATION
// ============================================================================

function ValidateAIInput(prompt)
	if (!prompt)
		return struct{ "error" := "Empty prompt" };
	endif

	var prompt_len := len(prompt);
	if (prompt_len < 3)
		return struct{ "error" := "Prompt too short (min 3 chars)" };
	endif

	if (prompt_len > 500)
		return struct{ "error" := "Prompt too long (max 500 chars)" };
	endif

	return 0; // Valid
endfunction

// ============================================================================
// OPENAI COMPLETION FUNCTION
// ============================================================================

function OpenAICompletion_Thor(systemMessage, questions, temperatura:=0.7, top_p:=1.0, skipSafetyCheck:=1)
	// print("=== Starting OpenAI Completion ===");

	// Limit temperature to valid range
	if (temperatura > 1.0)
		temperatura := 1.0;
		// print("Temperature capped at 1.0");
	endif

	// Validate questions parameter
	if (!questions || TypeOf(questions) != "Array" || !questions.size())
		print("Error: questions parameter is empty or not an array");
		return "Error: Invalid questions format. Please provide a valid array of questions.";
	endif

	// Check if each question has required fields
	var hasValidQuestion := 0;
	foreach question in questions
		if (TypeOf(question) == "Struct" && (question.user || question.assistant))
			hasValidQuestion := 1;
			break;
		endif
	endforeach

	if (!hasValidQuestion)
		print("Error: No valid questions found in the array");
		return "Error: Each question must contain 'assistant' or 'user' field.";
	endif

	// Basic input validation (replaced safety check)
	var prompt := "";
	foreach question in questions
		if (question.user)
			prompt += question.user;
		endif
	endforeach

	var validation := ValidateAIInput(prompt);
	if (validation)
		return "Entrada inválida.";
	endif;

	var formatedQuestions := array{ struct{
		"role" := "system",
		"content" := systemMessage
	} };

	foreach question in questions
		if (TypeOf(question) != "Struct")
			return "Error: Invalid question format.";
		endif
		if (!question.assistant && !question.user)
			return "Error: Each question must contain 'assistant' or 'user' field.";
		endif

		if (question.assistant)
			formatedQuestions.append(struct{
				"role" := "assistant",
				"content" := question.assistant
			});
		endif

		if (question.user)
			formatedQuestions.append(struct{
				"role" := "user",
				"content" := question.user
			});
		endif
	endforeach

	var apiURL := "https://api.openai.com/v1/chat/completions";
	var method := "POST";
	var jsonData := PackJSON(struct{
		"model" := "gpt-4o-mini",
		"messages" := formatedQuestions,
		"temperature" := temperatura,
		"top_p" := top_p
	});

	// Implement retry logic
	var maxRetries := 3;
	var retryCount := 0;
	var success := false;
	var apiResponse, unpackedJSON;

	while (retryCount < maxRetries && !success)
		var startTime := ReadMillisecondClock();
		apiResponse := HTTPRequest(
			apiURL,
			method,
			struct{
				data := jsonData,
				headers := struct{
					"Content-Type" := "application/json",
					"Authorization" := "Bearer " + GetEnvironmentVariable("OPENAI_KEY")
				},
				timeout := 10000 // 10 second timeout
			}
		);
		var endTime := ReadMillisecondClock();

		if (!apiResponse)
			retryCount += 1;
			SleepMS(2000 * retryCount); // Exponential backoff
			continue;
		endif

		unpackedJSON := UnpackJSON(apiResponse);

		if (!unpackedJSON)
			retryCount += 1;
			SleepMS(2000 * retryCount);
			continue;
		endif

		if (unpackedJSON["error"])
			retryCount += 1;
			SleepMS(2000 * retryCount);
			continue;
		endif

		if (!unpackedJSON["choices"] || !unpackedJSON["choices"][1] || !unpackedJSON["choices"][1]["message"])
			retryCount += 1;
			SleepMS(2000 * retryCount);
			continue;
		endif

		success := true;
	endwhile

	if (!success)
		return "Failed to get a response after multiple attempts. Please try again later.";
	endif

	return unpackedJSON["choices"][1]["message"]["content"];
endfunction

// ============================================================================
// CLAUDE COMPLETION FUNCTION (FALLBACK)
// ============================================================================

function ClaudeCompletion(systemMessage, questions, temperatura:=0.7, top_p:=1.0, modelo:="claude-3-haiku-20240307", skipSafetyCheck:=1)
	print("=== Starting Claude Completion ===");

	// Limit temperature to valid range
	if (temperatura > 1.0)
		temperatura := 1.0;
	endif

	// Validate questions parameter
	if (!questions || TypeOf(questions) != "Array" || !questions.size())
		return "Error: Invalid questions format. Please provide a valid array of questions.";
	endif

	// Check if each question has required fields
	var hasValidQuestion := 0;
	foreach question in questions
		if (TypeOf(question) == "Struct" && (question.user || question.assistant))
			hasValidQuestion := 1;
			break;
		endif
	endforeach

	if (!hasValidQuestion)
		return "Error: Each question must contain 'assistant' or 'user' field.";
	endif

	// Basic input validation
	var prompt := "";
	foreach question in questions
		if (question.user)
			prompt += question.user;
		endif
	endforeach

	var validation := ValidateAIInput(prompt);
	if (validation)
		return "Entrada inválida.";
	endif;

	// Format messages for Claude API
	var formatedMessages := array{};

	// Add user/assistant messages in order
	foreach question in questions
		if (TypeOf(question) != "Struct")
			continue;
		endif

		if (question.assistant)
			formatedMessages.append(struct{
				"role" := "assistant",
				"content" := question.assistant
			});
		endif

		if (question.user)
			formatedMessages.append(struct{
				"role" := "user",
				"content" := question.user
			});
		endif
	endforeach

	// Ensure we have at least one message
	if (formatedMessages.size() < 1)
		return "Error: No valid messages to send.";
	endif

	// Claude API URL
	var apiURL := "https://api.anthropic.com/v1/messages";
	var method := "POST";
	var jsonData := PackJSON(struct{
		"model" := modelo,
		"system" := systemMessage,  // System message as top-level parameter
		"messages" := formatedMessages,  // Only user and assistant messages here
		"temperature" := temperatura,
		"top_p" := top_p,
		"max_tokens" := 4096
	});

	// Implement retry logic
	var maxRetries := 3;
	var retryCount := 0;
	var success := false;
	var apiResponse, unpackedJSON;

	while (retryCount < maxRetries && !success)

		var startTime := ReadMillisecondClock();
		apiResponse := HTTPRequest(
			apiURL,
			method,
			struct{
				data := jsonData,
				headers := struct{
					"Content-Type" := "application/json",
					"x-api-key" := "ANTHROPIC_KEY_NOT_SET",
					"anthropic-version" := "2023-06-01"
				},
				timeout := 10000 // 10 second timeout
			}
		);
		var endTime := ReadMillisecondClock();

		if (!apiResponse)
			retryCount += 1;
			SleepMS(2000 * retryCount); // Exponential backoff
			continue;
		endif

		unpackedJSON := UnpackJSON(apiResponse);

		if (!unpackedJSON)
			retryCount += 1;
			SleepMS(2000 * retryCount);
			continue;
		endif

		if (unpackedJSON["error"])
			retryCount += 1;
			SleepMS(2000 * retryCount);
			continue;
		endif

		if (!unpackedJSON["content"] || unpackedJSON["content"].size() < 1 || !unpackedJSON["content"][1]["text"])
			retryCount += 1;
			SleepMS(2000 * retryCount);
			continue;
		endif

		success := true;
	endwhile

	if (!success)
		return "Failed to get a response after multiple attempts. Please try again later.";
	endif

	return unpackedJSON["content"][1]["text"];
endfunction

// ============================================================================
// UNIFIED AI COMPLETION WITH FALLBACK
// ============================================================================

function AICompletion(context, questions, temperatura:=0.7, top_p:=1.0, skipSafetyCheck:=1)
	// Validate questions parameter
	if (!questions || TypeOf(questions) != "Array" || !questions.size())
		return "Error: Invalid questions format. Please provide a valid array of questions.";
	endif

	// Check if each question has required fields
	var hasValidQuestion := 0;
	foreach question in questions
		if (TypeOf(question) == "Struct" && (question.user || question.assistant))
			hasValidQuestion := 1;
			break;
		endif
	endforeach

	if (!hasValidQuestion)
		return "Error: Each question must contain 'assistant' or 'user' field.";
	endif

	// Try Claude first
	var responseAI;
	var success := 0;

	responseAI := ClaudeCompletion(context, questions, temperatura, top_p, "claude-3-haiku-20240307", skipSafetyCheck);
	if (responseAI && responseAI != "Failed to get a response after multiple attempts. Please try again later.")
		success := 1;
	else
		print("Claude API error, falling back to OpenAI");
	endif

	// Fallback to OpenAI if Claude fails
	if (!success)
		responseAI := OpenAICompletion_Thor(context, questions, temperatura, top_p, skipSafetyCheck);
	endif

	return responseAI;
endfunction

// ============================================================================
// ASYNC QUEUE FUNCTION (for Phase 3)
// ============================================================================

function QueueAIRequest(npc, context, user_input, response_event_id)
	// Store request in NPC property
	SetObjProperty(npc, "#ai_request", struct{
		"context" := context,
		"input" := user_input,
		"event_id" := response_event_id,
		"timestamp" := polcore().systime
	});

	// Start background script
	var params := array{npc, context, user_input, response_event_id};
	start_script(":thor:scripts/process_ai_response", params);

	return 1;
endfunction
