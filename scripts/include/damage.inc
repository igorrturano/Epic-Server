/* $Id$
 *
 * Purpose
 * Handles damages and resistances to damage types
 *
 */

use uo;
use os;
use math;
use cfgfile;
USE math;
use util;

include ":tn:cooldown";
include ":brainAI:npcUtil";
include ":attributes:attributeConstants";
include ":attributes:attributes";
include ":charactercreation:habilidades";
include "include/say";
include ":blood:blood";
include ":fls_core:packets";
include "epicCombatUtils";
include ":commands:debugsystem";


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
		SendSysMessageEx(targ, "Voce nao pode ser curado.", SSM_FAIL);

	endif

endfunction


function getNewDamageList(attacker)

	var list := dictionary;

	//GAMBI PRO BUG DO POL. REMOVER QUANDO CORRIGIREM
	if (attacker.npctemplate)
		var npctemplate := NPC_ParseTemplateName(attacker.npctemplate);
		var cfg := ReadConfigFile(":brainai:npcdesc");
		list[DMG_PHYSICAL] := Cdbl(cfg[npctemplate.template].PhysicalDamage/100.0);
		list[DMG_FIRE] := Cdbl(cfg[npctemplate.template].FireDamage/100.0);
		list[DMG_COLD] := Cdbl(cfg[npctemplate.template].ColdDamage/100.0);
		list[DMG_POISON] := Cdbl(cfg[npctemplate.template].PoisonDamage/100.0);
		list[DMG_ENERGY] := Cdbl(cfg[npctemplate.template].EnergyDamage/100.0);
		//printtextabove(attacker, " " + list);
		return list;

	else

		list[DMG_PHYSICAL] := Min(Cdbl(attacker.damage_physical/100.0),90.0);
		list[DMG_FIRE] := Min(Cdbl(attacker.damage_fire/100.0),90.0);
		list[DMG_COLD] := Min(Cdbl(attacker.damage_cold/100.0),90.0);
		list[DMG_POISON] := Min(Cdbl(attacker.damage_poison/100.0),90.0);
		list[DMG_ENERGY] := Min(Cdbl(attacker.damage_energy/100.0),90.0);

		var elemental := GetObjProperty(attacker.weapon, "#elementalweapon");
		if (elemental != error)
			list[elemental.type] := list[elemental.type] + elemental.bonus;
		endif

		var ammo := GetObjProperty(attacker.weapon, "ammo");

//		SendSysMessage( attacker, "Municao usada:" + ammo);
		if (ammo)
			var cfg:=ReadConfigFile(":woodworking:itemdesc");
			var elem := FindConfigElem(cfg, ammo);
//			SendSysMessage( attacker, "elem:" + elem);
			var dmgtype := GetConfigStringArray(elem, "dmgtype");
//			SendSysMessage( attacker, "dmgtype:" + dmgtype);
			var dmgbonus := CDbl(GetConfigString(elem, "dmgbonus"))/100.0;
//			SendSysMessage( attacker, "dmgbonus:" + dmgbonus);
			if(dmgtype)
				foreach dmg in dmgtype
//					SendSysMessage( attacker, "Entendeu que tem ao menos um tipo especial de dano");
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

function CalculateTotalDamage(defender, basedamage, attacker, dmglist)

	var dmg := 0;

	foreach key in (dmgList.keys())

		dmg := dmg + CalculateDamage(defender, key, basedamage*dmglist[key]);

	endforeach

	return cint(dmg);

endfunction

function CalculateDamage(defender, type, dmg)

	var resistSpells := 0;
	var resistMod := 0;
