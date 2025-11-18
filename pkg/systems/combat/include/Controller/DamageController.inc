/* $Id$
 *
 * Purpose
 * Handles damages and resistances to damage types
 *
 */

// Internal
include ":combat:armorZones";
include ":combat:Model/CombatAbility";
include ":combat:Model/Damage";
// include "AbilitiesController";
include "SkillController";
include "ParryController";
include ":combat:weardown";

// External
include ":io:CFGFile";

function ProccessCombatDamage(byRef attackInfo)
	// Pre Hit
	CalculateCombatDamage(attackInfo);
	CheckParry(attackInfo);
	ExecuteCombatAbilitiesBeforeDamage(attackInfo);
	
	// Hit
	ApplyCombatDamage(attackInfo);
	
	// Post Hit
	CheckPoison(attackInfo);
	ExecuteCombatAbilitiesAfterDamage(attackInfo);
	ProgressAttackSkills(attackInfo);
endfunction

function CalculateCombatDamage(byRef attackInfo)
	CalculateAttackerDamage(attackInfo);
	CheckCriticalHit(attackInfo);
	CalculateDefenderResist(attackInfo);
endfunction

function CalculateAttackerDamage(byRef attackInfo)
	attackInfo.SetDamage(Damage());
	CalculateStatusDamage(attackInfo);
	CalculateSkillDamage(attackInfo);
	CalculateWeaponDamage(attackInfo);
	
	var damage := attackInfo.GetDamage();
	damage.SetSlashBaseDamage(damage.GetSlashRawDamage());
	damage.SetPiercingBaseDamage(damage.GetPiercingRawDamage());
	damage.SetBluntBaseDamage(damage.GetBluntRawDamage());
	damage.SetFireBaseDamage(damage.GetFireRawDamage());
	damage.SetColdBaseDamage(damage.GetColdRawDamage());
	damage.SetPoisonBaseDamage(damage.GetPoisonRawDamage());
	damage.SetEnergyBaseDamage(damage.GetEnergyRawDamage());
	attackInfo.SetDamage(damage);

	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"Dano a ser calculado para defesa: {attackInfo.GetDamage().ToString()}");
endfunction

function CalculateStatusDamage(byRef attackInfo)
	var damage := attackInfo.GetDamage();
	var damageMod := 0;

	var stat := "";
	var statMod := 0;
	if (attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetAttribute() == RANGED)
		stat := DEXTERITY;
		statMod := CInt(attackInfo.GetAttacker().GetDex());
	elseif (attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetAttribute() == THROWING)
		stat := $"{STRENGTH} + {DEXTERITY}";
		statMod := CInt((attackInfo.GetAttacker().GetStr() + attackInfo.GetAttacker().GetDex()) / 2);
	else
		stat := STRENGTH;
		statMod := CInt(attackInfo.GetAttacker().GetStr());
	endif

	damageMod := CInt(statMod / 10);
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"Bônus de dano por {Upper(stat)}: {damageMod}");
	
	if (attackInfo.GetAttacker().GetCombatTraits().GetPosture() == DAMAGE_TYPE::SLASH)
		damage.IncreaseSlashRawDamage(damageMod);
	elseif (attackInfo.GetAttacker().GetCombatTraits().GetPosture() == DAMAGE_TYPE::PIERCING)
		damage.IncreasePiercingRawDamage(damageMod);
	elseif (attackInfo.GetAttacker().GetCombatTraits().GetPosture() == DAMAGE_TYPE::BLUNT)
		damage.IncreaseBluntRawDamage(damageMod);
	endif

	attackInfo.SetDamage(damage);
endfunction

function CalculateSkillDamage(byRef attackInfo)
	var skill := TACTICS;
	var damage := attackInfo.GetDamage();
	var damageMod := CInt(attackInfo.GetAttacker().GetSkill(skill) / 10);

	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"Bônus de dano por {Upper(skill)}: {damageMod}");
	
	if (attackInfo.GetAttacker().GetCombatTraits().GetPosture() == DAMAGE_TYPE::SLASH)
		damage.IncreaseSlashRawDamage(damageMod);
	elseif (attackInfo.GetAttacker().GetCombatTraits().GetPosture() == DAMAGE_TYPE::PIERCING)
		damage.IncreasePiercingRawDamage(damageMod);
	elseif (attackInfo.GetAttacker().GetCombatTraits().GetPosture() == DAMAGE_TYPE::BLUNT)
		damage.IncreaseBluntRawDamage(damageMod);
	endif
	
	attackInfo.SetDamage(damage);
