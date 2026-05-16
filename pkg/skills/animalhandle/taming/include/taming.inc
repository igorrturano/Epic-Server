use uo;
use vitals;

include ":timedScripts:timedScripts";
include ":mounts:mounts";
include ":charactercreation:habilidades";
include ":stables:include/stable";
include "include/NPCUtil";
include ":taming:petidentity";
include ":tn:cooldown";

function ListTamablePetsNearLocation(x, y, z, range, realm := _DEFAULT_REALM)

	var cfg := ReadConfigFile(":*:npcdesc");

	var mobs := array;
	foreach mob in ListMobilesNearLocation( x, y, z, range, realm);

		if (mob.npctemplate)

			var npctemplate := mob.npctemplate;
			npctemplate := npctemplate[10, len(npctemplate)-9];

			if ( mob.getTameDifficulty() )
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
	if (!Cint(pet.getTier()))
		return 1;
	else
		return Cint(pet.getTier());
	endif

endfunction

// IsTamedPet() vive em include/petidentity (leve) para consumidores
// sensiveis poderem reutiliza-lo sem o peso de taming.inc. Reexportado
// aqui via include para todos os callers existentes de :taming:taming.

// Criatura de template montaria (mesmo gate que MP_Mount usa: npcdesc
// Environment == "Mount"). Independe de estar sendo cavalgada agora.
function IsMountType(pet)
	var npctemplate := NPC_ParseTemplateName(pet.npctemplate);
	var cfg := ReadConfigFile(":*:npcdesc");
	return cfg[npctemplate.template].Environment == "Mount";
endfunction

// Categoria de slot: "war" (criatura evil de combate, cap 1) ou "nonwar"
// (cinza/neutro OU montaria — nao-combatente, pool compartilhado Taming/20).
function PetSlotCategory(pet)
	if (pet.alignment == 0 || IsMountType(pet))
		return "nonwar";
	endif
	return "war";
endfunction

/* Passo canonico sobre TamedPets. Poda entradas invalidas/summoned, persiste
   a lista limpa, sincroniza who.followers e retorna o tally por categoria.
   Pets no estabulo/limbo continuam do player mas nao custam slot. */
function _TallyPets(who)
	var war := 0;
	var nonwar := 0;
	var tamed := GetObjProperty(who, "TamedPets");
	if ( !tamed )
		who.followers := 0;
		return struct{ war := 0, nonwar := 0, total := 0 };
	endif

	foreach serial in tamed
		var pet := SystemFindObjectBySerial(serial);
		if (!pet)
			// Miss transiente (pet fora do cache, etc). Nao podar a entrada —
			// ondelete.src cuida de exclusoes reais.
			sleepms(2);
			continue;
		endif
		if ( pet.master != who || GetObjProperty(pet, "summoned") )
			tamed.erase(_serial_iter);
			sleepms(2);
			continue;
		endif
		// Pets no estabulo ou em limbo nao ocupam slots, mas continuam pertencendo
		// ao player. Contratos expirados nao sao limpos aqui — o gump do post
		// cuida disso no momento do resgate.
		if ( !IsStabled(pet) && !IsInLimbo(pet) )
			var cost := GetPetCost(pet);
			if (PetSlotCategory(pet) == "war")
				war := war + cost;
			else
				nonwar := nonwar + cost;
			endif
		endif
		sleepms(2);
	endforeach

	if(!tamed)
		tamed := array;
	endif
	SetObjProperty(who, "TamedPets", tamed);
	var total := war + nonwar;
	who.followers := total;
	return struct{ war := war, nonwar := nonwar, total := total };
endfunction

/* funcao pega o custo dos pets. tbm atualiza os pets perdidos ou que foram transferidos*/
function GetNumPets(who)
	return _TallyPets(who).total;
endfunction

function GetNumWarPets(who)
	return _TallyPets(who).war;
endfunction

function GetNumNonWarPets(who)
	return _TallyPets(who).nonwar;
endfunction

function GetNumSummons(who)
    var num_summoned := 0;
    var summoned_pets := GetObjProperty(who, "SummonedPets");

    if (!summoned_pets)  // If no summoned pets, return 0
        return 0;
    endif

    foreach serial in summoned_pets
        var pet := SystemFindObjectBySerial(serial);

        // Check if the pet is still a valid summon
        if (pet && pet.master == who && GetObjProperty(pet, "summoned"))
			var pet_slot := CInt(pet.getTier());
            if (!pet_slot)
                pet_slot := 1;  // Default to 1 if no pet slot is set
            endif

            // Accumulate the number of summoned pets
            num_summoned := num_summoned + pet_slot;
        else
            summoned_pets.erase(_serial_iter);  // Remove invalid pets from the list
        endif

        sleepms(2);  // Prevent performance issues with large lists
    endforeach

    if (!summoned_pets)  // Ensure the array is still initialized properly
        summoned_pets := array;
    endif

    SetObjProperty(who, "SummonedPets", summoned_pets);  // Update the property with valid pets

    return num_summoned;
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
	OwnerClearPetRef(who, myserial);
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

