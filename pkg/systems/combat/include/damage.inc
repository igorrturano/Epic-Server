/* $Id$
 *
 * Purpose
 * Handles damages and resistances to damage types
 *
 */

include ":tn:cooldown";
include ":attributes:attributes";
include ":blood:blood";
include ":fls_core:packets";
include "include/epicCombatUtils";
include "include/facings";
include ":timedScripts:timedScripts";
include ":commands:debugsystem";
include ":combat:armorZones";
include ":combat:combat";
include ":combat:Habilidades/include/habilidades";
include ":equipsys:utils";
include ":equipsys:props";
include "include/epicNPCUtils";
include "weardown";

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

function HealFLS(targ, amt)
	if (!GEtObjProperty(targ, "nocure"))
		HealDamage(targ, amt);
	else
		SendSysMessageEx(targ, "Você não pode ser curado.", SSM_FAIL);
	endif
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
			if (CheckBastardSword(attacker)) // Usando espada bastarda com as duas mãos
				weaponDice := "1d10";
			else
				weaponDice := weapon.getMainDamageDice();
			endif
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

function DamageFLS(defender, dmg, type := DMG_FORCED, source := 0, spell := "") // Mantido, apesar de criar o CombatDamageEPIC(), para não dar conflito com os demais danos do shard após criar o novo sistema de danos por região
	var totaldmg;

	if (type == DMG_FORCED)
		totaldmg := CInt(dmg);
	else
		totaldmg := CInt(CalculateDamage(defender, type, dmg));
	endif

	if (source)
		SetScriptController(source);
	endif

	// Checa as habilidades antes do dano
	totaldmg := CheckHab_Martir(0, defender, totaldmg, type);
	totaldmg := CheckHab_MantoRothen(0, defender, totaldmg, type);

	// Aplica o dano
	DisplayDamage(defender, totaldmg, source);
	ApplyRawDamage(defender, totaldmg);
	SetLastDamageInfo(defender, totaldmg, type, source, spell);

	// Checa as habilidades depois do dano
	CheckHab_RecuperacaoSalvadora(0, defender, totaldmg, type);
	CheckHab_Vinganca(0, defender, totaldmg, type);

	if (defender.frozen)
		defender.frozen := 0;
	endif

	return totaldmg;
endfunction

function DamageEPIC(attacker, defender, weapon, armor, damage, type := DMG_FORCED)
	var totaldamage;
	DebugMsg(attacker, DEBUG_DAMAGE, $"Dano Base {type}: {damage}");

	if (type == DMG_FORCED)
		totaldamage := CInt(damage);
	else
		totaldamage := CInt(EpicCalculateDamage(attacker, defender, armor, damage, type));
	endif

	totaldamage := CheckDefenderHabsBeforeDamage(attacker, defender, totaldamage, type);

	DisplayDamage(defender, totaldamage, attacker);
	ApplyRawDamage(defender, totaldamage);
	SetLastDamageInfo(defender, totaldamage, type, attacker);

	CheckDefenderHabsAfterDamage(attacker, defender, totaldamage, type);

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

	if (damages.FireDamage)
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

    if(!armor || armor == error)
        return 0;
    endif

	case(type)
		DMG_PHYSICAL: resist := CInt(armor.PhysicalResist) + CInt(armor.resist_physical_mod) + Cint(GetObjProperty(defender, "phMod")); break;
		DMG_SLASH:
			if (armorResists.SlashResist)
				resist := CInt(armorResists.SlashResist) + CInt(armorResists.SlashResistMod);
			else
				return getResistArmorByType(armor, DMG_PHYSICAL);
			endif
			break;

		DMG_PIERCING:
			if (armorResists.PiercingResist)
				resist := CInt(armorResists.PiercingResist) + CInt(armorResists.PiercingResistMod);
			else
				return getResistArmorByType(armor, DMG_PHYSICAL);
			endif
			break;

		DMG_BLUNT:  
			if (armorResists.BluntResist)
				resist := CInt(armorResists.BluntResist) + CInt(armorResists.BluntResistMod);
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

