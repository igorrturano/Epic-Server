

/////////////////////////////////////////////////////////////////////////////
//
// include/objtype.inc - Object Type definitions
//
// TODO: consider moving these into include/client.inc
//       or, moving CID_XX constants in here
//
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//
// 0x0000 to 0x3FFF - "Standard" objects
//
/////////////////////////////////////////////////////////////////////////////



// Adicionados by Gha
// Forgotten Lore Shard

Const UOBJ_GEM_AMBER		:= 0x0F25;
Const UOBJ_GEM_STAR_SAPPHIRE	:= 0x0F0F;
Const UOBJ_GEM_EMERALD		:= 0x0F10;
Const UOBJ_GEM_SAPPHIRE		:= 0x0F11;
Const UOBJ_GEM_RUBY			:= 0x0F13;
Const UOBJ_GEM_CITRINE		:= 0x0F15;
Const UOBJ_GEM_AMETHYST		:= 0x0F16;
Const UOBJ_GEM_TOURMALINE	:= 0x0F18;
Const UOBJ_GEM_DIAMOND		:= 0x0F30;



///////////////////
//  leather armor
///////////////////

Const UOBJ_LEATHER_GORGET	:= 0x13C7;
Const UBOJ_LEATHER_SLEEVES	:= 0x13C5;
Const UOBJ_LEATHER_SLEEVES2	:= 0x13CD;
Const UOBJ_LEATHER_GLOVES	:= 0x13C6;
Const UOBJ_LEATHER_CAP		:= 0x1DB9;
Const UOBJ_LEATHER_LEGGINGS	:= 0x13CB;
Const UOBJ_LEATHER_TUNIC		:= 0x13CC;
Const UOBJ_LEATHER_TUNIC2	:= 0x13D3;
Const UOBJ_LEATHER_FEMALE	:= 0x1C06;
Const UOBJ_LEATHER_SKIRT		:= 0x1C08;
Const UOBJ_LEATHER_BUSTIER	:= 0x1C0A;
Const UOBJ_LEATHER_SHORTS	:= 0x1C00;




///////////////////
//  studded armor
///////////////////

Const UOBJ_STUDDED_CAP		:= 0x708b;
Const UOBJ_STUDDED_GORGET	:= 0x13D6;
Const UOBJ_STUDDED_GLOVES	:= 0x13D5;
Const UOBJ_STUDDED_SLEEVES	:= 0x13D4;
Const UOBJ_STUDDED_SLEEVES2	:= 0x13DC;
Const UOBJ_STUDDED_LEGGINGS	:= 0x13DA;
Const UOBJ_STUDDED_TUNIC		:= 0x13DB;
Const UOBJ_STUDDED_FEMALE	:= 0x1C02;
Const UOBJ_STUDDED_BUSTIER	:= 0x1C0C;




///////////////////
//  ringmail armor
///////////////////

Const UOBJ_RINGMAIL_COIF		:= 0x708c;
Const UOBJ_RINGMAIL_TUNIC	:= 0x13ec;
Const UOBJ_RINGMAIL_TUNIC2	:= 0x13ed;
Const UOBJ_RINGMAIL_SLEEVES	:= 0x13ee;
Const UOBJ_RINGMAIL_SLEEVES2	:= 0x13ef;
Const UOBJ_RINGMAIL_LEGGINGS	:= 0x13f0;
Const UOBJ_RINGMAIL_LEGGINGS2	:= 0x13f1;
Const UOBJ_RINGMAIL_GLOVES	:= 0x13f2;
Const UOBJ_RINGMAIL_GLOVES2	:= 0x13eb;



///////////////////
//  chainmail armor
///////////////////

Const UOBJ_CHAINMAIL_COIF	:= 0x13bb;
Const UOBJ_CHAINMAIL_COIF2	:= 0x13c0;
Const UOBJ_CHAINMAIL_LEGGINGS	:= 0x13be;
Const UOBJ_CHAINMAIL_LEGGINGS2:= 0x13c3;
Const UOBJ_CHAINMAIL_TUNIC	:= 0x13bf;
Const UOBJ_CHAINMAIL_TUNIC2	:= 0x13c4;
Const UOBJ_CHAINMAIL_GLOVES	:= 0x7066;




///////////////////
//  Platemail armor
///////////////////

Const UOBJ_PLATE_FEMALE		:= 0x1c04;
Const UOBJ_PLATE_FEMALE2		:= 0x1c05;
Const UOBJ_PLATE_BREASTPLATE	:= 0x1415;
Const UOBJ_PLATE_BREASTPLATE2	:= 0x1416;
Const UOBJ_PLATE_ARMS		:= 0x1410;
Const UOBJ_PLATE_ARMS2		:= 0x1417;
Const UOBJ_PLATE_LEGS		:= 0x1411;
Const UOBJ_PLATE_LEGS2		:= 0x141a;
Const UOBJ_PLATE_GLOVES		:= 0x1414;
Const UOBJ_PLATE_GLOVES2		:= 0x1418;
Const UOBJ_PLATE_GORGET		:= 0x1413;