//	if (!defender.npctemplate)
//		resistSpells := cdbl(AP_GetSkill(defender, RESISTING_SPELLS)/200.0);
//	endif

	var resist;

	if (defender.npctemplate)
		var npctemplate := NPC_ParseTemplateName(defender.npctemplate);
		var cfg := ReadConfigFile(":brainai:npcdesc");

		case(type)
			DMG_FIRE: resist := cdbl( cfg[npctemplate.template].FireResist/100.0) + cdbl( defender.resist_fire_mod/100.0);
			DMG_COLD: resist := cdbl( cfg[npctemplate.template].ColdResist/100.0 ) + cdbl( defender.resist_cold_mod/100.0);
			DMG_ENERGY: resist := cdbl( cfg[npctemplate.template].EnergyResist/100.0 ) + cdbl( defender.resist_energy_mod/100.0);
			DMG_PHYSICAL: resist := cdbl( cfg[npctemplate.template].PhysicalResist /100.0 ) + cdbl( defender.resist_physical_mod/100.0);
			DMG_POISON: resist := cdbl( cfg[npctemplate.template].PoisonResist/100.0 ) + cdbl( defender.resist_poison_mod/100.0);
		endcase
	else
		//printtextabove( defender, "player " + type);
		case(type)
			DMG_FIRE: resist := cdbl( defender.resist_fire/100.0);
				  resistMod := cdbl( defender.resist_fire_mod/100.0);
			DMG_COLD: resist := cdbl( defender.resist_cold/100.0 );
				  resistMod := cdbl( defender.resist_cold_mod/100.0);
			DMG_ENERGY: resist := cdbl( defender.resist_energy/100.0 );
				    resistMod := cdbl( defender.resist_energy_mod/100.0);
			DMG_PHYSICAL: resist := cdbl( defender.resist_physical /100.0 );
				      resistMod := cdbl( defender.resist_physical_mod/100.0);
			DMG_POISON: resist := cdbl( defender.resist_poison/100.0 );
				    resistMod := cdbl( defender.resist_poison_mod/100.0);
		endcase
	endif

/*
	if (IsRaging(defender) && TemHabilidade(defender, "Pele de Rocha"))
		if (resist < 0.5)
			resist := 0.5;
		endif
	endif
*/
	if (type == DMG_PHYSICAL)
        if (TemHabilidade(defender, "Kani no Sutairu"))
            resist := CDbl(AP_GetSkill(defender, TACTICS) / 200); // Ganha até 50% de bonus
        endif
		if (TemHabilidade(defender, "Exoesqueleto"))
			resist := resist * 1.2;
		endif
		if (TemHabilidade(defender, "Especialista em Escudos") && (defender.shield).IsA(POLCLASS_ARMOR) ) // Se tiver a hab e usar escudo
			resist := resist + (AP_GetTrueSkill(defender,PARRY)/1000); // Ganha até +10% de bonus
		endif
		if (GetCooldown(defender, "furia") && TemHabilidade(defender, "Resiliência Bárbara")) // Antigo Pele de Rocha
			if (resist < 0.6)
				resist := 0.6;
			endif
        elseif (TemHabilidade(defender, "Resiliência Bárbara"))
            if (resist < 0.3)
                resist := 0.3;
            endif
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


//	printtextabove(defender, "dano " + type + " resist " + resist + " total dmg  " + dmg + " resistido " + (dmg*resist) + " final " + (dmg - (dmg*resist)) );

	if (!defender.npctemplate)
		if (resist >= 0.7)
			resist := 0.7;
		endif
	endif

//	if ( (resistSpells+resistMod) > resist)
//		return (dmg - (dmg*(resistSpells+resistMod)));
//	else
		return (dmg - (dmg*resist));
//	endif

endfunction

function ApplyDamageEX(defender, dmg, type, source := 0)

	var totaldmg := cint(CalculateDamage(defender, type, dmg ));

//	printtextabove(defender, "raw dmg " + dmg + " type " + type + " final " + totaldmg);

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

function CalculateTotalDamageBonus(damage, attacker, defender)
	damage += CalcDamageStats(attacker);
	damage += CalcDamageWeapon(attacker);
	damage += CalcDamageSpecial(attacker, defender);
	damage += CalcDamageHabs(attacker, defender);

	damage := CalcDamageReductor(attacker, damage);
	damage := CalcDamageFinal(attacker, damage);

	if (damage < 1)
		damage := 1;
	endif

	return damage;
endfunction

function CalcDamageFinal(attacker, damage)
	var polida := GetObjProperty(attacker.weapon, "polida");
	if (!polida)
		damage := Cint(damage * 0.8);
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
        damage := Cint(damage * 0.7);
    endif

	return damage;
endfunction

