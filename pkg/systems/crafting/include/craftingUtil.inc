use cfgfile;

include "include/damage";
include ":tn:tngumps";
include ":itemsys:propitens";

const  EXCEP := 1;
const  NORMAL := 2;
const  POOR := 3;
const STR_ERRORCHANCE := "ERROR 666";
const STR_NULLCHANCE := "nula";
const STR_VERYLOWCHANCE := "muito baixa";
const STR_LOWCHANCE := "baixa";
const STR_INTERMEDIATECHANCE := "media";
const STR_GOODCHANCE := "alta";
const STR_VERYHIGHCHANCE := "muito alta";

//funcao que pergunta por fechadura, caso o item seja lockable
function CraftLockable(who, theitem)
		if (theitem.isDoor())
			SetObjProperty(theitem, "pedreiro", who.serial);
			SetObjProperty(theitem, "criador", who.acctname);
			SetObjProperty(theitem, "validade", CInt(ReadGameClock()+360));
			MoveObjectToLocation(theitem, who.x, who.y, who.z, who.realm, MOVEOBJECT_NORMAL);
			theitem.movable := 0;
		endif

		if (KP_IsLockable(theitem ))
			var fechadura := YesNo(who, "Voce deseja por uma fechadura nesse item?");
			if (fechadura)
				SendSysMessageEx(who, "Escolha a fechadura para por neste item.", SSM_REQUEST);
				var targ := Target(who);
				if (targ)
					if (targ.objtype == 4001)
						if (!theitem.isDoor())
							var lvl := Cint(GetObjProperty(targ, "level"));
							if (!lvl)
								lvl := 1;
							endif
							SetObjProperty(theitem, "level", lvl);
						endif
						var lockid := CInt(GetObjProperty(targ, "LockId"));
						SetObjProperty(theitem, "LockId", lockid);
						DestroyItem(targ);
						SendSysMessageEx(who, "Voce instalou a fechadura com sucesso.", SSM_INFO);
					else
						SendSysMessageEx(who, "Isto nao parece uma fechadura.", SSM_FAIL);
					endif
				endif
			endif
		endif
endfunction

function GetMaterialName(obj);

  if ( (obj == "wood") || (obj == "leather") || (obj == "cloth") || (obj == "metal") || (obj == "glass") || (obj == "ossos") || (obj == "stone") || obj == "arrowhead")
    return obj;
  else
    var items := ReadconfigFile("::itemdesc");
    if (items[obj])
      return items[obj].desc;
    endif
  endif


endfunction

//Reference: http://uo.stratics.com/content/aos/baseproperties.shtml#exceptional
// as stratics says, exceptional weapons add +35 dmg increase

function AddQualityDamageMod(who, craft_skill, byref theitem, quality )
	if (quality >= 1.2)
//		var randomDmg; //Bors sugestão excluído dano elemental aleatório em EXP
//		var maxBonus := CInt(quality*20); //mínimo 24
		var maxBonus := CInt(quality*10); //novo mínimo de 12
		/* BORS
		Tabulamos bônus de 5% a 25% conforme as classes de itens.
		Contudo a estrutura do script usa a qualidade do item como multiplicador
		A qualidade do item vai até 3.0
		Então acabei arrendondando pra 12% a 30% de bônus*/


		//adiciona 5% a 20%
//		var i := 0;
		var cfg:=ReadConfigFile(":combat:itemdesc");
		var elem := FindConfigElem(cfg, theitem.objtype);
//		var qtd := 5;

		var attr := PegarProp(theitem);
		var attr_excp := PegarPropExcep(theitem);
		var physicalmod, firemod, coldmod, poisonmod, energymod;

		attr_excp["dmgmod"] := 0;
		attr_excp["speedmod"] := 0;
		attr_excp["physicalmod"] := 0;
		attr_excp["firemod"] := 0;
		attr_excp["coldmod"] := 0;
		attr_excp["poisonmod"] := 0;
		attr_excp["energymod"] := 0;

//ADD BORS - Novo Bônus EXP
		if (theitem.damage_physical_mod>0);
		theitem.damage_physical_mod := theitem.damage_physical_mod + maxBonus;
		attr_excp["physicalmod"] := maxBonus;
		endif
		if (theitem.damage_fire_mod>0);
		theitem.damage_fire_mod := theitem.damage_fire_mod + maxBonus;
		attr_excp["firemod"] := maxBonus;
		endif
		if (theitem.damage_cold_mod>0);
		theitem.damage_cold_mod := theitem.damage_cold_mod + maxBonus;
		attr_excp["coldmod"] := maxBonus;
		endif
		if (theitem.damage_poison_mod>0);
		theitem.damage_poison_mod := theitem.damage_poison_mod + maxBonus;
		attr_excp["energymod"] := maxBonus;
		endif
		if (theitem.damage_energy_mod>0);
		theitem.damage_energy_mod := theitem.damage_energy_mod + maxBonus;
		attr_excp["poisonmod"] := maxBonus;
		endif


