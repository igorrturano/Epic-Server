/* $Id$
 *
 * Purpose
 * Handles damages and resistances to damage types
 *
 */

// Internal
include ":combat:armorZones";
// include ":abilities:models/Ability";
include ":combat:Model/Damage";
// include "AbilitiesController";
include "SkillController";
include "ParryController";
include ":combat:weardown";

// External
include ":io:CFGFile";
include ":wrappers:include/Objects/Model/MobileWrapper";

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
	CalculateAttackerCriticalHit(attackInfo);
	CalculateDefenderResist(attackInfo);
endfunction

function CalculateAttackerDamage(byRef attackInfo)
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message("================="));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message("CÁLCULO DE DANO!"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message("================="));
	attackInfo.SetDamage(Damage());
	CalculateStatusDamage(attackInfo);
	CalculateSkillDamage(attackInfo);
	CalculateWeaponDamage(attackInfo);
	CalculateEntropyDamage(attackInfo);

	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"Dano a ser aplicar (antes da defesa): {attackInfo.GetDamage().ToString()}");
endfunction

function CalculateStatusDamage(byRef attackInfo)
	var stat := "";
	var statBase := 0;
	var statDivisor := 20;
	if (attackInfo.GetDamageHand() == DAMAGE_HAND::OFFHAND)
		statDivisor *= 2;
	endif

	if (attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetAttribute() == RANGED)
		stat := DEXTERITY;
		statBase := CInt(attackInfo.GetAttacker().GetDex());
		
		if (attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetCategory() == WEAPON_CATEGORY::THROWING)
			stat := $"{STRENGTH} + {DEXTERITY}";
			statBase := CInt((attackInfo.GetAttacker().GetStr() + attackInfo.GetAttacker().GetDex()) / 2);
		endif
	else
		stat := STRENGTH;
		statBase := CInt(attackInfo.GetAttacker().GetStr());
	endif

	var damageMod := CInt(statBase / statDivisor);
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"Bônus de dano por {Upper(stat)}: {damageMod}"));
	
	attackInfo.GetDamage().IncreaseSlashRawDamage(damageMod)
						  .IncreasePiercingRawDamage(damageMod)
						  .IncreaseBluntRawDamage(damageMod);
endfunction

function CalculateSkillDamage(byRef attackInfo)
	var skill := TACTICS;
	var skillDivisor := 20;
	if (attackInfo.GetDamageHand() == DAMAGE_HAND::OFFHAND)
		skillDivisor *= 2;
	endif
	var damageMod := CInt(attackInfo.GetAttacker().GetSkill(skill) / skillDivisor);

	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"Bônus de dano por {Upper(skill)}: {damageMod}"));
	
	attackInfo.GetDamage().IncreaseSlashRawDamage(damageMod)
						  .IncreasePiercingRawDamage(damageMod)
						  .IncreaseBluntRawDamage(damageMod);
endfunction

