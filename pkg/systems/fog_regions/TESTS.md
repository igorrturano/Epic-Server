# Plano de Testes — Sistema de Névoa Regional

Roteiro manual de validação. Use para regression testing após mudanças no sistema ou em código adjacente (packet handlers, gump framework, regions POL nativas).

## Pré-requisitos

1. Compilar arquivos do sistema:
```
ecompile pkg/commands/gm/fogregion.src
ecompile pkg/systems/fog_regions/logon.src
ecompile pkg/systems/fog_regions/start.src
ecompile pkg/systems/fog_regions/fogRegionWatcher.src
ecompile pkg/systems/fog_regions/fogRegionEffect.src
ecompile scripts/misc/logoff.src
```

Nota: `.inc` files (fogRegionCore.inc, fogRegionGump.inc) compilam junto com o `.src` que os usa.

2. Restart do server (start.src só roda no boot).
3. Login como GM (cmdlevel ≥ 3).
4. Cliente EpicUO atualizado com Camada B.

## Suite 1 — Smoke test

Validar que o comando abre e o menu responde.

1. Comando: `.fogregion`
2. **Esperado:** menu principal abre com 4 opções
3. Clicar "Onde estou agora"
4. **Esperado:** Tela 8 abre com X/Y/Z/Realm corretos da posição atual
5. Clicar "Fechar"
6. **Esperado:** gump fecha sem erros no log

## Suite 2 — Criar região com preset

Validar wizard de 3 passos + Tela 5a.

1. `.fogregion` → "Criar nova região"
2. **Esperado:** Tela 2 (passo 1/3)
3. Digitar nome `teste_preset`
4. Andar até X≈1001 Y≈661 (ou qualquer ponto)
5. Clicar "Marcar posição atual como NW"
6. **Esperado:** label em âmbar "NW: 1001, 661 [britannia_alt]"
7. Botão "Próximo >" habilita
8. Clicar → Tela 3 (passo 2/3)
9. Andar +50 +50 em X e Y
10. Clicar "Marcar posição atual como SE"
11. **Esperado:** "SE: ..." + cálculo de dimensão exibido
12. Clicar "Próximo >" → Tela 4
13. Clicar "Escolher preset pronto" → Tela 5a
14. Selecionar radio `noite_profunda`
15. Radio "Permanente" selecionado por default
16. Clicar "Criar"
17. **Esperado:** SysMessage `[FOG] Região 'teste_preset' criada.`

## Suite 3 — Watcher detecta região (CRÍTICO)

Validar coração funcional do sistema.

1. Andar para **dentro** do retângulo criado na Suite 2
2. **Esperado em até 2 segundos:** fog visível na tela com características de `noite_profunda` (escuro denso, cor cinza)
3. Andar para **fora** do retângulo
4. **Esperado em até 2 segundos:** fog desaparece
5. Andar para dentro de novo
6. **Esperado:** fog volta

Se esta suite falha, sistema não funciona. Investigar:
- Watcher rodando? Via `.props` no GM, verificar `#fogWatcherActive`
- Server log mostra erro em `SendFogPacketForRegion`?
- Cliente EpicUO está com Camada B? Build correto?

## Suite 4 — Editor manual

Validar Tela 5b (editor com 11 variáveis + duração).

1. `.fogregion` → "Criar nova região"
2. Completar wizard com nome `teste_manual` + NW/SE em local DIFERENTE da Suite 2 (sem overlap)
3. Tela 4 → "Configurar manualmente" → Tela 5b
4. **Esperado:** 13 linhas de variáveis + seção duração + 4 botões de rodapé
5. Clicar ▶ em `density` 3 vezes
6. **Esperado:** valor incrementa em steps de 0.001 escalado
7. Clicar [?] em `density`
8. **Esperado:** abre tooltip da Tela 6 com texto sobre densidade + analogia + aviso
9. Fechar tooltip → volta ao editor preservando state
10. Clicar "Testar"
11. **Esperado:** fog aparece NO GM com os valores atuais (preview, não persiste)
12. Mover GM, verificar que fog acompanha
13. Clicar "Criar"
14. **Esperado:** `[FOG] Região 'teste_manual' criada.` + fog de teste apaga
15. Andar para dentro da região
16. **Esperado:** fog reaparece (agora da região, não do teste)

## Suite 5 — Listar e ações

Validar Tela 7.

1. `.fogregion` → "Listar regiões"
2. **Esperado:** Tela 7 com 2 regiões (`teste_preset`, `teste_manual`)
3. Status visual:
   - Ambas em verde "Ativa permanente"
4. Clicar "Ir" em `teste_preset`
5. **Esperado:** GM teleporta para o centro do retângulo
6. Clicar "Parar" em `teste_preset`
7. **Esperado:** status muda para cinza "Parada"; fog desaparece da tela
8. Andar dentro/fora do retângulo parado
9. **Esperado:** fog NÃO aparece (região está parada)
10. Clicar "Ativar"
11. **Esperado:** status volta a "Ativa permanente"; fog reaparece quando dentro
12. Clicar "Del" em `teste_manual`
13. **Esperado:** gump de confirmação abre
14. Clicar "Excluir"
15. **Esperado:** região desaparece da lista
16. Verificar `pkg/systems/fog_regions/regions.cfg` — sem entrada `FogRegion teste_manual`

## Suite 6 — Editar região existente

Validar modo edit do editor manual.

