// O script usa três cfgs:
//* loot-group - ficam os grupos de itens;
//* loot-index - Onde os loots e baus retiram o grupo de itens (loot-group) e as chances de aparecerem e qual o efeito magico (template);
//* template - Esse é usado para saber qual efeito magico será atrelado ao item;
//
//A ideia aqui é que os itens sejam divididos quanto aos efeitos mágicos em:
//* Armas - podem ter efeito stats, vitals, chance de crítico e dmg_mod, esses dois últimos podem vir acompnhados de hitscript;
//* Armaduras e Roupas - podem ter efeito stats, vitals ou resists;
//* Joias - stats ou vitals;
use uo;
use util;
use cfgfile;
use os;

include "include/damage";
include ":tn:equipment";
include ":spells:tnmagery";

//	print("item name eh"+item.name);
//Magic Weapons 1 10 3
function Loot_CreateMagic(container, line)

	var parsed := Loot_ParseMagicString(line);

	if ( Loot_ChanceCheck(parsed.chance) )
		var i;
		for ( i:=1; i<= parsed.amount; i:=i+1 )
			var item := CreateMagicItem(container, line);
			if ( item.errortext )
				return error{"errortext" := "Error - Could not create ["+parsed.name+"] ->"+item.errortext};
			endif
			sleepms(2);
		endfor
	endif
	return 1;

endfunction

function CreateMagicItem(container, line)

	var parsed := Loot_ParseMagicString(line);
	var groups_cfg := ReadConfigFile(":loot:config/loot-groups");
	if ( groups_cfg.errortext )
		return error{"errortext" := "Error - Could not open loot-groups.cfg -> "+groups_cfg.errortext};
	endif

	var groups_elem := groups_cfg[parsed.name];
	if ( !groups_elem )
		return error{"errortext" := "Error - loot-groups.cfg elem ["+parsed.name+"] does not exist."};
	endif

	var items := GetConfigStringArray(groups_elem, "Item");
	var num_items := items.size();
	var i;
	var itemName := items[RandomInt(num_items)+1];
	var parsed2 := Loot_ParseMagicString( items[RandomInt(num_items)+1]);
	var itemobj := cint(GetObjTypeByName(itemName));
	var item := CreateItemInContainer(container, parsed2.name, 1);

	SetMagicTemplateBonus(item, parsed);
	return 1;

endfunction

function SetMagicTemplateBonus(item, parsed)

	var cfg := ReadConfigFile(":loot:config/templates");
	var elem := FindConfigElem(cfg, parsed.template);

	if (!elem)
		return elem;
	endif

	var rand := RandomInt(100) + 1;
	var possiveis := GetConfigStringDictionary( elem, "chance" );
	var achado := possiveis["all"];
	var chosen;
	
	if (achado)
		chosen := GetChosenBonus(item, possiveis, rand);
	else
		chosen := GetChosenBonusFromKeys(item, possiveis, rand);
	endif

	if (chosen)
		SetBonusOnItem(item, elem, chosen, parsed);
	endif

endfunction

function GetChosenBonus(item, possiveis, rand) // para o template all
	var margem := SplitWords(possiveis["all"]);
	var minv := Cint(margem[0]);
	var maxv := Cint(margem[1]);
	var chosenone;
	var chosen;
	if ((rand >= minv) && (rand <= maxv))
		if ( (item.isA(POLCLASS_WEAPON)) || (item.isa(POLCLASS_ARMOR)) ) 
			var random_chance := RandomInt(3);
			if (random_chance == 1)
				chosenone := "status";
			elseif (random_chance == 2)
				chosenone := "vital";
			else
				chosenone := "weaponarmor";
			endif 
		else
			var random_chance := RandomInt(3);
			if (random_chance == 1)
				chosenone := "status";
			elseif (random_chance == 2)
				chosenone := "vital";
			elseif (random_chance == 3)
				chosenone := "resistmod";
			endif 
		endif
		chosen := chosenone;
	endif

	return chosen;
endfunction

