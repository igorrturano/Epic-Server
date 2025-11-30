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
	CalculateAttackerCriticalHit(attackInfo);
	CalculateDefenderResist(attackInfo);
endfunction

function CalculateAttackerDamage(byRef attackInfo)
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, "=================");
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, "CÁLCULO DE DANO!");
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, "=================");
	attackInfo.SetDamage(Damage());
	CalculateStatusDamage(attackInfo);
	CalculateSkillDamage(attackInfo);
	CalculateWeaponDamage(attackInfo);
	
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"Dano a ser aplicar (antes da defesa): {attackInfo.GetDamage().ToString()}");
endfunction

function CalculateStatusDamage(byRef attackInfo)
	var damage := attackInfo.GetDamage();

	var stat := "";
	var statBase := 0;
	var statDivisor := 20;
	if (attackInfo.GetDamageHand() == DAMAGE_HAND::OFFHAND)
		statDivisor *= 2;
	endif

	if (attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetAttribute() == RANGED)
		if (attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetCategory() == WEAPON_CATEGORY::BOW)
			stat := DEXTERITY;
			statBase := CInt(attackInfo.GetAttacker().GetDex());
		elseif (attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetCategory() == WEAPON_CATEGORY::CROSSBOW)
			stat := DEXTERITY;
			statBase := CInt(attackInfo.GetAttacker().GetDex());
		elseif (attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetCategory() == WEAPON_CATEGORY::THROWING)
			stat := $"{STRENGTH} + {DEXTERITY}";
			statBase := CInt((attackInfo.GetAttacker().GetStr() + attackInfo.GetAttacker().GetDex()) / 2);
		endif
	else
		stat := STRENGTH;
		statBase := CInt(attackInfo.GetAttacker().GetStr());
	endif

	var damageMod := CInt(statBase / statDivisor);
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
	var skillDivisor := 20;
	if (attackInfo.GetDamageHand() == DAMAGE_HAND::OFFHAND)
		skillDivisor *= 2;
	endif
	var damage := attackInfo.GetDamage();
	var damageMod := CInt(attackInfo.GetAttacker().GetSkill(skill) / skillDivisor);

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

	if (attackInfo.GetDamageHand() == DAMAGE_HAND::MAINHAND)
		damageDice := attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetMainDamage();
	elseif (attackInfo.GetDamageHand() == DAMAGE_HAND::OFFHAND)
		damageDice := attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetOffDamage();
	endif

	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, "=================");
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"Dados de dano da arma ({attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetName()}):");
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"{Upper(DAMAGE_TYPE::SLASH)}: {damageDice.GetSlashDamage().ToString() ?: "N/A"}");
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"{Upper(DAMAGE_TYPE::PIERCING)}: {damageDice.GetPiercingDamage().ToString() ?: "N/A"}");
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"{Upper(DAMAGE_TYPE::BLUNT)}: {damageDice.GetBluntDamage().ToString() ?: "N/A"}");
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"{Upper(DAMAGE_TYPE::FIRE)}: {damageDice.GetFireDamage().ToString() ?: "N/A"}");
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"{Upper(DAMAGE_TYPE::COLD)}: {damageDice.GetColdDamage().ToString() ?: "N/A"}");
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"{Upper(DAMAGE_TYPE::POISON)}: {damageDice.GetPoisonDamage().ToString() ?: "N/A"}");
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"{Upper(DAMAGE_TYPE::ENERGY)}: {damageDice.GetEnergyDamage().ToString() ?: "N/A"}");

	var damage := attackInfo.GetDamage();
	var slashDamage := attackInfo.GetAttacker().GetCombatTraits().GetPosture() == DAMAGE_TYPE::SLASH ? damageDice.GetSlashDamage().Roll().GetResult() : 0;
	var piercingDamage := attackInfo.GetAttacker().GetCombatTraits().GetPosture() == DAMAGE_TYPE::PIERCING ? damageDice.GetPiercingDamage().Roll().GetResult() : 0;
	var bluntDamage := attackInfo.GetAttacker().GetCombatTraits().GetPosture() == DAMAGE_TYPE::BLUNT ? damageDice.GetBluntDamage().Roll().GetResult() : 0;
	var fireDamage := damageDice.GetFireDamage().Roll().GetResult() ?: 0;
	var coldDamage := damageDice.GetColdDamage().Roll().GetResult() ?: 0;
	var poisonDamage := damageDice.GetPoisonDamage().Roll().GetResult() ?: 0;
	var energyDamage := damageDice.GetEnergyDamage().Roll().GetResult() ?: 0;

	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, "=================");
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, "Dano rolado:");
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"{Upper(DAMAGE_TYPE::SLASH)}: {slashDamage}");
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"{Upper(DAMAGE_TYPE::PIERCING)}: {piercingDamage}");
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"{Upper(DAMAGE_TYPE::BLUNT)}: {bluntDamage}");
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"{Upper(DAMAGE_TYPE::FIRE)}: {fireDamage}");
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"{Upper(DAMAGE_TYPE::COLD)}: {coldDamage}");
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"{Upper(DAMAGE_TYPE::POISON)}: {poisonDamage}");
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"{Upper(DAMAGE_TYPE::ENERGY)}: {energyDamage}");
	
	damage.IncreaseSlashRawDamage(slashDamage);
	damage.IncreasePiercingRawDamage(piercingDamage);
	damage.IncreaseBluntRawDamage(bluntDamage);
	damage.IncreaseFireRawDamage(fireDamage);
	damage.IncreaseColdRawDamage(coldDamage);
	damage.IncreasePoisonRawDamage(poisonDamage);
	damage.IncreaseEnergyRawDamage(energyDamage);

	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, "=================");
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, "Modificadores de dano:");
	var damageMod := attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetDamageMod();
	damage.IncreaseSlashRawDamage(damageMod.GetSlashRawDamage());
	damage.IncreasePiercingRawDamage(damageMod.GetPiercingRawDamage());
	damage.IncreaseBluntRawDamage(damageMod.GetBluntRawDamage());
	damage.IncreaseFireRawDamage(damageMod.GetFireRawDamage());
	damage.IncreaseColdRawDamage(damageMod.GetColdRawDamage());
	damage.IncreasePoisonRawDamage(damageMod.GetPoisonRawDamage());
	damage.IncreaseEnergyRawDamage(damageMod.GetEnergyRawDamage());
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"{Upper(DAMAGE_TYPE::SLASH)}: {damageMod.GetSlashRawDamage()}");
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"{Upper(DAMAGE_TYPE::PIERCING)}: {damageMod.GetPiercingRawDamage()}");
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"{Upper(DAMAGE_TYPE::BLUNT)}: {damageMod.GetBluntRawDamage()}");
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"{Upper(DAMAGE_TYPE::FIRE)}: {damageMod.GetFireRawDamage()}");
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"{Upper(DAMAGE_TYPE::COLD)}: {damageMod.GetColdRawDamage()}");
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"{Upper(DAMAGE_TYPE::POISON)}: {damageMod.GetPoisonRawDamage()}");
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"{Upper(DAMAGE_TYPE::ENERGY)}: {damageMod.GetEnergyRawDamage()}");

	attackInfo.SetDamage(damage);
