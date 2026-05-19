// Internal
include ":combat:armorZones";
// include ":combat:Controller/DamageController_legacy";
// include ":combat:Controller/CombatAbilityController";
include ":combat:weardown_legacy";

// External
include ":tn:cooldown";
include ":attributes:attributes";
include ":blood:blood";
include ":fls_core:packets";
include "include/epicCombatUtils";
include "include/facings";
include ":timedScripts:timedScripts";
// include ":debug:Controller/DebugController";
// include ":equipsys:utils/consts";
include "include/epicNPCUtils";
include ":combat:weaponInfo";
include ":wrappers:Objects/Model/MobileWrapper";
include ":abilities:models/Ability";
include ":charactercreation:habilidades";
/*
 * http://uo.stratics.com/content/guides/resistance.shtml
 * These could be attributes, but not every shard will want to
 * follow OSI. This gives more freedom to make custom resistances
 * and remove existing ones more easily.
 */
enum DAMAGE_TYPES
	DMG_FORCED		:= -1, // Ignores resistance
	DMG_PHYSICAL	:= "Physical",
	
	DMG_SLASH		:= "Slash",
	DMG_PIERCING	:= "Piercing",
	DMG_BLUNT		:= "Blunt",

	DMG_FIRE		:= "Fire",
	DMG_COLD		:= "Cold",
	DMG_POISON		:= "Poison",
	DMG_ENERGY		:= "Energy"
endenum

function _GetElementalSpecForDmgType(dmgType)
	case (dmgType)
		DMG_FIRE:    return ABILITY_ID::ELEMENTAL_SPEC_FIRE;
		DMG_COLD:    return ABILITY_ID::ELEMENTAL_SPEC_COLD;
		DMG_ENERGY:  return ABILITY_ID::ELEMENTAL_SPEC_ENERGY;
		DMG_POISON:  return ABILITY_ID::ELEMENTAL_SPEC_POISON;
		default:     return 0;
	endcase
endfunction

function getNewDamageList(attacker)
	var list := dictionary;

	if (attacker.npctemplate)
		var npctemplate := NPC_ParseTemplateName(attacker.npctemplate);
		var cfg := ReadConfigFile(":*:npcdesc");
		list[DMG_PHYSICAL] := cfg[npctemplate.template].PhysicalDamage;
		list[DMG_FIRE] := cfg[npctemplate.template].FireDamage;
		list[DMG_COLD] := cfg[npctemplate.template].ColdDamage;
		list[DMG_POISON] := cfg[npctemplate.template].PoisonDamage;
		list[DMG_ENERGY] := cfg[npctemplate.template].EnergyDamage;

		return list;
	else

		list[DMG_PHYSICAL] := attacker.damage_physical;
		list[DMG_FIRE] := attacker.damage_fire;
		list[DMG_COLD] := attacker.damage_cold;
		list[DMG_POISON] := attacker.damage_poison;
		list[DMG_ENERGY] := attacker.damage_energy;

		var elemental := GetObjProperty(attacker.weapon, "#elementalweapon");
		if (elemental != error)
			list[elemental.type] := list[elemental.type] + elemental.bonus;
		endif

		var ammo := GetObjProperty(attacker.weapon, "ammo");

		if (ammo)
			var cfg := ReadConfigFile(":woodworking:itemdesc");
			var elem := FindConfigElem(cfg, ammo);
			var dmgtype := GetConfigStringArray(elem, "dmgtype");
			var dmgbonus := CDbl(GetConfigString(elem, "dmgbonus")) / 100.0;
			if (dmgtype)
				foreach dmg in dmgtype
					case (dmg)
						"Physical":
							list[DMG_PHYSICAL] += dmgbonus[_dmg_iter];
							break;
						"Cold":
							list[DMG_COLD] += dmgbonus[_dmg_iter];
							break;
						"Fire":
							list[DMG_FIRE] += dmgbonus[_dmg_iter];
							break;
						"Poison":
							list[DMG_POISON] += dmgbonus[_dmg_iter];
							break;
						"Energy":
							list[DMG_ENERGY] += dmgbonus[_dmg_iter];
							break;
						default:
							break;
					endcase
				endforeach
			endif
		endif

		// Primordial Form elemental conversion for druids in metamorphose
		var metamorfose := GetObjProperty(attacker, "metamorfose");
		if (metamorfose)
			// Check which primordial element is active
			if (GetObjProperty(attacker, "primordial_fire"))
				// Convert all damage to fire (100%)
				list[DMG_FIRE] := 1.0;
				list[DMG_PHYSICAL] := 0.0;
				list[DMG_COLD] := 0.0;
				list[DMG_POISON] := 0.0;
				list[DMG_ENERGY] := 0.0;
			elseif (GetObjProperty(attacker, "primordial_cold"))
				// Convert all damage to cold (100%)
				list[DMG_COLD] := 1.0;
				list[DMG_PHYSICAL] := 0.0;
				list[DMG_FIRE] := 0.0;
				list[DMG_POISON] := 0.0;
				list[DMG_ENERGY] := 0.0;
			elseif (GetObjProperty(attacker, "primordial_energy"))
				// Convert all damage to energy (100%)
				list[DMG_ENERGY] := 1.0;
				list[DMG_PHYSICAL] := 0.0;
				list[DMG_FIRE] := 0.0;
				list[DMG_COLD] := 0.0;
				list[DMG_POISON] := 0.0;
			elseif (GetObjProperty(attacker, "primordial_poison"))
				// Convert all damage to poison (100%)
				list[DMG_POISON] := 1.0;
				list[DMG_PHYSICAL] := 0.0;
				list[DMG_FIRE] := 0.0;
				list[DMG_COLD] := 0.0;
				list[DMG_ENERGY] := 0.0;
			endif
		endif

		return list;
	endif
endfunction

