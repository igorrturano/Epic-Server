/* $Id$
 *
 * Purpose
 * Handles damages and resistances to damage types
 *
 */

// use uo;
// use os;
// use math;
// use cfgfile;
// use util;

include ":tn:cooldown";
// include ":brainAI:npcUtil";
// include ":attributes:attributeConstants";
include ":attributes:attributes";
// include ":charactercreation:habilidades";
// include "include/say";
include ":blood:blood";
include ":fls_core:packets";
include "include/epicCombatUtils";
include "include/facings";
include ":timedScripts:timedScripts";
include "damage/damagebase";
include ":commands:debugsystem";
include ":combat:armorZones";

/*
 * http://uo.stratics.com/content/guides/resistance.shtml
 * These could be attributes, but not every shard will want to
 * follow OSI. This gives more freedom to make custom resistances
 * and remove existing ones more easily.
 */
enum DAMAGE_TYPES
	DMG_FORCED	:= -1, // Ignores resistance
	DMG_PHYSICAL	:= "Physical",
	DMG_FIRE	:= "Fire",
	DMG_COLD	:= "Cold",
	DMG_POISON	:= "Poison",
	DMG_ENERGY	:= "Energy"
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
		var cfg := ReadConfigFile(":brainai:npcdesc");
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
				DebugMsg(defender, DEBUG_NAME_DAMAGE, $"Defesa Física (Mod): {resist}({resistMod})");
				break;

			DMG_FIRE:
				resist := CInt(defender.resist_fire);
				resistMod := CInt(defender.resist_fire_mod);
				DebugMsg(defender, DEBUG_NAME_DAMAGE, $"Defesa Fire (Mod): {resist}({resistMod})");
				break;

			DMG_COLD:
				resist := CInt(defender.resist_cold);
				resistMod := CInt(defender.resist_cold_mod);
				DebugMsg(defender, DEBUG_NAME_DAMAGE, $"Defesa Cold (Mod): {resist}({resistMod})");
				break;

			DMG_ENERGY:
				resist := CInt(defender.resist_energy);
				resistMod := CInt(defender.resist_energy_mod);
				DebugMsg(defender, DEBUG_NAME_DAMAGE, $"Defesa Energy (Mod): {resist}({resistMod})");
				break;

			DMG_POISON:
				resist := CInt(defender.resist_poison);
				resistMod := CInt(defender.resist_poison_mod);
				DebugMsg(defender, DEBUG_NAME_DAMAGE, $"Defesa Poison (Mod): {resist}({resistMod})");
				break;
		endcase
	endif

	if (type == DMG_PHYSICAL)
        if (TemHabilidade(defender, "Kani no Sutairu"))
            resist += RandomDiceRoll("1d4");
			// resist := CDbl(AP_GetSkill(defender, TACTICS) / 200); // Ganha até 50% de bonus
        endif
		if (TemHabilidade(defender, "Exoesqueleto"))
			resist += 2;
		endif
		if (TemHabilidade(defender, "Especialista em Escudos") && (defender.shield).IsA(POLCLASS_ARMOR) ) // Se tiver a hab e usar escudo
			resist += RandomDiceRoll("1d4");
			// resist := resist + (AP_GetTrueSkill(defender,PARRY)/1000); // Ganha até +10% de bonus
		endif
		if (GetCooldown(defender, "furia") && TemHabilidade(defender, "Resiliência Bárbara")) // Antigo Pele de Rocha
			//TODO: Pensar no valor aqui (coloquei só pra não esquecer)
			resist += RandomDiceRoll("1d4");
        elseif (TemHabilidade(defender, "Resiliência Bárbara"))
			//TODO: Pensar no valor aqui (coloquei só pra não esquecer)
			resist += RandomDiceRoll("1d4");
			
		//TODO: Pensar nesse cálculo aqui
		elseif (TemHabilidade(defender, "Vínculo Elemental"))
			var arcane_armor := AP_GetSkill(defender, MAGICLORE) / 100 * 0.4;
			if (GetCooldown(defender, "armor_drain") > 0)
				arcane_armor += AP_GetSkill(defender, MAGICLORE) / 100 * 0.3;
				PlayObjectCenteredEffectEx(defender, 0x5454, 20, 20, 0);
			endif
			if (resist < arcane_armor)
				resist := arcane_armor;
				PlayObjectCenteredEffectEx(defender, 0x5487, 1, 8, 0);
			endif
		endif
	endif

	dmg -= resist;

	if (dmg < 1)
		dmg := 1;
	endif

	return dmg;
	// return (dmg - (dmg*resist));