endfunction

function CalculateWeaponDamage(byRef attackInfo)
	var damageDice;

	if (attackInfo.GetAttacker().IsNPC() && !attackInfo.GetAttacker().GetEquipBySlot(WEAPON))
		damageDice := attackInfo.GetAttacker().GetNPCDamage();
	else
		if (attackInfo.GetDamageHand() == DAMAGE_HAND::MAINHAND)
			damageDice := attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetMainDamage();
		elseif (attackInfo.GetDamageHand() == DAMAGE_HAND::OFFHAND)
			damageDice := attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetOffDamage();
		endif
	endif

	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, "Dados de dano:" +
		$"{Upper(DAMAGE_TYPE::SLASH)}: {damageDice.GetSlashDamage().ToString()}" +
		$"{Upper(DAMAGE_TYPE::PIERCING)}: {damageDice.GetPiercingDamage().ToString()}" +
		$"{Upper(DAMAGE_TYPE::BLUNT)}: {damageDice.GetBluntDamage().ToString()}" +
		$"{Upper(DAMAGE_TYPE::FIRE)}: {damageDice.GetFireDamage().ToString()}" +
		$"{Upper(DAMAGE_TYPE::COLD)}: {damageDice.GetColdDamage().ToString()}" +
		$"{Upper(DAMAGE_TYPE::POISON)}: {damageDice.GetPoisonDamage().ToString()}" +
		$"{Upper(DAMAGE_TYPE::ENERGY)}: {damageDice.GetEnergyDamage().ToString()}"
	);

	var damage := attackInfo.GetDamage();
	var slashDamage := attackInfo.GetAttacker().GetCombatTraits().GetPosture() == DAMAGE_TYPE::SLASH ? damageDice.GetSlashDamage().Roll().GetResult() : 0;
	var piercingDamage := attackInfo.GetAttacker().GetCombatTraits().GetPosture() == DAMAGE_TYPE::PIERCING ? damageDice.GetPiercingDamage().Roll().GetResult() : 0;
	var bluntDamage := attackInfo.GetAttacker().GetCombatTraits().GetPosture() == DAMAGE_TYPE::BLUNT ? damageDice.GetBluntDamage().Roll().GetResult() : 0;
	var fireDamage := damageDice.GetFireDamage().Roll().GetResult();
	var coldDamage := damageDice.GetColdDamage().Roll().GetResult();
	var poisonDamage := damageDice.GetPoisonDamage().Roll().GetResult();
	var energyDamage := damageDice.GetEnergyDamage().Roll().GetResult();

	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, "Dano por tipo:" +
		$"{Upper(DAMAGE_TYPE::SLASH)}: {slashDamage}" +
		$"{Upper(DAMAGE_TYPE::PIERCING)}: {piercingDamage}" +
		$"{Upper(DAMAGE_TYPE::BLUNT)}: {bluntDamage}" +
		$"{Upper(DAMAGE_TYPE::FIRE)}: {fireDamage}" +
		$"{Upper(DAMAGE_TYPE::COLD)}: {coldDamage}" +
		$"{Upper(DAMAGE_TYPE::POISON)}: {poisonDamage}" +
		$"{Upper(DAMAGE_TYPE::ENERGY)}: {energyDamage}"
	);
	
	damage.IncreaseSlashRawDamage(slashDamage);
	damage.IncreasePiercingRawDamage(piercingDamage);
	damage.IncreaseBluntRawDamage(bluntDamage);
	damage.IncreaseFireRawDamage(fireDamage);
	damage.IncreaseColdRawDamage(coldDamage);
	damage.IncreasePoisonRawDamage(poisonDamage);
	damage.IncreaseEnergyRawDamage(energyDamage);
	attackInfo.SetDamage(damage);
