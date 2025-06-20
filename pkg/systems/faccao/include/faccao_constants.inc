// faccao_constants.inc
//
// CONSTANTES COMPLETAS para o sistema de facções.
// ABORDAGEM CONSERVADORA: Mantém todas as constantes para evitar quebras.

// =======================================
// Propriedades padronizadas
// =======================================

// Propriedades padronizadas (resolve inconsistência de casing)
const TERRITORY_FACTION_PROP := "faction";  // Padronizado minúsculo
const OBJ_FACTION_PROP := "faction";        // Padronizado minúsculo

// Propriedade que identifica se um território é capital
const TERRITORY_IS_CAPITAL_PROP := "IsCapital";

// Propriedade que armazena o ID único do território
const TERRITORY_ID_PROP := "TerritoryID";

// Propriedades de território essenciais
const TERRITORY_NAME_PROP := "RectName";
const TERRITORY_RECT_PROP := "Rect";
const TERRITORY_REALM_PROP := "Realm";

// Propriedade que armazena o nome da facção na guild
const FACTION_PROP_NAME := "name";          // Nome da facção

// =======================================
// Propriedades de Baús de Doação
// =======================================

// Propriedade que identifica o tipo de interface
const INTERFACE_TYPE_PROP := "interface_type";

// Propriedade que armazena o ID único do baú
const CHEST_ID_PROP := "faction_chest_id";

// Propriedade que armazena o serial do baú na facção
const GUILD_CHEST_PROP := "donation_chest";

// =======================================
// Propriedades de Recursos
// =======================================

// Prefixo para propriedades de doação
const DONATION_PREFIX := "IsKingdomDonation";

// Tipos de recursos
const RESOURCE_TYPE_COMIDA := "Comida";
const RESOURCE_TYPE_MOEDAS := "Moedas";
const RESOURCE_TYPE_COURO := "Couro";
const RESOURCE_TYPE_METAL := "Metal";
const RESOURCE_TYPE_MADEIRA := "Madeira";
const RESOURCE_TYPE_PANO := "Pano";
const RESOURCE_TYPE_PEDRA := "Pedra";
const RESOURCE_TYPE_JOIAS := "Joias";

// =======================================
// Valores padrão
// =======================================

// Valor padrão para facção ausente
const DEFAULT_FACTION := "Selvagem";

// Valor para reino neutro
const KINGDOM_FACTION := "Reino";

// =======================================
// Constantes para sistema de recursos
// =======================================

// Nome do arquivo de dados para recursos do reino
const KINGDOM_RESOURCE_DATAFILE := "kingdom_resources";

// Nome do elemento para recursos do reino
const KINGDOM_RESOURCE_ELEM := "resources";

// Nome do datafile que armazena recursos de facções
const FACTION_RESOURCES_DATAFILE := "faction_resources";

// Nome do datafile que armazena recursos do reino
const REINO_RESOURCES_DATAFILE := "kingdom_resources";

// Nome do elemento para recursos do reino
const REINO_RESOURCES_ELEM := "resources";

// Nome da propriedade na guilda para armazenar recursos
const GUILD_RESOURCES_PROP := "faction_resources";

// =======================================
// Constantes para sistema de cache
// =======================================

// Tempo (em segundos) para expiração do cache
const CACHE_EXPIRY_TIME := 300; // 5 minutos

// Tempo (em segundos) para atualização do datafile global
const DATAFILE_UPDATE_INTERVAL := 1800; // 30 minutos

// Nome da propriedade onde o cache é armazenado
const CACHE_PROP_NAME := "recursos_cache";

// Nome da propriedade que armazena timestamp da última atualização
const CACHE_UPDATE_PROP := "recursos_ultimo_update";

// Nome da propriedade que armazena timestamp da última sincronização com datafile
const DATAFILE_SYNC_PROP := "recursos_ultima_sync";

// Flag para indicar que um recurso foi adicionado/removido e o cache precisa ser atualizado
const CACHE_DIRTY_FLAG := "recursos_cache_dirty";

