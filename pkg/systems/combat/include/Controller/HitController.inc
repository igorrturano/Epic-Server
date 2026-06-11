/* $Id$
 *
 * Purpose
 * Criado para organizar a lógica de cálculo de chance de acerto do combate
 */
// include "CombatAbilityController";
include ":wrappers:Objects/Model/WeaponWrapper";
include ":abilities:models/Ability";
include ":tn:cooldown";
include ":charactercreation:subclasses";
include "include/packets";
include "include/dungeons";

/**
 * Calcula a chance de acerto de um ataque baseado nos atributos do atacante e defensor,
 * habilidades e vários modificadores de combate.
 * 
 * @param attackInfo Referência ao objeto de informações do ataque contendo dados do atacante e defensor
 * @return Valor inteiro representando a porcentagem de chance de acerto (5-100)
 * 
 * Os cálculos são realizados em float para precisão, então convertidos para inteiro para rolagem de dados.
 */
function CalculateHitChance(byRef attackInfo)
	// Os cálculos são feitos em float, pra evitar perda de precisão em divisões
	// O retorno deve ser feito em int, pra facilitar a rolagem do dado
	// Condições especiais (que já retornam valor fixo) aqui em cima, pra economizar processamento
	// Jogadores que não estão em warmode devem ser fáceis de acertar (não podem se defender adequadamente)
	if (!attackInfo.GetDefender().IsNPC())
		if (!attackInfo.GetDefender().IsWarMode())
			return 100;
		endif
	endif

	if (attackInfo.GetAttacker().GetObjRef().GetProp("#GuaranteedHit"))
		attackInfo.GetAttacker().GetObjRef().EraseProp("#GuaranteedHit");
		return 100;
	endif

	if (attackInfo.GetWeapon().GetAttribute() == RANGED && GetCooldown(attackInfo.GetDefender().GetObjRef(), "Desviar Disparos") > 0)
		Message("*desviar disparos*").SendAbove(attackInfo.GetDefender().GetObjRef());
		return 0;
	endif

	// As condições que precisam de cálculo, começam aqui
	var attackerSkill := _GetSkillForCombat(attackInfo.GetAttacker());
	var defenderSkill := _GetSkillForCombat(attackInfo.GetDefender());
	var hit_chance := 50.0 + ((attackerSkill - defenderSkill) / 1.5);
	CalculateWeaponHitModifier(hit_chance, attackInfo);
	hit_chance += attackInfo.GetAttacker().GetObjRef().hitchance_mod - attackInfo.GetDefender().GetObjRef().evasionchance_mod;
	// They need to be here because they change dynamically
	CalculateAbilityHitModifier(hit_chance, attackInfo);
	CalculateEntropyHitModifier(hit_chance, attackInfo);
	CalculateCooldownHitModifier(hit_chance, attackInfo);
	CalculateDuelHitModifier(hit_chance, attackInfo);

	if (!attackInfo.GetAttacker().IsNPC())
		DarknessHitModifier(attackInfo.GetAttacker().GetObjRef(), attackInfo.GetDefender().GetObjRef(), hit_chance);
	endif

	if (attackInfo.GetDefender().GetTemplate() == ":epicai:spiderling")
		hit_chance -= 20.0;
	endif

	if (hit_chance < 5.0)
		hit_chance := 5.0;
	elseif (hit_chance > 100.0)
		hit_chance := 100.0;
	endif

	return CInt(hit_chance); // Devolve Int, pra facilitar a rolagem do dado
endfunction

function _GetSkillForCombat(byRef mobile)
	var skill;

	if (mobile.IsNPC()) // Se for NPC, a skill é wrestling
		skill := mobile.GetSkill(WRESTLING);
	else // Se não for, o bicho pega....
		var attribute := mobile.GetEquipBySlot(WEAPON).GetAttribute();

		if ((mobile.GetObjRef().GetProp("metamorfose") != error) ||
			(mobile.GetObjRef().GetProp("pilotando") != error)) // Se é Metamorfo ou Piloto de Golem, a skill tá na prop
			skill := mobile.GetObjRef().GetProp("skill_a");
		else
			skill := mobile.GetSkill(attribute);
		endif

		if (mobile.IsMounted())
			var hasTrait := mobile.GetObjRef().GetProp("chardata").trait == "Cavalaria de Escaramuça";
			var hasAbility := mobile.HasAbility(ABILITY_ID::MOUNTED_COMBAT);

			if (!hasTrait && !hasAbility)
				skill := skill / 2; // 50% penalty
			elseif (!hasTrait || !hasAbility)
				skill := CInt(skill * 75 / 100); // 25% penalty
			endif
			// Both trait + ability = no penalty
		endif
	endif

	return skill;
endfunction

