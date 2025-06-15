// faccao_constants.inc
//
// Este arquivo define constantes padronizadas para propriedades
// usadas pelo sistema de facções, territórios e recursos.
// 
// Usar estas constantes em vez de strings hardcoded evita
// erros de digitação e inconsistências no sistema.

// =======================================
// Propriedades de Facção
// =======================================

// Propriedade que armazena o nome da facção em objetos de facção
const FACTION_PROP_NAME := "name";

// Propriedade que armazena a facção em objetos do mundo
const OBJ_FACTION_PROP := "faction";  // Padronizado para minúsculo

// Propriedade que armazena a facção em elementos de território
const TERRITORY_FACTION_PROP := "faction";  // Padronizado para minúsculo

// =======================================
// Propriedades de Território
// =======================================

// Propriedade que armazena o nome do território
const TERRITORY_NAME_PROP := "RectName";

// Propriedade que armazena as coordenadas do território
const TERRITORY_RECT_PROP := "Rect";

// Propriedade que armazena o realm do território
const TERRITORY_REALM_PROP := "Realm";

// Propriedade que identifica se um território é capital
const TERRITORY_IS_CAPITAL_PROP := "IsCapital";

// Propriedade que armazena o ID único do território
const TERRITORY_ID_PROP := "TerritoryID";

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
// Comentários para referência dos Objtypes Territoriais
// =======================================

// Lista de objtypes territoriais (não pode ser array constante no POL):
// 0x46AE - muralrecrutamento
// 0xF8A8 - MesaDeInscricao  
// 0xFE5C - mesadegestaodereino
// 0xFF11 - BaudeDoacoes
// 0xFDDF - EstandarteReino

// =======================================
// Constantes para sistema de caravanas
// =======================================

// Tempo (em segundos) entre verificações de pagamento de caravanas
const CARAVAN_CYCLE_WAIT := 60; // 1 minuto

// Duração (em segundos) de um contrato de caravana para testes
const CARAVAN_TEST_DURATION_SECONDS := 300; // 5 minutos para testes rápidos