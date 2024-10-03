include "include/epicConstants";

function playAltarAnimations(who, item)
   var altar := altarByObjtype(who, item);
   var newGraphic := Cint(graphicByAltar(altar));
   item.graphic := newGraphic;
   sleep(2);
   item.graphic := item.objtype;
   var effect := effectByAltar(altar);
   PlaySoundEffect(who, SFX_SPELL_HEAL);
   PlayStationaryEffectEx(who.x, who.y, who.z,who.realm, effect, 255, 80, 0, 0);
endfunction

function effectByAltar(altar)
   var effect := 0;
   case (altar)
      ELUITHORTH:
         effect := 0x54F7;
      LORE:
         effect := 0xB462;
      VENERATIO:
         effect := 0x54B3;
      BOKK_BHARAZ:
         effect := 0x6DF1;
      KAI:
         effect := 0x6d57;
      USHKRAR:
         effect := 0X6C91;
      CETICOS:
         effect := 0;
      HALLEUAD:
         effect := 0x6CEB; //
   endcase

   return effect;
endfunction

function altarByObjtype(who, altar)
   var objtype := altar.objtype;
   var religion := "";
   case (objtype)
      0xA4A8: //0xFA67
         religion := ELUITHORTH;
      0xA06F: //0xFA68
         religion := LORE;
      0xA065: // 0xFA63
         religion := VENERATIO;
      0x9E81: // 0xFA64
         religion := BOKK_BHARAZ;
      0xA252: //0xFA65
         religion := KAI;
      0xA06A: // 0xFA66
         religion := USHKRAR;
      0xA261:
         religion := CETICOS;
      0xA060:
         religion := HALLEUAD;
   endcase

   return religion;
endfunction

function graphicByAltar(altar)
    print(altar);
   var graphic := 0;
   case (altar)
      ELUITHORTH:
         graphic := 0xA4A9; //0xFA20;
      LORE:
         graphic := 0xA070;//0xFA25;
      VENERATIO:
         graphic := 0xA066;//0xD1EA;
      BOKK_BHARAZ:
         graphic := 0x9DF2; // 0xD1EF;
      KAI:
         graphic := 0xA253;//0xD1F4;
      USHKRAR:
         graphic := 0xA06B;//0xD1F9;
      HALLEUAD:
         graphic := 0xA061;
      CETICOS:
         graphic := 0xA262;
      default:
         graphic := 0;
   endcase
   return graphic;
endfunction

function listAllMobilesNearbyButPetsAndParty(who, range, targ := 0)
   if (targ == 0)
      targ := who;
   endif

   var mobiles := array{};
   var allies := who.party;
	var alreadydamaged := array{};

   var party_members := who.party.members;
      if (who.party)
      foreach member in party_members
         if (member != who)
               var memberObj := SystemFindObjectBySerial(member.serial);
               var allPetsAndSummonedPets := ListAllPetsAndSummonedPetsNear(memberObj, 8 * 4);
               foreach pet in allPetsAndSummonedPets
                  mobiles.append(pet);
               endforeach
         endif
      endforeach
   endif

   var nearby := ListMobilesNearLocation( targ.x ,targ.y, targ.z, range, targ.realm);
   foreach char in nearby
      var char_owner := GetObjProperty(char, "owner");
      if (!(char in (allies.members) || char == who))
         if (!(char.serial in alreadydamaged) && !(char in mobiles) && !(char in who.party.members) && !(char_owner == who.serial))
            alreadydamaged.Append(char.serial);
            mobiles.append(char);
         endif
      endif
   endforeach

   return mobiles;
endfunction

function distanceAllowedForSpells(who, mlore, school)
   var dist_allowed := 5 + mlore;

   if (TemHabilidade(who, "Evocator") && school == EVOCATIUM)
      dist_allowed := dist_allowed + (mlore * 2);
   elseif (TemHabilidade(who, "Valdruk") && school == VELKYN)
      dist_allowed := dist_allowed + (mlore * 2);
   elseif (TemHabilidade(who, "Tirothan") && school == EVOCATIUM)
      dist_allowed := dist_allowed + (mlore * 2);
   endif

   return dist_allowed;
endfunction

function bonusSpellDamageForSpecialists(who, dmg, school)
   var bonus := 1.2;

   if (TemHabilidade(who, "Evocator") && school == EVOCATIUM)
      dmg := Cint(dmg * bonus);
   elseif (TemHabilidade(who, "Valdruk") && school == VELKYN)
      dmg := Cint(dmg * bonus);
   endif

   return dmg;
