enum TIERS
	TIER0,
	TIER1,
	TIER2,
	TIER3,
	TIER4,
	TIER5
endenum

enum SPEEDTAG
	MELEE_LOW := "MELEE_LOW",
	MELEE_MID := "MELEE_MID",
	MELEE_HIGH := "MELEE_HIGH",
	
	RANGED_LOW := "RANGED_LOW",
	RANGED_MID := "RANGED_MID",
	RANGED_HIGH := "RANGED_HIGH"
endenum
enum DAMAGETAG
	DAMAGE_NONE := "DAMAGE_NONE",

	DAMAGE_FIST := "DAMAGE_FIST",

	ONEHANDED_LOW := "ONEHANDED_LOW",
	ONEHANDED_MID := "ONEHANDED_MID",
	ONEHANDED_HIGH := "ONEHANDED_HIGH",

	TWOHANDED_LOW := "TWOHANDED_LOW",
	TWOHANDED_MID := "TWOHANDED_MID",
	TWOHANDED_HIGH := "TWOHANDED_HIGH",

	BOW_LOW := "BOW_LOW",
	BOW_MID := "BOW_MID",
	BOW_HIGH := "BOW_HIGH",

	CROSSBOW_LOW := "CROSSBOW_LOW",
	CROSSBOW_MID := "CROSSBOW_MID",
	CROSSBOW_HIGH := "CROSSBOW_HIGH",

	THROWING_LOW := "THROWING_LOW",
	THROWING_MID := "THROWING_MID",
	THROWING_HIGH := "THROWING_HIGH"
endenum
enum WEAPONCATEGORY
	UNARMED := "Unarmed",
	DAGGER := "Dagger",
	SWORD := "Sword",
	GREATSWORD := "Greatsword",
	AXE := "Axe",
	SPEAR := "Spear",
	MACE := "Mace",
	STAFF := "Staff",
	THROWING := "Throwing",
	POLEARM := "Polearm",

	BOW := "Bow",
	CROSSBOW := "Crossbow"
endenum
enum DAMAGETYPE
	SLASHDAMAGE := "Cortante",
	PIERCINGDAMAGE := "Perfurante",
	BLUNTDAMAGE := "Contusivo"
endenum
enum CRITICALTAG
	CRITICALCHANCE_NOCHANCE := "CRITICAL_NOCHANCE",
	CRITICALCHANCE_LOW := "CRITICAL_LOW",
	CRITICALCHANCE_MID := "CRITICAL_MID",
	CRITICALCHANCE_HIGH := "CRITICAL_HIGH"
endenum
enum EQUIPSLOT
	HEAD := "Head",
	NECK := "Neck",
	BODY := "Body",
	ARMS := "Arms",
	HANDS := "Hands",
	LEGS := "Legs",
	FEET := "Feet",
	SHIELD := "Shield",
	WEAPON := "Weapon"
endenum
enum PROTECTIONTYPE
	ARMOR_LOWLOW := "Armadura Leve Fraca",
	ARMOR_LOWMID := "Armadura Leve Media",
	ARMOR_LOWHIGH := "Armadura Leve Forte",

	ARMOR_MIDLOW := "Armadura Media Fraca",
	ARMOR_MIDMID := "Armadura Media Media",
	ARMOR_MIDHIGH := "Armadura Media Forte",

	ARMOR_HIGHLOW := "Armadura Pesada Fraca",
	ARMOR_HIGHMID := "Armadura Pesada Media",
	ARMOR_HIGHHIGH := "Armadura Pesada Forte",

	SHIELD_BUCKLER := "Escudo Broquel",
	SHIELD_SMALL := "Escudo Pequeno",
	SHIELD_MEDIUM := "Escudo Medio",
	SHIELD_LARGE := "Escudo Grande",
	SHIELD_TOWER := "Escudo Torre"
endenum
