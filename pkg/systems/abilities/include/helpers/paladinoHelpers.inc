/**
 * Paladino ability helpers
 * Shared functions for Paladino class abilities (IDs 400-404)
 */

var RELIGIOUS_BANNERS := { 0xF091, 0xF092, 0xF090, 0xECEB, 0xECEA, 0xECE9 };

/**
 * Check if paladin has a religious banner equipped
 * @param mobile - MobileWrapper of the paladin
 * @returns struct{ has_banner, range } - has_banner=1 and range=6 if banner equipped, else has_banner=0 and range=2
 */
function CheckReligiousBanner(mobile)
	var hand1 := mobile.GetEquipByLayer(LAYER_HAND1);
	var hand2 := mobile.GetEquipByLayer(LAYER_HAND2);

	if ((hand1 && hand1.objtype in RELIGIOUS_BANNERS) || (hand2 && hand2.objtype in RELIGIOUS_BANNERS))
		return struct{ has_banner := 1, range := 6 };
	endif

	return struct{ has_banner := 0, range := 2 };
endfunction