endfunction

function CalculateDefenderResist(byRef attackInfo)
	var armorsInBodyRegion := GetEquipsByAttackDirection(attackInfo);

	if (armorsInBodyRegion == array{})
		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, "Não há armadura na região atingida. Dano direto no corpo do defensor.");
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

	var damage := attackInfo.GetDamage();
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, "=================");
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, "Dano resistido:");
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"{Upper(DAMAGE_TYPE::SLASH)}: {CInt(damage.GetSlashRawDamage() - damage.GetSlashBaseDamage())}");
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"{Upper(DAMAGE_TYPE::PIERCING)}: {CInt(damage.GetPiercingRawDamage() - damage.GetPiercingBaseDamage())}");
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"{Upper(DAMAGE_TYPE::BLUNT)}: {CInt(damage.GetBluntRawDamage() - damage.GetBluntBaseDamage())}");
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"{Upper(DAMAGE_TYPE::FIRE)}: {CInt(damage.GetFireRawDamage() - damage.GetFireBaseDamage())}");
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"{Upper(DAMAGE_TYPE::COLD)}: {CInt(damage.GetColdRawDamage() - damage.GetColdBaseDamage())}");
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"{Upper(DAMAGE_TYPE::POISON)}: {CInt(damage.GetPoisonRawDamage() - damage.GetPoisonBaseDamage())}");
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"{Upper(DAMAGE_TYPE::ENERGY)}: {CInt(damage.GetEnergyRawDamage() - damage.GetEnergyBaseDamage())}");
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"Dano a ser aplicado (após defesa): {attackInfo.GetDamage().ToString()}");
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
		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"Ordem dos equipamentos a serem atingidos: {equipNamesForDebug}");
	endif

	return orderedEquips;
