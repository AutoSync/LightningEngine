# Lightning Engine — Melhorias de Engenharia e Design

Documento gerado a partir de auditoria arquitetural da v0.15.0 (2026-04-18).
Objetivo: corrigir problemas estruturais, reduzir riscos de estabilidade e
preparar a base de código para expansão nas próximas versões.

---

## Sumário

| #  | Área                              | Prioridade | Status |
|----|-----------------------------------|------------|--------|
| 1  | Decomposição do EditorApp         | P0         | [ ]    |
| 2  | Consolidação de pipelines         | P0         | [x]    |
| 3  | RAII para recursos GPU            | P0         | [x]    |
| 4  | Unificação children/ownedChildren | P1         | [x]    |
| 5  | Cache de WorldMatrix              | P1         | [x]    |
| 6  | Rollback em LoadLevel             | P1         | [x]    |
| 7  | Índices de busca em Level         | P1         | [x]    |
| 8  | Fixar double-free em Spark        | P1         | [x]    |
| 9  | Validação de massa em Physics     | P2         | [x]    |
| 10 | Hurricane: rand() → \<random\>    | P2         | [x]    |
| 11 | Renderer: state stack             | P2         | [x]    |
| 12 | EventBus: documentar thread model | P2         | [x]    |
| 13 | Texture: ownership explícito      | P2         | [x]    |

---

## P0 — Crítico

### 1. Decomposição do EditorApp

**Problema**: `EditorApp` (~250+ membros privados) concentra toda a lógica de
viewport, inspector, content browser, console, gizmos, drag-and-drop e script
panel em uma única classe. Anti-pattern God Object que dificulta manutenção,
testes e expansão.

**Arquivos afetados**:
- `src/editor/EditorApp.h`
- `src/editor/EditorApp.cpp`

**Solução**: Extrair cada painel em classe independente que recebe contexto do
EditorApp, mas gerencia seu próprio estado e renderização.

```
EditorApp : GameInstance
├── EditorViewport     — câmera, gizmos, pick, grid
├── EditorInspector    — properties do nó selecionado, componentes
├── ContentBrowser     — navegação de filesystem, thumbnails, drag
├── EditorHierarchy    — TreeView de nós da cena, seleção
├── EditorConsole      — log display (lê Logger)
├── EditorScriptPanel  — code editor inline
└── EditorToolbar      — play/pause, save, atalhos
```

**Tarefas**:
- [ ] Definir interface base `EditorPanel` com `Update(dt)`, `Render()`, `OnResize()`
- [ ] Extrair `EditorViewport` (câmera, gizmos, processGizmoDrag, pickViewportNode)
- [ ] Extrair `EditorInspector` (renderização de properties, componentes, sliders)
- [ ] Extrair `ContentBrowser` (cbCurrentDir, cbEntries, thumbnails, rename, delete)
- [ ] Extrair `EditorHierarchy` (TreeView, seleção, drag-and-drop de nós)
- [ ] Extrair `EditorConsole` (leitura de Logger, filtros, scroll)
- [ ] Extrair `EditorScriptPanel` (syntax highlight, edição, salvamento)
- [ ] Reduzir EditorApp para orquestrador: compõe painéis, gerencia layout, roteia eventos

**Critério de aceite**: EditorApp tem menos de 50 membros diretos; cada painel é
compilável isoladamente.

---

### 2. Consolidação de Pipelines do Renderer

**Status**: Concluído em 2026-04-18.

**Problema**: `Renderer.h` duplica 4 pipelines e 4 vertex buffers porque
swapchain e render-target usam formatos de textura diferentes. Toda
inicialização e flush é duplicada.

**Arquivos afetados**:
- `src/include/Renderer.h`
- `src/core/Renderer.cpp`

