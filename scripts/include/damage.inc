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

include ":tn:cooldown";
include ":brainAI:npcUtil";
include ":attributes:attributeConstants";
include ":attributes:attributes";
include ":charactercreation:habilidades";
include "include/say";
include ":blood:blood";
include ":fls_core:packets";

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
		if (IsRaging(defender) && TemHabilidade(defender, "Pele de Rocha"))
			if (resist < 0.6)
				resist := 0.6;
			endif
        elseif (TemHabilidade(defender, "Pele de Rocha"))
            if (resist < 0.3)
                resist := 0.3;
            endif
		endif
	endif


//	printtextabove(defender, "dano " + type + " resist " + resist + " total dmg  " + dmg + " resistido " + (dmg*resist) + " final " + (dmg - (dmg*resist)) );

	if (!defender.npctemplate)
		if (resist >= 0.8)
			resist := 0.8;
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
	//Bonus de dano por skill
	var tatics;
	var skillCriticalBonus;
	var categoriaDefender := NPC_GetNPCConfig(defender.npctemplate).Category;
	//print("Categoria: "+categoriaDefender);


	if (who.npctemplate) //Se for NPC utiliza tactics
		tatics := CDbl(AP_GetSkill(who, TACTICS))/100.0; //Armazena o valor da skill que afeta dano
		SkillCheck(who, TACTICS, -1);
	elseif (GetObjProperty(who, "kit") == "Mago de Combate")
		tatics := CDbl(AP_GetSkill(who, MAGICLORE))/100.0; //Aplica bonus de magiclore na chance de critico quando é Mago de Combate
		SkillCheck(who, MAGICLORE, -1);
	elseif (GetObjProperty(who, "kit") == "Medico de Combate")
		tatics := CDbl(AP_GetSkill(who, MEDICINE))/100.0; //Aplica bonus de medicine na chance de critico quando é Medico de Combate
		SkillCheck(who, MAGICLORE, -1);
	 elseif (GetObjProperty(who, "kit") == "Ranger")
		tatics := CDbl(AP_GetSkill(who, TACTICS))/100.0; //Aplica bonus de tactics na chance de critico quando não é nenhum caso específico
		SkillCheck(who, TACTICS, -1);
		if(categoriaDefender == "GiantAnimal" || categoriaDefender == "Animal" || categoriaDefender == "EvilAnimal" || categoriaDefender == "Snow" || categoriaDefender == "Mount" || categoriaDefender == "Plantas" || categoriaDefender == "Terathan" || categoriaDefender == "Dragon")
			tatics := tatics + CDbl(AP_GetSkill(who, SURVIVAL))/150.0; //Aplica bonus de Survival se forem criaturas selvagens ou dragões
			SkillCheck(who, SURVIVAL, -1);
		endif
	else
		tatics := CDbl(AP_GetSkill(who, TACTICS))/100.0; //Armazena o valor da skill que afeta dano
	endif
	if(temHabilidade(who, "Lobo Solitario"))
		tatics := CDbl(AP_GetSkill(who, SURVIVAL))/100.0; //Aplica bonus de tactics na chance de critico quando é classe Ranger
		SkillCheck(who, SURVIVAL, -1);
	endif

	//Bonus STR
	var stat := STRENGTH;
	if (temHabilidade(who, "Ataques Precisos"))
		stat := DEXTERITY;
	endif
	if (temHabilidade(who, "Ataques Oportunistas"))
		stat := INTELLIGENCE;
	endif
	var strBonus := 0.0;
	/*
	if (AP_GetStat(who, stat) >= 100 ) //Calcula um bonus se tiver mais de 100 do atributo que afeta o dano
		strBonus := 10.0;
	endif

	//Calcula modificador total do atributo
	var str := CDbl(AP_GetStat(who, stat)+strBonus)/150.0;
	*/
	var str;
	if (who.npctemplate)
		str := CDbl(AP_GetStat(who, stat))/200.0;
	else
		str := CDbl(AP_GetStat(who, stat)-60.0)/200.0;
	endif
	//Quando equipado arco curto, longo, besta, besta pesada e besta de repeticao a força não implica em modificador de dano
	if ( (who.weapon.objtype == 0x13B2) || (who.weapon.objtype == 0x26CC) || (who.weapon.objtype == 0x0f4f) || (who.weapon.objtype == 0x13fd) || (who.weapon.objtype == 0x26CD) )
		str := 0;
	endif


	//Bonus de prop da arma
	var weapon := CDbl(GetObjProperty(who.weapon, "dmgbonus")) + CDbl(GetObjProperty(who.weapon, "#dmgbonus"));

	//Bonus de prop do mobile
	var other := CDbl(GetObjProperty(who, "dmgbonus")) + CDbl(GetObjProperty(who, "#dmgbonus"));

	//Bonus de Spells
	var reducao_dano := CDbl(GetObjProperty(who, "dmgreduction"));

	var blessbonus := 0.0;
	if (GetCooldown(who.weapon, "blessweapon"))
		if (defender.npctemplate)
			//var elem := NPC_GetNPCConfig(defender.npctemplate);
			//var category := lower(categoriaDefender);
			if (lower(categoriaDefender) == "undead" )
				blessbonus := CDbl(GetObjProperty(who.weapon, "blessedbonus"));
				if (!blessbonus)
					blessbonus := 0.4; //20% - balanceamento depois da mudanca de calculo do dano
				endif
			endif
		endif
	endif

	if (getCooldown(who, "faroporsangue"))
		var bonus := AP_GetVital(defender, HITS) / AP_GetVitalMaximumValue(defender, HITS);
        if (bonus <= 0.25)
			bonus := 0.75;
		elseif (bonus <= 0.5)
			bonus := 0.5;
        elseif (bonus <= 0.75)
            bonus := 0.25;
		endif

		other := other + bonus;
	endif

	//Bonus de Habilidades especiais

    if (TemHabilidade(who, "Taticas Arcanas"))
		var manamoment 	:= AP_GetVital(who, "MANA");
		var manatotal	:= AP_GetVitalMaximumValue(who, "MANA");
		if 	(manamoment >= manatotal / 2)
			other := other + (manamoment / 200);
		endif
	endif

	var habesgrimista := cint(GetObjProperty(who, "#contraataquepoderoso"))-1.0;
	if (habesgrimista > 0)
		other := other +habesgrimista;
	endif

	if (GetCooldown(who, "lobosolitario"))
		var inimigos := ListHostiles(who, 3, LH_FLAG_LOS);
		if (inimigos > 2)
			other := other + 0.5; //50% - balanceamento depois da mudanca de calculo do dano
		endif
	endif

