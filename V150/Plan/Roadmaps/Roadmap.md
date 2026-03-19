# Lightning Engine — Roadmap v0.15.0

Migração de v0.14.0 (GLFW + OpenGL) para v0.15.0 (SDL3).

## Proposta

Engine rápida com acesso direto aos sistemas de compilação, renderização e execução de jogos.
Leve, simples, multiplataforma. API intuitiva.

- MultiRender: DX11, DX12, Vulkan, OpenGL
- Multiplataforma: Windows, Linux, macOS (via recompilação)
- Script engine com suporte a C#
- GUI construída com a própria engine

## Objetivo

O Objetivo da versao 0.15.0 é criar a base da engine, com os sistemas de renderização, execução e script funcionando, e um editor básico para criar e rodar projetos de exemplo e a mais importante. Essa versao deve concluir a base estrutural da engine e permitir a criação de projetos de exemplo, mas e precisa incluir todas as features planejadas para a versão final. O foco é criar uma base sólida e funcional, que possa ser expandida e melhorada nas versões futuras.

- [x] Criar a base da engine (renderização, execução, script)
- [x] Criar um editor básico para criar e rodar projetos de exemplo
- [x] Criar projetos de exemplo (vazio, 2D, 3D)

---

## Tecnologias e Nomes dos Sistemas

Todas as teconogias e sistemas da engine devem ter nomes únicos e memoráveis, que reflitam suas funcionalidades e características. Esses nomes devem ser usados consistentemente em toda a documentação, código e comunicação relacionada à engine, para garantir clareza e coesão na identidade da engine, fazem parte da identidade da engine e ajudam a criar uma marca forte e reconhecível no mercado de desenvolvimento de jogos.

- Engine: Lightning Engine
- Editor: Lightning Engine Editor
- Particulas: Hurricane
- Materiais: Equinox
- Shader Composer: Equinox
- Formato de Shader: .spark (arquivos monolíticos @vertex/@fragment com @inject)
- Script Engine: Nucleo Parser + VM
- Script Visual: Random Ignite
- Linguagem de Script: C#
- Landscape: Atlas
- GUI: Titan
- Physics: MotionPhysix - RigidBody2D, SoftBody, Cloth, etc
- Audio: Thunder
- Prefabs: Nodes System

## Hierarquia de Classes

```
Window → GameInstance → Level → Nodes → Components
```

---

## Engine Core

- [x] Window (SDL_Window + SDL_GPUDevice + loop principal com delta time)
- [x] GameInstance (ciclo de vida completo — herda Window, SDL encapsulado)
- [x] Renderer (SDL_GPU queue-based: Clear/Present via command buffer, FillRect/DrawRect)
- [x] Engine Settings (EngineSettings em Types.h)
- [x] ClearColor configurável
- [x] InputManager — ProcessEvent / IsKeyDown / IsKeyPressed / IsKeyReleased / mouse
- [x] Renderer — pipeline 2D funcional (colored + textured, alpha blend, SPIRV shaders)
- [x] Texture2D — LoadTexture (BMP), DrawTexture, DrawTextureRegion, SetDrawColor como tint
- [x] Renderer — integração com pipeline 3D (pipe3D, DrawMesh, SetView/Projection/Light, depth buffer)
- [x] Framebuffer

## Shader

O sistema de shader usa o formato `.spark` — arquivos monolíticos com seções `@vertex` / `@fragment` e diretivas `@inject` para injeção automática de uniforms do engine (Time, Light, Fog, Transform). O formato é independente de API; internamente compila para SPIR-V via glslc.

O **Equinox** (sistema de materiais e Shader Composer) será construído sobre essa base, expondo os shaders em nível mais alto — via editor visual de nós, MaterialInstance, PBR, etc.

Pipeline: `.spark` → `SparkCompiler` (gera GLSL + compila via glslc) → `.spv` → `SparkShader` (runtime)

### Implementado

- [x] `SparkShader` — runtime post-process (src/include/Spark.h + src/core/Spark.cpp)
  - [x] `Load(renderer, spvBase, flags)` — carrega `_vert.spv` + `_frag.spv`, cria pipeline SDL_GPU
  - [x] `Process(renderer, src, dest)` — full-screen quad → Framebuffer destino
  - [x] Setters: `SetTint`, `SetTime` (sin/cos auto), `SetLight`, `SetFog`
