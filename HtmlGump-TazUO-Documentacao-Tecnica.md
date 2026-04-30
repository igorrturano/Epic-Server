# HtmlGump System for TazUO

## Sistema declarativo de UI baseado em HTML/CSS para o Epic Server

**Versão:** 1.0 — Especificação Técnica
**Stack:** TazUO (C# / FNA / .NET 8) + AngleSharp + SkiaSharp
**Autor:** Documentação técnica para implementação no fork TazUO do Epic Server
**Data:** 2026

---

## Sumário

1. [Visão geral e objetivos](#1-visão-geral-e-objetivos)
2. [Arquitetura geral](#2-arquitetura-geral)
3. [Stack de dependências](#3-stack-de-dependências)
4. [Estrutura de arquivos no TazUO](#4-estrutura-de-arquivos-no-tazuo)
5. [Pipeline de renderização](#5-pipeline-de-renderização)
6. [Fase 1 — Fundação: parsing e DOM](#6-fase-1--fundação-parsing-e-dom)
7. [Fase 2 — Layout engine (flexbox)](#7-fase-2--layout-engine-flexbox)
8. [Fase 3 — Style engine (CSS)](#8-fase-3--style-engine-css)
9. [Fase 4 — Render layer](#9-fase-4--render-layer)
10. [Fase 5 — Estados e interatividade](#10-fase-5--estados-e-interatividade)
11. [Fase 6 — Animations e transitions](#11-fase-6--animations-e-transitions)
12. [Fase 7 — Efeitos visuais ricos (shaders)](#12-fase-7--efeitos-visuais-ricos-shaders)
13. [Fase 8 — SVG inline (SkiaSharp)](#13-fase-8--svg-inline-skiasharp)
14. [Fase 9 — Backdrop-filter (vidro fosco)](#14-fase-9--backdrop-filter-vidro-fosco)
15. [Fase 10 — Hot reload e theming](#15-fase-10--hot-reload-e-theming)
16. [Integração com servidor (POL/ServUO/ModernUO)](#16-integração-com-servidor-polservuomodernuo)
17. [Schema completo de elementos suportados](#17-schema-completo-de-elementos-suportados)
18. [Schema completo de propriedades CSS](#18-schema-completo-de-propriedades-css)
19. [Performance e otimizações](#19-performance-e-otimizações)
20. [Roadmap e cronograma](#20-roadmap-e-cronograma)
21. [Apêndices](#21-apêndices)

---

## 1. Visão geral e objetivos

### 1.1 Problema

O sistema atual de gumps no TazUO (herdado do ClassicUO) é imperativo, com coordenadas absolutas e estilização limitada via tilesets de arte. Cada gump exige código C# explícito ou XML estático. Não há separação entre estrutura, estilo e comportamento. Não há temas, hot reload, animações declarativas, layout responsivo, nem estados visuais (`:hover`, `:active`, `:focus`).

### 1.2 Solução

Implementar um subsistema de UI declarativo dentro do TazUO que aceita arquivos HTML e CSS reais, parseia-os via **AngleSharp**, calcula layout via engine flexbox próprio, aplica estilos via cascata CSS própria, e renderiza usando o pipeline FNA existente do TazUO. Efeitos visuais ricos (drop-shadow, glow, transform 3D, vidro fosco) são implementados como shaders FNA. SVG inline é renderizado via **SkiaSharp**.

### 1.3 Princípios de design

1. **Coexistência sem ruptura:** o sistema novo coexiste com o sistema clássico de gumps. Gumps existentes continuam funcionando. A migração é gradual.
2. **C# nativo:** zero dependências de runtime nativo pesado (sem Ultralight, CEF, WebView2). AngleSharp e SkiaSharp são libs .NET puras (SkiaSharp tem binários nativos pequenos).
3. **Renderização via FNA:** todo desenho final passa pelo `SpriteBatch` e shaders FNA do TazUO. Sem alternar contextos de renderização.
4. **Hot reload first:** todo o sistema é projetado para que arquivos `.html` e `.css` sejam editáveis com o cliente aberto e recarregados automaticamente.
5. **Subset focado:** implementamos o que importa para UI de jogo — não buscamos paridade com Chrome.

### 1.4 Escopo do subset suportado

| Categoria | Suportado | Não suportado |
|---|---|---|
| **Layout** | flexbox completo, padding, margin, gap, position absolute/relative | grid avançado, float, multi-column |
| **Tipografia** | font-family, size, weight, style, color, line-height, letter-spacing, text-align, text-shadow | text-decoration complexo, vertical writing modes |
| **Cores e fundos** | hex, rgb, rgba, hsl, gradients (linear/radial), background-image | conic-gradient, multiple backgrounds |
| **Bordas** | border (width/style/color), border-radius, box-shadow (incl. inset) | border-image |
| **Filtros** | filter (blur, drop-shadow, brightness, saturate, hue-rotate), backdrop-filter (blur) | filtros SVG complexos |
| **Transforms** | translate, rotate, scale, skew, rotateX/Y/Z (3D), perspective | matrix3d completa |
| **Animações** | transitions, @keyframes, animation | scroll-driven animations |
| **Pseudo-classes** | :hover, :active, :focus, :disabled, :first-child, :last-child, :nth-child(simples) | :has(), :is() complexo, :where() |
| **SVG** | path, circle, rect, line, gradients, animate, animateTransform | filters SVG, foreignObject |
| **JavaScript** | nenhum (lógica fica em C#) | ECMAScript completo |

### 1.5 Métricas de sucesso

- Hot reload de qualquer arquivo `.html` ou `.css` em < 200ms
- Layout de gump com 50 elementos em < 5ms
- Render por frame de 20 gumps abertos simultâneos com < 2ms total adicional ao frame budget
- Migração de pelo menos 5 gumps clássicos do TazUO (status, journal, paperdoll, party, hotbar) para o novo sistema sem perda de funcionalidade

---

## 2. Arquitetura geral

### 2.1 Diagrama de alto nível

```
┌──────────────────────────────────────────────────────────────────┐
│                        TazUO Client (FNA)                         │
│                                                                    │
│  ┌────────────────────┐       ┌──────────────────────────┐       │
│  │  Classic Gumps     │       │  HtmlGump Subsystem      │       │
│  │  (legacy system)   │       │                          │       │
│  └────────────────────┘       │  ┌────────────────────┐ │       │
│                                │  │  HtmlGump : Gump   │ │       │
│                                │  │  ----------------  │ │       │
│                                │  │  - LoadHtml(path)  │ │       │
│                                │  │  - LoadCss(path)   │ │       │
│                                │  │  - DataContext     │ │       │
│                                │  └─────────┬──────────┘ │       │
│                                │            │             │       │
│                                │  ┌─────────▼──────────┐ │       │
│                                │  │ HtmlDocumentParser │ │       │
│                                │  │   (AngleSharp)     │ │       │
│                                │  └─────────┬──────────┘ │       │
│                                │            │             │       │
│                                │  ┌─────────▼──────────┐ │       │
│                                │  │   StyleResolver    │ │       │
│                                │  │  (CSS cascade)     │ │       │
│                                │  └─────────┬──────────┘ │       │
│                                │            │             │       │
│                                │  ┌─────────▼──────────┐ │       │
│                                │  │    LayoutEngine    │ │       │
│                                │  │  (flexbox solver)  │ │       │
│                                │  └─────────┬──────────┘ │       │
│                                │            │             │       │
│                                │  ┌─────────▼──────────┐ │       │
│                                │  │  AnimationDriver   │ │       │
│                                │  │ (transitions/keyf) │ │       │
│                                │  └─────────┬──────────┘ │       │
│                                │            │             │       │
│                                │  ┌─────────▼──────────┐ │       │
│                                │  │    Renderer        │ │       │
│                                │  │  (FNA SpriteBatch  │ │       │
│                                │  │   + shaders + Skia)│ │       │
│                                │  └─────────┬──────────┘ │       │
│                                └────────────┼─────────────┘       │
│                                             │                      │
│  ┌──────────────────────────────────────────▼─────────────────┐  │
│  │                    FNA GraphicsDevice                       │  │
│  │  (textures, shaders, render targets, sprite batch)         │  │
│  └─────────────────────────────────────────────────────────────┘  │
└────────────────────────────────────────────────────────────────────┘
```

### 2.2 Camadas

O sistema é dividido em 7 camadas conceituais:

| Camada | Responsabilidade | Componentes principais |
|---|---|---|
| **1. Loading** | Carregar arquivos `.html` e `.css` do disco, resolver `<link>` e `@import`, gerenciar hot reload | `HtmlGumpLoader`, `FileSystemWatcher`, `ResourceCache` |
| **2. Parsing** | Converter texto HTML em DOM e CSS em CSSOM | `AngleSharp.HtmlParser`, `AngleSharp.Css.CssParser` |
| **3. Style cascade** | Aplicar regras CSS aos nós DOM, resolver herança, especificidade, `!important`, variáveis | `StyleResolver`, `ComputedStyle` |
| **4. Layout** | Calcular bounding boxes finais (X, Y, Width, Height) de cada nó | `FlexLayoutEngine`, `BoxModel` |
| **5. Animation** | Avançar transitions e keyframes a cada frame, interpolar valores | `AnimationDriver`, `Tween` |
| **6. Render** | Desenhar elementos via SpriteBatch + shaders, compor camadas, render targets para efeitos | `HtmlRenderer`, `EffectsPipeline`, `SkiaRenderer` |
| **7. Input** | Rotear eventos de mouse/teclado para nós, disparar callbacks, atualizar pseudo-classes | `InputRouter`, `EventBubbler` |

### 2.3 Modelo conceitual: do arquivo ao pixel

```
hotbar.html ─┐
             ├──> AngleSharp.Parse() ──> DOM tree
hotbar.css ──┘
                                          │
                                          ▼
                              ┌─ StyleResolver ─┐
theme.css ───────────────────►│ cascade + spec. │
                              └─────────┬───────┘
                                        │
                                        ▼
                                 ComputedStyle por nó
                                        │
                                        ▼
                              ┌─ FlexLayoutEngine ─┐
viewport size ───────────────►│   solve geometry   │
                              └─────────┬──────────┘
                                        │
                                        ▼
                              LayoutBox por nó (X,Y,W,H)
                                        │
                                        ▼
                              ┌─ AnimationDriver ──┐
delta time ──────────────────►│  apply tweens      │
                              └─────────┬──────────┘
                                        │
                                        ▼
                                 RenderState final
                                        │
                                        ▼
                              ┌─ HtmlRenderer ─────┐
FNA SpriteBatch ◄─────────────┤  draw + effects    │
                              └────────────────────┘
                                        │
                                        ▼
                                  Frame buffer
```

---

## 3. Stack de dependências

### 3.1 Pacotes NuGet a adicionar

```xml
<!-- src/ClassicUO.Client/ClassicUO.Client.csproj -->
<ItemGroup>
  <!-- HTML5 + CSS3 parser, retorna DOM e CSSOM completos -->
  <PackageReference Include="AngleSharp" Version="1.1.2" />
  <PackageReference Include="AngleSharp.Css" Version="1.0.0-beta.139" />
  
  <!-- SVG rendering vetorial via Skia -->
  <PackageReference Include="SkiaSharp" Version="2.88.8" />
  <PackageReference Include="SkiaSharp.NativeAssets.Linux" Version="2.88.8" />
  <PackageReference Include="SkiaSharp.NativeAssets.macOS" Version="2.88.8" />
  <PackageReference Include="SkiaSharp.NativeAssets.Win32" Version="2.88.8" />
  
  <!-- File watching já existe no .NET, mas verifique se TazUO já tem -->
</ItemGroup>
```

**Tamanho aproximado adicionado ao instalador:**
- AngleSharp + AngleSharp.Css: ~2.5 MB
- SkiaSharp + binários nativos por plataforma: ~10-15 MB por plataforma
- **Total estimado: ~15 MB no instalador**

### 3.2 Por que AngleSharp

**AngleSharp** é um parser HTML5/CSS3 .NET puro, MIT-licensed, mantido ativamente, usado em produção por inúmeros projetos. Ele dá:

- DOM completo e navegável (`IDocument`, `IElement`, `INode`)
- CSSOM completo (`ICssStyleSheet`, `ICssStyleRule`, `ICssStyleDeclaration`)
- Selectors API (`document.QuerySelectorAll(".slot.ready")`)
- Mutation events (útil para reagir a mudanças no DOM via JavaScript futuro ou data binding)

**O que AngleSharp não faz:** layout, render, animação, input. Tudo isso é responsabilidade do nosso código.

### 3.3 Por que SkiaSharp

SkiaSharp é o binding .NET do Skia (mesmo motor gráfico do Chrome, Android, Flutter). Usado especificamente para:

- Renderizar SVG inline (paths, gradients, animações vetoriais)
- Tessellation de bezier curves
- Geração de texturas para efeitos especiais

**Não usamos Skia para o resto do render** — o resto vai pelo SpriteBatch FNA nativo. Skia é chamado pontualmente, gera uma `Texture2D`, e devolve.

---

## 4. Estrutura de arquivos no TazUO

### 4.1 Layout de pastas no fork

```
TazUO/
├── src/
│   └── ClassicUO.Client/
│       └── Game/
│           └── UI/
│               ├── Gumps/                    (gumps clássicos existentes)
│               └── HtmlGumps/                  ★ NOVO
│                   ├── HtmlGump.cs              (classe principal)
│                   ├── HtmlGumpLoader.cs        (loading + hot reload)
│                   ├── DOM/
│                   │   ├── HtmlNode.cs          (wrapper sobre IElement)
│                   │   └── DataBinding.cs       (binding {{var}})
│                   ├── Style/
│                   │   ├── StyleResolver.cs     (cascade)
│                   │   ├── ComputedStyle.cs     (estilo final por nó)
│                   │   ├── CssValueParser.cs    (parsers de valor)
│                   │   ├── Selectors.cs         (matching de seletores)
│                   │   └── ThemeManager.cs      (variáveis CSS / temas)
│                   ├── Layout/
│                   │   ├── FlexLayoutEngine.cs  (solver flexbox)
│                   │   ├── LayoutBox.cs         (bounding box)
│                   │   ├── BoxModel.cs          (margin/border/padding/content)
│                   │   └── TextLayout.cs        (medição e quebra de linha)
│                   ├── Animation/
│                   │   ├── AnimationDriver.cs   (loop principal)
│                   │   ├── Transition.cs        (CSS transitions)
│                   │   ├── Keyframes.cs         (CSS @keyframes)
│                   │   ├── Tween.cs             (interpolação)
│                   │   └── Easing.cs            (cubic-bezier, ease-in-out, etc.)
│                   ├── Render/
│                   │   ├── HtmlRenderer.cs      (orquestrador)
│                   │   ├── BoxRenderer.cs       (background, border, shadow)
│                   │   ├── TextRenderer.cs      (texto + text-shadow)
│                   │   ├── ImageRenderer.cs     (background-image, <img>)
│                   │   ├── EffectsPipeline.cs   (shaders FNA)
│                   │   ├── SkiaRenderer.cs      (SVG via Skia)
│                   │   └── BackdropFilter.cs    (vidro fosco)
│                   ├── Input/
│                   │   ├── InputRouter.cs       (routing de eventos)
│                   │   ├── EventBubbler.cs      (event propagation)
│                   │   └── HitTester.cs         (point-in-box)
│                   └── Shaders/
│                       ├── DropShadow.fx        (efeito drop-shadow)
│                       ├── GaussianBlur.fx      (blur para backdrop)
│                       ├── Glow.fx              (glow effect)
│                       └── Transform3D.fx       (transform 3D matrix)
│
└── Data/
    ├── HtmlGumps/                              ★ NOVO (conteúdo)
    │   ├── hotbar.html
    │   ├── party.html
    │   ├── status.html
    │   ├── tooltip.html
    │   ├── deathscreen.html
    │   └── ...
    ├── Themes/                                 ★ NOVO
    │   ├── default.css
    │   ├── valtheris.css
    │   ├── polski.css
    │   └── _base.css                            (variáveis comuns, reset)
    ├── HtmlAssets/                              ★ NOVO
    │   ├── icons/
    │   │   ├── fireball.png
    │   │   ├── heal.png
    │   │   └── ...
    │   ├── fonts/
    │   │   ├── Cinzel-Bold.ttf
    │   │   ├── Cormorant.ttf
    │   │   └── ...
    │   └── backgrounds/
    │       ├── parchment.png
    │       └── ...
    └── XmlGumps/                                (sistema XML existente coexiste)
```

### 4.2 Convenção de nomes

- Arquivos HTML: `kebab-case.html` (ex: `death-screen.html`, `xp-bar.html`)
- Arquivos CSS: `kebab-case.css` ou `_base.css` para parciais
- IDs em CSS: `#gump-hotbar` (prefixo `gump-` para evitar colisão entre gumps)
- Classes em CSS: `.slot`, `.slot--ready`, `.slot__icon` (BEM opcional)
- Variáveis CSS: `--ep-primary`, `--ep-gold` (prefixo `--ep-` para Epic Server)

### 4.3 Estrutura mínima de um arquivo `.html`

```html
<!-- Data/HtmlGumps/hotbar.html -->
<gump 
    id="hotbar" 
    width="auto" 
    height="auto"
    movable="true"
    closable="false"
    theme="{{currentTheme}}">
    
    <link rel="stylesheet" href="../Themes/_base.css"/>
    <link rel="stylesheet" href="hotbar.css"/>
    
    <div class="hotbar">
        <button class="slot" data-key="1" data-skill="fireball">
            <img class="slot__icon" src="../HtmlAssets/icons/fireball.png"/>
            <span class="slot__key">1</span>
            <div class="slot__cooldown" style="--cd-progress: {{skill1.cooldown}}"></div>
        </button>
        <!-- mais slots... -->
    </div>
</gump>
```

Note três extensões nossas ao HTML padrão:

1. **`<gump>`** como root element no lugar de `<html>` — define metadata da gump (movable, closable, etc.)
2. **`{{var}}`** para data binding com C# (resolvido no momento de render, sem JS)
3. **`<link rel="stylesheet">`** funciona como em web normal

---

## 5. Pipeline de renderização

### 5.1 Loop principal por frame

A cada frame do TazUO, para cada `HtmlGump` aberto, executamos:

```csharp
// HtmlGump.Update() - chamado a cada frame pelo TazUO
public override void Update()
{
    base.Update();
    
    float deltaSeconds = Time.Delta;
    
    // 1. Hot reload check (apenas em modo dev)
    if (_loader.HasChanges())
    {
        ReloadDocument();
    }
    
    // 2. Atualizar data bindings
    if (_dataContext.IsDirty)
    {
        ApplyDataBindings();
        _styleResolver.MarkDirty();
    }
    
    // 3. Avançar animações e transitions
    _animationDriver.Tick(deltaSeconds);
    
    // 4. Recalcular estilo se algo mudou
    if (_styleResolver.IsDirty || _animationDriver.HasActiveAnimations)
    {
        _styleResolver.ResolveAll(_dom.Root);
    }
    
    // 5. Recalcular layout se geometria afetada
    if (_layoutEngine.IsDirty)
    {
        _layoutEngine.Layout(_dom.Root, ViewportWidth, ViewportHeight);
    }
}

// HtmlGump.Draw() - chamado a cada frame
public override bool Draw(UltimaBatcher2D batcher, int x, int y)
{
    if (!base.Draw(batcher, x, y)) return false;
    
    _renderer.Render(_dom.Root, batcher, x, y);
    
    return true;
}
```

### 5.2 Stages do render

Para cada nó visível do DOM, em ordem de árvore (depth-first, in-order):

```
┌─────────────────────────────────────────────────────────┐
│ Para cada nó visível:                                    │
│                                                          │
│   1. ┌─ Backdrop-filter ─┐                              │
│      │ Render gameworld   │  (se backdrop-filter exists)│
│      │ behind to RT, blur │                              │
│      └────────────────────┘                              │
│                                                          │
│   2. ┌─ Box shadow ──────┐                              │
│      │ Draw outset shadow │  (antes do background)      │
│      └────────────────────┘                              │
│                                                          │
│   3. ┌─ Background ──────┐                              │
│      │ Color, gradient,   │                              │
│      │ image              │                              │
│      └────────────────────┘                              │
│                                                          │
│   4. ┌─ Inner shadow ────┐                              │
│      │ inset box-shadow   │                              │
│      └────────────────────┘                              │
│                                                          │
│   5. ┌─ Border ──────────┐                              │
│      │ Width, color,      │                              │
│      │ border-radius      │                              │
│      └────────────────────┘                              │
│                                                          │
│   6. ┌─ Content ─────────┐                              │
│      │ Texto, imagens,    │                              │
│      │ SVG, filhos        │  (recursivo)                 │
│      └────────────────────┘                              │
│                                                          │
│   7. ┌─ Filter ──────────┐                              │
│      │ blur, drop-shadow, │  (aplicado no resultado)    │
│      │ brightness         │                              │
│      └────────────────────┘                              │
│                                                          │
│   8. ┌─ Transform ───────┐                              │
│      │ translate, rotate, │  (aplicado no SpriteBatch)  │
│      │ scale, 3D          │                              │
│      └────────────────────┘                              │
└─────────────────────────────────────────────────────────┘
```

### 5.3 Render targets e composição

Para efeitos como `filter` e `backdrop-filter`, precisamos de **render targets** intermediários. O TazUO/FNA suporta isso nativamente via `RenderTarget2D`.

```csharp
// Pool de render targets compartilhados, reaproveitados por frame
public class RenderTargetPool
{
    private Stack<RenderTarget2D> _available = new();
    private List<RenderTarget2D> _inUse = new();
    
    public RenderTarget2D Rent(int width, int height)
    {
        // ... reutiliza ou cria novo
    }
    
    public void ReturnAll()
    {
        // chamado no fim do frame
    }
}
```

**Estratégia:** evitamos criar/destruir RT a cada frame (caro). Usamos um pool com tamanhos comuns (ex: 256x256, 512x512, 1024x1024) e recortamos a região relevante.

---

## 6. Fase 1 — Fundação: parsing e DOM

### 6.1 Objetivo da fase

Construir a base que carrega arquivos HTML, parseia via AngleSharp, expõe a árvore DOM, e permite mapear cada elemento para uma `LayoutBox` que será preenchida nas fases seguintes.

**Esforço estimado:** 5-8 dias de trabalho focado.
**Dependência:** AngleSharp instalado.

### 6.2 Classe `HtmlGump`

```csharp
// src/ClassicUO.Client/Game/UI/HtmlGumps/HtmlGump.cs
using AngleSharp;
using AngleSharp.Dom;
using AngleSharp.Html.Parser;
using ClassicUO.Game.UI.Gumps;
using ClassicUO.Game.UI.HtmlGumps.Layout;
using ClassicUO.Game.UI.HtmlGumps.Render;
using ClassicUO.Game.UI.HtmlGumps.Style;

namespace ClassicUO.Game.UI.HtmlGumps;

/// <summary>
/// Um Gump cujo conteúdo é descrito por um arquivo .html + .css.
/// Coexiste com o sistema clássico de gumps do TazUO.
/// </summary>
public class HtmlGump : Gump
{
    // ===== Documento =====
    private IDocument _document;
    private HtmlNode _rootNode;
    
    // ===== Subsistemas =====
    private readonly StyleResolver _styleResolver;
    private readonly FlexLayoutEngine _layoutEngine;
    private readonly AnimationDriver _animationDriver;
    private readonly HtmlRenderer _renderer;
    private readonly InputRouter _inputRouter;
    
    // ===== Estado =====
    private string _htmlPath;
    private string _cssPath;
    private DataContext _dataContext;
    private bool _isDirty = true;
    
    public HtmlGump(uint local, uint server) : base(local, server)
    {
        _styleResolver = new StyleResolver();
        _layoutEngine = new FlexLayoutEngine();
        _animationDriver = new AnimationDriver();
        _renderer = new HtmlRenderer();
        _inputRouter = new InputRouter();
        _dataContext = new DataContext();
        
        AcceptMouseInput = true;
        AcceptKeyboardInput = true;
        CanMove = true;
        CanCloseWithRightClick = true;
    }
    
    /// <summary>
    /// Carrega um arquivo HTML do disco.
    /// </summary>
    public void LoadHtml(string path)
    {
        _htmlPath = path;
        var html = File.ReadAllText(path);
        ParseHtml(html);
    }
    
    /// <summary>
    /// Carrega o HTML de uma string (usado pelo servidor para enviar gumps via packet).
    /// </summary>
    public void LoadHtmlFromString(string html)
    {
        ParseHtml(html);
    }
    
    private void ParseHtml(string html)
    {
        var config = Configuration.Default
            .WithCss()
            .WithDefaultLoader();
        
        var context = BrowsingContext.New(config);
        _document = context.OpenAsync(req => req.Content(html)).Result;
        
        _rootNode = HtmlNode.Wrap(_document.DocumentElement);
        
        // Resolver os <link rel="stylesheet"> referenciados
        ResolveLinkedStylesheets();
        
        _isDirty = true;
    }
    
    private void ResolveLinkedStylesheets()
    {
        var links = _document.QuerySelectorAll("link[rel='stylesheet']");
        foreach (var link in links)
        {
            var href = link.GetAttribute("href");
            if (string.IsNullOrEmpty(href)) continue;
            
            var resolvedPath = ResolvePath(_htmlPath, href);
            if (File.Exists(resolvedPath))
            {
                var css = File.ReadAllText(resolvedPath);
                _styleResolver.AddStylesheet(css, resolvedPath);
            }
        }
        
        // Inline <style>
        var styleTags = _document.QuerySelectorAll("style");
        foreach (var styleTag in styleTags)
        {
            _styleResolver.AddStylesheet(styleTag.TextContent, "<inline>");
        }
    }
    
    private string ResolvePath(string baseFile, string relativePath)
    {
        var baseDir = Path.GetDirectoryName(baseFile);
        return Path.GetFullPath(Path.Combine(baseDir, relativePath));
    }
    
    public override void Update()
    {
        base.Update();
        
        var deltaSeconds = (float)Time.Ticks / 1000f - _lastTickSeconds;
        _lastTickSeconds = (float)Time.Ticks / 1000f;
        
        _animationDriver.Tick(deltaSeconds);
        
        if (_isDirty || _animationDriver.HasActiveAnimations || _styleResolver.IsDirty)
        {
            _styleResolver.ResolveAll(_rootNode);
            _layoutEngine.Layout(_rootNode, Width, Height);
            _isDirty = false;
        }
    }
    
    private float _lastTickSeconds = 0;
    
    public override bool Draw(UltimaBatcher2D batcher, int x, int y)
    {
        if (!base.Draw(batcher, x, y)) return false;
        _renderer.Render(_rootNode, batcher, x, y);
        return true;
    }
    
    protected override void OnMouseDown(int x, int y, MouseButtonType button)
    {
        _inputRouter.HandleMouseDown(_rootNode, x, y, button);
        base.OnMouseDown(x, y, button);
    }
    
    protected override void OnMouseOver(int x, int y)
    {
        _inputRouter.HandleMouseMove(_rootNode, x, y);
        base.OnMouseOver(x, y);
    }
    
    /// <summary>
    /// Define um valor no DataContext, dispara re-bind e re-layout se necessário.
    /// </summary>
    public void SetData(string key, object value)
    {
        _dataContext.Set(key, value);
        _isDirty = true;
    }
}
```

### 6.3 Wrapper `HtmlNode`

Não trabalhamos diretamente com `IElement` do AngleSharp em todas as camadas — criamos um wrapper que armazena estado computado (estilo final, layout box, etc.):

```csharp
// src/ClassicUO.Client/Game/UI/HtmlGumps/DOM/HtmlNode.cs
using AngleSharp.Dom;
using ClassicUO.Game.UI.HtmlGumps.Layout;
using ClassicUO.Game.UI.HtmlGumps.Style;

namespace ClassicUO.Game.UI.HtmlGumps.DOM;

/// <summary>
/// Wrapper sobre AngleSharp's IElement, com estado computado de estilo e layout.
/// </summary>
public class HtmlNode
{
    public IElement Element { get; }
    public HtmlNode Parent { get; private set; }
    public List<HtmlNode> Children { get; } = new();
    
    public ComputedStyle ComputedStyle { get; set; } = new();
    public LayoutBox LayoutBox { get; set; } = new();
    
    // Estados runtime
    public bool IsHovered { get; set; }
    public bool IsActive { get; set; }
    public bool IsFocused { get; set; }
    public bool IsDisabled { get; set; }
    
    // Identificação
    public string TagName => Element.TagName.ToLowerInvariant();
    public string Id => Element.Id;
    public IReadOnlyList<string> Classes => Element.ClassList.ToList();
    
    private HtmlNode(IElement element) { Element = element; }
    
    public static HtmlNode Wrap(IElement root)
    {
        var node = new HtmlNode(root);
        BuildTree(node);
        return node;
    }
    
    private static void BuildTree(HtmlNode parent)
    {
        foreach (var child in parent.Element.Children)
        {
            var childNode = new HtmlNode(child) { Parent = parent };
            parent.Children.Add(childNode);
            BuildTree(childNode);
        }
    }
    
    public string GetAttribute(string name) => Element.GetAttribute(name);
    public bool HasClass(string cls) => Element.ClassList.Contains(cls);
    public string TextContent => Element.TextContent;
    
    /// <summary>
    /// Retorna todos os descendants em ordem de árvore (depth-first pré-ordem).
    /// </summary>
    public IEnumerable<HtmlNode> Descendants()
    {
        foreach (var child in Children)
        {
            yield return child;
            foreach (var desc in child.Descendants())
                yield return desc;
        }
    }
}
```

### 6.4 Data binding `{{var}}`

Antes de parsear o HTML, fazemos uma passagem simples de string substitution:

```csharp
// src/ClassicUO.Client/Game/UI/HtmlGumps/DOM/DataBinding.cs
using System.Text.RegularExpressions;

namespace ClassicUO.Game.UI.HtmlGumps.DOM;

public class DataContext
{
    private readonly Dictionary<string, object> _data = new();
    public bool IsDirty { get; private set; }
    
    public void Set(string key, object value)
    {
        if (!_data.TryGetValue(key, out var existing) || !Equals(existing, value))
        {
            _data[key] = value;
            IsDirty = true;
        }
    }
    
    public object Get(string key) => _data.TryGetValue(key, out var v) ? v : null;
    
    public void ClearDirty() => IsDirty = false;
}

public static class TemplateProcessor
{
    private static readonly Regex BindingRegex = new(@"\{\{([^}]+)\}\}", RegexOptions.Compiled);
    
    public static string Process(string template, DataContext context)
    {
        return BindingRegex.Replace(template, match =>
        {
            var key = match.Groups[1].Value.Trim();
            var value = ResolveKey(key, context);
            return value?.ToString() ?? "";
        });
    }
    
    private static object ResolveKey(string key, DataContext context)
    {
        // Suporte a paths com ponto: skill1.cooldown
        var parts = key.Split('.');
        object current = context.Get(parts[0]);
        
        for (int i = 1; i < parts.Length && current != null; i++)
        {
            var prop = current.GetType().GetProperty(parts[i]);
            current = prop?.GetValue(current);
        }
        
        return current;
    }
}
```

### 6.5 Critérios de aceite da Fase 1

- [ ] `HtmlGump` carrega arquivo `.html` e `.css` do disco
- [ ] DOM é construído e navegável via `HtmlNode`
- [ ] `<link rel="stylesheet">` resolve paths relativos
- [ ] `{{var}}` é substituído antes do parsing
- [ ] Stylesheets inline (`<style>`) são detectados e armazenados
- [ ] Teste unitário: carregar um HTML simples com 10 elementos aninhados produz árvore DOM correspondente


---

## 7. Fase 2 — Layout engine (flexbox)

### 7.1 Objetivo da fase

Implementar um solver flexbox que recebe a árvore DOM com `ComputedStyle` resolvido e atribui `X`, `Y`, `Width`, `Height` para cada nó. É a fase mais matematicamente densa do projeto.

**Esforço estimado:** 8-12 dias.
**Referência teórica:** [CSS Flexbox spec do W3C](https://www.w3.org/TR/css-flexbox-1/) — não implementaremos 100%, mas as 12 etapas do algoritmo são o roteiro mestre.

### 7.2 Box model

Cada nó tem 4 caixas concêntricas (do externo para o interno):

```
┌─── margin box ──────────────────────────┐
│  ┌── border box ──────────────────────┐ │
│  │  ┌─ padding box ───────────────┐  │ │
│  │  │  ┌─ content box ─────────┐  │  │ │
│  │  │  │                       │  │  │ │
│  │  │  │   conteúdo do nó      │  │  │ │
│  │  │  │   (filhos)            │  │  │ │
│  │  │  │                       │  │  │ │
│  │  │  └───────────────────────┘  │  │ │
│  │  └──────────────────────────────┘  │ │
│  └─────────────────────────────────────┘ │
└──────────────────────────────────────────┘
```

```csharp
// src/ClassicUO.Client/Game/UI/HtmlGumps/Layout/LayoutBox.cs
namespace ClassicUO.Game.UI.HtmlGumps.Layout;

public struct EdgeInsets
{
    public float Top, Right, Bottom, Left;
    public float Horizontal => Left + Right;
    public float Vertical => Top + Bottom;
    
    public static EdgeInsets Zero => new();
    public static EdgeInsets All(float v) => new() { Top = v, Right = v, Bottom = v, Left = v };
}

public class LayoutBox
{
    // Posição absoluta na viewport (em pixels)
    public float X { get; set; }
    public float Y { get; set; }
    
    // Dimensões da border-box
    public float Width { get; set; }
    public float Height { get; set; }
    
    // Edges resolvidos
    public EdgeInsets Margin { get; set; }
    public EdgeInsets Border { get; set; }
    public EdgeInsets Padding { get; set; }
    
    // Calculados durante layout
    public float ContentWidth => Width - Border.Horizontal - Padding.Horizontal;
    public float ContentHeight => Height - Border.Vertical - Padding.Vertical;
    
    public float ContentX => X + Border.Left + Padding.Left;
    public float ContentY => Y + Border.Top + Padding.Top;
    
    // Para flex items
    public float FlexBasis { get; set; }
    public float FlexGrow { get; set; }
    public float FlexShrink { get; set; }
    
    // Para texto
    public TextLayoutInfo TextInfo { get; set; }
}
```

### 7.3 Algoritmo de layout flexbox

Vou descrever as etapas do algoritmo. Cada etapa é uma função no `FlexLayoutEngine`:

```csharp
// src/ClassicUO.Client/Game/UI/HtmlGumps/Layout/FlexLayoutEngine.cs
namespace ClassicUO.Game.UI.HtmlGumps.Layout;

public class FlexLayoutEngine
{
    public bool IsDirty { get; set; } = true;
    
    public void Layout(HtmlNode root, float viewportWidth, float viewportHeight)
    {
        // Etapa 1: definir o root como o tamanho da viewport
        root.LayoutBox.X = 0;
        root.LayoutBox.Y = 0;
        root.LayoutBox.Width = ResolveWidth(root, viewportWidth);
        root.LayoutBox.Height = ResolveHeight(root, viewportHeight);
        
        // Etapa 2: layout recursivo
        LayoutNode(root, root.LayoutBox.ContentWidth, root.LayoutBox.ContentHeight);
        
        IsDirty = false;
    }
    
    private void LayoutNode(HtmlNode node, float availableWidth, float availableHeight)
    {
        var style = node.ComputedStyle;
        
        // Resolver margin/border/padding
        node.LayoutBox.Margin = ResolveEdges(style, "margin");
        node.LayoutBox.Border = ResolveEdges(style, "border-width");
        node.LayoutBox.Padding = ResolveEdges(style, "padding");
        
        if (style.Display == DisplayType.Flex)
        {
            LayoutFlexContainer(node, availableWidth, availableHeight);
        }
        else if (style.Display == DisplayType.Block)
        {
            LayoutBlock(node, availableWidth, availableHeight);
        }
        else if (style.Display == DisplayType.Inline)
        {
            LayoutInline(node, availableWidth, availableHeight);
        }
        else if (style.Display == DisplayType.None)
        {
            // Não renderiza nem ocupa espaço
            node.LayoutBox.Width = 0;
            node.LayoutBox.Height = 0;
        }
    }
    
    /// <summary>
    /// Algoritmo de layout flex. Implementa o spec em 12 passos simplificado.
    /// </summary>
    private void LayoutFlexContainer(HtmlNode container, float availableWidth, float availableHeight)
    {
        var style = container.ComputedStyle;
        var isRow = style.FlexDirection == FlexDirection.Row || style.FlexDirection == FlexDirection.RowReverse;
        var isReverse = style.FlexDirection == FlexDirection.RowReverse || style.FlexDirection == FlexDirection.ColumnReverse;
        
        // Eixo principal (main axis) e cruzado (cross axis)
        var mainSize = isRow ? availableWidth : availableHeight;
        var crossSize = isRow ? availableHeight : availableWidth;
        
        var children = container.Children.Where(c => c.ComputedStyle.Display != DisplayType.None).ToList();
        
        // Passo 1: calcular flex-basis de cada filho
        foreach (var child in children)
        {
            child.LayoutBox.FlexBasis = ResolveFlexBasis(child, isRow, mainSize);
        }
        
        // Passo 2: determinar se cabe na linha. Se flex-wrap é wrap, dividir em linhas.
        var lines = SplitIntoLines(children, mainSize, style.FlexWrap, isRow);
        
        // Passo 3: para cada linha, distribuir espaço livre via flex-grow/shrink
        float crossOffset = 0;
        foreach (var line in lines)
        {
            DistributeMainAxisSpace(line, mainSize, style.JustifyContent, isRow);
            
            float lineCrossSize = line.Max(c => GetCrossSize(c, isRow));
            
            // Passo 4: posicionar cada filho na linha
            float mainOffset = 0;
            foreach (var child in line)
            {
                if (isRow)
                {
                    child.LayoutBox.X = container.LayoutBox.ContentX + mainOffset;
                    child.LayoutBox.Y = container.LayoutBox.ContentY + crossOffset 
                        + ResolveAlignItems(child, style.AlignItems, lineCrossSize);
                }
                else
                {
                    child.LayoutBox.Y = container.LayoutBox.ContentY + mainOffset;
                    child.LayoutBox.X = container.LayoutBox.ContentX + crossOffset 
                        + ResolveAlignItems(child, style.AlignItems, lineCrossSize);
                }
                
                mainOffset += GetMainSize(child, isRow) + style.Gap;
                
                // Passo 5: layout recursivo dos filhos
                LayoutNode(child, child.LayoutBox.ContentWidth, child.LayoutBox.ContentHeight);
            }
            
            crossOffset += lineCrossSize + style.Gap;
        }
    }
    
    // ... mais métodos auxiliares
}
```

### 7.4 Sizing: FIT, GROW, FIXED, PERCENT, AUTO

Definimos um enum para as estratégias de sizing inspiradas no Clay (que você viu no link), mas usando sintaxe CSS:

```css
.slot {
  width: 56px;        /* FIXED */
  height: auto;       /* AUTO (FIT to content) */
  flex-grow: 1;       /* GROW */
}

.bar {
  width: 50%;         /* PERCENT */
  height: 14px;
}

.container {
  width: 100%;
  min-width: 200px;
  max-width: 800px;
}
```

```csharp
public enum SizingMode { Auto, Fixed, Percent, Grow }

public struct SizeValue
{
    public SizingMode Mode;
    public float Value;
    public float Min;
    public float Max;
    
    public float Resolve(float availableSpace, float contentSize)
    {
        return Mode switch
        {
            SizingMode.Fixed => Math.Clamp(Value, Min, Max),
            SizingMode.Percent => Math.Clamp(availableSpace * (Value / 100f), Min, Max),
            SizingMode.Auto => Math.Clamp(contentSize, Min, Max),
            SizingMode.Grow => availableSpace,
            _ => contentSize
        };
    }
}
```

### 7.5 Layout de texto

Texto é o caso mais complexo de layout. Precisamos:

1. Quebrar em linhas conforme largura disponível
2. Medir cada linha (largura, altura)
3. Considerar `text-align`, `line-height`, `letter-spacing`
4. Suportar `<br>`, espaços, palavras longas que ultrapassam

Para o TazUO, usamos o sistema de fonte TTF que ele já tem (você mesmo trabalhou nisso). Adaptamos:

```csharp
// src/ClassicUO.Client/Game/UI/HtmlGumps/Layout/TextLayout.cs
public class TextLayout
{
    public List<TextLine> Lines { get; } = new();
    public float TotalHeight { get; set; }
    public float MaxWidth { get; set; }
    
    public static TextLayout Compute(string text, ComputedStyle style, float maxWidth)
    {
        var layout = new TextLayout();
        var font = FontResolver.Resolve(style.FontFamily, style.FontSize, style.FontWeight, style.FontStyle);
        
        var words = SplitIntoWords(text);
        var currentLine = new TextLine();
        float currentLineWidth = 0;
        
        foreach (var word in words)
        {
            float wordWidth = font.MeasureString(word).X + style.LetterSpacing * word.Length;
            
            if (currentLineWidth + wordWidth > maxWidth && currentLine.Words.Count > 0)
            {
                // Quebra de linha
                layout.Lines.Add(currentLine);
                currentLine = new TextLine();
                currentLineWidth = 0;
            }
            
            currentLine.Words.Add(new TextWord { Text = word, Width = wordWidth });
            currentLineWidth += wordWidth;
        }
        
        if (currentLine.Words.Count > 0)
            layout.Lines.Add(currentLine);
        
        layout.TotalHeight = layout.Lines.Count * style.LineHeight;
        layout.MaxWidth = layout.Lines.Max(l => l.Width);
        
        return layout;
    }
}
```

### 7.6 Critérios de aceite da Fase 2

- [ ] `<div style="display: flex; gap: 8px"><div>A</div><div>B</div></div>` produz dois elementos lado-a-lado com 8px de gap
- [ ] `flex-direction: column` empilha verticalmente
- [ ] `justify-content: center / space-between / space-around` distribui corretamente
- [ ] `align-items: center` alinha no eixo cruzado
- [ ] `flex-grow: 1` em um filho expande para ocupar espaço restante
- [ ] `padding`, `margin`, `border` calculam corretamente as dimensões
- [ ] Quebra de texto em palavras conforme largura disponível
- [ ] `width: 100%` e `width: 50%` calculam baseado no pai
- [ ] `min-width` e `max-width` são respeitados

---

## 8. Fase 3 — Style engine (CSS)

### 8.1 Objetivo da fase

Implementar a cascata CSS: pegar todas as regras dos stylesheets carregados, calcular qual aplica para cada nó (com base em seletores e especificidade), e produzir um `ComputedStyle` final por nó. Suportar variáveis CSS, herança, `!important`.

**Esforço estimado:** 6-9 dias.
**Referência:** AngleSharp.Css já parseia o CSS pra você. Você implementa a lógica de matching e cascade.

### 8.2 ComputedStyle: o que armazenar

O `ComputedStyle` é o estilo final, resolvido, aplicado a um nó. Tem todas as propriedades suportadas:

```csharp
// src/ClassicUO.Client/Game/UI/HtmlGumps/Style/ComputedStyle.cs
public class ComputedStyle
{
    // ===== Display & Layout =====
    public DisplayType Display = DisplayType.Block;
    public PositionType Position = PositionType.Static;
    public FlexDirection FlexDirection = FlexDirection.Row;
    public FlexWrap FlexWrap = FlexWrap.NoWrap;
    public JustifyContent JustifyContent = JustifyContent.FlexStart;
    public AlignItems AlignItems = AlignItems.Stretch;
    public float Gap;
    public float FlexGrow;
    public float FlexShrink = 1;
    public SizeValue FlexBasis = new() { Mode = SizingMode.Auto };
    
    // ===== Sizing =====
    public SizeValue Width = new() { Mode = SizingMode.Auto };
    public SizeValue Height = new() { Mode = SizingMode.Auto };
    public SizeValue MinWidth, MinHeight, MaxWidth, MaxHeight;
    
    // ===== Spacing =====
    public EdgeInsets Margin;
    public EdgeInsets Padding;
    
    // ===== Position =====
    public float Top, Right, Bottom, Left;
    public int ZIndex;
    
    // ===== Border =====
    public EdgeInsets BorderWidth;
    public BorderStyle BorderStyle = BorderStyle.None;
    public Color BorderColor = Color.Transparent;
    public float BorderRadiusTopLeft, BorderRadiusTopRight, BorderRadiusBottomLeft, BorderRadiusBottomRight;
    
    // ===== Background =====
    public Color BackgroundColor = Color.Transparent;
    public Gradient BackgroundGradient;
    public string BackgroundImage;
    public BackgroundSize BackgroundSize;
    public BackgroundRepeat BackgroundRepeat;
    public Vector2 BackgroundPosition;
    
    // ===== Typography =====
    public string FontFamily = "Cormorant";
    public float FontSize = 14;
    public FontWeight FontWeight = FontWeight.Normal;
    public FontStyle FontStyle = FontStyle.Normal;
    public Color Color = Color.White;
    public TextAlign TextAlign = TextAlign.Left;
    public float LineHeight = 1.4f;
    public float LetterSpacing;
    public TextShadow TextShadow;
    
    // ===== Effects =====
    public List<BoxShadow> BoxShadows = new();
    public List<FilterEffect> Filters = new();
    public BackdropFilter BackdropFilter;
    public float Opacity = 1f;
    public Transform Transform = Transform.Identity;
    public TransformOrigin TransformOrigin = TransformOrigin.Center;
    public float Perspective;
    
    // ===== Animation =====
    public List<TransitionRule> Transitions = new();
    public AnimationRule Animation;
    
    // ===== Cursor =====
    public CursorType Cursor = CursorType.Default;
    
    // ===== Overflow =====
    public OverflowType OverflowX = OverflowType.Visible;
    public OverflowType OverflowY = OverflowType.Visible;
}
```

### 8.3 Matching de seletores

AngleSharp tem `IElement.Matches(selector)` que faz matching CSS. Mas para performance precisamos calcular especificidade nós mesmos:

```csharp
// src/ClassicUO.Client/Game/UI/HtmlGumps/Style/Selectors.cs
public class CssSelector
{
    public string Raw { get; set; }
    public Specificity Specificity { get; set; }
    
    public bool Matches(HtmlNode node)
    {
        // Use AngleSharp para matching base
        return node.Element.Matches(Raw);
    }
}

public struct Specificity : IComparable<Specificity>
{
    public int A, B, C; // (inline, IDs, classes+attrs+pseudo, elementos)
    
    public static Specificity Calculate(string selector)
    {
        // Algoritmo da spec CSS: contar IDs, classes, elementos
        int a = 0, b = 0, c = 0;
        // ...parsing simplificado
        return new Specificity { A = a, B = b, C = c };
    }
    
    public int CompareTo(Specificity other)
    {
        if (A != other.A) return A.CompareTo(other.A);
        if (B != other.B) return B.CompareTo(other.B);
        return C.CompareTo(other.C);
    }
}
```

### 8.4 Cascade

O algoritmo de cascade decide, dentre todas as regras que casam com um nó, qual valor cada propriedade recebe:

```csharp
// src/ClassicUO.Client/Game/UI/HtmlGumps/Style/StyleResolver.cs
public class StyleResolver
{
    private List<CssRule> _allRules = new();
    public bool IsDirty { get; private set; } = true;
    
    public void AddStylesheet(string css, string source)
    {
        var parser = new AngleSharp.Css.Parser.CssParser();
        var sheet = parser.ParseStyleSheet(css);
        
        foreach (ICssStyleRule rule in sheet.Rules.OfType<ICssStyleRule>())
        {
            _allRules.Add(new CssRule
            {
                Selector = new CssSelector { Raw = rule.SelectorText, Specificity = Specificity.Calculate(rule.SelectorText) },
                Declarations = ParseDeclarations(rule.Style),
                Source = source,
                Order = _allRules.Count
            });
        }
        
        IsDirty = true;
    }
    
    public void ResolveAll(HtmlNode root)
    {
        ResolveRecursive(root);
        IsDirty = false;
    }
    
    private void ResolveRecursive(HtmlNode node)
    {
        // 1. Coletar todas as regras que casam com este nó
        var matching = new List<MatchedRule>();
        foreach (var rule in _allRules)
        {
            if (rule.Selector.Matches(node))
            {
                matching.Add(new MatchedRule { Rule = rule, Specificity = rule.Selector.Specificity });
            }
        }
        
        // 2. Adicionar pseudo-classes :hover, :active etc.
        if (node.IsHovered)
            AddPseudoMatches(matching, node, ":hover");
        if (node.IsActive)
            AddPseudoMatches(matching, node, ":active");
        if (node.IsFocused)
            AddPseudoMatches(matching, node, ":focus");
        
        // 3. Ordenar por especificidade ASC (depois sobrescreve antes)
        matching.Sort((a, b) => a.Specificity.CompareTo(b.Specificity));
        
        // 4. Aplicar declarações em ordem
        var computed = new ComputedStyle();
        InheritFromParent(computed, node.Parent);
        
        foreach (var match in matching)
        {
            foreach (var decl in match.Rule.Declarations)
            {
                ApplyDeclaration(computed, decl);
            }
        }
        
        // 5. Resolver variáveis CSS
        ResolveCssVariables(computed, node);
        
        node.ComputedStyle = computed;
        
        // Recursão
        foreach (var child in node.Children)
            ResolveRecursive(child);
    }
    
    private void InheritFromParent(ComputedStyle child, HtmlNode parent)
    {
        if (parent == null) return;
        var p = parent.ComputedStyle;
        
        // Propriedades herdáveis (color, font, etc.)
        child.Color = p.Color;
        child.FontFamily = p.FontFamily;
        child.FontSize = p.FontSize;
        child.FontWeight = p.FontWeight;
        child.FontStyle = p.FontStyle;
        child.LineHeight = p.LineHeight;
        child.LetterSpacing = p.LetterSpacing;
        child.TextAlign = p.TextAlign;
        // ... outras propriedades herdáveis
    }
    
    private void ApplyDeclaration(ComputedStyle style, CssDeclaration decl)
    {
        switch (decl.Property)
        {
            case "display": style.Display = ParseDisplay(decl.Value); break;
            case "width": style.Width = CssValueParser.ParseSize(decl.Value); break;
            case "height": style.Height = CssValueParser.ParseSize(decl.Value); break;
            case "background-color": style.BackgroundColor = CssValueParser.ParseColor(decl.Value); break;
            case "color": style.Color = CssValueParser.ParseColor(decl.Value); break;
            case "padding": style.Padding = CssValueParser.ParseEdges(decl.Value); break;
            case "padding-left": style.Padding = style.Padding.WithLeft(CssValueParser.ParseLength(decl.Value)); break;
            // ... centenas de propriedades
        }
    }
}
```

### 8.5 Variáveis CSS (custom properties)

```css
:root {
  --ep-primary: #722f37;
  --ep-gold: #c9a961;
  --ep-font: 'Cinzel', serif;
}

.button {
  background: var(--ep-primary);
  color: var(--ep-gold);
  font-family: var(--ep-font);
}
```

```csharp
// src/ClassicUO.Client/Game/UI/HtmlGumps/Style/CssVariables.cs
public class CssVariableScope
{
    private Dictionary<string, string> _vars = new();
    public CssVariableScope Parent { get; set; }
    
    public void Set(string name, string value) => _vars[name] = value;
    
    public string Resolve(string name)
    {
        if (_vars.TryGetValue(name, out var v)) return v;
        return Parent?.Resolve(name);
    }
    
    public string ResolveValue(string cssValue)
    {
        // var(--name, fallback)
        return Regex.Replace(cssValue, @"var\(\s*(--[\w-]+)(?:\s*,\s*([^)]+))?\s*\)", match =>
        {
            var name = match.Groups[1].Value;
            var fallback = match.Groups[2].Value;
            return Resolve(name) ?? fallback ?? "";
        });
    }
}
```

### 8.6 Critérios de aceite da Fase 3

- [ ] `.button { background: red }` aplica vermelho a todo `<button class="button">`
- [ ] `#main { color: blue }` aplica azul ao `<x id="main">`
- [ ] `div > span { ... }` matcha apenas filhos diretos
- [ ] Especificidade: `#x` ganha de `.x` ganha de `x`
- [ ] `!important` força aplicação independente de especificidade
- [ ] Herança: `color` no pai propaga para filhos
- [ ] `var(--name)` é resolvido conforme escopo `:root`
- [ ] `:hover`, `:active`, `:focus` adicionam regras quando estados estão ativos
- [ ] Mudanças no CSS recarregado refletem em todos os nós

---

## 9. Fase 4 — Render layer

### 9.1 Objetivo da fase

Desenhar cada nó do DOM no `UltimaBatcher2D` (o sprite batch do TazUO). Esta é a fase que "dá vida" ao sistema — você vê pixels na tela.

**Esforço estimado:** 6-10 dias.

### 9.2 Orquestrador `HtmlRenderer`

```csharp
// src/ClassicUO.Client/Game/UI/HtmlGumps/Render/HtmlRenderer.cs
public class HtmlRenderer
{
    private readonly BoxRenderer _boxRenderer = new();
    private readonly TextRenderer _textRenderer = new();
    private readonly ImageRenderer _imageRenderer = new();
    private readonly EffectsPipeline _effects = new();
    private readonly SkiaRenderer _skia = new();
    
    public void Render(HtmlNode root, UltimaBatcher2D batcher, int gumpX, int gumpY)
    {
        RenderNode(root, batcher, gumpX, gumpY);
    }
    
    private void RenderNode(HtmlNode node, UltimaBatcher2D batcher, int offsetX, int offsetY)
    {
        var style = node.ComputedStyle;
        if (style.Display == DisplayType.None || style.Opacity <= 0) return;
        
        var box = node.LayoutBox;
        var screenX = (int)(box.X + offsetX);
        var screenY = (int)(box.Y + offsetY);
        
        // Push transform se houver
        bool pushedTransform = false;
        if (!style.Transform.IsIdentity)
        {
            _effects.PushTransform(batcher, style.Transform, style.TransformOrigin, box);
            pushedTransform = true;
        }
        
        // 1. Backdrop filter (vidro fosco) — apenas se especificado
        if (style.BackdropFilter != null)
        {
            _effects.RenderBackdropFilter(batcher, screenX, screenY, (int)box.Width, (int)box.Height, style.BackdropFilter);
        }
        
        // 2. Box-shadow outset (atrás)
        foreach (var shadow in style.BoxShadows.Where(s => !s.Inset))
        {
            _effects.RenderBoxShadow(batcher, box, screenX, screenY, shadow);
        }
        
        // 3. Background (color, gradient, image)
        _boxRenderer.RenderBackground(batcher, screenX, screenY, box, style);
        
        // 4. Box-shadow inset (na frente do bg)
        foreach (var shadow in style.BoxShadows.Where(s => s.Inset))
        {
            _effects.RenderBoxShadow(batcher, box, screenX, screenY, shadow);
        }
        
        // 5. Border
        _boxRenderer.RenderBorder(batcher, screenX, screenY, box, style);
        
        // 6. Conteúdo: texto, imagens, SVG, filhos
        if (node.TagName == "img")
        {
            _imageRenderer.RenderImage(batcher, screenX, screenY, box, node, style);
        }
        else if (node.TagName == "svg")
        {
            _skia.RenderSvg(batcher, screenX, screenY, box, node);
        }
        else if (HasDirectTextContent(node))
        {
            _textRenderer.RenderText(batcher, screenX, screenY, box, node, style);
        }
        
        // Filhos (recursivo)
        foreach (var child in node.Children)
        {
            RenderNode(child, batcher, offsetX, offsetY);
        }
        
        // 7. Filter (filter geral) — aplicado como pós-processamento
        if (style.Filters.Any())
        {
            _effects.ApplyFilters(batcher, screenX, screenY, box, style.Filters);
        }
        
        if (pushedTransform)
        {
            _effects.PopTransform(batcher);
        }
    }
}
```

### 9.3 Renderização de background, border, gradients

```csharp
// src/ClassicUO.Client/Game/UI/HtmlGumps/Render/BoxRenderer.cs
public class BoxRenderer
{
    public void RenderBackground(UltimaBatcher2D batcher, int x, int y, LayoutBox box, ComputedStyle style)
    {
        if (style.BorderRadiusTopLeft > 0 || style.BorderRadiusTopRight > 0 ||
            style.BorderRadiusBottomLeft > 0 || style.BorderRadiusBottomRight > 0)
        {
            RenderRoundedBackground(batcher, x, y, box, style);
        }
        else
        {
            RenderRectBackground(batcher, x, y, box, style);
        }
    }
    
    private void RenderRectBackground(UltimaBatcher2D batcher, int x, int y, LayoutBox box, ComputedStyle style)
    {
        if (style.BackgroundGradient != null)
        {
            RenderGradient(batcher, x, y, (int)box.Width, (int)box.Height, style.BackgroundGradient);
        }
        else if (style.BackgroundColor.A > 0)
        {
            // Usa o sistema de retângulos sólidos do TazUO
            batcher.Draw(
                SolidColorTextures.GetTexture(style.BackgroundColor),
                new Rectangle(x, y, (int)box.Width, (int)box.Height),
                Color.White
            );
        }
        
        if (!string.IsNullOrEmpty(style.BackgroundImage))
        {
            var texture = TextureCache.Load(style.BackgroundImage);
            if (texture != null)
            {
                RenderBackgroundImage(batcher, x, y, box, texture, style);
            }
        }
    }
    
    private void RenderRoundedBackground(UltimaBatcher2D batcher, int x, int y, LayoutBox box, ComputedStyle style)
    {
        // Borders radius requer ou:
        // a) tessellation (gerar mesh de triângulos do retângulo arredondado)
        // b) shader que descarta pixels fora do raio
        // c) usar SkiaSharp para gerar uma textura uma vez e cachear
        
        // Solução escolhida: opção (c) — gerar textura no Skia, cachear por (size, radius) hash
        var key = new RoundedBoxKey 
        { 
            Width = (int)box.Width, 
            Height = (int)box.Height,
            RadiusTL = style.BorderRadiusTopLeft,
            RadiusTR = style.BorderRadiusTopRight,
            RadiusBL = style.BorderRadiusBottomLeft,
            RadiusBR = style.BorderRadiusBottomRight,
            Color = style.BackgroundColor
        };
        
        var texture = RoundedBoxCache.GetOrCreate(key);
        batcher.Draw(texture, new Rectangle(x, y, (int)box.Width, (int)box.Height), Color.White);
    }
    
    public void RenderBorder(UltimaBatcher2D batcher, int x, int y, LayoutBox box, ComputedStyle style)
    {
        if (style.BorderWidth.Top > 0)
            DrawLine(batcher, x, y, x + (int)box.Width, y, style.BorderWidth.Top, style.BorderColor);
        // ... outros 3 lados
    }
}
```

### 9.4 Renderização de texto

```csharp
// src/ClassicUO.Client/Game/UI/HtmlGumps/Render/TextRenderer.cs
public class TextRenderer
{
    public void RenderText(UltimaBatcher2D batcher, int x, int y, LayoutBox box, HtmlNode node, ComputedStyle style)
    {
        var textInfo = box.TextInfo;
        if (textInfo == null) return;
        
        var font = FontResolver.Resolve(style.FontFamily, style.FontSize, style.FontWeight, style.FontStyle);
        
        float lineY = y + box.Padding.Top;
        foreach (var line in textInfo.Lines)
        {
            float lineX = x + box.Padding.Left;
            
            // Aplica text-align
            switch (style.TextAlign)
            {
                case TextAlign.Center:
                    lineX += (box.ContentWidth - line.Width) / 2;
                    break;
                case TextAlign.Right:
                    lineX += box.ContentWidth - line.Width;
                    break;
            }
            
            // Text-shadow primeiro (atrás)
            if (style.TextShadow != null)
            {
                font.DrawString(batcher, line.Text, 
                    new Vector2(lineX + style.TextShadow.OffsetX, lineY + style.TextShadow.OffsetY),
                    style.TextShadow.Color);
            }
            
            // Texto principal
            font.DrawString(batcher, line.Text, new Vector2(lineX, lineY), style.Color);
            
            lineY += style.LineHeight;
        }
    }
}
```

### 9.5 Critérios de aceite da Fase 4

- [ ] `<div style="background:red; width:100px; height:100px"/>` desenha quadrado vermelho
- [ ] `border: 2px solid gold` desenha borda dourada
- [ ] `border-radius: 8px` arredonda cantos
- [ ] `background: linear-gradient(...)` renderiza gradiente
- [ ] `background-image: url(...)` carrega e desenha imagem
- [ ] Texto é desenhado com fonte/cor/tamanho corretos
- [ ] `text-shadow` aparece atrás do texto
- [ ] Texto quebra em múltiplas linhas conforme `text-wrap`
- [ ] Filhos aninhados são desenhados em ordem correta
- [ ] `opacity: 0.5` reduz a alpha de todo o subtree


---

## 10. Fase 5 — Estados e interatividade

### 10.1 Objetivo da fase

Implementar o roteamento de eventos de mouse e teclado para os nós do DOM, atualizando estados (`:hover`, `:active`, `:focus`) e disparando callbacks C#.

**Esforço estimado:** 4-6 dias.

### 10.2 Hit testing

Para saber qual nó está sob o mouse, percorremos o DOM em ordem reversa (filhos por cima, depois pais):

```csharp
// src/ClassicUO.Client/Game/UI/HtmlGumps/Input/HitTester.cs
public class HitTester
{
    public HtmlNode HitTest(HtmlNode root, float x, float y)
    {
        return HitTestRecursive(root, x, y);
    }
    
    private HtmlNode HitTestRecursive(HtmlNode node, float x, float y)
    {
        var box = node.LayoutBox;
        var style = node.ComputedStyle;
        
        if (style.Display == DisplayType.None) return null;
        
        // Aplicar transform inverso se tiver
        if (!style.Transform.IsIdentity)
        {
            var (tx, ty) = style.Transform.InverseTransform(x, y, box);
            x = tx; y = ty;
        }
        
        // Está dentro da bounding box?
        if (x < box.X || x > box.X + box.Width ||
            y < box.Y || y > box.Y + box.Height)
        {
            return null;
        }
        
        // Checar filhos primeiro (eles ficam por cima na ordem visual)
        foreach (var child in node.Children.AsEnumerable().Reverse())
        {
            var hit = HitTestRecursive(child, x, y);
            if (hit != null) return hit;
        }
        
        // Se nenhum filho casou, é este nó
        // (a menos que pointer-events: none)
        if (style.PointerEvents == PointerEventsType.None) return null;
        
        return node;
    }
}
```

### 10.3 InputRouter

```csharp
// src/ClassicUO.Client/Game/UI/HtmlGumps/Input/InputRouter.cs
public class InputRouter
{
    private HtmlNode _hoveredNode;
    private HtmlNode _activeNode;
    private HtmlNode _focusedNode;
    
    public event Action<HtmlNode, MouseEventArgs> OnClick;
    public event Action<HtmlNode> OnHoverEnter;
    public event Action<HtmlNode> OnHoverLeave;
    
    public void HandleMouseMove(HtmlNode root, float x, float y)
    {
        var hit = new HitTester().HitTest(root, x, y);
        
        if (hit != _hoveredNode)
        {
            // Mudança de hover
            if (_hoveredNode != null)
            {
                _hoveredNode.IsHovered = false;
                OnHoverLeave?.Invoke(_hoveredNode);
                MarkDirty(_hoveredNode);
            }
            
            _hoveredNode = hit;
            
            if (_hoveredNode != null)
            {
                _hoveredNode.IsHovered = true;
                OnHoverEnter?.Invoke(_hoveredNode);
                MarkDirty(_hoveredNode);
            }
        }
    }
    
    public void HandleMouseDown(HtmlNode root, float x, float y, MouseButtonType button)
    {
        var hit = new HitTester().HitTest(root, x, y);
        if (hit == null) return;
        
        _activeNode = hit;
        hit.IsActive = true;
        MarkDirty(hit);
        
        // Focus
        if (_focusedNode != hit)
        {
            if (_focusedNode != null)
            {
                _focusedNode.IsFocused = false;
                MarkDirty(_focusedNode);
            }
            _focusedNode = hit;
            hit.IsFocused = true;
            MarkDirty(hit);
        }
    }
    
    public void HandleMouseUp(HtmlNode root, float x, float y, MouseButtonType button)
    {
        var hit = new HitTester().HitTest(root, x, y);
        
        if (_activeNode != null)
        {
            _activeNode.IsActive = false;
            MarkDirty(_activeNode);
            
            // Se solta no mesmo nó que pressionou: click
            if (hit == _activeNode)
            {
                OnClick?.Invoke(_activeNode, new MouseEventArgs { X = x, Y = y, Button = button });
                DispatchClickHandler(_activeNode);
            }
            
            _activeNode = null;
        }
    }
    
    private void DispatchClickHandler(HtmlNode node)
    {
        // Procurar handler registrado via data-onclick="MyHandler"
        var handlerName = node.GetAttribute("data-onclick");
        if (!string.IsNullOrEmpty(handlerName))
        {
            ClickHandlerRegistry.Invoke(handlerName, node);
        }
    }
    
    private void MarkDirty(HtmlNode node)
    {
        // Marcar para re-resolve de estilo (pseudo-classes mudaram)
        node.StyleDirty = true;
    }
}
```

### 10.4 Registro de handlers C#

Como não temos JavaScript, eventos `onclick` são roteados para handlers C# registrados:

```csharp
// src/ClassicUO.Client/Game/UI/HtmlGumps/Input/ClickHandlerRegistry.cs
public static class ClickHandlerRegistry
{
    private static Dictionary<string, Action<HtmlNode>> _handlers = new();
    
    public static void Register(string name, Action<HtmlNode> handler)
    {
        _handlers[name] = handler;
    }
    
    public static void Invoke(string name, HtmlNode node)
    {
        if (_handlers.TryGetValue(name, out var handler))
        {
            handler(node);
        }
    }
}
```

Uso no HTML:

```html
<button class="slot" data-onclick="UseSkill" data-skill="fireball">
  Fireball
</button>
```

E no C# de cada Gump:

```csharp
public class HotbarGump : HtmlGump
{
    public HotbarGump(uint local, uint server) : base(local, server)
    {
        ClickHandlerRegistry.Register("UseSkill", node => 
        {
            var skill = node.GetAttribute("data-skill");
            World.Player.UseSkill(skill);
        });
        
        LoadHtml("Data/HtmlGumps/hotbar.html");
    }
}
```

### 10.5 Critérios de aceite da Fase 5

- [ ] Mouse over em um botão dispara `:hover`, mudando estilo conforme CSS
- [ ] Clique pressionado dispara `:active`
- [ ] Click bem-sucedido (down + up no mesmo nó) chama o handler em C#
- [ ] `pointer-events: none` faz o nó ser invisível ao hit test
- [ ] Múltiplos eventos hover/leave em movimento contínuo do mouse

---

## 11. Fase 6 — Animations e transitions

### 11.1 Objetivo da fase

Implementar `transition: property duration easing` e `@keyframes` + `animation: name duration`. É o que dá vida visual à UI.

**Esforço estimado:** 5-7 dias.

### 11.2 Modelo conceitual

Cada animação é um **Tween** que:
- Tem um valor inicial (de) e final (para)
- Tem uma duração e easing
- A cada frame, calcula seu valor atual via `easing(t / duration)` aplicado a interpolação

```csharp
// src/ClassicUO.Client/Game/UI/HtmlGumps/Animation/Tween.cs
public abstract class Tween
{
    public float Duration;
    public float Elapsed;
    public Func<float, float> Easing;
    public bool IsComplete => Elapsed >= Duration;
    
    public abstract void Apply(ComputedStyle style);
    
    public void Tick(float deltaSeconds)
    {
        Elapsed = Math.Min(Elapsed + deltaSeconds, Duration);
    }
    
    protected float Progress
    {
        get
        {
            float t = Duration > 0 ? Elapsed / Duration : 1f;
            return Easing?.Invoke(t) ?? t;
        }
    }
}

public class ColorTween : Tween
{
    public Color From, To;
    public string Property; // "color", "background-color", "border-color"
    
    public override void Apply(ComputedStyle style)
    {
        var color = Color.Lerp(From, To, Progress);
        switch (Property)
        {
            case "color": style.Color = color; break;
            case "background-color": style.BackgroundColor = color; break;
            case "border-color": style.BorderColor = color; break;
        }
    }
}

public class FloatTween : Tween
{
    public float From, To;
    public Action<float, ComputedStyle> Setter;
    
    public override void Apply(ComputedStyle style)
    {
        var v = MathHelper.Lerp(From, To, Progress);
        Setter(v, style);
    }
}

public class TransformTween : Tween
{
    public Transform From, To;
    
    public override void Apply(ComputedStyle style)
    {
        style.Transform = Transform.Lerp(From, To, Progress);
    }
}
```

### 11.3 Easing functions

```csharp
// src/ClassicUO.Client/Game/UI/HtmlGumps/Animation/Easing.cs
public static class Easing
{
    public static float Linear(float t) => t;
    public static float EaseIn(float t) => t * t;
    public static float EaseOut(float t) => 1f - (1f - t) * (1f - t);
    public static float EaseInOut(float t) => t < 0.5f ? 2 * t * t : 1f - 2 * (1 - t) * (1 - t);
    
    // Cubic-bezier: a função CSS principal
    public static Func<float, float> CubicBezier(float x1, float y1, float x2, float y2)
    {
        return t =>
        {
            // Resolução de cubic bezier — método Newton-Raphson ou aproximação
            // Para fins de produção, usar uma LUT pre-calculada
            return SolveCubicBezier(t, x1, y1, x2, y2);
        };
    }
    
    public static Func<float, float> Parse(string css)
    {
        return css switch
        {
            "linear" => Linear,
            "ease-in" => EaseIn,
            "ease-out" => EaseOut,
            "ease-in-out" => EaseInOut,
            _ when css.StartsWith("cubic-bezier") => ParseCubicBezier(css),
            _ => Linear
        };
    }
}
```

### 11.4 AnimationDriver

```csharp
// src/ClassicUO.Client/Game/UI/HtmlGumps/Animation/AnimationDriver.cs
public class AnimationDriver
{
    private List<ActiveAnimation> _animations = new();
    public bool HasActiveAnimations => _animations.Any();
    
    /// <summary>
    /// Inicia transitions automaticamente quando o estilo de um nó muda.
    /// </summary>
    public void StartTransitions(HtmlNode node, ComputedStyle oldStyle, ComputedStyle newStyle)
    {
        foreach (var rule in newStyle.Transitions)
        {
            var tween = CreateTween(node, rule, oldStyle, newStyle);
            if (tween != null)
            {
                _animations.Add(new ActiveAnimation { Node = node, Tween = tween });
            }
        }
    }
    
    /// <summary>
    /// Inicia um @keyframes animation declarado em CSS.
    /// </summary>
    public void StartKeyframeAnimation(HtmlNode node, AnimationRule rule)
    {
        var keyframes = KeyframesRegistry.Get(rule.Name);
        if (keyframes == null) return;
        
        foreach (var kf in keyframes.Keyframes)
        {
            // Cria um tween que vai do keyframe N para o N+1
            var tween = new KeyframeTween 
            {
                Node = node,
                Keyframes = keyframes,
                Duration = rule.Duration,
                IterationCount = rule.IterationCount,
                Easing = Easing.Parse(rule.TimingFunction)
            };
            _animations.Add(new ActiveAnimation { Node = node, Tween = tween });
        }
    }
    
    public void Tick(float deltaSeconds)
    {
        for (int i = _animations.Count - 1; i >= 0; i--)
        {
            var anim = _animations[i];
            anim.Tween.Tick(deltaSeconds);
            anim.Tween.Apply(anim.Node.ComputedStyle);
            
            if (anim.Tween.IsComplete && !anim.Tween.IsLoop)
            {
                _animations.RemoveAt(i);
            }
        }
    }
}
```

### 11.5 @keyframes

```csharp
// src/ClassicUO.Client/Game/UI/HtmlGumps/Animation/Keyframes.cs
public class Keyframes
{
    public string Name;
    public List<Keyframe> Frames = new();
}

public class Keyframe
{
    public float Position; // 0.0 a 1.0 (0% a 100%)
    public Dictionary<string, string> Properties = new();
}

public static class KeyframesRegistry
{
    private static Dictionary<string, Keyframes> _registry = new();
    
    public static void Register(string name, Keyframes kf) => _registry[name] = kf;
    public static Keyframes Get(string name) => _registry.GetValueOrDefault(name);
    
    public static void ParseFromStylesheet(ICssStyleSheet sheet)
    {
        foreach (var rule in sheet.Rules)
        {
            if (rule is ICssKeyframesRule kfRule)
            {
                var kf = new Keyframes { Name = kfRule.Name };
                foreach (ICssKeyframeRule frame in kfRule.Rules.OfType<ICssKeyframeRule>())
                {
                    var f = new Keyframe();
                    f.Position = ParsePosition(frame.KeyText);
                    foreach (var prop in frame.Style)
                    {
                        f.Properties[prop.Name] = prop.Value;
                    }
                    kf.Frames.Add(f);
                }
                Register(kfRule.Name, kf);
            }
        }
    }
}
```

### 11.6 Critérios de aceite da Fase 6

- [ ] `transition: background-color 0.3s ease-out` anima color suavemente em `:hover`
- [ ] `@keyframes pulse { 50% { transform: scale(1.1) } }` + `animation: pulse 2s infinite` faz elemento pulsar
- [ ] Múltiplas transitions simultâneas em propriedades diferentes
- [ ] `animation-delay` atrasa início
- [ ] `animation-iteration-count: infinite` repete indefinidamente
- [ ] Cubic-bezier customizado funciona

---

## 12. Fase 7 — Efeitos visuais ricos (shaders)

### 12.1 Objetivo da fase

Implementar `box-shadow`, `filter: drop-shadow()`, `filter: blur()`, `filter: brightness()`, e transforms 3D usando shaders FNA. Esta é a fase que transforma a UI de "competente" para "deslumbrante".

**Esforço estimado:** 7-10 dias.
**Pré-requisito:** familiaridade com shaders FNA (você já tem essa expertise).

### 12.2 Drop-shadow shader

Drop-shadow CSS é diferente de box-shadow: ele segue o **shape** do elemento (incluindo transparência da imagem), não apenas a bounding box.

**Algoritmo:**
1. Renderiza o elemento em um RenderTarget separado
2. Aplica gaussian blur na alpha channel
3. Tinge com a cor da sombra
4. Compõe deslocado por (offsetX, offsetY) atrás do elemento original

```hlsl
// src/ClassicUO.Client/Game/UI/HtmlGumps/Shaders/DropShadow.fx
sampler2D InputTexture : register(s0);
float4 ShadowColor;
float2 BlurDirection;
float BlurRadius;
float2 TexelSize;

float4 GaussianBlurPass(float2 uv) : COLOR0
{
    // 9-tap Gaussian
    float weights[9] = { 0.0162, 0.0540, 0.1216, 0.1946, 0.2270, 0.1946, 0.1216, 0.0540, 0.0162 };
    
    float4 sum = float4(0, 0, 0, 0);
    for (int i = -4; i <= 4; i++)
    {
        float2 offset = BlurDirection * TexelSize * (i * BlurRadius);
        sum += tex2D(InputTexture, uv + offset) * weights[i + 4];
    }
    
    // Tinge com a cor da sombra mantendo o alpha
    return float4(ShadowColor.rgb, sum.a * ShadowColor.a);
}

technique DropShadow
{
    pass P0 { PixelShader = compile ps_3_0 GaussianBlurPass(); }
}
```

```csharp
// src/ClassicUO.Client/Game/UI/HtmlGumps/Render/EffectsPipeline.cs
public class EffectsPipeline
{
    private Effect _dropShadowEffect;
    private RenderTargetPool _rtPool;
    
    public void RenderDropShadow(UltimaBatcher2D batcher, RenderTarget2D source, 
                                  Vector2 position, DropShadowEffect effect)
    {
        // Pass 1: blur horizontal
        var horizontalBlur = _rtPool.Rent(source.Width, source.Height);
        batcher.GraphicsDevice.SetRenderTarget(horizontalBlur);
        _dropShadowEffect.Parameters["BlurDirection"].SetValue(new Vector2(1, 0));
        _dropShadowEffect.Parameters["BlurRadius"].SetValue(effect.BlurRadius);
        _dropShadowEffect.Parameters["ShadowColor"].SetValue(effect.Color.ToVector4());
        _dropShadowEffect.CurrentTechnique.Passes[0].Apply();
        batcher.Draw(source, Vector2.Zero, Color.White);
        
        // Pass 2: blur vertical
        var verticalBlur = _rtPool.Rent(source.Width, source.Height);
        batcher.GraphicsDevice.SetRenderTarget(verticalBlur);
        _dropShadowEffect.Parameters["BlurDirection"].SetValue(new Vector2(0, 1));
        _dropShadowEffect.CurrentTechnique.Passes[0].Apply();
        batcher.Draw(horizontalBlur, Vector2.Zero, Color.White);
        
        // Compor: desenha o blur deslocado, depois o original em cima
        batcher.GraphicsDevice.SetRenderTarget(null);
        batcher.Draw(verticalBlur, position + new Vector2(effect.OffsetX, effect.OffsetY), Color.White);
        batcher.Draw(source, position, Color.White);
    }
}
```

### 12.3 Box-shadow

Box-shadow é mais simples: é um retângulo desfocado, sem precisar do shape do elemento:

```csharp
public void RenderBoxShadow(UltimaBatcher2D batcher, LayoutBox box, int x, int y, BoxShadow shadow)
{
    // Desenha um rect ligeiramente maior que a box, com blur
    // Pode ser pré-renderizado e cacheado
    var key = new BoxShadowKey 
    {
        Width = (int)box.Width,
        Height = (int)box.Height,
        Blur = shadow.Blur,
        Spread = shadow.Spread,
        Color = shadow.Color,
        Inset = shadow.Inset
    };
    
    var texture = BoxShadowCache.GetOrCreate(key, () => GenerateBoxShadowTexture(key));
    
    batcher.Draw(texture, 
        new Rectangle(
            x + (int)shadow.OffsetX - (int)shadow.Blur,
            y + (int)shadow.OffsetY - (int)shadow.Blur,
            (int)box.Width + (int)shadow.Blur * 2,
            (int)box.Height + (int)shadow.Blur * 2),
        Color.White);
}
```

### 12.4 Glow effect

Glow é drop-shadow com offset zero e cor da própria fonte de luz:

```css
.slot.ready {
  box-shadow: 0 0 12px cyan;  /* glow simples */
  filter: drop-shadow(0 0 8px cyan); /* glow no shape */
}
```

### 12.5 Transforms 3D

Para `transform: rotateY(45deg)` precisamos de matriz 3D:

```csharp
// src/ClassicUO.Client/Game/UI/HtmlGumps/Style/Transform.cs
public struct Transform
{
    public Matrix Matrix;
    public bool Has3D;
    
    public static Transform Identity => new() { Matrix = Matrix.Identity };
    public bool IsIdentity => Matrix == Matrix.Identity;
    
    public static Transform RotateX(float degrees)
    {
        return new Transform 
        { 
            Matrix = Matrix.CreateRotationX(MathHelper.ToRadians(degrees)),
            Has3D = true
        };
    }
    
    public static Transform RotateY(float degrees) { /* ... */ }
    public static Transform RotateZ(float degrees) { /* ... */ }
    public static Transform Translate(float x, float y) { /* ... */ }
    public static Transform Scale(float x, float y) { /* ... */ }
    
    public static Transform Lerp(Transform from, Transform to, float t)
    {
        // Decompose, interpola componentes, recompose
        // Ou usa Matrix.Lerp se for 2D
        return new Transform { Matrix = Matrix.Lerp(from.Matrix, to.Matrix, t) };
    }
}
```

```csharp
// Aplicação no render
public void PushTransform(UltimaBatcher2D batcher, Transform t, TransformOrigin origin, LayoutBox box)
{
    var originPx = ResolveOrigin(origin, box);
    
    // Translade para origin, aplica transform, translade de volta
    var matrix = Matrix.CreateTranslation(-originPx.X, -originPx.Y, 0)
               * t.Matrix
               * Matrix.CreateTranslation(originPx.X, originPx.Y, 0);
    
    batcher.End();
    batcher.Begin(transformMatrix: matrix);
}
```

### 12.6 Critérios de aceite da Fase 7

- [ ] `box-shadow: 0 4px 12px rgba(0,0,0,0.5)` desenha sombra desfocada atrás do elemento
- [ ] `filter: drop-shadow(0 0 8px gold)` desenha glow seguindo shape (incluindo PNG transparente)
- [ ] `filter: blur(8px)` desfoca o elemento e seus filhos
- [ ] `filter: brightness(1.5)` clareia
- [ ] `transform: rotateY(45deg)` rotaciona em 3D com perspectiva
- [ ] Glow pulsante via `@keyframes` funciona suavemente

---

## 13. Fase 8 — SVG inline (SkiaSharp)

### 13.1 Objetivo da fase

Renderizar `<svg>` inline no HTML, suportando paths, shapes básicos, gradients, e animações SVG.

**Esforço estimado:** 5-8 dias.

### 13.2 Por que SkiaSharp

Renderizar SVG corretamente envolve:
- Parsing de path data (`M 10 10 L 50 50 Q 100 0 150 50 ...`)
- Tessellation de bezier curves para triângulos GPU-friendly
- Gradients (linear, radial)
- Stroke caps, joins, dash patterns
- Transformações

Implementar isso do zero seriam meses. SkiaSharp resolve em ~10 linhas — é o mesmo motor do Chrome.

### 13.3 Pipeline

```csharp
// src/ClassicUO.Client/Game/UI/HtmlGumps/Render/SkiaRenderer.cs
using SkiaSharp;

public class SkiaRenderer
{
    private GraphicsDevice _gd;
    
    public void RenderSvg(UltimaBatcher2D batcher, int x, int y, LayoutBox box, HtmlNode svgNode)
    {
        var width = (int)box.Width;
        var height = (int)box.Height;
        
        // Cache: textura por (svg content hash, animation frame)
        var key = ComputeCacheKey(svgNode, AnimationDriver.CurrentFrame);
        if (SvgTextureCache.TryGet(key, out var cached))
        {
            batcher.Draw(cached, new Rectangle(x, y, width, height), Color.White);
            return;
        }
        
        // Renderiza no Skia
        using var bitmap = new SKBitmap(width, height);
        using var canvas = new SKCanvas(bitmap);
        canvas.Clear(SKColors.Transparent);
        
        RenderSvgNode(canvas, svgNode, width, height);
        
        // Converte SKBitmap -> Texture2D do FNA
        var texture = ConvertToTexture(bitmap);
        SvgTextureCache.Set(key, texture);
        
        batcher.Draw(texture, new Rectangle(x, y, width, height), Color.White);
    }
    
    private void RenderSvgNode(SKCanvas canvas, HtmlNode node, int width, int height)
    {
        var viewBox = ParseViewBox(node.GetAttribute("viewBox"));
        var scaleX = width / viewBox.Width;
        var scaleY = height / viewBox.Height;
        canvas.Scale(scaleX, scaleY);
        
        foreach (var child in node.Children)
        {
            RenderSvgElement(canvas, child);
        }
    }
    
    private void RenderSvgElement(SKCanvas canvas, HtmlNode element)
    {
        switch (element.TagName)
        {
            case "circle":
                var cx = float.Parse(element.GetAttribute("cx") ?? "0");
                var cy = float.Parse(element.GetAttribute("cy") ?? "0");
                var r = float.Parse(element.GetAttribute("r") ?? "0");
                using (var paint = ResolvePaint(element))
                    canvas.DrawCircle(cx, cy, r, paint);
                break;
                
            case "path":
                var d = element.GetAttribute("d");
                using (var path = SKPath.ParseSvgPathData(d))
                using (var paint = ResolvePaint(element))
                    canvas.DrawPath(path, paint);
                break;
                
            case "rect":
                // ...
                break;
                
            case "line":
                // ...
                break;
                
            case "g":
                // Group: aplicar transform, recursão
                var transform = element.GetAttribute("transform");
                if (!string.IsNullOrEmpty(transform))
                {
                    canvas.Save();
                    ApplyTransform(canvas, transform);
                    foreach (var child in element.Children)
                        RenderSvgElement(canvas, child);
                    canvas.Restore();
                }
                else
                {
                    foreach (var child in element.Children)
                        RenderSvgElement(canvas, child);
                }
                break;
        }
    }
    
    private SKPaint ResolvePaint(HtmlNode element)
    {
        var paint = new SKPaint { IsAntialias = true };
        
        var fill = element.GetAttribute("fill") ?? element.ComputedStyle.Fill;
        var stroke = element.GetAttribute("stroke") ?? element.ComputedStyle.Stroke;
        var strokeWidth = float.Parse(element.GetAttribute("stroke-width") ?? "1");
        
        if (!string.IsNullOrEmpty(fill) && fill != "none")
        {
            paint.Style = SKPaintStyle.Fill;
            paint.Color = ParseSkColor(fill);
        }
        else if (!string.IsNullOrEmpty(stroke))
        {
            paint.Style = SKPaintStyle.Stroke;
            paint.Color = ParseSkColor(stroke);
            paint.StrokeWidth = strokeWidth;
        }
        
        return paint;
    }
    
    private Texture2D ConvertToTexture(SKBitmap bitmap)
    {
        var texture = new Texture2D(_gd, bitmap.Width, bitmap.Height);
        var pixels = bitmap.Pixels;
        var data = new uint[pixels.Length];
        for (int i = 0; i < pixels.Length; i++)
        {
            var c = pixels[i];
            // Skia ARGB -> FNA RGBA
            data[i] = (uint)((c.Alpha << 24) | (c.Blue << 16) | (c.Green << 8) | c.Red);
        }
        texture.SetData(data);
        return texture;
    }
}
```

### 13.4 Animações SVG

SVG suporta `<animate>` e `<animateTransform>` declarativos:

```xml
<svg viewBox="0 0 100 100">
  <circle cx="50" cy="50" r="30" stroke="gold">
    <animate attributeName="r" values="20;40;20" dur="3s" repeatCount="indefinite"/>
  </circle>
</svg>
```

Implementação: ler atributos `<animate>`, registrar no AnimationDriver, atualizar atributo a cada frame, e invalidar o cache de textura SVG (re-render):

```csharp
public class SvgAnimation
{
    public HtmlNode TargetElement;
    public string AttributeName;
    public List<float> Values;
    public float Duration;
    public bool Repeat;
    public float Elapsed;
    
    public void Tick(float delta)
    {
        Elapsed += delta;
        if (Repeat) Elapsed %= Duration;
        else Elapsed = Math.Min(Elapsed, Duration);
        
        var t = Elapsed / Duration;
        var current = InterpolateValues(Values, t);
        TargetElement.SetAttribute(AttributeName, current.ToString());
        
        // Invalida cache da textura
        SvgTextureCache.InvalidateForNode(TargetElement);
    }
}
```

### 13.5 Critérios de aceite da Fase 8

- [ ] `<svg><circle/></svg>` desenha círculo
- [ ] `<path d="M..."/>` renderiza paths complexos com bezier
- [ ] `<linearGradient>` e `<radialGradient>` aplicam corretamente
- [ ] `<animate>` modifica atributos no tempo
- [ ] `<animateTransform>` rotaciona/escala elementos
- [ ] Cache de textura evita re-render quando nada muda

---

## 14. Fase 9 — Backdrop-filter (vidro fosco)

### 14.1 Objetivo da fase

Implementar `backdrop-filter: blur(12px)` — o efeito "vidro fosco" que aplica blur ao que está atrás do elemento, não no elemento em si. Visualmente impressionante para tooltips e overlays.

**Esforço estimado:** 6-9 dias (é o mais complexo dos efeitos).

### 14.2 Algoritmo

```
1. Antes de desenhar o elemento com backdrop-filter:
   a. Snapshot do que já foi desenhado na região do elemento
      (gameworld + UI atrás)
   b. Esse snapshot vai pra um RenderTarget temporário
2. Aplica gaussian blur no snapshot
3. Desenha o snapshot blurred na posição original
4. Desenha o elemento normalmente por cima (com seu background semi-transparente)
```

### 14.3 Implementação

```csharp
// src/ClassicUO.Client/Game/UI/HtmlGumps/Render/BackdropFilter.cs
public class BackdropFilterRenderer
{
    private Effect _gaussianBlurEffect;
    private RenderTargetPool _rtPool;
    
    public void Apply(UltimaBatcher2D batcher, int x, int y, int width, int height, BackdropFilter filter)
    {
        // 1. Snapshot do framebuffer atual na região
        var snapshot = _rtPool.Rent(width, height);
        var sourceRect = new Rectangle(x, y, width, height);
        
        batcher.End();
        var currentRT = batcher.GraphicsDevice.GetRenderTargets().FirstOrDefault();
        
        // Copia da framebuffer atual para o snapshot
        // Em FNA: usar GraphicsDevice.SetRenderTarget e Draw the back buffer
        // Alternativa: usar GraphicsDevice.GetBackBufferData (lento) 
        // Solução melhor: o sistema todo de gumps deveria já estar renderizando 
        // num RT separado, e copiamos dele
        CopyRegion(batcher.GraphicsDevice, currentRT, snapshot, sourceRect);
        
        // 2. Aplicar blur (two-pass: horizontal, depois vertical)
        var blurred = ApplyTwoPassBlur(batcher.GraphicsDevice, snapshot, filter.BlurRadius);
        
        // 3. Desenhar o blurred na posição original
        batcher.GraphicsDevice.SetRenderTarget(currentRT);
        batcher.Begin();
        batcher.Draw(blurred, sourceRect, Color.White);
        
        // 4. Caller desenha o elemento por cima com background semi-transparente
        
        _rtPool.Return(snapshot);
        _rtPool.Return(blurred);
    }
    
    private RenderTarget2D ApplyTwoPassBlur(GraphicsDevice gd, RenderTarget2D source, float radius)
    {
        var temp1 = _rtPool.Rent(source.Width, source.Height);
        var temp2 = _rtPool.Rent(source.Width, source.Height);
        
        // Horizontal blur
        gd.SetRenderTarget(temp1);
        _gaussianBlurEffect.Parameters["Direction"].SetValue(new Vector2(1, 0));
        _gaussianBlurEffect.Parameters["Radius"].SetValue(radius);
        _gaussianBlurEffect.Parameters["TexelSize"].SetValue(new Vector2(1f / source.Width, 1f / source.Height));
        _gaussianBlurEffect.CurrentTechnique.Passes[0].Apply();
        // ...batcher draws source to temp1
        
        // Vertical blur
        gd.SetRenderTarget(temp2);
        _gaussianBlurEffect.Parameters["Direction"].SetValue(new Vector2(0, 1));
        _gaussianBlurEffect.CurrentTechnique.Passes[0].Apply();
        // ...batcher draws temp1 to temp2
        
        _rtPool.Return(temp1);
        return temp2;
    }
}
```

### 14.4 Otimização: cache temporal

Backdrop-filter é caro. Otimização: se o conteúdo atrás do elemento não mudou (gameworld está parado, sem outras gumps movendo), reusa o blur do frame anterior:

```csharp
public class BackdropCache
{
    private Dictionary<Rectangle, CachedBackdrop> _cache = new();
    
    public RenderTarget2D GetOrCompute(GraphicsDevice gd, Rectangle region, float blurRadius, Func<RenderTarget2D> compute)
    {
        if (_cache.TryGetValue(region, out var cached) && 
            cached.BlurRadius == blurRadius && 
            !HasGameworldChanged(cached.Frame))
        {
            return cached.Texture;
        }
        
        var fresh = compute();
        _cache[region] = new CachedBackdrop 
        { 
            Texture = fresh, 
            BlurRadius = blurRadius, 
            Frame = AnimationDriver.CurrentFrame 
        };
        return fresh;
    }
}
```

### 14.5 Critérios de aceite da Fase 9

- [ ] `backdrop-filter: blur(12px)` em tooltip aplica blur visível ao gameworld atrás
- [ ] Performance: cache evita recomputar quando nada muda
- [ ] `backdrop-filter: blur(20px) saturate(1.5)` combina filtros
- [ ] Funciona corretamente com `border-radius` (blur só dentro do shape)

---

## 15. Fase 10 — Hot reload e theming

### 15.1 Objetivo da fase

Permitir editar arquivos `.html` e `.css` com o cliente aberto e ver mudanças em tempo real. Adicionar comando in-game para trocar tema.

**Esforço estimado:** 2-4 dias.

### 15.2 FileSystemWatcher

```csharp
// src/ClassicUO.Client/Game/UI/HtmlGumps/HtmlGumpLoader.cs
public class HtmlGumpLoader
{
    private FileSystemWatcher _htmlWatcher;
    private FileSystemWatcher _cssWatcher;
    private readonly ConcurrentQueue<string> _pendingReloads = new();
    
    public event Action<string> OnFileChanged;
    
    public HtmlGumpLoader(string htmlDir, string themesDir)
    {
        _htmlWatcher = new FileSystemWatcher(htmlDir, "*.html") 
        { 
            NotifyFilter = NotifyFilters.LastWrite | NotifyFilters.FileName,
            EnableRaisingEvents = true,
            IncludeSubdirectories = true
        };
        _htmlWatcher.Changed += (s, e) => _pendingReloads.Enqueue(e.FullPath);
        
        _cssWatcher = new FileSystemWatcher(themesDir, "*.css") 
        { 
            NotifyFilter = NotifyFilters.LastWrite,
            EnableRaisingEvents = true,
            IncludeSubdirectories = true
        };
        _cssWatcher.Changed += (s, e) => _pendingReloads.Enqueue(e.FullPath);
    }
    
    public bool HasChanges() => !_pendingReloads.IsEmpty;
    
    public void ProcessChanges()
    {
        while (_pendingReloads.TryDequeue(out var path))
        {
            // Debounce: aguardar 100ms para evitar reload em meio a write
            Task.Delay(100).Wait();
            OnFileChanged?.Invoke(path);
        }
    }
}
```

### 15.3 Comando in-game

```csharp
// Registro no sistema de comandos do TazUO
CommandManager.Register(".theme", args =>
{
    if (args.Length == 0)
    {
        GameActions.Print("Tema atual: " + ThemeManager.CurrentTheme);
        return;
    }
    
    var themeName = args[0];
    var path = $"Data/Themes/{themeName}.css";
    if (File.Exists(path))
    {
        ThemeManager.SwitchTo(themeName);
        GameActions.Print($"Tema alterado para: {themeName}");
    }
    else
    {
        GameActions.Print($"Tema não encontrado: {themeName}");
    }
});

CommandManager.Register(".reloadui", args =>
{
    HtmlGumpManager.ReloadAll();
    GameActions.Print("UI recarregada.");
});
```

### 15.4 ThemeManager

```csharp
public static class ThemeManager
{
    public static string CurrentTheme { get; private set; } = "default";
    public static event Action<string> ThemeChanged;
    
    public static void SwitchTo(string themeName)
    {
        CurrentTheme = themeName;
        ThemeChanged?.Invoke(themeName);
        
        // Todos os HtmlGumps abertos recebem o evento e reaplicam estilo
    }
}
```

No HTML:

```html
<gump theme="{{currentTheme}}">
  <link rel="stylesheet" href="../Themes/{{currentTheme}}.css"/>
  <!-- conteúdo -->
</gump>
```

Quando o tema muda, o `{{currentTheme}}` é re-resolvido, o `<link>` aponta para outro CSS, todo o estilo é recalculado.

### 15.5 Critérios de aceite da Fase 10

- [ ] Edita `hotbar.html` no VS Code, salva, e o gump aberto no jogo redesenha em < 200ms
- [ ] Edita `valtheris.css`, e todos os gumps com tema Valtheris atualizam
- [ ] Comando `.theme polski` troca o tema globalmente
- [ ] Comando `.reloadui` força recarregamento de todos os gumps abertos

---

## 16. Integração com servidor (POL/ServUO/ModernUO)

### 16.1 Cenários

Existem dois modos de uso do HtmlGump:

**Modo A — Cliente lê arquivo local:** o cliente carrega `Data/HtmlGumps/hotbar.html`. Útil para UI estática (paperdoll, status, journal, hotbar). Servidor não envia HTML, só envia dados via packets normais que o cliente bind via `{{var}}`.

**Modo B — Servidor envia HTML inline:** análogo ao gump system clássico, mas o payload é HTML em vez de comandos `AddBackground` etc. Útil para gumps dinâmicos que dependem do contexto (vendor, NPC dialog, container customizado).

### 16.2 Packet protocol (Modo B)

Definimos um novo packet `0xFA` (não usado atualmente) ou estendemos o `0xDD` (compressed gump) com um marcador:

```
Packet 0xFA — Send HtmlGump

Byte    Field
0       Packet ID (0xFA)
1-2     Length
3-6     Gump ID (uint, identifies this gump instance)
7-10    Server Serial (uint)
11-14   X position
15-18   Y position
19-22   HTML payload length
23-N    HTML payload (UTF-8)
N+1-M   CSS payload length (uint)
M+1-K   CSS payload (UTF-8, optional)
```

### 16.3 Lado servidor — exemplo ModernUO

```csharp
// Servidor: ModernUO ou ServUO
public class HtmlGumpPacket : Packet
{
    public HtmlGumpPacket(uint gumpId, Mobile target, int x, int y, string html, string css = null)
        : base(0xFA)
    {
        EnsureCapacity(64);
        Stream.Write(gumpId);
        Stream.Write(target.Serial);
        Stream.Write((short)x);
        Stream.Write((short)y);
        
        var htmlBytes = Encoding.UTF8.GetBytes(html);
        Stream.Write((uint)htmlBytes.Length);
        Stream.Write(htmlBytes);
        
        if (!string.IsNullOrEmpty(css))
        {
            var cssBytes = Encoding.UTF8.GetBytes(css);
            Stream.Write((uint)cssBytes.Length);
            Stream.Write(cssBytes);
        }
        else
        {
            Stream.Write((uint)0);
        }
    }
}

// Uso no servidor
public class VendorDialog
{
    public static void OpenFor(Mobile customer, BaseVendor vendor)
    {
        var html = $@"
            <gump movable='true' closable='true'>
                <link rel='stylesheet' href='vendor.css'/>
                <div class='vendor-dialog'>
                    <h1>{vendor.Name}</h1>
                    <p>Greetings, {customer.Name}! How may I help you?</p>
                    <div class='actions'>
                        <button data-onclick='Buy'>Buy</button>
                        <button data-onclick='Sell'>Sell</button>
                        <button data-onclick='Close'>Leave</button>
                    </div>
                </div>
            </gump>";
        
        customer.NetState.Send(new HtmlGumpPacket(0x1234, customer, 200, 200, html));
    }
}
```

### 16.4 Lado cliente — handler do packet

```csharp
// src/ClassicUO.Client/Network/Encryption/PacketsHandlers.cs ou similar
PacketHandlers.Handlers[0xFA] = HandleHtmlGump;

private static void HandleHtmlGump(ref StackDataReader p)
{
    var gumpId = p.ReadUInt32BE();
    var serial = p.ReadUInt32BE();
    var x = p.ReadInt16BE();
    var y = p.ReadInt16BE();
    
    var htmlLen = p.ReadUInt32BE();
    var html = p.ReadASCII((int)htmlLen);
    
    var cssLen = p.ReadUInt32BE();
    var css = cssLen > 0 ? p.ReadASCII((int)cssLen) : null;
    
    var gump = new HtmlGump(gumpId, serial)
    {
        X = x, Y = y
    };
    
    gump.LoadHtmlFromString(html);
    if (!string.IsNullOrEmpty(css))
        gump.LoadCssFromString(css);
    
    UIManager.Add(gump);
}
```

### 16.5 Resposta do cliente ao servidor (eventos)

Cliques em botões precisam voltar para o servidor. Reaproveitamos o packet `0xB1` (gump response) ou criamos `0xFB`:

```csharp
ClickHandlerRegistry.Register("Buy", node =>
{
    var packet = new GumpHtmlResponsePacket(currentGumpId, "Buy", node.GetAttribute("data-id"));
    NetworkClient.Send(packet);
});
```

---

## 17. Schema completo de elementos suportados

### 17.1 Elementos estruturais

| Elemento | Comportamento |
|---|---|
| `<gump>` | Root element, define metadata da gump (movable, closable, theme) |
| `<div>` | Container genérico, display: block default |
| `<span>` | Container inline |
| `<section>`, `<article>`, `<header>`, `<footer>`, `<nav>`, `<main>` | Aliases semânticos para `<div>` |

### 17.2 Tipográficos

| Elemento | Comportamento |
|---|---|
| `<h1>`–`<h6>` | Headings com font-size default decrescente |
| `<p>` | Parágrafo, margin vertical default |
| `<b>`, `<strong>` | Bold |
| `<i>`, `<em>` | Italic |
| `<u>` | Underline |
| `<br>` | Line break |
| `<small>`, `<sub>`, `<sup>` | Tipografia variante |

### 17.3 Listas

| Elemento | Comportamento |
|---|---|
| `<ul>`, `<ol>` | Listas com marker default |
| `<li>` | Item de lista |

### 17.4 Imagens e mídia

| Elemento | Comportamento |
|---|---|
| `<img src="..."/>` | Imagem PNG/JPG do disco |
| `<svg>` | Renderizado via SkiaSharp |
| `<canvas>` | NÃO suportado no MVP (sem JS) |

### 17.5 Interativos

| Elemento | Comportamento |
|---|---|
| `<button>` | Botão clicável, `data-onclick` aciona handler |
| `<input type="text">` | Input de texto |
| `<input type="checkbox">` | Checkbox |
| `<input type="range">` | Slider |
| `<a>` | Link clicável (data-href) |
| `<details>`, `<summary>` | Disclosure widget |

### 17.6 Customizados Epic Server

Adicionamos elementos próprios para comodidades específicas:

| Elemento | Comportamento |
|---|---|
| `<gump-icon serial="0x4001"/>` | Renderiza ícone do UO art (gump art) |
| `<item-icon graphic="0x1B72"/>` | Renderiza ícone de item |
| `<paperdoll-slot layer="..."/>` | Slot de paperdoll com item equipado |
| `<status-bar type="hp"/>` | Barra de status conectada ao player |

---

## 18. Schema completo de propriedades CSS

### 18.1 Layout

```
display: block | inline | flex | none
flex-direction: row | column | row-reverse | column-reverse
flex-wrap: nowrap | wrap | wrap-reverse
justify-content: flex-start | flex-end | center | space-between | space-around | space-evenly
align-items: stretch | flex-start | flex-end | center | baseline
gap, row-gap, column-gap: <length>
flex-grow, flex-shrink: <number>
flex-basis: auto | <length> | <percentage>
flex: <shorthand>

position: static | relative | absolute | fixed
top, right, bottom, left: <length> | <percentage>
z-index: <integer>
```

### 18.2 Sizing

```
width, height: auto | <length> | <percentage>
min-width, min-height, max-width, max-height: <length> | <percentage>
```

### 18.3 Spacing

```
margin, padding: <length> [<length>...]
margin-top/right/bottom/left, padding-top/right/bottom/left: <length>
```

### 18.4 Border

```
border: <width> <style> <color>
border-top/right/bottom/left: <shorthand>
border-width, border-style, border-color: <values>
border-radius: <length> [<length>...]
border-top-left-radius, etc.
```

### 18.5 Background

```
background: <shorthand>
background-color: <color>
background-image: url(...) | linear-gradient(...) | radial-gradient(...)
background-size: cover | contain | <length> <length>
background-position: <position>
background-repeat: repeat | no-repeat | repeat-x | repeat-y
```

### 18.6 Tipografia

```
color: <color>
font-family: <family-name>
font-size: <length>
font-weight: 100..900 | normal | bold
font-style: normal | italic
line-height: <number> | <length>
letter-spacing: <length>
text-align: left | center | right | justify
text-shadow: <offset-x> <offset-y> <blur> <color>
text-decoration: none | underline | line-through
```

### 18.7 Effects

```
opacity: <0..1>
filter: blur(<length>) | drop-shadow(...) | brightness(<number>) | saturate(<number>) | hue-rotate(<deg>)
backdrop-filter: blur(<length>) | saturate(<number>)
box-shadow: [inset] <offset-x> <offset-y> <blur> [<spread>] <color>
```

### 18.8 Transform

```
transform: translate(<x>, <y>) | rotate(<deg>) | scale(<x>, <y>) | rotateX/Y/Z(<deg>) | skew(<x>, <y>) | matrix(...)
transform-origin: <x> <y> | center | top left | etc.
perspective: <length>
```

### 18.9 Animation

```
transition: <property> <duration> <timing-function> <delay>
animation: <name> <duration> <timing-function> <delay> <iteration-count> <direction>
animation-iteration-count: infinite | <number>
animation-direction: normal | reverse | alternate | alternate-reverse
@keyframes <name> { 0% { ... } 100% { ... } }
```

### 18.10 Misc

```
cursor: default | pointer | text | move | not-allowed
overflow, overflow-x, overflow-y: visible | hidden | scroll | auto
pointer-events: auto | none
visibility: visible | hidden
```

---

## 19. Performance e otimizações

### 19.1 Princípios

1. **Layout só recalcula em mudanças.** Marcar `IsDirty` em vez de recalcular todo frame.
2. **Style cache.** Mesmas regras + mesmos estados = mesmo `ComputedStyle`. Hash + dicionário.
3. **Texture caching.** Backgrounds com border-radius, box-shadows, SVGs — tudo tem cache por (size, style hash).
4. **Render target pooling.** Nunca criar/destruir RTs por frame.
5. **Animation is the only continuous redraw cost.** Layout/style não mudam todo frame se não há animação ativa.

### 19.2 Métricas alvo

Em hardware moderno (RTX 3060, i7):

| Operação | Alvo | Notas |
|---|---|---|
| Parse HTML 50 elementos | < 5ms | Uma vez no load |
| Parse CSS 100 regras | < 3ms | Uma vez no load |
| Style resolve árvore 50 nós | < 1ms | Apenas em mudanças |
| Layout flexbox 50 nós | < 2ms | Apenas em mudanças |
| Render 50 nós sem efeitos | < 0.5ms | Toda frame |
| Render com 5 box-shadows cacheadas | < 1ms | Toda frame |
| Render com 1 backdrop-filter | < 3ms | Pode cachear |
| Animation tick 10 tweens | < 0.2ms | Toda frame |

**Frame budget total para UI: ~5ms** dos 16.67ms (60fps). O resto é gameworld, network, sistema.

### 19.3 Profiling

Adicionar instrumentação (`Stopwatch`) em pontos críticos:

```csharp
public class HtmlGumpProfiler
{
    public TimeSpan ParseTime;
    public TimeSpan StyleResolveTime;
    public TimeSpan LayoutTime;
    public TimeSpan AnimationTime;
    public TimeSpan RenderTime;
    
    public void Print()
    {
        Console.WriteLine($"Parse: {ParseTime.TotalMilliseconds:F2}ms");
        Console.WriteLine($"Style: {StyleResolveTime.TotalMilliseconds:F2}ms");
        Console.WriteLine($"Layout: {LayoutTime.TotalMilliseconds:F2}ms");
        Console.WriteLine($"Animation: {AnimationTime.TotalMilliseconds:F2}ms");
        Console.WriteLine($"Render: {RenderTime.TotalMilliseconds:F2}ms");
    }
}
```

Comando in-game `.htmlprof` toggle.

---

## 20. Roadmap e cronograma

### 20.1 Cronograma agregado

Assumindo dedicação focada (~30h/semana):

| Fase | Descrição | Duração | Acumulado |
|---|---|---|---|
| 1 | Fundação: parsing, DOM, data binding | 5-8 dias | 1-2 semanas |
| 2 | Layout flexbox engine | 8-12 dias | 3-4 semanas |
| 3 | CSS cascade e style resolution | 6-9 dias | 4-5 semanas |
| 4 | Render layer básico | 6-10 dias | 6-7 semanas |
| 5 | Estados e interatividade | 4-6 dias | 7 semanas |
| 6 | Animations e transitions | 5-7 dias | 8-9 semanas |
| 7 | Efeitos visuais ricos (shaders) | 7-10 dias | 9-10 semanas |
| 8 | SVG inline (SkiaSharp) | 5-8 dias | 10-11 semanas |
| 9 | Backdrop-filter | 6-9 dias | 11-12 semanas |
| 10 | Hot reload e theming | 2-4 dias | 12 semanas |
| Buffer | Bug fixing, integração | 1-2 semanas | 13-14 semanas |
| **Total** | | **~14 semanas** | **~3.5 meses** |

### 20.2 Marcos de demo

**Marco 1 (fim da semana 4):** primeiro `<div style="background:red">Hello</div>` aparece dentro do TazUO.

**Marco 2 (fim da semana 7):** hotbar HTML funcional com hover, click, e estados visuais.

**Marco 3 (fim da semana 10):** três gumps clássicos do TazUO migrados (status, journal, paperdoll) com animações suaves.

**Marco 4 (fim da semana 14):** sistema completo, drop-shadow + glow + 3D transforms + SVG telegraphs + vidro fosco + temas Valtheris/Polski/Default.

### 20.3 Riscos identificados

| Risco | Probabilidade | Impacto | Mitigação |
|---|---|---|---|
| Performance de layout em gumps complexos | Média | Alto | Profile cedo, otimize iterativamente |
| Bugs sutis em flex algorithm | Alta | Médio | Casos de teste extensivos, comparar com Chrome |
| Complexidade do backdrop-filter no FNA | Alta | Médio | Implementar versão simples primeiro, otimizar depois |
| AngleSharp.Css não cobre algum CSS exótico | Baixa | Baixo | Implementar parser próprio para casos faltantes |
| FNA + SkiaSharp interop | Média | Médio | Testar texture conversion cedo |
| Cross-platform (Windows/Mac/Linux) | Média | Médio | Testar em todas plataformas a cada milestone |

---

## 21. Apêndices

### 21.1 Apêndice A: exemplo de hotbar completa

**Data/HtmlGumps/hotbar.html:**

```html
<gump id="hotbar" movable="true" closable="false">
    <link rel="stylesheet" href="../Themes/_base.css"/>
    <link rel="stylesheet" href="hotbar.css"/>
    
    <div class="hotbar">
        <button class="slot {{slot1.state}}" data-key="1" data-onclick="UseSkill" data-skill="{{slot1.skill}}">
            <gump-icon serial="{{slot1.iconSerial}}"/>
            <span class="slot__key">1</span>
            <div class="slot__cooldown" style="--cd: {{slot1.cooldownProgress}}%"></div>
        </button>
        <button class="slot {{slot2.state}}" data-key="2" data-onclick="UseSkill" data-skill="{{slot2.skill}}">
            <gump-icon serial="{{slot2.iconSerial}}"/>
            <span class="slot__key">2</span>
        </button>
        <!-- ... mais 6 slots ... -->
    </div>
</gump>
```

**Data/HtmlGumps/hotbar.css:**

```css
.hotbar {
    display: flex;
    gap: 6px;
    padding: 8px;
    background: var(--ep-bg-deep);
    border: 1px solid var(--ep-border-dark);
    box-shadow: inset 0 2px 8px rgba(0,0,0,0.6), 0 4px 16px rgba(0,0,0,0.5);
}

.slot {
    width: 56px;
    height: 56px;
    background: radial-gradient(circle at 30% 30%, var(--ep-bg-mid), var(--ep-bg-deep));
    border: 2px solid var(--ep-border-dark);
    position: relative;
    transition: border-color 0.2s, transform 0.1s, box-shadow 0.2s;
    cursor: pointer;
}

.slot:hover {
    border-color: var(--ep-gold);
    transform: translateY(-2px);
    box-shadow: 0 4px 12px rgba(201,169,97,0.4);
}

.slot.ready {
    border-color: var(--ep-rune-glow);
    box-shadow: 0 0 12px rgba(107,163,212,0.5), inset 0 0 8px rgba(107,163,212,0.2);
    animation: pulse-ready 2s ease-in-out infinite;
}

.slot__key {
    position: absolute;
    top: 2px;
    left: 4px;
    font-family: var(--ep-font-mono);
    font-size: 11px;
    color: var(--ep-gold);
    text-shadow: 1px 1px 2px black;
    font-weight: 600;
}

.slot__cooldown {
    position: absolute;
    inset: 0;
    background: rgba(0,0,0,0.7);
    pointer-events: none;
}

.slot__cooldown::before {
    content: '';
    position: absolute;
    bottom: 0; left: 0; right: 0;
    background: rgba(114,47,55,0.4);
    height: var(--cd);
    transition: height 0.1s linear;
}

@keyframes pulse-ready {
    50% { 
        box-shadow: 0 0 20px rgba(107,163,212,0.8), inset 0 0 12px rgba(107,163,212,0.3); 
    }
}
```

**C# de inicialização:**

```csharp
public class HotbarGump : HtmlGump
{
    public HotbarGump() : base(0xH0TB, 0)
    {
        ClickHandlerRegistry.Register("UseSkill", node => 
        {
            var skill = node.GetAttribute("data-skill");
            World.Player.UseSkill(skill);
        });
        
        LoadHtml("Data/HtmlGumps/hotbar.html");
        
        // Atualizar dados a cada tick
        OnTick += UpdateData;
    }
    
    private void UpdateData()
    {
        for (int i = 1; i <= 8; i++)
        {
            var skill = Player.Hotbar[i - 1];
            SetData($"slot{i}", new
            {
                state = skill.IsReady ? "ready" : skill.IsCoolingDown ? "cooldown" : "",
                skill = skill.Name,
                iconSerial = skill.IconSerial,
                cooldownProgress = skill.CooldownProgress
            });
        }
    }
}
```

### 21.2 Apêndice B: estrutura de testes

```
tests/
├── ClassicUO.UnitTests/
│   └── HtmlGumps/
│       ├── ParserTests.cs
│       ├── StyleResolverTests.cs
│       ├── FlexLayoutTests.cs
│       ├── AnimationTests.cs
│       └── HitTesterTests.cs
└── ClassicUO.IntegrationTests/
    └── HtmlGumps/
        ├── HotbarRenderTests.cs
        ├── PartyHealthBarTests.cs
        └── HotReloadTests.cs
```

### 21.3 Apêndice C: glossário

| Termo | Significado |
|---|---|
| **Gump** | Janela/UI element no Ultima Online classic |
| **DOM** | Document Object Model — árvore de elementos do HTML |
| **CSSOM** | CSS Object Model — representação parseada do CSS |
| **Cascade** | Algoritmo CSS de aplicação de regras conforme especificidade |
| **Computed Style** | Estilo final resolvido para um elemento, após cascade e herança |
| **Layout Box** | Bounding box calculada (X, Y, W, H) com margin/border/padding |
| **Flex** | Modelo de layout unidirecional do CSS |
| **Render Target** | Textura GPU usada como destino temporário de desenho |
| **Tween** | Interpolação animada entre dois valores |
| **Easing** | Função que curva o tempo da interpolação (ease-in, ease-out, etc.) |
| **Specificity** | Peso de um seletor CSS para resolver conflitos |
| **Pseudo-class** | Estado virtual de elemento (:hover, :active, :focus) |
| **TazUO** | Fork do ClassicUO com features de QoL |
| **FNA** | Framework cross-platform alternativo ao XNA, usado pelo TazUO |
| **AngleSharp** | Lib .NET de parsing HTML5/CSS3 |
| **SkiaSharp** | Binding .NET do motor gráfico Skia |

### 21.3b Apêndice E: O que não construir agora (KISS — features diferidas)

Estas features estão **documentadas nas fases acima mas intencionalmente adiadas**.
Não implementar até que haja um caso de uso concreto no Epic Server.

| Fase do doc | Feature | Motivo para adiar |
|---|---|---|
| Fase 7 | Shaders (drop-shadow, glow, transform 3D) | Zero gumps precisam disso. Adicionar só quando um gump específico exigir. |
| Fase 8 | SVG / SkiaSharp | Dependência pesada (~15 MB). Nenhum caso de uso urgente. Nenhum gump atual usa SVG. |
| Fase 9 | Backdrop-filter (vidro fosco) | Efeito visual opcional — custo alto de render target. Adicionar por último. |
| Fase 10 | Hot reload (FileSystemWatcher) | Útil para dev, não bloqueante. Implementar quando os primeiros gumps forem estabilizados. |
| Fase 6 | @keyframes completo | CSS transitions simples (`transition: opacity 0.2s`) cobrem 90% dos casos de UI de jogo. |
| Fase 2 ext | Flexbox wrap + grid | Flexbox linear (row/column) cobre todos os gumps iniciais. Wrap e grid para depois. |
| Fase 3 ext | CSS cascade completo (classes, IDs, especificidade) | Fase A usa apenas inline styles. Adicionar cascade quando mais de 3 gumps estiverem em uso. |
| Fase 5 ext | DataContext + `{{var}}` binding | Adicionar na Fase B assim que o primeiro gump dinâmico (status, hotbar) for migrado. |

**Regra de desbloqueio:** uma feature sai dessa lista quando existe um gump real no Epic Server
que não pode ser construído sem ela. Não antes.

---

### 21.4 Apêndice D: referências externas

- [W3C CSS Flexbox Level 1](https://www.w3.org/TR/css-flexbox-1/) — spec de flexbox
- [W3C CSS Cascading and Inheritance](https://www.w3.org/TR/css-cascade/) — spec de cascade
- [AngleSharp docs](https://anglesharp.github.io/) — documentação da lib
- [SkiaSharp docs](https://learn.microsoft.com/dotnet/api/skiasharp) — documentação
- [Clay UI library](https://github.com/nicbarker/clay) — referência de algoritmo flex em C
- [TazUO repository](https://github.com/jsebold666/TazUO) — fork base
- [FNA documentation](https://fna-xna.github.io/docs/) — framework gráfico
- [MDN CSS Reference](https://developer.mozilla.org/docs/Web/CSS/Reference) — referência geral CSS

---

**Fim do documento.**

*Esta documentação especifica o sistema HtmlGump completo para o fork TazUO do Epic Server. É um documento vivo: atualize-o conforme o desenvolvimento progride, ajuste estimativas de tempo conforme a realidade dos bugs encontrados, e expanda os apêndices com exemplos reais de gumps migrados.*

