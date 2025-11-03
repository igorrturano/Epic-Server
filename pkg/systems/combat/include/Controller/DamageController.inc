/* $Id$
 *
 * Purpose
 * Handles damages and resistances to damage types
 *
 */

// Internal
include ":combat:armorZones";
include ":combat:Controller/AbilitiesController";
include ":combat:Controller/SkillController";
include ":combat:weardown";

// External
include ":tn:cooldown";
include ":attributes:attributes";
include ":blood:blood";
include ":fls_core:packets";
include "include/epicCombatUtils";
include "include/facings";
include ":timedScripts:timedScripts";
include ":debug:Controller/DebugController";
include ":equipsys:utils/consts";
include "include/epicNPCUtils";
include ":crafting:/include/caracteristicasFabricacao";

/*
 * http://uo.stratics.com/content/guides/resistance.shtml
 * These could be attributes, but not every shard will want to
 * follow OSI. This gives more freedom to make custom resistances
 * and remove existing ones more easily.
 */
enum DAMAGE_TYPES
	DMG_SLASH		:= "Slash",
	DMG_PIERCING	:= "Piercing",
	DMG_BLUNT		:= "Blunt",

	DMG_FIRE		:= "Fire",
	DMG_COLD		:= "Cold",
	DMG_POISON		:= "Poison",
	DMG_ENERGY		:= "Energy"
endenum

function CheckCombatDamage(byRef attackInfo)
	CalculateCombatDamage(attackInfo);
	CheckAbilitiesBeforeHit(attackInfo);
	ApplyCombatDamage(attackInfo);
	CheckPoison(attackInfo);
	CheckAbilitiesAfterHit(attackInfo);
	ProgressSkills(attackInfo);
endfunction

function CalculateCombatDamage(byRef attackInfo)
	CalculateAttackerDamage(attackInfo);
	CheckCriticalHit(attackInfo);
	CalculateDefenderResist(attackInfo);
endfunction

function CheckCriticalHit(byRef attackInfo)
endfunction

function ApplyCombatDamage(byRef attackInfo)
	var slash := attackInfo.GetDamage().GetSlashDamage(),
		piercing := attackInfo.GetDamage().GetPierceDamage(),
		blunt := attackInfo.GetDamage().GetBluntDamage(),
		fire := attackInfo.GetDamage().GetFireDamage(),
		cold := attackInfo.GetDamage().GetColdDamage(),
		poison := attackInfo.GetDamage().GetPoisonDamage(),
		energy := attackInfo.GetDamage().GetEnergyDamage();
	
	if (slash > 0)
		ApplyRawDamage(attackInfo.GetDefender().GetObjRef(), slash);
		DisplayDamage(attackInfo.GetDefender().GetObjRef(), slash, attackInfo.GetAttacker().GetObjRef());
	endif

	if (piercing > 0)
		ApplyRawDamage(attackInfo.GetDefender().GetObjRef(), piercing);
		DisplayDamage(attackInfo.GetDefender().GetObjRef(), piercing, attackInfo.GetAttacker().GetObjRef());
	endif

	if (blunt > 0)
		ApplyRawDamage(attackInfo.GetDefender().GetObjRef(), blunt);
		DisplayDamage(attackInfo.GetDefender().GetObjRef(), blunt, attackInfo.GetAttacker().GetObjRef());
	endif

	if (fire > 0)
		ApplyRawDamage(attackInfo.GetDefender().GetObjRef(), fire);
		DisplayDamage(attackInfo.GetDefender().GetObjRef(), fire, attackInfo.GetAttacker().GetObjRef());
	endif

	if (cold > 0)
		ApplyRawDamage(attackInfo.GetDefender().GetObjRef(), cold);
		DisplayDamage(attackInfo.GetDefender().GetObjRef(), cold, attackInfo.GetAttacker().GetObjRef());
	endif

	if (poison > 0)
		ApplyRawDamage(attackInfo.GetDefender().GetObjRef(), poison);
		DisplayDamage(attackInfo.GetDefender().GetObjRef(), poison, attackInfo.GetAttacker().GetObjRef());
	endif

	if (energy > 0)
		ApplyRawDamage(attackInfo.GetDefender().GetObjRef(), energy);
		DisplayDamage(attackInfo.GetDefender().GetObjRef(), energy, attackInfo.GetAttacker().GetObjRef());
	endif
endfunction