- [x] `SparkCompiler` — offline .spark → GLSL → SPIR-V
  - [x] `Compile(sparkPath, spvBase)` — gera GLSL + invoca glslc
  - [x] `GenerateGLSL(sparkPath, outVert, outFrag)` — só geração GLSL
- [x] Formato `.spark` monolítico — `@vertex` + `@fragment` + `@inject`
- [x] Injeção de uniforms
  - [x] `@inject Time`      — `spark_Time` (x=sec, y=dt, z=sin(t), w=cos(t)) — fragment slot 1
  - [x] `@inject Light`     — `spark_LightPos`, `spark_LightColor`             — fragment slot 2
  - [x] `@inject Fog`       — `spark_FogColor`, `spark_FogParams`              — fragment slot 3
  - [x] `@inject Transform` — `spark_Transform` mat4                           — vertex slot 0
- [x] `spark_default.spark` — pass-through tinted blit
- [x] `spark_wave.spark`    — distorção senoidal com `@inject Time`

### Pendente (base)

- [ ] `@inject UVCoords` — UV screen-space / world-space
- [ ] Lights System (Directional, Point, Spot, Rect, Probes)
  - [ ] Shadow Mapping
  - [ ] Deferred Shading
  - [ ] Forward+ Shading
- [ ] Fog variants (Distance, Height, Volumetric)
- [ ] Geometry / Tessellation / Compute Shaders

### Framebuffers

- [x] Render to Texture (`Framebuffer`: Create/Resize/Release, RGBA8)
- [x] `BeginRenderToTexture` / `EndRenderToTexture`
- [x] `BlurTexture` (gaussian blur pipeline separável)
- [x] Post-Processing via `SparkShader::Process`
- [ ] Multiple Render Targets

### Culling

- [x] Backface Culling (SDL_GPU_CULLMODE_BACK em pipe3D)
- [x] Frustum Culling (FrustumCuller.h — Gribb-Hartmann, TestAABB + TestSphere)
- [ ] Occlusion Culling

## Model

- [x] Model Object (Mesh struct com vertices/normals/UVs)
- [x] Loader OBJ (ModelLoader::Load — parser .obj nativo sem Assimp, fan-triangulation, deduplicação por hash)
- [ ] Loader Assimp (requer lib instalada)

## Titan - GUI

- O Titan
- A GUI deve ser construída utilizando a própria engine, sem depender de bibliotecas externas. O sistema de GUI deve ser flexível e personalizável, permitindo a criação de interfaces gráficas complexas e adaptáveis às necessidades do jogo.
- A UI deve tambem ser utilizada para criar o editor da engine, permitindo a criação de uma interface gráfica intuitiva e funcional para o desenvolvimento de jogos com a engine. O sistema de GUI deve ser projetado para ser fácil de usar, permitindo que os desenvolvedores criem interfaces gráficas personalizadas sem precisar de conhecimento profundo sobre as APIs de renderização ou a estrutura interna da engine.
- O sistema de GUI deve incluir uma variedade de widgets básicos, como botões, caixas de texto, sliders, etc, e deve permitir a criação de widgets personalizados para atender às necessidades específicas do jogo. O sistema de GUI deve ser projetado para ser responsivo, permitindo que as interfaces gráficas se adaptem a diferentes resoluções e tamanhos de tela. O sistema de GUI deve ser projetado para ser scriptable, permitindo a criação e manipulação de widgets via scripts, e a visualização em tempo real das mudanças feitas na GUI no editor.

- [x] Sistema de GUI
- [x] Menus, Janelas, Abas, etc
- [x] Widgets
  - [x] Border
  - [x] Backdrop (Dim, Shadow, Outline, Blur via Framebuffer)
  - [x] Button
  - [x] Image
  - [x] Label
  - [x] TextField
  - [x] Text (multi-linha, word wrap)
  - [x] Checkbox
  - [x] ProgressBar
  - [x] Dropdown
  - [x] List (scrollable, single/multi-select)
  - [x] Slider
  - [x] Window (flutuante, arrastável, botão fechar)
  - [x] TabStrip (abas com troca de conteúdo)
  - [x] ContextMenu (popup right-click, separadores)
  - [x] Viewport2D (inline e isolado via Framebuffer)
    - [x] Canvas / Panel
    - [x] ScrollView
    - [x] Layouts (VStack, HStack, Grid, Separator)