function GetChosenBonusFromKeys(item, possiveis, rand) //para os demais templates
var chosen;
	foreach x in (possiveis.keys())
		var margem := SplitWords(possiveis[x]);
		var minv := Cint(margem[0]);
		var maxv := Cint(margem[1]);

		if ((rand >= minv) && (rand <= maxv))
			chosen := x;
			break;
		endif
	endforeach

	return chosen;
endfunction

function SetBonusOnItem(item, elem, chosen, parsed)
	var bonus := GetConfigStringDictionary( elem, chosen );

	if (bonus != error)
		var bonusKeys := bonus.keys();
		var selected := bonusKeys[ RandomInt(bonusKeys.size())+1];

		var mods := dictionary;
		if (chosen == "vital")
			mods[selected] := GetBonusAmount(parsed.level, chosen);
		elseif (chosen == "status")
			mods[selected] := GetBonusAmount(parsed.level, chosen);
		elseif (chosen == "weaponarmor")
			var parsed_elem := parsed.template;
			SetMagicWeaponArmor(item, parsed.level, parsed_elem);
			return 0;
		elseif (chosen == "resistmod")
			mods[selected] := GetBonusAmount(parsed.level, chosen);
		endif

			var cores := GetConfigString(elem, "cores");
			if (cores != error)
				if (cores == "Random")
					var cor := RandomDiceRoll("1d1000");
					SetObjProperty(item, "magicColor", cor);
				else
					cores := SplitWords(cores);
					SetObjProperty(item, "magicColor", cores[RandomInt(cores.size())+1]);
				endif
			endif

			SetObjProperty(item, "equip", mods);
			SetObjProperty(item, "magic", 1);

		
	endif

endfunction

function GetBonusAmount(level, type)
var dice := RandomDiceRoll("1d"+level);
//	if (type == "skill")
	//	return (level*3)+2;
	if (type == "vital")
		return dice*4; //vitals de +1 até +20;
	elseif( type == "status")
		return dice*3;//status de +1 até +15;
	elseif( type == "resistmod")
		return dice*3;//resists de +1 até +15;
	endif

endfunction

function GetRandomResistance()

	var resist := RandomDiceRoll("1d5");

	if (resist == 1)
		return "PhysicalResistMod";
	elseif (resist == 2)
		return "FireResistMod";
	elseif (resist == 3)
		return "ColdResistMod";
	elseif (resist == 4)
		return "PoisonResistMod";
	elseif (resist == 5)
		return "EnergyResistMod";
	endif

endfunction

function SetMagicWeaponArmor(item, parsed, parsed_elem)
	var cfg := ReadConfigFile(":loot:config/templates");
	var elem := FindConfigElem(cfg, parsed_elem);

	//armas podem ter:
	// 1- dmgmod
	// 2 - aumenta chance de crítico
	// 3- hitscript
	var mods := dictionary;
	if (item.isA(POLCLASS_WEAPON) )

		var roll := RandomDiceRoll("1d2");
		if ( roll == 1) //aumento no dano
			var dmg_mod := (RandomInt(parsed)) + parsed;  //de 1 a 10
			mods.+"dmg_mod" := dmg_mod;
		elseif ( roll == 2)//aumento na chance de crítico
			mods.+"chancetocritic":= parsed * RandomInt(3); //de +1 a +15
		endif

		if (parsed > 3) //lvl 4 ou 5 tem 10% de chance de ter hitscript
			if (RandomInt(100) < 10 )
				var hits := RandomDiceRoll("1d3");
				if (hits == 1)
					mods["hitscript"] := ":combat:weaponHitScripts/FireHit";
				elseif (hits == 2)
					mods["hitscript"] := ":combat:weaponHitScripts/IceHit";
				else
					mods["hitscript"] := ":combat:weaponHitScripts/LightningHit";
				endif
			endif
		endif

	//armaduras podem ter:
	//1- aumento em alguma resist
	elseif (item.isa(POLCLASS_ARMOR) )
		var resistance := GetRandomResistance();
		mods[resistance] :=  GetBonusAmount(parsed, "resistmod");

	endif

			var cores := GetConfigString(elem, "cores");
			if (cores != error)
				if (cores == "Random")
					var cor := RandomDiceRoll("1d1000");
					SetObjProperty(item, "magicColor", cor);
				else
					cores := SplitWords(cores);
					SetObjProperty(item, "magicColor", cores[RandomInt(cores.size())+1]);
				endif
			endif


	SetObjProperty(item, "magic", 1);
	SetObjProperty(item, "equip", mods);