// Libera estado de estabulo/limbo de um pet e desassocia o post.
// Chame antes de transferir o pet ou logo apos libertá-lo/matá-lo.
function ClearPetStableAndLimboState(pet)
	if (!pet)
		return 0;
	endif

	if (IsStabled(pet) || IsInLimbo(pet))
		pet.setParalyzed(0);
		RevokePrivilege(pet, "invul");
		pet.disable("invul");
		pet.hidden := 0;
		pet.frozen := 0;
	endif

	var data := GetPetStable(pet);
	if (data != error && CInt(data["post"]))
		var post := SystemFindObjectBySerial(CInt(data["post"]));
		if (post && GetObjProperty(post, "StoredAnimal") == pet.serial)
			EraseObjProperty(post, "StoredAnimal");
		endif
	endif

	ClearPetStable(pet);
	return 1;
endfunction

// Limpa refs do master para este pet (limbo_pet, post, stabled_pets).
function ClearMasterLimboRef(master, pet_serial)
	if (!master)
		return 0;
	endif
	OwnerClearPetRef(master, pet_serial);
	return 1;
endfunction

// War pet = criatura evil (alignment 1). Cap 1 para todos; Ranger ganha +1
// via a hab Camuflagem (sem rework do efeito — adiado). extrapets (bonus de
// GM/evento) continua somando no lado de combate.
function GetMaxWarPets(who)
	var maxwar := 1;

	if (MobileWrapper(who).HasAbility(ABILITY_ID::VINCULO_DE_GUERRA))
		maxwar := maxwar + 1;
	endif

	maxwar := maxwar + cint( GetObjProperty(who, "extrapets") );

	return maxwar;
endfunction

// NonWar = criaturas cinza (alignment 0) E montarias. Pool unico que escala
// com Taming: 1 slot a cada 20 pontos, com piso de 1 para que todo mundo
// tenha ao menos uma montaria/pet utilitario (Taming 0-19 -> 1, 100 -> 5).
function GetMaxNonWarPets(who)
	var slots := CInt( EffectiveSkill(who, "Taming") / 20 );
	if (slots < 1)
		slots := 1;
	endif
	return slots;
endfunction

// Agregado mantido por compatibilidade: paperdoll/followers/recall leem este
// numero e continuam coerentes sem precisar conhecer a divisao por categoria.
function GetMaxPets(who)
	return GetMaxWarPets(who) + GetMaxNonWarPets(who);
endfunction

// Portao unico de aquisicao (tame/transfer/resurrect). Grandfather: so
// bloqueia o NOVO pet; pets legados acima do cap nunca sao removidos aqui.
function CanAcquirePet(who, pet)
	var tally := _TallyPets(who);
	var cost := GetPetCost(pet);
	if (PetSlotCategory(pet) == "war")
		return (tally.war + cost) <= GetMaxWarPets(who);
	endif
	return (tally.nonwar + cost) <= GetMaxNonWarPets(who);
endfunction

function GetMaxSummons(who)
	var maxpets := 1;

	//Influência da skill na quantidade
	var skill := cint(AP_GetSkill(who, THEOLOGY));
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
			MoveObjectToLocation(pet, 5688, 625, 0, "britannia_alt", MOVEOBJECT_FORCELOCATION);
			SetObjProperty(pet, "previous_location", location);
		endif
		sleepms(2);
	endforeach


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
	EraseObjProperty(me, "Cruzar");
	RestartScript(me);
	ErasePetMods(me);
endfunction

function isTamable(mob)

	if (!mob.npctemplate)
		return 0;
	endif

	if (mob.isAnimalBrain())
		return 1;
	endif

	if ( !mob.getTameDifficulty() )
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
	ApplyEpicHealing(pet, 255);

	var npctemplate := NPC_ParseTemplateName(pet.npctemplate);
	var cfg := ReadConfigFile(":*:npcdesc");
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
	var cfg := ReadConfigFile(":*:npcdesc");
	var phys := cint(cfg[npctemplate.template].PhysicalResist);
	if (phys > 35)
		phys := 35;
	endif
	if (pet.resist_physical_mod > 0)
		pet.resist_physical_mod := pet.resist_physical_mod - phys;
	endif

endfunction