endfunction

function CalculateTotalDamage(defender, basedamage, attacker, dmglist := 0)
	var damage := 0;

	if (!dmglist)
		dmgList := getNewDamageList(attacker);
	endif

	foreach key in (dmgList.keys())
		damage += CalculateDamage(defender, key, basedamage-dmglist[key]);
		// dmg += CalculateDamage(defender, key, basedamage*dmglist[key]);
	endforeach

	if (damage < 1)
		damage := 1;
	endif

	return CInt(damage);
endfunction

function CalculateRawDamage(attacker, weapon)
	var dmg;

	if (attacker.npctemplate)
		var npctemplate := NPC_ParseTemplateName(attacker.npctemplate);
		var cfg := ReadConfigFile(":brainai:npcdesc");
		dmg := cfg[npctemplate.template].AttackDamage;
		dmg := RandomDiceRoll(dmg);
	else
		var weaponcfg := ReadConfigFile(":*:itemdesc");
		var weaponelem := FindConfigElem(weaponcfg, weapon.objtype);
		if (GetObjProperty(attacker, "metamorfose") != error)
      		dmg := RandomDiceRoll(GetObjProperty(attacker, "dicedmg"));
		elseif(GetObjProperty(attacker, "pilotando") != error)
			dmg := RandomDiceRoll(GetObjProperty(attacker, "dicedmg"));
		else
    		dmg := RandomDiceRoll(CStr(weaponelem.Damage));
    	endif

		dmg += weapon.dmg_mod;
	endif

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

function DamageFLS(defender, dmg, type := DMG_FORCED, source := 0, spell := "")
	var totaldmg;

	if (type == DMG_FORCED)
		totaldmg := CInt(dmg);
	else
		totaldmg := CInt(CalculateDamage(defender, type, dmg));
	endif

	if (source)
		SetScriptController(source);
	endif

	if (GetObjProperty(defender, "#martir"))
		var martir := SystemFindObjectBySerial(GetObjProperty(defender,"martirserial"));
		if (martir && !martir.dead)
			defender := martir;
		endif
	endif

	if (GetObjProperty(defender, "#mantorothen"))
		var healed := cint(totaldmg*0.3);
		totaldmg := totaldmg - healed;
		var party := defender.party;
		var amthealed := cint(healed/party.members.size());
		if (amthealed < 1)
			amthealed := 1;
		endif
		foreach member in (party.members)
			HealDamage(member, amthealed);
		endforeach
	endif

	DebugMsg(source, DEBUG_NAME_DAMAGE, "Dano Final: {}({} - {})".format(totaldmg, dmg, (dmg-totaldmg)));
  	DisplayDamage(defender, totaldmg, source);
	ApplyRawDamage(defender, totaldmg);

	if (isBloodied(defender))
		RecalcVitals(defender);
		MakeBleed(defender);
		if (getObjProperty(defender, "#recuperacaosalvadora"))
			EraseObjProperty(defender, "#recuperacaosalvadora");
			SendSysMessageEx(defender, "Uma recuperacao salvadora salvou voce.", SSM_INFO);
			HealDamage(defender, CInt(GetPH(defender)/2));
		endif
	endif

	SetLastDamageInfo(defender, totaldmg, type, source, spell);

	if (GetObjProperty(defender, "#vinganca"))
		SetObjProperty(defender, "#vinganca", (cint(GetObjProperty(defender, "#vinganca")) + totaldmg));
	endif

	if (defender.frozen)
		defender.frozen := 0;
	endif

	return totaldmg;
endfunction