endfunction

function epicSpellDamageRoll(who, spell_circle)
   var dice := cstr("1d4");
   if (spell_circle == 1)
      dice := cstr("2d4");
   elseif (spell_circle == 2)
      dice := cstr("4d6");
   elseif (spell_circle == 3)
      dice := cstr("6d8");
   elseif (spell_circle == 4) 
      dice := cstr("8d10");
   elseif (spell_circle == 5) 
      dice := cstr("10d12");
   endif

   var dmg := RandomDiceRoll(dice);

   var bonus := epicWisdomPathBonus(who);
   dmg := Cint(dmg + bonus);

   return dmg;
endfunction

function handleSilentMagic(who, targ := 0, sound := 0)
    if (!GetObjProperty(who, "#magiasilenciosa") && sound != 0 && targ != 0)
        PlaySoundEffect(targ, sound);
        return 0;
    endif

    SendSysMessageEx(who, "O efeito da magia silenciosa acabou.", SSM_INFO);
    EraseObjProperty(who, "#magiasilenciosa");
    return 0;
endfunction

function handleSpellResistDiffForSpecialists(who, school)
   var diff := RESIST_EASY;
   if (TemHabilidade(who, "Evocator") && school == EVOCATIUM)
      diff := RESIST_MEDIUM;
   elseif (TemHabilidade(who, "Valdruk") && school == VELKYN)
      diff := RESIST_MEDIUM;
   elseif (TemHabilidade(who, "Cetro da Morte") && school == KAIJIN)
		diff := RESIST_MEDIUM;
   endif
 
   return diff;
endfunction

function bonusSpellDurationForSpecialists(who, duration, school)
   var bonus := 1.2;

   if (TemHabilidade(who, "Evocator") && school == EVOCATIUM)
      duration := Cint(duration * bonus);
   elseif (TemHabilidade(who, "Valdruk") && school == VELKYN)
      duration := Cint(duration * bonus);
   elseif(Temhabilidade(who, "Treinamento Druídico") && school == ARADALORE)
      duration := Cint(duration * bonus);
   endif

   return duration;
endfunction

function checkForElementalWeapons(who, targ)
		var encantada := GetObjProperty(targ, "#elementalweapon");
		if (encantada != error)
			if (readgameclock() < encantada.endtime )
				SendSysMessageEx(who, "Essa arma já está com um encantamento.", SSM_FAIL);
				return 1;
			endif
		endif

		if ( GetCooldown(targ, "blessweapon") > 0 )
			SendSysMessageEx(who, "Essa arma já está com um encantamento.", SSM_FAIL);
			return 1;
		endif

      if (targ.objtype == FIST)
         SendSysMessageEx(who, "Você não pode encantar suas mãos.", SSM_FAIL);
         return 1;
      endif

      return 0;
endfunction

function epicWisdomPathBonus(who)
   var int_bonus := Cint((AP_GetStat(who, INTELLIGENCE) - 50) / 10);
   var mlore := Cint(AP_GetSkill(who, MAGICLORE) / 10); 

   return (int_bonus + mlore);
endfunction

//Função para aplicar o efeito de arma elemental do swordmage 
function ApplyElementalWeaponProperties( who, targ, elemento, duracao )

  var dmgdice := DefineElementalWeaponDice(who);

  var dmg := struct;
  dmg.+type := elemento;
  dmg.+serial := who.serial;
  dmg.+endtime := readgameclock() + duracao;
  dmg.+bonus := dmgdice;

  SetObjProperty( targ, "#elementalweapon", dmg );
  start_script( ":spells:elementalweapon", { targ, duracao, elemento, who } );

endfunction

function DefineElementalWeaponDice(who)

  var faixaMLore := AP_GetSkill( who, MAGICLORE ); 
  var dmgdice := "0";

  if ( faixaMLore < 40 )
    dmgdice := cstr( "1d2" );
  elseif ( faixaMLore >= 40 && faixaMLore < 70 )
    dmgdice := cstr( "1d4" );
  elseif ( faixaMLore >= 70 && faixaMLore < 90 )
    dmgdice := cstr( "1d6" );
  elseif (faixaMLore >= 90 )
    dmgdice := cstr( "1d8" );
  endif

  return dmgdice;
endfunction