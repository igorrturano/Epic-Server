use uo;

include ":timedScripts:timedScripts";

function ListTamablePetsNearLocation(x, y, z, range, realm := _DEFAULT_REALM)

	var cfg := ReadConfigFile(":brainAI:npcdesc");

	var mobs := array;
	foreach mob in ListMobilesNearLocation( x, y, z, range, realm);

		if (mob.npctemplate)

			var npctemplate := mob.npctemplate;
			npctemplate := npctemplate[10, len(npctemplate)-9];

			if ( cfg[npctemplate].TameDifficulty )
				//if (!mob.script["tamed"])
					mobs.append(mob);
				//endif
			endif

		endif

	endforeach

	return mobs;

endfunction

//ve quantos slots custa o npc
function GetPetCost(pet)

	var cfg := ReadConfigFile(":brainAI:npcdesc");
	var npctemplate := pet.npctemplate;
	npctemplate := npctemplate[10, len(npctemplate)-9];
	cfg := cfg[npctemplate];
	if (!Cint(cfg.petslot))
		return 1;
	else
		return Cint(cfg.petslot);
	endif

endfunction

/* funcao pega o custo dos pets. tbm atualiza os pets perdidos ou que foram transferidos*/
function GetNumPets(who)
	var num_tamed := 0;
	var tamed := GetObjProperty(who, "TamedPets");
	if ( !tamed )
		return 0;
	endif

	foreach serial in tamed
		var pet := SystemFindObjectBySerial(serial);
		if ( pet.master == who && !GetObjProperty(pet, "summoned")) //se não é summon
			var cfg := ReadConfigFile(":brainAI:npcdesc");
			var npctemplate := pet.npctemplate;
			npctemplate := npctemplate[10, len(npctemplate)-9];
			cfg := cfg[npctemplate];
			if (!Cint(cfg.petslot))
				num_tamed := num_tamed+1;
			else
				num_tamed := num_tamed + Cint(cfg.petslot);
			endif
		else
			tamed.erase(_serial_iter);
		endif
		sleepms(2);
	endforeach

	if(!tamed)
		tamed := array;
	endif
	SetObjProperty(who, "TamedPets", tamed);
	who.followers := num_tamed;
	return num_tamed;
endfunction

function GetNumSummons(who)
	var num_tamed := 0;
	var tamed := GetObjProperty(who, "SummonedPets");
	if ( !tamed )
		return 0;
	endif

	foreach serial in tamed
		var pet := SystemFindObjectBySerial(serial);
		if ( pet.master == who && GetObjProperty(pet, "summoned")) //se é summon
			var cfg := ReadConfigFile(":brainAI:npcdesc");
			var npctemplate := pet.npctemplate;
			npctemplate := npctemplate[10, len(npctemplate)-9];
			cfg := cfg[npctemplate];
			if (!Cint(cfg.petslot))
				num_tamed := num_tamed+1;
			else
				num_tamed := num_tamed + Cint(cfg.petslot);
			endif
		else
			tamed.erase(_serial_iter);
		endif
		sleepms(2);
	endforeach
	if(!tamed)
		tamed := array;
	endif
	SetObjProperty(who, "SummonedPets", tamed);
	return num_tamed;
endfunction

//deleta o serial da lista de seriais do char (quando o npc morre ou eh transferido)
function ErasePet(who, myserial)

	var tamed := GetObjProperty(who, "TamedPets");
	if ( tamed )
		foreach serial in tamed
			if ( myserial == serial )
				MP_Unmount(who);
				tamed.erase(_serial_iter);
				SetObjProperty(who, "TamedPets", tamed);
				var pet := SystemFindObjectBySerial(serial);
				who.followers := who.followers - GetPetCost(pet);
				break;
			endif
			sleepms(2);
		endforeach
	endif
	tamed := GetObjProperty(who, "SummonedPets");
	if ( tamed )
		foreach serial in tamed
			if ( myserial == serial )
				var pet := SystemFindObjectBySerial(serial);
				TS_LowerDuration(pet, "summon", -1);
				MP_Unmount(who);
				tamed.erase(_serial_iter);
				SetObjProperty(who, "SummonedPets", tamed);
				var timer_num := GetObjProperty(pet, "slot");
				var timer := "summon_"+timer_num;
				TS_LowerDuration(who, timer, -1);				
				break;
			endif
			sleepms(2);
		endforeach
	endif
	return;
endfunction

//adiciona o serial do pet
function AddPet(who, serial)
	var tamed := GetObjProperty(who, "TamedPets");
	if ( !tamed )
		tamed := array;
	endif
	tamed.append(serial);
	SetObjProperty(who, "TamedPets", tamed);

	var pet := SystemFindObjectBySerial(serial);
	who.followers := who.followers + GetPetCost(pet);
endfunction