- [x] Events (EventBus.h — Subscribe/Emit/Post/FlushQueue, EventToken, common event types: NodeAdded/Removed, SceneLoaded/Saved, Play/Stop)
- [ ] Responsive (layout adaptativo — pós editor)
- [ ] Scriptable
  - [ ] Link com Script Engine (pós Nucleo)
  - [ ] Realtime GUI em editor (pós Nucleo)
- [ ] GUI Editor
  - [ ] Drag and Drop de Widgets
  - [ ] Configuração de Propriedades dos Widgets
  - [ ] Visualização em tempo real

### Framebuffer (desbloqueado)

- [x] Framebuffer (Create/Resize/Release, COLOR_TARGET + SAMPLER)
- [x] Renderer: BeginRenderToTexture / EndRenderToTexture
- [x] Renderer: BlurTexture (gaussian blur pipeline)
- [x] Shader: blur.frag (SPIRV via compile_shaders.bat)
- [x] Sistema de tooltips (Widget.tooltipText + TitanUI auto-hover 600ms)
- [x] Sistema de diálogos modais (Modal — dim backdrop + centered dialog)
- [x] Sistema de popups/notificações (Toast — TitanUI::ShowToast)
- [x] Widgets avançados (editor-ready):
  - [x] TreeView (expand/collapse, tags, onSelect, scroll)
  - [x] NumericUpDown (−/+ buttons, edit mode, scroll wheel, clamp)
  - [x] Splitter (vertical/horizontal, drag handle, minA/minB)
  - [x] Toolbar (toggle buttons, separators, auto-width)
  - [x] MenuBar (dropdown menus, separators, actions)
  - [x] ColorPicker (R/G/B/A sliders, swatch, hex display)
  - [x] Table/GridView
  - [x] FileDialog
  - [ ] DatePicker
  - [x] StatusBar
- [ ] Drag and Drop entre widgets e arquivos do sistema
- [x] Sistema de atalhos de teclado customizáveis (TitanUI::AddShortcut — Ctrl/Shift/Alt combos)
- [x] Suporte a redimensionamento e movimentação de janelas internas (Window: resizable=true, 8-direction)
- [x] Sistema de docking de painéis (DockSpace — binary-tree docking, splitters, tabs, chrome)
- [ ] Suporte a múltiplas janelas (multi-window)
- [ ] Sistema de undo/redo para interações de UI
- [x] Suporte a renderização de gráficos (Chart widget — Line/Bar, auto-scale, grid, legend)
- [x] Integração com clipboard do sistema operacional (TextField: Ctrl+C/V/X/A)
- [x] Suporte a input de texto avançado (TextField: cursor, seleção, Ctrl+C/V/X/A/Delete)
- [x] Sistema de eventos customizados e binding de comandos (EventBus.h + InputBinding.h — Define/Rebind/IsDown/IsPressed/IsReleased, SaveToIni/LoadFromIni)
- [ ] Suporte a widgets customizados via scripts
- [ ] Preview em tempo real de alterações de UI no editor
- [ ] Sistema de Estilos (Customização visual via código e editor estilo CSS-like)
  - Design: `Plan/Design/Titan ui Styles.png` — glassmorphism preset planejado
  - [ ] Glassmorphism (`radius`, `backdropBlur`, `border`, `backgroundColor` com alpha)
  - [ ] Preset de temas (Dark, Light, Glassmorphism, Custom)
  - [ ] Estilo por widget (Container.styles.radius = 46, etc.)

- O Titan Posteriormente sera um software que ira permitir criar interfaces para aplicações c++, mas por hora ira alimentar a Lightning Engine a GUI que o sistema precisa.

---

## API

- [x] Math — LMath namespace: Lerp, Clamp, Distance, Normalize, Dot, Smoothstep, MoveTowards, Reflect (Math.h)
- [x] V2 — operadores aritméticos completos + scalar ops (Types.h)
- [x] Physics — RigidBody2D (velocity, acceleration, drag, gravity, Integrate), Rect2D AABB (Physics.h)
- [ ] Physics — MotionPhysics 3D
- [x] 2D
  - [x] DrawShapes (DrawLine, DrawCircle, FillCircle em Renderer — debug, colisões, estética minimalista)
  - [x] Tilemap (Tilemap.h — atlas-based grid, DrawTile, TilemapComponent com Render automático)
  - [x] Parallax Background (ParallaxLayer.h — multi-layer, scrollFactor, tiling seamless via fmodf, ParallaxBackgroundComponent)
