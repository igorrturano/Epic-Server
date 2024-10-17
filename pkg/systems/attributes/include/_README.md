# Sistema de Avanço e Verificação de Skills

Este diretório contém arquivos essenciais para o gerenciamento de habilidades (skills), atributos (stats) e outras funcionalidades.

### Relação entre os Arquivos:
- **`advanceCheck.inc`** e **`skillCheck.inc`** são os principais arquivos para a validação e avanço de skills e stats. Eles usam funções de `skills.inc`, `stats.inc` e `settings.inc` para verificar os limites e aplicar as mudanças.
- **`vitals.inc`** é chamado quando um stat ou skill afeta as condições vitais do jogador.
- **`settings.inc`** centraliza as configurações usadas em todos os outros arquivos para controlar limites de ganho e chances de sucesso.

### Futuro:
No futuro, o arquivo `advanceCheck.inc` será dividido entre **skills** e **stats**, para facilitar o gerenciamento. O arquivo `skillCheck.inc`  deverá ser incorporado dentro de `skills.inc`. 

## Estrutura dos Arquivos:

### 1. `advanceCheck.inc`
Este arquivo é responsável por controlar como os jogadores aumentam suas **habilidades** (skills) e **atributos** (stats). Ele faz a validação para ver se um jogador pode ganhar pontos de skill ou stat e aplica esses ganhos quando aplicável.

**Principais Funções:**
- Verifica se o jogador está elegível para ganhar um ponto de skill/stat.
- Faz o cálculo e aplica o ganho (geralmente em pequenos incrementos).
- Informa o jogador sobre o progresso ou sucesso/falha do avanço.

### 2. `skillCheck.inc`
Este arquivo lida com a **validação das skills**. Sempre que um jogador usa uma habilidade, este arquivo calcula as chances de sucesso ou falha, e se a ação resultará em aumento de skill ou stat.

**Principais Funções:**
- Verifica se o jogador pode usar a skill.
- Calcula as chances de sucesso de acordo com o nível de dificuldade e a skill do jogador.
- Verifica se a skill ou stat será aumentada após o uso.

### 3. `skills.inc`
Este arquivo contém funções para **gerenciamento e manipulação das skills**. Ele fornece métodos para:
- Recuperar valores de skills.
- Modificar valores de skills (modificadores).
- Aplicar limites de skills (caps) com base em regras e configuração do servidor.

### 4. `stats.inc`
Similar ao `skills.inc`, este arquivo cuida dos **atributos** (força, destreza, inteligência, etc.), oferecendo funções para:
- Recuperar valores de stats.
- Modificar valores de stats.
- Aplicar limites de stats (caps).

### 5. `vitals.inc`
Este arquivo gerencia as **condições vitais** (vida, mana, stamina). Ele é utilizado principalmente para recalcular esses valores sempre que um jogador ganha ou perde pontos de skill ou stat.

### 6. `titles.inc`
Este arquivo é responsável pela gestão dos **títulos** dos jogadores, que podem ser atribuídos com base em seus níveis de skill ou estatísticas.

### 7. `regen.inc`
Controla o **regeneramento** de mana, vida e stamina ao longo do tempo.

### 8. `settings.inc`
Este arquivo contém as **configurações gerais** para o sistema de skills e stats. Ele é usado por vários arquivos, como `advanceCheck.inc` e `skillCheck.inc`, para buscar valores de configuração como limites máximos (caps) ou tempos de espera entre avanços.
