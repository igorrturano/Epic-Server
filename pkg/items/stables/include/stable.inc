use uo;
use os;
use math;

const SEGUNDOS_POR_DIA := 86400;

const STABLE_STATE_ACTIVE  := "active";
const STABLE_STATE_STABLED := "stabled";
const STABLE_STATE_LIMBO   := "limbo";

// -----------------------------------------------------------------------------
// Pet StableData
// -----------------------------------------------------------------------------
// Dicionario em GetObjProperty(pet, "StableData") com o formato:
//   post           integer  (serial do post que guarda o contrato)
//   expires_at     integer  (systime em que o contrato vence)
//   rental_days    integer  (duracao original para default de renovacao)
//   price_per_day  integer  (custo diario para calculo de renovacao)
//   state          string   ("active" | "stabled" | "limbo")
//   limbo_origin   struct   (opcional — so existe enquanto state = "limbo")
//
// Ausencia do dicionario == pet sem contrato. state = "active" == contrato
// vigente com pet no mundo; "stabled" == pet guardado no post; "limbo" ==
// enviado via .assoviar.

function NewPetStableData(post_serial, days, price_per_day)
	var data := dictionary;
	data["post"] := CInt(post_serial);
	data["expires_at"] := polcore().systime + CInt(days) * SEGUNDOS_POR_DIA;
	data["rental_days"] := CInt(days);
	data["price_per_day"] := CInt(price_per_day);
	data["state"] := STABLE_STATE_STABLED;
	return data;
endfunction

function GetPetStable(pet)
	return GetObjProperty(pet, "StableData");
endfunction

function SetPetStable(pet, data)
	SetObjProperty(pet, "StableData", data);
endfunction

function ClearPetStable(pet)
	EraseObjProperty(pet, "StableData");
endfunction

function HasContract(pet)
	var data := GetPetStable(pet);
	if (data == error)
		return 0;
	endif
	return CInt(data["post"]) != 0;
endfunction

function IsStabled(pet)
	var data := GetPetStable(pet);
	if (data == error)
		return 0;
	endif
	return data["state"] == STABLE_STATE_STABLED;
endfunction

function IsInLimbo(pet)
	var data := GetPetStable(pet);
	if (data == error)
		return 0;
	endif
	return data["state"] == STABLE_STATE_LIMBO;
endfunction

function IsContractExpired(pet)
	var data := GetPetStable(pet);
	if (data == error || !CInt(data["expires_at"]))
		return 1;
	endif
	return polcore().systime >= CInt(data["expires_at"]);
endfunction

function FormatContractTimeLeft(pet)
	var data := GetPetStable(pet);
	if (data == error)
		return "expirado";
	endif
	var restante := CInt(data["expires_at"]) - polcore().systime;
	if (restante <= 0)
		return "expirado";
	endif
	var dias := CInt(restante / SEGUNDOS_POR_DIA);
	var horas := CInt((restante % SEGUNDOS_POR_DIA) / 3600);
	return dias + "d " + horas + "h";
endfunction

// -----------------------------------------------------------------------------
// Owner StableData
// -----------------------------------------------------------------------------
// Dicionario em GetObjProperty(owner, "StableData") com o formato:
//   post           integer  (serial do post contratado — zero se sem contrato)
//   stabled_pets   array    (seriais dos pets guardados agora)
//   limbo_pet      integer  (serial do pet em limbo ou 0)
//   last_whistle   integer  (systime do ultimo .assoviar para cooldown)

function NewOwnerStableData()
	var data := dictionary;
	data["post"] := 0;
	data["stabled_pets"] := array;
	data["limbo_pet"] := 0;
	data["last_whistle"] := 0;
	return data;
endfunction

function GetOwnerStable(owner)
	var data := GetObjProperty(owner, "StableData");
	if (data == error)
		return error;
	endif
	if (!data.Exists("stabled_pets") || data["stabled_pets"] == error)
		data["stabled_pets"] := array;
	endif
	return data;
endfunction

function SetOwnerStable(owner, data)
	SetObjProperty(owner, "StableData", data);
endfunction

function EnsureOwnerStable(owner)
	var data := GetOwnerStable(owner);
	if (data == error)
		data := NewOwnerStableData();
		SetOwnerStable(owner, data);
	endif
	return data;
endfunction

function OwnerHasStabledPet(owner)
	var data := GetOwnerStable(owner);
	if (data == error)
		return 0;
	endif
	return data["stabled_pets"].size() > 0;
endfunction

function OwnerLimboPetSerial(owner)
	var data := GetOwnerStable(owner);
	if (data == error)
		return 0;
	endif
	return CInt(data["limbo_pet"]);
endfunction

function OwnerPostSerial(owner)
	var data := GetOwnerStable(owner);
	if (data == error)
		return 0;
	endif
	return CInt(data["post"]);
