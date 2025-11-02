/* $Id$
 *
 * Purpose
 * Criado para organizar a lógica de cálculo de chance de acerto do combate
 */

// include "combat";
include "AbilitiesController";
include ":combat:Model/CombatAbility";
include ":crafting:/include/caracteristicasFabricacao";

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
	if (!attackInfo.GetAttacker().IsNPC())
		if (!attackInfo.GetDefender().IsWarMode())
			return 100;
		endif
	endif

	if (attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetAttribute() == RANGED && GetCooldown(attackInfo.GetDefender().GetObjRef(), "Desviar Disparos") > 0)
		Message("*desviar disparos*").SendAbove(attackInfo.GetDefender().GetObjRef());
		return 0;
	endif

	if (attackInfo.GetAttacker().GetObjRef().GetProp("#attackfail"))
		attackInfo.GetAttacker().GetObjRef().EraseProp("#attackfail");
		return 0;
	endif

	if (attackInfo.GetDefender().GetObjRef().GetProp("#defensefail"))
		attackInfo.GetDefender().GetObjRef().EraseProp("#defensefail");
		return 100;
	endif

	if (GetCooldown(attackInfo.GetAttacker().GetObjRef(), "ataquetotal"))
		return 100;
	endif
	if (GetCooldown(attackInfo.GetDefender().GetObjRef(), "ataquetotal"))
		return 100;
	endif

	// As condições que precisam de cálculo, começam aqui
	var attackerSkill := _GetSkillForCombat(attackInfo.GetAttacker());
	var defenderSkill := _GetSkillForCombat(attackInfo.GetDefender());
	var hit_chance := 50.0 + ((attackerSkill - defenderSkill) / 1.5);

	hit_chance += CInt(PegaValorFabricacao(attackInfo.GetAttacker().GetWeapon().GetObjRef(), CARACTERISTICA_FABRICACAO_PRECISAO) * 3);
	hit_chance += attackInfo.GetAttacker().GetObjRef().hitchance_mod - attackInfo.GetDefender().GetObjRef().evasionchance_mod;
	hit_chance += CInt(attackInfo.GetAttacker().GetObjRef().GetProp("#attackbonus"));

	if (CInt(attackInfo.GetAttacker().GetObjRef().GetProp("#attackbonus"))) // TODO: Isso aqui não é pra estar aqui. Colocar em alguma função do dano/combate.
		attackInfo.GetAttacker().GetObjRef().EraseProp("#attackbonus");
	endif

	attackInfo.GetAttacker().GetObjRef().EraseProp("insistencia"); // Mesma coisa do ataque bonus, isso não é pra estar aqui

	if (GetCooldown(attackInfo.GetDefender().GetObjRef(), "evasion_chance"))
		hit_chance -= 10.0;
	endif

	if (GetCooldown(attackInfo.GetAttacker().GetObjRef(), "blinded") > 0)
		hit_chance -= 20.0;
	endif

	if (GetCooldown(attackInfo.GetDefender().GetObjRef(), "exposed") > 0)
		hit_chance += 10.0;
	endif

	if (attackInfo.GetAttacker().HasAbility(COMBAT_ABILITY::POSTURA_OFENSIVA))
		hit_chance += 10.0;
	endif

	if (attackInfo.GetDefender().HasAbility(COMBAT_ABILITY::POSTURA_OFENSIVA))
		hit_chance += 10.0;
	endif

	if (attackInfo.GetAttacker().HasAbility(COMBAT_ABILITY::POSTURA_DEFENSIVA))
		hit_chance -= 10.0;
	endif

	if (attackInfo.GetDefender().HasAbility(COMBAT_ABILITY::POSTURA_DEFENSIVA))
		hit_chance -= 10.0;
	endif

	if (attackInfo.GetDefender().HasAbility(COMBAT_ABILITY::PRESERVAR_ORGAOS))
		hit_chance -= 10.0;
	endif

	if (GetCooldown(attackInfo.GetAttacker().GetObjRef(), "posturadecombate") > 0)
		hit_chance += 10.0;
	endif

	var challenged_by := attackInfo.GetAttacker().GetObjRef().GetProp("#DuelChallenge");
	if (challenged_by && !attackInfo.GetAttacker().IsNPC)
		if (challenged_by.time < ReadGameClock())
			attackInfo.GetAttacker().GetObjRef().EraseProp("#DuelChallenge");
		else
			var challenger := SystemFindObjectBySerial(challenged_by.challenger);
			if (challenger && challenger.connected)
				if (attackInfo.GetDefender().GetSerial() != challenger.serial)
					hit_chance -= 10.0;
					if (RandomInt(100 + 1) < 20.0)
						Message("Você está em duelo! Seus ataques são menos efetivos contra outros alvos.", FONTCOLOR_ERROR).SendSys(attackInfo.GetAttacker().GetObjRef());
					endif
				endif
			else
				attackInfo.GetAttacker().GetObjRef().EraseProp("#DuelChallenge");
			endif
		endif
	endif

	if (attackInfo.GetAttacker().HasAbility(COMBAT_ABILITY::AURA_DE_FE))
		hit_chance += CDbl(attackInfo.GetAttacker().GetObjRef().party.members.size());
	endif

	if (attackInfo.GetAttacker().GetObjRef().GetProp("armapreferida") == attackInfo.GetAttacker().GetWeapon().GetObjType())
		hit_chance += 5.0;
	endif

	if (GetCooldown(attackInfo.GetAttacker().GetObjRef(), "furia") && attackInfo.GetAttacker().HasAbility(COMBAT_ABILITY::FURIA_INDOMAVEL))
		if (!attackInfo.GetDefender().HasAbility("Fortitudo Impia"))
			hit_chance += 10.0;
		endif
	endif

	if (IsRaging(attackInfo.GetAttacker().GetObjRef()))
		if (attackInfo.GetDefender().HasAbility(COMBAT_ABILITY::ILUDIR_O_FRACO))
			hit_chance -= 10.0;
		endif
		
		if (GetCooldown(attackInfo.GetAttacker().GetObjRef(), "Ataques Enlouquecidos") > 0)
			return 100;
		endif
	endif

	if (IsRaging(attackInfo.GetDefender().GetObjRef()))
		if (attackInfo.GetDefender().HasAbility(COMBAT_ABILITY::ESQUIVA_INSTINTIVA))
			hit_chance -= 10.0;
		endif

		if (GetCooldown(attackInfo.GetDefender().GetObjRef(), "Ataques Enlouquecidos") > 0)
			return 100;
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

	if (attackInfo.GetAttacker().HasAbility(COMBAT_ABILITY::LOBO_SOLITARIO))
		if (ListHostiles(attackInfo.GetAttacker().GetObjRef(), 3).size() > 1)
			hit_chance += 15.0;
		endif
	endif

	var tiroCerteiro := CInt(attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetObjRef().GetProp("tirocerteiro"));
	if (tiroCerteiro > 0)
		tiroCerteiro--;

		if (tiroCerteiro == 0)
			attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetObjRef().EraseProp("tirocerteiro");
		else
			attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetObjRef().SetProp("tirocerteiro", tiroCerteiro);
		endif

		return 100;
	endif

	if (attackInfo.GetDefender().HasAbility(COMBAT_ABILITY::BUDOKA))
		if (attackInfo.GetDefender().GetWeapon().GetAttribute() == WRESTLING)
			hit_chance -= CDbl(attackInfo.GetDefender().GetSkill(TACTICS) / 5);
		endif
	endif

	if (attackInfo.GetAttacker().HasAbility(COMBAT_ABILITY::DANCA_DAS_SOMBRAS) && GetCooldown(attackInfo.GetDefender().GetObjRef(), "shadowdancer_blur") > 0)
		hit_chance += CDbl(attackInfo.GetAttacker().GetSkill(ARCANA) / 5);
	endif

	if (attackInfo.GetAttacker().HasAbility(COMBAT_ABILITY::MOVIMENTACAO_GRACIOSA))
		hit_chance += CDbl(attackInfo.GetAttacker().GetSkill(TACTICS) / 10);
	endif

	if (attackInfo.GetDefender().HasAbility(COMBAT_ABILITY::MOVIMENTACAO_GRACIOSA))
		hit_chance -= CDbl(attackInfo.GetDefender().GetSkill(TACTICS) / 10);
	endif

	if (attackInfo.GetAttacker().HasAbility(COMBAT_ABILITY::MANCHA_DA_CORRUPCAO))
		hit_chance += CDbl(attackInfo.GetAttacker().GetSkill(THEOLOGY) / 10);
	endif

	if (attackInfo.GetDefender().HasAbility(COMBAT_ABILITY::MANCHA_DA_CORRUPCAO))
		hit_chance -= CDbl(attackInfo.GetDefender().GetSkill(THEOLOGY) / 10);
	endif

	if (attackInfo.GetAttacker().HasAbility("Oleo do Cacador"))
		hit_chance += CDbl(attackInfo.GetAttacker().GetSkill(SCIENCE) / 5);
	endif

	if (!attackInfo.GetAttacker().IsNPC())
		DarknessHitModifier(attackInfo.GetAttacker().GetObjRef(), attackInfo.GetDefender().GetObjRef(), hit_chance);
	endif
	
	if (attackInfo.GetAttacker().GetObjRef().GetProp("#ataqueinesperado"))
		if (attackInfo.GetDefender().GetObjRef().GetProp("lastHit").serial == attackInfo.GetAttacker().GetSerial())
			hit_chance += 50.0;
			Message("*ataque inesperado*", FONTCOLOR_ERROR).SendAbove(attackInfo.GetAttacker().GetObjRef());
		endif

		attackInfo.GetAttacker().GetObjRef().EraseProp("#ataqueinesperado");
	endif

	if (attackInfo.GetAttacker().GetObjRef().GetProp("#bomcomeco"))
		var attackers := attackInfo.GetDefender().GetObjRef().GetProp("#bomcomecoserials");
		if (attackers == error) attackers := array{}; endif

		if (!(attackInfo.GetAttacker().GetSerial() in attackers))
			hit_chance += 50.0;
			attackers.append(attackInfo.GetAttacker().GetSerial());
			attackInfo.GetDefender().GetObjRef().SetProp("#bomcomecoserials", attackers);
			Message("Você começou bem.", FONTCOLOR_SUCCESS).SendSys(attackInfo.GetAttacker().GetObjRef());
		endif

		attackInfo.GetAttacker().GetObjRef().EraseProp("#bomcomeco");
	endif

	if (attackInfo.GetDefender().GetObjRef().GetProp("#mobilidade"))
		hit_chance -= 15.0;
		AP_ConsumeVital(attackInfo.GetDefender().GetObjRef(), STAMINA, 3);
	endif

	if (attackInfo.GetAttacker().GetObjRef().GetProp("#mobilidade"))
		hit_chance -= 15.0;
	endif

	if (attackInfo.GetDefender().GetNPCTemplate() == ":epicai:spiderling")
		hit_chance -= 20.0;
	endif

	if (attackInfo.GetAttacker().HasAbility(COMBAT_ABILITY::TIROTHAN))
		hit_chance += CDbl(attackInfo.GetAttacker().GetSkill(ARCANA) / 10);
	endif

	if (hit_chance < 5.0)
		hit_chance := 5.0;
	elseif (hit_chance > 100.0)
		hit_chance := 100.0;
	endif

	return CInt(hit_chance); // Devolve Int, pra facilitar a rolagem do dado