///////////////////
//  Bone armor
///////////////////

Const UOBJ_BONE_ARMS		:= 0x144e;
Const UOBJ_BONE_TUNIC		:= 0x144f;
Const UOBJ_BONE_GLOVES		:= 0x1450;
Const UOBJ_BONE_HELM		:= 0x1451;
Const UOBJ_BONE_LEGS		:= 0x1452;




///////////////////
//  Masks
///////////////////

Const UOBJ_ORC_MASK			:= 0x141B;
Const UOBJ_ORC_MASK2		:= 0x141C;
Const UOBJ_BEAR_MASK		:= 0x1545;
Const UOBJ_BEAR_MASK2		:= 0x1546;
Const UOBJ_DEER_MASK		:= 0x1547;
Const UOBJ_DEER_MASK2		:= 0x1548;
Const UOBJ_TRIBAL_MASK		:= 0x1549;
Const UOBJ_TRIBAL_MASK2		:= 0x154A;
Const UOBJ_TRIBAL_MASK3		:= 0x154B;
Const UOBJ_TRIBAL_MASK4		:= 0x154C;




///////////////////
//  helms
///////////////////

Const UOBJ_PLATE_HELM		:= 0x1412;
Const UOBJ_PLATE_HELM2		:= 0x1419;
Const UOBJ_CLOSEHELM		:= 0x1408;
Const UOBJ_CLOSEHELM2		:= 0x1409;
Const UOBJ_HELMET			:= 0x140a;
Const UOBJ_HELMET2			:= 0x140b;
Const UOBJ_BASCINET			:= 0x140c;
Const UOBJ_BASCINET2		:= 0x140d;
Const UOBJ_NOSEHELM			:= 0x140e;
Const UOBJ_NOSEHELM2		:= 0x140f;
Const UOBJ_ORCHELM			:= 0x1F0B;
Const UOBJ_ORCHELM2			:= 0x1F0C;




///////////////////
//  shields
///////////////////

Const UOBJ_BUCKLER			:= 0x1b73;
Const UOBJ_BRONZESHIELD		:= 0x1b72;
Const UOBJ_METALSHIELD		:= 0x1b7b;
Const UOBJ_KITESHIELD		:= 0x1b74;
Const UOBJ_KITESHIELD2		:= 0x1b75;
Const UOBJ_HEATER			:= 0x1b76;
Const UOBJ_HEATER2			:= 0x1b77;
Const UOBJ_WOODEN_SHIELD		:= 0x1b7a;
Const UOBJ_WOODEN_KITESHIELD	:= 0x1b78;
Const UOBJ_WOODEN_KITESHIELD2	:= 0x1b79;
Const UOBJ_CHAOS_SHIELD		:= 0x1bc3;
Const UOBJ_ORDER_SHIELD		:= 0x1bc4;
Const UOBJ_ORDER_SHIELD2		:= 0x1bc5;




///////////////////
//  woodeden weapons
///////////////////

Const UOBJ_CLUB			:= 0x13b3;
Const UOBJ_SHEEPHERDS_CROOK	:= 0x0e81;
Const UOBJ_BLACK_STAFF		:= 0x0df0;
Const UOBJ_GNARLED_STAFF		:= 0x13f9;
Const UOBJ_MAGE_STAFF		:= 0x7065;
Const UOBJ_DRUID_STAFF		:= 0x7068;
Const UOBJ_NECRO_STAFF		:= 0x706a;
Const UOBJ_QUARTER_STAFF		:= 0x0e8a;


Const UOBJ_FIST			:= 0xF020;
const UOBJ_BACKPACK := 0x0E75;
Const UOBJ_FISHHEAD		:= 0x1e19;


Const UOBJ_WAX                := 0x1422;
Const UOBJ_EMPTY_KETTLE       := 0x09ed;
Const UOBJ_DIPPING_STICK      := 0x1428;
Const UOBJ_KETTLE_OF_WAX      := 0x142a;
Const UOBJ_SKULL              := 0x1ae2;
Const UOBJ_PORCELAIN	      := 0xee21;

Const UOBJ_DEFAULT_LOG	      := 0x1bdd;
Const UOBJ_CYPRESS_LOG	      := 0xee3a;
Const UOBJ_WILLOW_LOG	      := 0xee39;
Const UOBJ_WALNUT_LOG	      := 0xee38;
Const UOBJ_OAK_LOG	      := 0xee37;
Const UOBJ_PINE_LOG	      := 0xee36;

const FLS_SFX_SKILL_LUMBERJACK          :=  0x013f;