/**
 * CheckDefenderHabsBeforeDamage(byref defender, byref damage)
 *
 * Purpose
 * Checar as habilidades que são ativadas antes de o defensor tomar o dano;
 * Criado com byRef pois algumas habilidades modificadam quem é o defensor, dano, dentro outras possibiliades.
 * 
 * Parameters
 * attacker: Ref do atacante.
 * defender: Ref do alvo que recebe o dano.
 * damage: Valor (em int) do dano.
 * 
 * Return void
 *
 */
function CheckDefenderHabsBeforeDamage(byref attacker, byref defender, damage, type)
	var resist := damage;
	
	CheckHab_Martir(attacker, defender, damage, type);
	CheckHab_MantoRothen(attacker, defender, damage, type);
	CheckHab_KaniNoSutairu(attacker, defender, damage, type);
	CheckHab_Exoesqueleto(attacker, defender, damage, type);
	CheckHab_EspecialistaEscudos(attacker, defender, damage, type);
	CheckHab_Furia(attacker, defender, damage, type);
	CheckHab_ResilienciaBarbara(attacker, defender, damage, type);
	CheckHab_VinculoElemental(attacker, defender, damage, type);
	CheckHab_Backstab(attacker, defender, damage, type);
	CheckHab_Charge(attacker, defender, damage, type);

	resist := resist - damage; // Caso alguma hab precise saber quanto de dano foi reduzido.

	// var totaldamage := CInt(damage-resist); 
	return damage < 1 ? 1 : damage; // Garante dano mínimo = 1.
endfunction

/**
 * CheckDefenderHabsAfterDamage(byref defender, byref damage)
 *
 * Purpose
 * Checar as habilidades que são ativadas depois de o defensor tomar o dano;
 * Criado com byRef pois algumas habilidades modificadam quem é o defensor, dano, dentro outras possibiliades.
 * 
 * Parameters
 * attacker: Ref do atacante.
 * defender: Ref do alvo que recebe o dano.
 * damage: Valor (em int) do dano.
 * 
 * Return void
 *
 */
function CheckDefenderHabsAfterDamage(byRef attacker, byRef defender, damage, type)
	CheckHab_RecuperacaoSalvadora(attacker, defender, damage, type);
	CheckHab_Vinganca(attacker, defender, damage, type);
	CheckHab_ChainOfPain(attacker, defender, damage, type);
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

function IsTwoHandedWeapon(item)
	if (!item.isa (POLCLASS_WEAPON))
		return 0;
	endif
	var weaponcfgfile := ReadConfigFile (":*:itemdesc");
	var weapon_elem := FindConfigElem (weaponcfgfile, item.objtype);
	if (weapon_elem)
		if (weapon_elem.Two_Handed)
			return 1;
		endif
	endif
	return 0;
endfunction

function IsThrowingWeapon(item)
	if (!item.isa (POLCLASS_WEAPON))
		return 0;
	endif
	var weaponcfgfile := ReadConfigFile (":*:itemdesc");
	var weapon_elem := FindConfigElem (weaponcfgfile, item.objtype);
	if (weapon_elem)
		if (weapon_elem.Throwing)
			return 1;
		endif
	endif
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

// function getElementalDamageNPC(npc, type)
// 	var damages := struct;
// 	var npctemplate := NPC_ParseTemplateName(npc.npctemplate);
// 	var cfg := ReadConfigFile(":*:npcdesc");



// 	return damages;
// endfunction

function CalcDamageStats(attacker, damage)
	var stat;
	var elem := getItemCfgElem(attacker.weapon);

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

	var sharpened_weapon := Cint(GetObjProperty(attacker.weapon, "Aprimoramentos")["Arma Afiada"]);
	if(sharpened_weapon)
		damage += sharpened_weapon;
	endif

	DebugMsg(attacker, DEBUG_DAMAGE, "Dano: " + CInt(damage));
	DebugMsg(attacker, DEBUG_DAMAGE, "Arma Afiada: " + CInt(sharpened_weapon));

	return damage;
endfunction

