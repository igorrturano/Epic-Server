include ":itemsys:tabelas";
/*
TODO:
ADICIONAR A ATUALIZAÇÃO NO SCRIPT QUE CRIA A PROP CREATEDBY EM TODOS OS ITENS
VER A QUESTÃO DOS ITENS MÁGICOS
VER ITENS COM RECIPE
*/

//TOGGLES PARA SEGURANÇA NA FASE DE IMPLEMENTAÇÃO
//COLOCAR O NIVEL DO CMDLEVEL MÍNIMO NO TOGGLE PARA LIMITAR O USO EM FASE DE TESTES - PLAYER -> cmdlevel = 1; GM -> cmdlevel = 4; ADMIN -> cmdlevel = 5
const TOGGLELVL_CRAFT := 0;
const TOGGLELVL_EQUIP := 0;

function NovoSistemaDeItens(who, item, TOGGLE, PROCEDIMENTO);
    if (who.cmdlevel < TOGGLE) return; endif

    if (PROCEDIMENTO == 1)
        AtualizarItemAntigo(who, item);
    elseif (PROCEDIMENTO == 2)
        SetarAllProps(item);
    elseif (PROCEDIMENTO == 3)
        AtualizarItemAntigo(who, item);
        SetarModsNoItem(who, item);
    endif
endfunction

function SetarModsNoItem(who, item)
    SetarAllProps(item);
    var attr := PegarProp(item);
    var attr_excp := PegarPropExcep(item);
    
    if (attr)
        var hpmod := attr["base_hpmod"];

        var physicalmod := attr["base_physicalmod"];
        var firemod := attr["base_firemod"];
        var coldmod := attr["base_coldmod"];
        var poisonmod := attr["base_poisonmod"];
        var energymod := attr["base_energymod"];
        var dmgmod, speedmod, dexmod, parrymod, parrydelaymod;
        
        item.quality := attr["Quality"];

        if (item.IsA(POLCLASS_ARMOR))
            item.maxhp_mod := hpmod;

            item.resist_physical_mod := physicalmod;
            item.resist_fire_mod := firemod;
            item.resist_cold_mod := coldmod;
            item.resist_poison_mod := poisonmod;
            item.resist_energy_mod := energymod;

            if (IsEscudo(item))
                item.SetProp("penaltybonus", attr["hab_dexpenmod"]);
                item.SetProp("parrybonus", attr["hab_parrymod"]);
                item.SetProp("parrydelay_mod", attr["hab_parrydelaymod"]);
            endif

        elseif (item.IsA(POLCLASS_WEAPON))
            dmgmod := attr["base_dmgmod"];
            speedmod := attr["base_speedmod"];

            item.maxhp_mod := hpmod;
            item.dmg_mod := dmgmod;
            item.speed_mod := speedmod; item.SetProp("attackspeedbonus", speedmod); //DEPOIS CONFERIR NOS OUTRO SCRIPTS PRA USAR SÓ A NOVA

            item.damage_physical_mod := physicalmod;
            item.damage_fire_mod := firemod;
            item.damage_cold_mod := coldmod;
            item.damage_poison_mod := poisonmod;
            item.damage_energy_mod := energymod;
        endif
    else
        SendSysMessageEx(who, "BUG 666: ITEM {} SEM VALORES NO SISTEMA! FAVOR COMUNICAR A STAFF".Format(Hex(item.objtype)));
    endif
endfunction



//FUNCTIONS DE SETAR PROPS COMPLETAS
function SetarAllProps(item) //SETA TODAS AS PROPS BASE DO ITEM
    var attr := PegarProp(item);

    if (attr."Atualiza" == 0)
        return;
    else
        SetarPropTipo(item);
        SetarPropItem(item);
        SetarPropBase(item);
    endif
endfunction

