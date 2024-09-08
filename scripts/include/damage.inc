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
        if (TemHabilidade(defender, "Corpo Diamante"))
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


function CalculateTotalDamageBonus(dmg, who, defender)
	var tatics;
	var skillCriticalBonus;
	var categoriaDefender := NPC_GetNPCConfig(defender.npctemplate).Category;

	SkillCheck(who, TACTICS, -1);
	tatics := Cint(AP_GetSkill(who, TACTICS)/10);

	//Bonus STR
	var cfg:=ReadConfigFile(":combat:itemdesc");
	var elem := FindConfigElem(cfg, who.weapon.objtype);

	var	stat := STRENGTH;
	if (elem.Attribute == "Ranged")
		stat := DEXTERITY;
	endif

	var str;
	if (elem.Throwable)
		str := Cint(AP_GetStat(who, DEXTERITY)) + Cint(AP_GetStat(who, STRENGTH) / 2);
	else
		str := Cint(AP_GetStat(who, stat));
	endif
	//Bonus de prop da arma
	var weapon_bonus_dmg := Cint(GetObjProperty(who.weapon, "dmgbonus")) + Cint(GetObjProperty(who.weapon, "#dmgbonus"));

	//Bonus de prop do mobile
	var other := Cint(GetObjProperty(who, "dmgbonus")) + Cint(GetObjProperty(who, "#dmgbonus"));

	//Bonus de Spells
	var reducao_dano := CDbl(GetObjProperty(who, "dmgreduction"));

	var blessbonus := 0.0;
	if (GetCooldown(who.weapon, "blessweapon"))
		var cursed_creatures := {"undead", "demon"};
		if (defender.npctemplate)
			if (lower(categoriaDefender) in cursed_creatures )
				blessbonus := Cint(GetObjProperty(who.weapon, "blessedbonus"));
				if (!blessbonus)
					blessbonus := 0;
				endif
			endif
		endif
	endif

	if (getCooldown(who, "faroporsangue"))
		var bonus := AP_GetVital(defender, HITS) / AP_GetVitalMaximumValue(defender, HITS);
        if (bonus <= 0.25)
			bonus := "1d12";
		elseif (bonus <= 0.5)
			bonus := "1d10";
        elseif (bonus <= 0.75)
            bonus := "1d8";
		endif
		var bonus_dice := RandomDiceRoll(bonus);
		other := other + bonus_dice;
	endif

	//Bonus de Habilidades especiais

    if (TemHabilidade(who, "Taticas Arcanas"))
		var manamoment 	:= AP_GetVital(who, "MANA");
		var manatotal	:= AP_GetVitalMaximumValue(who, "MANA");
		if 	(manamoment >= manatotal / 2)
			other := other + Cint(manamoment / 20);
		endif
	endif

	if (GetCooldown(who, "lobosolitario"))
		var inimigos := ListHostiles(who, 3, LH_FLAG_LOS);
		if (inimigos > 2)
			other := other + Cint(AP_GetSkill(who, SURVIVAL) / 10); 
		endif
	endif

	if (TemHabilidade(who, "Precisao Cirurgica"))
		other := other + Cint(AP_GetSkill(who, ALCHEMY) / 10);
	endif

    if (TemHabilidade(who, "Arsenal Sombrio"))
        var sneak_dmg := Cint(AP_GetSkill(who, SNEAK) / 20);
        if (isAssassinWeapon(who))
            other += sneak_dmg;
        endif
    endif

	var witcher := Cint(AP_GetSkill(who, ALCHEMY) / 10);
	var inimigo_predileto := 0;
	var inimigo_dmg := Cint(AP_GetSkill(who, SURVIVAL) / 10);
	if (TemHabilidade(who, "Inimigo Predileto"))
		var npctemplate := NPC_ParseTemplateName(defender.npctemplate);
		var cfg := ReadConfigFile(":brainai:npcdesc");
		var inimigo := GetObjProperty(who, "InimigoPredileto");
		if (cfg[npctemplate.template].Category == inimigo)
			inimigo_predileto := inimigo_predileto + inimigo_dmg; //dobra o dano se for inimigo predileto
		endif
    elseif (TemHabilidade(who, "Exitium Maleficorum"))
        var npctemplate := NPC_ParseTemplateName(defender.npctemplate);
		var cfg := ReadConfigFile(":brainai:npcdesc");
		if (cfg[npctemplate.template].Category == "Undead" || cfg[npctemplate.template].Category == "Demons")
			inimigo_predileto := inimigo_predileto + inimigo_dmg; //dobra o dano se for inimigo predileto
		endif
	elseif (TemHabilidade(who, "Oleo do Cacador"))
		var npctemplate := NPC_ParseTemplateName(defender.npctemplate);
		var cfg := ReadConfigFile(":brainai:npcdesc");
		var inimigo := GetObjProperty(who, "Óleo");
		if (cfg[npctemplate.template].Category == inimigo)
			inimigo_predileto := witcher + 1.0; //dobra o dano se for inimigo predileto
		endif
	endif
	

	if (GetCooldown(defender, "prey"))
		inimigo_predileto := inimigo_predileto + inimigo_dmg;
	endif

	if (TemHabilidade(who, "Estilo Mao Livre"))
		if (who.weapon.attribute == "One_Handed" && !who.shield)
			other := other + Cint(AP_GetSkill(who, TACTICS) / 25);
		endif
	endif

	if (isBloodied(defender))
		if (temHabilidade(who, "Burz"))
			other := other + Cint(AP_GetSkill(who, TACTICS) / 10);
		endif
	endif

	var rage_bonus := Cint(AP_GetSkill(who, TACTICS) / 10);
	if (GetCooldown(who, "furia"))
		other := other + rage_bonus;
		if (TemHabilidade(who, "Destruidor"))
			other := other + rage_bonus * 2;
		endif
	endif

	if (temHabilidade(who, "Instinto de Sobreviência") && AP_GetVital(who, HITS) <= (AP_GetSkill(who, MAGICLORE) / 2))
		if (GetObjProperty(who, "metamorfose") != error)
			other := other + Cint(AP_GetSkill(who, MAGICLORE) / 10);
			PrintText(who, "*enfurecido*");
		endif
	endif

	if ( IsRaging(who))
		if (temHabilidade(who, "Nazg Khalug") || GetObjProperty(who, "NpcFuria"))
			other:= other + Cint(AP_GetSkill(who, TACTICS) / 10);
			PrintText(who, "*enfurecido*");
		endif
		if (temHabilidade(who, "Furia Primitiva"))
			if (GetObjProperty(who, "metamorfose") != error)
				other:= other + Cint(AP_GetSkill(who, MAGICLORE) / 10);
			endif
		endif
		if (GetCooldown(who, "FuriaPrimordial"))
			var inimigos := ListHostiles(who, 3, LH_FLAG_LOS);
			inimigos := inimigos.size();
			other := other + Cint(inimigos.size()); //15% de bonus - balanceamento depois da mudanca de calculo do dano
		endif
		if (GetCooldown(who, "furiamortal"))
			var bonus := Cint((AP_GetVitalMaximumValue(who, HITS) - AP_GetVital(who, HITS)) / 10);
			other := other + bonus;
		endif
	endif
	var backStab := CDbl(GetObjProperty(who, "#backstab"));
	if (backStab)
		other := other + 2;
		EraseObjProperty(who, "#backstab");
	endif



	if ( GetObjProperty(who, "armapredileta") == who.weapon.objtype )
		other := other + Cint(AP_GetSkill(who, TACTICS) / 25); 
	endif

	//#shooting_effects
	if (TemHabilidade(who, "Ainalaith"))
		var arrow_effects := GetObjProperty(who, "#shooting_effects");
		if (!arrow_effects)
			return 0; // No effects recorded, exit early
		endif

		var arrow_types := array{"fire_arrow", "cold_arrow", "energy_arrow"};
		foreach arrow_type in arrow_types
			if (arrow_effects[arrow_type] >= 3)
				case (arrow_type)
					"fire_arrow":
						TS_StartTimer(defender, "queimar", 10);
					"cold_arrow":
						TS_StartTimer(defender, "lentidao", 10);
					"energy_arrow":
						other := other + Cint(AP_GetSkill(who, MAGICLORE) / 20); 
				endcase

				// Reset the counter for this arrow type
				arrow_effects[arrow_type] := 0;
				SetObjProperty(who, "#shooting_effects", arrow_effects);
			endif
		endforeach
	endif

    var itemdesc := ReadConfigFile("::itemdesc");

    if (TemHabilidade(who, "Brigador de Taverna") && (who.weapon.objtype == 0x1F020) && (dmg != 0))
        dmg += Cint(AP_GetSkill(who, TACTICS) / 20);
    endif

    if (TemHabilidade(who, "Artista Marcial") )
		var monk_weapons := array{ 0x0df0, 0x13f9, 0x0e8a, 0x0e81, 0x1F020, 0x27AD, 0x27AE, 0x27A7};
        var isArmor := array{"ArmaduraPesada", "ArmaduraMedia", "ArmaduraLeve"};
    	if ( who.weapon.objtype in monk_weapons)
			other := other + Cint(AP_GetSkill(who, TACTICS) / 10);
		endif
        var i_config := ReadConfigFile(":*:itemdesc");
		var this_item := "";
        foreach item in ListEquippedItems(who)
            this_item := i_config[item.objtype];
            if (this_item.TipoProtecao in isArmor)
                SendSysMessageEx(who, "Voce perde mobilidade por estar de armadura e consequentemente, dano.", SSM_INFO);
                other := 0;
                break;
            endif
        endforeach
	endif

	var weaponcfg := ReadConfigFile(":*:itemdesc");
	var weaponelem := FindConfigElem(weaponcfg, who.weapon.objtype);
	var weapon_dice := RandomDiceRoll(cstr(weaponelem.Damage));
	if (who.npctemplate)
		weapon_dice := 0; // Tava dando dano duas vzs
 	endif

	if (dmg == 0) // Check de Parry que tava atravessando por conta do novo calculo de dano
		dmg := 1;
		return dmg;
	endif

	if (IsTwoHandedWeapon(who.weapon))
		other := other + Cint(dmg * 1.5);
	endif

	if (who.weapon.objtype == 0x13b9 && !who.shield) // Usando espada bastarda com as duas mãos
		other := other + Cint(dmg * 1.5);
        weapon_dice := RandomDiceRoll("1d10");
    endif
	var sharpened_weapon := GetObjProperty(who.weapon, "Aprimoramentos")["Arma Afiada"];
	if(sharpened_weapon)
		other += sharpened_weapon;
	endif

	var DEBUGTYPE := "damage";
	DebugMsg(who, DEBUGTYPE, "Dano base: " + Cint(dmg));
	DebugMsg(who, DEBUGTYPE, "Rolagem de dano da arma: " + weapon_dice);
	DebugMsg(who, DEBUGTYPE, "Dano de habilidades: " + (other - sharpened_weapon));
	DebugMsg(who, DEBUGTYPE, "Dano de tatics: " + tatics);
	DebugMsg(who, DEBUGTYPE, "Dano de força: " + ((str-50)/10));
	DebugMsg(who, DEBUGTYPE, "Dano de bêncãos: " + blessbonus);
	DebugMsg(who, DEBUGTYPE, "Dano de inimigo predileto: " + inimigo_predileto);
	DebugMsg(who, DEBUGTYPE, "Redução de dano: " + (-1 * reducao_dano));
	DebugMsg(who, DEBUGTYPE, "Arma Afiada: " + sharpened_weapon);

	dmg := CInt( (dmg + (((str-50)/10)+tatics+weapon_dice+blessbonus+weapon_bonus_dmg+other)+inimigo_predileto) * (1-reducao_dano) );
	if (dmg < 1)
		dmg := 1;
	endif

	if (GetObjProperty(who, "chardata").trait == "Defensor Anão")
        dmg := Cint(dmg * 0.7);
    endif

	var polida := GetObjProperty(who.weapon, "polida");
	if (!polida)
		dmg := Cint(dmg * 0.8);
	else
		polida := polida - 1;
		if (polida < 1)
			EraseObjProperty(who.weapon, "polida");
			SendSysMessageEX(who, "Sua arma perdeu a polidez.", SSM_INFO);
		else
			SetObjProperty(who.weapon, "polida", polida);
		endif
	endif
	
	return dmg;

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

function IsTwoHandedWeapon (byref item)
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

function IsThrowingWeapon (byref item)
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