function CalculateDualWieldPenalty(attackInfo)
	attackInfo.GetAttacker().GetSkill(attackInfo.GetWeapon().GetAttribute());

	var penalty := (100 - attackInfo.GetAttacker().GetSkill(attackInfo.GetWeapon().GetAttribute())) / 2;

	if (penalty < 10)
		penalty := 10;
	endif
	
	return penalty;
endfunction

function DarknessHitModifier(attacker, defender, byref hit_chance)
    var chardata := GetObjProperty(attacker, "chardata");
    var attacker_race := chardata.raca;
    var attacker_subclasse := (TypeOfInt(chardata.subclasse) == OT_ARRAY ? chardata.subclasse[1] : "");
    var in_darkness := InDarkness(attacker);
	var metamorfose := GetObjProperty(attacker, "metamorfose");
	if (metamorfose)
		return;
	endif

	// Check for true darkness from Shadow Dominion
    if (GetCooldown(defender, "true_darkness") > 0)
        if (attacker_race != DROW)
            hit_chance -= 30; // Heavy penalty for attacking someone in true darkness
            SendSysMessageEx(attacker, "Você luta contra as trevas sagradas!", SSM_FAIL);
        endif
        return; // True darkness overrides normal darkness checks
    endif

    if (in_darkness && attacker_race != DROW)
        if (!HasLightNearby(defender))
			if (GetCooldown(attacker, "nightvision") > 0)
            	hit_chance -= 10;
            	// SendSysMessageEx(attacker, "A escuridão faz você lutar com certa dificuldade.", SSM_FAIL);
			else
				hit_chance -= 20;
				SendSysMessageEx(attacker, "A escuridão faz você lutar com muita dificuldade.", SSM_FAIL);
			endif
			// Arqueiros têm visão treinada para combate noturno: +10 que compensa parcialmente.
			// Empilha com nightvision (-10 + 10 = 0) e sozinho reduz -20 para -10. Só se aplica
			// dentro do ramo de penalidade, então nunca vira bônus em ambiente iluminado.
			if (attacker_subclasse == SUBCLASS::ARQUEIRO)
				hit_chance += 10;
			endif
        endif
    elseif (!in_darkness && attacker_race == DROW)
		if (GetCooldown(attacker, "nightvision") > 0)
			hit_chance -= 10;
			// SendSysMessageEx(attacker, "A claridade faz você lutar com certa dificuldade.", SSM_FAIL);
		else
			hit_chance -= 20;
			SendSysMessageEx(attacker, "A claridade faz você lutar com muita dificuldade.", SSM_FAIL);
		endif
    endif
endfunction

function HasLightNearby(defender)
	if (GetCooldown(defender, "fire_arrow_lit"))
		return 1;
	endif

	const SEARCH_RANGE := 2;

	// Mais barato: checa tochas/lanternas equipadas em players proximos (caso mais comum)
	var nearby_mobiles := ListMobilesNearLocation(defender.x, defender.y, defender.z, SEARCH_RANGE, defender.realm);
	var isEquippedLightSource := {0x0a12, 0x0a22, 0x0a23, 0x0a24, 0x0a15, 0xC626, 0xC620, 0xC622, 0xC624, 0xC628};

	foreach mob in nearby_mobiles
		if (!mob.npctemplate)
			foreach item in (listequippeditems(mob))
				if (Cint(item.graphic) in isEquippedLightSource)
					return 1;
				endif
			endforeach
		endif
	endforeach

	// Medio: checa tiledata flag de light source em itens proximos
	var flag_lightsources := ListItemsNearLocationWithFlag(defender.x, defender.y, defender.z, SEARCH_RANGE, TILEDATA_FLAG_LIGHTSOURCE, defender.realm);
	if (flag_lightsources.size() > 0)
		return 1;
	endif

	// Pesado: checa statics e itens contra lista de light sources do config
	var cfg := ReadConfigFile(":combat:config/lightsources");
	var elem := cfg["LightSources"];
	var objtype_lightsources := array{};

	foreach light in GetConfigStringArray(elem, "LightSource")
		objtype_lightsources.append(light);
	endforeach

	var statics_lightsources := ListStaticsInBox(defender.x+SEARCH_RANGE, defender.y+SEARCH_RANGE, defender.z+5, defender.x-SEARCH_RANGE, defender.y-SEARCH_RANGE, defender.z-5, 0, defender.realm);
	foreach light in objtype_lightsources
		foreach static in statics_lightsources
			if (CInt(static.objtype) == CInt(light))
				return 1;
			endif
		endforeach
	endforeach

	var nearby_items := ListItemsNearLocation(defender.x, defender.y, defender.z, SEARCH_RANGE, defender.realm);
	foreach obj in nearby_items
		if (obj.objtype in objtype_lightsources)
			return 1;
		endif
	endforeach

	return 0;
endfunction

