use os;
use cfgfile;

function ReadResourceModifiers(mod)
    var cfg_file := ReadConfigFile(":crafting:config/resourceList");
    if (cfg_file.errortext)
        Syslog("Erro ao ler resourceList.cfg: " + cfg_file.errortext);
        return 0;
    endif

    var resource_modifiers := dictionary;
    foreach resource_id in GetConfigStringKeys(cfg_file)
        var elem := FindConfigElem(cfg_file, resource_id);
        if (elem)
            var aos_resist_mod;
            if(mod == "resist")
                aos_resist_mod := GetConfigStringArray(elem, "AoSResistMod");
            elseif(mod == "damage")
                aos_resist_mod := GetConfigStringArray(elem, "AoSDamageMod");
            endif

            var hp_mod := CInt(GetConfigString(elem, "hpmod")); // TODO: Remover daqui e substituir por uam função que calcule no item, baseado em um fórmula matemática envolvendo tier, peso, etc.
            if (aos_resist_mod.size() >= 5)
                resource_modifiers[Lower(resource_id)] := struct{
                    "Physical" := CInt(aos_resist_mod[1] * 100),
                    "Fire" := CInt(aos_resist_mod[2] * 100),
                    "Cold" := CInt(aos_resist_mod[3] * 100),
                    "Poison" := CInt(aos_resist_mod[4] * 100),
                    "Energy" := CInt(aos_resist_mod[5] * 100),
                    "HP" := hp_mod
                };
            endif
        endif
    endforeach

    return resource_modifiers;
endfunction

// Função refatorada TabelaMaterialResistMod
function TabelaMaterialResistMod(item)
    var material := Lower(Hex(item.GetProp("material")));
    var resource_modifiers := ReadResourceModifiers("resist");

    if (material in resource_modifiers)
        return resource_modifiers[material];
    endif

    // Valor padrão caso o material não seja encontrado
    return array{0, 0, 0, 0, 0, 0};
endfunction

function TabelaMaterialDamageMod(item) //objtype do material: return {FISICO, FIRE, COLD, POISON, ENERGY, HP}
    var material := Lower(Hex(item.GetProp("material")));
    var resource_modifiers := ReadResourceModifiers("damage");

    if (material in resource_modifiers)
        return resource_modifiers[material];
    endif

    // Valor padrão caso o material não seja encontrado
    return array{0, 0, 0, 0, 0, 0};
endfunction

function getMaterialMod(item) //objtype do material: return {FISICO, FIRE, COLD, POISON, ENERGY}
    var material := Hex(item.GetProp("material"));
    case(material)
        //////////METAL
        //TIER 1
        0x6309: //FERRO
        0x6993: //PRATA LUNAR
        0x6991: //ESTANHO
        0x6465: //COBRE
        0x6995: //AÇO
        0x6994: //BRONZE
        0x630e: //OURO

        //TIER 2
        0x630a: //LITAN
        0x6315: //ILLYRA
        0x630d: //FIGIN
        0x6460: //FRIGUTIUM
        0x630c: //SINQUE
        0x630b: //HELCE
        0x6602: //ACIELITH
        0x6603: //CRISTALITH
        0x6601: //STALITH

        //TIER 3
        0x6310: //GALVORN
        0x6605: //IGNARILITH
        0x6606: //GLACIALITH
        0x6607: //VENERILITH
        0x6608: //ENERLITH
        0x6604: //PLATINUM
        0x6609: //LUNIUM
        0x6610: //CELERIUM
        
        //TIER 4
        0x6997: //SOLIUM
        0x699a: //ICELUM
        0x6999: //HISSIUM
        0x6998: //TESLIUM
        0x6311: //MITHRIL

        //TIER 5
        0x699b: //ORICALCUM
            return {0,0,0,0,0}; //MOD METAIS

        //////////COURO
        //TIER 1
        0xee45: //CURTIDO
        0xef24: //PELUDO

        //TIER 2
        0xef21: //ESPESSO
        0xef26: //POLAR
        0xef20: //ESCAMOSO
        0xef22: //ANFIBIO

        //TIER 3
        0xc601: //courorobusto
        0xc602: //MARINHO
        0xc603: //couroescamosoduro
        0xc604: //LAMELAR

        //TIER 4
        0xed29: //DRACONIANO
        0xef25: //courocinzento
        0xef28: //courorijo
        0xef27: //GIGANTE

        //TIER 5
        0xef23: //ASPERO
        0xc605: //NEGRO
            return {0,0,0,0,0}; //MOD COUROS

        //////////OSSO
        //TIER 1
        0xf7e: //OSSO
        0xef32: //CINZA

        //TIER 2
        0xef30: //VERMELHO
        0xef31: //PRETO

        //TIER 3
        0xef38: //LICH
        0xef39: //REPTILIANO

        //TIER 4
        0xef3a: //GIGANTE
        0xef3b: //TERATHAN

        //TIER 5
        0xef29: //DEMONÍACO
            return {0,0,0,0,0}; //MOD OSSOS

        //////////TECIDO
        //TIER 1
        0x1765: //LÃ
        0x1764: //ALGODÃO

        //TIER 2
        0xf92: //SEDA
        0xf9f: //LINHO

        //TIER 3
        0xfa2: //MALVA
        0xfad: //SISAL
        0xfaf: //DENIM
        0xfb2: //RAMI

        //TIER 4
        0xfb7: //FLAMEL
        0xfbd: //RAJAL
            return {0,0,0,0,0}; //MOD TECIDOS

        //////////TECIDO
        //TIER 1

        /*
        //"QUITINA": //QUITINA - BUSCAR OBJTYPE
            return {0,0,0,0,0};

        //"MADEIRA": //MADEIRA - BUSCAR OBJTYPE
            return {0,0,0,0,0};
        */

		default: return {0,0,0,0,0};
    endcase
endfunction