endfunction

function CalculateDefenderResist(byRef attackInfo)
	var armorsInBodyRegion := attackInfo.GetDefender().GetEquipBySlot(attackInfo.GetBodyRegion());
	armorsInBodyRegion := OrderEquipsByAttackDirection(armorsInBodyRegion, attackInfo.GetDirection());

	if (armorsInBodyRegion == {})
		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, "Não há armadura na região atingida.");
	else
		foreach equip in (armorsInBodyRegion)
			var armorResist := equip.GetResists();
			attackInfo.GetDamage().DecreaseSlashBaseDamage(armorResist.GetSlashResist());
			attackInfo.GetDamage().DecreasePiercingBaseDamage(armorResist.GetPiercingResist());
			attackInfo.GetDamage().DecreaseBluntBaseDamage(armorResist.GetBluntResist());
			attackInfo.GetDamage().DecreaseFireBaseDamage(armorResist.GetFireResist());
			attackInfo.GetDamage().DecreaseColdBaseDamage(armorResist.GetColdResist());
			attackInfo.GetDamage().DecreasePoisonBaseDamage(armorResist.GetPoisonResist());
			attackInfo.GetDamage().DecreaseEnergyBaseDamage(armorResist.GetEnergyResist());
			DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"Equipamento {equip.GetName()} reduziu dano em: {armorResist.ToString()}");
			DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"Restou: {attackInfo.GetDamage().ToString()}");
		endforeach
	endif

	// TODO: Colocar um debug informando quanto foi reduzido por cada tipo de dano

	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"Dano a ser aplicado: {attackInfo.GetDamage().ToString()}");
endfunction

function OrderEquipsByAttackDirection(armorsInBodyRegion, attackDirection := ATTACK_DIRECTION::FRONT)
	var FRONT_TO_BACK := array{
		LAYER_ROBE,
		LAYER_TUNIC,
		LAYER_SKIRT,
		LAYER_HELM,
		LAYER_GLOVES,
		LAYER_NECK,
		LAYER_WAIST,
		LAYER_ARMS,
		LAYER_CHEST,
		LAYER_SHIRT,
		LAYER_LEGS,
		LAYER_PANTS,
		LAYER_SHOES,
		LAYER_CAPE
	};
	var BACK_TO_FRONT := array{
		LAYER_CAPE,
		LAYER_ROBE,
		LAYER_TUNIC,
		LAYER_SKIRT,
		LAYER_HELM,
		LAYER_ARMS,
		LAYER_WAIST,
		LAYER_CHEST,
		LAYER_SHIRT,
		LAYER_LEGS,
		LAYER_PANTS,
		LAYER_SHOES,
		LAYER_NECK,
		LAYER_GLOVES
	};

	var orderedEquips := array{};
	var orderArray := attackDirection == ATTACK_DIRECTION::BACK ? BACK_TO_FRONT : FRONT_TO_BACK;

	foreach layer in (orderArray)
		foreach equip in (armorsInBodyRegion)
			if (equip.GetLayer() == layer)
				orderedEquips.Append(equip);
			endif
		endforeach
	endforeach

	return orderedEquips;
endfunction

function CheckCriticalHit(byRef attackInfo)
	// Testar primeiro sem delay. Se der errado, a gente ativa isso aqui de novo...
	// if (attackInfo.GetAttacker().GetObjRef().GetProp("critdelay") > ReadGameClock())
	// 	return false;
	// endif
	
	CalculateCriticalChance(attackInfo);

	if (attackInfo.IsCriticalHit())
		ApplyCriticalDamage(attackInfo);
	endif

	return true;
endfunction

function CalculateCriticalChance(byRef attackInfo)
endfunction

