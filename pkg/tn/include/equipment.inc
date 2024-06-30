use uo;
use cfgfile;

include ":attributes:attributes";
include ":charactercreation:chargumps";
include ":charactercreation:habilidades";
include "include/client";

function SetEquipmentMods(who, item)

	//SendSysMessage(who, "equip");

	var itemdesc := ReadConfigFile("::itemdesc");

	var restrict := Cint(itemdesc[item.objtype].GenderSpecific);
	if((restrict == 1) && (who.graphic == 400 ||who.graphic == 1828 || who.graphic == 1830 || who.graphic == 1832 || who.graphic == 1834 ))
		SendSysMessage(who, "Apenas mulheres podem vestir isto.");
		return 0;
	endif

	if (item.isa(POLCLASS_ARMOR) && (item.tile_layer == LAYER_HAND2))
		EraseObjProperty(who, "twohanded");
		var mod_percentual := 0.0;
		var parryskill := AP_GetSkill(who, PARRY);
		var atributos_escudo := Dictionary;
		atributos_escudo["physical"] := item.resist_physical-item.resist_physical_mod;
		atributos_escudo["physicalmod"] := item.resist_physical_mod;
		atributos_escudo["fire"] := item.resist_fire-item.resist_fire_mod;
		atributos_escudo["firemod"] := item.resist_fire_mod;
		atributos_escudo["cold"] := item.resist_cold-item.resist_cold_mod;
		atributos_escudo["coldmod"] := item.resist_cold_mod;
		atributos_escudo["poison"] := item.resist_poison-item.resist_poison_mod;
		atributos_escudo["poisonmod"] := item.resist_poison_mod;
		atributos_escudo["energy"] := item.resist_energy-item.resist_energy_mod;
		atributos_escudo["energymod"] := item.resist_energy_mod;
		atributos_escudo["_equipped"] := 1;


		mod_percentual := (parryskill-80.0) /100.0; //Acima de 0 o cara consegue mais proteção que o item base oferece e abaixo, menos.
		SendSysMessage(who, "Eficiencia do escudo: " + CStr((1.0 + mod_percentual)*100.0) +"%.");
		var shieldequipped := item.GetProp("atributos_escudo");
		if (shieldequipped["_equipped"] != 1) //Gambiarra pra evitar que o POL mude as resistência no logon
			SetObjProperty(item, "atributos_escudo", atributos_escudo);
			SetResists(who, item, mod_percentual);
		endif
	endif

	if (item.isa(POLCLASS_WEAPON))
		item.speed_mod := CInt(GetObjProperty(item,"attackspeedbonus"));
	endif
    var chardata := GetObjProperty(who, "chardata");
	var penalty  := CInt(itemdesc[item.objtype].DexPenalty);
    var isScutum := {0x1B76, 0x1B77, 0xc68d};
    if (chardata.trait == "Força da Legião" && (item.objtype in isScutum))
        penalty /= 2;
    endif
	var penaltyBonus := Cint(getObjProperty(item, "penaltybonus"));
	penalty := penalty - penaltyBonus;
	if (penalty < 0)
		penalty := 0;
	endif
	AP_ModifyStatMod(who, DEXTERITY, (0-penalty));

	//sneak
	penalty := CInt(itemdesc[item.objtype].SneakPenalty);
	if (TemHabilidade(who, "Ignorar Armadura"))
		penalty := cint(penalty*0.5);
	endif
	AP_ModifySkillMod(who, SNEAK, penalty);

	//mechanical apitude
	penalty := CInt(itemdesc[item.objtype].MechanicalApitudePenalty);
	if (TemHabilidade(who, "Ignorar Armadura"))
		penalty := cint(penalty*0.5);
	endif
	AP_ModifySkillMod(who, MECHANICALAPITUDE, penalty);

	//awareness
	penalty := CInt(itemdesc[item.objtype].AwarenessPenalty);
	if (TemHabilidade(who, "Ignorar Armadura"))
		penalty := cint(penalty*0.5);
	endif
	AP_ModifySkillMod(who, AWARENESS, penalty);

	//magery
	penalty := CInt(itemdesc[item.objtype].MageryPenalty);
	if (!(TemHabilidade(who, "Senhor do Medo") || TemHabilidade(who, "Devocao") || TemHabilidade(who, "Colheita de Almas")|| TemHabilidade(who, "Campeao das Montanhas")))
        AP_ModifySkillMod(who, MAGERY, penalty);
    endif
    
	//verifica se eh magico
	var mods := GetObjProperty(item, "equip");