**Estado atual**:
```cpp
// Swapchain (formato runtime)
SDL_GPUGraphicsPipeline* pipe2D, pipeTex;
SDL_GPUBuffer* vbuf2D, vbufTex;

// Render-target (R8G8B8A8_UNORM)
SDL_GPUGraphicsPipeline* pipe2D_rt, pipeTex_rt;
SDL_GPUBuffer* vbuf2D_rt, vbufTex_rt;
```

**Solução**: Agrupar em struct parametrizado pelo formato:

```cpp
struct PipelineSet {
    SDL_GPUGraphicsPipeline* pipe2D  = nullptr;
    SDL_GPUGraphicsPipeline* pipeTex = nullptr;
    SDL_GPUBuffer*           vb2D   = nullptr;
    SDL_GPUBuffer*           vbTex  = nullptr;

    bool Init(SDL_GPUDevice* dev, SDL_GPUTextureFormat fmt, ...);
    void Release(SDL_GPUDevice* dev);
};

PipelineSet swapPipes_;  // formato dinâmico
PipelineSet rtPipes_;    // R8G8B8A8_UNORM
```

**Tarefas**:
- [x] Criar struct `PipelineSet` com `Release()` e verificação de prontidão
- [x] Consolidar inicialização 2D em `initPipelineSet(...)` para swapchain e render-target
- [x] Ajustar `flushQueueTo()` para receber `PipelineSet` via `FlushParams`
- [x] Simplificar `FlushParams` para usar `const PipelineSet*`
- [x] Validar build Debug x64

**Critério de aceite**: Nenhum pipeline/buffer duplicado fora do `PipelineSet`;
`Renderer.h` tem ~30% menos membros privados.

---

### 3. RAII para Recursos GPU

**Status**: Concluído em 2026-04-18.

**Problema**: Pipelines, buffers e texturas são ponteiros brutos SDL sem
destrutor automático. Se a inicialização falha no meio, recursos já alocados
não são limpos. `Release()` é chamado manualmente.

**Arquivos afetados**:
- `src/include/Renderer.h`
- `src/include/Spark.h`
- `src/include/Framebuffer.h`
- `src/include/Texture.h`

**Solução**: Criar wrappers RAII mínimos em um header utilitário:

```cpp
// src/include/GPUResource.h
namespace LightningEngine {

template<typename T, void(*Deleter)(SDL_GPUDevice*, T*)>
class GPUHandle {
    SDL_GPUDevice* dev_ = nullptr;
    T*             ptr_ = nullptr;
public:
    GPUHandle() = default;
    GPUHandle(SDL_GPUDevice* d, T* p) : dev_(d), ptr_(p) {}
    ~GPUHandle() { if (ptr_ && dev_) Deleter(dev_, ptr_); }

    GPUHandle(GPUHandle&& o) noexcept : dev_(o.dev_), ptr_(o.ptr_) {
        o.ptr_ = nullptr;
    }
    GPUHandle& operator=(GPUHandle&& o) noexcept {
        if (this != &o) { reset(); dev_ = o.dev_; ptr_ = o.ptr_; o.ptr_ = nullptr; }
        return *this;
    }

    T* get() const { return ptr_; }
    explicit operator bool() const { return ptr_ != nullptr; }
    void reset() { if (ptr_ && dev_) Deleter(dev_, ptr_); ptr_ = nullptr; }
};

using UniquePipeline = GPUHandle<SDL_GPUGraphicsPipeline, SDL_ReleaseGPUGraphicsPipeline>;
using UniqueBuffer   = GPUHandle<SDL_GPUBuffer, SDL_ReleaseGPUBuffer>;

} // namespace LightningEngine
```

**Tarefas**:
- [x] Criar `src/include/GPUResource.h` com `GPUHandle<T, Deleter>`
- [x] Migrar `Renderer` para usar `UniquePipeline`, `UniqueBuffer` e `UniqueTexture`
- [x] Migrar `SparkShader` para usar `UniquePipeline` e `UniqueBuffer`
- [x] Migrar `Framebuffer`, `Texture` e `Mesh` para wrappers RAII
- [x] Garantir que `Release()` manual continue funcional via `reset()`
- [x] Validar build Debug x64

