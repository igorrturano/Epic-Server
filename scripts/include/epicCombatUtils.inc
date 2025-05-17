use uo;
include "include/facings";

function isAssassinWeapon(who)
    var assassin_weapons := {
        0x27EF, 0x27ab, 0x27af, 11570, 11565, 11566, 0x1401, 0x1440, 0x0f51, 0x0f63, 0x143c, 0x13B2, 0xF9FC, 0xF99F, 
        0xF998, 0xF99B, 0xF99A, 0xF999, 0xF99D, 0xF9A1, 0xF9A2, 0xF9D2, 0xF9CA, 0xF9DC, 0xF9F4,  0xF9A0, 0xF9D4, 0xF9DE,
        0xF9E5, 0xF9A9, 0xF9B5, 0xF9C1, 0xF99E, 0xF9DB, 0xFA4D, 0xFA4E, 0xFA4F, 0xFA51, 0x27a9, 0x27A3, 0x8fd
        };
    foreach weapon in assassin_weapons
        if (weapon == who.weapon.objtype)
            return 1;
        endif
    endforeach

    return 0;
endfunction

function isWoodWeapon(who)
    var wood_weapons := { 
        0xF996, 0x0df0, 0xECE6, 0x13f9, 0x0e8a, 0x0e81, 0x27AD, 0x27AE, 0x27A7, 0x13b3, 0xF9C0, 0x13B2, 0xe137, 0x26CC, 0xe139, 0x2d2b, 
        0x2d2a, 0x0f4f, 0x13fd, 0x26CD, 0x27A5, 0xF983, 0xF9BF, 0xFA14, 0xFA15
    };
    foreach weapon in wood_weapons
        if (weapon == who.weapon.objtype)
            return 1;
        endif
    endforeach

    return 0;
endfunction

function isFireWeapon(item)
    var fire_weapons := { 
        0xC5FD, 0xC5FE, 0xC5FF, 0xC600, 0x89A7
    };

    foreach weapon in fire_weapons
        if (weapon == item.objtype)
            return 1;
        endif
    endforeach

    return 0;
endfunction

function leaveGolem(who)
	EraseObjProperty(who, "skill_a");
	EraseObjProperty(who, "skill_d");
	EraseObjProperty(who, "dicedmg");

    var hitsmod := cint(getObjProperty(who, "golemhitsmod"));
	setobjproperty(who, "hitsmod", cint(getobjproperty(who,"hitsmod")) - hitsmod);
	var resist := cint(GetObjProperty(who, "resistmod"));

	who.resist_physical_mod := who.resist_physical_mod - resist;
	who.resist_fire_mod := who.resist_fire_mod - resist;
	who.resist_cold_mod := who.resist_cold_mod - resist;
	who.resist_poison_mod := who.resist_poison_mod - resist;
	who.resist_energy_mod := who.resist_energy_mod - resist;

	EraseObjProperty(who, "polymorphbonus");
	EraseObjProperty(who, "metamorfose");
	EraseObjProperty(who, "golemhitsmod");
endfunction

function enterGolem(who)
    var skill_a := Ap_GetSkill(who, METALWORKING) * 0.8;
    var skill_d := Ap_GetSkill(who, METALWORKING) * 0.6;
    var dicedmg := "1d12";
    var hitsmod := 200;

	SetObjProperty(who, "skill_a", skill_a);
	SetObjProperty(who, "skill_d", skill_d);
	SetObjProperty(who, "dicedmg", dicedmg);
	SetObjProperty(who, "golemhitsmod", hitsmod);

    var resistmod := 60;
	who.resist_physical_mod := resistmod;
	who.resist_fire_mod := resistmod;
	who.resist_cold_mod := resistmod;
	who.resist_poison_mod := resistmod;
	who.resist_energy_mod := 0;
    
	SetObjProperty(who, "resistmod", resistmod);
	SetObjProperty(who, "hitsmod", hitsmod);

	recalcvitals(who);
	HealDamage(who, hitsmod);
endfunction

function moveToBehind(attacker, defender)
	if ( !IsBehind( attacker, defender.x, defender.y ) && IsFacing( defender, attacker.x,
	  attacker.y ) )
	  var behindX := defender.x + ( defender.x - attacker.x );
	  var behindY := defender.y + ( defender.y - attacker.y );

	  if ( defender.x == attacker.x && defender.y == attacker.y )
		SendSysMessageEx( attacker, "Você esta muito proximo do oponente!", SSM_FAIL );
		return 0;
	  else
		PlayObjectCenteredEffect( attacker, 0x5433, 7, 16 );
		PlaySoundEffect( attacker, 0x526 );
		MoveObjectToLocation( attacker, behindX, behindY, defender.z, defender.realm, MOVEOBJECT_FORCELOCATION );
		if ( !IsFacing( attacker, defender.x, defender.y ) )
		  TurnObjectToward( attacker, defender.x, defender.y );
		endif
	  endif
	endif
endfunction

function listAllMobilesNearbyButPetsAndParty(who, range, targ := 0)
   if (targ == 0)
      targ := who;
   endif

   var mobiles := array{};
   var excluded_serials := array{who.serial};
   var alreadyProcessed := dictionary{};
   
   // Add party members to excluded list
   if (who.party)
      foreach member in (who.party.members)
         excluded_serials.append(member.serial);
         
         // Get pets of party members
         var memberObj := SystemFindObjectBySerial(member.serial);
         if (memberObj)
            var allPets := ListAllPetsAndSummonedPetsNear(memberObj, range);
            foreach pet in allPets
               excluded_serials.append(pet.serial);
            endforeach
         endif
      endforeach
   else
      // If not in party, still exclude own pets
      var ownPets := ListAllPetsAndSummonedPetsNear(who, range);
      foreach pet in ownPets
         excluded_serials.append(pet.serial);
      endforeach
   endif

   // Get all mobiles near target location
   var nearby := ListMobilesNearLocation(targ.x, targ.y, targ.z, range, targ.realm);
   foreach char in nearby
      // Skip if already processed or in excluded list
      if (char.serial in excluded_serials || alreadyProcessed[char.serial])
         continue;
      endif
      
      // Skip if char is owned by the caster
      var char_owner := GetObjProperty(char, "owner");
      if (char_owner == who.serial || CStr(char_owner) == CStr(who.serial))
         continue;
      endif
      
      // Add to results and mark as processed
      alreadyProcessed[char.serial] := 1;
      mobiles.append(char);
   endforeach

   return mobiles;
endfunction