//numero maximo de pets que alguem pode ter
function GetMaxPets(who)
	var maxpets := 1;

	//Influência da skill na quantidade
	var skill := cint(AP_GetSkill(who, HANDLEANIMAL));
	if (skill >= 45 && skill < 75)
		maxpets := maxpets + 1;
	elseif (skill >= 75 && skill < 90)
		maxpets := maxpets + 2;
	elseif (skill >= 90)
		maxpets := maxpets + 3;
	endif

	//Influência de Habs na quantidade
	if(TemHabilidade(who, "Lider do Bando"))
		maxpets := maxpets + 1;
	endif
	if(TemHabilidade(who, "Boiadeiro"))
		maxpets := maxpets + 2;
	endif
	if(TemHabilidade(who, "Rei do Gado"))
		maxpets := maxpets * 2;
	endif

	maxpets := maxpets + cint( GetObjProperty(who, "extrapets") );

	return maxpets;
endfunction

function GetMaxSummons(who)
	var maxpets := 1;

	//Influência da skill na quantidade
	var skill := cint(AP_GetSkill(who, MAGICLORE));
	if (skill >= 45 && skill < 75)
		maxpets := maxpets + 1;
	elseif (skill >= 75 && skill < 90)
		maxpets := maxpets + 2;
	elseif (skill >= 90)
		maxpets := maxpets + 3;
	endif
	return maxpets;
endfunction

//desloga os pets, teleportando eles pra um lugar l
function LogoutPets(who)

	var tamed := GetObjProperty(who, "TamedPets");
	if ( !tamed )
		return;
	endif

	foreach serial in tamed
		var pet := SystemFindObjectBySerial(serial);
		if ( (pet.master == who)  && (Distance(pet, who) < 3))
			pet.hidden := 1;
			var location := array;
			location.append(pet.x);
			location.append(pet.y);
			location.append(pet.z);
			MoveObjectToLocation(pet, 6066, 60, 22, pet.realm, MOVEOBJECT_FORCELOCATION);
			SetObjProperty(pet, "previous_location", location);
		endif
		sleepms(2);
	endforeach


endfunction

//pega os pets da pessoa na area
function ListPetsNear(who,  range)

	var pets := array;
	var tamed := GetObjProperty(who, "TamedPets");
	if ( !tamed )
		return pets;
	endif

	foreach serial in tamed
		var pet := SystemFindObjectBySerial(serial);
		if ( (pet.master == who)  && (Distance(pet, who) <= range))
			pets.append(pet);
		endif
		sleepms(2);
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

//traz os pets pra proximo do dono.
function LogonPets(who)

	var tamed := GetObjProperty(who, "TamedPets");
	if ( !tamed )
		return;
	endif

	foreach serial in tamed
		var pet := SystemFindObjectBySerial(serial);
		if ( (pet.master == who)  )
			pet.hidden := 0;
			var location := GetObjProperty(pet, "previous_location");
			MoveObjectToLocation(pet, location[1], location[2], location[3], pet.realm, MOVEOBJECT_FORCELOCATION);
			EraseObjProperty(pet, "previous_location");
		endif
		sleepms(2);
	endforeach

endfunction

//igual a untame que tem na IA :ghaia:tamed
function destama(serial)

	var me := SystemFindObjectBySerial(serial);
	me.SetMaster(0);
	var orig_script := GetObjProperty(me, "script");
	me.script := orig_script;
	SetName(me, GetObjProperty(me, "name"));
	EraseObjProperty(me, "owner");
	EraseObjProperty(me, "name");
	EraseObjproperty(me, "script");
	EraseObjProperty(me, "happiness");
	EraseObjProperty(me, "bond");
	EraseObjProperty(me,"countercheck");
	EraseObjProperty(me, "NeverRelease");
	EraseObjProperty(me, "Fidelizar?");
	RestartScript(me);
	ErasePetMods(me);
endfunction

function isTamable(mob)

	if (!mob.npctemplate)
		return 0;
	endif

	var cfg := ReadConfigFile(":brainAI:npcdesc");
	var npctemplate := mob.npctemplate;
	npctemplate := npctemplate[10, len(npctemplate)-9];
	cfg := cfg[npctemplate];
	if ( !cfg.TameDifficulty )
		return 0;
	else
		return 1;
	endif


endfunction


//funcao da hab companheiro animal

function SetPetMods(pet)
	var hits := Cint(GetVital(pet, "hits")/100);
	SetObjProperty(pet, "hitsmod", hits);
	recalcvitals(pet);
	healdamage(pet, 255);

	var npctemplate := NPC_ParseTemplateName(pet.npctemplate);
	var cfg := ReadConfigFile(":brainai:npcdesc");
	var phys := cint(cfg[npctemplate.template].PhysicalResist);
	if (phys > 35)
		phys := 35;
	endif
	pet.resist_physical_mod := phys;

endfunction

function ErasePetMods(pet)

	EraseObjProperty(pet, "hitsmod");
	recalcvitals(pet);

	var npctemplate := NPC_ParseTemplateName(pet.npctemplate);
	var cfg := ReadConfigFile(":brainai:npcdesc");
	var phys := cint(cfg[npctemplate.template].PhysicalResist);
	if (phys > 35)
		phys := 35;
	endif
	if (pet.resist_physical_mod > 0)
		pet.resist_physical_mod := pet.resist_physical_mod - phys;
	endif

endfunction