endfunction

function OwnerLastWhistle(owner)
	var data := GetOwnerStable(owner);
	if (data == error)
		return 0;
	endif
	return CInt(data["last_whistle"]);
endfunction

function SetOwnerLastWhistle(owner, timestamp)
	var data := EnsureOwnerStable(owner);
	data["last_whistle"] := CInt(timestamp);
	SetOwnerStable(owner, data);
endfunction

// Remove qualquer referencia ao pet_serial do StableData do owner (stabled_pets
// e limbo_pet). Se o pet era o unico guardado/em limbo, zera tambem o post.
function OwnerClearPetRef(owner, pet_serial)
	if (!owner)
		return;
	endif
	var data := GetOwnerStable(owner);
	if (data == error)
		return;
	endif
	var dirty := 0;
	var stabled := data["stabled_pets"];
	foreach serial in stabled
		if (CInt(serial) == CInt(pet_serial))
			stabled.erase(_serial_iter);
			data["stabled_pets"] := stabled;
			dirty := 1;
			break;
		endif
		sleepms(2);
	endforeach
	if (CInt(data["limbo_pet"]) == CInt(pet_serial))
		data["limbo_pet"] := 0;
		dirty := 1;
	endif
	if (stabled.size() == 0 && !CInt(data["limbo_pet"]))
		data["post"] := 0;
		dirty := 1;
	endif
	if (dirty)
		SetOwnerStable(owner, data);
	endif
endfunction

function AddOwnerStabledPet(owner, pet_serial, post_serial)
	var data := EnsureOwnerStable(owner);
	data["post"] := CInt(post_serial);
	var stabled := data["stabled_pets"];
	var already := 0;
	foreach serial in stabled
		if (CInt(serial) == CInt(pet_serial))
			already := 1;
			break;
		endif
		sleepms(2);
	endforeach
	if (!already)
		stabled.append(CInt(pet_serial));
		data["stabled_pets"] := stabled;
	endif
	SetOwnerStable(owner, data);
endfunction

function RemoveOwnerStabledPet(owner, pet_serial)
	var data := GetOwnerStable(owner);
	if (data == error)
		return;
	endif
	var stabled := data["stabled_pets"];
	foreach serial in stabled
		if (CInt(serial) == CInt(pet_serial))
			stabled.erase(_serial_iter);
			data["stabled_pets"] := stabled;
			SetOwnerStable(owner, data);
			return;
		endif
		sleepms(2);
	endforeach
endfunction

function SetOwnerLimboPet(owner, pet_serial)
	var data := EnsureOwnerStable(owner);
	data["limbo_pet"] := CInt(pet_serial);
	SetOwnerStable(owner, data);
endfunction

// -----------------------------------------------------------------------------
// Transicoes de pet — API publica
// -----------------------------------------------------------------------------
// IMPORTANTE: estas funcoes cuidam APENAS do estado fisico (paralisar/invul),
// do StoredAnimal no post e do dict StableData do pet e do owner. As listas
// de ownership (TamedPets, followers) sao responsabilidade do caller, que
// chama ErasePet/AddPet em sequencia conforme o fluxo. Isto evita circular
// include entre stable.inc e taming.inc.

// Abre o contrato e coloca o pet no estado fisico de guardado.
// Caller (stablepost.src) ja pagou as moedas e deve chamar ErasePet(owner, pet.serial)
// depois para remover o pet de TamedPets e liberar slots.
function EnterStable(owner, pet, post, days, price_per_day)
	if (!owner || !pet || !post)
		return 0;
	endif
	pet.hidden := 0;
	pet.setParalyzed(1);
	GrantPrivilege(pet, "invul");
	pet.enable("invul");
	SetObjProperty(post, "StoredAnimal", pet.serial);

	SetPetStable(pet, NewPetStableData(post.serial, days, price_per_day));
	AddOwnerStabledPet(owner, pet.serial, post.serial);
	return 1;
endfunction

// Reestaciona um pet ja contratado no mesmo post (sem cobrar). O contrato
// continua com o mesmo expires_at. Caller tambem deve chamar ErasePet para
// liberar os slots (pet saiu de TamedPets e volta para stabled_pets).
function RestageStable(owner, pet, post)
	if (!owner || !pet || !post)
		return 0;
	endif
	pet.hidden := 0;
	pet.setParalyzed(1);
	GrantPrivilege(pet, "invul");
	pet.enable("invul");
	SetObjProperty(post, "StoredAnimal", pet.serial);

	var pet_data := GetPetStable(pet);
	if (pet_data == error)
		return 0;
	endif
	pet_data["state"] := STABLE_STATE_STABLED;
	pet_data.Erase("limbo_origin");
	SetPetStable(pet, pet_data);

	AddOwnerStabledPet(owner, pet.serial, post.serial);
	return 1;
endfunction