function CalcDamageStats(attacker)
	var stat;
	var damage := 0;
	var damagemod := 0;

	var cfg := ReadConfigFile(":combat:itemdesc");
	var elem := FindConfigElem(cfg, attacker.weapon.objtype);

	SkillCheck(attacker, TACTICS, -1);
	damagemod += Cint(AP_GetSkill(attacker, TACTICS)/20);

	stat := STRENGTH;
	if (elem.Attribute == "Ranged")
		stat := DEXTERITY;
	endif

	if (elem.Throwable)
		damage := Cint(AP_GetStat(attacker, DEXTERITY)) + Cint(AP_GetStat(attacker, STRENGTH) / 2);
	else
		damage := Cint(AP_GetStat(attacker, stat));
	endif

	damage := (damage-50) / 10;
	DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano de "+stat+": " + damage);

	return damage;
endfunction

function CalcDamageWeapon(attacker)
	var weaponcfg := ReadConfigFile(":*:itemdesc");
	var weaponelem := FindConfigElem(weaponcfg, attacker.weapon.objtype);
	var damage := 0;
	
	var weapondice := CStr(weaponelem.Damage);
	var damageweapon;
	if (CheckBastardSword(attacker)) // Usando espada bastarda com as duas mãos
		weapondice := "1d10";
	endif
	damageweapon := RandomDiceRoll(weapondice);
	damage += damageweapon;

	damage += Cint(GetObjProperty(attacker.weapon, "dmgbonus")) + Cint(GetObjProperty(attacker.weapon, "#dmgbonus"));

	if (GetObjProperty(attacker, "armapredileta") == attacker.weapon.objtype)
		damage += Cint(AP_GetSkill(attacker, TACTICS) / 25); 
	endif

	var sharpened_weapon := GetObjProperty(attacker.weapon, "Aprimoramentos")["Arma Afiada"];
	if(sharpened_weapon)
		damage += sharpened_weapon;
	endif

	DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano da arma: " + weapondice);
	DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano da arma (sem mods): " + damageweapon);
	DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano da arma (com mods): " + damage);
	DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Arma Afiada: " + sharpened_weapon);

	return damage;
endfunction

