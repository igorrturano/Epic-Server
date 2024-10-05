# Guia de Organização e Atualização de Habilidades

Este documento fornece diretrizes para desenvolvedores sobre como organizar e atualizar as habilidades (`habilities`) no projeto. 

## Estrutura de Pastas

Para manter a organização e facilitar o acesso às habilidades, todas as habilidades devem ser armazenadas em suas respectivas pastas de classe. A estrutura recomendada é a seguinte:

```
./charactercreation/
│
├── config/
│   └── habilidades.cfg
│
└── habilidades/
    ├── sabio/
    │   ├── controlarbestas.ecl
    │   └── controlarbestas.src
    ├── guerreiro/
    │   ├── tirocarregado.ecl
    │   └── tirocarregado.src
    ├── ladino/
    │   ├── spellthief.ecl
    │   └── spellthief.src
    ├── artifice/
    │   ├── canalizarFerreiro.ecl
    │   └── canalizarFerreiro.src
    └── ... (outras classes)
```

## Adicionando ou Atualizando Habilidades

Quando você criar ou modificar uma habilidade, siga os passos abaixo:

1. **Criar os Arquivos `.ecl` e `.src`:**
   
   - **Localização:** Coloque os arquivos nas pastas correspondentes à classe da habilidade.
   - **Exemplo:**
     ```
     habilidades/sabio/controlarbestas.ecl
     habilidades/sabio/controlarbestas.src
     ```

2. **Atualizar o Arquivo `habilidades.cfg`:**
   
   - **Caminho:** `./charactercreation/config/habilidades.cfg`
   - **Formato:** Adicione ou atualize a definição da habilidade com o caminho do script correto.
   - **Exemplo:**
     ```cfg
     hab Controlar Besta
     {
         nome Controlar Besta
         classe Sabio 4
         uso Ativa
         duracao 1 uso
         script :charactercreation:habilidades/sabio/controlarbestas
         delay 180
         stam 25
         desc Permite controlar uma besta selvagem, aumentando suas habilidades em combate.
         requisito Treinamento Druídico
     }
     ```

## Exemplo Prático

### Adicionando a Habilidade "Controlar Besta"

1. **Criar os Arquivos:**
   
   - `habilidades/sabio/controlarbestas.ecl`
   - `habilidades/sabio/controlarbestas.src`

2. **Atualizar `habilidades.cfg`:**
   
   ```cfg
   hab Controlar Besta
   {
       nome Controlar Besta
       classe Sabio 4
       uso Ativa
       duracao 1 uso
       script :charactercreation:habilidades/sabio/controlarbestas
       delay 180
       stam 25
       desc Permite controlar uma besta selvagem, aumentando suas habilidades em combate.
       requisito Treinamento Druídico
   }
   ```

## Subclasses com Habilidades Portadas

Atualmente, todas as habilidades da classe **Sabio** relacionadas à **Druida** já foram portadas seguindo esta estrutura. Isso inclui habilidades como:

- **Guardião do Bosque**
- **Treinamento Druídico**
- **Instinto de Sobrevivência**
- **Natureza Selvagem**
- **Controlar Besta**

### Verificação

Você pode verificar as habilidades portadas navegando pelas pastas de classe correspondentes e conferindo o arquivo `habilidades.cfg` para garantir que todas as referências estão corretas.

## Boas Práticas

- **Consistência nos Nomes:** Utilize nomes de arquivos consistentes e representativos para facilitar a identificação.
- **Comentários Claros:** Adicione comentários nos arquivos `.cfg` para explicar funcionalidades complexas.
- **Testes:** Sempre teste as habilidades após adicionar ou modificar para garantir que funcionem conforme o esperado.
- **Documentação:** Mantenha a documentação atualizada com todas as mudanças realizadas nas habilidades.

## Contribuindo

Se você identificar a necessidade de adicionar novas classes ou subclasses, siga a mesma estrutura de pastas e atualize o `habilidades.cfg` adequadamente. Mantenha a comunicação com a equipe para garantir que todos estejam alinhados com as mudanças realizadas.

---

**Notas:**

- Este guia visa melhorar a organização e facilitar a manutenção das habilidades no projeto.
```