// Tira o pet do post: libera paralisia/invul e atualiza dicts. Contrato
// continua vigente (state volta para "active"). Caller deve chamar
// AddPet(owner, pet.serial) em seguida para registrar nos followers de novo,
// APOS ter validado GetMaxPets.
function ExitStable(owner, pet, post)
	if (!pet)
		return 0;
	endif
	pet.setParalyzed(0);
	RevokePrivilege(pet, "invul");
	pet.disable("invul");
	if (post)
		EraseObjProperty(post, "StoredAnimal");
	endif

	var pet_data := GetPetStable(pet);
	if (pet_data != error)
		pet_data["state"] := STABLE_STATE_ACTIVE;
		SetPetStable(pet, pet_data);
	endif

	if (owner)
		RemoveOwnerStabledPet(owner, pet.serial);
	endif
	return 1;
endfunction

// Contrato expirado + dono recusou renovar (ou falhou pagamento): libera
// o pet do estado fisico e limpa todas as refs. Caller deve chamar destama
// (pra zerar o master) e ErasePet (pra garantir que nao ficou em TamedPets).
function ReleaseAsWild(owner, pet, post)
	if (!pet)
		return 0;
	endif
	pet.setParalyzed(0);
	RevokePrivilege(pet, "invul");
	pet.disable("invul");
	if (post)
		EraseObjProperty(post, "StoredAnimal");
	endif

	ClearPetStable(pet);
	if (owner)
		OwnerClearPetRef(owner, pet.serial);
	endif
	return 1;
endfunction

// Contrato vigente: estende a duracao. Caller ja cobrou as moedas. Usado
// tanto pela renovacao automatica (vencido + aceitou) quanto por extensao
// voluntaria futura.
function RenewContract(pet, days, price_per_day)
	var data := GetPetStable(pet);
	if (data == error)
		return 0;
	endif
	data["expires_at"] := polcore().systime + CInt(days) * SEGUNDOS_POR_DIA;
	data["rental_days"] := CInt(days);
	data["price_per_day"] := CInt(price_per_day);
	data["state"] := STABLE_STATE_STABLED;
	SetPetStable(pet, data);
	return 1;
endfunction

// -----------------------------------------------------------------------------
// Limbo (.assoviar)
// -----------------------------------------------------------------------------

// Marca o pet como em limbo e salva a origem. Caller faz o movimento fisico
// (teleport + hidden + frozen + invul) via ai/sendpetlimbo.
function EnterLimbo(owner, pet)
	if (!owner || !pet)
		return 0;
	endif
	var pet_data := GetPetStable(pet);
	if (pet_data == error)
		return 0;
	endif
	pet_data["state"] := STABLE_STATE_LIMBO;
	pet_data["limbo_origin"] := struct{
		x := pet.x, y := pet.y, z := pet.z, realm := pet.realm
	};
	SetPetStable(pet, pet_data);
	SetOwnerLimboPet(owner, pet.serial);
	return 1;
endfunction

// Retorno do limbo: dict do pet volta para "active", owner.limbo_pet limpo.
// Caller faz o movimento fisico (teleport para perto do dono).
function ExitLimbo(owner, pet)
	if (!pet)
		return 0;
	endif
	var pet_data := GetPetStable(pet);
	if (pet_data != error)
		pet_data["state"] := STABLE_STATE_ACTIVE;
		pet_data.Erase("limbo_origin");
		SetPetStable(pet, pet_data);
	endif
	if (owner)
		var data := GetOwnerStable(owner);
		if (data != error && CInt(data["limbo_pet"]) == CInt(pet.serial))
			data["limbo_pet"] := 0;
			SetOwnerStable(owner, data);
		endif
	endif
	return 1;
endfunction

// -----------------------------------------------------------------------------
// Mundo — utilitarios
// -----------------------------------------------------------------------------

// Procura um tile caminhavel ao redor do dono usando CanWalk (valida pathing
// real — nao cai em casa, montanha ou agua). Tenta ate 16 amostras aleatorias
// num raio de 3 a 6 tiles. Fallback: o proprio tile do dono (sempre valido).
function FindSpawnLocationNearby(character)
	var attempts := 16;
	while (attempts > 0)
		attempts -= 1;
		var dist := RandomInt(4) + 3;
		var angle := RandomInt(360);
		var radian := angle * (ConstPi() / 180);
		var x := character.x + CInt(dist * Cos(radian));
		var y := character.y + CInt(dist * Sin(radian));
		var map_info := GetMapInfo(x, y, character.realm);
		if (!map_info)
			continue;
		endif
		if (CanWalk("L", character.x, character.y, character.z, x, y, character.realm) != error)
			return struct{ x := x, y := y, z := map_info.z };
		endif
	endwhile
	return struct{ x := character.x, y := character.y, z := character.z };
endfunction

