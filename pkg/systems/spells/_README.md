### Visão Geral

O sistema de **spells** no POL é modular e configurável, composto por **configurações de spells** (`.cfg`), que definem parâmetros e chamam scripts, e **lógicas de spells**, localizadas em `spell_logic`. 

**Arquivos `.cfg**` devem ser criados **apenas** se a execução da magia for diferente. Caso contrário, magias que compartilham a mesma lógica devem estar no **mesmo arquivo `.cfg`**. A modularização principal acontece em `spell_logic`, dividida por classe ou tipo de magia.

### Estrutura de Configuração e Lógica de Spells

- **Configurações de Spells (config/)**:  
  Configurações agrupam magias com **lógicas comuns** no mesmo arquivo `.cfg`. Caso a execução varie, deve-se criar um arquivo específico.

  Exemplo: `shamanspells.cfg` lida com rituais de xamã, enquanto `priestspells.cfg` pode lidar com as mesmas magias, mas com lógica diferente.

- **Lógicas de Spells (spell_logic/)**:  
  A lógica de execução é dividida por classe, como `mage`, `druid`, etc. Scripts específicos são armazenados em subpastas para facilitar a manutenção.

### Quando Criar Novos Arquivos `.cfg`

Crie novos `.cfg` apenas quando as lógicas de execução forem diferentes. Caso contrário, agrupe-as no mesmo arquivo `.cfg`.

**Exemplo**:
- `shamanspells.cfg` lida com rituais de xamã. Se uma magia também for usada por `priest`, mas com lógica diferente, será necessário criar `priestspells.cfg`.

### Arquivos de Configuração Ativos

Atualmente, para a fase **Epic Shard**, os arquivos `.cfg` ativos são:

- `shamanspells.cfg`
- `priestspells.cfg`
- `spellcaster.cfg`
- `tricksterspells.cfg`

Todos os outros arquivos de configuração foram removidos ou desativados. Isso significa que algumas referências a configurações antigas podem ainda estar presentes no código, mas **não são mais utilizadas**.

Além disso, todas as magias agora estão documentadas no arquivo `allSpellsNotUsed.cfg`, que serve como um repositório comentado de magias para facilitar a busca.

### Nomeação de Arquivos e Variáveis

- **Clareza e consistência**:  
  Use nomes de arquivos e variáveis claros e duradouros, como `necromancySpells.cfg`. Evite termos obscuros ou inventados.

### Modularização em `spell_logic`

A modularização deve ocorrer em `spell_logic`, agrupando scripts por classe ou tipo de magia. Magias que compartilham lógicas podem usar o mesmo script.

**Exemplo de Arquivo `.cfg` Compartilhado**:
```c
Spell 1
{
    SpellId        1
    Name           "Criar comida"
    SpellScript    spells/spell_logic/mage/createfood
}

Spell 2
{
    SpellId        2
    Name           "Dardo Igneo"
    SpellScript    spells/spell_logic/mage/firebolt
}
```

### Classes com Magias Compartilhadas

Magias compartilhadas entre classes (como `shaman` e `priest`) devem ser separadas se houver **diferença de execução**. Se uma classe utiliza lógicas complexas (como rituais de `shaman`), cada classe terá sua configuração individual.

### Boas Práticas

1. **Mantenha o código limpo**:  
   Deixe o código mais simples e compreensível após cada modificação.

2. **Evite duplicação**:  
   Magias com lógicas semelhantes devem compartilhar scripts.

3. **Comente o código**:  
   Documente lógicas complexas com comentários úteis para facilitar futuras alterações.

### Conclusão

Novos arquivos `.cfg` devem ser criados apenas quando a lógica da magia exigir. Centralize magias que compartilham lógicas no mesmo arquivo e modularize a execução em `spell_logic`.