function CalculateWeaponDamage(byRef attackInfo)
	var damageDice;

	if (attackInfo.GetDamageHand() == DAMAGE_HAND::MAINHAND)
		damageDice := attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetMainDamage();
	elseif (attackInfo.GetDamageHand() == DAMAGE_HAND::OFFHAND)
		damageDice := attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetOffDamage();
	endif

	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message("================="));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"Dados de dano da arma ({attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetName()}):"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::SLASH)}: {damageDice.GetSlashDamage().ToString() ?: "N/A"}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::PIERCING)}: {damageDice.GetPiercingDamage().ToString() ?: "N/A"}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::BLUNT)}: {damageDice.GetBluntDamage().ToString() ?: "N/A"}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::FIRE)}: {damageDice.GetFireDamage().ToString() ?: "N/A"}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::COLD)}: {damageDice.GetColdDamage().ToString() ?: "N/A"}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::POISON)}: {damageDice.GetPoisonDamage().ToString() ?: "N/A"}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::ENERGY)}: {damageDice.GetEnergyDamage().ToString() ?: "N/A"}"));

	var slashDamage := damageDice.GetSlashDamage().Roll().GetResult() ?: 0;
	var piercingDamage := damageDice.GetPiercingDamage().Roll().GetResult() ?: 0;
	var bluntDamage := damageDice.GetBluntDamage().Roll().GetResult() ?: 0;
	var fireDamage := damageDice.GetFireDamage().Roll().GetResult() ?: 0;
	var coldDamage := damageDice.GetColdDamage().Roll().GetResult() ?: 0;
	var poisonDamage := damageDice.GetPoisonDamage().Roll().GetResult() ?: 0;
	var energyDamage := damageDice.GetEnergyDamage().Roll().GetResult() ?: 0;

	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message("================="));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message("Dano rolado:"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::SLASH)}: {slashDamage}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::PIERCING)}: {piercingDamage}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::BLUNT)}: {bluntDamage}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::FIRE)}: {fireDamage}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::COLD)}: {coldDamage}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::POISON)}: {poisonDamage}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::ENERGY)}: {energyDamage}"));
	
	attackInfo.GetDamage().IncreaseSlashRawDamage(slashDamage)
						  .IncreasePiercingRawDamage(piercingDamage)
						  .IncreaseBluntRawDamage(bluntDamage)
						  .IncreaseFireRawDamage(fireDamage)
						  .IncreaseColdRawDamage(coldDamage)
						  .IncreasePoisonRawDamage(poisonDamage)
						  .IncreaseEnergyRawDamage(energyDamage);

	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message("================="));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message("Modificadores de dano:"));
	var damageMod := attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetDamageMod();
	attackInfo.GetDamage().IncreaseSlashRawDamage(damageMod.GetSlashRawDamage())
						  .IncreasePiercingRawDamage(damageMod.GetPiercingRawDamage())
						  .IncreaseBluntRawDamage(damageMod.GetBluntRawDamage())
						  .IncreaseFireRawDamage(damageMod.GetFireRawDamage())
						  .IncreaseColdRawDamage(damageMod.GetColdRawDamage())
						  .IncreasePoisonRawDamage(damageMod.GetPoisonRawDamage())
						  .IncreaseEnergyRawDamage(damageMod.GetEnergyRawDamage());
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::SLASH)}: {damageMod.GetSlashRawDamage()}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::PIERCING)}: {damageMod.GetPiercingRawDamage()}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::BLUNT)}: {damageMod.GetBluntRawDamage()}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::FIRE)}: {damageMod.GetFireRawDamage()}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::COLD)}: {damageMod.GetColdRawDamage()}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::POISON)}: {damageMod.GetPoisonRawDamage()}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::ENERGY)}: {damageMod.GetEnergyRawDamage()}"));

	// Apply ammunition damage if ammo exists
	if (attackInfo.GetAmmo())
		ApplyAmmoDamage(attackInfo);
	endif
endfunction