function ApplyCriticalDamage(byRef attackInfo)
	// var weapon_scripts := array{};

	// var pet_instinto := attacker.GetProp("Group_savage"); // animal tamado com a hab Instinto Selvagem para adicionad criticos diferentes
	// var pet_critic := attacker.GetProp("Critic");
	// var pet_selvagem := attacker.GetProp("Selvagem"); // verifica se é animal selvagem - nunca tamado, para dar dano critico

	// var npcCfg := CFGFile("combat", "npcdesc").SetElementName(attacker.npctemplate);
	// var tamingCfg := CFGFile("taming", "taming").SetElementName(pet_critic);
	// var weaponCfg := CFGFile("*", "itemdesc").SetElementName(weapon.objtype);
	
	// var pet_txt := tamingCfg.SetKeyName("Text").GetKeyValue();
	// var tem_critico := npcCfg.SetElementName("CriticalHitScripts").GetElementValue(); // Se tem critico no npcdesc definido

	// if (!attacker.ip && weapon.IsA(POLCLASS_WEAPON) && !pet_instinto && !pet_selvagem)
	// 	if (tem_critico == {})
	// 		weapon_scripts := weaponCfg.SetKeyName("CriticalHitScripts").GetElementValue();
	// 	else
	// 		weapon_scripts := tem_critico;
	// 	endif
	// elseif (pet_instinto == 1 || pet_selvagem == 1)
	// 	weapon_scripts := tamingCfg.GetElementValue("CriticalHitScripts");
	// 	Message(pet_txt[1]).SendAbove(attacker);
	// else
	// 	weapon_scripts := weaponCfg.SetKeyName("CriticalHitScripts").GetElementValue();
	// endif
	// // Possivel mudar o critico de npc's. Os npcs só tem uma arma, a Intrinsic Weapon, mesmo que alguma esteja equipada.

	// var criticoFabricacao := CInt(PegaValorFabricacao(attacker.weapon, CARACTERISTICA_FABRICACAO_CRITICO));
	// var reducaoCriticoFabricacao := CInt(PegaValorFabricacao(attacker.weapon, CARACTERISTICA_FABRICACAO_REDUCAOCRITICO));
	// var can_crit := criticoFabricacao > 0 || attacker.npctemplate || weapon == FIST;

	// if (weapon_scripts.size() > 0 && can_crit)
	// 	var multiplicador := 7;
	// 	reducaoCriticoFabricacao := reducaoCriticoFabricacao == OT_LONG ? reducaoCriticoFabricacao : 0;
	// 	multiplicador -= (criticoFabricacao - reducaoCriticoFabricacao);
		
	// 	var critdelay := RandomIntMinMax(15 * multiplicador, 130 + (15 * multiplicador));
	// 	if (!GetObjProperty(weapon, "polida"))
	// 		Message("Sua arma está embotada!").SetColor(SSM_FAIL).SendSys(attacker);
	// 		critdelay := ReadGameClock() + RandomIntMinMax(240, 450);
	// 	endif

	// 	Message("*CRÍTICO*").SetColor(2118).SendAbove(attacker);
	// 	SetObjProperty(attacker, "critdelay", ReadGameClock() + critdelay);

	// 	var isDwarverDefender := GetObjProperty(defender, "chardata").trait == "Defensor Anão";
	// 	if (GetCooldown(defender, "vitalidadeinabalavel") > 0 || isDwarverDefender)
	// 		Message("Você ignora o crítico!").SetColor(SSM_INFO).SendSys(defender);
	// 		Message("O defensor ignorou o crítico!").SetColor(SSM_FAIL).SendSys(attacker);
	// 		return 1;
	// 	endif
	// elseif (!can_crit)
	// 	// Message("Apenas armas elaboradas causam crítico").SetColor(SSM_FAIL).SendSys(attacker);
	// 	return 1;
	// endif

	// foreach hitscript in weapon_scripts
	// 	var script := start_script(
	// 		hitscript,
	// 		array{ attacker, defender, weapon, armor, basedamage, rawdamage }
	// 	);
	// 	if (!script || script.errortext)
	// 		Message("*Attacker* Weapon script error starting [" + hitscript + "] :" + script.errortext).SendSys(attacker);
	// 		Message("*Attacker* Weapon script error starting [" + hitscript + "] :" + script.errortext).SendSys(defender);
	// 	endif
	// endforeach

	// return true;
