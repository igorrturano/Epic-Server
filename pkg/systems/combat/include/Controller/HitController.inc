/* $Id$
 *
 * Purpose
 * Criado para organizar a lógica de cálculo de chance de acerto do combate
 */

// include "combat";
include "AbilitiesController";
include ":combat:Model/CombatAbility";
include ":crafting:/include/caracteristicasFabricacao";

function CalculateHitChance(byRef attackInfo)
	// Condições especiais (que já retornam valor fixo) aqui em cima, pra economizar processamento
	if (!attackInfo.GetAttacker().IsNPC())
		if (!attackInfo.GetDefender().IsWarMode())
			return 100.0;
		endif
	endif

	if (attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetAttribute() == RANGED && GetCooldown(attackInfo.GetDefender().GetMobRef(), "Desviar Disparos") > 0)
		Message("*desviar disparos*").SendAbove(attackInfo.GetDefender().GetMobRef());
		return 0;
	endif

	if (attackInfo.GetAttacker().GetMobRef().GetProp("#attackfail"))
		attackInfo.GetAttacker().GetMobRef().EraseProp("#attackfail");
		return 0;
	endif

	if (attackInfo.GetDefender().GetMobRef().GetProp("#defensefail"))
		attackInfo.GetDefender().GetMobRef().EraseProp("#defensefail");
		return 100;
	endif

	if (GetCooldown(attackInfo.GetAttacker().GetMobRef(), "ataquetotal"))
		return 100;
	endif
	if (GetCooldown(attackInfo.GetDefender().GetMobRef(), "ataquetotal"))
		return 100;
	endif

	// As condições que precisam de cálculo, começam aqui
	var attackerSkill := _GetSkillForCombat(attackInfo.GetAttacker());
	var defenderSkill := _GetSkillForCombat(attackInfo.GetDefender());
	var hit_chance := 50.0 + ((attackerSkill - defenderSkill) / 1.5);

	hit_chance += attackInfo.GetAttacker().GetMobRef().hitchance_mod - attackInfo.GetDefender().GetMobRef().evasionchance_mod;
	hit_chance += CInt(attackInfo.GetAttacker().GetMobRef().GetProp("#attackbonus"));

	if (GetCooldown(attackInfo.GetDefender().GetMobRef(), "evasion_chance"))
		hit_chance -= 10;
	endif

	if (GetCooldown(attackInfo.GetAttacker().GetMobRef(), "blinded") > 0)
		hit_chance -= 20;
	endif

	if (GetCooldown(attackInfo.GetDefender().GetMobRef(), "exposed") > 0) // Defender is exposed, easier to hit
		hit_chance += 10;
	endif

	if (attackInfo.GetAttacker().HasAbility(COMBAT_ABILITY::POSTURA_OFENSIVA))
		hit_chance += 10;
	endif

	if (attackInfo.GetDefender().HasAbility(COMBAT_ABILITY::POSTURA_OFENSIVA))
		hit_chance += 10;
	endif

	if (attackInfo.GetAttacker().HasAbility(COMBAT_ABILITY::POSTURA_DEFENSIVA))
		hit_chance -= 10;
	endif

	if (attackInfo.GetDefender().HasAbility(COMBAT_ABILITY::POSTURA_DEFENSIVA))
		hit_chance -= 10;
	endif

	if (attackInfo.GetDefender().HasAbility(COMBAT_ABILITY::PRESERVAR_ORGAOS))
		hit_chance -= 10;
	endif

	if (GetCooldown(attackInfo.GetAttacker().GetMobRef(), "posturadecombate") > 0)
		hit_chance += 10;
	endif

	var challenged_by := attackInfo.GetAttacker().GetMobRef().GetProp("#DuelChallenge");
	if (challenged_by && !attackInfo.GetAttacker().IsNPC) // Only affects players
		if (challenged_by.time < ReadGameClock()) // Challenge expired
			attackInfo.GetAttacker().GetMobRef().EraseProp("#DuelChallenge");
		else
			var challenger := SystemFindObjectBySerial(challenged_by.challenger);
			if (challenger && challenger.connected)
				if (attackInfo.GetDefender().GetSerial() != challenger.serial) // If attacking someone other than the challenger
					hit_chance -= 10;
					if (RandomInt(100) < 20) // 20% chance to remind
						Message("Você está em duelo! Seus ataques são menos efetivos contra outros alvos.", FONTCOLOR_ERROR).SendSys(attackInfo.GetAttacker().GetMobRef());
					endif
				endif
			else
				attackInfo.GetAttacker().GetMobRef().EraseProp("#DuelChallenge"); // Clean up if challenger disconnected
			endif
		endif
	endif

	if (attackInfo.GetAttacker().HasAbility(COMBAT_ABILITY::AURA_DE_FE))
		hit_chance += attackInfo.GetAttacker().GetMobRef().party.members.size();
	endif

	if (attackInfo.GetAttacker().GetMobRef().GetProp("armapreferida") == attackInfo.GetAttacker().GetWeapon().GetObjType())
		hit_chance += 5;
	endif

	hit_chance += CInt(PegaValorFabricacao(attackInfo.GetAttacker().GetWeapon().GetItemRef(), CARACTERISTICA_FABRICACAO_PRECISAO)) * 3;

	if (GetCooldown(attackInfo.GetAttacker().GetMobRef(), "furia") && attackInfo.GetAttacker().HasAbility(COMBAT_ABILITY::FURIA_INDOMAVEL))
		if (!attackInfo.GetDefender().HasAbility("Fortitudo Impia"))
			hit_chance += 10;
		endif
	endif

	if (CInt(attackInfo.GetAttacker().GetMobRef().GetProp("#attackbonus"))) // TODO: Isso aqui não é pra estar aqui. Ver em qual função colocar.
		EraseObjProperty(attackInfo.GetAttacker().GetMobRef(), "#attackbonus");
	endif

	if (IsRaging(attackInfo.GetAttacker().GetMobRef()))
		if (attackInfo.GetDefender().HasAbility(COMBAT_ABILITY::ILUDIR_O_FRACO))
			hit_chance -= 10;
		endif
		
		if (GetCooldown(attackInfo.GetAttacker().GetMobRef(), "Ataques Enlouquecidos") > 0)
			hit_chance += 100;
		endif
	endif

	if (IsRaging(attackInfo.GetDefender().GetMobRef()))
		if (attackInfo.GetDefender().HasAbility(COMBAT_ABILITY::ESQUIVA_INSTINTIVA))
			hit_chance -= 10;
		endif

		if (GetCooldown(attackInfo.GetDefender().GetMobRef(), "Ataques Enlouquecidos") > 0)
			hit_chance += 100;
		endif
	endif

	if (GetCooldown(attackInfo.GetAttacker().GetMobRef(), "desafioheroico") > 0)
		hit_chance -= CInt(attackInfo.GetDefender().GetSkill(TACTICS) / 5);
	endif

	if (GetCooldown(attackInfo.GetAttacker().GetMobRef(), "defesapersistente") > 0)
		hit_chance -= CInt(attackInfo.GetAttacker().GetSkill(PARRY) / 5);
	endif

	if (GetCooldown(attackInfo.GetDefender().GetMobRef(), "defesapersistente") > 0)
		hit_chance -= CInt(attackInfo.GetDefender().GetSkill(PARRY) / 5);
	endif

	if (GetCooldown(attackInfo.GetDefender().GetMobRef(), "resistenciaheroica") > 0)
		var hostiles := ListHostiles(attackInfo.GetDefender().GetMobRef(), 3);

		if (hostiles.size() >= 5)
			hit_chance -= 15;
		elseif (hostiles.size() >= 3)
			hit_chance -= 10;
		elseif (hostiles.size() >= 1)
			hit_chance -= 5;
		endif
	endif

	if (attackInfo.GetAttacker().HasAbility(COMBAT_ABILITY::LOBO_SOLITARIO))
		if (ListHostiles(attackInfo.GetAttacker().GetMobRef(), 3).size() > 1)
			hit_chance += 15;
		endif
	endif

	var tiroCerteiro := CInt(attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetItemRef().GetProp("tirocerteiro"));
	if (tiroCerteiro > 0)
		tiroCerteiro--;

		if (tiroCerteiro == 0)
			attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetItemRef().EraseProp("tirocerteiro");
		else
			attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetItemRef().SetProp("tirocerteiro", tiroCerteiro);
		endif

		hit_chance += 100.0;
	endif

	if (attackInfo.GetDefender().HasAbility(COMBAT_ABILITY::BUDOKA))
		if (attackInfo.GetDefender().GetWeapon().GetAttribute() == WRESTLING)
			hit_chance -= attackInfo.GetDefender().GetSkill(TACTICS) / 5;
		endif
	endif

	if (attackInfo.GetAttacker().HasAbility(COMBAT_ABILITY::DANCA_DAS_SOMBRAS) && GetCooldown(attackInfo.GetDefender().GetMobRef(), "shadowdancer_blur") > 0)
		hit_chance += CInt(attackInfo.GetAttacker().GetSkill(ARCANA) / 5);
	endif

	if (attackInfo.GetAttacker().HasAbility(COMBAT_ABILITY::MOVIMENTACAO_GRACIOSA))
		hit_chance += CInt(attackInfo.GetAttacker().GetSkill(TACTICS) / 10);
	endif

	if (attackInfo.GetDefender().HasAbility(COMBAT_ABILITY::MOVIMENTACAO_GRACIOSA))
		hit_chance -= CInt(attackInfo.GetDefender().GetSkill(TACTICS) / 10);
	endif

	if (attackInfo.GetAttacker().HasAbility(COMBAT_ABILITY::MANCHA_DA_CORRUPCAO))
		hit_chance += CInt(attackInfo.GetAttacker().GetSkill(THEOLOGY) / 10);
	endif

	if (attackInfo.GetDefender().HasAbility(COMBAT_ABILITY::MANCHA_DA_CORRUPCAO))
		hit_chance -= CInt(attackInfo.GetDefender().GetSkill(THEOLOGY) / 10);
	endif

	if (attackInfo.GetAttacker().HasAbility("Oleo do Cacador"))
		hit_chance += CInt(attackInfo.GetAttacker().GetSkill(SCIENCE) / 5);
	endif

	if (!attackInfo.GetAttacker().IsNPC())
		DarknessHitModifier(attackInfo.GetAttacker().GetMobRef(), attackInfo.GetDefender().GetMobRef(), hit_chance);
	endif
	
	if (attackInfo.GetAttacker().GetMobRef().GetProp("#ataqueinesperado"))
		if (attackInfo.GetDefender().GetMobRef().GetProp("lastHit").serial == attackInfo.GetAttacker().GetSerial())
			hit_chance += 50;
			Message("*ataque inesperado*", FONTCOLOR_ERROR).SendAbove(attackInfo.GetAttacker().GetMobRef());
		endif
		attackInfo.GetAttacker().GetMobRef().EraseProp("#ataqueinesperado");
	endif

	if (attackInfo.GetAttacker().GetMobRef().GetProp("#bomcomeco"))
		var attackers := attackInfo.GetDefender().GetMobRef().GetProp("#bomcomecoserials");
		if (attackers == error) attackers := array{}; endif

		if (!(attackInfo.GetAttacker().GetSerial() in attackers))
			hit_chance += 50;
			attackers.append(attackInfo.GetAttacker().GetSerial());
			attackInfo.GetDefender().GetMobRef().SetProp("#bomcomecoserials", attackers);
			Message("Você começou bem.", FONTCOLOR_SUCCESS).SendSys(attackInfo.GetAttacker().GetMobRef());
		endif
		attackInfo.GetAttacker().GetMobRef().EraseProp("#bomcomeco");
	endif

	if (attackInfo.GetDefender().GetMobRef().GetProp("#mobilidade"))
		hit_chance -= 15;
		AP_ConsumeVital(attackInfo.GetDefender().GetMobRef(), STAMINA, 3);
	endif

	if (attackInfo.GetAttacker().GetMobRef().GetProp("#mobilidade"))
		hit_chance -= 15;
	endif

	if (attackInfo.GetDefender().GetNPCTemplate() == ":epicai:spiderling")
		hit_chance -= 20;
	endif

	attackInfo.GetAttacker().GetMobRef().EraseProp("insistencia");

	if (attackInfo.GetAttacker().HasAbility("Tirothan"))
		var mlore_bonus := CInt(AP_GetSkill(attacker, ARCANA) / 10);
		hit_chance += mlore_bonus;
	endif

	if (hit_chance < 5.0)
		hit_chance := 5.0;
	elseif (hit_chance > 100.0)
		hit_chance := 100.0;
	endif

	return hit_chance;
