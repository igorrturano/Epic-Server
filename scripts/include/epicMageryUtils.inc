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
   endcase

   return effect;
endfunction

function altarByObjtype(who, altar)
   var objtype := altar.objtype;
   var religion := "";
   case (objtype)
      0xFA67:
         religion := ELUITHORTH;
      0xFA68:
         religion := LORE;
      0xFA63:
         religion := VENERATIO;
      0xFA64:
         religion := BOKK_BHARAZ;
      0xFA65:
         religion := KAI;
      0xFA66:
         religion := USHKRAR;
      default:
         religion := CETICOS;
   endcase

   return religion;
endfunction

function graphicByAltar(altar)
    print(altar);
   var graphic := 0;
   case (altar)
      ELUITHORTH:
         graphic := 0xFA20;
      LORE:
         graphic := 0xFA25;
      VENERATIO:
         graphic := 0xD1EA;
      BOKK_BHARAZ:
         graphic := 0xD1EF;
      KAI:
         graphic := 0xD1F4;
      USHKRAR:
         graphic := 0xD1F9;
      default:
         graphic := 0;
   endcase
    print ("graphic: " + graphic);
   return graphic;
endfunction


function listAllMobilesNearbyButPetsAndParty(who)
   var mobiles := array{};
   var allies := who.party;
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

   var nearby := ListMobilesNearLocation( who.x ,who.y, who.z, range, who.realm);
   foreach char in nearby
      var char_owner := GetObjProperty(char, "owner");
      if (!(char in (allies.members) || char == who))
         if (!(char in allPets) && !(char in who.party.members) && !(char_owner == who.serial))
            mobiles.append(char);
         endif
      endif
   endforeach

   return mobiles;
endfunction