// =======================================
// Constantes para sistema de identificação
// =======================================

// Propriedade para ID único padronizado
const UNIQUE_ID_PROP := "unique_id";
const ITEM_UNIQUE_ID_PROP := "unique_id";

// =======================================
// Objtypes dos Itens Territoriais
// =======================================

// Baú de doações
const TERRITORIO_BAU := 0xFF11;

// Mesa de gestão de reino
const TERRITORIO_MESA := 0xFE5C; 

// Mesa de inscrição da facção
const TERRITORIO_MESA_INSCRICAO := 0xF8A8;

// Mural de recrutamento
const TERRITORIO_MURAL := 0x46AE;

// Estandarte do reino
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

// Tempo (em segundos) entre verificações de pagamento de caravanas
const CARAVAN_CYCLE_WAIT := 60; // 1 minuto

// Duração (em segundos) de um contrato de caravana para testes
const CARAVAN_TEST_DURATION_SECONDS := 300; // 5 minutos para testes rápidos

// =======================================
// TIMING DE CICLOS
// =======================================
const UPKEEP_CYCLE_WAIT := 60;          // 1 minuto para upkeep
const CYCLE_WAIT := 120;                // 2 minutos para ciclos gerais

// =======================================
// BORDAS DO MAPEADOR
// =======================================
const BORDER_ITEM_LATERAL := 0xF873;
const BORDER_ITEM_TRANSVERSAL := 0xF878;
const BORDER_DURATION := 300;           // 5 minutos
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

// Backgrounds padrão para gumps do sistema (SOMENTE ESPECÍFICOS DA FACÇÃO)
const BKGND_BLUESLAB_FACCAO := 9200;  // Blue slab background (específico para sistema de facção)

// =======================================
// Constantes de Botões e Interface
// =======================================

// Botões principais do sistema de gestão
const BUTTON_BUY_UNIFORM := 0x2;
const BUTTON_ADD_PLAYER := 0x3;
const BUTTON_MONEY_TRANSFER := 0x5;
const BUTTON_CHANGE_DOORS := 0x4;
const BUTTON_BAN_PLAYER := 0x6;
const BUTTON_CHANGE_PLAYER := 0x1222;

// Botões de NPC
const BUTTON_ADD_NPC := 0x100;
const BUTTON_REPORT_NPC := 0x109;
const BUTTON_CONTRACT_NPC := 0x110;
const BUTTON_DISMISS_NPC := 0x115;

// Botões de leis
const BUTTON_ADD_LAW := 0x200;
const BUTTON_CHANGE_LAW := 0x225;

// Botões administrativos
const BUTTON_GM_OPTS := 0x301;
const BTN_FACCAO := 0x100;
const BTN_NEW_FACCAO := 0x4;
const BUTTON_CHECK_GUILD := 0x350;

// Botões de gestão de reino
const BOTAO_REINO := 1;
const BOTAO_LEIS := 2;
const BOTAO_FACCOES := 3;
const BOTAO_DOACOES := 4;
const BOTAO_CONSTRUCOES := 5;
const BOTAO_GUERRA := 6;

// =======================================
// Constantes de Tipos de Objetos (MAIÚSCULO)
// =======================================

// Tipos de containers
const TIPO_BAU := 0xFF11;
const TIPO_BAU_DOACOES := 0xFF11;

// Tipos de mobília
const TIPO_MESA := 0xFE5C;
const TIPO_MESA_INSCRICAO := 0xF8A8;
const TIPO_MURAL := 0x46AE;

// Tipos de itens especiais
const TIPO_TOTEM_AQUISICAO := 0xA102;
const TIPO_MAPEADOR := 0xA103;
const TIPO_MESA_INSCRICAO_ITEM := 0xA104;
const TIPO_FERRAMENTAS_CONSTRUCAO := 0xFA50;
const TIPO_ESTANDARTE := 0xFDDF;