**Critério de aceite**: Nenhum `SDL_Release*` manual fora dos destrutores RAII;
leak check via Renderer destructor sem crashs.

---

## P1 — Importantes

### 4. Unificação children/ownedChildren em Node

**Status**: Concluído em 2026-04-18.

**Problema**: `Node` mantém dois vetores sincronizados manualmente:
`children` (`vector<Node*>`) para acesso rápido e `ownedChildren`
(`vector<unique_ptr<Node>>`) para ownership. Toda operação (Add/Remove)
precisa atualizar ambos — propenso a bugs.

**Arquivo**: `src/include/Node.h`

**Estado atual**:
```cpp
Node*                                parent = nullptr;
std::vector<Node*>                   children;
std::vector<std::unique_ptr<Node>>   ownedChildren;
```

**Solução**: Manter apenas `ownedChildren_` e expor acesso via helper:

```cpp
std::vector<std::unique_ptr<Node>> children_;

// API pública
size_t ChildCount() const { return children_.size(); }
Node*  GetChild(size_t i) const { return children_[i].get(); }

// Iteração (para Update/Render)
// Range-based: for (auto& c : node.Children()) c->Update(dt);
```

**Tarefas**:
- [x] Substituir `children` + `ownedChildren` por `children_` único
- [x] Ajustar `AddChild()`, `RemoveChild()` para operar em `children_` apenas
- [x] Ajustar `Update()`, `Render()`, `SetContext()` para iterar `children_`
- [x] Expor `GetChildren()` retornando span ou referência ao vetor de unique_ptr
- [x] Verificar impacto em `EditorApp` e `Level` (que usam `GetChildren()`)
- [x] Validar build Debug x64

---

### 5. Cache de WorldMatrix com Dirty Flag

**Status**: Concluído em 2026-04-18.

**Problema**: `WorldMatrix()` recalcula recursivamente toda a cadeia de pais a
cada chamada. Em hierarquias profundas com N nós, isso é O(N × profundidade)
por frame — potencialmente O(N²).

**Arquivo**: `src/include/Node.h`

**Solução**: Cache com dirty flag propagado na hierarquia:

```cpp
mutable glm::mat4 cachedWorldMatrix_ = glm::mat4(1.f);
mutable bool      worldMatrixDirty_  = true;

void MarkTransformDirty() {
    if (!worldMatrixDirty_) {
        worldMatrixDirty_ = true;
        for (auto& c : children_) c->MarkTransformDirty();
    }
}

const glm::mat4& WorldMatrix() const {
    if (worldMatrixDirty_) {
        cachedWorldMatrix_ = parent
            ? parent->WorldMatrix() * LocalMatrix()
            : LocalMatrix();
        worldMatrixDirty_ = false;
    }
    return cachedWorldMatrix_;
}
```

**Tarefas**:
- [x] Adicionar `cachedWorldMatrix_` e `worldMatrixDirty_` (mutable)
- [x] Implementar `MarkTransformDirty()` com propagação descendente
- [x] Chamar `MarkTransformDirty()` em setters de Transform (position/rotation/scale)
- [x] Alterar `WorldMatrix()` para usar cache
- [x] Validar com exemplos rpg2D e 3D

---

### 6. Rollback em LoadLevel

**Status**: Concluído em 2026-04-18.

**Problema**: `GameInstance::LoadLevel<T>()` destrói o level atual antes de
inicializar o novo. Se `Initialize()` falhar, a engine fica sem level ativo.

**Arquivo**: `src/include/GameInstance.h`

**Estado atual**:
```cpp
if (activeLevel) {
    activeLevel->Shutdown();
    activeLevel.reset();
}
auto lvl = std::make_unique<T>();
lvl->Initialize();  // se falhar, sem level
activeLevel = std::move(lvl);
```

