use uo;
include "include/facings";
include ":stables:include/stable";

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

function leaveGolem(who)
	EraseObjProperty(who, "skill_a");
	EraseObjProperty(who, "dicedmg");

	var hitsmod := CInt(GetObjProperty(who, "golemhitsmod"));
	SetObjProperty(who, "hitsmod", CInt(GetObjProperty(who, "hitsmod")) - hitsmod);
	var resist := CInt(GetObjProperty(who, "golemresistmod"));
	if (!resist)
		resist := CInt(GetObjProperty(who, "resistmod"));
	endif

	who.resist_physical_mod := who.resist_physical_mod - resist;
	who.resist_fire_mod := who.resist_fire_mod - resist;
	who.resist_cold_mod := who.resist_cold_mod - resist;
	who.resist_poison_mod := who.resist_poison_mod - resist;
	who.resist_energy_mod := who.resist_energy_mod - resist;

	EraseObjProperty(who, "golemhitsmod");
	EraseObjProperty(who, "golemresistmod");
endfunction

function enterGolem(who)
	var metalworking := Ap_GetSkill(who, METALWORKING);

	var skill_a := metalworking * 0.8;
	var dicedmg := array{ "TWOHANDED_LOW", "TWOHANDED_LOW", "TWOHANDED_LOW" };
	var hitsmod := CInt(metalworking * 2);
	var resistmod := CInt(metalworking * 0.6);

	SetObjProperty(who, "skill_a", skill_a);
	SetObjProperty(who, "dicedmg", dicedmg);
	SetObjProperty(who, "golemhitsmod", hitsmod);
	SetObjProperty(who, "golemresistmod", resistmod);

	who.resist_physical_mod := who.resist_physical_mod + resistmod;
	who.resist_fire_mod := who.resist_fire_mod + resistmod;
	who.resist_cold_mod := who.resist_cold_mod + resistmod;
	who.resist_poison_mod := who.resist_poison_mod + resistmod;
	who.resist_energy_mod := who.resist_energy_mod + resistmod;

	SetObjProperty(who, "hitsmod", CInt(GetObjProperty(who, "hitsmod")) + hitsmod);

	recalcvitals(who);
	ApplyEpicHealing(who, hitsmod);
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

//pega os pets e summons da pessoa na area
function ListAllPetsAndSummonedPetsNear(who,  range)

	var pets := array;
	var tamedPets := ListPetsNear(who,  range);
	var summonedPets := ListSummonedPetsNear(who,  range);

	if ( !tamedPets && !summonedPets)
		return pets;
	endif

	foreach pet in tamedPets
		pets.append(pet);
	endforeach

	foreach pet in summonedPets
		pets.append(pet);
	endforeach

	return pets;

endfunction

//pega os Summoned Pets da pessoa na area
function ListSummonedPetsNear(who,  range)

	var pets := array;
	var tamed := GetObjProperty(who, "SummonedPets");
	if ( !tamed )
		return pets;
	endif

	foreach serial in tamed
		var pet := SystemFindObjectBySerial(serial);
		var owner := GetObjProperty(pet, "owner");
		if ( (owner == who || owner == who.serial || cstr(owner) == cstr(who.serial))  && (Distance(pet, who) <= range))
			pets.append(pet);
		endif
		sleepms(2);
	endforeach

	return pets;

endfunction

//pega os pets da pessoa na area
function ListPetsNear(who,  range)
	var pets := array;
	// Stabled/limbo nao participam de combate — GetActivePets filtra.
	foreach pet in GetActivePets(who)
		if ((pet.master == who) && (Distance(pet, who) <= range))
			pets.append(pet);
		endif
		sleepms(2);
	endforeach
	return pets;
endfunction

// True for NPCs flagged as a "true boss" via the `true_boss` ObjProp.
// Used by loot generation (5x drop chance multiplier), charm/hypnotize spells
// (immune to mind-control), and taming (can't be tamed).
function IsTrueBoss(npc)
	if (!npc || npc == error)
		return 0;
	endif
	var flag := GetObjProperty(npc, "true_boss");
	if (flag == error || !flag)
		return 0;
	endif
	return 1;
endfunction