function SetarPropTipo(item) 
    var cfg := ReadConfigFile(":combat:itemdesc");
	var elem := FindConfigElem(cfg, item.objtype);
    
    var attr := PegarProp(item);

   	if (item.IsA(POLCLASS_WEAPON))
		attr["TipoEquip"] := CStr(elem.TipoAtaque);
	elseif (item.IsA(POLCLASS_ARMOR))
		attr["TipoEquip"] := CStr(elem.TipoProtecao);
	endif

    SetarProp(item, attr);
endfunction

function SetarPropItem(item) //função para setar os valores base DO ITEM!
    var attr := PegarProp(item);
    var newattr := BuscarResistSlot(item);
    var newattrdebuff := BuscarDebuffMod(item);
    var elem := BuscarValoresCfg(item);

    attr["Atualiza"] := 1;
    attr["item_hp"] := elem.MaxHP;

    if (IsEscudo(item))
        attr["item_dexpen"]     := newattrdebuff[2];
        attr["item_parry"]      := newattrdebuff[3];
        attr["item_parrydelay"] := newattrdebuff[4];

        attr["item_physical"]   := newattr[1];
        attr["item_fire"]       := 0;
        attr["item_cold"]       := 0;
        attr["item_poison"]     := 0;
        attr["item_energy"]     := 0;

    elseif (item.isA(POLCLASS_WEAPON))
        attr["item_physical"]   := newattr[2];
        attr["item_fire"]       := newattr[3];
        attr["item_cold"]       := newattr[4];
        attr["item_poison"]     := newattr[5];
        attr["item_energy"]     := newattr[6];

    elseif (item.isA(POLCLASS_ARMOR))
        //attr["item_dexpen"] := newattrdebuff[2]; //REVER PARA CONSERTAR DEPOIS

        attr["item_physical"]   := newattr[1];
        attr["item_fire"]       := newattr[2];
        attr["item_cold"]       := newattr[3];
        attr["item_poison"]     := newattr[4];
        attr["item_energy"]     := newattr[5];
    endif

    SetarProp(item, attr);
endfunction

function SetarPropBase(item) //SETA AS PROPS DE MODIFICADORES DE (MATERIAL - ITEM) JÁ COM SEUS VALORES - OS MODIFICADORES DE EXCEPCIONAIS SÃO SETADOS NO CRAFTING
    var attr := PegarProp(item);
    var attr_excp := PegarPropExcep(item);
    var newattr := Array;
    var newattrescudo := Array;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //ADICIONAR MODS BASE
    attr["Atualiza"] := 1;
    if (IsEscudo(item))
        var slot := "shield";

        attr["base_hpmod"]          := CalcPropBase(item, "hpmod", slot);
        attr["base_dexpenmod"]      := CalcPropBase(item, "dexpenmod", slot);
        attr["base_parrymod"]       := CalcPropBase(item, "parrymod", slot);
        attr["base_parrydelaymod"]  := CalcPropBase(item, "parrydelaymod", slot);

        attr["base_physicalmod"]    := CalcPropBase(item, "physicalmod", slot);
		attr["base_firemod"]        := CalcPropBase(item, "firemod", slot);
		attr["base_coldmod"]        := CalcPropBase(item, "coldmod", slot);
		attr["base_poisonmod"]      := CalcPropBase(item, "poisonmod", slot);
		attr["base_energymod"]      := CalcPropBase(item, "energymod", slot);

    elseif (item.isA(POLCLASS_ARMOR))
        var slot := "armor";

        attr["base_hpmod"]          := CalcPropBase(item, "hpmod", slot);
        attr["base_dexpenmod"]      := CalcPropBase(item, "dexpenmod", slot);

		attr["base_physicalmod"]    := CalcPropBase(item, "physicalmod", slot);
		attr["base_firemod"]        := CalcPropBase(item, "firemod", slot);
		attr["base_coldmod"]        := CalcPropBase(item, "coldmod", slot);
		attr["base_poisonmod"]      := CalcPropBase(item, "poisonmod", slot);
		attr["base_energymod"]      := CalcPropBase(item, "energymod", slot);

    elseif (item.isA(POLCLASS_WEAPON))
        var slot := "weapon";

        attr["base_hpmod"] := CalcPropBase(item, "hpmod", slot);

        attr["base_dmgmod"] := CalcPropBase(item, "dmgmod", slot);
        attr["base_speedmod"] := CalcPropBase(item, "speedmod", slot);

		attr["base_physicalmod"] := CalcPropBase(item, "physicalmod", slot);
		attr["base_firemod"] := CalcPropBase(item, "firemod", slot);
		attr["base_coldmod"] := CalcPropBase(item, "coldmod", slot);
		attr["base_poisonmod"] := CalcPropBase(item, "poisonmod", slot);
		attr["base_energymod"] := CalcPropBase(item, "energymod", slot);

    endif

    SetarProp(item, attr);