endfunction

function _GetSkillForCombat(mobile)
	var skill;

	if (mobile.IsNPC()) // Se for NPC, a skill é wrestling
		skill := mobile.GetSkill(WRESTLING);
	else // Se não for, o bicho pega....
		var attribute := mobile.GetEquipBySlot(WEAPON).GetAttribute();

		if ((mobile.GetMobRef().GetProp("metamorfose") != error) ||
			(mobile.GetMobRef().GetProp("pilotando") != error)) // Se é Metamorfo ou Piloto de Golem, a skill tá na prop
			skill := mobile.GetMobRef().GetProp("skill_a");
		elseif (attribute == WRESTLING) // Se é melee desarmado, pega a maior entre as skills de melee
			skill := Max(array{
				mobile.GetSkill(ONEHANDED),
				mobile.GetSkill(TWOHANDED),
				mobile.GetSkill(DUALWIELD)
			});
		else
			skill := mobile.GetSkill(attribute);
		endif

		if (mobile.IsMounted() && // Se estiver montado
			mobile.GetMobRef().GetProp("chardata").trait != "Cavalaria de Escaramuça") // E não tiver hab de cavalaria
			skill := skill / 2;
		endif
	endif

	return skill;
endfunction

function CalculateDualWieldPenalty(attackInfo)
	var attacker := attackInfo.GetAttacker();

	attackInfo.GetAttacker().GetSkill(attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetAttribute());

	var penalty := (100 - attackInfo.GetAttacker().GetSkill(attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetAttribute())) / 2;

	if (penalty < 10)
		penalty := 10;
	endif
	
	return penalty;