function CalculateDefenderResist(byRef attackInfo)
	var armorsInBodyRegion := GetEquipsByAttackDirection(attackInfo);
	var bodyResist := attackInfo.GetDefender().GetBodyResist();

	attackInfo.GetDamage().DecreaseSlashBaseDamage(bodyResist.GetSlashResist())
						  .DecreasePiercingBaseDamage(bodyResist.GetPiercingResist())
						  .DecreaseBluntBaseDamage(bodyResist.GetBluntResist())
						  .DecreaseFireBaseDamage(bodyResist.GetFireResist())
						  .DecreaseColdBaseDamage(bodyResist.GetColdResist())
						  .DecreasePoisonBaseDamage(bodyResist.GetPoisonResist())
						  .DecreaseEnergyBaseDamage(bodyResist.GetEnergyResist());
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"Defesa natural (BODY) do defensor reduziu dano em: {bodyResist.ToString()}"));
	
	if (armorsInBodyRegion == array{})
		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message("Não há armadura na região atingida. Dano direto no corpo do defensor."));
	else
		foreach equip in (armorsInBodyRegion)
			var armorResist := equip.GetResists();
			attackInfo.GetDamage().DecreaseSlashBaseDamage(armorResist.GetSlashResist())
								  .DecreasePiercingBaseDamage(armorResist.GetPiercingResist())
								  .DecreaseBluntBaseDamage(armorResist.GetBluntResist())
								  .DecreaseFireBaseDamage(armorResist.GetFireResist())
								  .DecreaseColdBaseDamage(armorResist.GetColdResist())
								  .DecreasePoisonBaseDamage(armorResist.GetPoisonResist())
								  .DecreaseEnergyBaseDamage(armorResist.GetEnergyResist());
			DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"Equipamento {equip.GetName()} reduziu dano em: {armorResist.ToString()}"));
			DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"Restou: {attackInfo.GetDamage().ToString()}"));
		endforeach
	endif

	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message("================="));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message("Dano resistido:"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::SLASH)}: {CInt(attackInfo.GetDamage().GetSlashRawDamage() - attackInfo.GetDamage().GetSlashBaseDamage())}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::PIERCING)}: {CInt(attackInfo.GetDamage().GetPiercingRawDamage() - attackInfo.GetDamage().GetPiercingBaseDamage())}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::BLUNT)}: {CInt(attackInfo.GetDamage().GetBluntRawDamage() - attackInfo.GetDamage().GetBluntBaseDamage())}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::FIRE)}: {CInt(attackInfo.GetDamage().GetFireRawDamage() - attackInfo.GetDamage().GetFireBaseDamage())}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::COLD)}: {CInt(attackInfo.GetDamage().GetColdRawDamage() - attackInfo.GetDamage().GetColdBaseDamage())}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::POISON)}: {CInt(attackInfo.GetDamage().GetPoisonRawDamage() - attackInfo.GetDamage().GetPoisonBaseDamage())}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::ENERGY)}: {CInt(attackInfo.GetDamage().GetEnergyRawDamage() - attackInfo.GetDamage().GetEnergyBaseDamage())}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"Dano a ser aplicado (após defesa): {attackInfo.GetDamage().ToString()}"));
endfunction

function GetEquipsByAttackDirection(byRef attackInfo)
	var armorsInBodyRegion := attackInfo.GetDefender().GetEquipBySlot(attackInfo.GetBodyRegion());
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
	var orderArray := attackInfo.GetDirection() == ATTACK_DIRECTION::BACK ? BACK_TO_FRONT : FRONT_TO_BACK;
	var equipNamesForDebug := array{};

	foreach layer in (orderArray)
		foreach equip in (armorsInBodyRegion)
			if (equip.GetLayer() == layer)
				orderedEquips.Append(equip);
				equipNamesForDebug.Append(equip.GetName());
			endif
		endforeach
	endforeach

	if (equipNamesForDebug.size() > 0)
		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"Ordem dos equipamentos a serem atingidos: {equipNamesForDebug}"));
	endif

	return orderedEquips;
endfunction

function CalculateAttackerCriticalHit(byRef attackInfo)
	attackInfo.SetCriticalHitChance(CalculateCriticalChance(attackInfo));
	attackInfo.SetCriticalHitChanceMod(CalculateCriticalChanceMod(attackInfo));
	attackInfo.RollCriticalHitChance();

	if (attackInfo.GetDamage().IsCritical())
		ApplyCriticalDamage(attackInfo);
	endif

	attackInfo.GetAttacker().EraseProperty("#CriticalChance"); // Limpa a CProp de bonus critico para o próximo ataque

	return true;
endfunction