function CheckPoison(byRef attackInfo)
	// TODO: Tem várias melhorias que podem ser feitas aqui. Vou deixar pra depois,se der algum bug no combate com veneno. Por enquanto vai tomar muito tempo.
	// TODO: Sugestão de melhoria: Criar um CanPoison() e depois um TryToPoison(), separando a lógica de verificação da aplicação do veneno em si.
	if (attackInfo.GetDamage().GetTotalDamage() < 1)
		return false;
	endif

	if (attackInfo.GetDefender().HasAbility(COMBAT_ABILITY::SENSO_DA_NATUREZA))
		Message("Você é imune ao efeito dos venenos.", FONTCOLOR_SUCCESS).SendSys(attackInfo.GetDefender().GetObjRef());
		return false;
	endif

	if (attackInfo.GetAttacker().IsNPC()) // Se for NPC, checa pela skill dele
		var poisonchance := GetConfigInt(attackInfo.GetAttacker().GetNPCTemplateCfg(), "Poisoning");
		if (poisonChance > 0)
			if (RandomInt(100) < poisonChance)
				var poisonName := GetConfigString(attackInfo.GetAttacker().GetNPCTemplateCfg(), "PoisonType");
				var level := Cint(GetConfigInt(attackInfo.GetAttacker().GetNPCTemplateCfg(), "PoisonLevel"));

				if (!level)
					level := 1;
				endif

				if (poisonName == error)
					return false;
				endif

				var time := level * 15;
				if (poisonName == "paralyzePoison")
					time := 10;
				endif

				if (IsPoisonImune(attackInfo.GetDefender().GetObjRef(), poisonName))
					Message("Você está imune ao efeito dos venenos.", FONTCOLOR_SUCCESS).SendSys(attackInfo.GetDefender().GetObjRef());
					return false;
				endif

				if (CheckResist(attackInfo.GetDefender().GetObjRef(), FORTITUDE, RESIST_EASY, -2 * level))
					Message("Você resistiu ao veneno.", FONTCOLOR_SUCCESS).SendSys(attackInfo.GetDefender().GetObjRef());
					return false;
				else
					TS_StartTimer(attackInfo.GetDefender().GetObjRef(), poisonName, time, level, attackInfo.GetAttacker().GetObjRef());
				endif
			endif
		else
			return false;
		endif
	else // Se for player, checa pela arma/munição
		var sourcePoison := attackInfo.GetAmmo() ?: attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetObjRef();
		var poisonInfo := sourcePoison.GetProp("poisonInfo");

		if (poisonInfo == error)
			return false;
		endif

		if (poisonInfo.dosage != "infinito")
			poisonInfo.dosage := poisonInfo.dosage - 1;
			if (poisonInfo.dosage <= 0) // TODO: Talvez dê bug na aplicação do veneno se a dosagem for 1, prestar atenção nisso aqui quando o combate estiver pronto
				Message("Sua carga de veneno acabou.", FONTCOLOR_FAIL).SendSys(attackInfo.GetAttacker().GetObjRef());
				sourcePoison.EraseProp("poisonInfo");
				return false;
			else
				Message($"Restam {poisonInfo.dosage} doses de {poisonInfo.name}.", FONTCOLOR_ALERT).SendSys(attackInfo.GetAttacker().GetObjRef());
				sourcePoison.SetProp("poisonInfo", poisonInfo);
			endif
		endif

		if (IsPoisonImune(attackInfo.GetDefender().GetObjRef(), poisonInfo.name))
			Message("Seu veneno parece não ter surtido efeito no alvo.", FONTCOLOR_FAIL).SendSys(attackInfo.GetAttacker().GetObjRef());
			Message("Você está imune ao efeito dos venenos.", FONTCOLOR_SUCCESS).SendSys(attackInfo.GetDefender().GetObjRef());
			return false;
		endif

		if (poisonInfo.name == "paralyzePoison")
			var duration := CInt(attackInfo.GetAttacker().GetSkill(SCIENCE) / 20);

			if (duration < 2)
				duration := 2;
			endif

			if (attackInfo.GetAttacker().HasAbility(COMBAT_ABILITY::VENENO_PURO))
				duration := duration * 2;
			endif

			poisonInfo.time := duration;
		endif

		if (CheckResist(attackInfo.GetDefender().GetObjRef(), FORTITUDE, poisonInfo.difficulty, -poisonInfo.difficulty_mod))
			Message("Seu veneno parece não ter surtido efeito no alvo.", FONTCOLOR_FAIL).SendSys(attackInfo.GetAttacker().GetObjRef());
			Message("Você resistiu ao veneno.", FONTCOLOR_SUCCESS).SendSys(attackInfo.GetDefender().GetObjRef());
			return false;
		else
			if (TS_GetTimerInfo(attackInfo.GetDefender().GetObjRef(), "defaultPoison")) // confere se está sob default poison para estabelecer o mais forte
				var defaultpoison := cint(TS_GetTimerLevel(attackInfo.GetDefender().GetObjRef(), "defaultPoison"));
				attackInfo.GetDefender().GetObjRef().SetProp("#poisontier", defaultpoison);
			endif
			
			if (attackInfo.GetDefender().GetObjRef().GetProp("#poisontier"))
				var poison := TS_GetTimerNames(attackInfo.GetDefender().GetObjRef(), type := "P");
				var teste := CInt(attackInfo.GetDefender().GetObjRef().GetProp("#poisontier"));

				if (teste <= poisonInfo.tier)
					// compara o poison atual com o novo, substituindo caso o novo seja maior
					TS_LowerDuration(attackInfo.GetDefender().GetObjRef(), poison, -1);
					TS_StartTimer(attackInfo.GetDefender().GetObjRef(), poisonInfo.Name, poisonInfo.time, poisonInfo.level, attackInfo.GetAttacker().GetObjRef());
				endif
			else
				attackInfo.GetDefender().GetObjRef().SetProp("#poisontier", poisonInfo.tier);
				attackInfo.GetDefender().GetObjRef().SetProp("#poisonbonus", poisonInfo.difficulty_mod);
				TS_StartTimer(attackInfo.GetDefender().GetObjRef(), poisonInfo.name, poisonInfo.time, poisonInfo.level, attackInfo.GetAttacker().GetObjRef());
			endif
		endif
	endif

	Message("Seu veneno surtir efeito no alvo!", FONTCOLOR_SUCCESS).SendSys(attackInfo.GetAttacker().GetObjRef());
	Message("Você foi envenenado.", FONTCOLOR_FAIL).SendSys(attackInfo.GetDefender().GetObjRef());
	return true;
endfunction