//	SendSysMessage(who, " " + mods + " " + mods.keys().size());
	if (mods.keys().size() >= 1)
    		foreach mod in (mods.keys())
      			if ((mod == "strmod") &&(GetObjProperty(item,"magic")!=1))
        			AP_ModifyStatMod(who, STRENGTH, Cint( mods["strmod"] ) );
      			elseif ((mod == "dexmod" )&&(GetObjProperty(item,"magic")!=1))
        			AP_ModifyStatMod(who, DEXTERITY, Cint( mods["dexmod"] ) );
      			elseif ((mod == "intmod")&&(GetObjProperty(item,"magic")!=1))
        			AP_ModifyStatMod(who, INTELLIGENCE, Cint( mods["intmod"] ) );
      			elseif ((mod == "setprop" )&&(GetObjProperty(item,"magic")!=1))
        			SetObjProperty(who, ("#" + GetObjProperty(item, "propname")), GetObjProperty(item, "propvalue"));
      			elseif ((mod == "manamod")&&(GetObjProperty(item,"magic")!=1))
        			SetObjProperty(who, "#manamod", Cint(GetObjProperty(who, "#manamod")) + Cint(mods["manamod"] ) );
        			RecalcVitals(who);
     			elseif ((mod == "hitsmod")&&(GetObjProperty(item,"magic")!=1))
        			var prevhits := Cint(GetObjProperty(who, "#hitsmod"));
        			SetObjProperty(who, "#hitsmod",  prevhits + Cint(mods["hitsmod"] ) );
        			RecalcVitals(who);
     			elseif ((mod == "stammod")&&(GetObjProperty(item,"magic")!=1))
        			SetObjProperty(who, "#stammod", Cint(GetObjProperty(who, "#stammod")) + Cint( mods["stammod"] ) );
        			RecalcVitals(who);				
      			elseif ( ((mod == "ColdResistMod")&&(GetObjProperty(item,"magic")!=1)) || ((mod == "FireResistMod")&&(GetObjProperty(item,"magic")!=1)) || ((mod == "PhysicalResistMod")&&(GetObjProperty(item,"magic")!=1)) || ((mod == "EnergyResistMod")&&(GetObjProperty(item,"magic")!=1)) || ((mod == "PoisonResistMod")&&(GetObjProperty(item,"magic")!=1)) )
        			//var mod_cold := Cint(mods["ColdResistMod"]);
					//who.resist_cold_mod := who.resist_cold_mod - Cint(mods["ColdResistMod"]);
        			RecalcVitals(who);				
      		else // se nao eh nada acima eh skillmod ou error :p
				if (GetObjProperty(item,"magic")!=1)
					//sendsysmessage(who, "mod: "+mod+" "+ Cint(mods[mod]));
	 				AP_ModifySkillMod(who, mod, Cint(mods[mod]));
				endif
     			endif
    		endforeach
	endif

	return 1;
endfunction;

function SetResists(who, item, eff)
	var result, resultmod, resultresist, resistmod, resistbase, resist;
	var limit := 0;
	var attr_escudo := item.GetProp("atributos_escudo");

	resist := item.resist_physical;
	resistmod := item.resist_physical_mod;
	resistbase := item.resist_physical - item.resist_physical_mod;
	result := resist * (1 + eff);
	resultmod := result - (resistbase);
	resultresist := resist + resultmod;
	if (resultresist <= limit) item.resist_physical_mod := CInt(resist * -1) - resistbase;
	else item.resist_physical_mod := CInt(resultmod);
	endif

	resist := item.resist_fire;
	resistmod := item.resist_fire_mod;
	resistbase := item.resist_fire - item.resist_fire_mod;
	result := resist * (1 + eff);
	resultmod := result - (resistbase);
	resultresist := resist + resultmod;
	if (resultresist <= limit) item.resist_fire_mod := CInt(resist * -1) - resistbase;
	else item.resist_fire_mod := CInt(resultmod);
	endif

	resist := item.resist_cold;
	resistmod := item.resist_cold_mod;
	resistbase := item.resist_cold - item.resist_cold_mod;
	result := resist * (1 + eff);
	resultmod := result - (resistbase);
	resultresist := resist + resultmod;
	if (resultresist <= limit) item.resist_cold_mod := CInt(resist * -1) - resistbase;
	else item.resist_cold_mod := CInt(resultmod);
	endif

	resist := item.resist_poison;
	resistmod := item.resist_poison_mod;
	resistbase := item.resist_poison - item.resist_poison_mod;
	result := resist * (1 + eff);
	resultmod := result - (resistbase);
	resultresist := resist + resultmod;
	if (resultresist <= limit) item.resist_poison_mod := CInt(resist * -1) - resistbase;
	else item.resist_poison_mod := CInt(resultmod);
	endif

	resist := item.resist_energy;
	resistbase := item.resist_energy - item.resist_energy_mod;
	resistmod := item.resist_energy_mod;
	result := resist * (1 + eff);
	resultmod := result - (resistbase);
	resultresist := resist + resultmod;
	if (resultresist <= limit) item.resist_energy_mod := CInt(resist * -1) - resistbase;
	else item.resist_energy_mod := CInt(resultmod);
	endif
