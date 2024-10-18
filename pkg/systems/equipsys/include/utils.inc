use uo;
use util;
use cfgfile;

include "include/client";
include "props";

enum TIERS
	TIER0, TIER1, TIER2, TIER3, TIER4, TIER5
endenum

enum SPEEDTAG
	MELEE_LOW := "MELEE_LOW", MELEE_MID := "MELEE_MID", MELEE_HIGH := "MELEE_HIGH",
	RANGED_LOW := "RANGED_LOW", RANGED_MID := "RANGED_MID", RANGED_HIGH := "RANGED_HIGH"
endenum
enum DAMAGETAG
	DAMAGE_NONE := "DAMAGE_NONE",
	ONEHANDED_LOW := "ONEHANDED_LOW", ONEHANDED_MID := "ONEHANDED_MID", ONEHANDED_HIGH := "ONEHANDED_HIGH",
	TWOHANDED_LOW := "TWOHANDED_LOW", TWOHANDED_MID := "TWOHANDED_MID", TWOHANDED_HIGH := "TWOHANDED_HIGH",
	BOW_LOW := "BOW_LOW", BOW_MID := "BOW_MID", BOW_HIGH := "BOW_HIGH",
	CROSSBOW_LOW := "CROSSBOW_LOW", CROSSBOW_MID := "CROSSBOW_MID", CROSSBOW_HIGH := "CROSSBOW_HIGH"
endenum
enum WEAPONTYPE
	BOW := "Bow", CROSSBOW := "Crossbow"
endenum
// enum DAMAGEDICE
// 	DAMAGE_NONE_DAMAGE := 0,
// 	ONEHANDED_LOW_DAMAGE := "1d4", ONEHANDED_MID_DAMAGE := "1d6", ONEHANDED_HIGH_DAMAGE := "1d8",
// 	TWOHANDED_LOW_DAMAGE := "1d8", TWOHANDED_MID_DAMAGE := "1d10", TWOHANDED_HIGH_DAMAGE := "1d12",
// 	BOW_LOW_DAMAGE := "1d4", BOW_MID_DAMAGE := "1d6", BOW_HIGH_DAMAGE := "1d8",
// 	CROSSBOW_LOW_DAMAGE := "1d6", CROSSBOW_MID_DAMAGE := "1d8", CROSSBOW_HIGH_DAMAGE := "1d10"
// endenum
enum DAMAGETYPE
	SLASHDAMAGE := "Cortante", PIERCINGDAMAGE := "Perfurante", BLUNTDAMAGE := "Contusivo"
endenum
enum CRITICALTAG
	CRITICALCHANCE_NOCHANCE := "CRITICAL_NOCHANCE",
	CRITICALCHANCE_LOW := "CRITICAL_LOW",
	CRITICALCHANCE_MID := "CRITICAL_MID",
	CRITICALCHANCE_HIGH := "CRITICAL_HIGH"
endenum
enum ARMORSLOT
	HEAD := "HEAD", NECK := "NECK", BODY := "BODY", ARMS := "ARMS", HANDS := "HANDS", LEGS := "LEGS", FEET := "FEET", SHIELD := "SHIELD"
