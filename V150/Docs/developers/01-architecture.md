# 1. Arquitetura em 3 camadas

A Lightning Engine adota uma arquitetura estrita em três camadas. A separação
é o que torna possível expandir a engine sem comprometer estabilidade.

```
┌─────────────────────────────────────────────────────────────┐
│  PLUGINS (externos / oficiais)                              │
│  • AI, pathfinding, integrações, ferramentas editoriais     │
│  • Componentes ECS adicionais                               │
│  • Carregados dinamicamente (.dll / .so / WASM)             │
└─────────────────────────────────────────────────────────────┘
                              ▲
                              │  (ABI C estável)
┌─────────────────────────────┴──────────────────────────────┐
│  RUNTIME (módulos internos plugáveis)                      │
│  • Render (SDL3 → Vulkan/OpenGL)   • Física                │
│  • Áudio                          • UI (Titan)             │
│  • Input                          • Scripting (Nucleo)     │
└─────────────────────────────────────────────────────────────┘
                              ▲
                              │
┌─────────────────────────────┴──────────────────────────────┐
│  CORE (imutável, mínimo, estável)                          │
│  Lifecycle • Scheduler/Loop • EventBus • PluginManager     │
│  TypeRegistry (reflexão) • Memória • EngineApiVersion      │
└─────────────────────────────────────────────────────────────┘
```

**Regra de ouro:** Core não depende de Runtime. Runtime não depende de
plugins externos. Setas de dependência só apontam para baixo.

## 1.1 Layout-alvo do repositório

```
engine/
  core/
    engine.hpp / engine.cpp
    lifecycle.hpp
    plugin_manager.hpp
    type_registry.hpp
    event_bus.hpp
    memory.hpp
    engine_api_version.hpp

  runtime/
    render/   (Renderer, Framebuffer, GPUResource, Shaders)
    audio/
    input/    (InputManager, InputBinding)
    physics/
    ui/       (Titan)
    scripting/(Nucleo)

  plugins/
    official/
    third_party/

  bindings/
    rust_bridge/   (consumido pelo Tauri)
    js_bridge/     (gerado a partir do TypeRegistry)
```

## 1.2 Mapeamento do código atual → alvo

| Hoje                                              | Camada-alvo            |
| ------------------------------------------------- | ---------------------- |
| `src/include/Engine.h`, `src/core/Engine.cpp`     | `engine/core`          |
| `src/include/EventBus.h`                          | `engine/core`          |
| `src/include/PluginContracts.h`, `PluginManager.h`| `engine/core`          |
| `src/include/Renderer.h`, `src/graphics/*`        | `engine/runtime/render`|
| `src/include/InputManager.h`, `src/input/*`       | `engine/runtime/input` |
| `src/include/Physics.h`                           | `engine/runtime/physics`|
| `src/include/gui/*` (Titan)                       | `engine/runtime/ui`    |
| `src/include/nucleo/*`, `src/core/nucleo/*`       | `engine/runtime/scripting`|
| `src/include/Hurricane.h` (partículas)            | `engine/runtime/render` (subsistema) |
| `src/include/Equinox.h` (materiais)               | `engine/runtime/render` (subsistema) |
| `src/include/EditorBridge.h`, `src/editor/*`      | `engine/bindings/rust_bridge` (lado C++) |
| `src/include/components/*`                        | Plugins oficiais (`engine/plugins/official/components`) |

> A migração física é **opcional e faseada** — ver
> [`06-restructure-roadmap.md`](06-restructure-roadmap.md). A regra de
> dependências, porém, vale **a partir de agora**: PRs que fizerem
> Core depender de Runtime ou Plugins devem ser rejeitadas.

## 1.3 Anti-padrões

| Anti-padrão                                        | Correção                                                  |
| -------------------------------------------------- | --------------------------------------------------------- |
| Core `#include` de header de Runtime               | Mover símbolo para Core ou inverter via interface         |
| Plugin chamando Renderer direto                    | Solicitar render via API exposta pelo Runtime/EventBus    |
| Lógica em macros de reflexão                       | Macros só registram; lógica fica em código normal         |
| Reflexão dentro de hot-loop                        | Cachear `TypeInfo*` uma vez no setup                      |
| Dependência circular entre módulos                 | Substituir chamada direta por evento no `EventBus`        |