function CalculateCriticalChance(byRef attackInfo)
	var chance := 0;
	var skillUsedForCritical := TACTICS;

	if (attackInfo.GetAttacker().GetProperty("#nextcritical"))
		attackInfo.GetAttacker().EraseProperty("#nextcritical");
		return 100;
	endif

	if (attackInfo.GetDefender().HasAbility(ABILITY_ID::MUMIFICACAO))
		Message("Seu crítico não surte efeito", Color(SYSTEM_COLOR::FAIL)).SendSys(attackInfo.GetAttacker().GetObjRef());
		Message("Seu corpo evita o crítico", Color(SYSTEM_COLOR::SUCCESS)).SendSys(attackInfo.GetDefender().GetObjRef());
		return 0;
	endif

	chance := CInt(
		CInt(attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetCriticalChance()) +	// Crítico base da arma
		CInt(attackInfo.GetAttacker().GetCriticalChance()) +						// Crítico extra do Char (base + mod)
		CInt(attackInfo.GetAttacker().GetSkill(skillUsedForCritical) / 20) 			// Crítico extra da Skill
	);

	if (attackInfo.GetAttacker().HasAbility(ABILITY_ID::DESTRUIDOR))
		chance := chance * 2.0;
	endif

	if (attackInfo.GetDefender().HasAbility(ABILITY_ID::LADRAO_DA_SORTE))
		Message("Seu alvo desviou do crítico!", Color(SYSTEM_COLOR::FAIL)).SendSys(attackInfo.GetAttacker().GetObjRef());
		Message("Você desviou do crítico e usará isso contra seu oponente!", Color(SYSTEM_COLOR::SUCCESS)).SendSys(attackInfo.GetDefender().GetObjRef());
		attackInfo.GetDefender().SetProperty("#nextcritical", true);
		return 0;
	endif

	if (attackInfo.GetAttacker().HasAbility(ABILITY_ID::OPORTUNIDADE_CRITICA))
		Message("*ataque de oportunidade*").SendAbove(attackInfo.GetAttacker().GetObjRef());
		attackInfo.GetAttacker().Attack(attackInfo.GetDefender());
	endif

	if (GetCooldown(attackInfo.GetDefender().GetObjRef(), "VontadeIndomavel") ||
		GetCooldown(attackInfo.GetDefender().GetObjRef(), "vitalidadeinabalavel") > 0 ||
	 	GetCooldown(attackInfo.GetDefender().GetObjRef(), "balance_restored"))
		Message("Você não leva o crítico").SendSys(attackInfo.GetDefender().GetObjRef());
		return 0;
	endif

	return CInt(chance);
endfunction

function CalculateCriticalChanceMod(byRef attackInfo)
	var criticalChanceMod := 0;

	criticalChanceMod -= CInt(attackInfo.GetDefender().GetEquipBySlot(attackInfo.GetBodyRegion()).GetCriticalReductionMod()); // Redução de critico da Armadura defensora.

	return CInt(criticalChanceMod);
endfunction

function ApplyCriticalDamage(byRef attackInfo)
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message("***DANO CRÍTICO***", Color(COLOR_NAME::RED)));

	// TODO: Refatorar isso daqui, tá MUITO cagado....
		// Jogar as coisas de pet pra uma função separada
	
	var weapon_scripts := array{};

	var npcCfg := CFGFile("combat", "npcdesc").SetElementName(attackInfo.GetAttacker().GetTemplate());
	var tamingCfg := CFGFile("taming", "taming").SetElementName(attackInfo.GetAttacker().GetObjRef().GetProp("Critic"));
	var weaponCfg := CFGFile("*", "itemdesc").SetElementName(attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetObjType());

	var pet_instinto := attackInfo.GetAttacker().GetObjRef().GetProp("Group_savage"); // animal tamado com a hab Instinto Selvagem para adicionad criticos diferentes
	var pet_selvagem := attackInfo.GetAttacker().GetObjRef().GetProp("Selvagem"); // verifica se é animal selvagem - nunca tamado, para dar dano critico
	var pet_txt := tamingCfg.GetPropertyValue("Text");

	if (pet_instinto || pet_selvagem) // Se for pet
		weapon_scripts := tamingCfg.GetPropertyValue("CriticalHitScripts");
		Message(pet_txt[1]).SendAbove(attackInfo.GetAttacker().GetObjRef());
	elseif (attackInfo.GetAttacker().GetEquipBySlot(WEAPON))
		weapon_scripts := weaponCfg.GetPropertyValue("CriticalHitScripts");
	elseif (attackInfo.GetAttacker().IsNPC())
		weapon_scripts := npcCfg.GetPropertyValue("CriticalHitScripts");
	endif

	Message("CRÍTICO", Color(COLOR_NAME::RED)).SendEmote(attackInfo.GetAttacker().GetObjRef());
	ScreenShake(attackInfo.GetAttacker().GetObjRef(), 0.1, 5);
	// TODO: Checar se isso vai buscar apenas o 1º atributo ou se vem o array
	foreach hitscript in (weapon_scripts)
		foreach armor in (attackInfo.GetDefender().GetEquipBySlot(attackInfo.GetBodyRegion()))
			var script := start_script(
				hitscript,
				array{
					attackInfo.GetAttacker().GetObjRef(),
					attackInfo.GetDefender().GetObjRef(),
					attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetObjRef(),
					armor.GetObjRef(),
					attackInfo.GetDamage().GetTotalBaseDamage(),
					attackInfo.GetDamage().GetTotalRawDamage()
				}
			);
			if (!script || script.errortext)
				Message("*Attacker* Weapon script error starting [" + hitscript + "] :" + script.errortext, Color(SYSTEM_COLOR::FAIL)).SendSys(attackInfo.GetAttacker().GetObjRef());
			endif

			SleepMS(2);
		endforeach
	endforeach

	return true;
