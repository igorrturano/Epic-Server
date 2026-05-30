use os;
use polsys;

include "Translator";
include ":text:TranslatableText";

const TRANSLATION_CACHE_PROP := "translation_cache";
const TRANSLATION_BREAKER_PROP := "translation_breaker";
const TRANSLATION_BREAKER_WINDOW := 120;

class CachedTranslator()
	function CachedTranslator(this)
	endfunction

	/**
	 * Translate `text` from `sourceLang` to `targetLang`, returning the translated string.
	 * On same-lang, empty text, breaker-open, or HTTP failure, returns the source `text`
	 * unchanged. Successful translations are persisted to GlobalProperty("translation_cache")
	 * keyed by (targetLang, text). Failures trip a process-wide breaker that short-circuits
	 * all callers for TRANSLATION_BREAKER_WINDOW seconds.
	 */
	function Translate(this, text, sourceLang, targetLang)
		if (!text || text == "")
			return text;
		endif
		if (sourceLang == targetLang)
			return text;
		endif

		var cache := this._GetCache();
		var langBucket := cache.Exists(targetLang) ? cache[targetLang] : 0;
		if (langBucket && langBucket.Exists(text))
			return langBucket[text];
		endif

		if (this._BreakerOpen())
			return text;
		endif

		var translator := Translator();
		var result := translator.Translate(
			TranslatableText(text, sourceLang, targetLang)
		);
		if (!(result is @TranslatableText))
			this._BreakerTrip();
			return text;
		endif
		var translated := result.GetTextTarget();
		if (!translated || translated == "")
			this._BreakerTrip();
			return text;
		endif

		this._BreakerClear();
		if (!langBucket)
			langBucket := dictionary{};
		endif
		langBucket[text] := translated;
		cache[targetLang] := langBucket;
		SetGlobalProperty(TRANSLATION_CACHE_PROP, cache);
		return translated;
	endfunction

	// Private
	function _GetCache(this)
		var cache := GetGlobalProperty(TRANSLATION_CACHE_PROP);
		if (cache == error || !cache || TypeOf(cache) != "Dictionary")
			return dictionary{};
		endif
		return cache;
	endfunction

	function _BreakerOpen(this)
		var openedAt := GetGlobalProperty(TRANSLATION_BREAKER_PROP);
		if (openedAt == error || !openedAt)
			return 0;
		endif
		return (polcore().systime - CInt(openedAt)) < TRANSLATION_BREAKER_WINDOW;
	endfunction

	function _BreakerTrip(this)
		SetGlobalProperty(TRANSLATION_BREAKER_PROP, polcore().systime);
	endfunction

	function _BreakerClear(this)
		var openedAt := GetGlobalProperty(TRANSLATION_BREAKER_PROP);
		if (openedAt && openedAt != error)
			SetGlobalProperty(TRANSLATION_BREAKER_PROP, 0);
		endif
	endfunction
endclass