endfunction



//FUNCTIONS DE BUSCA
function BuscarValoresCfg(item) //RETORNA TODOS OS VALORES DO ITEM QUE AINDA ESTIVEREM NO .CFG
    var cfg := ReadConfigFile(":combat:itemdesc");
    var elem := FindConfigElem(cfg, item.objtype);

    return elem;
endfunction

function BuscarValorMaterial(item)
    var valor := 0;

    if(item.IsA(POLCLASS_ARMOR)) valor := TabelaMaterialResistMod(item);
    elseif(item.IsA(POLCLASS_WEAPON)) valor := TabelaMaterialDamageMod(item);
    endif

    return valor;
endfunction

function BuscarResistSlot(item)
    var resultado := Array;
    var valresist := TabelaTipoProtecaoMod(item);
    valresist := valresist[1];
    var valslot := TabelaSlot(item);
    valslot := valslot[1];
    var valmaterial := Array;

    if(IsEscudo(item)) //ESCUDO TEM SOMA BASEADO NO SLOT

        resultado := valresist;
        resultado[1] := resultado[1] + valslot[1];

    elseif(item.IsA(POLCLASS_ARMOR)) //ARMADURA TAMBÉM
        valmaterial := TabelaMaterialMod(item);

        resultado := valmaterial;

        resultado[1] := resultado[1] + valresist + valslot;
        if (resultado[2] > 0) resultado[2] := resultado[2] + valresist + valslot; endif
        if (resultado[3] > 0) resultado[3] := resultado[3] + valresist + valslot; endif
        if (resultado[4] > 0) resultado[4] := resultado[4] + valresist + valslot; endif
        if (resultado[5] > 0) resultado[5] := resultado[5] + valresist + valslot; endif

    elseif(item.IsA(POLCLASS_WEAPON))
        resultado := TabelaDanos(item);

    endif

    return resultado;
endfunction

function BuscarDebuffMod(item)
    var valores := Array;
    var valdebuff := Array;
    var valslot := Array;

    valdebuff := TabelaTipoProtecaoMod(item);
    valslot := TabelaSlot(item);

    valores := valdebuff;
    valores[1] := 0; //PARA NÃO USAR - APENAS PARA MANTER A ORDEM DOS OUTROS;
    valores[2] := valores[2] + valslot[2];
    valores[3] := valores[3] + valslot[3];
    valores[4] := valores[4] + valslot[4];

    return valores;
endfunction

function IsEscudo(item)
    var tipoequip := PegarTipo(item);
    var escudo := SplitWords(Lower(tipoequip)); escudo := escudo[1];

    if (escudo == "escudo") return 1;
    else return;
    endif
endfunction



//FUNCTIONS PARA MANIPULAR PROPS
function PegarTipo(item) //PEGA A PROP DO TIPO DO EQUIPAMENTO
    var tipo := PegarProp(item);
    tipo := tipo."TipoEquip";
    return tipo;
endfunction

