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
include ":combat:combat";

enum SPEED
	MELEE_LOW := "MELEE_LOW", MELEE_MID := "MELEE_MID", MELEE_HIGH := "MELEE_HIGH",
	RANGED_LOW := "RANGED_LOW", RANGED_MID := "RANGED_MID", RANGED_HIGH := "RANGED_HIGH"
endenum
enum DAMAGE
	ONEHANDED_LOW := "ONEHANDED_LOW", ONEHANDED_MID := "ONEHANDED_MID", ONEHANDED_HIGH := "ONEHANDED_HIGH",
	TWOHANDED_LOW := "TWOHANDED_LOW", TWOHANDED_MID := "TWOHANDED_MID", TWOHANDED_HIGH := "TWOHANDED_HIGH",
	BOW_LOW := "BOW_LOW", BOW_MID := "BOW_MID", BOW_HIGH := "BOW_HIGH",
	CROSSBOW_LOW := "CROSSBOW_LOW", CROSSBOW_MID := "CROSSBOW_MID", CROSSBOW_HIGH := "CROSSBOW_HIGH"
endenum
enum DAMAGETYPE
	SLASHDAMAGE := "CORTANTE", PIERCINGDAMAGE := "PERFURANTE", BLUNTDAMAGE := "CONTUSIVO"
endenum
enum CRITICAL
	CRITICALCHANCE_NOCHANCE := "NOCHANCE",
	CRITICALCHANCE_LOW := "LOW",
	CRITICALCHANCE_MID := "MID",
	CRITICALCHANCE_HIGH := "HIGH"
endenum
enum ARMORSLOT
	HEAD := "HEAD", NECK := "NECK", BODY := "BODY", ARMS := "ARMS", HANDS := "HANDS", LEGS := "LEGS/FEET", SHIELD := "SHIELD"
endenum
enum PROTECTIONTYPE
	ARMOR_LOWLOW := "ARMADURA LEVE FRACA", ARMOR_LOWMID := "ARMADURA LEVE MEDIA", ARMOR_LOWHIGH := "ARMADURA LEVE FORTE",
	ARMOR_MIDLOW := "ARMADURA MEDIA FRACA", ARMOR_MIDMID := "ARMADURA MEDIA MEDIA", ARMOR_MIDHIGH := "ARMADURA MEDIA FORTE",
	ARMOR_HIGHLOW := "ARMADURA PESADA FRACA", ARMOR_HIGHMID := "ARMADURA PESADA MEDIA", ARMOR_HIGHHIGH := "ARMADURA PESADA FORTE",
	SHIELD_LOWLOW := "ESCUDO LEVE FRACA", SHIELD_LOWMID := "ESCUDO LEVE MEDIA", SHIELD_LOWHIGH := "ESCUDO LEVE FORTE",
	SHIELD_MIDLOW := "ESCUDO MEDIA FRACA", SHIELD_MIDMID := "ESCUDO MEDIA MEDIA", SHIELD_MIDHIGH := "ESCUDO MEDIA FORTE",
	SHIELD_HIGHLOW := "ESCUDO PESADA FRACA", SHIELD_HIGHMID := "ESCUDO PESADA MEDIA", SHIELD_HIGHHIGH := "ESCUDO PESADA FORTE",
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
function getWeaponStats(weapon)
	// TODO: Sistema de tier para somar os ganhos pra cada tier
	var cfg := ReadConfigFile(":combat:itemdesc");
	var elem := cfg[weapon.objtype];
	var stats := struct;

	case (elem.TipoAtaque)
		// TODO: Ver se o const não vai atrapalhar o + do struct
		// TODO: Pensar se todas as armas terão todos os atributos com penaliade em alguns, ou se só 1 stats.Damage passado aqui
		SLASHDAMAGE: stats.+SlashDamage := getDamageByTier(elem.Damage); break;
		PIERCINGDAMAGE: stats.+PiercingDamage := getDamageByTier(elem.Damage); break;
		BLUNTDAMAGE: stats.+BluntDamage := getDamageByTier(elem.Damage); break;
	endcase
	stats.+AttackType := elem.TipoAtaque;
	stats.+AttackSpeed := getSpeedByTier(elem.Speed);
	stats.+CriticalChange := getCriticalChanceByTier(elem.CriticalChance);

	return stats;
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
function getSpeedByTier(tier)
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
function getDamageByTier(tier)
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
function getCriticalChanceByTier(tier)
	var min, max;

	case(tier)
		CRITICALCHANCE_NOCHANCE: return struct{"min" := 0, "max" := 0};
		CRITICALCHANCE_LOW: return struct{"min" := 1, "max" := 2};
		CRITICALCHANCE_MID: return struct{"min" := 3, "max" := 4};
		CRITICALCHANCE_HIGH: return struct{"min" := 5, "max" := 6};
		
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

