///////////////////////////////////////////////////////////////////////////////
///// Criado para substituir algumas funções do .cfg, que são limitadoras /////
///////////////////////////////////////////////////////////////////////////////

//TODO: Refazer muitas das funções que usam array usando dictionary/struct
	//TODO: Tem muita coisa cagada que na época eu fiz de forma porca (AHUahuAHUa)

/* 
 * Purpose
 * Ler o .cfg e extrair os modificadores
 */

use os;
use cfgfile;

include "propitens";

enum SPEED
	MELEE_LOW, MELEE_MID, MELEE_HIGH,
	RANGED_LOW, RANGED_MID, RANGED_HIGH
endenum
enum DAMAGE
	ONEHANDED_LOW, ONEHANDED_MID, ONEHANDED_HIGH,
	TWOHANDED_LOW, TWOHANDED_MID, TWOHANDED_HIGH,
	BOW_LOW, BOW_MID, BOW_HIGH,
	CROSSBOW_LOW, CROSSBOW_MID, CROSSBOW_HIGH
endenum
enum CRITICAL
	CRITICALCHANCE_NOCHANCE,
	CRITICALCHANCE_LOW,
	CRITICALCHANCE_MID,
	CRITICALCHANCE_HIGH
endenum

/**
 * getWeaponStats(tier, damageTier, speedTier, criticalTier)
 *
 * Purpose
 * Passar todos os valores da arma em um único struct.
 *
 * Parameters
 * tier: O Tier geral da arma (o Tier no .cfg)
 * damageTier: O Damage Tier da arma (o Damage no .cfg)
 * speedTier: O Speed Tier da arma (o Speed no .cfg)
 * criticalTier: O Critical Tier da arma (o CriticalChance no .cfg)
 *
 * Return struct
 *
 */
function getWeaponStats(tier, damageTier, speedTier, criticalTier)
	// TODO: Sistema de tier para somar os ganhos pra cada tier

	return struct{
		"Damage" := getWeaponDamage(damageTier),
		"Speed" := getWeaponSpeed(speedTier),
		"CriticalChance" := getWeaponCriticalChance(criticalTier)
	};
endfunction

/**
 * getWeaponSpeed(tier)
 *
 * Purpose
 * Passar os valores máximos da arma baseado em seu Grupo de Velocidade.
 *
 * Parameters
 * tier: Grupo de Velocidade da arma (o Speed no .cfg)
 *
 * Return struct
 *
 */
function getWeaponSpeed(tier)
	case (tier)
		MELEE_LOW: return struct{"min" := 25, "max" := 29};
		MELEE_MID: return struct{"min" := 33, "max" := 37};
		MELEE_HIGH: return struct{"min" := 39, "max" := 45};

		RANGED_LOW: return struct{"min" := 25, "max" := 29};
		RANGED_MID: return struct{"min" := 33, "max" := 37};
		RANGED_HIGH: return struct{"min" := 39, "max" := 45};
		
		default: return error{ "errortext" := "Damage value not found for this Weapon Speed Tier!"};
	endcase
endfunction

/**
 * getWeaponDamage(tier)
 *
 * Purpose
 * Verificar o valor do dado do dano da arma baseado no seu Grupo de Dano.
 *
 * Parameters
 * tier: Grupo de dano da arma (o Damage no .cfg)
 *
 * Return dado
 *
 */
function getWeaponDamage(tier)
	case (tier)
		// ONE HANDED
		ONEHANDED_LOW: return "1d4";
		ONEHANDED_MID: return "1d6";
		ONEHANDED_HIGH: return "1d8";
			
		// TWO HANDED
		TWOHANDED_LOW: return "1d8";
		TWOHANDED_MID: return "1d10";
		TWOHANDED_HIGH: return "1d12";

		// BOW
		BOW_LOW: return "";
		BOW_MID: return "";
		BOW_HIGH: return "";

		// CROSSBOW
		CROSSBOW_LOW: return "";
		CROSSBOW_MID: return "";
		CROSSBOW_HIGH: return "";
			
		default: return error{ "errortext" := "Damage value not found for this Weapon Damage Tier!"};
	endcase