**Solução**: Inicializar novo level primeiro, destruir antigo só após sucesso:

```cpp
template<typename T>
T* LoadLevel() {
    auto lvl = std::make_unique<T>();
    T* ptr   = lvl.get();
    lvl->SetContext(renderer, inputManager);
    lvl->Initialize();

    if (activeLevel) activeLevel->Shutdown();
    activeLevel = std::move(lvl);
    return ptr;
}
```

**Tarefas**:
- [x] Reordenar operações em `LoadLevel()` (init novo → shutdown antigo)
- [x] Documentar que dois levels coexistem brevemente durante transição
- [x] Validar build Debug x64

---

### 7. Índices de Busca em Level

**Status**: Concluído em 2026-04-18.

**Problema**: `FindNode(name)` e `FindNodeByTag(tag)` fazem busca linear O(n)
na lista de nós. Em cenas grandes isso é custoso, especialmente se chamado
a cada frame.

**Arquivo**: `src/include/Level.h`

**Solução**: Manter índices de hash atualizados ao adicionar/remover nós:

```cpp
std::unordered_map<std::string, Node*>           nameIndex_;
std::unordered_multimap<std::string, Node*>       tagIndex_;
```

**Tarefas**:
- [x] Adicionar `nameIndex_` e `tagIndex_` como membros privados de Level
- [x] Atualizar índices em `AddNode()` e `RemoveNode()`
- [x] Refatorar `FindNode()` para usar `nameIndex_` — O(1)
- [x] Refatorar `FindNodeByTag()` para usar `tagIndex_` — O(1)
- [x] Adicionar `FindNodesByTag()` retornando `vector<Node*>` (todos com a tag)
- [x] Validar build Debug x64

---

### 8. Fixar Double-Free em SparkShader

**Status**: Concluído em 2026-04-18.

**Problema**: `SparkShader::Release()` libera `pipeline` e `vbuf` via SDL mas
não seta os ponteiros para `nullptr` após liberação. Se `Release()` for
chamado duas vezes (ex: destrutor + chamada manual), ocorre double-free.

**Arquivo**: `src/include/Spark.h`

**Solução**:
```cpp
void Release() {
    if (pipeline) { SDL_ReleaseGPUGraphicsPipeline(device, pipeline); pipeline = nullptr; }
    if (vbuf)     { SDL_ReleaseGPUBuffer(device, vbuf); vbuf = nullptr; }
    device = nullptr;
}
```

**Tarefas**:
- [x] Adicionar `= nullptr` após cada `SDL_Release*` em `SparkShader::Release()`
- [x] Repetir o padrão em `Framebuffer::Release()` e `Texture::Release()`
- [x] Validar build Debug x64

---

## P2 — Melhorias Recomendadas

### 9. Validação de Massa em Physics

**Status**: Concluído em 2026-04-18.

**Problema**: `RigidBody2D::Integrate()` não verifica `mass <= 0`. Com massa
zero ou negativa, `ApplyForce()` gera aceleração infinita ou NaN.

**Arquivo**: `src/include/Physics.h`

**Tarefas**:
- [x] Adicionar `if (mass <= 0.f) return;` no início de `Integrate()`
- [ ] Clamp `mass` em setter ou construtor (mínimo `0.001f`)
- [x] Validar build Debug x64

---

### 10. Hurricane: Substituir rand() por \<random\>

**Status**: Concluído em 2026-04-18.

**Problema**: O sistema de partículas usa `rand()` global que não é thread-safe
e produz distribuição de baixa qualidade.

**Arquivo**: `src/include/Hurricane.h`

**Tarefas**:
- [x] Adicionar membro `std::mt19937 rng_` inicializado com `std::random_device`
- [x] Adicionar `std::uniform_real_distribution<float> dist01_{0.f, 1.f}`
- [x] Substituir todas as chamadas `rand()` por `dist01_(rng_)`
- [x] Remover `#include <cstdlib>` se não mais necessário
- [x] Validar build Debug x64