function CalculateDamage(defender, type, dmg)
	var resistMod := 0;
	var resist := 0;

	if (defender.npctemplate)
		var npctemplate := NPC_ParseTemplateName(defender.npctemplate);
		var cfg := ReadConfigFile(":*:npcdesc");

		case(type)
			DMG_PHYSICAL: resist := CInt(cfg[npctemplate.template].PhysicalResist) + CInt(defender.resist_physical_mod); break;
			DMG_FIRE: resist := CInt(cfg[npctemplate.template].FireResist) + CInt(defender.resist_fire_mod); break;
			DMG_COLD: resist := CInt(cfg[npctemplate.template].ColdResist) + CInt(defender.resist_cold_mod); break;
			DMG_ENERGY: resist := CInt(cfg[npctemplate.template].EnergyResist) + CInt(defender.resist_energy_mod); break;
			DMG_POISON: resist := CInt(cfg[npctemplate.template].PoisonResist) + CInt(defender.resist_poison_mod); break;
		endcase
	else
		case(type)
			DMG_PHYSICAL:
				resist := CInt(defender.resist_physical);
				resistMod := CInt(defender.resist_physical_mod);
				DebugMsg(defender, DEBUG_DAMAGE, $"Defesa Física (Mod): {resist}({resistMod})");
				break;

			DMG_FIRE:
				resist := CInt(defender.resist_fire);
				resistMod := CInt(defender.resist_fire_mod);
				DebugMsg(defender, DEBUG_DAMAGE, $"Defesa Fire (Mod): {resist}({resistMod})");
				break;

			DMG_COLD:
				resist := CInt(defender.resist_cold);
				resistMod := CInt(defender.resist_cold_mod);
				DebugMsg(defender, DEBUG_DAMAGE, $"Defesa Cold (Mod): {resist}({resistMod})");
				break;

			DMG_ENERGY:
				resist := CInt(defender.resist_energy);
				resistMod := CInt(defender.resist_energy_mod);
				DebugMsg(defender, DEBUG_DAMAGE, $"Defesa Energy (Mod): {resist}({resistMod})");
				break;

			DMG_POISON:
				resist := CInt(defender.resist_poison);
				resistMod := CInt(defender.resist_poison_mod);
				DebugMsg(defender, DEBUG_DAMAGE, $"Defesa Poison (Mod): {resist}({resistMod})");
				break;
		endcase
	endif

	dmg -= resist;

	if (dmg < 1)
		dmg := 1;
	endif

	return dmg;
endfunction

function CalculateTotalDamage(defender, basedamage, attacker, dmglist := 0)
	var damage := 0;

	if (!dmglist)
		dmgList := getNewDamageList(attacker);
	endif

	foreach key in (dmgList.keys())
		damage += CalculateDamage(defender, key, basedamage-dmglist[key]);
	endforeach

	if (damage < 1)
		damage := 1;
	endif

	return damage;
endfunction

function CalculateRawDamage(attacker, weapon, offDamage := 0, attackType := BLUNTDAMAGE)
	var dmg;
	var weaponDice, weaponModDice;

	if (attacker.npctemplate)
		// TODO: Implementar uma mini AI para fazer o NPC modificar o dano baseado na postura de ataque possível (assim como é possível com os players)
			// TODO: Por enquanto vai usar só o maior dano base.
		var damage := getNPCMainDamage(attacker, attackType);
		weaponDice := damage.dice;
	else
		if (GetObjProperty(attacker, "metamorfose") != error)
			weaponDice := GetObjProperty(attacker, "dicedmg");
		elseif (GetObjProperty(attacker, "pilotando") != error)
			weaponDice := GetObjProperty(attacker, "dicedmg");
		elseif (offDamage)
			weaponDice := weapon.getOffDamageDice();
		else
			weaponDice := weapon.getMainDamageDice();
    	endif
	endif

	DebugMsg(attacker, DEBUG_DAMAGE, "Dano da arma: "+weapondice);

	if (!weapondice || weapondice == error)
		dmg := 0;
	else
		dmg := RandomDiceRoll(weapondice);
	endif

	DebugMsg(attacker, DEBUG_DAMAGE, "Rolou dano: "+dmg);

	return dmg;
endfunction

function ApplyDamageEX(defender, dmg, type, source := 0)
	var totaldmg := cint(CalculateDamage(defender, type, dmg ));

	ApplyRawDamage( defender, totaldmg );

	if (source)
		SetLastDamageInfo(defender, totaldmg, type, source);
		SetScriptController(source);
	endif

	return totaldmg;
endfunction


/*
 * SetLastDamageInfo(mobile, amount, type, source)
 *
 * Purpose
 *
 * Parameters
 * mobile:
 * amount:
 * type:
 * source:
 *
 * Return value
 *
 */
function SetLastDamageInfo(mobile, amount, type, source, spell := "")
	var last_damage := struct;
	last_damage.+serial	:= source.serial;
	last_damage.+time	:= polcore().systime;
	last_damage.+amount	:= amount;
	last_damage.+type	:= type;

	if ( spell )
		last_damage.+spell := spell;
	endif

////AUTOPSIA
var life := AP_GetVital(mobile, "HITS");
if (amount >= life)
var autopsy := struct;
	if ( source.weapon )// adicionado para o forensicevaluation detectar a arma do crime
		//last_damage.+weapon := source.weapon;
		autopsy.+serial_weapon := source.weapon.serial; 
		autopsy.+objtype := source.weapon.objtype;
	endif
	
    // Descobrir mais Dano
    var slashDamage := getNPCMainDamage(mobile, SLASHDAMAGE);
    var piercingDamage := getNPCMainDamage(mobile, PIERCINGDAMAGE);
    var bluntDamage := getNPCMainDamage(mobile, BLUNTDAMAGE);
    // Compara os valores dos dados (assumindo que dice tem um valor numérico)
    var maxDamage := slashDamage.dice;
    var maxType := SLASHDAMAGE;  
    if (piercingDamage.dice > maxDamage)
        maxDamage := piercingDamage.dice;
        maxType := PIERCINGDAMAGE;
    endif
    if (bluntDamage.dice > maxDamage)
        maxDamage := bluntDamage.dice;
        maxType := BLUNTDAMAGE;
    endif
    autopsy.+dano_principal := maxType;

	var stat := struct;
	stat.+STR := AP_GetStat(mobile, "STRENGTH");
	stat.+DEX := AP_GetStat(mobile, "DEXTERITY");
	stat.+INT := AP_GetStat(mobile, "INTELLIGENCE");
	
	var vital := struct;
	vital.+HIT := AP_GetVitalMaximumValue(mobile, "HITS");
	vital.+STA := AP_GetVitalMaximumValue(mobile, "STAMINA");
	vital.+MAN := AP_GetVitalMaximumValue(mobile, "MANA");
	//Descobir as resits
	autopsy.+resists := getNPCResists(mobile);
	autopsy.+elemdamage := getNPCElemDamages(mobile);
	autopsy.+stats := stat;
	autopsy.+vitals := vital;
	
	var npc_elem := NPC_GetNPCConfig(mobile);
	var classe := npc_elem.Classe;
	var skills_combat := struct;
	
	if (classe == SABIO)
		skills_combat.+magery := AP_GetTrueSkill(mobile, MAGERY);
		skills_combat.+arcana := AP_GetTrueSkill(mobile, ARCANA);
	elseif(classe == LADINO || mobile.isSpider())
		skills_combat.+sneak := EffectiveTrueSkill(mobile, "Stealth");
		skills_combat.+awareness := EffectiveTrueSkill(mobile, "Perception");
		skills_combat.+pickpocketing := EffectiveTrueSkill(mobile, "Theft");
	else
		skills_combat.+wrestling := AP_GetTrueSkill(mobile, WRESTLING);
		skills_combat.+tactics := AP_GetTrueSkill(mobile, TACTICS);
	endif
	autopsy.+skills := skills_combat;
	autopsy.+taxonomy := npc_elem.Taxonomy;
	autopsy.+rastros := npc_elem.rastro;

	SetObjProperty(mobile, "Autopsy", autopsy);
