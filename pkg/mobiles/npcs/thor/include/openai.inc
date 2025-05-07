use uo;
use os;
use basic;

function isPromptSafe(prompt)
	// If prompt is empty or too short, consider it safe
	if (!prompt || len(prompt) < 3)
		print("Prompt too short, skipping safety check");
		return 0; // Safe
	endif
	
	var apiURL := "https://api.openai.com/v1/moderations";
	var method := "POST";
	var jsonData := PackJSON(struct{
		"input" := prompt
	});

	print("Sending moderation request for prompt: " + prompt[1..min(20, len(prompt))] + "...");
	
	var apiResponse := HTTPRequest(
		apiURL,
		method,
		struct{
			data := jsonData,
			headers := struct{
				"Content-Type" := "application/json",
				"Authorization" := "Bearer {1}".format(GetEnvironmentVariable("OPENAI_KEY"))
			},
			timeout := 5000 // 5 second timeout
		}
	);

	if (!apiResponse)
		print("Empty moderation API response");
		return 0; // Assume safe if API fails
	endif
	
	var unpackedJSON := UnpackJSON(apiResponse);
	
	if (!unpackedJSON)
		print("Failed to unpack moderation JSON");
		return 0; // Assume safe if JSON parsing fails
	endif
	
	if (unpackedJSON["error"])
		print("Error in moderation API: " + unpackedJSON["error"]["message"]);
		return struct{ "error" := unpackedJSON["error"]["message"] };
	endif

	var results := unpackedJSON["results"]; // Acessa a lista de resultados
	if (!results || !results[1])
		print("Invalid moderation response format");
		return 0; // Assume safe if we can't verify
	endif
	
	// Convert to explicit 0 or 1 to avoid type issues
	if (results[1]["flagged"])
		return 1; // Unsafe
	else
		return 0; // Safe
	endif
endfunction

function OpenAICompletion_Thor(systemMessage, questions, temperatura:=0.7, top_p:=1.0, skipSafetyCheck:=1)
	print("=== Starting OpenAI Completion ===");
	// Limit temperature to valid range
	if (temperatura > 1.0)
		temperatura := 1.0;
		print("Temperature capped at 1.0");
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
	
	var prompt := "";
    foreach question in questions
        if (question.user)
            prompt += question.user;
        endif
    endforeach
	prompt += systemMessage;
	print("Prompt length: " + len(prompt));

	// Only perform safety check if not skipped
	if (!skipSafetyCheck)
		var safetyCheck := isPromptSafe(prompt);
		if (TypeOf(safetyCheck) == "Struct" && safetyCheck)
			print("Safety check error: " + safetyCheck);
			return "Error checking prompt safety. Please try again.";
		elseif (safetyCheck == 1) // Only flag as unsafe if explicitly returned as 1 (true)
			print("Prompt flagged as unsafe");
			return "I cannot respond to that request.";
		endif
		
		// Debug the safety check result
		print("Safety check result: " + safetyCheck);
	else
		print("Safety check skipped for OpenAI");
	endif

	var formatedQuestions := array{ struct{
		"role" := "system",
		"content" := systemMessage
	} };

	foreach question in questions
		if (typeof(question) != "Struct")
			print("Question is not a struct");
			return "Error: Invalid question format.";
		endif
		if (!question.assistant && !question.user)
			print("Question missing required fields");
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
		print("API attempt " + (retryCount + 1) + " of " + maxRetries);
		
		var startTime := ReadMillisecondClock();
		apiResponse := HTTPRequest(
			apiURL,
			method,
			struct{
				data := jsonData,
				headers := struct{
					"Content-Type" := "application/json",
					"Authorization" := "Bearer {1}".format(GetEnvironmentVariable("OPENAI_KEY"))
				},
				timeout := 10000 // 10 second timeout
			}
		);
		var endTime := ReadMillisecondClock();
		print("API request took " + (endTime - startTime) + "ms");
		
		if (!apiResponse)
			print("Empty API response on attempt " + (retryCount + 1));
			retryCount += 1;
			SleepMS(2000 * retryCount); // Exponential backoff
			continue;
		endif
		
		unpackedJSON := UnpackJSON(apiResponse);
		print("API Response: " + apiResponse);
		
		if (!unpackedJSON)
			print("Failed to unpack JSON on attempt " + (retryCount + 1));
			retryCount += 1;
			SleepMS(2000 * retryCount);
			continue;
		endif
		
		if (unpackedJSON["error"])
			print("API Error: " + unpackedJSON["error"]["message"]);
			retryCount += 1;
			SleepMS(2000 * retryCount);
			continue;
		endif
		
		if (!unpackedJSON["choices"] || !unpackedJSON["choices"][1] || !unpackedJSON["choices"][1]["message"])
			print("Invalid response format on attempt " + (retryCount + 1));
			retryCount += 1;
			SleepMS(2000 * retryCount);
			continue;
		endif
		
		success := true;
	endwhile
	
	if (!success)
		print("=== OpenAI Completion Failed ===");
		return "Failed to get a response after multiple attempts. Please try again later.";
	endif
	
	print("=== OpenAI Completion Successful ===");
	return unpackedJSON["choices"][1]["message"]["content"];