endfunction

function DarknessHitModifier(attacker, defender, byref hit_chance)
    var attacker_race := GetObjProperty(attacker, "chardata").raca;
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
            	SendSysMessageEx(attacker, "Você luta com dificuldade.", SSM_FAIL);
			else
				hit_chance -= 20;
				SendSysMessageEx(attacker, "Você luta com muita dificuldade.", SSM_FAIL);
			endif
        endif
    elseif (!in_darkness && attacker_race == DROW)
		SendSysMessageEx(attacker, "Você luta com dificuldade.", SSM_FAIL);
		if (GetCooldown(attacker, "lightvision") > 0)
			hit_chance -= 10;
			SendSysMessageEx(attacker, "Você luta com dificuldade.", SSM_FAIL);
		else
			hit_chance -= 20;
			SendSysMessageEx(attacker, "Você luta com muita dificuldade.", SSM_FAIL);
		endif
    endif
endfunction

function HasLightNearby(defender)
    const SEARCH_RANGE := 2;
    
    // Check for items with light flag
    var flag_lightsources := ListItemsNearLocationWithFlag(defender.x, defender.y, defender.z, SEARCH_RANGE, TILEDATA_FLAG_LIGHTSOURCE, defender.realm);
    if (flag_lightsources.size() > 0)
        return 1;
    endif

	var statics_lightsources := ListStaticsInBox( defender.x+SEARCH_RANGE, defender.y+SEARCH_RANGE, defender.z+5, defender.x-SEARCH_RANGE, defender.y-SEARCH_RANGE, defender.z-5, 0, defender.realm);
    // Check for specific light source objects
	UnloadConfigFile(":combat:config/lightsources");
    var cfg := ReadConfigFile(":combat:config/lightsources");
    var elem := cfg["LightSources"];
    var objtype_lightsources := array{};

	foreach light in GetConfigStringArray(elem, "LightSource")
        objtype_lightsources.append(light);
	endforeach

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

	var nearby_mobiles := ListMobilesNearLocation(defender.x, defender.y, defender.z, SEARCH_RANGE, defender.realm);
	var isEquippedLightSource := {0x0a12, 0x0a22, 0x0a23, 0x0a24, 0x0a15, 0xC626, 0xC620, 0xC622, 0xC628}; // tocha e lanterna acesas

	foreach mob in nearby_mobiles
		if (!mob.npctemplate)
			foreach item in (listequippeditems(mob))
				if (Cint(item.graphic) in isEquippedLightSource)
					return 1;
				endif
			endforeach
		endif
	endforeach
    
	return 0;