endfunction

function ApplyCombatDamage(byRef attackInfo)
	ExecuteCombatAbilitiesOnHit(attackInfo);

	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message("================="));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"Dano aplicado em: {attackInfo.GetDefender().GetName()}"));

	var posture := attackInfo.GetAttacker().GetCombatTraits().GetPosture();
	if (posture == DAMAGE_TYPE::SLASH)
		if (attackInfo.GetDamage().GetSlashBaseDamage() < 1)
			attackInfo.GetDamage().SetSlashBaseDamage(1);
			DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::SLASH)}: Dano mínimo (1) garantido.", Color(SYSTEM_COLOR::FAIL)));
		else
			DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::SLASH)}: {attackInfo.GetDamage().GetSlashBaseDamage()}"));
		endif

		DisplayDamage(attackInfo.GetDefender().GetObjRef(), attackInfo.GetDamage().GetSlashBaseDamage(), attackInfo.GetAttacker().GetObjRef());
		ApplyRawDamage(attackInfo.GetDefender().GetObjRef(), attackInfo.GetDamage().GetSlashBaseDamage());
	elseif (posture == DAMAGE_TYPE::PIERCING)
		if (attackInfo.GetDamage().GetPiercingBaseDamage() < 1)
			attackInfo.GetDamage().SetPiercingBaseDamage(1);
			DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::PIERCING)}: Dano mínimo (1) garantido.", Color(SYSTEM_COLOR::FAIL)));
		else
			DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::PIERCING)}: {attackInfo.GetDamage().GetPiercingBaseDamage()}"));
		endif

		DisplayDamage(attackInfo.GetDefender().GetObjRef(), attackInfo.GetDamage().GetPiercingBaseDamage(), attackInfo.GetAttacker().GetObjRef());
		ApplyRawDamage(attackInfo.GetDefender().GetObjRef(), attackInfo.GetDamage().GetPiercingBaseDamage());
	elseif (posture == DAMAGE_TYPE::BLUNT)
		if (attackInfo.GetDamage().GetBluntBaseDamage() < 1)
			attackInfo.GetDamage().SetBluntBaseDamage(1);
			DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::BLUNT)}: Dano mínimo (1) garantido.", Color(SYSTEM_COLOR::FAIL)));
		else
			DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::BLUNT)}: {attackInfo.GetDamage().GetBluntBaseDamage()}"));
		endif

		DisplayDamage(attackInfo.GetDefender().GetObjRef(), attackInfo.GetDamage().GetBluntBaseDamage(), attackInfo.GetAttacker().GetObjRef());
		ApplyRawDamage(attackInfo.GetDefender().GetObjRef(), attackInfo.GetDamage().GetBluntBaseDamage());
	endif

	if (attackInfo.GetDamage().GetFireBaseDamage() > 0)
		DisplayDamage(attackInfo.GetDefender().GetObjRef(), attackInfo.GetDamage().GetFireBaseDamage(), attackInfo.GetAttacker().GetObjRef());
		ApplyRawDamage(attackInfo.GetDefender().GetObjRef(), attackInfo.GetDamage().GetFireBaseDamage());
		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::FIRE)}: {attackInfo.GetDamage().GetFireBaseDamage()}"));
	endif

	if (attackInfo.GetDamage().GetColdBaseDamage() > 0)
		DisplayDamage(attackInfo.GetDefender().GetObjRef(), attackInfo.GetDamage().GetColdBaseDamage(), attackInfo.GetAttacker().GetObjRef());
		ApplyRawDamage(attackInfo.GetDefender().GetObjRef(), attackInfo.GetDamage().GetColdBaseDamage());
		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::COLD)}: {attackInfo.GetDamage().GetColdBaseDamage()}"));
	endif

	if (attackInfo.GetDamage().GetPoisonBaseDamage() > 0)
		DisplayDamage(attackInfo.GetDefender().GetObjRef(), attackInfo.GetDamage().GetPoisonBaseDamage(), attackInfo.GetAttacker().GetObjRef());
		ApplyRawDamage(attackInfo.GetDefender().GetObjRef(), attackInfo.GetDamage().GetPoisonBaseDamage());
		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::POISON)}: {attackInfo.GetDamage().GetPoisonBaseDamage()}"));
	endif

	if (attackInfo.GetDamage().GetEnergyBaseDamage() > 0)
		DisplayDamage(attackInfo.GetDefender().GetObjRef(), attackInfo.GetDamage().GetEnergyBaseDamage(), attackInfo.GetAttacker().GetObjRef());
		ApplyRawDamage(attackInfo.GetDefender().GetObjRef(), attackInfo.GetDamage().GetEnergyBaseDamage());
		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::ENERGY)}: {attackInfo.GetDamage().GetEnergyBaseDamage()}"));
	endif