function CalcDamageHabs(attacker, defender, damage)
	if (GetObjProperty(attacker, "#tiropenetrante")) // TODO: Pensar o que fazer aqui, já que perfura armadura, talvez jogar pra função que aplica o dano das habs, ao invés de entrar no cálculo aqui
		EraseObjProperty(attacker, "#tiropenetrante");
        var damagemod := RandomDiceRoll("1d4");
		damage += damagemod;
		// DamageFLS(defender, cint(damage-rawdamage*2), DMG_FORCED, attacker);
		DebugMsg(attacker, DEBUG_DAMAGE, "Dano de Habilidade[Tiro Penetrante]: " + CInt(damagemod));
	endif

	if (TemHabilidade(attacker, "Brigador de Taverna") && attacker.weapon.objtype == 0x1F020)
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

	if (TemHabilidade(attacker, "Taticas Arcanas"))
		var manaatual := AP_GetVital(attacker, "MANA");
		var manatotal := AP_GetVitalMaximumValue(attacker, "MANA");
		if (manaatual >= manatotal / 2)
			var damagemod := CInt(manaatual / 20);
			damage += damagemod;
			DebugMsg(attacker, DEBUG_DAMAGE, "Dano de Habilidade[Taticas Arcanas]: " + CInt(damagemod));
		endif
	endif

	if (GetCooldown(attacker, "lobosolitario"))
		if (ListHostiles(attacker, 3, LH_FLAG_LOS) > 2)
			var damagemod := CInt(AP_GetSkill(attacker, SURVIVAL) / 20);
			damage += damagemod;
			DebugMsg(attacker, DEBUG_DAMAGE, "Dano de Habilidade[Lobo Solitário]: " + CInt(damagemod));
		endif
	endif

	if (TemHabilidade(attacker, "Precisao Cirurgica"))
		var damagemod := CInt(AP_GetSkill(attacker, SCIENCE) / 20);
		damage += damagemod;
		DebugMsg(attacker, DEBUG_DAMAGE, "Dano de Habilidade[Precisão Cirurgica]: " + CInt(damagemod));
	endif

    if (TemHabilidade(attacker, "Arsenal Sombrio"))
        var damagemod := CInt(AP_GetSkill(attacker, SNEAK) / 20);
        if (isAssassinWeapon(attacker))
			damage += damagemod;
			DebugMsg(attacker, DEBUG_DAMAGE, "Dano de Habilidade[Arsenal Sombrio]: " + CInt(damagemod));
        endif
    endif

	if (TemHabilidade(attacker, "Estilo Mao Livre"))
		if (attacker.weapon.attribute == "One_Handed" && !attacker.shield)
			var damagemod := CInt(AP_GetSkill(attacker, TACTICS) / 25);
			damage += damagemod;
			DebugMsg(attacker, DEBUG_DAMAGE, "Dano de Habilidade[Estilo Mao Livre]: " + CInt(damagemod));
		endif
	endif

	if (TemHabilidade(attacker, "Ainalaith"))
		var damagemod := 0;
		var arrow_effects := GetObjProperty(attacker, "#shooting_effects");
		if (!arrow_effects)
			// return 0; // No effects recorded, exit early
		endif

		var arrow_types := array{"fire_arrow", "cold_arrow", "energy_arrow"};
		foreach arrow_type in arrow_types
			if (arrow_effects[arrow_type] >= 3)
				case (arrow_type)
					"fire_arrow":
						TS_StartTimer(defender, "queimar", 10);
						break;

					"cold_arrow":
						TS_StartTimer(defender, "lentidao", 10);
						break;

					"energy_arrow":
						damagemod += Cint(AP_GetSkill(attacker, ARCANA) / 20);
						break;
				endcase

				arrow_effects[arrow_type] := 0;
				SetObjProperty(attacker, "#shooting_effects", arrow_effects);
			endif
		endforeach

		damage += damagemod;
		DebugMsg(attacker, DEBUG_DAMAGE, "Dano de Habilidade[Ainalaith]: " + CInt(damagemod));
	endif

	if (TemHabilidade(attacker, "Inimigo Predileto"))
		var npctemplate := NPC_ParseTemplateName(defender.npctemplate);
		var cfg := ReadConfigFile(":*:npcdesc");
		var inimigo := GetObjProperty(attacker, "InimigoPredileto");

		if (cfg[npctemplate.template].Category == inimigo)
			var damagemod := Cint(AP_GetSkill(attacker, SURVIVAL) / 20);
			damage += damagemod;
			DebugMsg(attacker, DEBUG_DAMAGE, "Dano de Habilidade[Inimigo Predileto]: " + CInt(damagemod));
		endif
    elseif (TemHabilidade(attacker, "Consagracao Divina"))
        var npctemplate := NPC_ParseTemplateName(defender.npctemplate);
		var cfg := ReadConfigFile(":*:npcdesc");
		var inimigo := array{"Undead"};

		if (cfg[npctemplate.template].Category in inimigo)
			var damagemod := Cint(AP_GetSkill(attacker, THEOLOGY) / 20);
			damage := damagemod;
			DebugMsg(attacker, DEBUG_DAMAGE, "Dano de Habilidade[Consagracao Divina]: " + CInt(damagemod));
		endif
	elseif (TemHabilidade(attacker, "Sigilo de Protecao"))
        var npctemplate := NPC_ParseTemplateName(defender.npctemplate);
		var cfg := ReadConfigFile(":*:npcdesc");
		var inimigo := array{"Demon"}; 

		if (cfg[npctemplate.template].Category in inimigo)
			var damagemod := Cint(AP_GetSkill(attacker, THEOLOGY) / 20);
			damage := damagemod;
			DebugMsg(attacker, DEBUG_DAMAGE, "Dano de Habilidade[Sigilo de Protecao]: " + CInt(damagemod));
		endif
	elseif (TemHabilidade(attacker, "Oleo do Cacador"))
		var npctemplate := NPC_ParseTemplateName(defender.npctemplate);
		var cfg := ReadConfigFile(":*:npcdesc");
		var inimigo := GetObjProperty(attacker, "Óleo");
		if (cfg[npctemplate.template].Category == inimigo)
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

	if (temHabilidade(attacker, "Instinto de Sobreviência") && AP_GetVital(attacker, HITS) <= (AP_GetSkill(attacker, THEOLOGY) / 2))
		if (GetObjProperty(attacker, "metamorfose") != error)
			var damagemod := Cint(AP_GetSkill(attacker, THEOLOGY) / 20);
			damage += damagemod;
			DebugMsg(attacker, DEBUG_DAMAGE, "Dano de Habilidade[Arsenal Sombrio]: " + CInt(damagemod));
			PrintTextAbove(attacker, "*enfurecido*");
		endif
	endif

	if (IsRaging(attacker))
		var damagemod;
		if (temHabilidade(attacker, "Nazg Khalug") || attacker.hasRage())
			damagemod := Cint(AP_GetSkill(attacker, TACTICS) / 20);
			damage += damagemod;
			DebugMsg(attacker, DEBUG_DAMAGE, "Dano de Habilidade[Nazg Khalug]: " + CInt(damagemod));
			PrintTextAbove(attacker, "*enfurecido*");
		endif
		if (temHabilidade(attacker, "Furia Primitiva"))
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

	if (TemHabilidade(attacker, "Budōka"))
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
		if (temHabilidade(attacker, "Burz"))
			var damagemod := CInt(AP_GetSkill(attacker, TACTICS) / 20);
			damage += damagemod;
			DebugMsg(attacker, DEBUG_DAMAGE, "Dano de Habilidade[Burz]: " + CInt(damagemod));
		endif
	endif

	if (GetCooldown(attacker, "furia"))
		var damagemod := CInt(AP_GetSkill(attacker, TACTICS) / 20);
		if (TemHabilidade(attacker, "Destruidor"))
			damagemod := damagemod * 2;
		endif
		damage += damagemod;
		DebugMsg(attacker, DEBUG_DAMAGE, "Dano de Habilidade[Furia]: " + CInt(damagemod));
	endif

	if(TemHabilidade(attacker, "Flanquear"))
		if(IsBehind(defender, attacker.x, attacker.y))
			var damagemod := RandomDiceRoll("1d4");
			damage += damagemod;
			DebugMsg(attacker, DEBUG_DAMAGE, "Dano de Habilidade[Flanquear]: " + CInt(damagemod));
		endif
	endif

	if (CInt(GetObjProperty(attacker, "#mataroumorrer")) > 0)
		var damagemod := RandomDiceRoll("1d4");
		damage += damagemod;
		DebugMsg(attacker, DEBUG_DAMAGE, "Dano de Habilidade[Matar ou Morrer]: " + CInt(damagemod));
	endif

	return damage;