function RessuscitarPet(corpse, owner, is_bandage := 0)
    var npc := CreateNpcFromTemplate(GetObjProperty(corpse, "npctemplate"), corpse.x, corpse.y, corpse.z, 0, corpse.realm);
    CopyPropsPet(corpse, npc);
    var corpse_parsed := corpse.name[13, len(corpse.name)-12];
    npc.script := ":epicai:tamedBrain";
    RestartScript(npc);
    SetName(npc, corpse_parsed);
    RecalcVitals(npc);

    // Lógica de contagem de animais fidelizados
    var pets := GetObjProperty(owner, "TamedPets");
    var habs := MobileWrapper(owner).HasAbility(ABILITY_ID::ESPREITA_SELVAGEM);
    var limite_animais := habs ? 2 : 1;
    var animais_fidelizados := 0;
    foreach serial in pets
        var pet := SystemFindObjectBySerial(serial);
        if (GetObjProperty(pet, "bond") == "fiel")
            animais_fidelizados := animais_fidelizados + 1;
            if (animais_fidelizados > 2)
                break;
            endif
        endif
    endforeach

	//Checagem de limite de slots de animais
    if (!owner)
        destama(npc.serial);
    else
        if (!CanAcquirePet(owner, npc))
            SendSysMessageEx(owner, "Voce ja tem muitos animais.", SSM_FAIL);
            destama(npc.serial);
        elseif ( GetObjProperty(npc, "bond") == "fiel" && animais_fidelizados >= limite_animais)
            SendSysMessageEx(owner, "O animal sente-se traído por você ter atingido o limite de animais fieis.", SSM_FAIL);
            destama(npc.serial);
        else
            AddPet(owner, npc.serial);
        endif
    endif

    DestroyItem(corpse);
    SetCooldown(npc, "resurrect", is_bandage ? 120 : 180);
    var skills := GetObjProperty(npc, "petskills");
    foreach skill in (skills.keys())
        AP_SetTrueSkill(npc, skill, skills[skill]);
    endforeach

    // Desconta o DP dos fieis
    if (GetObjProperty(npc, "bond") == "fiel")
        var DP := AP_GetVital(npc, "DP");
        AP_SetVital(npc, "DP", DP - 10);
        if (DP < 0)
            AP_SetVital(npc, "DP", 0);
        endif
    endif

    // Repõe as habilidades do despertar natureza do pet
    var instincts := GetObjProperty(npc, "instincts");
    if (instincts)
        if (instincts["Ferocidade"].ins_name == "Ferocidade")
            AP_SetTrueStat(npc, STRENGTH, AP_GetStat(npc, STRENGTH) + 15);
        endif
        if (instincts["Fortaleza"].ins_name == "Fortaleza")
            var hits := CInt(AP_GetVitalMaximumValue(npc, "Hits"));
            var variavel := CInt(hits / 3);
            var variavel2 := CInt(variavel / 2);
            SetObjProperty(npc, "hitsmod", variavel2);
        endif
        if (instincts["Flexibilidade"].ins_name == "Flexibilidade")
            AP_SetTrueStat(npc, DEXTERITY, AP_GetStat(npc, DEXTERITY) + 15);
        endif
        RecalcVitals(npc);
    endif
	
	//Checar a felicidade do animal quando ressuscita
			var happy := GetObjProperty(npc,"happiness"); //happiness do pet
			var fiel := GetObjProperty(npc, "NeverRelease"); // se é animal fiel
			if (happy)
				SetObjProperty(npc,"happiness", happy -20);
				if(happy < 10 )
					if (!fiel) //destama se happiness menor que 10
						SendSysMessage(owner, "*Seu pet "+npc.name+" decidiu que eh melhor ficar sozinho*");
						destama(npc.serial);
//					else // se é fiel só perde fidelidade
//						SendSysMessage(owner, "*Seu pet "+npc.name+" nao sente mais tanta confiança em você**");
//						EraseObjProperty(npc, "NeverRelease");
//						EraseObjProperty(npc, "bond");
					endif
				endif
			endif

    return npc;
endfunction

function CopyPropsPet(origin, destiny)
    var propnames := GetObjPropertyNames(origin);
    foreach prop in propnames
        SetObjProperty(destiny, prop, GetObjProperty(origin, prop));
    endforeach
endfunction

function GetDietByTaxonomy(tax)

    tax := Lower(tax);

    if (tax == "canidae")
        return {"meat"};
    elseif (tax == "felidae")
        return {"meat", "fish"};
    elseif (tax == "arachnida")
        return {"meat"};
    elseif (tax == "serpentes")
        return {"meat"};

    elseif (tax == "delphinidae")
        return {"fish"};
    elseif (tax == "odobenidae")
        return {"fish"};
    elseif (tax == "crustacean")
        return {"fish"};

    elseif (tax == "reptilia")
        return {"meat", "fish"};
    elseif (tax == "draco")
        return {"meat", "fish"};

    elseif (tax == "ursidae")
        return {"meat", "fish", "fruit"};
    elseif (tax == "suidae")
        return {"meat", "fruit", "vegetable"};
    elseif (tax == "chiroptera")
        return {"fruit", "meat"};
    elseif (tax == "primatomorpha")
        return {"fruit", "grain", "vegetable"};

    elseif (tax == "bovidae")
        return {"grain", "vegetable"};
    elseif (tax == "equus")
        return {"grain", "vegetable"};
    elseif (tax == "camelidae")
        return {"grain", "vegetable"};
    elseif (tax == "cervidae")
        return {"grain", "vegetable"};
    elseif (tax == "rodentia")
        return {"grain", "vegetable"};

    elseif (tax == "aves")
        return {"grain", "fish"};
    elseif (tax == "phasianidae")
        return {"grain"};
    endif

    return {"all"};
endfunction