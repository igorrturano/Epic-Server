use uo;

include ":attributes:attributeConstants";

const PROFISSAO_CONFIG_PATH := ":crafting:config/craftConfig";
const RECURSOS_CONFIG_PATH := ":crafting:config/resourceList";
const VICIO_MAX_POINTS := 100;

const CRAFTING_ESTACAO_SUPERFICIE := "superficie";

enum class DAYNIGHT_EVTS
    EV_ADVANCE_DAY_ID,
    EV_DAY_ACCELATARION_CHANGE_ID
endenum

enum class SKILL_PRIORITY
    PRIMARY := 3,
    SECONDARY := 2,
    TERTIARY := 1
endenum

enum CULTURA
    AIGLANA := "Aiglana",
    HAROLOTH := "Har'oloth",
    LESTE := "Povos do Leste",
    BJORSKA := "Björska",
    ALURIEL := "Alüriel",
    BADURAN := "Badûran",
    GULTHRAK := "Gulthrak",
    COLHESIANA := "Colhesiana"
endenum

enum RACA
    ANAO := "Anão",
    DROW := "Drow",
    ELFO := "Elfo",
    ORC := "Orc",
    HUMANO :="Humano",
    PEQUILHO := "Pequilho"
endenum

enum RELIGIAO
    VENERATIO := "Veneratio",
    ELUITHORTH := "Eluith'orth",
    KAI := "Kai", 
    HALLEUAD := "Halleuad",
    LORE := "Lórё", 
    BOKK_BHARAZ := "Bokk Bharaz", 
    USHKRAR := "Ushkr'ar",
    CETICOS := "Céticos"
endenum

enum ARCANE_SCHOOLS
    EVOCATIUM := "Evocatium",
    VELKYN := "Velkyn",
    ARADALORE := "Aradalorë",
    AHTALELORETH := "Ahtalelorëth",
    KAIJIN := "Kaijin"
endenum

enum ALCHEMICAL_SCIENCES
    BOMBA := "Bomba",
    VENENO := "Veneno",
    BUFF := "Buff",
endenum

enum CAMINHOS
    GUERREIRO := "Guerreiro",
    LADINO := "Ladino",
    SABIO := "Sabio"
endenum

// Path-rework hide rules: each archetype loses exactly one Military skill
// so all three end up with 13 visible out of 14 raw. Guerreiro and Ladino
// hide Magery (they have no spellcasting identity); Sabio hides Tactics
// (gish subclasses scale melee on patron skill instead — see spec 05).
function HiddenSkillForArchetype(classe)
    if (classe == GUERREIRO || classe == LADINO)
        return MAGERY;
    elseif (classe == SABIO)
        return TACTICS;
    endif
    return "";
endfunction

enum PATH
    MILITAR := "Militar",
    CIVIL := "Civil"
endenum

// Path-rework skill buckets (Phase 2). Each path has 13 skills; total 26 new system skills.
// Order within each bucket determines display order in the slot picker.
var MILITARY_SKILLS := array{
    "Wrestling", "One_Handed", "Two_Handed", "Dual_Wield", "Parry",
    "Ranged", "Tactics", "Stealth", "Beastmastery", "Magery",
    "Arcana", "Theology", "Science", "Nature"
};

var CIVIC_SKILLS := array{
    "Smithing", "Carpentry", "Tailoring", "Provisioning", "Alchemy",
    "Mining", "Lumberjacking", "Survival", "Taming", "Medicine",
    "Mechanics", "Perception", "Theft"
};

const FIST := 0x1F020;

enum RESIST_DIFF
	RESIST_EASY := 12,
	RESIST_MEDIUM := 19,
	RESIST_HARD := 24,
	RESIST_VERY_HARD := 29,
	RESIST_EXTREMELY_HARD := 34,
	RESIST_ALMOST_IMPOSSIBLE := 40
endenum

enum MOEDAS
	SESTERTIUS := 0xBA63, // cobre
	DENARIUS := 0xBA64, // prata
	AUREUS := 0xBA65 // ouro
endenum

enum HTML_TEXT_COLOR
    WHITE := "<BASEFONT COLOR=#FFFFFF>",
	AMARELO_CLARO := "<BASEFONT COLOR=#f5d189>",
	LARANJA :=  "<BASEFONT COLOR=#FFA500>",
	ROXO := "<BASEFONT COLOR=#d98cd9>",
	SOFT_GREEN := "<BASEFONT COLOR=#a1eca7>",
	BRIGHT_GREEN := "<BASEFONT COLOR=#00FF7F>",
	BLUE_ICE := "<BASEFONT COLOR=#00BFFF>",
	BLUE := "<BASEFONT COLOR=#7bf8fe>",
	RED_FIRE := "<BASEFONT COLOR=#e95c4b>",
	LILAS := "<BASEFONT COLOR=#d998a0>",
	SOFT_PINK := "<BASEFONT COLOR=#af7ac5>",
    FISICO_OFF := "<BASEFONT COLOR=#B18904>",
	FISICO := "<BASEFONT COLOR=#BDBDBD>",
	GREEN := "<BASEFONT COLOR=#00FF00>",
	RED := "<BASEFONT COLOR=#FF0000>"
endenum