endfunction

function DistanceChecks(attacker, defender)
	var cur_range := Distance(attacker, defender);
	var wpn_a_elem2 := GetWpnCfgInfo(attacker, attacker.weapon);
	var weapon_max_range := wpn_a_elem2.MaxRange;
	var weapon_min_range := wpn_a_elem2.MinRange;
	if ( wpn_a_elem2.MaxRange == error )
		return 1;
	endif
	
	if ( cur_range > weapon_max_range )
		return 0;
	elseif ( cur_range < weapon_min_range )
		return 0;
	else
		return 1;
	endif
endfunction

function CanBackStab(who, weapon)
	var can_back_stab := 0;

	if ((weapon.attribute) != "Two_Handed")
		can_back_stab := 1;
	endif

	if (!can_back_stab)
		Message("Esta arma não é apropriada para um ataque furtivo.").SendSys(who);
		return 0;
	endif

	return can_back_stab;
endfunction

function AmmoCheck(attacker, defender)
	var attackerWeapon := EquipWrapperWeapon(attacker.weapon);

	if (!attackerWeapon.GetTemplate().ProjectileType)
		return 1;
	endif
	var arcane_bows := array{0xFA6D, 0XF997};

	if (!GetCooldown(attacker, "arcane_bow") && !(attackerWeapon.GetObjType() in arcane_bows) && !attackerWeapon.GetTemplate().ProjectileType)
		return 1;
	endif
	var has_ammo := 0;

	var aljavas := array{0x791c, 0x2fb7};
	var aljava := GetEquipmentByLayer(attacker, 0x14);

		if (aljava.objtype in aljavas)
			aljava := GetEquipmentByLayer(attacker, 0x14);
		else
			aljava := GetEquipmentByLayer(attacker, 0xc);
		endif
	var ammo := GetObjProperty(attacker.weapon, "ammo");
	if (!ammo)
		Message("Você não definiu o tipo de flecha que vai usar. [clique 2x no arco e na flecha]").SendSys(attacker);
		return 0;
	endif

	var in_aljava := 0;
	foreach arrow in (EnumerateItemsInContainer(aljava))
		if (arrow.serial == ammo)
			in_aljava := 1;
		endif
	endforeach
	if (in_aljava == 0)
		return 0;
	endif

	if ( SubtractAmount( ammo, 1 ) )
		var cfg:=ReadConfigFile(":woodworking:itemdesc");
		var elem := FindConfigElem(cfg, GetObjType( ammo ));
		var ammo_color := elem.color;
		PlayMovingEffectXYZHued(attacker, defender, !attackerWeapon.GetTemplate().ProjectileType, 10, 10, 0, ammo_color);
		return 1;
	else
		return 0;
	endif

