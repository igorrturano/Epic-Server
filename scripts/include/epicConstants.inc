use uo;

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
	RESIST_EASY := 8,
	RESIST_MEDIUM := 15,
	RESIST_HARD := 20,
	RESIST_VERY_HARD := 25,
	RESIST_EXTREMELY_HARD := 30,
	RESIST_ALMOST_IMPOSSIBLE := 35
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
	BLUE_ICE := "<BASEFONT COLOR=#00BFFF>",
	RED_FIRE := "<BASEFONT COLOR=#e95c4b>",
	LILAS := "<BASEFONT COLOR=#d998a0>",
    FISICO_OFF := "<BASEFONT COLOR=#B18904>",
	FISICO := "<BASEFONT COLOR=#BDBDBD>",
	GREEN := "<BASEFONT COLOR=#00FF00>",
	RED := "<BASEFONT COLOR=#FF0000>"
endenum
