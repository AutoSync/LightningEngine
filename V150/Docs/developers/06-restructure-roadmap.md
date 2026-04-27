# 6. Roadmap de reestruturação

A migração do layout atual (`src/include`, `src/core`, `src/graphics`, …)
para o layout-alvo (`engine/core`, `engine/runtime`, `engine/plugins`,
`engine/bindings`) é **faseada e não-destrutiva**. O objetivo é nunca
quebrar o build da .sln nem o Makefile.

> Status atual: **Fase 1** concluída (interfaces de Runtime criadas).

## Fase 0 — Regras e scaffolding (concluída)

- [x] Documentar arquitetura-alvo (`docs/developers/01-architecture.md`).
- [x] Adicionar `EngineApiVersion.h` (contrato ABI).
- [x] Adicionar `TypeRegistry.h` (skeleton da reflexão).
- [x] Estabelecer regra de dependência (Core ⇎ Runtime ⇎ Plugins).
- [x] Adicionar exemplo `examples/plugins/HelloPlugin`.

Nenhum arquivo existente foi movido. Builds permanecem verdes.

## Fase 1 — Cobertura por interfaces (concluída)

Interfaces puras criadas em `src/include/runtime/`:

- [x] `IRenderer.h` — decupla renderização SDL3/GPU
- [x] `IInputManager.h` — decupla input SDL3
- [x] `IPhysicsWorld.h` — contrato de física 2D
- [x] `IAudioEngine.h` — contrato de áudio (implementação futura)
- [x] `IUiHost.h` — decupla Titan UI
- [x] `IScriptHost.h` — decupla Nucleo scripting
- [x] `runtime.h` — umbrella include

Implementações concretas continuam em `src/graphics`/`src/input`/etc.;
mudamos apenas o lado dos consumidores para depender da interface.

> **Próximo passo (Fase 2):** migrar `#include "Renderer.h"` em componentes
> e plugins para `#include "runtime/IRenderer.h"`. Implementações concretas
> herdam as interfaces — adicionar `public IRenderer` em `Renderer`, etc.

## Fase 2 — Engine API estabilizada

- [ ] Mover símbolos do Core para `src/include/core/` com `Core.h` umbrella.
- [ ] Marcar APIs estáveis com `LIGHTNING_API_STABLE`/`_EXPERIMENTAL`.
- [ ] Bump para `LIGHTNING_ENGINE_API_VERSION = 2` se necessário.
- [ ] Habilitar carregamento dinâmico real (`LoadLibrary`/`dlopen`) no
  `PluginManager` (hoje só fluxo in-memory).

## Fase 3 — Reorganização física do repositório

Só agora arquivos são **movidos**:

- [ ] `git mv src/include/* engine/core/...` conforme tabela em
  [`01-architecture.md`](01-architecture.md#12-mapeamento-do-código-atual--alvo).
- [ ] Atualizar `LightningEngine.vcxproj` e `Makefile` em paralelo.
- [ ] Atualizar `compile_commands.json`.
- [ ] Sweep de includes (`#include "Renderer.h"` → `#include "runtime/render/Renderer.h"`).

> Esta fase deve ser **uma PR isolada** (apenas movimentação) seguida de
> outra PR que ajusta sintaxe.

## Fase 4 — Plugins oficiais externos

- [ ] Extrair `Hurricane` (partículas) como plugin oficial.
- [ ] Extrair `Equinox` (materiais) como plugin oficial.
- [ ] Provar carregamento dinâmico em release build.

## Fase 5 — WASM (escopo aberto)

- [ ] Avaliar Wasmtime / Wasmer.
- [ ] Definir API C plana correspondente a `IEnginePlugin`.
- [ ] Sandbox de plugins user-generated.

## Critérios anti-regressão

Em qualquer fase, são bloqueios para merge:

1. Build Debug x64 / Release x64 quebrado.
2. Core dependendo de Runtime ou Plugins.
3. Plugin oficial sem `LightningPluginApiVersion`.
4. Documento desatualizado em relação ao código nesta pasta.
