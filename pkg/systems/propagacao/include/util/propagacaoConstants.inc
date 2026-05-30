const MARCADOR_PROPAGACAO_OBJTYPE := 0xe13a;
const PROPAGACAO_TEMPERATURA_BASE := 20;

enum class EV_PROPAGACAO
	TIMEOUT, // OBS: Esta tem que ser a posicao zero, pois é o retorno da wait_for_event.
	ESTIMULO,
	COMBUSTIVEL,
	RESFRIAMENTO,
	PISAO,
endenum

enum class PROPAGACAO_TIPO
	INCENDIO := 1,
	PATOGENICO := 2,
endenum

enum class TILE_SCANNER
	ITENS := 1,
	STATICOS := 2,
	MOBILES := 3,
	TERRENO := 4
endenum

enum class RESFRIAMENTOS
	BASICO := -10,
	RAZOAVEL := -55,
	INTENSO := -120,
endenum

enum class COMBUSTIVEIS
	TERRENO := "terrenoInflamavel",
	ESTATICOS := "staticInflamavel",
	MAGICO := "magico",
endenum

enum class ESTADO_PROPAGACAO
	DORMENTE, //Só aguarda estimulo.
	LATENTE, //Danifica, mas não propaga
	PRINCIPIO, //Danifica e propaga
	REGULAR, //Danifica + e propaga
	INTENSO //Danifica ++ e propaga
endenum


enum class CONTROLE_PROPAGACAO
	ATIVO,
	DORMENTE,
	MORTO
endenum

enum tipoCalculoEnergia
	CALCULO_ENERGIA_GRAPHIC, 
	CALCULO_ENERGIA_LANDTILE, 
endenum