endfunction

function ActivateMagicItem(item)

	var mods := GetObjProperty(item, "equip");

	if (item.isA(POLCLASS_WEAPON)) //se for arma é dmg_mod; chance de critico e hitscript

		if (mods["dmg_mod"])
			item.dmg_mod := mods["dmg_mod"];
		endif

		if (mods["chancetocritic"])
			SetObjProperty(item, "CriticalChance", mods["chancetocritic"]);
		endif

		if (mods["hitscript"])
			var hit_scripts := GetObjProperty(item, "HitScripts");
			if ( !hit_scripts )
				hit_scripts := array{};
			endif
			hit_scripts.append(mods["hitscript"]);
			SetObjProperty(item, "HitScripts", hit_scripts);
		endif

	elseif (item.isA(POLCLASS_ARMOR)) //se for armadura é resist, vitals ou stats
		var equip := GetObjProperty(item, "equip");
		if (!equip)
			equip := dictionary;
		endif
		foreach key in (mods.keys())
			if (key == "PhysicalResistMod")
				item.resist_physical_mod := item.resist_physical_mod + cint(mods[key]);
			elseif (key == "ColdResistMod")
				item.resist_cold_mod := item.resist_cold_mod + cint(mods[key]);
			elseif (key == "FireResistMod")
				item.resist_fire_mod := item.resist_fire_mod + cint(mods[key]);
			elseif (key == "EnergyResistMod")
				item.resist_energy_mod := item.resist_energy_mod + cint(mods[key]);
			elseif (key == "PoisonResistMod")
				item.resist_poison_mod := item.resist_poison_mod + cint(mods[key]);
			else
				equip[key] := mods[key];
			endif
		endforeach
		SetObjProperty(item, "equip", equip);
	else//se for joia ou outro é bonus de stats, vitals ou resist
		var equip := GetObjProperty(item, "equip");
		if (!equip)
			equip := dictionary;
		endif
		foreach key in (mods.keys())


			if (key == "PhysicalResistMod")
				item.resist_physical_mod := item.resist_physical_mod + cint(mods[key]);
				printtext(item,"yaaaaa");
			elseif (key == "ColdResistMod")
				item.resist_cold_mod := item.resist_cold_mod + cint(mods[key]);
			elseif (key == "FireResistMod")
				item.resist_fire_mod := item.resist_fire_mod + cint(mods[key]);
			elseif (key == "EnergyResistMod")
				item.resist_energy_mod := item.resist_energy_mod + cint(mods[key]);
			elseif (key == "PoisonResistMod")
				item.resist_poison_mod := item.resist_poison_mod + cint(mods[key]);
			else
				equip[key] := mods[key];
			endif
		endforeach
		SetObjProperty(item, "equip", equip);
	endif

	if (GetObjProperty(item, "magicColor") != error) //seta a cor
		item.color := Cint(GetObjProperty(item, "magicColor"));
		EraseObjProperty(item, "magicColor");
	endif

	var spellinfo := GetObjProperty(item, "spellinfo");
	if (spellinfo != error)
		if (spellinfo.id < 100)
			var cfg := ReadConfigFile(":tn:config/tnspells");
			var elem := FindConfigElem(cfg, spellinfo.id);
			var name := GetConfigString(elem, "Name");
			//printtextabove(item, " " + name);
			if (name && GetObjProperty(item, "magic") != 2)
				SetName(item, item.desc+" ["+name+"]");
			endif
		endif
	endif
	MAGIC_SetupName(item); //seta o nome
	SetObjProperty(item, "magic", 2);

endfunction