endfunction

function SetEquipMod(item, modname, value)

	var mods := GetObjProperty(item, "equip");

	if (!mods)
		mods := array;
	endif

	if ((modname != "skillmod")&&(modname!=error))
		mods.append(modname);
		SetObjProperty(item, modname, value);
	else
		mods.append(modname);
		var skill := array;
		skill.append( skilllist[RandomIntMinMax(1, skilllist.size()+1)]);
		var skillamt := array;
		skillamt.append(value);
		SetObjProperty(item, "skillmod", skill );
		SetObjProperty(item, "skillamt", skillamt );
	endif

	SetObjProperty(item, "equip", mods);

endfunction

function RemoveEquipmentMods(who, item)
	var itemdesc := ReadConfigFile("itemdesc");
	var penalty := CInt(itemdesc[item.objtype].DexPenalty);
	var penaltyBonus := Cint(getObjProperty(item, "penaltybonus"));
	penalty := penalty - penaltyBonus;
	if (penalty < 0)
		penalty := 0;
	endif

	if (item.isa(POLCLASS_ARMOR) && (item.tile_layer == LAYER_HAND2))
		var atributos_escudo := GetObjProperty(item, "atributos_escudo");

		item.resist_physical_mod := atributos_escudo["physicalmod"];
		item.resist_fire_mod := atributos_escudo["firemod"];
		item.resist_cold_mod := atributos_escudo["coldmod"];
		item.resist_poison_mod := atributos_escudo["poisonmod"];
		item.resist_energy_mod := atributos_escudo["energymod"];
		
		atributos_escudo["_equipped"] := 0;
		SetObjProperty(item, "atributos_escudo", atributos_escudo);
	endif

	if (item.isa(POLCLASS_WEAPON))
		item.speed_mod := 0;
	endif

	AP_ModifyStatMod(who, DEXTERITY, (penalty));

	//sneak
  	penalty := CInt(itemdesc[item.objtype].SneakPenalty);
	if (TemHabilidade(who, "Ignorar Armadura"))
		penalty := cint(penalty*0.5);
	endif
  	AP_ModifySkillMod(who, SNEAK, (-1)*penalty);

	//mechanical  apitude
  	penalty := CInt(itemdesc[item.objtype].MechanicalApitudePenalty);
	if (TemHabilidade(who, "Ignorar Armadura"))
		penalty := cint(penalty*0.5);
	endif
  	AP_ModifySkillMod(who, MECHANICALAPITUDE, (-1)*penalty);

	//awareness
  	penalty := CInt(itemdesc[item.objtype].AwarenessPenalty);
	if (TemHabilidade(who, "Ignorar Armadura"))
		penalty := cint(penalty*0.5);
	endif
  	AP_ModifySkillMod(who, AWARENESS, (-1)*penalty);

	penalty := CInt(itemdesc[item.objtype].MageryPenalty);
	if (!(TemHabilidade(who, "Senhor do Medo") || TemHabilidade(who, "Devocao") || TemHabilidade(who, "Colheita de Almas") || TemHabilidade(who, "Campeao das Montanhas")))
	  	AP_ModifySkillMod(who, MAGERY, (-1)*penalty);
	endif

	var mods := GetObjProperty(item, "equip");
	if (mods.keys().size() >= 1)
    		foreach mod in (mods.keys())
      			if ((mod == "strmod" ) && (GetObjProperty(item,"magic") !=1))
        			AP_ModifyStatMod(who, STRENGTH, (-1)*Cint( mods["strmod"] ));
      			elseif ((mod == "dexmod"  )&& (GetObjProperty(item,"magic") !=1))
        			AP_ModifyStatMod(who, DEXTERITY, (-1)*Cint( mods["dexmod"] ));
      			elseif ((mod == "intmod" )&& (GetObjProperty(item,"magic") !=1))
        			AP_ModifyStatMod(who, INTELLIGENCE, (-1)*Cint( mods["intmod"] ));
      			elseif ((mod == "setprop" )&& (GetObjProperty(item,"magic") !=1))
		        	EraseObjProperty(who, "#" + GetObjProperty(item, "propname"));
      			elseif ((mod == "manamod")&& (GetObjProperty(item,"magic") !=1))
        			SetObjProperty(who, "#manamod", Cint(GetObjProperty(who, "#manamod")) - Cint( mods["manamod"] ));
        			RecalcVitals(who);
      			elseif ((mod == "hitsmod" )&& (GetObjProperty(item,"magic") !=1))
        			var hitswho := Cint(GetObjProperty(who, "#hitsmod"));
        			var hitsitem :=  Cint( mods["hitsmod"] );
        			var final := hitswho - hitsitem;
        			if (final <= 0)
          				EraseObjProperty(who, "#hitsmod");
        			else
          				SetObjProperty(who, "#hitsmod", final );
        			endif
        			RecalcVitals(who);
      			elseif ((mod == "stammod")&& (GetObjProperty(item,"magic") !=1))
        			SetObjProperty(who, "#stammod", Cint(GetObjProperty(who, "#stammod")) - Cint( mods["stammod"] ));
        			RecalcVitals(who);
      			elseif ( ((mod == "ColdResistMod")&&(GetObjProperty(item,"magic")!=1)) || ((mod == "FireResistMod")&&(GetObjProperty(item,"magic")!=1)) || ((mod == "PhysicalResistMod")&&(GetObjProperty(item,"magic")!=1)) || ((mod == "EnergyResistMod")&&(GetObjProperty(item,"magic")!=1)) || ((mod == "PoisonResistMod")&&(GetObjProperty(item,"magic")!=1)) )
        			//var mod_cold := Cint(mods["ColdResistMod"]);
					//who.resist_cold_mod := who.resist_cold_mod - Cint(mods["ColdResistMod"]);
        			RecalcVitals(who);
      			else
				if (GetObjProperty(item,"magic") !=1)
					AP_ModifySkillMod(who, mod, (-1)*Cint(mods[mod]));
				endif

     			 endif
		endforeach
	endif