endfunction

function CheckPoison(byRef attackInfo)
	// TODO: Tem várias melhorias que podem ser feitas aqui. Vou deixar pra depois,se der algum bug no combate com veneno. Por enquanto vai tomar muito tempo.
	// TODO: Sugestão de melhoria: Criar um CanPoison() e depois um TryToPoison(), separando a lógica de verificação da aplicação do veneno em si.
	if (attackInfo.GetDamage().GetTotalBaseDamage() < 1)
		return false;
	endif

	if (attackInfo.GetDefender().HasAbility(ABILITY_ID::SENSO_DA_NATUREZA))
		Message("Você é imune ao efeito dos venenos.", Color(SYSTEM_COLOR::SUCCESS)).SendSys(attackInfo.GetDefender().GetObjRef());
		return false;
	endif

	if (attackInfo.GetAttacker().IsNPC()) // Se for NPC, checa pela skill dele
		var poisonchance := GetConfigInt(attackInfo.GetAttacker().GetTemplate(), "Poisoning");
		if (poisonChance > 0)
			if (RandomInt(100) < poisonChance)
				var poisonName := GetConfigString(attackInfo.GetAttacker().GetTemplate(), "PoisonType");
				var level := Cint(GetConfigInt(attackInfo.GetAttacker().GetTemplate(), "PoisonLevel"));

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
					Message("Você está imune ao efeito dos venenos.", Color(SYSTEM_COLOR::SUCCESS)).SendSys(attackInfo.GetDefender().GetObjRef());
					return false;
				endif

				if (CheckResist(attackInfo.GetDefender().GetObjRef(), FORTITUDE, RESIST_EASY, -2 * level))
					Message("Você resistiu ao veneno.", Color(SYSTEM_COLOR::SUCCESS)).SendSys(attackInfo.GetDefender().GetObjRef());
					return false;
				else
					TS_StartTimer(attackInfo.GetDefender().GetObjRef(), poisonName, time, level, attackInfo.GetAttacker().GetObjRef());
				endif
			endif
		else
			return false;
		endif
	else // Se for player, checa pela arma/munição
		var sourcePoison := attackInfo.GetAmmo().GetItem().GetObjRef() ?: attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetObjRef();
		var poisonInfo := sourcePoison.GetProp("poisonInfo");

		if (poisonInfo == error)
			return false;
		endif

		if (poisonInfo.dosage != "infinito")
			poisonInfo.dosage := poisonInfo.dosage - 1;
			if (poisonInfo.dosage <= 0) // TODO: Talvez dê bug na aplicação do veneno se a dosagem for 1, prestar atenção nisso aqui quando o combate estiver pronto
				Message("Sua carga de veneno acabou.", Color(SYSTEM_COLOR::FAIL)).SendSys(attackInfo.GetAttacker().GetObjRef());
				sourcePoison.EraseProp("poisonInfo");
				return false;
			else
				Message($"Restam {poisonInfo.dosage} doses de {poisonInfo.name}.", Color(SYSTEM_COLOR::ALERT)).SendSys(attackInfo.GetAttacker().GetObjRef());
				sourcePoison.SetProp("poisonInfo", poisonInfo);
			endif
		endif

		if (IsPoisonImune(attackInfo.GetDefender().GetObjRef(), poisonInfo.name))
			Message("Seu veneno parece não ter surtido efeito no alvo.", Color(SYSTEM_COLOR::FAIL)).SendSys(attackInfo.GetAttacker().GetObjRef());
			Message("Você está imune ao efeito dos venenos.", Color(SYSTEM_COLOR::SUCCESS)).SendSys(attackInfo.GetDefender().GetObjRef());
			return false;
		endif

		if (poisonInfo.name == "paralyzePoison")
			var duration := CInt(attackInfo.GetAttacker().GetSkill(SCIENCE) / 20);

			if (duration < 2)
				duration := 2;
			endif

			if (attackInfo.GetAttacker().HasAbility(ABILITY_ID::VENENO_PURO))
				duration := duration * 2;
			endif

			poisonInfo.time := duration;
		endif

		if (CheckResist(attackInfo.GetDefender().GetObjRef(), FORTITUDE, poisonInfo.difficulty, -poisonInfo.difficulty_mod))
			Message("Seu veneno parece não ter surtido efeito no alvo.", Color(SYSTEM_COLOR::FAIL)).SendSys(attackInfo.GetAttacker().GetObjRef());
			Message("Você resistiu ao veneno.", Color(SYSTEM_COLOR::SUCCESS)).SendSys(attackInfo.GetDefender().GetObjRef());
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

	Message("Seu veneno surtiu efeito no alvo!", Color(SYSTEM_COLOR::SUCCESS)).SendSys(attackInfo.GetAttacker().GetObjRef());
	Message("Você foi envenenado.", Color(SYSTEM_COLOR::FAIL)).SendSys(attackInfo.GetDefender().GetObjRef());
	return true;