endif	
//FIM AUTOPSIA

	if ( last_damage.acctname )
		last_damage.+acctname := source.acctname;
	endif
	if ( source.name )
		last_damage.+name := source.name;
	else
		last_damage.+name := source.desc;
	endif

	SetObjProperty(mobile, "LastDamage", last_damage);

	return last_damage;
endfunction

/*
 * GetLastDamageInfo(mobile)
 *
 * Purpose
 *
 * Parameters
 * mobile:
 *
 * Return value
 *
 */
function GetLastDamageInfo(mobile)
	return GetObjProperty(mobile, "LastDamage");
endfunction

// Barreira de Mana (spell 114): redirects incoming damage to mana at shield.ratio:1
// (default 2:1, 1:1 when caster has Arcana primary).
// Returns the damage that should still be applied to HP after absorption.
function _ApplyManaShield(defender, dmg)
	if (dmg <= 0)
		return dmg;
	endif

	var shield := GetObjProperty(defender, "#manashield");
	if (!shield || shield is @Exception)
		return dmg;
	endif

	if (shield.endtime == error || shield.endtime <= ReadGameClock())
		EraseObjProperty(defender, "#manashield");
		return dmg;
	endif

	var current_mana := AP_GetVital(defender, MANA);
	if (current_mana <= 0)
		EraseObjProperty(defender, "#manashield");
		return dmg;
	endif

	var ratio := CInt(shield.ratio);
	if (ratio < 1)
		ratio := 2;
	endif

	var absorbable := Cint(current_mana / ratio);
	var absorbed := dmg;
	if (absorbed > absorbable)
		absorbed := absorbable;
	endif
	if (absorbed <= 0)
		return dmg;
	endif

	AP_ConsumeVital(defender, MANA, absorbed * ratio);

	if (AP_GetVital(defender, MANA) <= 0)
		EraseObjProperty(defender, "#manashield");
		PrintTextAbove(defender, "*barreira arcana quebra*");
	endif

	return dmg - absorbed;
endfunction

function DamageFLS(defender, dmg, type := DMG_FORCED, source := 0, spell := "")
	var totaldmg;
	var attackInfo := struct{};

	// Caminho: Elemental Specialization bonus (+2 per purchase)
	if (source && type != DMG_FORCED)
		var elemSpecId := _GetElementalSpecForDmgType(type);
		if (elemSpecId)
			var mob := MobileWrapper(source);
			var count := mob.GetPoolAbilityCount(elemSpecId);
			if (count > 0)
				dmg := dmg + (count * 2);
			endif
		endif
	endif

	if (type == DMG_FORCED)
		totaldmg := CInt(dmg);
	else
		totaldmg := CInt(CalculateDamage(defender, type, dmg));
	endif

	if (type == DMG_PHYSICAL || type == DMG_SLASH || type == DMG_PIERCING || type == DMG_BLUNT)
		totaldmg := _ApplyManaShield(defender, totaldmg);
	endif

	if (source)
		SetScriptController(source);
	endif

	// Mesmo sendo uma função VANILLA. Talvez esses CheckHab_ precisem ser trocados por CheckDefenderHabsBeforeDamage() e CheckDefenderHabsAfterDamage()
	attackInfo.+damage := dmg;
	attackInfo.+type := type;
	
	// Checa as habilidades antes do dano
	// CheckHab_Martir(0, defender, attackInfo);
	// CheckHab_MantoRothen(0, defender, attackInfo);

	// Aplica o dano
	DisplayDamage(defender, totaldmg, source);
	ApplyRawDamage(defender, totaldmg);
	SetLastDamageInfo(defender, totaldmg, type, source, spell);

	if (totaldmg > 0)
		var vingancaCur := GetObjProperty(defender, "#vinganca");
		if (vingancaCur != error)
			SetObjProperty(defender, "#vinganca", CInt(vingancaCur) + CInt(totaldmg));
		endif
	endif

//FRAJOLAS Adicionado para o discordwebhook detectar dano de trap
if (totaldmg > 0)
    var lastHit := dictionary;

    if (source) // dano normal (npc/player)
        lastHit["attacker"] := source.serial;
    else // trap ou dano sem source
        lastHit["attacker"] := 0;
        lastHit["source"] := "trap";
    endif

    lastHit["amount"] := totaldmg;
    lastHit["time"] := polcore().systime;
    lastHit["type"] := type;

    SetObjProperty(defender, "lastHit", lastHit);
endif
//end FRAJOLAS

	// Checa as habilidades depois do dano
	// CheckHab_RecuperacaoSalvadora(0, defender, attackInfo);
	// CheckHab_Vinganca(0, defender, attackInfo);

	if (defender.frozen)
		defender.frozen := 0;
	endif

	return totaldmg;
endfunction

