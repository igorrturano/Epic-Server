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

// Propriedade que armazena o ID único do território
const TERRITORY_ID_PROP := "TerritoryID";

const ITEM_UNIQUE_ID_PROP := "unique_id";

// =======================================
// Constantes para sistema de identificação
// =======================================

// Propriedade para ID único padronizado
const UNIQUE_ID_PROP := "unique_id";

// Tipos de itens
const ITEM_TYPE_BAU := "BAU";
const ITEM_TYPE_MESA := "MESA";
const ITEM_TYPE_MURAL := "MURAL";

// Propriedade para território capital
const TERRITORY_IS_CAPITAL_PROP := "is_capital";