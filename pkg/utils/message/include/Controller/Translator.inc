use os;

include "ITranslator";
include ":text:TranslatableText";
include "system/exception/FormatException/InvalidFormatException";

class Translator(ITranslator)
	var _apiURL := ""; // URL da API de tradução
	var _translatableText; // Objeto de texto traduzível
	
	function Translator(this)
		this._apiURL := "http://127.0.0.1:5000/translate";
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
		
		this.GetText();
	endfunction

	function _apiTranslate(this)
		var jsonData := PackJSON(struct{
			"q" := this._translatableText.GetTextSource(),
			"source" := this._translatableText.GetLanguageSource(),
			"target" := this._translatableText.GetLanguageTarget()
		});

		var apiResponse := HTTPRequest(
			this._apiURL,
			"POST",
			struct{
				data := jsonData,
				headers := struct{
					"Content-Type" := "application/json"
				}
			},
			HTTPREQUEST_EXTENDED_RESPONSE
		);

		print ("apiResponse: " + apiResponse);

		if (apiResponse.status == 200)
			// Convertendo o JSON da PIOR FORMA POSSÍVEL pq o Unpack e UnpackJSON não funcionam
			var body := apiResponse.body;
			var start_pos := body.Find("\":\"") + 3;
			var end_pos := body.Length() - start_pos - 2;
			body := body[start_pos, end_pos];
			this._translatableText.SetTextTarget(body);
		endif
	endfunction
endclass

enum class TRANSLATION_MODE
	OFF,
	SUBTITLED,
	DUBBED
endenum