endfunction

function CalculateEntropyDamage(byRef attackInfo)
	var damage := attackInfo.GetDamage();
	var bonus_damage := 0;

	// Player entropy bonus: +1 damage per 4k entropy against undead/demons (12k-20k range)
	if (!attackInfo.GetAttacker().IsNPC() && attackInfo.GetDefender().IsNPC())
		var attacker_entropy := attackInfo.GetAttacker().GetEntropy();
		if (attacker_entropy >= 12000)
			var npc_category := Lower(attackInfo.GetDefender().GetCategory());
			if (npc_category == "undead" || npc_category == "demon" || npc_category == "void")
				// +1 per 4k entropy, max +5 at 20k
				var entropy_bonus := CInt(attacker_entropy / 4000);
				if (entropy_bonus > 5)
					entropy_bonus := 5;
				endif
				bonus_damage += entropy_bonus;
				DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"Bônus de entropia contra {npc_category}: +{entropy_bonus}");
			endif
		endif
	endif

	// Global entropy bonus: NPCs (undead/demon/void) get +1 damage per -20k global entropy
	var globalEntropy := CInt(GetGlobalProperty("GlobalEntropy"));
	if (globalEntropy <= -20000 && attackInfo.GetAttacker().IsNPC())
		var npc_category := Lower(attackInfo.GetAttacker().GetCategory());
		if (npc_category == "undead" || npc_category == "demon" || npc_category == "void")
			// +1 per -20k global entropy, max +5 at -100k
			var global_bonus := CInt(Abs(globalEntropy) / 20000);
			if (global_bonus > 5)
				global_bonus := 5;
			endif
			bonus_damage += global_bonus;
			DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"Bônus de entropia global para {npc_category}: +{global_bonus}");
		endif
	endif

	// Apply bonus damage to all damage types proportionally
	if (bonus_damage > 0)
		if (attackInfo.GetAttacker().GetCombatTraits().GetPosture() == DAMAGE_TYPE::SLASH)
			damage.IncreaseSlashRawDamage(bonus_damage);
		elseif (attackInfo.GetAttacker().GetCombatTraits().GetPosture() == DAMAGE_TYPE::PIERCING)
			damage.IncreasePiercingRawDamage(bonus_damage);
		elseif (attackInfo.GetAttacker().GetCombatTraits().GetPosture() == DAMAGE_TYPE::BLUNT)
			damage.IncreaseBluntRawDamage(bonus_damage);
		endif

		attackInfo.SetDamage(damage);
	endif
