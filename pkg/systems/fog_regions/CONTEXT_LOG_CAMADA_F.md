# Camadas F + G — Fog Altura Z, Fade Temporal e Z Suave

Status em 2026-05-28. **Camadas F e G: COMPLETAS E TESTADAS.**

## Objetivo da Camada F

Evoluir o sistema de fog regional (Camada E, já funcionando)
com duas features independentes:

1. **Altura Z de mundo** — fog limitado por altitude. Permite
   bruma rasteira nos vales (z_max baixo) ou névoa pairando
   sobre telhados (z_min alto). Default -128/127 = sem limite
   (comportamento idêntico ao anterior).

2. **Fade temporal in/out** — ao entrar/sair de região, a
   densidade do fog cresce/desvanece suave em 1.5s com curva
   ease-in-out (smoothstep no tempo). Resolve o "switch seco"
   na borda. JÁ IMPLEMENTADO E FUNCIONANDO.

## Problema original que motivou

Fog era binário: fora do retângulo = zero fog; dentro = 100%
fog em 360° (inclusive atrás, onde 1s antes estava limpo).
Transição teleportativa. Investigação (IF6) revelou que o
shader é 100% screen-space, sem coords de mundo.

Decisão estratégica: a atenuação espacial HORIZONTAL (resolver
o 360°) foi ADIADA para uma Camada G futura, porque exige passar
world XY pro shader (difícil — só há 1 slot livre, TexCoord.z).
A altura Z foi priorizada por ser factível (Z já existe por
vértice no C#) e dramática visualmente.

## Dois repositórios

- **Epic-Server** (POL) — branch feat/nevoa
- **EpicUO** (cliente C#) — branch fog/camada-a-vertical-decay

## Decisões arquiteturais travadas

- Curva de fade: ease-in-out (smoothstep t*t*(3-2t)), 1.5s
- Fade só na densidade (cor/noise/dark/desat trocam direto)
- Fade dispara só ao entrar/sair de região (não pela faixa Z)
- Fade 100% client-side (GameScene), zero servidor/shader/packet
- Z com SMOOTH nas bordas, z_falloff CONFIGURÁVEL por região (C-G)
- Z range -128 a 127 (sbyte), step 5 no editor
- Z falloff range 1-60, step 5, default 10, label "z suave"
- Presets recebem default z_min=-128/z_max=127 (sem limite)
- Overlap de regiões: closest wins (decisão da Camada E mantida)
- Land stretched: Z do centro nos 4 cantos (simplificação
  aceita; erro <5 < falloff 10, imperceptível)
- Packet v4 (29 bytes) → v5 (33 bytes) → v6 (35 bytes)
- /fog debug = instantâneo (snap, sem fade)

## Packet 0x0070 v6 — layout (35 bytes)

Offsets 0-28 idênticos à v4. v5 adicionou:
- offset 29-30: WorldZMin (Int16 BE signed)
- offset 31-32: WorldZMax (Int16 BE signed)
v6 adicionou:
- offset 33-34: FalloffZ (Int16 BE não-signed, 1-60)

SendFogOff continua 29 bytes (cliente lê enabled=0 e para).
Backward compat: v5 sem offset 33 → cliente mantém default 10f.

Helper signed no servidor: _Fog_SetInt16BESigned — normaliza
negativo via (value + 65536) antes de serializar. Cliente lê
via (float)(short)p.ReadUInt16BE() (sign-extension automática).

## CHECKPOINTS — progresso

### ✅ C-F.1 — Fade temporal (COMPLETO, TESTADO)
EpicUO, GameScene.cs + PacketHandlers.cs + CommandManager.cs.
- 4 campos: _currentFogDensity, _fogTargetDensity,
  _fogStartDensity, _fogTransitionTime + const FOG_FADE_DURATION=1.5f
- BeginFogTransition(target) com guard idempotente (epsilon)
- SnapFogDensity(value) para o /fog debug (instantâneo)
- Bloco de interpolação smoothstep no Draw, ANTES do activeDensity
- activeDensity agora = _currentFogDensity (não mais Fog.Enabled)
- PacketHandler on: scene.Fog=fog + BeginFogTransition(density)
- PacketHandler off: só BeginFogTransition(0f), NÃO toca Enabled
- CommandManager: SnapFogDensity após scene.Fog=fog
Semântica nova: render olha _currentFogDensity>0, não Fog.Enabled.

### ✅ C-F.2 — Server core + packet v5 (COMPLETO, COMPILADO 0 erros)
Epic-Server, fogRegionCore.inc exclusivamente.
- _Fog_SetInt16BESigned (novo helper, byref packet)
- _ElemToStruct: +2 campos com fallback -128/127 via .errortext
  (padrão confirmado: GetProp de prop ausente retorna error obj)
- SaveRegion: +2 SetProp (WorldZMin, WorldZMax)
- SendFogPacketForRegion: size 29→33, +2 bytes signed offset 29/31
- SendFogTestPacket: idem, lê params.world_z_min/world_z_max
- SendFogOff: inalterada (29 bytes)

### ✅ C-F.3 — Gump editor (COMPLETO, COMPILADO, TESTADO)
Epic-Server, fogRegionGump.inc exclusivamente.
- Constantes: FR_BTN_ED_ZMIN/ZMAX_DEC/INC (81-84),
  HELP_ZMIN/ZMAX (85-86), FR_STEP_Z=5, FR_MIN_Z=-128, FR_MAX_Z=127
- GetEditorState: +world_z_min=-128, +world_z_max=127
- _ApplyEditorStep: 2 casos Z com _Step
- Handler principal: elseif próprio pra 81-84 (gate separado,
  range antigo 30-57 não cobre Z) + elseif pra help 85-86
- UI: 2 linhas (z min / z max) abaixo de desat, com ◀▶ e [?],
  display CStr (inteiro, pode ser negativo)
- _ButtonIdToVarName + _GetTooltipData: 2 tooltips de Z
- Validação z_min <= z_max no Criar (erro vermelho)
- _StateToRegionStruct: +WorldZMin/WorldZMax
- _PopulateEditorStateFromRegion (edit): +world_z_min/world_z_max
Controles aparecem na tela e respondem. Efeito visual ainda não
(shader é C-F.6).

### ✅ C-F.4 — Cliente data (COMPLETO, COMPILADO, TESTADO)
EpicUO, 4 arquivos (3 alterados, 1 adiado).
- FogState.cs: +WorldZMin/WorldZMax float, default -128f/127f
- PacketHandlers.cs: bloco v5 lê 2 Int16 signed via (short)cast
- BasicUOEffect.cs: +2 EffectParameter (FogWorldZMin/Max) + binds
- Batcher2D.SetFog: NÃO alterado — SetValue adiado pro C-F.6
DESCOBERTA CRÍTICA: Parameters["X"] de uniform inexistente
retorna null (FNA EffectParameterCollection.cs:48). SetValue em
null = NRE garantido. Por isso o bind é seguro mas o SetValue
espera o shader existir (C-F.6).

### ✅ C-F.5 — Cliente Batcher (COMPLETO, COMPILADO, TESTADO)
EpicUO, 8 arquivos: Batcher2D.cs, View.cs, LandView.cs,
StaticView.cs, ItemView.cs, MobileView.cs, MultiView.cs,
DragEffect.cs.
- float worldZ=0f adicionado a SetVertex, AddSprite, 7 Draw
  genéricos, DrawStretchedLand, DrawCharacterSitted (3 sub-quads),
  DrawSway, DrawShadow
- DrawStatic + DrawStaticAnimated em View.cs recebem worldZ e
  repassam para todas as chamadas internas
- 5 Views + DragEffect passam (float)this.Z (ou owner.Z no caso
  do DrawInternal estático de MobileView)
- Bloco Font/UI (Draw IFontStashRenderer, linhas 221-233)
  INTOCADO — Z=0 correto para UI
- Land stretched: Z do centro nos 4 cantos (aprovado)
GOTCHA: DrawInternal em MobileView é private static — usar
owner.Z, não this.Z (this não existe no escopo).

### ✅ C-F.6 — Cliente shader (COMPLETO, COMPILADO, TESTADO)
EpicUO, IsometricWorld.fx + Batcher2D.cs.
- 2 uniforms: FogWorldZMin/FogWorldZMax
- ZAttenuation(float tileZ): entre ComputeFogAmount e
  VertexShaderFunction. Z_FALLOFF=10.0 inline (const hlsl — depois
  substituída em C-G.2 pelo uniform FogWorldZFade).
- fogAmt *= ZAttenuation(IN.TexCoord.z) no PS após ComputeFogAmount,
  antes de qualquer branch — cobre TODOS os modos.
- Batcher2D.SetFog: SetValue ativados para FogWorldZMin/Max.
Build: 0 erros, 51 warnings pré-existentes.
NOTA: qualquer edição em IsometricWorld.fx exige recompilar o shader
antes do dotnet build. O fxc.exe está em src/ClassicUO.Renderer/shaders/
(NÃO é sistema, está no repo). Comando:
  cd src/ClassicUO.Renderer/shaders
  fxc.exe /T fx_2_0 /O3 /Fo IsometricWorld.fxc IsometricWorld.fx
(O compile_shaders.bat na mesma pasta também funciona, mas recompila
xBR junto — desnecessário se só o IsometricWorld foi tocado.)

### ✅ C-G.1 — Server FalloffZ + packet v6 (COMPLETO, COMPILADO, TESTADO)
Epic-Server, fogRegionCore.inc + fogRegionGump.inc.
- fogRegionCore: _ElemToStruct +FalloffZ (fallback 10), SaveRegion
  +SetProp, SendFogPacketForRegion/SendFogTestPacket → v6 (35 bytes),
  +_Fog_SetInt16BE(pkt, 33, FalloffZ/world_z_fade).
- fogRegionGump: constantes FR_BTN_ED_ZFADE_DEC/INC=87/88,
  HELP_ZFADE=89, FR_STEP_ZFADE=5, FR_MIN_ZFADE=1, FR_MAX_ZFADE=60.
  GetEditorState +world_z_fade=10, _ApplyEditorStep +ZFADE,
  UI "z suave" com ◀▶ e [?], handler own-elseif (87-88),
  help gate +89, tooltips, _StateToRegionStruct, _PopulateEditorState.
Build servidor: 2020 scripts up-to-date, 0 erros.

### ✅ C-G.2 — Cliente FalloffZ (COMPLETO, COMPILADO, TESTADO)
EpicUO, 5 arquivos.
- FogState.cs: +WorldZFade float, default 10f.
- PacketHandlers.cs: bloco v6 lê (float)p.ReadUInt16BE() não-signed,
  backward compat (v5 sem byte → mantém 10f).
- BasicUOEffect.cs: +FogWorldZFade EffectParameter + bind.
- Batcher2D.SetFog: +FogWorldZFade.SetValue(fog.WorldZFade).
- IsometricWorld.fx: +uniform FogWorldZFade, ZAttenuation substituída
  — remove const Z_FALLOFF, usa max(FogWorldZFade, 1.0) com clamp.
- IsometricWorld.fxc: recompilado com fxc.exe /T fx_2_0 /O3 (sucesso).
Build: 0 erros, 51 warnings pré-existentes, 0 novos.

## Convenções do shard (referência)

- Botões gump: 2103/2104 navegação larga, 2117/2118 ◀▶ compacto,
  22153 disabled (GF_DUMMY_BTN)
- Cores: 53 âmbar título, 1153 cyan desc, 90 body, 33 vermelho
  erro, 68 verde, 230 cinza muted
- Comentários // FOG-FASE5/F: marcam código da Camada F
- IDs gump fog editor: 30-57 step das 11 vars, 60-61 testar/criar,
  70-80 help original, 81-86 Z min/max, 87-89 Z suave (Camada G)

## Camada G — Z Suave (z_falloff configurável)

Status: COMPLETA E TESTADA.

Motivação: o corte de altura da Camada F criava uma linha reta
dura nas construções altas (prédios), porque o Z é por-sprite
(cada sprite tem Z único, não gradiente por pixel). Solução:
expor o z_falloff (antes constante fixa 10.0 no shader) como
parâmetro por região, controlável pelo GM. Permite escolher
entre corte seco ("teto de névoa" definido) e degradê difuso.

Packet v5 (33 bytes) → v6 (35 bytes): +1 Int16 NÃO-signed
(FalloffZ no offset 33-34, range 1-60, default 10).

### C-G.1 — Servidor (COMPLETO, COMPILADO, TESTADO)
fogRegionCore.inc + fogRegionGump.inc.
- _ElemToStruct: +FalloffZ com fallback 10 via .errortext
- SaveRegion: +SetProp FalloffZ
- SendFogPacketForRegion + SendFogTestPacket: size 33→35,
  +offset 33 via _Fog_SetInt16BE (não-signed)
- Constantes gump: FR_BTN_ED_ZFADE_DEC=87, ZFADE_INC=88,
  HELP_ZFADE=89, FR_STEP_ZFADE=5, FR_MIN_ZFADE=1, FR_MAX_ZFADE=60
- GetEditorState: +world_z_fade=10
- _ApplyEditorStep: +caso z_fade
- Handler step: elseif PRÓPRIO (87||88), NÃO estende gate 81-84
- Handler help: +HELP_ZFADE no gate OR
- _ButtonIdToVarName + _GetTooltipData: tooltip "z suave"
- UI: linha "z suave" abaixo de z max
- _StateToRegionStruct: +FalloffZ
- _PopulateEditorStateFromRegion: +world_z_fade
- Cadeia de nomes: params.world_z_fade = state.world_z_fade =
  region.FalloffZ = cfg FalloffZ

### C-G.2 — Cliente + shader (COMPLETO, COMPILADO, TESTADO)
- FogState.cs: +WorldZFade float, default 10f
- PacketHandlers.cs: bloco v6, (float)ReadUInt16BE() não-signed
- BasicUOEffect.cs: +FogWorldZFade EffectParameter + bind
  (seguro bindar+usar de uma vez: shader declara o uniform no
  mesmo checkpoint e fxc recompila antes do build)
- Batcher2D.SetFog: +SetValue FogWorldZFade
- IsometricWorld.fx: +uniform FogWorldZFade; ZAttenuation
  usa max(FogWorldZFade, 1.0) em vez da const Z_FALLOFF=10.0
  (removida); clamp evita smoothstep degenerado
- Recompilado via fxc.exe + dotnet build --no-incremental

### Efeito por valor de z suave
- 1: corte quase seco (teto de névoa definido)
- 10: comportamento padrão (= constante antiga, retrocompat)
- 30: degradê suave (mata a linha reta nos prédios)
- 60: transição super difusa

### Limitação remanescente (futura Camada H, se desejado)
O Z é por-sprite, não por-pixel-de-altura. O z suave ameniza
a linha reta nas construções mas não a elimina conceitualmente
— sprites grandes verticais ainda têm Z único. Resolver de vez
exigiria gradiente de altura por pixel dentro do sprite (mais
complexo). Aceito como está; z suave alto resolve na prática.

### Limitação ADIADA da Camada F (atenuação espacial horizontal)
O fog ainda é screen-space na horizontal (o problema 360°
original). Foi conscientemente ADIADO. Resolver exigiria passar
world XY pro shader (só há 1 slot livre TexCoord.z, já usado
pelo Z). Seria a "Camada G horizontal" original — virou candidata
a trabalho futuro maior.

## Padrão de trabalho (importante)

Claude Code tende a violar Regra 3 (descrever código em vez de
colar) e a simplificar escopo por conta própria (já cortou
tooltips, mudou step, omitiu validação no C-F.3 — pego na
auditoria). SEMPRE exigir código colado literal e revisar
cada resumo contra o que foi aprovado.

Workflow validado: investigação prévia (cola código-molde) →
proposta auditada → implementação com código real → auditoria
→ compile → teste in-game → próximo checkpoint.
