use uo;
use cfgfile;

include ":megacliloc:/include/tradutores";
include ":megacliloc:/include/toolTipUtils";
include ":equipsys:props";



function GetDmgStringDesc(elem, item)
	var dmg_mod := CInt(item.dmg_mod);
	var resultado := CSTR(elem.Damage);
	if (dmg_mod>0)
		resultado := resultado + " (+" + CSTR(dmg_mod) +")";
	elseif (dmg_mod<0)
		resultado := resultado + " (" + CSTR(dmg_mod) +")";
	endif
	return resultado;
	/*
	var avgdmg := cint(elem.MinDam + elem.MaxDam)/2;

	if(avgdmg >= 20)
		return "Letal";
	elseif(avgdmg >= 18)
		return "Muito";
	elseif(avgdmg >= 16)
		return "Bom";
	elseif(avgdmg >= 14)
		return "Mediano";
	elseif(avgdmg >= 12)
		return "Pouco";
	elseif(avgdmg >= 10)
		return "Pouquissimo ";
	else
		return "Inofensivo";
	endif
	*/

endfunction

function GetOffHandDmgStringDesc(elem, item)
	var dmg_mod := CInt(item.dmg_mod);
	var resultado := CSTR(elem.OffHandDamage);
	if (dmg_mod>0)
		resultado := resultado + " (+" + CSTR(dmg_mod) +")";
	elseif (dmg_mod<0)
		resultado := resultado + " (" + CSTR(dmg_mod) +")";
	endif
	return resultado;
endfunction


function GetDmgStringSpecialDmg(elem, item)
	var dmg_mod := CInt(item.dmg_mod);
	var minmod := elem.MinDam + dmg_mod;
	var maxmod := elem.MaxDam + dmg_mod;
	var mindmg := minmod*0.5 + 1;
	var maxdmg := maxmod*0.5 +1;
	var resultado := (CInt(mindmg) + "-" + Cint(maxdmg));
	if (dmg_mod>0)
		resultado := resultado;
	elseif (dmg_mod<0)
		resultado := resultado;
	endif
	return resultado;
endfunction

function getQuality(num)
	var rank;
	if(num>2.8)
		rank := "Lendária";
	elseif(num>2.5)
		rank := "Épica";
	elseif(num>2.2)
		rank := "Rara";
	elseif(num>1.9)
		rank := "Excepcional";
	else
		rank := "Aprimorada";
	endif
	return rank;
endfunction

function getCondition(item) //Durabilidade dos itens
	// return CSTR(item.hp) + "/" + CSTR(item.maxhp);

	var c := cdbl(item.hp) / cdbl(item.maxhp);

	if (c > 0.9)
		return "Boa";
	elseif (c > 0.6)
		return "Pouco Danificada";
	elseif (c > 0.3)
		return "Bastante Danificada";
	else
		return "Quebradiça";
	endif

endfunction


function GetCriticalInfo(entry)

	var criticals := GetconfigStringArray(entry, "CriticalHitScripts");

	var critstr := "";

	var str;
	foreach crit in criticals
		str := ParseScriptString(crit);
		critstr := critstr + TranslateCritical(lower(str)) + "<br>";
	endforeach

	return critstr;

endfunction

function GetHitsInfo(entry)

	var criticals := GetconfigStringArray(entry, "HitScripts");

	var critstr := "";

	var str;
	foreach crit in criticals
		str := ParseScriptString(crit);
		critstr := critstr + TranslateHit(lower(str)) + "<br>";
	endforeach

	return critstr;

endfunction

function getTipoStringDesc(elem) //Exibe o tipo de arma
	var tipo := ClassificaParametro(elem); 
	case(tipo)
		ITEM_ARMA:
		TEMPLATE_ARMA:
			return CStr(elem.TipoAtaque) + "<br>" + CSTR(StrReplace(elem.Attribute, "_", " "));
		ITEM_ARMADURA:
		TEMPLATE_ARMADURA:
			return CStr(elem.TipoProtecao) + "<br>";
	endcase
endfunction


function getVelStringDesc(elem, item) //Velocidade da arma. Mostra diferença pra item base
	var speedI := GetObjProperty(item,"attackspeedbonus");
	var itemProp := getPropBase(item);
	var speedE := CInt(itemProp.AttackSpeed);
	var speedDesc := "";
	if (speedE >= 41)
		speedDesc := "Muito Rápida";
	elseif (speedE >= 37)
		speedDesc := "Rápida";
	elseif (speedE >= 33)
		speedDesc := "Normal";
	elseif (speedE >= 29)
		speedDesc := "Lenta";
	else
		speedDesc := "Muito Lenta";
	endif

	if (speedI != error && speedI > 0)
		return speedDesc + " (+" + CSTR(speedI) + ")";
	elseif (speedI != error && speedI < 0)
		return speedDesc + " (" + CSTR(speedI) + ")";
	else
		return speedDesc;
	endif
endfunction

function getParryBonus(elem, item)
	var parrybonusI := Cint(GetObjProperty(item, "parrybonus"));
	var parrybonusE := Cint(elem.parrybonus);
	if (parrybonusI > 0)
		return CSTR(parrybonusE) + " (+" + CSTR(parrybonusI) + ")";
	elseif (parrybonusI < 0)
		return CSTR(parrybonusE) + " (" + CSTR(parrybonusI) + ")";
	else
		return CSTR(parrybonusE);
	endif
endfunction

function getParryDelay(elem, item)
	var parrydelayI := Cint(GetObjProperty(item, "parrydelay_mod"));
	var parrydelayE := Cint(elem.parrydelay);

	if (parrydelayI > 0)
		return CSTR(parrydelayE) + " (-" + CSTR(parrydelayI) + ")";
	elseif (parrydelayI < 0)
		return CSTR(parrydelayE) + " (+" + CSTR(-parrydelayI) + ")";
	else
		return CSTR(parrydelayE);
	endif
endfunction

function getDexPenalty(elem, item)
	var DexPenaltyI := Cint(GetObjProperty(item, "penaltybonus"));
	var DexPenaltyE := Cint(elem.DexPenalty);

	if (DexPenaltyI > 0)
		return CSTR(DexPenaltyE) + " (-" + CSTR(DexPenaltyI) + ")";
	elseif (DexPenaltyI < 0)
		return CSTR(DexPenaltyE) + " (+" + CSTR(-DexPenaltyI) + ")";
	else
		return CSTR(DexPenaltyE);
	endif
endfunction

function GetConsultaEspecialistaTexto(profissao)
    var profissao_traduzida := TraduzirProfissao(profissao);
    case (profissao_traduzida)
        "Ferreiro": return "Consulte um Ferreiro";
        "Alquimista": return "Consulte um Alquimista";
        "Mago": return "Consulte um Mago";
        "Sobrevivencialista": return "Consulte um Sobrevivencialista";
        "Médico": return "Consulte um Médico";
        "Trabalhador Doméstico": return "Consulte um Trabalhador Doméstico";
        "Mecânico": return "Consulte um Mecânico";
        "Carpinteiro": return "Consulte um Carpinteiro";
        "Coletor": return "Consulte um Coletor";
        "Adestrador de Animais": return "Consulte um Adestrador de Animais";
        default: return "Consulte um especialista em " + profissao_traduzida;
    endcase
endfunction