endfunction

function ApplyAmmoDamage(byref attackInfo)
		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message("================="));
		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message("Dano da munição:"));

		var ammoItem := attackInfo.GetAmmo().GetItem().GetObjRef();
		var ammoObjtype := ammoItem.objtype;

		// 1. Apply PHYSICAL damage from crafting (fabricado CProp -> componentes -> dano)
		var fabricado := attackInfo.GetAmmo().GetItem().GetProperty(PROPNAME_FABRICADO);
		if (fabricado && fabricado.componentes)
			var physicalDamage := 0;

			// Extract damage from components where caracteristica == "dano"
			foreach comp in (fabricado.componentes)
				if (comp.propriedades && comp.propriedades.caracteristica == "dano")
					var danoValue := comp.propriedades.valorPrincipal;
					physicalDamage += CInt(danoValue);
				endif
			endforeach

			// Apply as PIERCING for arrows/bolts, BLUNT for fireweapon ammo
			if (attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetCategory() == WEAPON_CATEGORY::BOW ||
				attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetCategory() == WEAPON_CATEGORY::CROSSBOW)
				attackInfo.GetDamage().IncreasePiercingRawDamage(physicalDamage);
				DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::PIERCING)}: {physicalDamage}"));
			else
				attackInfo.GetDamage().IncreaseBluntRawDamage(physicalDamage);
				DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::BLUNT)}: {physicalDamage}"));
			endif
		endif

		// 2. Apply ELEMENTAL damage from enchantments (AmmoDamage CProp)
		var ammoDamage := attackInfo.GetAmmo().GetItem().GetProperty("AmmoDamage");
		var fireDamage := ammoDamage.fire ?: 0;
		var coldDamage := ammoDamage.cold ?: 0;
		var poisonDamage := ammoDamage.poison ?: 0;
		var energyDamage := ammoDamage.energy ?: 0;

		if (ammoDamage)
			attackInfo.GetDamage()
				.IncreaseFireRawDamage(fireDamage)
				.IncreaseColdRawDamage(coldDamage)
				.IncreasePoisonRawDamage(poisonDamage)
				.IncreaseEnergyRawDamage(energyDamage);

			DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::FIRE)}: {fireDamage}"));
			DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::COLD)}: {coldDamage}"));
			DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::POISON)}: {poisonDamage}"));
			DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::ENERGY)}: {energyDamage}"));
		endif
endfunction