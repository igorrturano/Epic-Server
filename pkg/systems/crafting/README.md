# Sistema de Fabricação (Crafting)

Este diretório contém o **sistema de fabricação customizado** do servidor, responsável por:

- **Criar Componentes** a partir de materiais (mesa de componentes)
- **Montar Itens Complexos** (itens que dependem de componentes e escolhas de materiais)
- **Tratar Itens Montados** em estações específicas (forja, bigorna, etc)
- **Aplicar Qualidade, Materiais e Efeitos** baseados em *tiers*, *perícias* e *habilidades*

---

## 1) Visão Geral do Fluxo de Fabricação

### 1 Processo
1. O jogador usa a mesa de fabricaçao de componentes, ela abre o "Gump de criação de componentes".
2. Ele escolhe componente que deseja produzir, ou usa a ferramenta de memoria para trazer uma lista de componentes.
3. Após selecionar ele escolhe o esforço (se deseja fazer na melhor qualidade ou menor), os materiais necessários (sendo 1 deles o principal), se deseja reciclar e a quantidade.
4. Os componentes serão criados de 20 em vinte, no máximo. Todo componente criado terá uma qualidade atribuída a ele baseada na skill do criador.
   - Os componentes são definidos no arquivo `componentes.cfg` dentro da pasta da respectiva profissão (ex: `pkg/skill/Alchemy` contém as definições dos compoentes de alquimia.).
		`Veja um exemplo de definição:
			{
				name                    Solvente
				encaixe                 solvente // onde este componente encaixa. (também será definido em `encaixes.cfg`)
				pericia                 Alchemy // Profissão usada

				principalMaterial       Oleo // Principal material em `resources.cfg`
				materials               Oleo 1 // [Material Quantidade].
				materials               glass 1
			}
		`
5. Com os componentes em mãos, o jogador segue para a mesa de fabricação de complexos da respectiva profissão.
1. O jogador abre o **Gump de Fabricação** (menu UI) usando a a mesa respectiva (ex: Blacksmithy, Woodworking, Tailoring, etc).
	- Cada profissão tem um cfg especifico para definir as receitas que aparecem no menu de fabricação, ex: Blacksmith -> `SmithMenu.cfg`, Alchemy -> `AlchMenu.cfg.
	- As receitas que existem são definidas no arquivo `recipes.cfg`. Este arquivo é usado para criação de pergaminhos.
	- Toda receita presente nos "Menus.cfg" precisa fazer referência a um objtype [Ou name ] da definição de um itemdesc. Aqui também é definido a classe da receita, ela permite que uma mesma receita tenha diferentes configurações. Para definir uma receita com classe utilize a nomenclatura "nome-classe", exemplo:
				`
					Item	roundnecklace-2
				`
		Agora no arquivo `fabricacaoDesc.cfg` respectivo, defina os encaixes como no exemplo:
				fabricacao 0x1088
				{
					name     necklace
					skill    40 60
					DuracaoFabricacao MUITO_RAPIDO

					ComponenteEncaixe 1 prestigio baseMetalica 1 principal

					#Classe 2
					ComponenteEncaixe 2 prestigio gema 1 principal
					ComponenteEncaixe 2 vazio baseMetalica 1
				}
				`
	- Além disso, É preciso descrever as caracteristicas e encaixes de cada receita. Isso é feito na config `fabricacaoDesc.cfg` especifico de cada profissão.
			Exemplo de definição:
				`fabricacao 0x1efd // Objtype do itemdesc do item
				{
					name     fancy shirt // Nome que irá aparecer na receita
					skill    15 // Dificuldade
					DuracaoFabricacao RAPIDO // Enum que define quantos Ciclos até o item ser fabricado
					
					# (ComponenteEncaixe) (classe da receita) (caracteristica) (encaixe) (quantidade necessária) [principal|opcional]
					ComponenteEncaixe 1 resistenciaClimaCalor tecidoAlinhado 5 principal
				}`