function PegarSlot(item) //PEGA O SLOT DO ITEM NO .CFG
    var elem := BuscarValoresCfg(item);
    var slot := Lower(elem.Coverage);
    if (IsEscudo(item)) slot := "Shield"; endif
    return slot;
endfunction

function PegarProp(item) //PEGA A PROP DE MODIFICADORES BASE
    var prop := item.GetProp("atributos");
    if (!prop) prop := Dictionary{}; endif
    return prop;
endfunction

function PegarPropExcep(item) //PEGA A PROP DE MODIFICADORES EXCEPCIONAL
    var prop := item.GetProp("atributos_excp");
    if (!prop) prop := Dictionary{}; endif
    return prop;
endfunction

function SetarProp(item, prop) //SETA A PROP DE MODIFICADORES BASE
    if (prop.Size() != 0) item.SetProp("atributos", prop); endif
endfunction

function SetarPropExcep(item, prop) //SETA A PROP DE MODIFICADORES EXCEPCIONAL
    if (prop.Size() != 0) item.SetProp("atributos_excp", prop); endif
endfunction



//FUNCTIONS DE CÁLCULO
function CalcPropBase(item, mod, slot) //CALCULA O VALOR REAL PARA DEFINIR NO ITEM
    var valor, newattrescudo, newattrdebuff;
    var attr := PegarProp(item);
    var attr_excp := PegarPropExcep(item);
    var newattr := BuscarValorMaterial(item);
    mod.Lower(); slot.Lower();

    case (mod) //COMUM A TODOS
        "quality": valor := attr["Quality"]; break;

        "hpmod": valor := ((newattr[6]) + CInt(attr["hab_hpmod"]) + CInt(attr_excp["hpmod"])); break;
    endcase
    if (slot == "shield")
        newattrdebuff := BuscarDebuffMod(item);

        case (mod)
            "dexpenmod":        valor := (newattrdebuff[2] + CInt(attr["item_dexpen"]) + CInt(attr."hab_dexpenmod") + CInt(attr_excp."dexpenmod")); break;
            "parrymod":         valor := (newattrdebuff[3] + CInt(attr["item_parry"]) + CInt(attr."hab_parrymod") + CInt(attr_excp."parrymod")); break;
            "parrydelaymod":    valor := (newattrdebuff[4] + CInt(attr["item_parrydelay"]) + CInt(attr."hab_parrydelaymod") + CInt(attr_excp."parrydelaymod")); break;

            "physicalmod":      valor := (newattr[1] + CInt(attr["item_physical"]) + CInt(attr."hab_physicalmod") + CInt(attr_excp."physicalmod"))-(item.resist_physical-item.resist_physical_mod); break;
            "firemod":          valor := (newattr[2] + CInt(attr["item_fire"]) + CInt(attr."hab_firemod") + CInt(attr_excp."firemod"))-(item.resist_fire-item.resist_fire_mod); break;
            "coldmod":          valor := (newattr[3] + CInt(attr["item_cold"]) + CInt(attr."hab_coldmod") + CInt(attr_excp."coldmod"))-(item.resist_cold-item.resist_cold_mod); break;
            "poisonmod":        valor := (newattr[4] + CInt(attr["item_poison"]) + CInt(attr."hab_poisonmod") + CInt(attr_excp."poisonmod"))-(item.resist_poison-item.resist_poison_mod); break;
            "energymod":        valor := (newattr[5] + CInt(attr["item_energy"]) + CInt(attr."hab_energymod") + CInt(attr_excp."energymod"))-(item.resist_energy-item.resist_energy_mod); break;
        
            default:            valor := 0;
        endcase
    elseif (slot == "armor")
        newattrdebuff := BuscarDebuffMod(item);

        case (mod)
            "dexpenmod":        valor := (newattrdebuff[2] + CInt(attr["item_dexpen"]) + CInt(attr."hab_dexpenmod") + CInt(attr_excp."dexpenmod")); break;

            "physicalmod":      valor := (newattr[1] + CInt(attr["item_physical"]) + CInt(attr."hab_physicalmod") + CInt(attr_excp."physicalmod"))-(item.resist_physical-item.resist_physical_mod); break;
            "firemod":          valor := (newattr[2] + CInt(attr["item_fire"]) + CInt(attr."hab_firemod") + CInt(attr_excp."firemod"))-(item.resist_fire-item.resist_fire_mod); break;
            "coldmod":          valor := (newattr[3] + CInt(attr["item_cold"]) + CInt(attr."hab_coldmod") + CInt(attr_excp."coldmod"))-(item.resist_cold-item.resist_cold_mod); break;
            "poisonmod":        valor := (newattr[4] + CInt(attr["item_poison"]) + CInt(attr."hab_poisonmod") + CInt(attr_excp."poisonmod"))-(item.resist_poison-item.resist_poison_mod); break;
            "energymod":        valor := (newattr[5] + CInt(attr["item_energy"]) + CInt(attr."hab_energymod") + CInt(attr_excp."energymod"))-(item.resist_energy-item.resist_energy_mod); break;
        
            default:            valor := 0;
        endcase

    elseif (slot == "weapon")
        
        case (mod)
            "dmgmod":           valor := (CInt(attr."hab_dmgmod") + CInt(attr_excp."dmgmod")); break;
            "speedmod":         valor := (CInt(attr["item_speed"]) + CInt(attr["hab_speedmod"]) + CInt(attr_excp["speedmod"])); break;

            "physicalmod":      valor := (newattr[1] + CInt(attr["item_physical"]) + CInt(attr."hab_physicalmod") + CInt(attr_excp."physicalmod"))-(item.damage_physical-item.damage_physical_mod); break;
            "firemod":          valor := (newattr[2] + CInt(attr["item_fire"]) + CInt(attr."hab_firemod") + CInt(attr_excp."firemod"))-(item.damage_fire-item.damage_fire_mod); break;
            "coldmod":          valor := (newattr[3] + CInt(attr["item_cold"]) + CInt(attr."hab_coldmod") + CInt(attr_excp."coldmod"))-(item.damage_cold-item.damage_cold_mod); break;
            "poisonmod":        valor := (newattr[4] + CInt(attr["item_poison"]) + CInt(attr."hab_poisonmod") + CInt(attr_excp."poisonmod"))-(item.damage_poison-item.damage_poison_mod); break;
            "energymod":        valor := (newattr[5] + CInt(attr["item_energy"]) + CInt(attr."hab_energymod") + CInt(attr_excp."energymod"))-(item.damage_energy-item.damage_energy_mod); break;
        
            default:            valor := 0;
        endcase
    endif

    return valor;