1. `.fogregion` → "Listar regiões" → "Editar" em `teste_preset`
2. **Esperado:** editor abre com título "Editar Região — teste_preset"
3. **Esperado:** valores correspondem aos do preset `noite_profunda`
4. **Esperado:** botão "Criar" mostra label "Salvar"; botão "Anterior" mostra "Cancelar"
5. Modificar `density` (clicar ▶ algumas vezes)
6. Clicar "Salvar"
7. **Esperado:** `[FOG] Região 'teste_preset' atualizada.`
8. Sair e voltar ao retângulo (ou logout/login)
9. **Esperado:** fog reflete novo valor de density

## Suite 7 — Timer

Validar modo timed.

1. `.fogregion` → wizard → preset → seleciona radio "Por tempo"
2. Botões ◀▶ de minutos habilitam (eram disabled em "Permanente")
3. Definir `duracao = 1` minuto
4. Clicar "Criar"
5. `.fogregion` → "Listar regiões"
6. **Esperado:** região aparece com status âmbar "Ativa: 00:59" (ou similar)
7. Aguardar 60 segundos
8. Re-abrir lista
9. **Esperado:** status cinza "Parada"
10. Verificar `regions.cfg`: `Status = 0`

## Suite 8 — Edge cases

### 8.1 — Logout durante fog ativo
1. Criar região permanente, ficar dentro
2. Disconnect
3. Reconnect
4. **Esperado:** fog reaparece em até 2 segundos (watcher reinicia via logon hook)

### 8.2 — Edição em tempo real (LIMITAÇÃO CONHECIDA)
1. Criar região com `density` baixa, ficar dentro com fog ativo
2. Em outra sessão (multi-window se possível): abrir lista, "Editar", mudar `density` MUITO mais alto, "Salvar"
3. **Esperado documentado:** primeiro GM NÃO vê fog mudar imediatamente
4. Sair da região e voltar
5. **Esperado:** fog agora reflete novo valor

### 8.3 — Server reboot com timer ativo
1. Criar região timed de 5 minutos
2. Aguardar 1 minuto
3. Restart server
4. Login após boot
5. `.fogregion` → "Listar regiões"
6. **Esperado:** status âmbar "Ativa: 03:5x" (não resetou — start.src re-disparou effect script com sleep remanescente)
7. Aguardar até expirar
8. **Esperado:** expira automaticamente

### 8.4 — Server reboot com timer expirado durante offline
1. Criar região timed de 1 minuto
2. Restart server imediatamente
3. Aguardar 90 segundos após boot
4. Login
5. `.fogregion` → "Listar regiões"
6. **Esperado:** status cinza "Parada" — start.src detectou TimerEnd no passado e expirou direto sem disparar effect script

### 8.5 — Overlap de regiões
1. Criar 2 regiões com retângulos que se sobrepõem
2. Ficar na área de overlap
3. **Esperado:** fog de UMA das regiões aplicado (a primeira encontrada pelo `LoadRegions` foreach — não há merge de parâmetros)
4. Sair do overlap, entrar só na primeira
5. Reentrar overlap
6. **Esperado:** mesma região aplicada (determinístico)

### 8.6 — Validação de retângulo invertido
1. `.fogregion` → wizard → marcar NW em (1100, 700)
2. Marcar SE em (1000, 600) (cantos invertidos)
3. Clicar "Próximo >"
4. **Esperado:** erro em vermelho "NW deve ser menor que SE (canto superior esquerdo)."
5. Permanece na Tela 3 com state preservado

### 8.7 — Validação de realm cruzado
1. `.fogregion` → wizard → marcar NW
2. Andar até outro realm (gate, comando, etc)
3. Marcar SE
4. Clicar "Próximo >"
5. **Esperado:** erro em vermelho "NW e SE estão em realms diferentes."

### 8.8 — Override do `.fog` manual em região
1. Estar dentro de região permanente, fog ativo da região
2. Comando `.fog teste 1` (preset 1 do comando GM original — se ainda existe)
3. **Esperado:** fog muda para o teste
4. Sair da região e voltar
5. **Esperado:** fog da região reaplicado

## Suite 9 — Persistência

Validar regions.cfg como source of truth.

1. Verificar conteúdo de `pkg/systems/fog_regions/regions.cfg`
2. **Esperado:** formato `FogRegion <nome> { ... }` com 22 campos por entrada
3. Editar manualmente uma região no arquivo (mudar valor de `DensityInt`)
4. Restart server
5. **Esperado:** novo valor reflete no jogo quando player entra na região

## Bugs comuns a investigar se algo falhar

| Sintoma | Diagnóstico provável |
|---|---|
| `ecompile` falha | Cole stderr completo. Sintaxe POL (`is @error`?), include path incorreto, função inexistente |
| Tela não abre ao clicar `.fogregion` | Verificar log POL. Comando registrado em icp.cfg? .ecl existe? |
| Watcher não detecta região | `.props` no GM verifica `#fogWatcherActive`. Se ausente, logon hook não disparou |
| Fog não desaparece ao sair | SendFogOff não chamado. Verificar transição no watcher loop |
| Timer não expira | Effect script rodando? `.scripts` (se houver tool) ou log POL deve mostrar |
| Gump renderiza estranho | Cores ou IDs de botão errados. Cross-check com `convenção de cores e botões` no README |
| Botão "Criar" sempre desabilitado | `state.preset_selecionado` vazio. Radio button leitura via `_ReadPresetSelection` |
| Cor preview ████ sempre cinza | `_RgbToUoHue` não identifica dominância. Verificar branches da função |

## Histórico de validação

| Data | Versão | Suite passou | Notas |
|---|---|---|---|
| YYYY-MM-DD | 1.0 | 1-9 | Implementação inicial |