2. Ele escolhe uma **receita** (produto final) e pode criar ou selecionar um **esquema** (lista de componentes escolhidos).
3. O sistema gera uma prop `propFabricado` para representar o produto em construção. Ele contém:
   - receita selecionada (objtype, skill, etc)
   - esquema (componentes escolhidos, cor, material principal, qualidade, modificadores)
   - componentes escolhidos (cada um com `caracteristica`, `encaixe`, `qualidade`, `poderAvaliado` etc)

4. O produto é **montado** (fase de criação) usando `CriaItemFabricado` + `CriaComponenteDescriptor`.
   - Todo item montado é representado pelo mesmo itemDesc(objtype), só o graphic deste item que muda.
		`
		Item 0xe72
		{
			Name itemMontado
			desc Produto Montado
			Script :crafting:useItemMontado
			EquipScript :crafting:equipItemMontado
			graphic 0x0FC1
			weight	1
			RequiresAttention 1

		}
		`
5. Após montado, o item precisa passar por um **tratamento final** (p. ex. forjar na bigorna / estação de tratamento). Essa fase é gerenciada pelos programas:
   - `gerenteTratamentoProduto.src` (controle geral)
   - `cicloTratamentoProduto.src` (loops de ciclos de tratamento / sucessos e falhas)
   - `animacaoTratamentoProduto.src` (mostra animações / validações de estação e ferramenta)
6. Ao final, o item recebe modificadores finais (qualidade, hp, peso, efeitos de material, fechadura, etc) e é finalmente criado..
   - Os valores finais do item serão um somatório do valor da qualidade de cada `caracteristica` dos componentes escolhidos para cada `ComponenteEncaixe` em `fabricacaoDesc`,  mais o valor do `ComponenteEfeito` respectivo em  `resourceList.cfg`.
---

## 2) Componentes-chave e Arquivos Principais

### 2.1 Arquivos de apoio / utilitários

- `include/fabricacaoUtil.inc`
  - Centro da lógica de fabricação: dificuldade, chance, criação de descriptors, cálculo de qualidade, aplicação de efeitos de material, tratamento de estações.
  - Contém funções como:
    - `CriaItemFabricado` (cria o item final com propriedades e `CProps`)
    - `PegaDificuldade` / `PegaChanceFabricacaoBase` (calcula chances e modificadores)
    - `AplicaFechadura` (cria chave se o item tiver atributo de tranca)

- `include/fabricacaoConstants.inc`
  - Constantes que definem estados, enums (componentes, tipos de material, estações, etc).
  - Por exemplo: `OBJTYPE_ITEM_MONTADO`, `EV_TRATAMENTO`, `COMPONENTE_TIERS`, `FABRICACAO_MODO_MATERIAL`, etc.

- `include/caracteristicasFabricacao.inc`
  - Helpers para extrair valores de características e sub-efeitos definidos no `resourceList.cfg`.
  - Ex: `PegaValorFabricacao`, `PegaSubValoresFabricacao`.

### 2.2 Gumps & UI

- `gumps/fabricacaoGumps.inc`
  - Monta e envia os gumps de produtos complexos e componentes.
  - Define IDs, layouts, enums, e a funcionalidade de navegação de páginas.

- `include/controladoraComplexos.inc`
  - Controla o fluxo de criação de itens complexos (receitas + esquemas).
  - Contém: `EnviaItensComplexosGump`, `SelecionaReceita`, `MontaMelhorEsquema`, etc.

- `include/controladoraComponentes.inc`
  - Controla a fabricação de *componentes* (itens que viram parte de outros itens).
  - Inclui a lógica para produzir componentes, reciclar materiais, etc.

- `include/controladoraFabricacaoAjuda.inc`
  - Gump de ajuda (texto das abas para cada profissão) baseado em `config/ajuda.cfg`.

### 2.3 Scripts de execução / tratamento de produto

- `animacaoTratamentoProduto.src` / `include/animacaoTratamentoProduto.inc`
  - Validações de estação / ferramenta, anima a fase de tratamento.
  - Possui duas variantes de animação: ferreiro e padrão. //TODO + no futuro