endfunction

function CalculateAttackerCriticalHit(byRef attackInfo)
	attackInfo.SetCriticalHitChance(CalculateCriticalChance(attackInfo));
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

	if (attackInfo.GetDefender().HasAbility(COMBAT_ABILITY::MUMIFICACAO))
		Message("Seu crítico não surte efeito", FONTCOLOR_FAIL).SendSys(attackInfo.GetAttacker().GetObjRef());
		Message("Seu corpo evita o crítico", FONTCOLOR_SUCCESS).SendSys(attackInfo.GetDefender().GetObjRef());
		return 0;
	endif

	chance := CInt(
		CInt(attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetCriticalChance()) +	// Crítico base da arma
		CInt(attackInfo.GetAttacker().GetCriticalChance()) +						// Crítico extra do Char (base + mod)
		CInt(attackInfo.GetAttacker().GetSkill(skillUsedForCritical) / 20) -		// Crítico extra da Skill
	);

	if (attackInfo.GetAttacker().HasAbility(COMBAT_ABILITY::DESTRUIDOR))
		chance := chance * 2.0;
	endif

	if (attackInfo.GetDefender().HasAbility(COMBAT_ABILITY::LADRAO_DA_SORTE))
		Message("Seu alvo desviou do crítico!", FONTCOLOR_FAIL).SendSys(attackInfo.GetAttacker().GetObjRef());
		Message("Você desviou do crítico e usará isso contra seu oponente!", FONTCOLOR_SUCCESS).SendSys(attackInfo.GetDefender().GetObjRef());
		attackInfo.GetDefender().SetProperty("#nextcritical", true);
		return 0;
	endif

	if (attackInfo.GetAttacker().HasAbility(COMBAT_ABILITY::OPORTUNIDADE_CRITICA))
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

function ApplyCriticalDamage(byRef attackInfo)
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, "***DANO CRÍTICO***");

	// TODO: Refatorar isso daqui, tá MUITO cagado....
		// Jogar as coisas de pet pra uma função separada
	
	var weapon_scripts := array{};

	var npcCfg := CFGFile("combat", "npcdesc").SetElementName(attackInfo.GetAttacker().GetTemplate());
	var tamingCfg := CFGFile("taming", "taming").SetElementName(attackInfo.GetAttacker().GetObjRef().GetProp("Critic"));
	var weaponCfg := CFGFile("*", "itemdesc").SetElementName(attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetObjType());

	var pet_instinto := attackInfo.GetAttacker().GetObjRef().GetProp("Group_savage"); // animal tamado com a hab Instinto Selvagem para adicionad criticos diferentes
	var pet_selvagem := attackInfo.GetAttacker().GetObjRef().GetProp("Selvagem"); // verifica se é animal selvagem - nunca tamado, para dar dano critico
	var pet_txt := tamingCfg.SetKeyName("Text").GetKeyValue();

	if (pet_instinto || pet_selvagem) // Se for pet
		weapon_scripts := tamingCfg.SetKeyName("CriticalHitScripts").GetKeyValue();
		Message(pet_txt[1]).SendAbove(attackInfo.GetAttacker().GetObjRef());
	elseif (attackInfo.GetAttacker().GetEquipBySlot(WEAPON))
		weapon_scripts := weaponCfg.SetKeyName("CriticalHitScripts").GetKeyValue();
	elseif (attackInfo.GetAttacker().IsNPC())
		weapon_scripts := npcCfg.SetKeyName("CriticalHitScripts").GetKeyValue();
	endif

	Message("CRÍTICO", FONTCOLOR_RED).SendEmote(attackInfo.GetAttacker().GetObjRef());
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
				Message("*Attacker* Weapon script error starting [" + hitscript + "] :" + script.errortext, FONTCOLOR_FAIL).SendSys(attackInfo.GetAttacker().GetObjRef());
			endif

			SleepMS(2);
		endforeach
	endforeach

	return true;