endfunction

function ApplyCombatDamage(byRef attackInfo)
	var slash, piercing, blunt, fire, cold, poison, energy;
	slash 	 := attackInfo.GetDamage().GetSlashDamage();
	piercing := attackInfo.GetDamage().GetPierceDamage();
	blunt 	 := attackInfo.GetDamage().GetBluntDamage();
	fire 	 := attackInfo.GetDamage().GetFireDamage();
	cold 	 := attackInfo.GetDamage().GetColdDamage();
	poison 	 := attackInfo.GetDamage().GetPoisonDamage();
	energy 	 := attackInfo.GetDamage().GetEnergyDamage();
	
	if (slash > 0)
		ApplyRawDamage(attackInfo.GetDefender().GetObjRef(), CInt(slash));
		DisplayDamage(attackInfo.GetDefender().GetObjRef(), CInt(slash), attackInfo.GetAttacker().GetObjRef());
	endif

	if (piercing > 0)
		ApplyRawDamage(attackInfo.GetDefender().GetObjRef(), CInt(piercing));
		DisplayDamage(attackInfo.GetDefender().GetObjRef(), CInt(piercing), attackInfo.GetAttacker().GetObjRef());
	endif

	if (blunt > 0)
		ApplyRawDamage(attackInfo.GetDefender().GetObjRef(), CInt(blunt));
		DisplayDamage(attackInfo.GetDefender().GetObjRef(), CInt(blunt), attackInfo.GetAttacker().GetObjRef());
	endif

	if (fire > 0)
		ApplyRawDamage(attackInfo.GetDefender().GetObjRef(), CInt(fire));
		DisplayDamage(attackInfo.GetDefender().GetObjRef(), CInt(fire), attackInfo.GetAttacker().GetObjRef());
	endif

	if (cold > 0)
		ApplyRawDamage(attackInfo.GetDefender().GetObjRef(), CInt(cold));
		DisplayDamage(attackInfo.GetDefender().GetObjRef(), CInt(cold), attackInfo.GetAttacker().GetObjRef());
	endif

	if (poison > 0)
		ApplyRawDamage(attackInfo.GetDefender().GetObjRef(), CInt(poison));
		DisplayDamage(attackInfo.GetDefender().GetObjRef(), CInt(poison), attackInfo.GetAttacker().GetObjRef());
	endif

	if (energy > 0)
		ApplyRawDamage(attackInfo.GetDefender().GetObjRef(), CInt(energy));
		DisplayDamage(attackInfo.GetDefender().GetObjRef(), CInt(energy), attackInfo.GetAttacker().GetObjRef());
	endif
endfunction

function CheckPoison(byRef attackInfo)
	// TODO: Tem várias melhorias que podem ser feitas aqui. Vou deixar pra depois,se der algum bug no combate com veneno. Por enquanto vai tomar muito tempo.
	// TODO: Sugestão de melhoria: Criar um CanPoison() e depois um TryToPoison(), separando a lógica de verificação da aplicação do veneno em si.
	if (attackInfo.GetDamage().GetTotalBaseDamage() < 1)
		return false;
	endif

	if (attackInfo.GetDefender().HasAbility(COMBAT_ABILITY::SENSO_DA_NATUREZA))
		Message("Você é imune ao efeito dos venenos.", FONTCOLOR_SUCCESS).SendSys(attackInfo.GetDefender().GetObjRef());
		return false;
	endif

	if (attackInfo.GetAttacker().IsNPC()) // Se for NPC, checa pela skill dele
		var poisonchance := GetConfigInt(attackInfo.GetAttacker().GetNPCTemplate(), "Poisoning");
		if (poisonChance > 0)
			if (RandomInt(100) < poisonChance)
				var poisonName := GetConfigString(attackInfo.GetAttacker().GetNPCTemplate(), "PoisonType");
				var level := Cint(GetConfigInt(attackInfo.GetAttacker().GetNPCTemplate(), "PoisonLevel"));

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
		var sourcePoison := attackInfo.GetAmmoObjRef() ?: attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetObjRef();
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