function CalcDamageHabs(attacker, defender)
	var damage := 0;
	
	if (TemHabilidade(attacker, "Brigador de Taverna") && attacker.weapon.objtype == 0x1F020)
        var damagemod := 2;
		damage += damagemod;
		DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano de Habilidade[Brigador de Taverna]: " + damagemod);
    endif

	if (getCooldown(attacker, "faroporsangue"))
		var damagemod;
		var bonus := AP_GetVital(defender, HITS) / AP_GetVitalMaximumValue(defender, HITS);
        if (bonus <= 0.25)
			bonus := "1d12";
		elseif (bonus <= 0.5)
			bonus := "1d10";
        elseif (bonus <= 0.75)
            bonus := "1d8";
		endif
		damagemod := RandomDiceRoll(bonus);
		damage += damagemod;
		DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano de Habilidade[Faro por Sangue]: " + damagemod);
	endif

	if (TemHabilidade(attacker, "Taticas Arcanas"))
		var manaatual := AP_GetVital(attacker, "MANA");
		var manatotal := AP_GetVitalMaximumValue(attacker, "MANA");
		if (manaatual >= manatotal / 2)
			var damagemod := Cint(manaatual / 20);
			damage += damagemod;
			DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano de Habilidade[Faro por Sangue]: " + damagemod);
		endif
	endif

	if (GetCooldown(attacker, "lobosolitario"))
		if (ListHostiles(attacker, 3, LH_FLAG_LOS) > 2)
			var damagemod := Cint(AP_GetSkill(attacker, SURVIVAL) / 20);
			damage += damagemod;
			DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano de Habilidade[Lobo Solitário]: " + damagemod);
		endif
	endif

	if (TemHabilidade(attacker, "Precisao Cirurgica"))
		var damagemod := Cint(AP_GetSkill(attacker, ALCHEMY) / 20);
		damage += damagemod;
		DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano de Habilidade[Precisão Cirurgica]: " + damagemod);
	endif

    if (TemHabilidade(attacker, "Arsenal Sombrio"))
        var damagemod := Cint(AP_GetSkill(attacker, SNEAK) / 20);
        if (isAssassinWeapon(attacker))
			damage += damagemod;
			DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano de Habilidade[Arsenal Sombrio]: " + damagemod);
        endif
    endif

	if (TemHabilidade(attacker, "Estilo Mao Livre"))
		if (attacker.weapon.attribute == "One_Handed" && !attacker.shield)
			var damagemod := Cint(AP_GetSkill(attacker, TACTICS) / 25);
			damage += damagemod;
			DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano de Habilidade[Estilo Mao Livre]: " + damagemod);
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
						damagemod += Cint(AP_GetSkill(attacker, MAGICLORE) / 20);
						break;
				endcase

				arrow_effects[arrow_type] := 0;
				SetObjProperty(attacker, "#shooting_effects", arrow_effects);
			endif
		endforeach

		damage += damagemod;
		DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano de Habilidade[Ainalaith]: " + damagemod);
	endif

	if (TemHabilidade(attacker, "Inimigo Predileto"))
		var npctemplate := NPC_ParseTemplateName(defender.npctemplate);
		var cfg := ReadConfigFile(":brainai:npcdesc");
		var inimigo := GetObjProperty(attacker, "InimigoPredileto");

		if (cfg[npctemplate.template].Category == inimigo)
			var damagemod := Cint(AP_GetSkill(attacker, SURVIVAL) / 20);
			damage += damagemod;
			DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano de Habilidade[Inimigo Predileto]: " + damagemod);
		endif
    elseif (TemHabilidade(attacker, "Exitium Maleficorum"))
        var npctemplate := NPC_ParseTemplateName(defender.npctemplate);
		var cfg := ReadConfigFile(":brainai:npcdesc");
		var inimigo := array{"Undead", "Demons"};

		if (cfg[npctemplate.template].Category in inimigo)
			var damagemod := Cint(AP_GetSkill(attacker, MAGICLORE) / 20);
			damage := damagemod;
			DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano de Habilidade[Exitium Maleficorum]: " + damagemod);
		endif
	elseif (TemHabilidade(attacker, "Oleo do Cacador"))
		var npctemplate := NPC_ParseTemplateName(defender.npctemplate);
		var cfg := ReadConfigFile(":brainai:npcdesc");
		var inimigo := GetObjProperty(attacker, "Óleo");
		if (cfg[npctemplate.template].Category == inimigo)
			var damagemod := Cint(AP_GetSkill(attacker, ALCHEMY) / 20);
			damage := damagemod;
			DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano de Habilidade[Oleo do Cacador]: " + damagemod);
		endif
	endif

	if (GetCooldown(defender, "prey"))
		var damagemod := Cint(AP_GetSkill(attacker, SURVIVAL) / 20);
		damage += damagemod;
		DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano de Habilidade[Presa]: " + damagemod);
	endif

	if (temHabilidade(attacker, "Instinto de Sobreviência") && AP_GetVital(attacker, HITS) <= (AP_GetSkill(attacker, MAGICLORE) / 2))
		if (GetObjProperty(attacker, "metamorfose") != error)
			var damagemod := Cint(AP_GetSkill(attacker, MAGICLORE) / 20);
			damage += damagemod;
			DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano de Habilidade[Arsenal Sombrio]: " + damagemod);
			PrintText(attacker, "*enfurecido*");
		endif
	endif

	if (IsRaging(attacker))
		var damagemod;
		if (temHabilidade(attacker, "Nazg Khalug") || GetObjProperty(attacker, "NpcFuria"))
			damagemod := Cint(AP_GetSkill(attacker, TACTICS) / 20);
			damage += damagemod;
			DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano de Habilidade[Nazg Khalug]: " + damagemod);
			PrintText(attacker, "*enfurecido*");
		endif
		if (temHabilidade(attacker, "Furia Primitiva"))
			if (GetObjProperty(attacker, "metamorfose") != error)
				damagemod := Cint(AP_GetSkill(attacker, MAGICLORE) / 20);
				damage += damagemod;
				DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano de Habilidade[Furia Primitiva]: " + damagemod);
			endif
		endif
		if (GetCooldown(attacker, "FuriaPrimordial"))
			var inimigos := ListHostiles(attacker, 3, LH_FLAG_LOS);
			inimigos := inimigos.size();
			var damagemod := Cint(inimigos.size());
			damage += damagemod;
			DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano de Habilidade[Furia Primordial]: " + damagemod);
		endif
		if (GetCooldown(attacker, "furiamortal"))
			var damagemod := Cint((AP_GetVitalMaximumValue(attacker, HITS) - AP_GetVital(attacker, HITS)) / 10);
			damage += damagemod;
			DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano de Habilidade[Furia Mortal]: " + damagemod);
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
                SendSysMessageEx(attacker, "Voce perde mobilidade por estar de armadura e consequentemente, dano.", SSM_INFO);
                damagemod := 0;
                break;
            endif
        endforeach
    	
		if (damagemod)
			if (attacker.weapon.objtype in monk_weapons)
				damage += damagemod;
				DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano de Habilidade[Budoka]: " + damagemod);
			endif
		endif
	endif

	if (isBloodied(defender))
		if (temHabilidade(attacker, "Burz"))
			var damagemod := Cint(AP_GetSkill(attacker, TACTICS) / 20);
			damage += damagemod;
			DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano de Habilidade[Burz]: " + damagemod);
		endif
	endif

	if (GetCooldown(attacker, "furia"))
		var damagemod := Cint(AP_GetSkill(attacker, TACTICS) / 20);
		if (TemHabilidade(attacker, "Destruidor"))
			damagemod := damagemod * 2;
		endif
		DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano de Habilidade[Furia]: " + damagemod);
	endif

	return damage;
