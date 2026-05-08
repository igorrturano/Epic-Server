use os;

include "ITranslator";
include ":text:TranslatableText";
include "system/exception/FormatException/InvalidFormatException";

class Translator(ITranslator)
	var _apiURL := ""; // URL da API de tradução
	var _apiKey := ""; // API key opcional, enviada como header X-API-Key
	var _translatableText; // Objeto de texto traduzível

	function Translator(this)
		this._apiURL := "http://127.0.0.1:5000/translate";
		this._apiKey := "";
	endfunction

	function GetText(this)
		return this._translatableText;
	endfunction
	
	function SetText(this, text)
		if (!(text is @TranslatableText))
			return InvalidFormatException("Parameter 'text' must be of type TranslatableText.");
		endif
		
		this._translatableText := text;
		return this;
	endfunction

	function Translate(this, translatableText)
		this.SetText(translatableText);
		if (!this._translatableText)
			return InvalidFormatException("There is no text to translate");
		endif

		this._apiTranslate();

		return this.GetText();
	endfunction

	function _apiTranslate(this)
		var jsonData := PackJSON(struct{
			"q" := this._translatableText.GetTextSource(),
			"source" := this._translatableText.GetLanguageSource(),
			"target" := this._translatableText.GetLanguageTarget()
		});

		var headers := struct{
			"Content-Type" := "application/json"
		};
		if (this._apiKey)
			headers.+"X-API-Key" := this._apiKey;
		endif

		var apiResponse := HTTPRequest(
			this._apiURL,
			"POST",
			struct{
				data := jsonData,
				headers := headers,
				timeout := 10000
			},
			HTTPREQUEST_EXTENDED_RESPONSE
		);

		if (apiResponse == error)
			Print("[Translator] HTTPRequest error for " + this._apiURL + ": " + apiResponse.errortext);
			return;
		endif

		if (!apiResponse.status)
			Print("[Translator] No HTTP status from " + this._apiURL + " (connection refused, DNS, or timeout — is LibreTranslate up?)");
			return;
		endif

		if (apiResponse.status != 200)
			Print("[Translator] HTTP " + apiResponse.status + " from " + this._apiURL + ": " + apiResponse.body);
			return;
		endif

		var parsed := UnpackJSON(apiResponse.body);
		if (parsed is @Exception || !parsed.translatedText)
			Print("[Translator] Failed to parse response body: " + apiResponse.body);
			return;
		endif

		this._translatableText.SetTextTarget(parsed.translatedText);
	endfunction
endclass

enum class TRANSLATION_MODE
	OFF,
	SUBTITLED,
	DUBBED
endenum