endenum
enum PROTECTIONTYPE
	ARMOR_LOWLOW := "ARMADURA LEVE FRACA", ARMOR_LOWMID := "ARMADURA LEVE MEDIA", ARMOR_LOWHIGH := "ARMADURA LEVE FORTE",
	ARMOR_MIDLOW := "ARMADURA MEDIA FRACA", ARMOR_MIDMID := "ARMADURA MEDIA MEDIA", ARMOR_MIDHIGH := "ARMADURA MEDIA FORTE",
	ARMOR_HIGHLOW := "ARMADURA PESADA FRACA", ARMOR_HIGHMID := "ARMADURA PESADA MEDIA", ARMOR_HIGHHIGH := "ARMADURA PESADA FORTE",
	SHIELD_LOWLOW := "ESCUDO LEVE FRACO", SHIELD_LOWMID := "ESCUDO LEVE MEDIO", SHIELD_LOWHIGH := "ESCUDO LEVE FORTE",
	SHIELD_MIDLOW := "ESCUDO MEDIO FRACO", SHIELD_MIDMID := "ESCUDO MEDIO MEDIO", SHIELD_MIDHIGH := "ESCUDO MEDIO FORTE",
	SHIELD_HIGHLOW := "ESCUDO PESADO FRACA", SHIELD_HIGHMID := "ESCUDO PESADO MEDIO", SHIELD_HIGHHIGH := "ESCUDO PESADO FORTE",
	SHIELD_BODYLOW := "ESCUDO DE CORPO FRACA", SHIELD_BODYMID := "ESCUDO DE CORPO MEDIO", SHIELD_BODYHIGH := "ESCUDO DE CORPO FORTE",
endenum


var MISS_FX := struct {
	SFX := array{0x0239, 0x023A, 0x023B, 0x539, 0x53A, 0x53B}
};
var BOW_FX := struct {
	ANIM := array{ANIM_ATTACK_BOW},
	SFX := array{0x235, 0x4C9, 0x4CA}
};
var CROSSBOW_FX := struct {
	ANIM := array{ANIM_ATTACK_XBOW},
	SFX := array{0x235, 0x4C9, 0x4CA}
};
var SLASH_1H_FX := struct {
	ANIM := array{ANIM_ATTACK_1HAND_WIDE, ANIM_ATTACK_2HAND_WIDE},
	SFX := array{0x23C,0x23D}
};
var SLASH_2H_FX := struct {
	ANIM := array{ANIM_ATTACK_1HAND_WIDE, ANIM_ATTACK_2HAND_WIDE},
	SFX := array{0x23C,0x23D}
};
var PIERCING_1H_FX := struct {
	ANIM := array{ANIM_ATTACK_1HAND_JAB, ANIM_ATTACK_2HAND_JAB},
	SFX := array{0x23C,0x23D}
};
var PIERCING_2H_FX := struct {
	ANIM := array{ANIM_ATTACK_1HAND_JAB, ANIM_ATTACK_2HAND_JAB},
	SFX := array{0x23D}
};
var BLUNT_1H_FX := struct {
	ANIM := array{ANIM_ATTACK_2HAND_DOWN},
	SFX := array{0x3AC}
};
var BLUNT_2H_FX := struct {
	ANIM := array{ANIM_ATTACK_2HAND_DOWN},
	SFX := array{0x3AE}
};

function getItemCfgElem(item) //RETORNA TODOS OS VALORES DO ITEM QUE AINDA ESTIVEREM NO .CFG
	return FindConfigElem(ReadConfigFile(":combat:itemdesc"), item.objtype);
endfunction

function getType(item)
    var elem := getItemCfgElem(item);
	var tipo;

	if (item.IsA(POLCLASS_WEAPON))
		tipo := elem.TipoAtaque;
	elseif (item.IsA(POLCLASS_ARMOR))
		tipo := elem.TipoProtecao;
	endif

    return tipo;
endfunction

function getSlot(item) //PEGA O SLOT DO ITEM NO .CFG
    var elem := getItemCfgElem(item);
    var slot := elem.Coverage;
    if (IsShield(item)) slot := "Shield"; endif
    return slot;
endfunction

function IsDualWield(item)
	var prop := getPropBase(item);

	if (prop.DualWield)
		return 1;
	endif

	return 0;
endfunction

function IsShield(item)
    var tipoequip := getType(item);
    var escudo := SplitWords(tipoequip); escudo := escudo[1];
	escudo.Lower();

    if (escudo == "escudo")
		return 1;
    else
		return 0;
    endif
endfunction

function getAnim(anims)
	return anims[RandomInt(anims.Size())+1];
endfunction
function getSound(anims)
	return anims[RandomInt(anims.Size())+1];
endfunction