endfunction

function CalcDamageSpecial(attacker, defender, damage)
	if (GetCooldown(attacker.weapon, "blessweapon"))
		var categoriaDefender := NPC_GetNPCConfig(defender.npctemplate).Category;
		var cursed_creatures := {"undead", "demon"};
		if (defender.npctemplate)
			if (lower(categoriaDefender) in cursed_creatures)
				var damagemod := CInt(GetObjProperty(attacker.weapon, "blessedbonus"));
				damage += damagemod;
				DebugMsg(attacker, DEBUG_DAMAGE, "Dano Especial[Bless Weapon]: " + CInt(damagemod));
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

	var polida := GetObjProperty(attacker.weapon, "polida");
	if (!polida)
		var penalty := 0.8;
		var damagemod := damage * (1-penalty);
		damage -= damagemod;
		DebugMsg(attacker, DEBUG_DAMAGE, $"Penalidade polidez (20%): {damagemod}");
	else
		polida -= 1;
		if (polida < 1)
			EraseObjProperty(attacker.weapon, "polida");
			SendSysMessageEX(attacker, "Sua arma perdeu a polidez.", SSM_INFO);
		else
			SetObjProperty(attacker.weapon, "polida", polida);
		endif
	endif

	if (GetObjProperty(attacker, "chardata").trait == "Defensor Anão")
		var penalty := 0.7;
		var damagemod := damage * (1-penalty);
		damage -= damagemod;
		DebugMsg(attacker, DEBUG_DAMAGE, $"Penalidade Defensor Anão (30%): {damagemod}");
	endif

	return damage;