endfunction

/**
 * getWeaponCriticalChance(tier)
 *
 * Purpose
 *
 * Parameters
 * tier:
 *
 * Return struct
 *
 */
function getWeaponCriticalChance(tier)
	var min, max;

	case(tier)
		CRITICALCHANCE_NOCHANCE: struct{"min" := 0, "max" := 0};
		CRITICALCHANCE_LOW: struct{"min" := 1, "max" := 2};
		CRITICALCHANCE_MID: struct{"min" := 3, "max" := 4};
		CRITICALCHANCE_HIGH: struct{"min" := 5, "max" := 6};
		
		default: return error{ "errortext" := "Damage value not found for this Weapon Critical Chance Tier!"};
	endcase
endfunction














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

            var hp_mod := CInt(GetConfigString(elem, "hpmod"));
            if (aos_resist_mod.size() >= 5)
                resource_modifiers[Lower(resource_id)] := array{
                    CInt(aos_resist_mod[1] * 100),
                    CInt(aos_resist_mod[2] * 100),
                    CInt(aos_resist_mod[3] * 100),
                    CInt(aos_resist_mod[4] * 100),
                    CInt(aos_resist_mod[5] * 100),
                    hp_mod
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

function TabelaMaterialMod(item) //objtype do material: return {FISICO, FIRE, COLD, POISON, ENERGY}
	// TODO: Refazer essa porqueira direito.... Como que porra eu fiz isso desse jeito na época?! PUTA MERDA, HEIN.....
    var material := Lower(Hex(item.GetProp("material")));
    case(material)
        //////////METAL
        //TIER 1
        "0x6309": //FERRO
        "0x6993": //PRATA LUNAR
        "0x6991": //ESTANHO
        "0x6465": //COBRE
        "0x6995": //AÇO
        "0x6994": //BRONZE
        "0x630e": //OURO

        //TIER 2
        "0x630a": //LITAN
        "0x6315": // ILLYRA
        "0x630d": //FIGIN
        "0x6460": //frigutium
        "0x630c": //SINQUE
        "0x630b": //HELCE
        "0x6602": //ACIELITH
        "0x6603": //CRISTALITH
        "0x6601": //STALITH

        //TIER 3
        "0x6310": //GALVORN
        "0x6605": //IGNARILITH
        "0x6606": //GLACIALITH
        "0x6607": //VENERILITH
        "0x6608": //ENERLITH
        "0x6604": //PLATINUM
        "0x6609": //LUNIUM
        "0x6610": //CELERIUM
        
        //TIER 4
        "0x6997": //SOLIUM
        "0x699a": //ICELUM
        "0x6999": //HISSIUM
        "0x6998": //TESLIUM
        "0x6311": //MITHRIL

        //TIER 5
        "0x699b": //ORICALCUM
            return {0,0,0,0,0}; //MOD METAIS

        //////////COURO
        //TIER 1
        "0xee45": //CURTIDO
        "0xef24": //PELUDO

        //TIER 2
        "0xef21": //ESPESSO
        "0xef26": //POLAR
        "0xef20": //ESCAMOSO
        "0xef22": //ANFIBIO

        //TIER 3
        "0xc601": //courorobusto
        "0xc602": //MARINHO
        "0xc603": //couroescamosoduro
        "0xc604": //LAMELAR

        //TIER 4
        "0xed29": //DRACONIANO
        "0xef25": //courocinzento
        "0xef28": //courorijo
        "0xef27": //GIGANTE

        //TIER 5
        "0xef23": //ASPERO
        "0xc605": //NEGRO
            return {0,0,0,0,0}; //MOD COUROS

        //////////OSSO
        //TIER 1
        "0xf7e": //OSSO
        "0xef32": //CINZA

        //TIER 2
        "0xef30": //VERMELHO
        "0xef31": //PRETO

        //TIER 3
        "0xef38": //LICH
        "0xef39": //REPTILIANO

        //TIER 4
        "0xef3a": //GIGANTE
        "0xef3b": //TERATHAN

        //TIER 5
        "0xef29": //DEMONÍACO
            return {0,0,0,0,0}; //MOD OSSOS

        //////////TECIDO
        //TIER 1
        "0x1765": //LÃ
        "0x1764": //ALGODÃO

        //TIER 2
        "0xf92": //SEDA
        "0xf9f": //LINHO

        //TIER 3
        "0xfa2": //MALVA
        "0xfad": //SISAL
        "0xfaf": //DENIM
        "0xfb2": //RAMI

        //TIER 4
        "0xfb7": //FLAMEL
        "0xfbd": //RAJAL
            return {0,0,0,0,0}; //MOD TECIDOS

        //////////TECIDO
        //TIER 1

        /*
        //"QUITINA": //QUITINA - BUSCAR OBJTYPE
            return {0,0,0,0,0};

        //"MADEIRA": //MADEIRA - BUSCAR OBJTYPE
            return {0,0,0,0,0};
        */
    endcase
endfunction

function TabelaDanos(item)
	// TODO: Refazer essa porqueira direito.... Como que porra eu fiz isso desse jeito na época?! PUTA MERDA, HEIN.....
		// TODO: Usar um struct ou dictionary, ao invés de array...
    var cfg := ReadConfigFile(":combat:itemdesc");
    var elem := FindConfigElem(cfg, item.objtype);
    var dano := Array;
    dano.append(elem.Damage);
    dano.append(elem.PhysicalDamage);
    dano.append(elem.FireDamage);
    dano.append(elem.ColdDamage);
    dano.append(elem.PoisonDamage);
    dano.append(elem.EnergyDamage);

    return dano;
endfunction

function TabelaTipoProtecaoMod(item) // TipoProtecao -=> return {RESIST, DEXPENALTY, PARRYBONUS, PARRYDELAY}
	// TODO: Refazer essa porqueira direito.... Como que porra eu fiz isso desse jeito na época?! PUTA MERDA, HEIN.....
		// TODO: Usar um struct ou dictionary, ao invés de array...
    var tipo := Lower(PegarTipo(item));
    case(tipo)
        //TIER 1
        "armadura leve fraca": return {0,0,0,0};
        "armadura leve media": return {1,0,0,0};
        "armadura leve forte": return {2,0,0,0};

        //TIER 2
        "armadura media fraca": return {3,0,0,0};
        "armadura media media": return {4,0,0,0};
        "armadura media forte": return {5,0,0,0};

        //TIER 3
        "armadura pesada fraca": return {6,0,0,0};
        "armadura pesada media": return {7,0,0,0};
        "armadura pesada forte": return {8,0,0,0};
    endcase

    case(tipo)
        //TIER 1
        "escudo leve fraco": return {0,0,0,0};
        "escudo leve medio": return {1,0,0,0};
        "escudo leve forte": return {2,0,0,0};

        //TIER 2
        "escudo medio fraco": return {2,0,0,0};
        "escudo medio medio": return {3,0,0,0};
        "escudo medio forte": return {5,0,0,0};

        //TIER 3
        "escudo pesado fraco": return {6,0,0,0};
        "escudo pesado medio": return {7,0,0,0};
        "escudo pesado forte": return {8,0,0,0};

        //TIER 4
        "escudo de corpo fraco": return {9,0,0,0};
        "escudo de corpo medio": return {10,0,0,0};
        "escudo de corpo forte": return {11,0,0,0};
    endcase
endfunction

function TabelaSlot(item)
	// TODO: Refazer essa porqueira direito.... Como que porra eu fiz isso desse jeito na época?! PUTA MERDA, HEIN.....
		// TODO: Usar um struct ou dictionary, ao invés de array...
    var slot := Lower(PegarSlot(item));

    case(slot) //[1] = ResistBase; [2] = DexPenalty
        "head": return {1,-1};
        "neck": return {1,-1};
        "body": return {4,-3};
        "arms": return {2,-2};
        "hands": return {1,-1};
        "legs/feet": return {3,-2};
        "shield": return {2,-1};
    endcase
endfunction