/**
 * DamageFLSCombat(attacker, defender, armor, damage, type)
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
function DamageFLSCombat(attacker, defender, armor, damage, type)
	SetScriptController(attacker);
	var totaldamage := CalculateDamageCombat(attacker, defender, armor, damage, type);

	CheckDefenderHabsBeforeDamage(attacker, defender, totaldamage);

	var regiao := CS_GetArmorZone(armor);
	if (regiao == error)
		regiao := "Corpo";
	endif
	DebugMsg(attacker, DEBUG_NAME_DAMAGE, $"Região atingida: {regiao}");
	DebugMsg(defender, DEBUG_NAME_DAMAGE, $"Região atingida: {regiao}");
	DebugMsg(defender, DEBUG_NAME_DAMAGE, $"Defesa {type}: {damage-totaldamage}");
	DebugMsg(attacker, DEBUG_NAME_DAMAGE, $"Dano Final: {totaldamage}({damage} - {damage-totaldamage})");
	DisplayDamage(defender, totaldamage, attacker);
	ApplyRawDamage(defender, totaldamage);
	CheckDefenderHabsAfterDamage(attacker, defender, totaldamage);
	SetLastDamageInfo(defender, totaldamage, type, attacker);

	if (defender.frozen)
		defender.frozen := 0;
	endif

	return totaldamage;
endfunction

/**
 * CalculateDamageCombat(attacker, defender, armor, damage, type)
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
function CalculateDamageCombat(attacker, defender, armor, damage, type)
	var resistMod := 0;
	var resist := 0;

	if (defender.npctemplate)
		resist := getResistNPCByType(defender, type, armor);
	else
		resist := getResistArmorByType(armor, type, attacker, defender);
	endif

	if (damage < 1)
		damage := 1;
	endif

	return CInt(damage-resist);
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
function getResistNPCByType(npc, type, armor := 0)
	var resist;
	var npctemplate := NPC_ParseTemplateName(npc.npctemplate);
	var cfg := ReadConfigFile(":*:npcdesc");

	if (!armor || armor == error)
		case(type)
			DMG_PHYSICAL: resist := CInt(cfg[npctemplate.template].PhysicalResist) + CInt(npc.resist_physical_mod); break;
			DMG_FIRE: resist := CInt(cfg[npctemplate.template].FireResist) + CInt(npc.resist_fire_mod); break;
			DMG_COLD: resist := CInt(cfg[npctemplate.template].ColdResist) + CInt(npc.resist_cold_mod); break;
			DMG_ENERGY: resist := CInt(cfg[npctemplate.template].EnergyResist) + CInt(npc.resist_energy_mod); break;
			DMG_POISON: resist := CInt(cfg[npctemplate.template].PoisonResist) + CInt(npc.resist_poison_mod); break;
		endcase
	else
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
	var resist;

	case(type)
		DMG_PHYSICAL: resist := CInt(armor.PhysicalResist) + CInt(armor.resist_physical_mod); break;
		DMG_FIRE: resist := CInt(armor.FireResist) + CInt(armor.resist_fire_mod); break;
		DMG_COLD: resist := CInt(armor.ColdResist) + CInt(armor.resist_cold_mod); break;
		DMG_ENERGY: resist := CInt(armor.EnergyResist) + CInt(armor.resist_energy_mod); break;
		DMG_POISON: resist := CInt(armor.PoisonResist) + CInt(armor.resist_poison_mod); break;
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
function CheckDefenderHabsBeforeDamage(attacker, byref defender, byref damage)
	if (GetObjProperty(defender, "#martir"))
		var martir := SystemFindObjectBySerial(GetObjProperty(defender,"martirserial"));
		if (martir && !martir.dead)
			defender := martir;
		endif
	endif
	
	if (GetObjProperty(defender, "#mantorothen"))
		var healed := cint(damage*0.3); // TODO: Ver se o valor curado será esse mesmo.
		damage := damage - healed;
		var party := defender.party;
		var amthealed := cint(healed/party.members.size());
		if (amthealed < 1)
			amthealed := 1;
		endif
		foreach member in (party.members)
			HealDamage(member, amthealed);
		endforeach
	endif
endfunction

/**
 * CheckDefenderHabsAfterDamage(byref defender, byref damage)
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
function CheckDefenderHabsAfterDamage(attacker, defender, damage)
	if (isBloodied(defender))
		RecalcVitals(defender);
		MakeBleed(defender);
		if (getObjProperty(defender, "#recuperacaosalvadora"))
			EraseObjProperty(defender, "#recuperacaosalvadora");
			SendSysMessageEx(defender, "Uma recuperacao salvadora salvou voce.", SSM_INFO);
			HealDamage(defender, CInt(GetPH(defender)/2));
		endif
	endif

	if (GetObjProperty(defender, "#vinganca"))
		SetObjProperty(defender, "#vinganca", (cint(GetObjProperty(defender, "#vinganca")) + damage));
	endif
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

