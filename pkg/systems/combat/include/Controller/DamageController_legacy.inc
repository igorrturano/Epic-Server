// Internal
include ":combat:armorZones";
// include ":combat:combat";
// include ":combat:Controller/AbilitiesController";
include ":combat:weardown";

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
	if (!GetObjProperty(targ, "nocure"))
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
		skills_combat.+sneak := AP_GetTrueSkill(mobile, SNEAK);
		skills_combat.+awareness := AP_GetTrueSkill(mobile, AWARENESS);
		skills_combat.+pickpocketing := AP_GetTrueSkill(mobile, PICKPOCKETING);
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

function DamageFLS(defender, dmg, type := DMG_FORCED, source := 0, spell := "")
	var totaldmg;
	var attackInfo := struct{};

	if (type == DMG_FORCED)
		totaldmg := CInt(dmg);
	else
		totaldmg := CInt(CalculateDamage(defender, type, dmg));
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

	// Checa as habilidades depois do dano
	// CheckHab_RecuperacaoSalvadora(0, defender, attackInfo);
	// CheckHab_Vinganca(0, defender, attackInfo);

	if (defender.frozen)
		defender.frozen := 0;
	endif

	return totaldmg;
endfunction