function CheckDistance(attacker, defender) // attacker: MobileWrapper, defender: MobileWrapper
	var attackerDistance := Distance(attacker.GetObjRef(), defender.GetObjRef());
	var weapon := attacker.GetEquipBySlot(WEAPON);

	if (attackerDistance < weapon.GetMinRange() || attackerDistance > weapon.GetMaxRange())
		return false;
	endif

	if (weapon.GetAttribute() != RANGED)
		var zDelta := Abs(attacker.GetObjRef().z - defender.GetObjRef().z);
		var zCap := (weapon.GetMaxRange() > 1) ? 6 : 5;
		if (zDelta > zCap)
			return false;
		endif
	endif

	return true;
endfunction

function CanBackStab(attacker) // attacker: MobileWrapper
	if (attacker.GetEquipBySlot(WEAPON).GetAttribute() == TWOHANDED ||
		attacker.GetEquipBySlot(WEAPON).GetAttribute() == RANGED)
		return false;
	endif

	return true;
endfunction

function ConsumeAmmo(byRef attackInfo)
	if (attackInfo.GetWeapon().isFireWeapon())
		var powderNeeded := GetPowderAmount(attackInfo.GetWeapon().GetCategory());

		var bandolier := attackInfo.GetAttacker().GetEquipByLayer(LAYER_WAIST).GetObjRef();
		var powder := FindObjtypeInContainer(bandolier, 0x9092); // Pólvora
		if (powder.amount < powderNeeded)
			Message("Você não tem pólvora suficiente na bandoleira.", Color(SYSTEM_COLOR::FAIL)).SendSys(attackInfo.GetAttacker().GetObjRef());
			return false;
		endif

		SubtractAmount(powder, powderNeeded);	
	endif
	
	return attackInfo.GetAmmo().GetItem().Consume(1);
endfunction

function GetPowderAmount(weaponCategory)
	case (weaponCategory)
		WEAPON_CATEGORY::CARABINE:
			return 2;
		WEAPON_CATEGORY::MUSKET:
			return 3;
		WEAPON_CATEGORY::HANDCANNON:
			return 5;
		WEAPON_CATEGORY::PISTOL:
		default:
			return 1;
	endcase
endfunction

function CalculateWeaponHitModifier(byref hit_chance, attackInfo)
	hit_chance += attackInfo.GetWeapon().GetAccuracyMod() * 2;
endfunction

function CalculateAbilityHitModifier(byref hit_chance, attackInfo)
	if (attackInfo.GetDefender().HasAbility(ABILITY_ID::MARTIAL_ARTIST))
		if (attackInfo.GetDefender().GetWeapon().GetAttribute() == WRESTLING)
			hit_chance -= CDbl(attackInfo.GetDefender().GetSkill(TACTICS) / 5);
		endif
	endif

	if (attackInfo.GetAttacker().HasAbility(ABILITY_ID::DANCA_DAS_SOMBRAS))
		hit_chance += CDbl(attackInfo.GetAttacker().GetSkill(ARCANA) / 5);
	endif

	if (attackInfo.GetAttacker().HasAbility(ABILITY_ID::MOVIMENTACAO_GRACIOSA))
		hit_chance += CDbl(attackInfo.GetAttacker().GetSkill(TACTICS) / 10);
	endif

	if (attackInfo.GetDefender().HasAbility(ABILITY_ID::MOVIMENTACAO_GRACIOSA))
		hit_chance -= CDbl(attackInfo.GetDefender().GetSkill(TACTICS) / 10);
	endif

	// Postura de Duelo: dual wield attacker bonus
	if (attackInfo.GetAttacker().HasAbility(ABILITY_ID::POSTURA_DE_DUELO))
		if (Upper(attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetAttribute()) == Upper(DUALWIELD))
			hit_chance += CDbl(attackInfo.GetAttacker().GetSkill(TACTICS) / 10);
		endif
	endif

	// Postura de Duelo: empty off-hand defender evasion
	if (attackInfo.GetDefender().HasAbility(ABILITY_ID::POSTURA_DE_DUELO))
		if (!GetEquipmentByLayer(attackInfo.GetDefender().GetObjRef(), LAYER_HAND2))
			hit_chance -= CDbl(attackInfo.GetDefender().GetSkill(TACTICS) / 10);
		endif
	endif

	if (attackInfo.GetAttacker().HasAbility(ABILITY_ID::MANCHA_DA_CORRUPCAO))
		hit_chance += CDbl(attackInfo.GetAttacker().GetSkill(THEOLOGY) / 10);
	endif

	if (attackInfo.GetDefender().HasAbility(ABILITY_ID::MANCHA_DA_CORRUPCAO))
		hit_chance -= CDbl(attackInfo.GetDefender().GetSkill(THEOLOGY) / 10);
	endif

	if (attackInfo.GetAttacker().HasAbility(ABILITY_ID::ARCANE_MASTERY))
		hit_chance += CDbl(attackInfo.GetAttacker().GetSkill(ARCANA) / 10);
	endif

	if (attackInfo.GetAttacker().HasAbility(ABILITY_ID::AURA_DE_FE))
		hit_chance += CDbl(attackInfo.GetAttacker().GetObjRef().party.members.size());
	endif
