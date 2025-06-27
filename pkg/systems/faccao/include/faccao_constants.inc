// :faccao:include/faccao_constants.inc
//
// CONSTANTES COMPLETAS para o sistema de facções.
// ABORDAGEM CONSERVADORA: Mantém todas as constantes para evitar quebras.

// =======================================
// Propriedades padronizadas
// =======================================

const TERRITORY_FACTION_PROP := "faction";
const OBJ_FACTION_PROP := "faction";
const LEGACY_FACTION_PROP := "Faction";
const TERRITORY_IS_CAPITAL_PROP := "IsCapital";
const TERRITORY_ID_PROP := "TerritoryID";
const TERRITORY_NAME_PROP := "Name";
const TERRITORY_RECT_PROP := "Rect";
const TERRITORY_REALM_PROP := "Realm";
const FACTION_PROP_NAME := "name";

// =======================================
// Propriedades de Baús de Doação
// =======================================

const INTERFACE_TYPE_PROP := "interface_type";
const CHEST_ID_PROP := "faction_chest_id";
const GUILD_CHEST_PROP := "donation_chest";

// =======================================
// Propriedades de Recursos
// =======================================

const DONATION_PREFIX := "IsKingdomDonation";
const RESOURCE_TYPE_COMIDA := "Comida";
const RESOURCE_TYPE_MOEDAS := "Moedas";
const RESOURCE_TYPE_COURO := "Couro";
const RESOURCE_TYPE_METAL := "Metal";
const RESOURCE_TYPE_MADEIRA := "Madeira";
const RESOURCE_TYPE_PANO := "Pano";
const RESOURCE_TYPE_PEDRA := "Pedra";
const RESOURCE_TYPE_JOIAS := "Joias";
const RESOURCE_TYPE_UNKNOWN := "desconhecido";

// =======================================
// Prefixos e padrões para datafiles
// =======================================

const FACTION_ELEMENT_PREFIX := "faction_";
const FACTION_DUPLICATE_PREFIX := "faction_faction_";

// =======================================
// Valores padrão
// =======================================

const DEFAULT_FACTION := "Selvagem";
const KINGDOM_FACTION := "Reino";

// =======================================
// Constantes para sistema de recursos
// =======================================

const KINGDOM_RESOURCE_DATAFILE := "kingdom_resources";
const KINGDOM_RESOURCE_ELEM := "resources";
const FACTION_RESOURCES_DATAFILE := "faction_resources";
const REINO_RESOURCES_DATAFILE := "kingdom_resources";
const REINO_RESOURCES_ELEM := "resources";
const GUILD_RESOURCES_PROP := "faction_resources";

// =======================================
// Constantes para sistema de cache
// =======================================

const CACHE_EXPIRY_TIME := 90;
const DATAFILE_UPDATE_INTERVAL := 1800;
const CACHE_PROP_NAME := "recursos_cache";
const CACHE_UPDATE_PROP := "recursos_ultimo_update";
const DATAFILE_SYNC_PROP := "recursos_ultima_sync";
const CACHE_DIRTY_FLAG := "recursos_cache_dirty";

// =======================================
// Constantes para sistema de identificação
// =======================================

const UNIQUE_ID_PROP := "unique_id";
const ITEM_UNIQUE_ID_PROP := "unique_id";

// =======================================
// Objtypes dos Itens Territoriais
// =======================================

const TERRITORIO_BAU := 0xFF11;
const TERRITORIO_MESA := 0xFE5C; 
const TERRITORIO_MESA_INSCRICAO := 0xF8A8;
const TERRITORIO_MURAL := 0x46AE;
const TERRITORIO_ESTANDARTE := 0xFDDF;

// Tipos de itens
const ITEM_TYPE_BAU := "BAU";
const ITEM_TYPE_MESA := "MESA";
const ITEM_TYPE_MESA_INSCRICAO := "MESA_INSCRICAO";
const ITEM_TYPE_MURAL := "MURAL";
const ITEM_TYPE_ESTANDARTE := "ESTANDARTE";
const ITEM_TYPE_FERRAMENTAS_CONSTRUCAO := "FERRAMENTAS_CONSTRUCAO";