endfunction

function ClaudeCompletion(systemMessage, questions, temperatura:=0.7, top_p:=1.0, modelo:="claude-3-7-sonnet-20250219", skipSafetyCheck:=1)
	print("=== Starting Claude Completion ===");
	// Limit temperature to valid range
	if (temperatura > 1.0)
		temperatura := 1.0;
		print("Temperature capped at 1.0");
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
	
	var prompt := "";
    foreach question in questions
        if (question.user)
            prompt += question.user;
        endif
    endforeach
	prompt += systemMessage;
	print("Prompt length: " + len(prompt));

	// Only perform safety check if not skipped
	if (!skipSafetyCheck)
		var safetyCheck := isPromptSafe(prompt);
		if (TypeOf(safetyCheck) == "Struct" && safetyCheck)
			print("Safety check error: " + safetyCheck);
			return "Error checking prompt safety. Please try again.";
		elseif (safetyCheck == 1) // Only flag as unsafe if explicitly returned as 1 (true)
			print("Prompt flagged as unsafe");
			return "I cannot respond to that request.";
		endif
		
		// Debug the safety check result
		print("Safety check result: " + safetyCheck);
	else
		print("Safety check skipped for Claude");
	endif

	// Format messages for Claude API
	var formatedMessages := array{};
	
	// // Add system message first
	// if (systemMessage && systemMessage != "")
	// 	formatedMessages.append(struct{
	// 		"role" := "system",
	// 		"content" := systemMessage
	// 	});
	// endif
	// print("Formated Messages: "+formatedMessages);

	// Add user/assistant messages in order
	foreach question in questions
		if (TypeOf(question) != "Struct")
			print("Question is not a struct");
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
		print("No valid messages to send to Claude API");
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
		print("API attempt " + (retryCount + 1) + " of " + maxRetries);
		
		var startTime := ReadMillisecondClock();
		apiResponse := HTTPRequest(
			apiURL,
			method,
			struct{
				data := jsonData,
				headers := struct{
					"Content-Type" := "application/json",
					"x-api-key" := "PLACEHOLDER",
					"anthropic-version" := "2023-06-01"  // Versão atual da API do Claude
				},
				timeout := 10000 // 10 second timeout
			}
		);
		var endTime := ReadMillisecondClock();
		print("API request took " + (endTime - startTime) + "ms");
		
		if (!apiResponse)
			print("Empty API response on attempt " + (retryCount + 1));
			retryCount += 1;
			SleepMS(2000 * retryCount); // Exponential backoff
			continue;
		endif
		
		unpackedJSON := UnpackJSON(apiResponse);
		print("API Response: " + apiResponse);
		
		if (!unpackedJSON)
			print("Failed to unpack JSON on attempt " + (retryCount + 1));
			retryCount += 1;
			SleepMS(2000 * retryCount);
			continue;
		endif
		
		if (unpackedJSON["error"])
			print("API Error: " + unpackedJSON["error"]["message"]);
			retryCount += 1;
			SleepMS(2000 * retryCount);
			continue;
		endif
		
		if (!unpackedJSON["content"] || !unpackedJSON["content"].size() < 1 || !unpackedJSON["content"][0]["text"])
			print("Invalid response format on attempt " + (retryCount + 1));
			retryCount += 1;
			SleepMS(2000 * retryCount);
			continue;
		endif
		
		success := true;
	endwhile
	
	if (!success)
		print("=== Claude Completion Failed ===");
		return "Failed to get a response after multiple attempts. Please try again later.";
	endif
	
	print("=== Claude Completion Successful ===");
	return unpackedJSON["content"][0]["text"];
endfunction