endfunction

function _GetSkillForCombat(mobile)
	var skill;

	if (mobile.IsNPC()) // Se for NPC, a skill é wrestling
		skill := mobile.GetSkill(WRESTLING);
	else // Se não for, o bicho pega....
		var attribute := mobile.GetEquipBySlot(WEAPON).GetAttribute();

		if ((mobile.GetObjRef().GetProp("metamorfose") != error) ||
			(mobile.GetObjRef().GetProp("pilotando") != error)) // Se é Metamorfo ou Piloto de Golem, a skill tá na prop
			skill := mobile.GetObjRef().GetProp("skill_a");
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
			mobile.GetObjRef().GetProp("chardata").trait != "Cavalaria de Escaramuça") // E não tiver hab de cavalaria
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

function CheckDistance(attacker, defender) // attacker: MobileWrapper, defender: MobileWrapper
	var attackerDistance := Distance(attacker.GetObjRef(), defender.GetObjRef());
	var weapon := attacker.GetEquipBySlot(WEAPON);

	if (attackerDistance < weapon.GetMinRange() || attackerDistance > weapon.GetMaxRange())
		return false;
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

function AmmoCheck(attacker, defender)
	var attackerWeapon := WeaponWrapper(attacker.weapon);

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

function CalculateParryChance(byRef attackInfo)
	// Verifica se o atacante já pode ser bloqueado novamente.
	if (GetCooldown(attackInfo.GetAttacker().GetObjRef(), "parrydelay") > 0)
		return 0;
	endif

	var shield := attackInfo.GetParryEquip(),
		parryBonus := 0,
		skill := 0,
		blocker;

	// Verifica se o defensor está sob efeito da a hab esudo do martir
	var wall := attackInfo.GetDefender().GetObjRef().party.GetProp("BarreiraDeEscudos"),
		wallChance := 0;

	if (wall)
		blocker := SystemFindObjectBySerial(wall.blocker);
		if (GetCooldown(blocker, "BarreiraDeEscudos") > 0)
			if (Distance(attackInfo.GetDefender().GetObjRef(), blocker) < 2)
				wallChance := CInt(wall.chance);
			endif
		else
			attackInfo.GetDefender().GetObjRef().party.EraseProp("BarreiraDeEscudos");
			blocker := 0;
		endif
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

	var skill;
	if (attackInfo.GetDefender().HasAbility(COMBAT_ABILITY::REFLEXO_ESTRATEGICO))
		skill := CInt(attackInfo.GetDefender().GetSkill(TACTICS));
	else
		skill := CInt(attackInfo.GetDefender().GetSkill(PARRY));
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
	if (attackInfo.GetParryEquip().GetObjRef().IsA(POLCLASS_WEAPON) && !TemHabilidade(defender, "Tabibito no Michi"))
		parry_chance := CInt(parry_chance * 0.7);
	endif

	if (attackInfo.GetParryEquip().GetObjRef().IsA(POLCLASS_WEAPON) && TemHabilidade(defender, "Tabibito no Michi"))
		parry_chance := CInt(parry_chance * 1.3);
	endif

	// Calcula a chance TOTAL de bloqueio
	parry_chance := parry_chance + parryBonus;

	// A habilidade do companheiro de party do defensor tentar bloquear em seu lugar só ativa se a chance de bloqueio dele for melhor que do defensor
	if (wallChance > parry_chance)
		parry_chance := wallChance;
	endif

	// Aplica o efeito da habilidade que deixa o defensor bloquear sempre
	if (CheckHab_BarreiraIndestrutivel(attacker, defender))
		parry_chance := 100;
		parryDelay := 2;
	endif

	if (!defendeu)
		return false;
	endif

	// Exibe o efeito visual do bloqueio
	PlayDefenseFX(attackInfo);
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

	return parry_chance;
endfunction