function Loot_ParseMagicString(line)
	line := SplitWords(line);

	var name := CStr(line[1]);

	var amount := CStr(line[2]);
	amount := RandomDiceRoll(amount);

	var chance := CDbl(line[3]);
	if ( !chance )
		chance := 100.0;
	endif

	var level := CInt(line[4]);
	if (!level)
		level := 1;
	endif

	var template := Cstr(line[5]);

	var result := struct;
	result.+name := name;
	result.+amount := amount;
	result.+chance := chance;
	result.+level := level;
	result.+template := template;

	return result;
endfunction


function MAGIC_SetupName(item)


	var npc_names := ReadConfigFile( ":loot:config/item-names");
	var name_list_substantivo := GetConfigStringArray(npc_names["Substantivo"], "Name");
	var name_list_adjetivo := GetConfigStringArray(npc_names["Adjetivo"], "Name");
	var name_list_adverbio := GetConfigStringArray(npc_names["Adverbio"], "Name");

	var my_name := item.name;
		var replacement1 := name_list_substantivo[RandomInt(name_list_substantivo.Size())+1];
		var replacement2 := name_list_adjetivo[RandomInt(name_list_adjetivo.Size())+1];
		var replacement3 := name_list_adverbio[RandomInt(name_list_adverbio.Size())+1];
		my_name := item.name +" "+replacement1+" "+replacement2+" "+replacement3;
	Setname(item, my_name);
	return 1;
endfunction

//está SEM uso por enquanto, script antigo
function SetMagicBonus(item, parsed)
	//armas podem ter:
	// 1- dmgmod
	// 2- aumento na chance de acerto
	var mods := dictionary;
	if (item.isA(POLCLASS_WEAPON) )

		var roll := RandomDiceRoll("1d2");
		if ( roll == 1) //aumento no dano
			var dmg_mod := (RandomInt(parsed.level)) + parsed.level;  //de 1 a 10
			mods["dmg_mod"] := dmg_mod;
		elseif ( roll == 2)
			mods["chancetocritic"]:= parsed.level * 3;
			//SetObjProperty(item, "chancetohit", parsed.level * 3); //de +3 a +15
		endif

		if (parsed.level > 3) //lvl 4 ou 5 tem 10% de chance de ter hitscript
			if (RandomInt(100) < 10 )
				var hits := RandomDiceRoll("1d3");
				if (hits == 1)
					mods["hitscript"] := ":combat:weaponHitScripts/FireHit";
				elseif (hits == 2)
					mods["hitscript"] := ":combat:weaponHitScripts/IceHit";
				else
					mods["hitscript"] := ":combat:weaponHitScripts/LightningHit";
				endif
			endif
		endif

	//armaduras podem ter:
	//1- aumento em alguma defesa
	elseif (item.isa(POLCLASS_ARMOR) )
		var resistance := GetRandomResistance();
		//var resistances := GetObjProperty(item, "Resistances");

		//resistances[resistance] := resistances[resistance] + parsed.level * 2; //de +2 a +10
		//SetObjProperty(item, "MagicResistances", resistances);
		mods[resistance] :=  parsed.level * 2;

	//se for outros itens (roupas ou joias)
	// 1- bonus no stat,
  	// 2- vitals ou
	// 3- skills
	else
		var nroll := RandomDiceRoll("1d3");
		var stat;
		var amt;
		if (nroll == 1)
			stat := RandomDiceRoll("1d3");
			amt := (RandomInt(parsed.level)) + parsed.level*2;  //de 3 a 15
			if (stat == 1)
				mods["strmod"] := amt;
			elseif (stat == 2)
				mods["dexmod"] :=  amt;
			else
				mods["intmod"] := amt;
			endif
		else //(nroll == 2)
			stat := RandomDiceRoll("1d3");
			amt := (RandomInt(parsed.level)) + (parsed.level)*2;  //de 3 a 15
			if (stat == 1)
				mods["hitsmod"] :=  amt;
			elseif (stat == 2)
				mods["manamod"] :=  amt;
			else
				mods["stammod"] :=  amt;
			endif
		//else
			//amt := (RandomInt(parsed.level)) + parsed.level*2;  //de 3 a 15
			//SetEquipMod(item, skilllist[RandomInt(skilllist.size())+1 ], amt);
		endif

	endif

	SetObjProperty(item, "magic", 1);
	SetObjProperty(item, "equip", mods);

endfunction