function CalcDamageHabs(attacker, defender, damage)
	if (GetObjProperty(attacker, "#tiropenetrante")) // TODO: Pensar o que fazer aqui, já que perfura armadura, talvez jogar pra função que aplica o dano das habs, ao invés de entrar no cálculo aqui
		EraseObjProperty(attacker, "#tiropenetrante");
        var damagemod := RandomDiceRoll("1d4");
		damage += damagemod;
		// DamageFLS(defender, cint(damage-rawdamage*2), DMG_FORCED, attacker);
		DebugMsg(attacker, DEBUG_DAMAGE, "Dano de Habilidade[Tiro Penetrante]: " + CInt(damagemod));
	endif

	if (MobileWrapper(attacker).HasAbility(ABILITY_ID::TAVERN_BRAWLER) && attacker.weapon.objtype == 0x1F020)
        var damagemod := 2;
		damage += damagemod;
		DebugMsg(attacker, DEBUG_DAMAGE, "Dano de Habilidade[Brigador de Taverna]: " + CInt(damagemod));
    endif

	if (getCooldown(attacker, "faroporsangue"))
		var bonus := AP_GetVital(defender, HITS) / AP_GetVitalMaximumValue(defender, HITS);

		if (bonus <= 0.25)
			bonus := "1d12";
		elseif (bonus <= 0.5)
			bonus := "1d10";
        elseif (bonus <= 0.75)
            bonus := "1d8";
		endif

		var damagemod := RandomDiceRoll(bonus);
		damage += damagemod;
		DebugMsg(attacker, DEBUG_DAMAGE, "Dano de Habilidade[Faro por Sangue]: " + CInt(damagemod));
	endif

	if (GetCooldown(attacker, "lobosolitario"))
		if (ListHostiles(attacker, 3, LH_FLAG_LOS) > 2)
			var damagemod := CInt(AP_GetSkill(attacker, SURVIVAL) / 20);
			damage += damagemod;
			DebugMsg(attacker, DEBUG_DAMAGE, "Dano de Habilidade[Lobo Solitário]: " + CInt(damagemod));
		endif
	endif

	if (MobileWrapper(attacker).HasAbility(ABILITY_ID::PRECISAO_CIRURGICA))
		var damagemod := CInt(AP_GetSkill(attacker, SCIENCE) / 20);
		damage += damagemod;
		DebugMsg(attacker, DEBUG_DAMAGE, "Dano de Habilidade[Precisão Cirurgica]: " + CInt(damagemod));
	endif

    var arsenalWrap := MobileWrapper(attacker);
    if (arsenalWrap.HasAbility(ABILITY_ID::ARSENAL_SOMBRIO))
        var damagemod := CInt(EffectiveSkill(attacker, "Stealth") / 20);
        var weapon := arsenalWrap.GetEquipBySlot(WEAPON);
        if (weapon && weapon.IsAssassin())
			damage += damagemod;
			DebugMsg(attacker, DEBUG_DAMAGE, "Dano de Habilidade[Arsenal Sombrio]: " + CInt(damagemod));
        endif
    endif

	if (MobileWrapper(attacker).HasAbility(ABILITY_ID::CONSAGRACAO_DIVINA))
        var npctemplate := NPC_ParseTemplateName(defender.npctemplate);
		var cfg := ReadConfigFile(":*:npcdesc");
		var inimigo := array{"Undead"};

		if (cfg[npctemplate.template].Environment in inimigo)
			var damagemod := Cint(AP_GetSkill(attacker, THEOLOGY) / 20);
			damage := damagemod;
			DebugMsg(attacker, DEBUG_DAMAGE, "Dano de Habilidade[Consagracao Divina]: " + CInt(damagemod));
		endif
	elseif (MobileWrapper(attacker).HasAbility(ABILITY_ID::SIGILO_DE_PROTECAO))
        var npctemplate := NPC_ParseTemplateName(defender.npctemplate);
		var cfg := ReadConfigFile(":*:npcdesc");
		var inimigo := array{"Demon"}; 

		if (cfg[npctemplate.template].Environment in inimigo)
			var damagemod := Cint(AP_GetSkill(attacker, THEOLOGY) / 20);
			damage := damagemod;
			DebugMsg(attacker, DEBUG_DAMAGE, "Dano de Habilidade[Sigilo de Protecao]: " + CInt(damagemod));
		endif
	elseif (MobileWrapper(attacker).HasAbility(ABILITY_ID::OLEO_DO_CACADOR))
		var npctemplate := NPC_ParseTemplateName(defender.npctemplate);
		var cfg := ReadConfigFile(":*:npcdesc");
		var inimigo := GetObjProperty(attacker, "Óleo");
		if (cfg[npctemplate.template].Environment == inimigo)
			var damagemod := Cint(AP_GetSkill(attacker, SCIENCE) / 20);
			damage := damagemod;
			DebugMsg(attacker, DEBUG_DAMAGE, "Dano de Habilidade[Oleo do Cacador]: " + CInt(damagemod));
		endif
	endif

	if (GetCooldown(defender, "prey"))
		var damagemod := Cint(AP_GetSkill(attacker, SURVIVAL) / 20);
		damage += damagemod;
		DebugMsg(attacker, DEBUG_DAMAGE, "Dano de Habilidade[Presa]: " + CInt(damagemod));
	endif

	if (MobileWrapper(attacker).HasAbility(ABILITY_ID::INSTINTO_DE_SOBREVIVENCIA) && AP_GetVital(attacker, HITS) <= (AP_GetSkill(attacker, THEOLOGY) / 2))
		if (GetObjProperty(attacker, "metamorfose") != error)
			var damagemod := Cint(AP_GetSkill(attacker, THEOLOGY) / 20);
			damage += damagemod;
			DebugMsg(attacker, DEBUG_DAMAGE, "Dano de Habilidade[Arsenal Sombrio]: " + CInt(damagemod));
			PrintTextAbove(attacker, "*enfurecido*");
		endif
	endif

	if (IsRaging(attacker))
		var damagemod;
		if (MobileWrapper(attacker).HasAbility(ABILITY_ID::FURIA_DO_DESERTO) || attacker.hasRage())
			damagemod := Cint(AP_GetSkill(attacker, TACTICS) / 20);
			damage += damagemod;
			DebugMsg(attacker, DEBUG_DAMAGE, "Dano de Habilidade[Nazg Khalug]: " + CInt(damagemod));
			PrintTextAbove(attacker, "*enfurecido*");
		endif
		if (MobileWrapper(attacker).HasAbility(ABILITY_ID::FURIA))
			if (GetObjProperty(attacker, "metamorfose") != error)
				damagemod := Cint(AP_GetSkill(attacker, THEOLOGY) / 20);
				damage += damagemod;
				DebugMsg(attacker, DEBUG_DAMAGE, "Dano de Habilidade[Furia Primitiva]: " + CInt(damagemod));
			endif
		endif
		if (GetCooldown(attacker, "FuriaPrimordial"))
			var inimigos := ListHostiles(attacker, 3, LH_FLAG_LOS);
			inimigos := inimigos.size();
			var damagemod := Cint(inimigos.size());
			damage += damagemod;
			DebugMsg(attacker, DEBUG_DAMAGE, "Dano de Habilidade[Furia Primordial]: " + CInt(damagemod));
		endif
		if (GetCooldown(attacker, "furiamortal"))
			var damagemod := Cint((AP_GetVitalMaximumValue(attacker, HITS) - AP_GetVital(attacker, HITS)) / 10);
			damage += damagemod;
			DebugMsg(attacker, DEBUG_DAMAGE, "Dano de Habilidade[Furia Mortal]: " + CInt(damagemod));
		endif
	endif

	if (MobileWrapper(attacker).HasAbility(ABILITY_ID::MARTIAL_ARTIST))
		var monk_weapons := array{ 0x0df0, 0x13f9, 0x0e8a, 0x0e81, 0x1F020, 0x27AD, 0x27AE, 0x27A7};
        var isArmor := array{"ArmaduraPesada", "ArmaduraMedia", "ArmaduraLeve"};
		var damagemod := Cint(AP_GetSkill(attacker, TACTICS) / 20);
        var i_config := ReadConfigFile(":*:itemdesc");
        foreach item in ListEquippedItems(attacker)
            var this_item := i_config[item.objtype];
            if (this_item.TipoProtecao in isArmor)
                SendSysMessageEx(attacker, "Você perde mobilidade por estar de armadura e consequentemente, dano.", SSM_INFO);
                damagemod := 0;
                break;
            endif
        endforeach
    	
		if (damagemod)
			if (attacker.weapon.objtype in monk_weapons)
				damage += damagemod;
				DebugMsg(attacker, DEBUG_DAMAGE, "Dano de Habilidade[Budoka]: " + CInt(damagemod));
			endif
		endif
	endif

	if (isRaging(defender))
		if (MobileWrapper(attacker).HasAbility(ABILITY_ID::FURIA))
			var damagemod := CInt(AP_GetSkill(attacker, TACTICS) / 20);
			damage += damagemod;
			DebugMsg(attacker, DEBUG_DAMAGE, "Dano de Habilidade[Burz]: " + CInt(damagemod));
		endif
	endif

	if (GetCooldown(attacker, "furia"))
		var damagemod := CInt(AP_GetSkill(attacker, TACTICS) / 20);
		if (MobileWrapper(attacker).HasAbility(ABILITY_ID::DESTRUIDOR))
			damagemod := damagemod * 2;
		endif
		damage += damagemod;
		DebugMsg(attacker, DEBUG_DAMAGE, "Dano de Habilidade[Furia]: " + CInt(damagemod));
	endif

	// TODO: Migrate Flanquear to V3 DamageController as a combat mechanic (+1d4 damage if behind)
	if(IsBehind(defender, attacker.x, attacker.y))
		var damagemod := RandomDiceRoll("1d4");
		damage += damagemod;
		DebugMsg(attacker, DEBUG_DAMAGE, "Dano de Habilidade[Flanquear]: " + CInt(damagemod));
	endif

	if (CInt(GetObjProperty(attacker, "#mataroumorrer")) > 0)
		var damagemod := RandomDiceRoll("1d4");
		damage += damagemod;
		DebugMsg(attacker, DEBUG_DAMAGE, "Dano de Habilidade[Matar ou Morrer]: " + CInt(damagemod));
	endif

	return damage;