// =======================================
// Constantes para sistema de caravanas
// =======================================

const CARAVAN_CYCLE_WAIT := 30;
const CARAVAN_TEST_DURATION_SECONDS := 140;

// =======================================
// TIMING DE CICLOS
// =======================================
const UPKEEP_CYCLE_WAIT := 45;
const CYCLE_WAIT := 60;

// =======================================
// BORDAS DO MAPEADOR
// =======================================
const BORDER_ITEM_LATERAL := 0xF878;
const BORDER_ITEM_TRANSVERSAL := 0xF873;
const BORDER_DURATION := 300;
const BORDER_HUE := 39;

// =======================================
// CUSTOS PADRÃO
// =======================================
const MAPEADOR_CUSTO := 500;
const FERRAMENTAS_CUSTO := 500;
const BANNER_CUSTO := 500;

// =======================================
// OBJTYPES DE FERRAMENTAS
// =======================================
const MAPEADOR_OBJTYPE := 0x14F5;
const FERRAMENTAS_OBJTYPE := 0xFA50;

// =======================================
// Constantes de Interface Gráfica (Gumps)
// =======================================

const BKGND_BLUESLAB_FACCAO := 9200;

// =======================================
// Constantes de Botões e Interface
// =======================================

const BUTTON_BUY_UNIFORM := 0x2;
const BUTTON_ADD_PLAYER := 0x3;
const BUTTON_MONEY_TRANSFER := 0x5;
const BUTTON_CHANGE_DOORS := 0x4;
const BUTTON_BAN_PLAYER := 0x6;
const BUTTON_CHANGE_PLAYER := 0x1222;
const BUTTON_ADD_NPC := 0x100;
const BUTTON_REPORT_NPC := 0x109;
const BUTTON_CONTRACT_NPC := 0x110;
const BUTTON_DISMISS_NPC := 0x115;
const BUTTON_ADD_LAW := 0x200;
const BUTTON_CHANGE_LAW := 0x225;
const BUTTON_GM_OPTS := 0x301;
const BTN_FACCAO := 0x100;
const BTN_NEW_FACCAO := 0x4;
const BUTTON_CHECK_GUILD := 0x350;
const BOTAO_REINO := 1;
const BOTAO_LEIS := 2;
const BOTAO_FACCOES := 3;
const BOTAO_DOACOES := 4;
const BOTAO_CONSTRUCOES := 5;
const BOTAO_GUERRA := 6;

// =======================================
// Constantes de Tipos de Objetos (MAIÚSCULO)
// =======================================

const TIPO_BAU := 0xFF11;
const TIPO_BAU_DOACOES := 0xFF11;
const TIPO_MESA := 0xFE5C;
const TIPO_MESA_INSCRICAO := 0xF8A8;
const TIPO_MURAL := 0x46AE;
const TIPO_TOTEM_AQUISICAO := 0xA102;
const TIPO_MAPEADOR := 0xA103;
const TIPO_MESA_INSCRICAO_ITEM := 0xA104;
const TIPO_FERRAMENTAS_CONSTRUCAO := 0xFA50;
const TIPO_ESTANDARTE := 0xFDDF;

// =======================================
// TAMANHOS DE TERRITÓRIO PADRÃO
// =======================================
// NOTA: Declarado como 'var' para permitir a inicialização de arrays não vazios.
// A intenção é que sejam tratados como constantes pelo resto do sistema.
var TERRITORY_DIMS_LEVEL_1 := array{15, 10};
var TERRITORY_DIMS_LEVEL_2 := array{30, 25};
var TERRITORY_DIMS_LEVEL_3 := array{40, 35};
var TERRITORY_DIMS_LEVEL_4 := array{45, 45};
var TERRITORY_DIMS_LEVEL_5 := array{50, 70};
const TERRITORY_TYPE_CUSTOM  := 0;


// Em :faccao:include/faccao_constants.inc
var KINGDOM_XP_LEVELS := array{ 100, 200, 300, 400, 500 };