function getArmorStats(armor)
	var cfg := ReadConfigFile(":combat:itemdesc");
	var elem := cfg[armor.objtype];
	var stats := struct;

	// TODO: Pegar todos os atributos da armadura e retornar no stats

	return stats;
endfunction

function getResistModByTier(item)
	var physical := array; // {SlashResist, PiercingResist, BluntResist};
    var tipo := PegarTipo(item).upper();

    case(tipo)
        //TIER 1
        ARMOR_LOWLOW: physical := {0,0,0};
        ARMOR_LOWMID: physical := {1,1,1};
        ARMOR_LOWHIGH: physical := {2,2,2};

        //TIER 2
        ARMOR_MIDLOW: physical := {3,3,3};
        ARMOR_MIDMID: physical := {4,4,4};
        ARMOR_MIDHIGH: physical := {5,5,5};

        //TIER 3
        ARMOR_HIGHLOW: physical := {6,6,6};
        ARMOR_HIGHMID: physical := {7,7,7};
        ARMOR_HIGHHIGH: physical := {8,8,8};
    endcase

    case(tipo)
        //TIER 1
        SHIELD_LOWLOW: return {0,0,0};
        SHIELD_LOWMID: return {0,0,0};
        SHIELD_LOWHIGH: return {0,0,0};

        //TIER 2
        SHIELD_MIDLOW: return {0,0,0};
        SHIELD_MIDMID: return {0,0,0};
        SHIELD_MIDHIGH: return {0,0,0};

        //TIER 3
        SHIELD_HIGHLOW: return {0,0,0};
        SHIELD_HIGHMID: return {0,0,0};
        SHIELD_HIGHHIGH: return {0,0,0};

        //TIER 4
        "escudo de corpo fraco": return {0,0,0};
        "escudo de corpo medio": return {0,0,0};
        "escudo de corpo forte": return {0,0,0};
    endcase

	var resists := struct;
	resists.+PhysicalResist := physical[1];
	resists.+SlashResist := physical[1];
	resists.+PiercingResist := physical[2];
	resists.+BluntResist := physical[3];
	resists.+FireResist := 0;
	resists.+ColdResist := 0;
	resists.+PoisonResist := 0;
	resists.+EnergyResist := 0;
	return resists;
endfunction

function getSlotMod(item)
	var stats := array;
	var slot := PegarSlot(item).upper;

    case(slot) // {SlashResist, PiercingResist, BluntResist, DexPenalty}
        HEAD: stats := {1,1,1,-1};
        NECK: stats := {1,1,1,-1};
        BODY: stats := {4,4,4,-3};
        ARMS: stats := {2,2,2,-2};
        HANDS: stats := {1,1,1,-1};
        LEGS: stats := {3,3,3,-2};
        SHIELD: stats := {0,0,0,-1};
    endcase

    var resists := struct;
	resists.+PhysicalResist := stats[1];
	resists.+SlashResist := stats[1];
	resists.+PiercingResist := stats[2];
	resists.+BluntResist := stats[3];
	resists.+FireResist := 0;
	resists.+ColdResist := 0;
	resists.+PoisonResist := 0;
	resists.+EnergyResist := 0;
	resists.+DexPenalty := stats[4];
	return resists;
endfunction