endfunction

function DamageEPIC(attacker, defender, weapon, armor, damage, type := DMG_FORCED)
	var totaldamage;
	var attackInfo := struct{};
	DebugMsg(attacker, DEBUG_DAMAGE, $"Dano Base {type}: {damage}");

	if (type == DMG_FORCED)
		totaldamage := CInt(damage);
	else
		totaldamage := CInt(EpicCalculateDamage(attacker, defender, armor, damage, type));
	endif

	attackInfo.+damage := totaldamage;
	attackInfo.+type := type;

	// CheckDefenderHabsBeforeDamage(attacker, defender, attackInfo);
	// CheckAttackerHabsBeforeDamage(attacker, defender, attackInfo);

	DisplayDamage(defender, totaldamage, attacker);
	ApplyRawDamage(defender, totaldamage);
	SetLastDamageInfo(defender, totaldamage, type, attacker);

//FRAJOLAS Adicionado para o discordwebhook detectar dano de trap
if (totaldmg > 0)
    var lastHit := dictionary;

    if (source) // dano normal (npc/player)
        lastHit["attacker"] := source.serial;
    else // trap ou dano sem source
        lastHit["attacker"] := 0;
        lastHit["source"] := "trap";
    endif

    lastHit["amount"] := totaldmg;
    lastHit["time"] := polcore().systime;
    lastHit["type"] := type;

    SetObjProperty(defender, "lastHit", lastHit);
endif
//end FRAJOLAS

	// CheckDefenderHabsAfterDamage(attacker, defender, attackInfo);
	// CheckAttackerHabsAfterDamage(attacker, defender, attackInfo);

	if (defender.frozen)
		defender.frozen := 0;
	endif

	return totaldamage;
endfunction

function hasAntiMountedWeapon(attacker, defender)
	if (defender.warmode == 0)
		return 0;
	endif

	var defenderHasAntiMountedWeapon := 0;
	if (GetObjProperty(attacker, "charge_damage"))
		var cfg := ReadConfigFile(":*:itemdesc");
		var elem := FindConfigElem(cfg, defender.weapon.objtype);
		var weapons_crits := GetconfigStringArray(elem, "HitScripts");
		foreach crit in (weapons_crits)
			if (crit == ":combat:weaponHitScripts/AntiMountedHit")
				defenderHasAntiMountedWeapon := 1;
			endif
		endforeach
	endif

	return defenderHasAntiMountedWeapon;
endfunction

/**
 * EpicApplyDamageBase(attacker, defender, armor, damage, type)
 *
 * Purpose
 * Criado para modificar a aplicação de dano do combate sem influenciar nas demais aplicações de dano do shard.
 * Ver o doc das demais funções abaixo para entender melhor.
 * 
 * Parameters
 * attacker: Ref do atacante aplicando o dano.
 * defender: Ref do alvo atingido no dano.
 * armor: itemRef da peça atingida.
 * damage: Valor total do dano a ser abatido.
 * type: Tipo do elemento do dano.
 * 
 * Return value
 *
 */
function EpicApplyDamageBase(attacker, defender, weapon, armor, damage, type)
	SetScriptController(attacker);

	// Calcula desgaste nos equipamentos
	WearDownWeapon(attacker, weapon);
	WearDownArmor(defender, armor, damage, weapon);

	DamageEPIC(attacker, defender, weapon, armor, damage, type);
endfunction

/**
 * EpicApplyDamageElemental(attacker, defender, weapon, armor)
 *
 * Purpose
 * Criado para modificar a aplicação de dano do combate sem influenciar nas demais aplicações de dano do shard.
 * Ver o doc das demais funções abaixo para entender melhor.
 * 
 * Parameters
 * attacker: Ref do atacante aplicando o dano.
 * defender: Ref do alvo atingido no dano.
 * armor: itemRef da peça atingida.
 * damage: Valor total do dano a ser abatido.
 * type: Tipo do elemento do dano.
 * 
 * Return value
 *
 */
function EpicApplyDamageElemental(attacker, defender, weapon, armor, damages)
	SetScriptController(attacker);

	if (damages.FireDamage > 0)
		WearDownArmor(defender, armor, damages.FireDamage, weapon);
		DamageEPIC(attacker, defender, weapon, armor, damages.FireDamage, DMG_FIRE);
	endif
	if (damages.ColdDamage > 0)
		WearDownArmor(defender, armor, damages.ColdDamage, weapon);
		DamageEPIC(attacker, defender, weapon, armor, damages.ColdDamage, DMG_COLD);
	endif
	if (damages.PoisonDamage > 0)
		WearDownArmor(defender, armor, damages.PoisonDamage, weapon);
		DamageEPIC(attacker, defender, weapon, armor, damages.PoisonDamage, DMG_POISON);
	endif
	if (damages.EnergyDamage > 0)
		WearDownArmor(defender, armor, damages.EnergyDamage, weapon);
		DamageEPIC(attacker, defender, weapon, armor, damages.EnergyDamage, DMG_ENERGY);
	endif