endfunction

function coverFace(item)
	case(item.graphic)
		0x1408: 0x1409: 0x1412:
		0x1419: 0x1451: 0x1F0B:
		0x140C: 0x140D: 0x1456:
		0x1F0C: 0x1549: 0x154A:
		0x154B: 0x154C:	0x278E:
		0x278F: 0x1388:
			return 1;
		default:
			return 0;
	endcase
endfunction

function EnumeratePhysicalItems(mobile)
	var EquipList := array;

	var SkipLayers := {11, 16, 21, 25};
	var i;

	for (i:=0; i<=25; i:=i+1)
		if (!(i in SkipLayers))
			var layer := GetEquipmentByLayer(mobile, i);
			if (layer)
				EquipList.append(layer);
			endif
		endif
	endfor

	return EquipList;
endfunction

function UnequipAll(who)

	var equips := EnumeratePhysicalItems(who);

	foreach equip in equips
		MoveItemToContainer(equip, who.backpack);
	endforeach

endfunction

function UnequipArmor(who)
	var equips := EnumeratePhysicalItems(who);

	foreach equip in equips
		if ((equip.isa(POLCLASS_ARMOR)) && equip.resist_physical > 0)
			MoveItemToContainer(equip, who.backpack);
		endif
	endforeach

endfunction

function UnequipWeapons(who)

	var equips := EnumeratePhysicalItems(who);

	foreach equip in equips
		if ((equip.isa(POLCLASS_WEAPON)))
			MoveItemToContainer(equip, who.backpack);
		endif
	endforeach
endfunction
