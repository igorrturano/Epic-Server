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
	SHIELD_BUCKLER := "ESCUDO BROQUEL", SHIELD_SMALL := "ESCUDO PEQUENO", SHIELD_MEDIUM := "ESCUDO MEDIO", SHIELD_LARGE := "ESCUDO GRANDE", SHIELD_TOWER := "ESCUDO TORRE"
endenum
enum WEAPONFX
	WEAPON_FX_SOUND, WEAPON_FX_ANIM
endenum

var MISS_FX := struct {
	SFX := array{0x0239, 0x023A, 0x023B, 0x539, 0x53A, 0x53B}
};
var UNARMED_FX := struct {
	ANIM := array{0x9, 0xA},
	SFX := array{0x136, 0x138, 0x13C, 0x13D, 0x13E, 0x142, 0x145, 0x146, 0x149, 0x14A, 0x14B}
};
var BOW_FX := struct {
	ANIM := array{0x12},
	SFX := array{0x235, 0x4C9, 0x4CA}
};
var CROSSBOW_FX := struct {
	ANIM := array{0x13},
	SFX := array{0x235, 0x4C9, 0x4CA}
};
var SLASH_1H_FX := struct {
	ANIM := array{0xD},
	SFX := array{0x23C,0x23D}
};
var SLASH_2H_FX := struct {
	ANIM := array{0xC},
	SFX := array{0x237,0x238}
};
var PIERCING_1H_FX := struct {
	ANIM := array{0xA},
	SFX := array{0x23C,0x23D}
};
var PIERCING_2H_FX := struct {
	ANIM := array{0xE},
	SFX := array{0x23D, 0x23D}
};
var BLUNT_1H_FX := struct {
	ANIM := array{0xB},
	SFX := array{0x3AC}
};
var BLUNT_2H_FX := struct {
	ANIM := array{0xB},
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
    // var slot := elem.Coverage;
	var slot := IsShield(item) ? "Shield" : elem.Coverage;
    
	// if (IsShield(item))
	// 	slot := "Shield";
	// endif

    return slot;
endfunction

function IsDualWield(item)
	var elem := getItemCfgElem(item);

	if (elem.Attribute.Upper() == DUALWIELD.Upper())
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

function getWeaponFX(FX, weapon)
	var attackFX;
	var elem := getItemCfgElem(weapon);
	var prop := getPropBase(weapon);

	if (!elem)
		if (FX == WEAPON_FX_ANIM)
			attackFX := getAnim(UNARMED_FX);
		else
			attackFX := getSound(UNARMED_FX);
		endif
	else
		if (elem.Attribute.Upper() == RANGED.Upper())
			var isBow := elem.ProjectileType == 0xf3f ? 1 : 0;
			if (isBow)
				if (FX == WEAPON_FX_ANIM)
					attackFX := getAnim(BOW_FX);
				else
					attackFX := getSound(BOW_FX);
				endif
			else
				if (FX == WEAPON_FX_ANIM)
					attackFX := getAnim(CROSSBOW_FX);
				else
					attackFX := getSound(CROSSBOW_FX);
				endif
			endif
		else
			if (elem.Attribute.Upper() == TWOHANDED.Upper())
				if (prop.AttackType.Upper() == SLASHDAMAGE.Upper())
					if (FX == WEAPON_FX_ANIM)
						attackFX := getAnim(SLASH_2H_FX);
					else
						attackFX := getSound(SLASH_2H_FX);
					endif
				elseif (prop.AttackType.Upper() == PIERCINGDAMAGE.Upper())
					if (FX == WEAPON_FX_ANIM)
						attackFX := getAnim(PIERCING_2H_FX);
					else
						attackFX := getSound(PIERCING_2H_FX);
					endif
				elseif (prop.AttackType.Upper() == BLUNTDAMAGE.Upper())
					if (FX == WEAPON_FX_ANIM)
						attackFX := getAnim(BLUNT_2H_FX);
					else
						attackFX := getSound(BLUNT_2H_FX);
					endif
				endif
			else
				if (prop.AttackType.Upper() == SLASHDAMAGE.Upper())
					if (FX == WEAPON_FX_ANIM)
						attackFX := getAnim(SLASH_1H_FX);
					else
						attackFX := getSound(SLASH_1H_FX);
					endif
				elseif (prop.AttackType.Upper() == PIERCINGDAMAGE.Upper())
					if (FX == WEAPON_FX_ANIM)
						attackFX := getAnim(PIERCING_1H_FX);
					else
						attackFX := getSound(PIERCING_1H_FX);
					endif
				elseif (prop.AttackType.Upper() == BLUNTDAMAGE.Upper())
					if (FX == WEAPON_FX_ANIM)
						attackFX := getAnim(BLUNT_1H_FX);
					else
						attackFX := getSound(BLUNT_1H_FX);
					endif
				endif
			endif
		endif
	endif

	return attackFX;
endfunction

function getAnim(fxs)
	return fxs.ANIM[RandomInt(fxs.ANIM.Size())+1];
endfunction

function getSound(fxs)
	return fxs.SFX[RandomInt(fxs.SFX.Size())+1];
endfunction