endfunction

/**
 * EpicCalculateDamage(attacker, defender, armor, damage, type)
 *
 * Purpose
 * Criado para modificar o cálculo de dano do combate sem influenciar nos demais cálculos de dano do shard.
 * Agora o cálculo de dano é baseado na região atingida.
 * Ver o doc das demais funções abaixo para entender melhor.
 * 
 * Parameters
 * attacker: Ref do atacante.
 * defender: Ref do alvo atingido no dano.
 * armor: itemRef da peça atingida.
 * damage: Valor total do dano a ser abatido.
 * type: Tipo do elemento do dano.
 * 
 * Return value
 *
 */
function EpicCalculateDamage(attacker, defender, armor, damage, type)
    var resist := 0;
    var totaldamage := damage;
    var regiao := struct;
	var zones := CS_GetArmorZone(armor);

    if (defender.npctemplate)
        resist := getResistNPCByType(defender, type, armor, attacker.weapon);
    else
        if (!zones || zones.size() == 0)
            zones := array{"Body"}; 
        endif
        
        var zone_name := zones[1];
        
        if(zone_name == "Body" || zone_name == "Legs") 
            var zone_info := GetArmorInZone(defender, zone_name, type);
            foreach item in (zone_info.items)
                WearDownArmor(defender, item, damage, attacker.weapon);
                sleepms(2);
            endforeach
            
            resist := zone_info.total_resist;
        else
            resist := getResistArmorByType(armor, type, attacker, defender);
            WearDownArmor(defender, armor, damage, attacker.weapon);
        endif
    endif

    regiao.+zone := zones;
    if (!regiao.zone) regiao.zone := "Body"; endif
    regiao.+equip := armor.desc;
    if (!regiao.equip) regiao.equip := "Body"; endif

    DebugMsg(attacker, DEBUG_DAMAGE, "Região atingida: " + regiao.zone);
    DebugMsg(defender, DEBUG_DAMAGE, "Região atingida: " + regiao.zone);
    DebugMsg(attacker, DEBUG_DAMAGE, "Equipamento atingido: " + regiao.equip);
    DebugMsg(defender, DEBUG_DAMAGE, "Equipamento atingido: " + regiao.equip);
    DebugMsg(attacker, DEBUG_DAMAGE, $"Defesa {type}: {resist}");
    DebugMsg(defender, DEBUG_DAMAGE, $"Defesa {type}: {resist}");

    damage := damage-resist;
    damage := damage < 1 ? 1 : damage;

    DebugMsg(attacker, DEBUG_DAMAGE, $"Dano Final {type}: {damage}({totaldamage} - {resist})");
    DebugMsg(defender, DEBUG_DAMAGE, $"Dano Final {type}: {damage}({totaldamage} - {resist})");

    return CInt(damage);
endfunction

/**
 * getWeaponDamageType(attacker, weapon)
 *
 * Purpose
 * Verificar o dano da arma do atacante é Cortante, Perfurante ou Contundente.
 *
 * Parameters
 * attacker: mobRef do atacante
 * weapon: itemRef da arma do atacante
 *
 * Return value
 *
 */
function getWeaponDamageType(attacker, weapon)
	var attackType;
	if (attacker.npctemplate)
		// TODO: NPC, pegar a função que faz ele girar o tipo, caso encontre um mais forte.
		// var npcDamages := getNPCDamages(attacker);
		// NPCRotateAttackType(attacker);
		attackType := "CONTUSIVO";
	else
		attackType := weapon.getAttrBase().AttackType.Upper();
	endif
	
	case (Upper(attackType))
		"CORTANTE":	return DMG_SLASH;
		"PERFURANTE": return DMG_PIERCING;
		"CONTUSIVO": return DMG_BLUNT;

		default: return DMG_BLUNT;
	endcase
endfunction

/**
 * getResistPlayerByType(npc, type)
 *
 * Purpose
 * Pegar os valores da resistência da armadura do NPC.
 * Segue a mesma lógica do getResistPlayerByType(), porém o NPC não tem variação na armadura,
 * é como se todas as peças possuíssem o mesmo resist.
 * 
 * Parameters
 * npc: Ref do NPC
 * type: Tipo do elemento do dano
 * armor: itemRef da armadura pega no combatHook();
 * 
 * Return value
 *
 */
function getResistNPCByType(npc, type, armor := 0, weapon := 0)
	var resist;
	var npctemplate := NPC_ParseTemplateName(npc.npctemplate);
	var cfg := ReadConfigFile(":*:npcdesc");
	var npcResists := getNPCResists(npc);

	if (!armor || armor == error)
		if (npcResists == error)
			SysLog(npc.npctemplate +" -> "+npcResists);
			resist := CInt(cfg[npctemplate.template].PhysicalResist) + CInt(npc.resist_physical_mod);
		else
			case(type)
				// DMG_PHYSICAL: resist := CInt(cfg[npctemplate.template].PhysicalResist) + CInt(npc.resist_physical_mod); break;
				DMG_SLASH: resist := CInt(npcResists.SlashResist); break;
				DMG_PIERCING: resist := CInt(npcResists.PiercingResist); break;
				DMG_BLUNT: resist := CInt(npcResists.BluntResist); break;

				DMG_FIRE: resist := CInt(npcResists.FireResist); break;
				DMG_COLD: resist := CInt(npcResists.ColdResist); break;
				DMG_POISON: resist := CInt(npcResists.PoisonResist); break;
				DMG_ENERGY: resist := CInt(npcResists.EnergyResist); break;
			endcase
		endif
	else
		armor.setAttrBase(0);
		resist := getResistArmorByType(armor, type);
	endif

	return CInt(resist);
endfunction

/**
 *
 * getResistPlayerByType(player, type, armor)
 *
 * Purpose
 * Pegar os valores da resistência da armadura do player.
 * Agora o dano é abatido pelo valor da região da armadura, não mais pela soma de todos eles.
 * Ex.: Se DANO = 12 e o dano acertar o peitoral. Se resist de CHEST = 7. Dano final = 5.
 * 
 * Parameters
 * armor: itemRef da armadura atingida na função de região (ver combatHook)
 * type: Tipo do elemento do dano
 * 
 * Return value
 *
 */
