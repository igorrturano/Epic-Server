use uo;

const PROFISSAO_CONFIG_PATH := ":crafting:config/craftConfig";
const RECURSOS_CONFIG_PATH := ":crafting:config/resourceList";

enum class DAYNIGHT_EVTS
    EV_ADVANCE_DAY_ID,
    EV_DAY_ACCELATARION_CHANGE_ID
endenum

enum CULTURA
    AIGLANA := "Aiglana",
    HAROLOTH := "Har'oloth",
    LESTE := "Povos do Leste",
    BJORSKA := "Björska",
    ALURIEL := "Alüriel",
    BADURAN := "Badûran",
    GULTHRAK := "Gulthrak",
    POLKINEA := "Polkinea"
endenum

enum RACA
    ANAO := "Anão",
    DROW := "Drow",
    ELFO := "Elfo",
    ORC := "Orc",
    HUMANO :="Humano",
    POLSKI := "Polski"
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

enum CORES_ELEMENTAIS
    FOGO := 2480,
	FOGO_LIGHT := 246,
    GELO := 2891,
	GELO_LIGHT := 196,
    RAIO := 2342,
	RAIO_LIGHT := 211
endenum

enum GAME_MODE
    TRUE_MODE := "True Mode",
    STORY_MODE := "Story Mode"
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

enum ANIMATIONS_ID
    EPIC_FX_TELEPORT := 0xD11B,
    EPIC_FX_STUN := 0xD106,
    EPIC_FX_MAZE := 0xD0D3,
    EPIC_FX_COLOR_SPRAY := 0xCC01,
    EPIC_FX_WARP_SPAWNER := 0xCBD8,
    EPIC_FX_SPEED_BOOTS := 0xCBAB,
    EPIC_FX_DAGGER_THROW := 0xCBA2,
    EPIC_FX_BAN_HAMMER := 0xCB38,
    EPIC_FX_SWORD_THROW := 0xCB2F,
    EPIC_FX_MACE_THROW := 0xCB26,
    EPIC_FX_FLAIL_THROW := 0xCB1D,
    EPIC_FX_GOLDEN_ARMOR_SPELL := 0xCB03,
    EPIC_FX_AMPULHETA := 0xCAED,
    EPIC_FX_AMPULHETA_ALT := 0xC9E0,
    EPIC_FX_TORNADO := 0xCADD,
    EPIC_FX_ILUMINADO := 0xCAC8,
    EPIC_FX_CURSE_SPELL := 0xCAB2,
    EPIC_FX_BLESS_SPELL := 0xCA9E,
    EPIC_FX_PELE_DE_ARVORE := 0xCA8B,
    EPIC_FX_SET_TRAP := 0xCA70,
    EPIC_FX_RAISE_UNDEAD := 0xCA63,
    EPIC_FX_SANTUARIO := 0xCA50,
    EPIC_FX_MAGIC_IMMUNITY := 0xCA43,
    EPIC_FX_SOUL_LEAVING := 0xCA32,
    EPIC_FX_MORALE_DOWN := 0xCA28,
    EPIC_FX_AXE_THROW := 0xCA0E,
    EPIC_FX_MORALE_UP := 0xCA03,
    EPIC_FX_BLOOD_DROP := 0xC9FC,
    EPIC_FX_HAMMER_THROW := 0xC9F5,
    EPIC_FX_HIPNOSYS := 0xC9EB,
    EPIC_FX_BODY_EXPLOSION := 0xC9D5,
    EPIC_FX_SUMMON_BEAST := 0xDAD8
endenum