endfunction

function ApplyCombatDamage(byRef attackInfo)
	var slash, piercing, blunt, fire, cold, poison, energy;
	slash 	 := attackInfo.GetDamage().GetSlashBaseDamage();
	piercing := attackInfo.GetDamage().GetPiercingBaseDamage();
	blunt 	 := attackInfo.GetDamage().GetBluntBaseDamage();
	fire 	 := attackInfo.GetDamage().GetFireBaseDamage();
	cold 	 := attackInfo.GetDamage().GetColdBaseDamage();
	poison 	 := attackInfo.GetDamage().GetPoisonBaseDamage();
	energy 	 := attackInfo.GetDamage().GetEnergyBaseDamage();

	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, "=================");
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"Dano aplicado em: {attackInfo.GetDefender().GetName()}");
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"{attackInfo.GetDamage().ToString()}");

	if (slash > 0)
		ApplyRawDamage(attackInfo.GetDefender().GetObjRef(), CInt(slash));
		DisplayDamage(attackInfo.GetDefender().GetObjRef(), CInt(slash), attackInfo.GetAttacker().GetObjRef());
		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"Slash: {CInt(slash)}");
	endif

	if (piercing > 0)
		ApplyRawDamage(attackInfo.GetDefender().GetObjRef(), CInt(piercing));
		DisplayDamage(attackInfo.GetDefender().GetObjRef(), CInt(piercing), attackInfo.GetAttacker().GetObjRef());
		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"Piercing: {CInt(piercing)}");
	endif

	if (blunt > 0)
		ApplyRawDamage(attackInfo.GetDefender().GetObjRef(), CInt(blunt));
		DisplayDamage(attackInfo.GetDefender().GetObjRef(), CInt(blunt), attackInfo.GetAttacker().GetObjRef());
		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"Blunt: {CInt(blunt)}");
	endif

	if (fire > 0)
		ApplyRawDamage(attackInfo.GetDefender().GetObjRef(), CInt(fire));
		DisplayDamage(attackInfo.GetDefender().GetObjRef(), CInt(fire), attackInfo.GetAttacker().GetObjRef());
		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"Fire: {CInt(fire)}");
	endif

	if (cold > 0)
		ApplyRawDamage(attackInfo.GetDefender().GetObjRef(), CInt(cold));
		DisplayDamage(attackInfo.GetDefender().GetObjRef(), CInt(cold), attackInfo.GetAttacker().GetObjRef());
		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"Cold: {CInt(cold)}");
	endif

	if (poison > 0)
		ApplyRawDamage(attackInfo.GetDefender().GetObjRef(), CInt(poison));
		DisplayDamage(attackInfo.GetDefender().GetObjRef(), CInt(poison), attackInfo.GetAttacker().GetObjRef());
		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"Poison: {CInt(poison)}");
	endif

	if (energy > 0)
		ApplyRawDamage(attackInfo.GetDefender().GetObjRef(), CInt(energy));
		DisplayDamage(attackInfo.GetDefender().GetObjRef(), CInt(energy), attackInfo.GetAttacker().GetObjRef());
		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"Energy: {CInt(energy)}");
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