- [ ] 3D
  - [ ] Draw Meshes (renderização de modelos 3D usado para debug, colisões, ou para criar jogos com estética minimalista)
  - [ ] Mesh Renderer (renderização de modelos 3D usando malhas e materiais)
  - [ ] Skeletal Animation (animação de personagens 3D usando esqueletos e pesos de vértice)
  - [ ] PostFX (bloom, motion blur, depth of field, etc)
- [x] Particles — Hurricane (Hurricane.h — pool fixo, emitRate, lifespan, gravity, lerp size+color, FillCircle, ParticleSystemComponent)
- [x] Node / Component system (Node, Component, Transform, AddChild, AddComponent, Clone)
- [x] Prefabs (ProjectManager::SavePrefab/LoadPrefab — formato .lprefab, gramática .lescene, subtree de Node reutilizável)
- [x] Inputs (InputManager completo — IsKeyDown/Pressed/Released, IsMouseDown/Pressed, GetMouseX/Y)

---

## Equinox — Sistema de Materiais e Shader Composer

Equinox e um Shader composer muito poderoso capas de expandir as funcionalidades de renderização da engine permitindo a criação de materiais personalizados e complexos para os objetos do jogo. A Criação de Shaders e expor suas funcionalidades via editor tanto via script quanto via editor visual de shader, permitindo a criação de materiais personalizados simulações complexas, acesso direto a GPU efeitos e conexão aos motores de física e partículas da engine, permitindo a criação de efeitos visuais avançados e interações dinâmicas entre os objetos do jogo. O Equinox deve ser projetado para ser fácil de usar, permitindo que os desenvolvedores criem materiais personalizados sem precisar de conhecimento profundo sobre as APIs de renderização ou a estrutura interna da engine.

O **Equinox** é o sistema de materiais e compositor de shaders da Lightning Engine, projetado para oferecer flexibilidade e poder na criação de materiais visuais para objetos 2D e 3D. Ele permite que desenvolvedores criem, editem e atribuam materiais personalizados a qualquer objeto da engine, integrando-se diretamente ao pipeline de renderização.

### Objetivos do Equinox

