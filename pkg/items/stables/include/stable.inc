use uo;
use os;
use math;

const SEGUNDOS_POR_DIA := 86400;

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

function IsStableExpired(pet)
	var dataExp := CInt(GetObjProperty(pet, "DataExpiracao"));
	if (!dataExp)
		var dataEnt := CInt(GetObjProperty(pet, "DataEntrada"));
		if (dataEnt)
			dataExp := dataEnt + SEGUNDOS_POR_DIA;
		else
			return 1;
		endif
	endif
	return polcore().systime >= dataExp;
endfunction

function FormatTempoRestante(pet)
	var dataExp := CInt(GetObjProperty(pet, "DataExpiracao"));
	if (!dataExp)
		var dataEnt := CInt(GetObjProperty(pet, "DataEntrada"));
		if (dataEnt)
			dataExp := dataEnt + SEGUNDOS_POR_DIA;
		endif
	endif

	var restante := dataExp - polcore().systime;
	if (restante <= 0)
		return "expirado";
	endif

	var dias := CInt(restante / 86400);
	var horas := CInt((restante % 86400) / 3600);

	return dias + "d " + horas + "h";
endfunction

// Tira o pet do estado fisico "no estabulo" (libera paralisia/invul, solta o
// slot do post). NAO encerra o contrato — StablePostSerial/DataExpiracao/etc
// continuam validos ate o contrato expirar naturalmente, permitindo que o
// player use .assoviar com o pet.
function RetirarPetFisicamente(pet, post)
	pet.setParalyzed(0);
	RevokePrivilege(pet, "invul");
	pet.disable("invul");

	EraseObjProperty(pet, "EstaNoEstabulo");

	if (post)
		EraseObjProperty(post, "StoredAnimal");
	endif
endfunction

// Coloca o pet no estado fisico "no estabulo": paraliza, concede invul, marca
// EstaNoEstabulo e vincula o slot do post. NAO mexe nas properties de contrato
// (StablePostSerial/DataExpiracao/...). Serve tanto para guarda inicial quanto
// para re-estacionar durante um contrato ativo.
function ParalisarPetNoEstabulo(pet, post)
	pet.hidden := 0;
	pet.setParalyzed(1);
	GrantPrivilege(pet, "invul");
	pet.enable("invul");

	SetObjProperty(pet, "EstaNoEstabulo", 1);
	SetObjProperty(post, "StoredAnimal", pet.serial);
endfunction

// Encerra o contrato totalmente: remove todas as properties do aluguel e libera
// o CurrentStablePostSerial do master para que ele possa guardar em outro post.
// Chamar quando o contrato expira naturalmente ou em cleanup anomalo. NAO chama
// GetNumPets — deixa essa responsabilidade para o caller (evita recursao em
// taming.inc:GetNumPets).
function EncerrarContratoEstabulo(pet, post, owner)
	RetirarPetFisicamente(pet, post);

	EraseObjProperty(pet, "StablePostSerial");
	EraseObjProperty(pet, "DataExpiracao");
	EraseObjProperty(pet, "DataEntrada");
	EraseObjProperty(pet, "PrecoPorDia");

	if (owner)
		EraseObjProperty(owner, "CurrentStablePostSerial");
	endif
endfunction
