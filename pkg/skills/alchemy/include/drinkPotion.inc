include "include/attributes";
include ":poisonwatcher:poisons";
include "include/statMod";
include "include/d20system";
use cfgfile;

const POISON_SOUND := 0x247;
const DRINK_SOUND := 0x32;
//const UOBJ_EMPTY_BOTTLE := 0x0f0e;
const EFFECT_BLUE_SPARKLE  := 0x373a;
const SOUND_EFFECT_CURE	:= 0x1e1;
const EFFECT_SPARKLE  := 0x374a;
const HIT_ACTION := 0x148b;
const MOVING_EFFECT_PURPLE_POTION  := 0x0f0d;
const SOUND_EFFECT_EXPLOSION	:= 0x208;
const SOUND_EFFECT_HEAL	:= 0x1f3;
const EFFECT_EXPLOSION := 0x36b0;

function checkifcandrink(character, type)
  // Check to make sure they have at least one hand free to drink the potion
  var equippedright := GetEquipmentByLayer(character, 0x01);
  var equippedleft := GetEquipmentByLayer(character, 0x02);
  var cfg := ReadConfigFile(":*:itemdesc");
  var twohanded := Cint(cfg[equippedleft.objtype].TwoHanded);
  if (equippedright && equippedleft || twohanded)
    SendSysMessage(character, "Voce precisa de pelo menos uma mao livre!");
    return 0;
  endif
  if((type != "red") && (type != "green_potion"))
    if(GetObjProperty(character, type))
      if(GetObjProperty(character, type) > ReadGameClock())
        SendSysMessage(character, "Voce deve esperar um pouco antes de beber outra pocao!");
        return 0;
      else
        SetObjProperty(character, type, ReadGameClock() + 10);
        return 1;
      endif
    else
      SetObjProperty(character, type, ReadGameClock() + 10);
       return 1;
    endif
  else
    return 1;
  endif
endfunction

function empty_bottle(character, potion, potiontype := 0)

  if(GetObjProperty(potion, "acabou") == 1)
  var pot2 := SubtractAmount( potion, 1 );
  var stacknow := GetAmount(0x0F0E);
    if (stacknow > 0)
      Addamount(0x0F0E, 1);
    else
      var pot := CreateItemInBackpack(character, 0x0F0E, 1);
    endif
  return 1;
  endif

  PlaySoundEffect(character, DRINK_SOUND);
  PrintText(character, "*usa uma pocao*");
  var goles;

  if (!GetObjProperty(potion, "goles"))
    goles := 3;
    SetObjProperty(potion, "goles", goles);
  else
    goles := cint(GetObjProperty(potion, "goles"));
  endif

  if(potiontype == 1)  //eh uma poção de 1 so gole
    goles := 0;
  endif

  if (goles > 1)
    goles := goles - 1;
    SetObjProperty(potion, "goles", goles);
  else
    SetObjProperty(potion, "acabou", 1);
    SetObjProperty(potion, "goles", 0);
  endif
endfunction

function is_poison(potion)
  var poison_level := GetObjProperty(potion, "poison_level");
  return poison_level;
endfunction

function do_poison(who, potion)
  empty_bottle(who, potion);
  var acfgfile := ReadConfigFile(":*:itemdesc");
  var theitem := acfgfile[the_item.objtype];
  var level, difficulty, points, poisonType;
  level := Cint(GetObjProperty(the_poison, "level"));
  if (!level)
    level := RandomDiceRoll("1d5");
  endif
  //se for uma poçao que nao a default a formula eh essa:
  difficulty := 15*level + 10;
  points := difficulty;
  case(the_poison.objtype)
    0xbf00: level := 1; difficulty := 40; points := 50; poisonType := "defaultPoison";
      0xbf01: poisonType :="strPoison"; difficulty := 60; points := 70;
    0xbf07: poisonType :="dexPoison"; difficulty := 60; points := 70;
    0xbf06: poisonType :="intPoison"; difficulty := 60; points := 70;
    0xbf02: poisonType:="staminaPoison"; difficulty := 60; points := 70;
    0xbf03: poisonType:="paralyzePoison"; difficulty := 80; points := 100;
    0xef54: poisonType :="manaPoison"; difficulty := 40; points := 70;
    0xbf04: level := 3; difficulty := 80; points := 60; poisonType := "defaultPoison";
    0xdc04: level := 2; difficulty := 60; points := 40; poisonType := "defaultPoison";
    0xbf05: level := 4; difficulty := 100; points := 100; poisonType := "defaultPoison";
    default: difficulty := 0; points := 0; poisonType := "invalid";
  endcase

  if(!CheckResist(critter, FORTITUDE, difficulty, 0))
 		SendSysMessage(who, "Voce foi envenenado", 3, 40);
    TS_StartTimer(critter, "defaultPoison", 30, lvl, who);
  else
    SendSysMessage(who, "Voce resistiu ao veneno", 3, 90);
  endif
endfunction