endfunction

/**
 * CheckDefenderHabsAfterDamage(byref defender, byref damage)
 *
 * Purpose
 * Calcular e aplicar o dano elemental de habilidade
 * 
 * Parameters
 * attacker: Ref do atacante.
 * defender: Ref do alvo que recebe o dano.
 * damageDice: string com o dado de dano
 * elementalType: O tipo de dano elemental
 * Return void
 *
 */
function ApplyDamageHabElementalWeapon( attacker, defender, damageDice, elementalType , divisor)
  var dmgBonus := RandomDiceRoll( damageDice);
  var totalDamage := CalculateDamage( defender, elementalType, dmgBonus );
  totalDamage := cint( Floor( totalDamage / divisor ) ); // Não tinha cargas suficiente, entao diminui a eficência do dano.
  DisplayDamage( defender, totalDamage, attacker );
  ApplyRawDamage( defender, totalDamage );
endfunction

function GetArmorInZone(mobile, zone_name, type)
    var result := struct{
        "items" := array{},
        "total_resist" := 0
    };
    
    var cfg_file := ReadConfigFile("::armrzone");
    if(!cfg_file)
        return result;
    endif
    
    var zone_elem := cfg_file[zone_name];
    if(!zone_elem)
        return result;
    endif
    
    var layers := GetConfigStringArray(zone_elem, "Layer");
    var layer_numbers := array{};
    foreach layer in layers
        layer_numbers.append(CInt(layer));
    endforeach
    
    var equipped_items := ListEquippedItems(mobile);
    
    foreach item in equipped_items
        if(item.layer in layer_numbers)
            result.items.append(item);
            var item_resist := getResistArmorByType(item, type);
            result.total_resist += item_resist;
        endif
        sleepms(2);
    endforeach
    
    return result;
endfunction