endfunction





//APENAS PARA ATUALIZAR ITENS ANTES DA IMPLEMENTAÇÃO
function AtualizarItemAntigo(who, item)
    var tipoequip;
    var propsenc := "";
    var attr := PegarProp(item);
    var material := BuscarValorMaterial(item);
    var propexcpantiga := item.GetProp("Exceptional");

    if (attr["Atualiza"] != 0 && attr["Atualiza"] != 1)
        SendSysMessageEx(who, "Item {} com base de dados antiga, atualizando para novo modelo...".Format(Hex(item.objtype)), SSM_FAIL);
        SendSysMessageEx(who, "----------");

        SetarPropTipo(item);
        SetarPropItem(item);
        attr := PegarProp(item);
        tipoequip := PegarTipo(item);

        attr["Quality"] := item.quality; SetarProp(item, attr);

        if (propexcpantiga == 1)
            SendSysMessageEx(who, "Item EXCEPCIONAL detectado! Adicionando valores...", FONTCOLOR_YELLOW);
            var attr_excp := PegarPropExcep(item);

            attr_excp["hpmod"]  := item.maxhp_mod; //REVER A FORMULA DISSO AQUI


            if(item.IsA(POLCLASS_ARMOR))
                attr_excp["physicalmod"]    := CInt(item.resist_physical_mod - CInt(material[1]));
                attr_excp["firemod"]        := CInt(item.resist_fire_mod - CInt(material[2]));
                attr_excp["coldmod"]        := CInt(item.resist_cold_mod - CInt(material[3]));
                attr_excp["poisonmod"]      := CInt(item.resist_poison_mod - CInt(material[4]));
                attr_excp["energymod"]      := CInt(item.resist_energy_mod - CInt(material[5]));

                if (IsEscudo(item))
                    attr_excp["dexpenmod"]      := CInt(item.GetProp("penaltybonus"));
                    attr_excp["parrymod"]       := CInt(item.GetProp("parrybonus"));
                    attr_excp["parrydelaymod"]  := CInt(item.GetProp("parrydelay_mod"));
                endif

            elseif (item.IsA(POLCLASS_WEAPON))
                attr_excp["dmgmod"]         := item.dmg_mod; //REVER A FORMULA DISSO AQUI
                attr_excp["speedmod"]       := CInt(item.GetProp("attackspeedbonus"));

                attr_excp["physicalmod"]    := CInt(item.damage_physical_mod - CInt(material[1]));
                attr_excp["firemod"]        := CInt(item.damage_fire_mod - CInt(material[2]));
                attr_excp["coldmod"]        := CInt(item.damage_cold_mod - CInt(material[3]));
                attr_excp["poisonmod"]      := CInt(item.damage_poison_mod - CInt(material[4]));
                attr_excp["energymod"]      := CInt(item.damage_energy_mod - CInt(material[5]));

            endif
            SendSysMessageEx(who, "Propriedades EXCEPCIONAIS encontradas: "+attr_excp);
            SetarPropExcep(item, attr_excp);

        else
            attr := PegarProp(item);
            var valor, itemhab;

            valor := CInt(attr["item_hp"]) - (item.maxhp-item.maxhp_mod); if(valor > 0) attr["hab_hpmod"] := valor; itemhab := 1; propsenc := propsenc+"hpmod, "; endif

            if (IsEscudo(item))
                var valor;
                valor := CInt(attr["item_physical"]) - (item.resist_physical-item.resist_physical_mod);     if(valor > 0) attr["hab_physicalmod"] := valor; itemhab := 1; propsenc := propsenc+"physicalmod, "; endif
                valor := CInt(attr["item_fire"]) - (item.resist_fire-item.resist_fire_mod);                 if(valor > 0) attr["hab_firemod"] := valor; itemhab := 1; propsenc := propsenc+"firemod, "; endif
                valor := CInt(attr["item_cold"]) - (item.resist_cold-item.resist_cold_mod);                 if(valor > 0) attr["hab_coldmod"] := valor; itemhab := 1; propsenc := propsenc+"coldmod, "; endif
                valor := CInt(attr["item_poison"]) - (item.resist_poison-item.resist_poison_mod);           if(valor > 0) attr["hab_poisonmod"] := valor; itemhab := 1; propsenc := propsenc+"poisonmod, "; endif
                valor := CInt(attr["item_energy"]) - (item.resist_energy-item.resist_energy_mod);           if(valor > 0) attr["hab_energymod"] := valor; itemhab := 1; propsenc := propsenc+"energymod, "; endif
                valor := CInt(item.GetProp("penaltybonus"));                                                if(valor > 0) attr["hab_dexpenmod"] := valor; itemhab := 1; propsenc := propsenc+"dexpenmod, "; endif
                valor := CInt(item.GetProp("parrybonus"));                                                  if(valor > 0) attr["hab_parrymod"] := valor; itemhab := 1; propsenc := propsenc+"parrymod, "; endif
                valor := CInt(item.GetProp("parrydelay_mod"));                                              if(valor > 0) attr["hab_parrydelaymod"] := valor; itemhab := 1; propsenc := propsenc+"parrydelaymod, "; endif

            elseif(item.IsA(POLCLASS_ARMOR))
                //DEXPEN

                valor := CInt(item.resist_physical_mod - material[1]);                                      if(valor > 0) attr["hab_physicalmod"] := valor; itemhab := 1; propsenc := propsenc+"physicalmod, "; endif
                valor := CInt(attr["item_fire"]) - (item.resist_fire-item.resist_fire_mod);                 if(valor > 0) attr["hab_firemod"] := valor; itemhab := 1; propsenc := propsenc+"firemod, "; endif
                valor := CInt(attr["item_cold"]) - (item.resist_cold-item.resist_cold_mod);                 if(valor > 0) attr["hab_coldmod"] := valor; itemhab := 1; propsenc := propsenc+"coldmod, "; endif
                valor := CInt(attr["item_poison"]) - (item.resist_poison-item.resist_poison_mod);           if(valor > 0) attr["hab_poisonmod"] := valor; itemhab := 1; propsenc := propsenc+"poisonmod, "; endif
                valor := CInt(attr["item_energy"]) - (item.resist_energy-item.resist_energy_mod);           if(valor > 0) attr["hab_energymod"] := valor; itemhab := 1; propsenc := propsenc+"energymod, "; endif

            elseif (item.IsA(POLCLASS_WEAPON))
                valor := item.dmg_mod;                                                                      if(valor > 0) attr["hab_dmgmod"] := valor; itemhab := 1; propsenc := propsenc+"dmgmod, "; endif //CONFERIR FORMULA
                valor := CInt(item.GetProp("attackspeedbonus"));                                            if(valor > 0) attr["hab_speedmod"] := valor; itemhab := 1; propsenc := propsenc+"physicalmod, "; endif

                valor := CInt(attr["item_physical"]) - (item.damage_physical-item.damage_physical_mod);     if(valor > 0) attr["hab_physicalmod"] := valor; itemhab := 1; propsenc := propsenc+"physicalmod, "; endif
                valor := CInt(attr["item_fire"]) - (item.damage_fire-item.damage_fire_mod);                 if(valor > 0) attr["hab_firemod"] := valor; itemhab := 1; propsenc := propsenc+"firemod, "; endif
                valor := CInt(attr["item_cold"]) - (item.damage_cold-item.damage_cold_mod);                 if(valor > 0) attr["hab_coldmod"] := valor; itemhab := 1; propsenc := propsenc+"coldmod, "; endif
                valor := CInt(attr["item_poison"]) - (item.damage_poison-item.damage_poison_mod);           if(valor > 0) attr["hab_poisonmod"] := valor; itemhab := 1; propsenc := propsenc+"poisonmod, "; endif
                valor := CInt(attr["item_energy"]) - (item.damage_energy-item.damage_energy_mod);           if(valor > 0) attr["hab_energymod"] := valor; itemhab := 1; propsenc := propsenc+"energymod, "; endif

            endif
            if (itemhab)
                SendSysMessageEx(who, "Item com propriedade de HABILIDADE detectado! Adicionando valores...", FONTCOLOR_YELLOW);
                SendSysMessageEx(who, "Propriedades encontradas: "+attr);
            else SendSysMessageEx(who, "Item SEM PROPRIEDADE de Habilidade ou Excepcional! Adicionando valores...", FONTCOLOR_YELLOW);
            endif
        endif

        SetarProp(item, attr);
        //if (item.hp<item.maxhp) item.hp := item.hp+item.maxhp_mod; endif //COMPENSAR O HP CORRIGIDO

        SendSysMessageEx(who, "----------");
        SendSysMessageEx(who, "Item {} atualizado para novo modelo de base de dados com SUCESSO !!!".Format(Hex(item.objtype)), SSM_INFO);
    endif
endfunction