# Novo Sistema de Equipamentos para o Epic! Shard

## Classe WeaponWrapper

### Visão Geral
A classe `WeaponWrapper` é um wrapper especializado para itens de arma dentro do sistema de equipamentos Epic. Ela estende a classe base `EquipWrapper` e implementa a interface `IWeaponWrapper`, fornecendo uma camada de abstração abrangente para gerenciamento de armas.

### Hierarquia da Classe
```
EquipWrapper (classe base)
└── WeaponWrapper
    └── implementa IWeaponWrapper
```

### Funcionalidades

#### Sistema de Dano
A classe suporta um sistema sofisticado de dano com três tipos de dano:
- **Dano Cortante**: Ataques de corte/fatiamento
- **Dano Perfurante**: Ataques de punhalada/perfuração  
- **Dano Contundente**: Ataques de esmagamento/impacto

Cada arma pode ter valores de **dano principal** e **dano secundário** para cenários de combate duplo.

#### Sistema de Velocidade
- **Níveis de Velocidade**: Categorias de velocidade baseadas em configuração para fácil balanceamento
- **Velocidade Real**: Valores numéricos de velocidade para cálculos de combate
- **Modificação de Velocidade**: Ajustes dinâmicos de velocidade através de modificadores

#### Sistema de Acerto Crítico
- **Níveis de Chance Crítica**: Categorias de chance crítica baseadas em template
- **Valores de Chance Crítica**: Probabilidade de acerto crítico baseada em porcentagem

#### Sistema de Alcance
- **Alcance Mínimo**: Distância mínima efetiva de ataque
- **Alcance Máximo**: Distância máxima efetiva de ataque

### Modos do Construtor

A classe opera em dois modos distintos baseados no parâmetro de entrada:

#### 1. Modo Instância de Item (`POLCLASS_WEAPON`)
Quando inicializada com um item de arma real:
- Carrega propriedades diretamente do estado atual do item
- Lê valores de dano das propriedades do item (`SlashMainDamage`, `PiercingMainDamage`, etc.)
- Obtém velocidade da propriedade `AttackSpeed` do item
- Recupera chance crítica da propriedade `CriticalChance` do item

#### 2. Modo Template
Quando inicializada com uma configuração de template:
- Calcula dano usando funções de busca baseadas em níveis (`getDamageByTier`)
- Analisa strings de template usando `SplitWords()` para propriedades multi-valor
- Deriva velocidade dos níveis de velocidade usando `getSpeedByTier()`
- Computa chance crítica dos níveis usando `getCriticalChanceByTier()`

### Propriedades

| Propriedade | Tipo | Descrição |
|-------------|------|-----------|
| `_attribute` | String | Atributo primário da arma (ex: "Strength", "Dexterity") |
| `_tipoAtaque` | String | Classificação do tipo de ataque |
| `_speedTier` | String | Identificador do nível de velocidade do template |
| `_speed` | Integer | Valor numérico da velocidade de ataque |
| `_mainDamageTier` | Struct | Informação do nível para dano principal |
| `_offDamageTier` | Struct | Informação do nível para dano secundário |
| `_mainDamage` | Struct | Valores de dano principal (Cortante/Perfurante/Contundente) |
| `_offDamage` | Struct | Valores de dano secundário (Cortante/Perfurante/Contundente) |
| `_criticalChanceTier` | String | Identificador do nível de chance crítica |
| `_criticalChance` | Integer | Porcentagem de chance de acerto crítico |
| `_minRange` | Integer | Alcance mínimo de ataque |
| `_maxRange` | Integer | Alcance máximo de ataque |
| `_penalties` | Struct | Penalidades e restrições específicas da arma |

### Métodos

#### Métodos Getter
- `GetAttribute()` - Retorna o atributo primário da arma
- `GetTipoAtaque()` - Retorna o tipo de ataque
- `GetSpeedTier()` / `GetSpeed()` - Retorna nível/valor de velocidade
- `GetMainDamageTier()` / `GetMainDamage()` - Retorna nível/valores de dano principal
- `GetOffDamageTier()` / `GetOffDamage()` - Retorna nível/valores de dano secundário
- `GetCriticalChanceTier()` / `GetCriticalChance()` - Retorna nível/valor de chance crítica
- `GetMinRange()` / `GetMaxRange()` - Retorna limites de alcance