function getResistArmorByType(armor, type, attacker := 0x0, defender := 0x0)
	var resist := 0;
	var armorResists := armor.getAttrBase();
	var armorResistsFabricacao := PegaValorFabricacao(armor, CARACTERISTICA_FABRICACAO_DEFESA); 

    if(!armor || armor == error)
        return 0;
    endif

	case(type)
		DMG_PHYSICAL:
			var valorFabricado :=  PegaValorFabricacao(armor, CARACTERISTICA_FABRICACAO_DEFESA); 
			resist := CInt(armor.PhysicalResist) + CInt(armor.resist_physical_mod) + Cint(GetObjProperty(defender, "phMod") + CInt(valorFabricado? valorFabricado : 0)); 
			break;
		DMG_SLASH:
			if (armorResists.SlashResist)
				var valorFabricado :=  armorResistsFabricacao[SUBCARACTERISTICA_FABRICACAO_CORTANTE].valor;
				resist := CInt(armorResists.SlashResist) + CInt(armorResists.SlashResistMod) + CInt(valorFabricado);
			else
				return getResistArmorByType(armor, DMG_PHYSICAL);
			endif
			break;

		DMG_PIERCING:
			if (armorResists.PiercingResist)
				var valorFabricado :=  armorResistsFabricacao[SUBCARACTERISTICA_FABRICACAO_PERFURANTE].valor;
				resist := CInt(armorResists.PiercingResist) + CInt(armorResists.PiercingResistMod) + CInt(valorFabricado);
			else
				return getResistArmorByType(armor, DMG_PHYSICAL);
			endif
			break;

		DMG_BLUNT:  
			if (armorResists.BluntResist)
				var valorFabricado :=  armorResistsFabricacao[SUBCARACTERISTICA_FABRICACAO_CONTUSAO].valor;
				resist := CInt(armorResists.BluntResist) + CInt(armorResists.BluntResistMod)  + CInt(valorFabricado);
			else
				return getResistArmorByType(armor, DMG_PHYSICAL);
			endif
			break;

		DMG_FIRE: resist := CInt(armorResists.FireResist) + CInt(armorResists.FireResistMod) + Cint(GetObjProperty(defender,"fire_protection")); break;
		DMG_COLD: resist := CInt(armorResists.ColdResist) + CInt(armorResists.ColdResistMod) + Cint(GetObjProperty(defender, "cold_protection")); break;
		DMG_ENERGY: resist := CInt(armorResists.EnergyResist) + CInt(armorResists.EnergyResistMod) + Cint(GetObjProperty(defender, "energy_protection" )); break;
		DMG_POISON: resist := CInt(armorResists.PoisonResist) + CInt(armorResists.PoisonResistMod) + Cint(GetObjProperty(defender, "poison_protection")); break;
	endcase

	return CInt(resist);
endfunction

function destroycraft(potion, who)
    if (!CanDestroyCraft(who))
        SendSysMessageEx(who, "Voce nao pode destruir nada sem um GM online", SSM_FAIL);
        return 0;
    endif

	foreach item in (ListItemsNearLocation(who.x, who.y, who.z, 8, who.realm))
        if (CheckLosAt(who, item.x, item.y, item.z))
            if (GetObjProperty(item, "crafteracc"))
                var roll := RandomInt(100)+1;
                if (roll > 75)
                    if (!GetObjProperty(item, "LockId"))
                        createitematlocation(item.x, item.y, item.z, 3119, 1, item.realm);
                        sleepms(10);
                        DestroyItem(item);
                    endif
                endif
            endif
        endif
	endforeach
endfunction

function CanDestroyCraft(who)
    foreach x in EnumerateOnlineCharacters()
            if (x.cmdlevel > 0)
                SendSysMessageEx(x, who.acctname+" quer destruir coisas!", SSM_REQUEST);
                return 1;
            endif
    endforeach
    return 0;
endfunction

function CalculateTotalBaseDamage(attacker, defender, rawdamage)
	var damage := rawdamage;

	// Aplica bonus de dano
	damage := CalcDamageStats(attacker, damage);
	damage := CalcDamageWeapon(attacker, damage);
	damage := CalcDamageSpecial(attacker, defender, damage);
	damage := CalcDamageHabs(attacker, defender, damage);
	
	// Aplica redutores e penalidades de dano
	damage := CalcDamageReducer(attacker, damage);
	damage := CInt(damage);

	if (damage < 1)
		damage := 1;
		DebugMsg(attacker, DEBUG_DAMAGE, "Dano Minimo Garantido ! (1)");
	endif

	// DebugMsg(attacker, DEBUG_DAMAGE, "Dano Base: " + damage);

	return damage;
endfunction

function CalculateDamageElemental(attacker, defender, weapon, armor)
	var damages := struct;

	//TODO: Pensar no caso de NPC humanoide que usa arma...
	if (attacker.npctemplate) // Se for NPC, o cálculo é pelo .cfg
		damages := getNPCElemDamages(attacker);
	else
		if (weapon.damage_fire_mod)
			damages.+FireDamage := weapon.damage_fire_mod;
		endif
		if (weapon.damage_cold_mod)
			damages.+ColdDamage := weapon.damage_cold_mod;
		endif
		if (weapon.damage_poison_mod)
			damages.+PoisonDamage := weapon.damage_poison_mod;
		endif
		if (weapon.damage_energy_mod)
			damages.+EnergyDamage := weapon.damage_energy_mod;
		endif
	endif

	return damages;
endfunction

function CalcDamageStats(attacker, damage)
	var stat;
	var elem := attacker.weapon.getCfgElem();

	SkillCheck(attacker, TACTICS, -1);
	var tacticsmod := CInt(AP_GetSkill(attacker, TACTICS)/10);
	damage += tacticsmod;

	stat := STRENGTH;
	if (attacker.weapon.isRanged())
		stat := DEXTERITY;
	endif

	var statmod;
	if (elem.Throwable)
		statmod := CInt(AP_GetStat(attacker, DEXTERITY)) + CInt(AP_GetStat(attacker, STRENGTH) / 2);
	else
		statmod := CInt(AP_GetStat(attacker, stat));
	endif

	statmod := (statmod/* -50 */) / 10;
	damage += statmod;
	DebugMsg(attacker, DEBUG_DAMAGE, $"Bônus de {stat}: {statmod}");
	DebugMsg(attacker, DEBUG_DAMAGE, $"Bônus de Tactics: {tacticsmod}");

	return damage;
endfunction

function CalcDamageWeapon(attacker, damage)
	damage += Cint(GetObjProperty(attacker.weapon, "dmgbonus")) + Cint(GetObjProperty(attacker.weapon, "#dmgbonus"));

	if (GetObjProperty(attacker, "armapredileta") == attacker.weapon.objtype)
		damage += Cint(AP_GetSkill(attacker, TACTICS) / 25);
	endif

	/*var sharpened_weapon := Cint(GetObjProperty(attacker.weapon, "Aprimoramentos")["Arma Afiada"]);
	if(sharpened_weapon)
		damage += sharpened_weapon;
	endif*/

	var danoFabricadoTotal := 0;
	danoFabricadoTotal += PegaValorFabricacao(attacker.weapon,CARACTERISTICA_FABRICACAO_DANO);
	var SubCaracteristicas := PegaSubValoresFabricacao(attacker.weapon,CARACTERISTICA_FABRICACAO_DANO);
		//TODO Adicionar os danos de contusao, cortante e perfurante quando der.
	
	DebugMsg(attacker, DEBUG_DAMAGE, "Modificador Fabricacao: " + CInt(danoFabricadoTotal));
	DebugMsg(attacker, DEBUG_DAMAGE, "Dano: " + CInt(damage));

	return damage;