//	if (GetCooldown(who, "marcadacaca"))
//			other := other + 0.75; //75% - balanceamento depois da mudanca de calculo do dano
//	endif

	if (TemHabilidade(who, "Precisao Cirurgica"))
		other := other + 0.50; //50% - balanceamento depois da mudanca de calculo do dano
	endif

    if (TemHabilidade(who, "Arsenal Sombrio"))
        var assassin_weapons := {0x27EF, 0x27ab, 0x27af, 11570, 11565, 11566, 0x1401, 0x1440, 0x0f51, 0x0f63, 0x143c, 0x13B2};
        var sneak_dmg := AP_GetSkill(who, SNEAK) / 200;
        if (who.weapon.objtype in assassin_weapons)
            other += sneak_dmg;
        endif
    endif

	var inimigo_predileto := 1.0;
	if (TemHabilidade(who, "Inimigo Predileto II"))
		var npctemplate := NPC_ParseTemplateName(defender.npctemplate);
		var cfg := ReadConfigFile(":brainai:npcdesc");
		var inimigo := GetObjProperty(who, "InimigoPrediletoII");
		if (cfg[npctemplate.template].Category == inimigo)
			inimigo_predileto := inimigo_predileto + 2.0; //triplica o dano se for inimigo predileto 2
		endif
	elseif (TemHabilidade(who, "Inimigo Predileto"))
		var npctemplate := NPC_ParseTemplateName(defender.npctemplate);
		var cfg := ReadConfigFile(":brainai:npcdesc");
		var inimigo := GetObjProperty(who, "InimigoPredileto");
		if (cfg[npctemplate.template].Category == inimigo)
			inimigo_predileto := inimigo_predileto + 1.0; //dobra o dano se for inimigo predileto
		endif
    elseif (TemHabilidade(who, "Equilibrio Khummardigan"))
        var npctemplate := NPC_ParseTemplateName(defender.npctemplate);
		var cfg := ReadConfigFile(":brainai:npcdesc");
		if (cfg[npctemplate.template].Category == "Undead" || cfg[npctemplate.template].Category == "Demons")
			inimigo_predileto := inimigo_predileto + 1.0; //dobra o dano se for inimigo predileto
		endif
    elseif (TemHabilidade(who, "Regencia de Asan"))
        var npctemplate := NPC_ParseTemplateName(defender.npctemplate);
		var cfg := ReadConfigFile(":brainai:npcdesc");
		if (cfg[npctemplate.template].Category == "EvilHuman" || cfg[npctemplate.template].Category == "GiantAnimal")
			inimigo_predileto := inimigo_predileto + 1.0; //dobra o dano se for inimigo predileto
		endif
	endif

	if (TemHabilidade(who, "Estilo Mao Livre"))
		if (who.weapon.attribute == "MeleeCombat1H" && !who.shield)
			other := other + 0.2;
		endif
	endif

    if (who.weapon.objtype == 0x13b9 && !who.shield)
        other := other + 0.1;
    endif

	if (isBloodied(defender))
		if (temHabilidade(who, "Golpe de Misericordia"))
			other:= other + 0.75; //75% - balanceamento depois da mudanca de calculo do dano
		endif
	endif

	if ( IsRaging(who))
		if (temHabilidade(who, "Baixo Limiar de Furia") || GetObjProperty(who, "NpcFuria"))
			other:= other + 1.0; //50% de bonus - balanceamento depois da mudanca de calculo do dano
			PrintText(who, "*enfurecido*");
		endif
		if (temHabilidade(who, "Furia Primitiva"))
			if (GetObjProperty(who, "metamorfose") != error)
				other:= other + 1.0; //50% de bonus - balanceamento depois da mudanca de calculo do dano
				PrintText(who, "*enfurecido*");
			endif
		endif
		if (GetCooldown(who, "FuriaPrimordial"))
			var inimigos := ListHostiles(who, 3, LH_FLAG_LOS);
			inimigos := inimigos.size();
			other := other + 0.15*inimigos; //15% de bonus - balanceamento depois da mudanca de calculo do dano
		endif
		if (GetCooldown(who, "furiamortal"))
			var bonus := (AP_GetVitalMaximumValue(who, HITS) - AP_GetVital(who, HITS))/150.0;
			other := other + bonus;
		endif
	endif
	var backStab := CDbl(GetObjProperty(who, "#backstab"));
	if (backStab)
		other := other + (0.1*backStab);
		EraseObjProperty(who, "#backstab");
	endif

	if (GetObjProperty(who, "twohanded") == 1)
		other := other + 0.1; //10% de bonus - balanceamento depois da mudanca de calculo do dano
	endif

	if (GetObjProperty(who, "fococomarma") == who.weapon.objtype)
		other := other + 0.1; //10% de bonus - balanceamento depois da mudanca de calculo do dano
	endif

	if ( GetObjProperty(who, "armapredileta") == who.weapon.objtype )
		other := other + 0.5; //50% de bonus - balanceamento depois da mudanca de calculo do dano
	endif

	if (GetObjProperty(who, "#SequenciaDevastadoraAcumulada"))
		other := CDbl(GetObjProperty(who, "#SequenciaDevastadoraAcumulada")) * 0.2; //20% de bonus - balanceamento depois da mudanca de calculo do dano
	endif
    var itemdesc := ReadConfigFile("::itemdesc");

    if (TemHabilidade(who, "Brigador de Taverna") && (who.weapon.objtype == 0x1F020))
        dmg += 5;
    endif

    if (TemHabilidade(who, "Artista Marcial") )
		var monk_weapons := array{ 0x0df0, 0x13f9, 0x0e8a, 0x0e81, 0x1F020};
        var isArmor := array{"ArmaduraPesada", "ArmaduraMedia", "ArmaduraLeve"};
    	if ( who.weapon.objtype in monk_weapons)
			other := other + 2.0;
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

	//Multiplicador de tactics X multiplicador de força X multiplicadores de habilidades e itens X Multiplicador de buff X multiplicador de Debuff
	//dmg := CInt(dmg*(1.0+(2*tatics))*(1.0+str)*(1.0+(weapon+other))*(inimigo_predileto)*(1.0+blessbonus)*(1.0-reducao_dano));
	//dmg := CInt(dmg*Pow(1.0+tatics,1.5)*(1.0+str)*(1.0+(weapon+other))*(inimigo_predileto)*(1.0+blessbonus)*(1.0-reducao_dano));
	dmg := CInt(dmg * (Pow(1.0+tatics,1.5) + str) * (1.0+(weapon+other+blessbonus)) * (inimigo_predileto) * (1.0-reducao_dano));
	if (dmg < 1)
		dmg := 1;
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
    if (GetObjProperty(defender, "chardata").trait == "Defensor Anão")
        totaldmg -= Cint(AP_GetSkill(defender, PARRY) / 30);
    endif
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

	if (cint(GetObjProperty(defender, "#vinganca")) > 1 )
		SetObjProperty( defender, "#vinganca", (cint(GetObjProperty(defender, "#vinganca")) + totaldmg));
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