- `cicloTratamentoProduto.src`
  - Loop de tratamento (ciclos de sucesso/falha), calcula chances usando `PegaDificuldadeFrabicarObjeto`.
  - Atualiza propriedade `PROPNAME_FABRICADO_TRATAMENTO` (sucessos, falhas, ciclos).

- `gerenteTratamentoProduto.src`
  - Programa principal que gerencia a fila de tratamento de itens.
  - Procura itens semelhantes para continuar processos em lote.

- `useItemMontado.src` / `equipItemMontado.src`
  - Funções ligadas ao uso/equipamento de itens montados (`ItemMontado`) para iniciar tratamento.

---

## 3) Configurações Importantes

### 3.1 Materiais & efeitos (`config/resourceList.cfg`)
Contém a definição de todos os materiais disponíveis no sistema, incluindo:
- Nome (`Name`), cor (`Color`), tipo (`Type`)
- Skill mínima para usar material (`Skill`)
- Modificadores (hpMod, weightMod, inflamável, etc)
- Definições de `ComponenteEfeito` que são usadas por `fabricacaoUtil.inc` para aplicar efeitos nos componentes.

### 3.2 Características de componentes (`config/caracteristicas.cfg`)
Mapeia o nome da característica para o texto exibido no Tooltip (“guardião:exposto”, “móvel:imóvel”, etc.).

### 3.3 Guias/Ajuda (`config/ajuda.cfg`)
Textos exibidos no gump de ajuda (aba “Geral”, “Tratamento”, etc).

### 3.4 Itens empilháveis (`config/stackingitens.cfg`)
Define quais `objtype` podem ser empilhados no inventário (ex: setas, materiais, etc.).

### 3.5 Encaixes (`config/encaixes.cfg`)
Define tipos de encaixes usados pelos componentes (ex: cabo, lâmina curta, ferragens, etc).

### 3.6 Item Montado (`config/itemdesc.cfg`)
Define o item `itemMontado` (objtype 0xE72) que funciona como um “container” temporário durante a criação/montagem.

### 3.7 Informações sobre o fabricado (`*/FabricacaoDesc.cfg`)
Define os requisitos para fabricar a receita de certo item e os encaixes de cada item.

### 3.8 Componentes (`*/componentes.cfg`)
Define os requisitos para fabricar os componentes.

### 3.9 Itens de Menu (`*/*Menu.cfg`)
Define quais as receitas devem aparecer no gump de fabricação da profissão especifica.

### 4.0 Informações sobre o fabricado (`pkg/systems/recipes.cfg`)
Descrições das receitas existentes, é usado no Menu e na criação de pergaminhos instrução.

---

## 4) Como o sistema de qualidade funciona (resumo)

1. Cada componente tem um **tier** (MEDIOCRE, COMUM, NOTÁVEL, SUPERIOR, EXCEPCIONAL).
2. Quando um componente é criado, usa-se `DistribuiQualidadeComponentes()` para gerar tiers de forma probabilística (baseado em skill).
3. Os efeitos de material (`ComponenteEfeito`) são interpretados em `InterpretaMaterialEfeitos()` para aplicar valores reais aos atributos.
4. O esquema final (seleção de componentes + materiais + cores) gera um `propFabricado` que acompanha o item até a fase de tratamento.
5. No tratamento, a chance de sucesso/falha é calculada em `PegaDificuldade()` usando:
   - skill requerida pela receita
   - modificadores de componentes escolhidos
   - chance base calculada em `PegaChanceFabricacaoBase()`

---

## 5) Principais `PROPNAME_` usados no sistema (props)

- `PROPNAME_FABRICADO` : guarda os metadados do produto em construção (receita, esquema, componentes).
- `PROPNAME_FABRICADO_TRATAMENTO` : guarda estado de tratamento (sucessos/falhas/ciclos).
- `PROPNAME_MATERIAL` : material principal usado no item.
- `PROPNAME_ENCAIXE_COMPONENTE` / `PROPNAME_TIER_COMPONENTE` : usados nos componentes.