endfunction



function CalcDamageSpecial(attacker, defender, damage)
	// Check for blessed weapon (spell cooldown or priest rune)
	var hasPriestRune := GetObjProperty(attacker.weapon, "priestrune");
	if (hasPriestRune)
		var categoriaDefender := NPC_GetNPCConfig(defender.npctemplate).Environment;
		var cursed_creatures := {"undead", "demons"};
		if (defender.npctemplate)
			if (lower(categoriaDefender) in cursed_creatures)
				var damagemod := CInt(GetObjProperty(attacker.weapon, "blessedbonus"));
				damage += damagemod;

				if (hasPriestRune)
					DebugMsg(attacker, DEBUG_DAMAGE, "Dano Especial[Holy Weapon Rune]: " + CInt(damagemod));
				else
					DebugMsg(attacker, DEBUG_DAMAGE, "Dano Especial[Bless Weapon]: " + CInt(damagemod));
				endif
			endif
		endif
	endif

	var backStab := CDbl(GetObjProperty(attacker, "#backstab"));
	if (backStab)
		var damagemod := 1;
		damage += damagemod;
		EraseObjProperty(attacker, "#backstab");
		DebugMsg(attacker, DEBUG_DAMAGE, "Dano Especial[Backstab]: " + CInt(damagemod));
	endif

	if(GetObjProperty(defender, "apodrecer"))
		var damagemod := RandomDiceRoll("1d4");
		damage += damagemod;
		DebugMsg(attacker, DEBUG_DAMAGE, "Dano Especial[Apodrecer]: " + CInt(damagemod));
	endif

	return damage;
endfunction

function CalcDamageReducer(attacker, damage)
	damage += CInt(GetObjProperty(attacker, "dmgbonus") + GetObjProperty(attacker, "#dmgbonus"));

	var damagemod := CDbl(GetObjProperty(attacker, "dmgreduction"));
	if (damagemod > 0)
		damage := damage*damagemod;
		DebugMsg(attacker, DEBUG_DAMAGE, $"Redutor de Dano: {CDbl((1-damagemod)*100)}%");
	endif

	if (GetObjProperty(attacker, "chardata").trait == "Defensor Anão")
		var penalty := 0.7;
		var damagemod := damage * (1-penalty);
		damage -= damagemod;
		DebugMsg(attacker, DEBUG_DAMAGE, $"Penalidade Defensor Anão (30%): {damagemod}");
	endif

	return damage;
endfunction



function ApplyCriticalHit(attacker, defender, weapon, armor, basedamage, rawdamage)
	if (attacker.GetProp("critdelay") > ReadGameClock())
		return 0;
	endif

	var weapon_scripts := array{};

	var pet_instinto := attacker.GetProp("Group_savage"); // animal tamado com a hab Instinto Selvagem para adicionad criticos diferentes
	var pet_critic := attacker.GetProp("Critic");
	var pet_selvagem := attacker.GetProp("Selvagem"); // verifica se é animal selvagem - nunca tamado, para dar dano critico

	var npcCfg := CFGFile("combat", "npcdesc").SetElementName(attacker.npctemplate);
	var tamingCfg := CFGFile("taming", "taming").SetElementName(pet_critic);
	var weaponCfg := CFGFile("*", "itemdesc").SetElementName(weapon.objtype);
	
	var pet_txt := tamingCfg.GetPropertyValue("Text");
	var tem_critico := npcCfg.GetPropertyValue("CriticalHitScripts"); // Se tem critico no npcdesc definido

	if (!attacker.ip && weapon.IsA(POLCLASS_WEAPON) && !pet_instinto && !pet_selvagem)
		if (tem_critico == {})
			weapon_scripts := weaponCfg.GetPropertyValue("CriticalHitScripts");
		else
			weapon_scripts := tem_critico;
		endif
	elseif (pet_instinto == 1 || pet_selvagem == 1)
		weapon_scripts := tamingCfg.GetPropertyValue("CriticalHitScripts");
		Message(pet_txt[1]).SendAbove(attacker);
	else
		weapon_scripts := weaponCfg.GetPropertyValue("CriticalHitScripts");
	endif
	// Possivel mudar o critico de npc's. Os npcs só tem uma arma, a Intrinsic Weapon, mesmo que alguma esteja equipada.

	var criticoFabricacao := CInt(PegaValorFabricacao(attacker.weapon, CARACTERISTICA_FABRICACAO_CRITICO));
	var reducaoCriticoFabricacao := CInt(PegaValorFabricacao(attacker.weapon, CARACTERISTICA_FABRICACAO_REDUCAOCRITICO));
	var can_crit := criticoFabricacao > 0 || attacker.npctemplate || weapon == FIST;

	if (weapon_scripts.size() > 0 && can_crit)
		var multiplicador := 7;
		reducaoCriticoFabricacao := reducaoCriticoFabricacao == OT_LONG ? reducaoCriticoFabricacao : 0;
		multiplicador -= (criticoFabricacao - reducaoCriticoFabricacao);
		
		var critdelay := RandomIntMinMax(15 * multiplicador, 130 + (15 * multiplicador));

		Message("*CRÍTICO*").SetColor(2118).SendAbove(attacker);
		SetObjProperty(attacker, "critdelay", ReadGameClock() + critdelay);

		var isDwarverDefender := GetObjProperty(defender, "chardata").trait == "Defensor Anão";
		if (GetCooldown(defender, "vitalidadeinabalavel") > 0 || isDwarverDefender)
			Message("Você ignora o crítico!").SetColor(SSM_INFO).SendSys(defender);
			Message("O defensor ignorou o crítico!").SetColor(SSM_FAIL).SendSys(attacker);
			return 1;
		endif
	elseif (!can_crit)
		// Message("Apenas armas elaboradas causam crítico").SetColor(SSM_FAIL).SendSys(attacker);
		return 1;
	endif

	foreach hitscript in weapon_scripts
		var script := start_script(
			hitscript,
			array{ attacker, defender, weapon, armor, basedamage, rawdamage }
		);
		if (!script || script.errortext)
			Message("*Attacker* Weapon script error starting [" + hitscript + "] :" + script.errortext).SendSys(attacker);
			Message("*Attacker* Weapon script error starting [" + hitscript + "] :" + script.errortext).SendSys(defender);
		endif
	endforeach
endfunction