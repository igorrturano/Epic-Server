use os;

include "model/TranslatableText";

class Translator()
	var _apiURL := ""; // URL da API de tradução
	var _translatableText := TranslatableText(); // Objeto de texto traduzível
	
	function Translator(this, translatableText := false)
		this._apiURL := "http://127.0.0.1:5000/translate";
		this._translatableText := translatableText;
	endfunction

	function GetTranslatableText(this)
		return this._translatableText;
	endfunction
	
	function SetTranslatableText(this, object)
		this._translatableText := object;
		return this;
	endfunction

	function Translate(this)
		print ("Translating text...");
		if (!this._translatableText) return error{ errortext := "There is no text to translate"}; endif
		
		var translation := _apiTranslate(struct{
			apiURL := this._apiURL,
			translatableText := this._translatableText
		});

		print ("Translation result: " + translation);
		print ("Translation result: " + translation.GetTextTarget());

		if (!translation)
			print("!!!!!!!!!! Translation failed !!!!!!!!!!");
		endif
		
		return this;
	endfunction

	function _apiTranslate(params)
		var apiURL := params.apiURL;
		var translatableText := params.translatableText;

		var jsonData := PackJSON(struct{
			"q" := translatableText.GetTextSource(),
			"source" := translatableText.GetLanguageSource(),
			"target" := translatableText.GetLanguageTarget()
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

		print ("apiResponse: " + apiResponse);

		if (apiResponse.status == 200)
			// Convertendo o JSON da PIOR FORMA POSSÍVEL pq o Unpack e UnpackJSON não funcionam
			var body := apiResponse.body;
			var start_pos := body.Find("\":\"") + 3;
			var end_pos := body.Length() - start_pos - 2;
			body := body[start_pos, end_pos];
			translatableText.SetTextTarget(body);
		endif

		return translatableText;
	endfunction
endclass