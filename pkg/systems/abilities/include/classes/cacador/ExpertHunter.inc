use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":traps:traps";
include ":timedScripts:timedScripts";

/**
 * Caçador Experiente (Expert Hunter) - ABILITY_ID::CACADOR_EXPERIENTE (101)
 *
 * OnUse (player): Install traps (net, container, floor).
 * OnUse (NPC):    Throw net at nearby hostile, paralyze 6s.
 * Passive effects (handled inline elsewhere):
 *   - Tracking: -30 difficulty, +15 range (survival.src)
 *   - Can craft throwable nets from ropes (corda.src)
 *   - Can craft traps (montartrap.src)
 */
class ExpertHunter(CombatAbility)

	function ExpertHunter(this)
		Super(ABILITY_ID::CACADOR_EXPERIENTE);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function CanActivate(this, mobile)
		var who := mobile.GetObjRef();

		if (GetEquipmentByLayer(who, 0x19))
			Message("Você não consegue fazer isso montado.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif

		return true;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();
		SetScriptController(who);

		if (mobile.IsNPC())
			return _NpcThrowNet(who);
		endif

		Message("Qual armadilha deseja instalar?", Color(SYSTEM_COLOR::INFO)).SendSys(who);
		var trap := Target(who);

		if (!trap)
			Message("Cancelado.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif

		if ((trap.objtype != 0x6076) && (trap.objtype != 0x4140) && (trap.objtype != 0x1BE6))
			Message("Isto não é uma armadilha.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif

		if ((!trap.movable) || !ReserveItem(trap))
			Message("Esta armadilha já está instalada.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif

		// Bear trap (0x4140)
		if (trap.objtype == 0x4140)
			return _InstallNetTrap(who, trap);
		endif

		// Other traps (container or floor)
		return _InstallOtherTrap(who, trap);
	endfunction

endclass

function _InstallNetTrap(who, trap)
	Message("Onde deseja instalar a armadilha?", Color(SYSTEM_COLOR::INFO)).SendSys(who);
	var targetted := TargetCoordinates(who);

	if (!targetted)
		Message("Cancelado.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		return false;
	endif

	if (Distance(targetted, who) > 2)
		Message("O alvo está muito distante.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		return false;
	endif

	if (targetted.IsA(POLCLASS_MOBILE) || targetted.npctemplate)
		Message("Você não pode colocar uma armadilha nisso.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		return false;
	endif

	if (targetted.isa(POLCLASS_CORPSE))
		Message("Você não pode colocar armadilhas em corpos.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		return false;
	endif

	if (targetted == who)
		Message("Você acha que talvez isso não seja uma boa ideia.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		return false;
	endif

	Message("*instalando armadilha*").SendAbove(who);
	DestroyItem(trap);
	sleep(4);
	var armadilhanochao := CreateItemAtLocation(who.x, who.y, who.z - 1, 0x4004, 1, who.realm);
	Message("Terminou de instalar a armadilha.", Color(SYSTEM_COLOR::SUCCESS)).SendSys(who);
	sleep(5);
	armadilhanochao.invisible := 1;
	return true;
endfunction

function _InstallOtherTrap(who, trap)
	Message("Escolha onde quer instalar a armadilha.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
	var alvo := Target(who);
	var traptype := GetObjProperty(trap, "traptype");
	var level := GetObjProperty(trap, "traplevel");

	if (!alvo)
		Message("Cancelado.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		return false;
	endif

	if (Distance(alvo, who) > 2)
		Message("O alvo está muito distante.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		return false;
	endif

	if (alvo.IsA(POLCLASS_MOBILE) || alvo.npctemplate)
		Message("Você não pode colocar uma armadilha nisso.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		return false;
	endif

	if (alvo.isa(POLCLASS_CORPSE))
		Message("Você não pode colocar armadilhas em corpos.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		return false;
	endif

	if (alvo == who)
		Message("Você acha que talvez isso não seja uma boa ideia.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		return false;
	endif

	// Container trap
	if (alvo.isA(POLCLASS_CONTAINER))
		if (!level)
			Message("Esta armadilha está quebrada ou desarmada.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif

		Message("*instalando armadilha*").SendAbove(who);
		sleep(4);
		Message("Você instalou a armadilha.", Color(SYSTEM_COLOR::SUCCESS)).SendSys(who);
		AddTrap(alvo, traptype, level);
		DestroyItem(trap);
		return true;
	else
		// Floor trap
		Message("Deseja instalar a armadilha no chão?", Color(SYSTEM_COLOR::INFO)).SendSys(who);
		var targetted := TargetCoordinates(who);

		if (!targetted)
			Message("Cancelado.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif

		if (Distance(targetted, who) > 2)
			Message("O alvo está muito distante.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif

		Message("*instalando armadilha*").SendAbove(who);
		DestroyItem(trap);
		sleep(4);
		var armadilhanochao := CreateItemAtLocation(who.x, who.y, who.z - 1, 0x367a, 1, who.realm);
		SetObjProperty(armadilhanochao, "traplevel", level);
		SetObjProperty(armadilhanochao, "traptype", traptype);
		Message("Terminou de instalar a armadilha.", Color(SYSTEM_COLOR::SUCCESS)).SendSys(who);
		sleep(2);
		armadilhanochao.invisible := 1;
		return true;
	endif
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