- Permitir a criação de materiais customizados para qualquer objeto da engine (sprites, meshes, UI, partículas, etc).
- Oferecer um editor visual de materiais (Material Graph) para composição de shaders via nós (node-based).
- Suportar materiais criados via script (C#) e via editor visual.
- Integrar com o sistema de shader (Spark), permitindo a injeção de funções, variáveis e efeitos diretamente nos materiais.
- Permitir preview em tempo real dos materiais no editor.
- Suportar múltiplos tipos de materiais: Standard, Unlit, PBR, Custom, PostFX, etc.
- Permitir a criação de bibliotecas de materiais reutilizáveis.

### Funcionalidades Principais

- [ ] Material Graph Editor (node-based, drag and drop, preview)
- [ ] Suporte a múltiplos tipos de materiais (Standard, Unlit, PBR, Custom)
- [ ] Integração com Spark (Shader System) para geração automática de shaders a partir do material graph
- [ ] Suporte a parâmetros customizáveis (cores, texturas, floats, bools, etc)
- [x] Sistema de instanciamento de materiais (MaterialInstance — Equinox.h)
- [x] Biblioteca de materiais prontos para uso (MaterialLibrary — Equinox.h)
- [ ] Suporte a texturas múltiplas, mapas de normal, especular, emissivo, etc
- [ ] Exportação/importação de materiais (formato .leasset)
- [ ] Preview em tempo real no editor
- [ ] Integração com o sistema de partículas (Hurricane) e física (MotionPhysix)
- [ ] Suporte a materiais animados (via script ou nodes de animação)
- [ ] Suporte a custom nodes (via script)

### Estrutura do Sistema

```text
EquinoxMaterial
    ├─ MaterialGraph (Editor Visual)
    ├─ MaterialInstance (Parâmetros customizados)
    ├─ MaterialLibrary (Coleção de materiais)
    └─ Integração com Equinox (Shader Generator)
```

### Exemplo de Uso (C#)

```csharp
// Criação de um material customizado via script
var material = new EquinoxMaterial();
material.SetColor("BaseColor", new Color(1.0f, 0.5f, 0.2f));
material.SetTexture("Albedo", "textures/brick.png");
material.SetFloat("Roughness", 0.8f);

// Atribuição do material a um mesh
var meshNode = new Node3D();
meshNode.AddComponent(new MeshRendererComponent(mesh, material));
```

### Exemplo de Material Graph (Editor Visual)

- Nodes disponíveis: Texture, Color, Multiply, Add, Lerp, NormalMap, Emissive, Fresnel, CustomFunction, etc.
- Conexão dos nodes define o fluxo de dados para o shader final.
- Preview em tempo real do resultado do material aplicado a um modelo 3D ou sprite.

### Integração com o Editor

- O Equinox será acessível diretamente no editor da Lightning Engine.
- Permite criar, editar e atribuir materiais a objetos da cena com drag and drop.
- Preview em tempo real das alterações no material.
- Suporte a undo/redo e salvamento automático dos materiais.

### Roadmap do Equinox

- [x] MVP: MaterialInstance (Equinox.h — SetFloat/Color/Texture, Apply → SparkShader setters Tint/Light/Fog/Time, MaterialLibrary com Add/Get/Has)
- [ ] Editor visual de materiais (Material Graph)
- [ ] Suporte a materiais PBR e customizados
- [ ] Biblioteca de materiais prontos (migrar essa tarefa para Viewport2D.md)
- [ ] Integração total com editor e preview em tempo real

---

## Features de Renderização

- [x] Render 2D (colored shapes + textures, alpha blend)
- [x] Render 3D (pipe3D, DrawMesh, SetView/Projection/Light, depth buffer, Lambert diffuse, Camera3DComponent, MeshRendererComponent)
- [ ] PostFX
  - [ ] PostFx Expandido com Equinox (bloom, motion blur, depth of field, etc)

---

## Núcleo (VM / Script)

- Tanto o RandonIgnite `Docs/RandomIgnite` Quanto a Linguagem C# serão processados por um parser que irá converter o código fonte para bytecode, que será interpretado pela Nucleo VM em tempo de execução. O processo de execução do código é otimizado para garantir uma performance adequada, mesmo em jogos mais complexos. O Script Engine deve ser projetado para ser fácil de usar, permitindo que os desenvolvedores criem scripts personalizados sem precisar de conhecimento profundo sobre as APIs de renderização ou a estrutura interna da engine. O Script Engine deve ser projetado para ser flexível e extensível, permitindo a criação de scripts

- No Editor a Execução do Codigo e feita de Forma Interpretada
- No Jogo o Codigo e Compilado para a plataforma alvo e otimizada para garantir a melhor performance possível, permitindo que jogos mais complexos sejam executados sem problemas de desempenho.

- [x] Build VM Windows x64
- [x] Binary: Read / Write / Bytecode
- [x] Run Game by Nucleo VM
- [x] Script Engine
  - [x] Link Game with Code
  - [x] Realtime gameplay in editor

## Exemplos

- Os Projetos de exemplo devem ser criados usando a propria engine, e não devem depender de bibliotecas externas (exceto para assets).

### Projetos

- Ao Iniciar a engine o usuario se depara com uma tela de seleção de projeto, onde ele pode escolher um dos projetos de exemplo ou criar um novo projeto vazio.
- [x] Modelo de Exemplo Vazio (Empty)
- [x] Modelo de Exemplo 2D (2D) — Player node + PlayerController.spark (WASD movement)
- [x] Modelo de Exemplo 3D (3D) — Camera + Cube nodes + FreeCam.spark

- Modelos prontos devem copiar todos os arquivos necessários para o projeto, incluindo os assets, scripts, cenas, etc. O usuário deve ser capaz de abrir o projeto e rodar o exemplo sem precisar configurar nada.

### Criação de Projetos

- [x] Estrutura de pastas criada automaticamente (`scenes/`, `assets/`, `scripts/`, `assets/textures`, `assets/audio`, `assets/models`, `assets/shaders`)
- [x] `project.ini` — nome, versão, última cena
- [x] `editor.ini` — configurações do editor
- [x] `game.ini` — resolução, fullscreen, inputs
- [x] `.gitignore` padrão gerado
- [x] Cena vazia padrão (`scenes/main.lescene`)
- [x] Histórico de projetos recentes (`%APPDATA%/LightningEngine/recent.ini`)
- [ ] Normalização de assets para formato `.leasset`

## Execução da engine

- A Engine deve ser capaz de rodar o projeto em tempo real utilizando uma maquina virtual (Nucleo VM) que interpreta o bytecode gerado pelos scripts do projeto. O processo de execução o codigo fonte utiliza o jogo utilzia scripts escritos em C# e (Nucleo Posteriormente), que são compilados para bytecode e interpretados pela Nucleo VM. e rodam o projeto em tempo de execução.
- A Nucleo VM deve utilizar escrita de tracer para mapear todos os caminhos de execução do código, permitindo a depuração e análise de performance do jogo em tempo real.
- A Engine é escrita em C++ e tambem deve ser capaz de rodar o codigo de suas extensoes em C++ (plugins), permitindo a criação de funcionalidades personalizadas e a integração com bibliotecas externas.
- O Core da engine e compilado mas os plugins devem utilizar o sistema de Reflection da engine para se registrar e serem reconhecidos pela Nucleo VM, permitindo a execução de código C++ em tempo real.
- Uma vez Compilado pela Nucleo VM, o codigo roda diretamente o bytecode, sem necessidade de recompilação, permitindo uma iteração rápida durante o desenvolvimento do jogo. O processo de execução do código é otimizado para garantir uma performance adequada, mesmo em jogos mais complexos.

## Editor

- O Editor deve ser capaz de abrir e editar os projetos criados, permitindo a criação de cenas, adição de objetos, configuração de propriedades, etc.
- O Editor deve ser capaz de rodar o projeto em tempo real, permitindo a visualização das mudanças feitas no editor sem precisar sair do ambiente de desenvolvimento.
- O Editor deve ser capaz de salvar as cenas e configurações do projeto, permitindo a continuidade do desenvolvimento em sessões futuras.
- O Editor deve ser intuitivo e fácil de usar, com uma interface gráfica amigável e acessível para desenvolvedores de todos os níveis de experiência.

### Status de Implementação

- [x] Splash / Welcome Screen (splash.png background + painel lateral de projetos)
- [x] Histórico de projetos recentes (lista clicável com nome + path)
- [x] Botão Browse de pasta — native OS folder picker (IFileOpenDialog/Win32, zenity/Linux)
- [x] ProjectManager (Create / Open / Save / Close + project.ini / game.ini / editor.ini)
- [x] SaveScene / LoadScene (formato .lescene, stack-based parser)
- [x] Hierarquia de Nodes real (TreeView com expand/collapse, userData = Node*)
- [x] Inspector de Node real (name, tag, active, Position/Rotation/Scale via NumericUpDown)
- [x] Viewport 2D com render da cena via Framebuffer
- [x] Content Browser lendo assets do disco (fs::recursive_directory_iterator)
- [x] Play / Pause — editorLevel.Update(dt), scripts Nucleo rodam em tempo real
- [x] Menu FILE completo (New/Open/Save Project, New/Save Scene, Close, Exit)
- [x] Tab System (Project, Random, Textura, Material, Hurricane)
- [x] DockSpace com painéis redimensionáveis (Hierarchy, Inspector, Viewport, Content Browser)
- [x] NucleoScriptComponent visível no Inspector + botão Add Script
- [x] Undo / Redo — UndoStack.h (command pattern, Ctrl+Z/Y, transform/add/delete node)
- [x] Drag & Drop de assets para a cena — arrastar .spark/.lescene/.png do Content Browser para o Viewport
- [x] Gizmos de transformação no Viewport (move X/Y) — setas vermelha/verde, drag com mouse
- [x] Play em janela separada (GamePreviewWindow.h — segunda SDL_Window, GPU device compartilhado via SDL_ClaimWindowForGPUDevice, Renderer+InputManager independentes, Tick por frame)
- [x] Console de output (logs de script, erros Nucleo) — Logger.h + painel console no editor

## Hierarquia

- Window → GameInstance → Level → Nodes → Components

### Window

- A classe Window é a base da hierarquia de classes da engine, e é responsável por criar a janela do jogo, gerenciar o loop principal e processar os eventos de entrada. A classe Window deve ser projetada para ser flexível e extensível, permitindo a criação de diferentes tipos de janelas (como janelas de editor, janelas de jogo, etc) e a integração com diferentes sistemas de renderização. O Usuario nunca lida diretamente com a classe Window, mas sim com a classe GameInstance, que herda de Window e expõe apenas os sistemas de renderização e entrada como membros protegidos.

### GameInstance

- A classe GameInstance é o ponto de entrada do usuário para criar e rodar jogos com a engine. A classe GameInstance deve ser projetada para ser fácil de usar, permitindo que os desenvolvedores criem jogos rapidamente sem precisar se preocupar com os detalhes de baixo nível da engine. A classe GameInstance deve expor os sistemas de renderização e entrada como membros protegidos, permitindo que os desenvolvedores acessem esses sistemas de forma fácil e intuitiva.

### Level

- A classe Level é responsável por gerenciar a cena ou fase do jogo, incluindo a hierarquia de objetos, a lógica de atualização e renderização, etc. A classe Level deve ser projetada para ser flexível e extensível, permitindo a criação de diferentes tipos de cenas (como cenas de menu, cenas de jogo, etc) e a integração com diferentes sistemas de jogo (como sistemas de física, sistemas de partículas, etc). A classe Level deve ser gerenciada pelo GameInstance, que é responsável por criar e destruir as instâncias de Level conforme necessário.

- Level deve gerenciar os estados do jogo e precisa de Componentes de Logica e Renderização para gerenciar esses estados
  - GameState (Estado atual do jogo, como Menu, Jogando, Pausado, etc)
  - GameController (Gerencia a lógica do jogo, como regras, pontuação, etc)
  - GameHUD (Interface do usuário, como barras de vida, pontuação, etc)
  - InputHandler (Gerencia a entrada do usuário, como teclado, mouse, etc)
  - Spawner (Gerencia o spawn de inimigos, itens, etc)
  - Viewer (Gerencia a visualização da cena, como câmeras, efeitos, etc)
  - BackgroundRenderer (Gerencia a renderização do plano de fundo da cena, como céu, etc)
  - RendererMode (Gerencia o modo de renderização da cena, como 2D, 3D, Apagado, Iluminação, etc)
  - PostFx (Gerencia os efeitos de pós-processamento da cena, como bloom, motion blur, etc)
  - Root Node (Gerencia a hierarquia de objetos da cena, como personagens, inimigos, itens, etc)
  - Landscape (Gerencia a renderização de paisagens, como terrenos, etc)

### Nodes / Components

- Os Nodes e Components são os elementos básicos da hierarquia de objetos do jogo, e são responsáveis por representar os objetos do jogo e suas funcionalidades. Os Nodes são os objetos principais da hierarquia, e podem conter outros Nodes e Components como filhos. Os Components são os elementos que adicionam funcionalidades aos Nodes, como renderização, física, etc. A hierarquia de Nodes e Components deve ser projetada para ser flexível e extensível, permitindo a criação de diferentes tipos de objetos (como personagens, inimigos, itens, etc) e a integração com diferentes sistemas de jogo (como sistemas de física, sistemas de partículas, etc).

- O Node pode ser um prefab, que é um modelo de objeto que pode ser instanciado várias vezes na cena, permitindo a criação de objetos complexos e reutilizáveis. Os Components podem ser usados para adicionar funcionalidades específicas aos Nodes, como renderização, física, etc, permitindo a criação de objetos com comportamentos personalizados.

- Os Componentes nao podem ser usados diretamente, mas sim através de Nodes, que são os objetos principais da hierarquia. Os Nodes são responsáveis por gerenciar os Components e garantir que eles sejam atualizados e renderizados corretamente durante o ciclo de vida do jogo. os componentes são projetados para ser modulares e reutilizáveis, permitindo a criação de diferentes tipos de objetos com diferentes combinações de funcionalidades.

- Os Nodes gerenciam os componentes e garantem tanto sua posição no mundo quanto sua renderização, alem de vincular scripts e manter o codigo organizado.

`[Cena] -> [Node] -> [Componentes de Cena]` (Renderização, Luz, Camera, etc)
`[Logica] -> [Componentes de Logica]` (Scripts, IA, etc)

Node -> Script Binded -> Acessa Componentes de Logica e Cena -> Modifica o comportamento do Node e seus Componentes de Cena

Estrutura base do Node:

Node Class
Transform Component (position, rotation, scale) (Relative e Absolute)

Node2D Class (herda de Node)
SpriteRenderer Component (texture, color, etc)

Node3D Class (herda de Node)
MeshRenderer Component (mesh, material, etc)

- Nodes ja integrados ao sistema que podem ser adicionados aos projetos:
  - Sprite: Node com Sprite Renderer integrado, para renderizar sprites 2D. e Transform para manipular a posição, rotação e escala do sprite na cena.
  - Camera: Node com Camera Component integrado, para controlar a visualização da cena. e Transform para manipular a posição, rotação e escala da câmera na cena.
    - Light: Node com Light Component integrado, para criar fontes de luz na cena. e Transform para manipular a posição, rotação e escala da luz na cena.

### Componentes

Os componentes sao elementos base que injetam dados diretamente no processo de renderização, e podem ser usados para criar uma variedade de funcionalidades e efeitos visuais. Alguns exemplos de componentes que podem ser implementados na engine incluem:
Duas categorias de componentes: Cena e Logica. Componentes de cena injetam dados diretamente no processo de renderização, enquanto componentes de lógica são usados para criar comportamentos personalizados e interações no jogo.

A Utilização de componentes diretamente na cena so podem ser feitas atraves de Nodes, que são os objetos principais da hierarquia. Os Nodes são responsáveis por gerenciar os Components e garantir que eles sejam atualizados e renderizados corretamente durante o ciclo de vida do jogo. os componentes são projetados para ser modulares e reutilizáveis, permitindo a criação de diferentes tipos de objetos com diferentes combinações de funcionalidades. para modificar os dados expostos pelos componentes, o usuario deve criar scripts que acessem esses dados e os modifiquem conforme necessário, permitindo a criação de comportamentos personalizados e interações no jogo por meio de scripts.

```csharp
// Exemplo de uso de componentes em C# na Lightning Engine

// Definição de um Node com componentes
public class PlayerNode : Node
{
    public PlayerNode()
    {
        // Adiciona um componente de renderização
        AddComponent(new SpriteRendererComponent("player.png"));

        // Adiciona um componente de física
        AddComponent(new RigidBody2DComponent());

        // Adiciona um componente de luz
        AddComponent(new LightComponent
        {
            Type = LightType.Point,
            Intensity = 1.0f,
            Color = new Color(1.0f, 0.9f, 0.7f)
        });
    }

    public override void Update(float deltaTime)
    {
        // Exemplo de acesso e modificação de componente
        var light = GetComponent<LightComponent>();
        if (light != null)
        {
            light.Intensity = 2.0f; // Modifica a intensidade da luz em tempo real
        }
    }
}

// Exemplo de criação de cena
public class GameLevel : Level
{
    public override void OnLoad()
    {
        var player = new PlayerNode();
        AddNode(player);

        // Adiciona uma câmera à cena
        var camera = new Node();
        camera.AddComponent(new CameraComponent
        {
            Projection = CameraProjection.Perspective,
            Fov = 60.0f
        });
        AddNode(camera);
    }
}
```

- Luz: (Direct, Point, Spot ou Square) podem ser implementados como componentes. ou como nodes que gerenciam os estados de visualização no mundo 2D ou 3D, e injetam os dados de luz nos shaders. A implementação de luzes como componentes permite a criação de objetos de luz reutilizáveis e flexíveis, que podem ser facilmente adicionados e configurados em diferentes cenas do jogo. Os componentes de luz devem ser projetados para ser compatíveis com o sistema de shader da engine, permitindo a injeção de dados de luz nos shaders para criar efeitos de iluminação realistas e dinâmicos.
- Camera: (Camera, Cinematic) os compoenentes de câmera podem ser usados para controlar a visualização da cena, permitindo a criação de diferentes tipos de câmeras (como câmeras de jogo, câmeras de menu, etc) e a integração com diferentes sistemas de jogo (como sistemas de física, sistemas de partículas, etc). Os componentes de câmera devem ser projetados para ser compatíveis com o sistema de shader da engine, permitindo a injeção de dados de câmera nos shaders para criar efeitos visuais realistas e dinâmicos.
- SpriteRenderer: os componentes de renderização de sprite podem ser usados para renderizar sprites