// BORS sugestao eliminado bônus de dano aleatório - Somente bônus dos elementos dos ORES agora
		var modadc := (((quality-1)/20)); //Bors Sugestao mudado cálculo para Habs darem no máximo 10% a mais - antigamente podia chegar a 60% 
		if(craft_skill == "MetalWorking")
			if(TemHabilidade(who,"Armas Velozes"))
				//SendSysMessage(who,"bonus velocidade" + (quality-1)/2 + "qualidade: "+ quality);
				SetObjProperty(theitem,"attackspeedbonus", CInt(elem.Speed * modadc) + attr."hab_speedmod"); //VAI FICAR OBSOLETA DEPOIS DA IMPLEMENTAÇÃO DO propitens.inc
				attr_excp["speedmod"] := CInt(elem.Speed * modadc) + attr."hab_speedmod"; //REVER PARA USAR APENAS ESTA PROP NOS OUTROS LOCAIS
				//theitem.speed_mod := CInt(elem.Speed * (0.1 + (quality-1)/4));
			endif
			//Acplica dano extra pra arma
			if(TemHabilidade(who,"Armas Letais") && (elem.TipoAtaque == "Cortante"))
				//SendSysMessage(who,"bonus dano" + theitem.quality-1);
				theitem.dmg_mod := CInt(elem.MaxDam * modadc) + attr."hab_dmgmod"; //VAI FICAR OBSOLETA DEPOIS DA IMPLEMENTAÇÃO DO propitens.inc
				attr_excp["dmgmod"] := CInt(elem.MaxDam * modadc) + attr."hab_dmgmod";
			endif
			if(TemHabilidade(who,"Armas Letais") && (elem.TipoAtaque == "Perfurante"))
				theitem.dmg_mod := CInt(elem.MaxDam * modadc) + attr."hab_dmgmod"; //VAI FICAR OBSOLETA DEPOIS DA IMPLEMENTAÇÃO DO propitens.inc
				attr_excp["dmgmod"] := CInt(elem.MaxDam * modadc) + attr."hab_dmgmod";
			endif
			if(TemHabilidade(who,"Armas Letais") && (elem.TipoAtaque == "Contusivo"))
				theitem.dmg_mod := CInt(elem.MaxDam * modadc) + attr."hab_dmgmod"; //VAI FICAR OBSOLETA DEPOIS DA IMPLEMENTAÇÃO DO propitens.inc
				attr_excp["dmgmod"] := CInt(elem.MaxDam * modadc) + attr."hab_dmgmod";
			endif
		elseif(craft_skill == "WoodWorking")
			if(TemHabilidade(who,"Entalhe para Combate"))
				SetObjProperty(theitem,"attackspeedbonus", CInt(elem.Speed * modadc)+ attr["hab_speedmod"]); //VAI FICAR OBSOLETA DEPOIS DA IMPLEMENTAÇÃO DO propitens.inc
				attr_excp["speedmod"] := CInt(elem.Speed * modadc) + attr["hab_speedmod"];
				//theitem.speed_mod := CInt(elem.Speed * modadc); //BUGADO
			endif
			if(TemHabilidade(who,"Flexao da Madeira")) //&& (Cint(elem.maxrange) > 2))
				theitem.dmg_mod := CInt(elem.MaxDam * modadc) + attr."hab_dmgmod"; //VAI FICAR OBSOLETA DEPOIS DA IMPLEMENTAÇÃO DO propitens.inc
				attr_excp["dmgmod"] := CInt(elem.MaxDam * modadc) + attr."hab_dmgmod";
			endif
		endif
		SetarPropExcep(theitem, attr_excp); //ATIVAR
	endif
	return;
endfunction

function AddQualityResistMod(who, craft_skill, byref theitem, quality)
	if (quality >= 1.2)
		var randomDmg;
		var maxBonus := CInt(quality);
		var attr_excp := PegarPropExcep(theitem);
		//SendSysMessage(who,"Adicionando resistencia do excepcional");

		var bonusphy := 0; var bonusfire := 0; var bonuscold := 0; var bonuspoison := 0; var bonusenergy := 0;
		//Soma um bonus em todas as resistencias que o item já dá
		var i;
		for (i := 1; i <= maxBonus; i := i + 1)
			if (theitem.resist_physical_mod)
				theitem.resist_physical_mod := theitem.resist_physical_mod +1;
				bonusphy := bonusphy + 1;
			endif
			if (theitem.resist_fire_mod)
				theitem.resist_fire_mod := theitem.resist_fire_mod +1;
				bonusfire := bonusfire + 1;
			endif
			if (theitem.resist_cold_mod)
				theitem.resist_cold_mod := theitem.resist_cold_mod +1; 
				bonuscold := bonuscold+1;
			endif
			if (theitem.resist_energy_mod)
				theitem.resist_energy_mod := theitem.resist_energy_mod +1;  
				bonusenergy := bonusenergy+1;
			endif
			if (theitem.resist_poison_mod)
				theitem.resist_poison_mod := theitem.resist_poison_mod +1;  
				bonuspoison := bonuspoison+1;
			endif
		endfor
		attr_excp["physicalmod"] := CInt(bonusphy);
		attr_excp["firemod"] := CInt(bonusfire);
		attr_excp["coldmod"] := CInt(bonuscold);
		attr_excp["poisonmod"] := CInt(bonuspoison);
		attr_excp["energymod"] := CInt(bonusenergy);
		SetarPropExcep(theitem, attr_excp); //ATIVAR
	else
		return;
	endif
endfunction

 function AddItemDescription(who, item)
	var repeating := 1;
	var str;
	SendSysMessage(who, "Utilize apenas 3 linhas do gump.");
	while (repeating)
		str := SmallRequestGump(who, "Descreva o item", 1);
		if (len(str) < 84)
			break;
		endif
		SendSysMessage(who, "Voce excedeu o numero maximo de caracteres! Utilize apenas 3 linhas do gump");
	endwhile
	SetObjProperty(item, "description", str);
endfunction

function GetChanceString(value)
	if (TypeOf(value) == "String")
		return STR_ERRORCHANCE;
	elseif (value == 0)
		return STR_NULLCHANCE;
	elseif (value <= 20)
		return STR_VERYLOWCHANCE;
	elseif (value <= 40)
		return STR_LOWCHANCE;
	elseif (value <= 60)
		return STR_INTERMEDIATECHANCE;
	elseif (value <= 80)
		return STR_GOODCHANCE;
	elseif (value > 80)
		return STR_VERYHIGHCHANCE;
	endif
endfunction