endfunction

function CalculateEntropyHitModifier(byref hit_chance, attackInfo)
	// Entropy bonus: negative entropy gives +1 to +10 bonus/penalty
	var attacker_entropy := attackInfo.GetAttacker().GetEntropy();
	if (attacker_entropy <= -2000)
		var entropy_bonus := Cdbl(abs(attacker_entropy) / 2000);
		if (entropy_bonus > 10.0) entropy_bonus := 10.0; endif
		hit_chance += entropy_bonus;
	endif

	var defender_entropy := attackInfo.GetDefender().GetEntropy();
	if (defender_entropy <= -2000)
		var entropy_penalty := Cdbl(abs(defender_entropy) / 2000);
		if (entropy_penalty > 10.0) entropy_penalty := 10.0; endif
		hit_chance -= entropy_penalty;
	endif

	var globalEntropy := Cint(GetGlobalProperty("GlobalEntropy"));
	if (globalEntropy <= -60000 && attackInfo.GetAttacker().IsNPC())
		var npc_category := Lower(attackInfo.GetAttacker().GetEnvironment());
		if (npc_category == "void")
			hit_chance += 10.0;
		endif
	elseif (globalEntropy <= -40000)
		var npc_category := Lower(attackInfo.GetAttacker().GetEnvironment());
		if (npc_category == "demon")
			hit_chance += 10.0;
		endif
	elseif (globalEntropy <= -20000)
		var npc_category := Lower(attackInfo.GetAttacker().GetEnvironment());
		if (npc_category == "undead")
			hit_chance += 10.0;
		endif
	endif

endfunction

function CalculateCooldownHitModifier(byref hit_chance, attackInfo)
	if (GetCooldown(attackInfo.GetAttacker().GetObjRef(), "furia") && attackInfo.GetAttacker().HasAbility(ABILITY_ID::FURIA_INDOMAVEL))
		if (!attackInfo.GetDefender().HasAbility(ABILITY_ID::PRESENCA_DIVINA))
			hit_chance += 10.0;
		endif
	endif

	if (GetCooldown(attackInfo.GetAttacker().GetObjRef(), "desafioheroico") > 0)
		hit_chance -= CDbl(attackInfo.GetDefender().GetSkill(TACTICS) / 5);
	endif

	if (GetCooldown(attackInfo.GetAttacker().GetObjRef(), "defesapersistente") > 0)
		hit_chance -= CDbl(attackInfo.GetAttacker().GetSkill(PARRY) / 5);
	endif

	if (GetCooldown(attackInfo.GetDefender().GetObjRef(), "defesapersistente") > 0)
		hit_chance -= CDbl(attackInfo.GetDefender().GetSkill(PARRY) / 5);
	endif

	if (GetCooldown(attackInfo.GetDefender().GetObjRef(), "resistenciaheroica") > 0)
		var hostiles := ListHostiles(attackInfo.GetDefender().GetObjRef(), 3);

		if (hostiles.size() >= 5)
			hit_chance -= 15.0;
		elseif (hostiles.size() >= 3)
			hit_chance -= 10.0;
		elseif (hostiles.size() >= 1)
			hit_chance -= 5.0;
		endif
	endif
endfunction

// Penalty for hitting a target other than your duel challenger.
// #DuelChallenge is set by :spells:paladin/forcarduelo and cleared lazily here on expiry.
// Kept as a prop read (instead of hitchance_mod) because the penalty is target-conditional.
function CalculateDuelHitModifier(byref hit_chance, attackInfo)
	var attacker := attackInfo.GetAttacker();
	var attackerRef := attacker.GetObjRef();
	var challenged_by := attackerRef.GetProp("#DuelChallenge");

	if (!challenged_by || attacker.IsNPC())
		return;
	endif

	if (challenged_by.time < ReadGameClock())
		attackerRef.EraseProp("#DuelChallenge");
		return;
	endif

	var challenger := SystemFindObjectBySerial(challenged_by.challenger);
	if (!challenger || !challenger.connected)
		attackerRef.EraseProp("#DuelChallenge");
		return;
	endif

	if (attackInfo.GetDefender().GetSerial() == challenger.serial)
		return;
	endif

	hit_chance -= 10.0;
	if (RandomInt(101) < 20)
		Message("Você está em duelo! Seus ataques são menos efetivos contra outros alvos.", Color(SYSTEM_COLOR::FAIL)).SendSys(attackerRef);
	endif
endfunction