---

### 11. Renderer: State Stack (Draw State Push/Pop)

**Status**: Concluído em 2026-04-18.

**Problema**: `dr/dg/db/da` e `screenSpace` são estados globais mutáveis.
Qualquer componente pode alterá-los e afetar componentes subsequentes sem
querer. Não há como restaurar estado anterior.

**Arquivo**: `src/include/Renderer.h`

**Solução**:
```cpp
struct DrawState {
    float r = 1.f, g = 1.f, b = 1.f, a = 1.f;
    bool screenSpace = false;
};
std::vector<DrawState> stateStack_;

void PushDrawState();  // salva estado atual
void PopDrawState();   // restaura estado anterior
```

**Tarefas**:
- [x] Definir struct `DrawState`
- [x] Implementar `PushDrawState()` e `PopDrawState()`
- [ ] Usar internamente nos pontos críticos (BeginScreenSpace, etc.)
- [x] Validar build Debug x64

---

### 12. EventBus: Documentar Thread Model

**Status**: Concluído em 2026-04-18.

**Problema**: `EventBus<T>` não é thread-safe. Se a engine for expandida para
multi-thread, data races em `Emit/Post/Subscribe` causarão UB.

**Arquivo**: `src/include/EventBus.h`

**Solução para agora**: Documentar invariante single-thread no header.
**Solução futura** (quando multi-thread): Adicionar `std::mutex`.

**Tarefas**:
- [x] Adicionar comentário no header: `// THREAD MODEL: single-thread only.`
- [x] (Futuro) Proteger `subscribers` e `queue` com `std::mutex` quando multi-thread

---

### 13. Texture: Ownership Explícito

**Status**: Concluído em 2026-04-18.

**Problema**: `Texture::ViewOf()` usa `device = nullptr` para indicar
non-owning. Isso é frágil — se `device` for atribuído depois, a view
destruiria textura alheia.

**Arquivo**: `src/include/Texture.h`

**Solução**:
```cpp
enum class OwnershipMode { Owning, NonOwning };
OwnershipMode ownership_ = OwnershipMode::Owning;

void Release() {
    if (ownership_ == OwnershipMode::NonOwning) return;
    // ... liberar GPU
}
```

**Tarefas**:
- [x] Adicionar `OwnershipMode` enum e membro `ownership_`
- [x] Usar `NonOwning` em `ViewOf()` ao invés de `device = nullptr`
- [x] Checar `ownership_` em `Release()` antes de destruir
- [x] Validar build Debug x64

---

## Dependências entre Tarefas

```
[3] RAII GPU ──────────────┐
                           ├──▶ [2] Consolidar Pipelines
[8] Fix double-free Spark ─┘

[4] Unificar children ──▶ [5] Cache WorldMatrix

[1] Decompor EditorApp (independente, pode iniciar a qualquer momento)

[6] Rollback LoadLevel (independente)
[7] Índices Level (independente)
[9-13] P2 (independentes entre si)
```

**Ordem de execução sugerida**:
1. `[8]` Fix double-free (rápido, previne crashes)
2. `[3]` RAII GPU resources (fundação para refatorações seguintes)
3. `[2]` Consolidar pipelines (depende de [3])
4. `[6]` Rollback LoadLevel (rápido, melhora robustez)
5. `[4]` Unificar children
6. `[5]` Cache WorldMatrix (depende de [4])
7. `[7]` Índices Level
8. `[1]` Decompor EditorApp (maior esforço, pode ser incremental)
9. `[9-13]` P2 conforme oportunidade

---

## Validação

Todas as tarefas devem ser validadas com:
- Build **Debug x64 (MSBuild)** sem erros
- Execução do editor sem crashs (abrir projeto, navegar cena, play/stop)
- Exemplos `rpg2D` e `3D` funcionando sem regressões