#### Métodos Setter
- `SetSpeed(speed)` - Modifica a velocidade de ataque da arma
- `SetMainDamage(slash, piercing, blunt)` - Atualiza valores de dano principal
- `SetOffDamage(slash, piercing, blunt)` - Atualiza valores de dano secundário
- `SetCriticalChance(criticalChance)` - Define porcentagem de acerto crítico

#### Métodos Utilitários
- `Update()` - Sincroniza todas as mudanças de volta para o item subjacente
- `ToString()` - Gera representação string detalhada para debug

### Exemplo de Uso

```escript
// Inicializar com um item de arma existente
var weapon := WeaponWrapper(weaponItem);

// Obter valores de dano atuais
var mainDamage := weapon.GetMainDamage();
print("Cortante: " + mainDamage.SlashDamage);

// Modificar propriedades da arma
weapon.SetSpeed(25);
weapon.SetMainDamage(15, 10, 5);

// Aplicar mudanças ao item
weapon.Update();

// Saída de debug
print(weapon.ToString());
```
## Classe ArmorWrapper

### Visão Geral
A classe `ArmorWrapper` é um wrapper especializado para itens de armadura dentro do sistema de equipamentos Epic. Ela estende a classe base `EquipWrapper` e implementa a interface `IArmorWrapper`, fornecendo gerenciamento completo de propriedades de proteção.

### Hierarquia da Classe
```
EquipWrapper (classe base)
└── ArmorWrapper
    └── implementa IArmorWrapper
```

### Funcionalidades

#### Sistema de Cobertura
- **Coverage**: Define quais partes do corpo a armadura protege
- **Tipo de Proteção**: Classificação do material/tipo da armadura (couro, malha, placa, etc.)

#### Sistema de Resistências
- **Resistências Físicas**: Proteção contra dano cortante, perfurante e contundente
- **Resistências Elementais**: Proteção contra fogo, frio, energia e veneno
- **Cálculo Dinâmico**: Combina resistências do template com modificadores por slot

#### Sistema de Penalidades
Armaduras aplicam penalidades baseadas no tipo e slot:
- **DexPen**: Penalidade de destreza
- **SneakPen**: Penalidade de furtividade  
- **MAPen**: Penalidade de artes marciais
- **MageryPen**: Penalidade de magia
- **AwarenessPen**: Penalidade de percepção

### Propriedades

| Propriedade | Tipo | Descrição |
|-------------|------|-----------|
| `_coverage` | Array | Lista de partes do corpo cobertas pela armadura |
| `_tipoProtecao` | String | Tipo de proteção/material da armadura |
| `_physicalResistsTier` | Struct | Níveis de resistência física do template |
| `_physicalResists` | Struct | Valores de resistência física (Cortante/Perfurante/Contundente) |
| `_elementalResistsTier` | Struct | Níveis de resistência elemental do template |
| `_elementalResists` | Struct | Valores de resistência elemental (Fogo/Frio/Energia/Veneno) |
| `_penalties` | Struct | Penalidades aplicadas pela armadura |

### Métodos

#### Métodos Getter
- `GetTipoProtecao()` - Retorna o tipo de proteção da armadura
- `GetCoverage()` - Retorna as partes do corpo cobertas
- `GetPhysicalResists()` - Retorna resistências físicas
- `GetElementalResists()` - Retorna resistências elementais
- `GetPenalties()` - Retorna penalidades da armadura

#### Métodos Setter
- `SetPhysicalResists(resists)` - Define resistências físicas
- `SetElementalResists(resists)` - Define resistências elementais
- `SetPenalties(penalties)` - Define penalidades da armadura

#### Métodos Utilitários
- `Update()` - Sincroniza mudanças para o item
- `ToString()` - Representação string para debug

### Exemplo de Uso

```escript
// Inicializar com item de armadura
var armor := ArmorWrapper(armorItem);

// Verificar cobertura
var coverage := armor.GetCoverage();
print("Cobertura: " + coverage);

// Obter resistências
var physResists := armor.GetPhysicalResists();
print("Resistência Cortante: " + physResists.SlashResist);

// Modificar resistências
armor.SetPhysicalResists(struct{
    SlashResist := 10,
    PiercingResist := 8,
    BluntResist := 12
});

// Aplicar mudanças
armor.Update();
```