use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":tn:cooldown";
include ":attributes:attributes";
include ":timedScripts:timedScripts";
include "include/eventID";
include "include/utils";
include "include/say";
include "include/epicNPCUtils";

/**
 * Comando de Feras - ABILITY_ID::COMANDO_DE_FERAS (101)
 *
 * Active. Opens a battle-command menu for the player's war pets.
 * Number of commands available scales with Beastmastery thresholds
 * {10,30,50,70,90} -> 1..5. Stamina/cooldown auto-applied by
 * AbilityService from habilidades.cfg (delay 120 / stam 20).
 *
 *   1 Atacar!   - all nearby pets gain +Beastmastery/10 hitchance for a window
 *   2 Proteger! - chosen pet taunts everything currently attacking the owner
 *   3 Emboscar! - chosen pet leaps a target and paralyzes it Beastmastery/10 s
 *   4 Recuar!   - chosen pet breaks off and is pushed back to safety
 *   5 Farejar!  - chosen pet reveals hidden hostiles around the owner
 */
class BeastCommand(CombatAbility)

	function BeastCommand(this)
		Super(ABILITY_ID::COMANDO_DE_FERAS);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function CanActivate(this, mobile)
		var who := mobile.GetObjRef();

		if (mobile.IsNPC())
			return true;
		endif

		if (CInt(AP_GetSkill(who, "Beastmastery")) < 10)
			Message("Seu domínio sobre as feras ainda é insuficiente.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif

		if (!_GetPets(who))
			Message("Você não tem feras de guerra por perto.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif

		return true;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();

		if (mobile.IsNPC())
			return _NpcCommand(who);
		endif

		var bm := CInt(AP_GetSkill(who, "Beastmastery"));
		var unlocked := _CmdCount(bm);

		Message("Comando de Feras:", Color(SYSTEM_COLOR::INFO)).SendSys(who);
		Message("1 - Atacar!", Color(SYSTEM_COLOR::INFO)).SendSys(who);
		if (unlocked >= 2)
			Message("2 - Proteger!", Color(SYSTEM_COLOR::INFO)).SendSys(who);
		endif
		if (unlocked >= 3)
			Message("3 - Emboscar!", Color(SYSTEM_COLOR::INFO)).SendSys(who);
		endif
		if (unlocked >= 4)
			Message("4 - Recuar!", Color(SYSTEM_COLOR::INFO)).SendSys(who);
		endif
		if (unlocked >= 5)
			Message("5 - Farejar!", Color(SYSTEM_COLOR::INFO)).SendSys(who);
		endif

		var input := CInt(RequestInput(who, who.backpack, "Comando (1-" + unlocked + "):"));
		if (input < 1 || input > unlocked)
			Message("Comando inválido.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif

		if (input == 1)
			return _Atacar(who, bm);
		endif

		var pet := _PickPet(who);
		if (!pet)
			Message("Nenhuma fera selecionada.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif

		case (input)
			2: return _Proteger(who, pet, bm);
			3: return _Emboscar(who, pet, bm);
			4: return _Recuar(pet);
			5: return _Farejar(who, pet, bm);
		endcase

		return false;
	endfunction

endclass

function _CmdCount(bm)
	var n := 0;
	if (bm >= 10)
		n := 1;
	endif
	if (bm >= 30)
		n := 2;
	endif
	if (bm >= 50)
		n := 3;
	endif
	if (bm >= 70)
		n := 4;
	endif
	if (bm >= 90)
		n := 5;
	endif
	return n;
endfunction

function _GetPets(who)
	var pets := array{};
	var tamed := GetObjProperty(who, "TamedPets");
	if (!tamed)
		return pets;
	endif

	foreach serial in tamed
		var pet := SystemFindObjectBySerial(serial);
		if (pet && pet.master == who && !pet.dead && !GetObjProperty(pet, "summoned") && Distance(who, pet) <= 12)
			pets.append(pet);
		endif
	endforeach

	return pets;
endfunction

function _PickPet(who)
	var pets := _GetPets(who);
	if (!pets)
		return 0;
	endif
	if (pets.size() == 1)
		return pets[1];
	endif

	Message("Selecione a fera.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
	var picked := Target(who);
	if (!picked)
		return 0;
	endif

	foreach pet in pets
		if (pet.serial == picked.serial)
			return pet;
		endif
	endforeach

	Message("Esta não é uma de suas feras.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
	return 0;
endfunction

function _Atacar(who, bm)
	var val := CInt(bm / 10);
	if (val < 1)
		val := 1;
	endif

	var pets := _GetPets(who);
	if (!pets)
		Message("Nenhuma fera por perto.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		return false;
	endif

	var serials := array{};
	foreach pet in pets
		pet.hitchance_mod := pet.hitchance_mod + val;
		serials.append(pet.serial);
		Message("*sente o instinto de caça*").SendAbove(pet);
	endforeach
	Message("Suas feras avançam para o ataque!", Color(SYSTEM_COLOR::INFO)).SendSys(who);

	var dur := CInt(bm / 5);
	if (dur < 1)
		dur := 1;
	endif
	sleep(dur);

	foreach serial in serials
		var pet := SystemFindObjectBySerial(serial);
		if (pet)
			pet.hitchance_mod := pet.hitchance_mod - val;
		endif
	endforeach

	return true;
endfunction

function _Proteger(who, pet, bm)
	var dur := CInt(bm / 2);
	if (dur < 1)
		dur := 1;
	endif

	var hostiles := ListHostiles(who, 8);
	var count := 0;
	foreach enemy in hostiles
		if (enemy.opponent != who)
			continue;
		endif
		SendEvent(enemy, struct{ "type" := EVID_TAUNT, "source" := pet });
		SetCooldown(enemy, "taunted", dur);
		MobileWrapper(enemy).SetProperty("#taunter", pet.serial);
		count := count + 1;
	endforeach

	if (!count)
		Message("Nenhum inimigo está atacando você.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		return false;
	endif

	Message("*ruge e atrai a fúria dos inimigos*").SendAbove(pet);
	return true;
endfunction

function _Emboscar(who, pet, bm)
	Message("Selecione o alvo.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
	var foe := Target(who, TGTOPT_CHECK_LOS);
	if (!foe || !foe.isa(POLCLASS_MOBILE) || foe == who || foe == pet)
		Message("Alvo inválido.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		return false;
	endif

	pet.opponent := foe;
	Message("*salta sobre " + foe.name + "*").SendAbove(pet);
	PlaySoundEffect(pet, 0x51C);
	TS_StartTimer(foe, "paralysis", CInt(bm / 10));
	return true;
endfunction

function _Recuar(pet)
	PrintTextAbove(pet, "*fuga*");
	pet.hidden := 1;
	sleepms(500);
	PushBack(pet, 4, 75, 0);
	pet.hidden := 0;
	pet.opponent := 0;
	return true;
endfunction

function _Farejar(who, pet, bm)
	var rng := CInt(bm / 10);
	if (rng < 4)
		rng := 4;
	endif

	var hostiles := ListHostiles(who, rng);
	var count := 0;
	foreach foe in hostiles
		if (foe.hidden)
			foe.hidden := 0;
			Message("*farejado*").SendAbove(foe);
			count := count + 1;
		endif
	endforeach

	Message("*fareja o ar em busca de presas escondidas*").SendAbove(pet);
	return true;
endfunction

// NPC path: an NPC beastmaster throws a net at its target and sics its
// own pets onto it (net + pet command in one).
function _NpcCommand(who)
	var foe := who.opponent;
	if (!foe)
		foe := GetRandomNearbyHostile(who);
	endif

	_NpcThrowNet(who);

	if (foe)
		var nearby := ListMobilesNearLocation(who.x, who.y, who.z, 12, who.realm);
		foreach creature in nearby
			if (creature.master == who && !creature.dead)
				creature.opponent := foe;
			endif
		endforeach
	endif

	return true;
endfunction

function _NpcThrowNet(who)
	var alvo := who.opponent;
	if (!alvo)
		alvo := GetRandomNearbyHostile(who);
	endif
	if (!alvo || alvo == who)
		return false;
	endif
	if (Distance(who, alvo) > 6)
		return false;
	endif
	if (!CheckLosAt(who, alvo.x, alvo.y, alvo.z))
		return false;
	endif
	if (GetObjProperty(alvo, "corda"))
		return false;
	endif

	Message("*arremessa uma rede em " + alvo.name + "*").SendAbove(who);
	PlaySoundEffect(who, 0x51C);
	PlayMovingEffect(who, alvo, 0x1EA3, 10);

	var preso := CreateItemAtLocation(alvo.x, alvo.y, alvo.z + 1, 0x1EA4, 1, alvo.realm);
	SetObjProperty(alvo, "corda", 1);
	TS_StartTimer(alvo, "paralysis", 6);
	sleep(6);
	if (preso)
		DestroyItem(preso);
	endif
	EraseObjProperty(alvo, "corda");
	return true;
endfunction
