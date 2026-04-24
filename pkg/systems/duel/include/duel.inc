use uo;
use os;

include ":attributes:attributes";
include ":gumps:yesNo";
include ":message:model/Message";
include "include/say";
include "include/client";

const DUEL_TIMEOUT := 120; // 2 minutes
const DUEL_MAX_DISTANCE := 20;
const DUEL_STALE_TIMEOUT := 180; // 3 minutes, cleanup threshold
const DUEL_RECENT_NPC_HIT := 30; // seconds

function ChallengeDuel(who, skill_name)

	if (IsInDuel(who))
		Message("Voce ja esta em um duelo.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		return 0;
	endif

	if (who.warmode)
		Message("Voce nao pode iniciar um duelo em modo de guerra.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		return 0;
	endif

	Message("Selecione um jogador para desafiar para um duelo.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
	var targ := Target(who);

	if (!targ)
		Message("Cancelado.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		return 0;
	elseif (!targ.IsA(POLCLASS_MOBILE))
		Message("Alvo invalido.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		return 0;
	elseif (targ.npctemplate)
		Message("Voce so pode desafiar jogadores.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		return 0;
	elseif (targ == who)
		Message("Voce nao pode desafiar a si mesmo.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		return 0;
	elseif (!targ.connected)
		Message("O jogador esta desconectado.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		return 0;
	elseif (targ.warmode)
		Message("O alvo esta em modo de guerra.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		return 0;
	endif

	var max_range := 10;
	if (skill_name == "Ranged")
		max_range := 15;
	endif
	if (Distance(who, targ) > max_range)
		Message("O alvo esta muito longe.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		return 0;
	endif

	if (!ValidateDuelConditions(who, targ))
		return 0;
	endif

	Message($"Voce convidou {targ.name} para um duelo.", Color(SYSTEM_COLOR::SUCCESS)).SendSys(who);
	var perm := YesNo(targ, $"Voce aceita duelar {who.name}?", "Sim", "Nao");

	if (!perm)
		Message(targ.name + " recusou o duelo.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
		return 0;
	endif

	// Re-validate after YesNo wait
	if (!ValidateDuelConditions(who, targ))
		return 0;
	endif

	StartDuel(who, targ, skill_name);
	return 1;

endfunction

function ChallengeDuelWithTarget(who, targ, skill_name)

	if (!ValidateDuelConditions(who, targ))
		return 0;
	endif

	Message($"Voce convidou {targ.name} para um duelo.", Color(SYSTEM_COLOR::SUCCESS)).SendSys(who);
	var perm := YesNo(targ, $"Voce aceita duelar {who.name}?", "Sim", "Nao");

	if (!perm)
		Message(targ.name + " recusou o duelo.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
		return 0;
	endif

	if (!ValidateDuelConditions(who, targ))
		return 0;
	endif

	StartDuel(who, targ, skill_name);
	return 1;

endfunction

function ValidateDuelConditions(who, targ)

	if (IsInDuel(who))
		Message("Voce ja esta em um duelo.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		return 0;
	endif
	if (IsInDuel(targ))
		Message("O alvo ja esta em um duelo.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		return 0;
	endif

	var hostiles_who := ListHostiles(who, 10);
	if (hostiles_who[1])
		Message("Voce nao pode fazer isso enquanto esta em combate.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		return 0;
	endif

	var hostiles_targ := ListHostiles(targ, 10);
	if (hostiles_targ[1])
		Message("O alvo esta em combate.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		return 0;
	endif

	if (HasRecentNpcHit(who))
		Message("Voce foi atacado por um NPC recentemente. Aguarde.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		return 0;
	endif
	if (HasRecentNpcHit(targ))
		Message("O alvo foi atacado por um NPC recentemente.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		return 0;
	endif

	return 1;

endfunction

function HasRecentNpcHit(who)

	var lastHit := GetObjProperty(who, "lastHit");
	if (lastHit == error || !lastHit["time"])
		return 0;
	endif

	if ((polcore().systime - lastHit["time"]) > DUEL_RECENT_NPC_HIT)
		return 0;
	endif

	var attacker := SystemFindObjectBySerial(lastHit["attacker"]);
	if (!attacker || attacker.acctname)
		return 0; // no attacker found or attacker is a player
	endif

	return 1; // attacker is NPC and hit was recent

endfunction

function StartDuel(who, targ, skill_name)

	var duel_who := dictionary;
	duel_who["partner"] := targ.serial;
	duel_who["start_time"] := polcore().systime;
	duel_who["skill"] := skill_name;
	SetObjProperty(who, "#duel", duel_who);

	var duel_targ := dictionary;
	duel_targ["partner"] := who.serial;
	duel_targ["start_time"] := polcore().systime;
	duel_targ["skill"] := skill_name;
	SetObjProperty(targ, "#duel", duel_targ);

	who.SetWarMode(1);
	targ.SetWarMode(1);

	Message($"Duelo iniciado com {targ.name}!", Color(SYSTEM_COLOR::SUCCESS)).SendSys(who);
	Message($"Duelo iniciado com {who.name}!", Color(SYSTEM_COLOR::SUCCESS)).SendSys(targ);

	PrintTextAbovePrivate(who, $"*Duelando {targ.name}*", who);
	PrintTextAbovePrivate(who, $"*Duelando {targ.name}*", targ);
	PrintTextAbovePrivate(targ, $"*Duelando {who.name}*", targ);
	PrintTextAbovePrivate(targ, $"*Duelando {who.name}*", who);

	Start_Script(":duel:duelMonitor", {who, targ});

endfunction

function EndDuel(who, targ, reason)

	// On faint, keep #duel on dead player so death script can read it
	if (reason == "faint")
		if (who && !who.dead)
			EraseObjProperty(who, "#duel");
		endif
		if (targ && !targ.dead)
			EraseObjProperty(targ, "#duel");
		endif
	else
		EraseObjProperty(who, "#duel");
		if (targ)
			EraseObjProperty(targ, "#duel");
		endif
	endif

	var msg;
	case (reason)
		"faint":
			msg := "O duelo acabou! Um dos duelistas foi derrotado.";
		"distance":
			msg := "O duelo foi cancelado. Os duelistas se afastaram demais.";
		"timeout":
			msg := "O duelo acabou por tempo.";
		"external_damage":
			msg := "O duelo foi cancelado. Dano externo detectado.";
		default:
			msg := "O duelo foi cancelado.";
	endcase

	if (who && who.connected)
		Message(msg, Color(SYSTEM_COLOR::INFO)).SendSys(who);
	endif
	if (targ && targ.connected)
		Message(msg, Color(SYSTEM_COLOR::INFO)).SendSys(targ);
	endif

endfunction

function IsInDuel(who)

	var duel := GetObjProperty(who, "#duel");
	if (duel == error)
		return 0;
	endif

	// Staleness check
	if ((polcore().systime - duel["start_time"]) > DUEL_STALE_TIMEOUT)
		EraseObjProperty(who, "#duel");
		return 0;
	endif

	return 1;

endfunction

function IsDuelPartner(who, other)

	var duel := GetObjProperty(who, "#duel");
	if (duel == error)
		return 0;
	endif

	return (duel["partner"] == other.serial);

endfunction

function GetDuelPartner(who)

	var duel := GetObjProperty(who, "#duel");
	if (duel == error)
		return 0;
	endif

	return SystemFindObjectBySerial(duel["partner"]);

endfunction
