use os;

function TranslateObj(translatableObj)
	print("Translating text: " + translatableObj.GetTextSource() + " from " + translatableObj.GetLanguageSource() + " to " + translatableObj.GetLanguageTarget());

	var translation := TranslateText(
		translatableObj.GetTextSource(),
		translatableObj.GetLanguageSource(),
		translatableObj.GetLanguageTarget()
	);

	if (translation)
		print("Translation successful: " + translation);
		translatableObj.SetTextTarget(translation);
	else
		print("Translation failed, using original text.");
	endif

	return translatableObj;
endfunction

function TranslateText(text, langSource, langTarget)
	var apiURL := "http://epic-shard.com:5000/translate";
	var jsonData := PackJSON(struct{
		"q" := text,
		"source" := langSource,
		"target" := langTarget
	});

	var apiResponse := HTTPRequest(
		apiURL,
		"POST",
		struct{
			data := jsonData,
			headers := struct{
				"Content-Type" := "application/json"
			}
		},
		HTTPREQUEST_EXTENDED_RESPONSE
	);

	// print ("apiResponse: " + apiResponse);

	if (apiResponse.status == 200)
		// Convertendo o JSON da PIOR FORMA POSSÍVEL pq o Unpack e UnpackJSON não funcionam
		var body := apiResponse.body;
		var start_pos := body.Find("\":\"") + 3;
		var end_pos := body.Length() - start_pos - 2;
		body := body[start_pos, end_pos];
		return body;
	else
		return text;
	endif
endfunction