endfunction

function CalcDamageSpecial(attacker, defender)
	var damage;
	
	if (GetCooldown(attacker.weapon, "blessweapon"))
		var categoriaDefender := NPC_GetNPCConfig(defender.npctemplate).Category;
		var cursed_creatures := {"undead", "demon"};
		if (defender.npctemplate)
			if (lower(categoriaDefender) in cursed_creatures)
				var damagemod := Cint(GetObjProperty(attacker.weapon, "blessedbonus"));
				/* if (!damagemod)
					damagemod := 0;
				endif */
				DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano Especial[Bless Weapon]: " + damagemod);
			endif
		endif
	endif

	var backStab := CDbl(GetObjProperty(attacker, "#backstab"));
	if (backStab)
		var damagemod := 1;
		damage += damagemod;
		DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano Especial[Backstab]: " + damagemod);
		EraseObjProperty(attacker, "#backstab");
	endif
endfunction

function CalcDamageReductor(attacker, damage)
	var damagemod := Cint(GetObjProperty(attacker, "dmgbonus")) + Cint(GetObjProperty(attacker, "#dmgbonus"));

	damagemod += CDbl(GetObjProperty(attacker, "dmgreduction"));
	damagemod := 1 - damagemod;
	damage := damage*damagemod;
	DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Redutor de Dano: " + CDbl(1-damagemod));

	return damage;
endfunction

function DamageFLS(defender, dmg, type := DMG_FORCED, source := 0, spell := "")
	var totaldmg;
	if (type == DMG_FORCED)
		totaldmg := CInt(dmg);
	else
		totaldmg := CInt(CalculateDamage(defender, type, dmg ));
		//printtextabove(defender, "raw dmg " + dmg + " type " + type + " final " + totaldmg);
	endif

	if (GetObjProperty(defender, "#martir"))
		var martir := SystemFindObjectBySerial(GetObjProperty(defender,"martirserial"));
		//printtextabove(defender, " " + martir + "serial" + GetObjProperty(defender, "martirserial"));
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
			healdamage(member, amthealed);
		endforeach
	endif

  	DisplayDamage(defender, totaldmg, source);

	ApplyRawDamage(defender, totaldmg);


	if (isBloodied(defender))
		RecalcVitals(defender);
		MakeBleed(defender);
    //PrintText(defender, "*sangrando*");
		if (getObjProperty(defender, "#recuperacaosalvadora"))
			EraseObjProperty(defender, "#recuperacaosalvadora");
			SendSysMessageEx(defender, "Uma recuperacao salvadora salvou voce.", SSM_INFO);
			HealDamage(defender, cint(GetPH(defender)/2));
		endif
	endif

	SetLastDamageInfo(defender, totaldmg, type, source, spell);

	if (source)
		SetScriptController(source);
	endif
	if (GetObjProperty(defender, "#vinganca"))
		SetObjProperty( defender, "#vinganca", (cint(GetObjProperty(defender, "#vinganca")) + totaldmg));
	endif

	if (defender.frozen)
		defender.frozen := 0;
	endif

	return totaldmg;

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
//TODO: adicionar influencia do .dmg_mod das armas

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

function CheckBastardSword(who)
	if (who.weapon.objtype == 0x13b9 && !who.shield)
		return 1;
	endif
	return 0;
endfunction