endfunction

function GetParryChance(byRef attackInfo)
	// Verifica se o atacante já pode ser bloqueado novamente.
	if (GetCooldown(attackInfo.GetAttacker().GetMobRef(), "parrydelay") > 0) return 0; endif

	// var usingWeapon := 0;
	// var parryBonus := 0;
	// var shield := defender.shield;
	// var shieldAttr := shield.getAttrBase();
	var fabricacaoApararBonus := PegaValorFabricacao(shield, CARACTERISTICA_FABRICACAO_BONUSAPARAR);
	var fabricacaoApararAtraso := PegaValorFabricacao(shield, CARACTERISTICA_FABRICACAO_ATRASOAPARAR);

	var blocker;
	// Verifica se o defensor está sob efeito da a hab esudo do martir
	var wall := attackInfo.GetDefender().GetMobRef().party.getProp("BarreiraDeEscudos");
	var wallChance := 0;
	if (wall)
		blocker := SystemFindObjectBySerial(wall.blocker);
		if (GetCooldown(blocker, "BarreiraDeEscudos") > 0)
			if (Distance(attackInfo.GetDefender().GetMobRef(), blocker) < 2)
				wallChance := CInt(wall.chance);
			endif
		else
			attackInfo.GetDefender().GetMobRef().party.eraseProp("BarreiraDeEscudos");
			blocker := 0;
		endif
	endif

	// Verifica se vai tentar bloquear usando uma armda
	if (!(shield.IsA(POLCLASS_ARMOR)))
		usingweapon := 1;
		shield := defender.weapon;
		// Não bloqueia se estivem sem arma equipada ou utilizando arma de Ranged (Arcos e bestas)
		if (shield.objtype == 0x1F020 || shield.Attribute == RANGED)
			return 0;
		endif
	endif

	parryBonus := attackInfo.GetDefender().GetEquipBySlot(SHIELD).GetItemRef().GetProp("parry_mod_percentual");
	if (parryBonus)
		parryBonus := 1.0 + parryBonus;
	else
		parryBonus := 0;
	endif
	// Calcula o Bonus basico de parry do escudo, Bonus de parry específico da CProp do escudo, Bonus de parry especifico da CProp do personagem
	// parryBonus := CInt(parryBonus * (CInt((item_cfg[shield.objtype]).parrybonus) + CInt(GetObjProperty(shield, "parrybonus")))) + CInt(GetObjProperty(defender, "parrybonus"));
	parryBonus := CInt(
		CInt(shieldAttr.ParryBonus) +					// Parry Bônus Básico do escudo
		CInt(GetObjProperty(shield, "parrybonus")) +	// Parry Bônus Extra do escudo
		Cint(fabricacaoApararBonus) +					// Parry Bônus Extra da Fabricação do escudo
		CInt(GetObjProperty(defender, "parrybonus"))	// Parry Bônus Extra do defensor
	);

	// Calcula o Bonus temporário de parry, se houver
	var tempParryBonus := CInt(getObjProperty(defender, "#tempparrybonus"));
	if (tempParryBonus)
		parryBonus := parryBonus + tempParryBonus;
		EraseObjPRoperty(defender, "#tempparrybonus");
	endif

	var skill := CInt(AP_GetSkill(defender, PARRY));

	if (TemHabilidade(defender, "Reflexo Estrategico"))
		skill := CInt(AP_GetSkill(defender, TACTICS));
	endif

	// Aplica delay básico do escudo, reduz delay com base na skill e soma fator de balanceamento
	// var parryDelay := CInt((item_cfg[shield.objtype]).parrydelay) - CInt(GetObjProperty(shield, "parrydelay_mod"))  - CInt(skill/20) + 8;
	var parryDelay := CInt(
		-(CInt(shieldAttr.ParryDelay)) +					// Parry Delay Básico do escudo
		-(CInt(GetObjProperty(shield, "parrydelay_mod"))) +	// Parry Delay Extra do escudo
		-(Cint(fabricacaoApararAtraso)) +					// Parry Delay Extra da fabricação do escudo
		-(CInt(skill / 20)) +								// Parry Delay Bônus de acordo com a skill
		8 													// Fator de balanceamento
	);

	// Faz com que os bloqueios garantidos por cauda de habilidade resetem o delay de bloqueio
	if (tempParryBonus == 100)
		parryDelay := 0;
	endif

	// Aumenta a frequência em que é possível bloquear o mesmo atacante quando o defensor está sob efeito de habilidade Inspirar Defesas
	if (TemHabilidade(defender.party.leader, "Inspirar Defesas"))
		parryDelay := parryDelay - 1;
	endif

	// Calcula o bonus de bloqueio com base no parry usando escudo
	var parry_chance := CInt(skill / 3);

	// Aplica penalidade de chance de bloqueio se estiver usando apenas arma
	if (usingweapon && !TemHabilidade(defender, "Tabibito no Michi"))
		parry_chance := CInt(parry_chance * 0.7);
	endif

	if (usingweapon && TemHabilidade(defender, "Tabibito no Michi"))
		parry_chance := CInt(parry_chance * 1.3);
	endif
	// Calcula a chance TOTAL de bloqueio
	parry_chance := parry_chance + parryBonus;

	// A habilidade do companheiro de party do defensor tentar bloquear em seu lugar só ativa se a chance de bloqueio dele for melhor que do defensor
	if (wallChance > parry_chance)
		parry_chance := wallChance;
	endif

	// Diminui a chance de bloqueio se estiver usando foices
	if ((attacker.weapon.graphic == 0x26C4) || (attacker.weapon.graphic == 0x26C5)) // foice grande e foice pequena
		parry_chance := parry_chance * 0.7; // diminui 30% da chance
	endif

	// Aplica o efeito da habilidade que deixa o defensor bloquear sempre
	if (CheckHab_BarreiraIndestrutivel(attacker, defender))
		parry_chance := 100;
		parryDelay := 2;
	endif

	// Calcula se defendeu ou não
	var defendeu := 0;
	if (RandomInt(100) < parry_chance)
		// Verifica habilidade Defesas Gloriosas e, se cabível, aplica seu efeito
		defendeu := 1;

		// Pike desarma defensor que bloquear sem escudo
		if ((attacker.weapon.graphic == 0x26BE) && (usingWeapon))
			var defweapon := defender.weapon;
			if (defweapon.objtype != 0x1F020)
				var x := defender.x + RandomInt(2) + 1;
				var y := defender.y + RandomInt(2) + 1;
				MoveObjectToLocation(
					defweapon,
					x,
					y,
					GetWorldHeight(x, y),
					attacker.realm,
					MOVEOBJECT_FORCELOCATION
				);
			endif
		endif

		CheckHab_KenNoTatsujin(attacker, defender);
	endif

	// sobe a skill
	if (attacker.npctemplate && defender.connected)
		if (defendeu)
			SkillCheck(defender, PARRY, -1);
		else
			if (RandomInt(5) == 1)
				SkillCheck(defender, PARRY, -1);
			endif
		endif
	endif

	if (!defendeu)
		return 0;
	endif

	// Exibe o efeito visual do bloqueio
	PerformAction(defender, ANIM_TWIST_DODGE);

	if (blocker)
		PerformAction(defender, 13);
		PrintTextAbove(blocker, "*defende o golpe pelo aliado*");
		SetCooldown(attacker, "parrydelay", parryDelay * 2);
	else
		PerformAction(defender, 13);
		PrintTextAbove(defender, "*defende o golpe*");
		SetCooldown(attacker, "parrydelay", parryDelay);
	endif

	// Aplica dano no shield
	WearDownShield(defender, shield, rawdamage, weapon);

	rawdamage := 0;
	return 1;
endfunction