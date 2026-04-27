# Lightning Engine — Especificação de Expansão

> **Documento normativo.** Define a arquitetura-alvo, contratos de código e
> regras inegociáveis para expandir a Lightning Engine.
> Estado de implementação atualizado em: 2026-04-27.

---

## 1. Arquitetura em três camadas

A engine é dividida em três camadas com separação estrita de dependências.
Setas de dependência **só apontam para baixo** — camadas superiores não
podem ser incluídas por camadas inferiores.

```
┌──────────────────────────────────────┐
│  PLUGINS (externos / oficiais)       │  Carregados dinamicamente
└──────────────────┬───────────────────┘
                   │  consome interfaces de →
┌──────────────────▼───────────────────┐
│  RUNTIME (módulos internos)          │  Expansível
│  Render · Áudio · Input · Física     │
│  UI (Titan) · Scripting (Nucleo)     │
└──────────────────┬───────────────────┘
                   │  consome interfaces de →
┌──────────────────▼───────────────────┐
│  CORE (imutável, mínimo, estável)    │  Nunca depende de Runtime/Plugins
│  Lifecycle · Scheduler · EventBus   │
│  PluginManager · TypeRegistry        │
│  EngineApiVersion · Memória          │
└──────────────────────────────────────┘
```

### 1.1 Core — responsabilidades

| Componente         | Header de referência                                |
| ------------------ | --------------------------------------------------- |
| Lifecycle / loop   | `src/include/Engine.h`                              |
| Sistema de eventos | `src/include/EventBus.h`                            |
| Interface de plugins | `src/include/PluginContracts.h`                   |
| Gerenciador de plugins | `src/include/PluginManager.h`                   |
| Reflexão (registro manual) | `src/include/TypeRegistry.h`              |
| Versão da ABI      | `src/include/EngineApiVersion.h`                    |

**Regra:** Core **não** faz `#include` de nenhum header de Runtime ou Plugin.

### 1.2 Runtime — módulos atuais

Cada módulo expõe uma **interface pura** em `src/include/runtime/` e tem
implementação concreta em `src/graphics`, `src/input`, etc.

| Interface             | Implementação concreta                  |
| --------------------- | --------------------------------------- |
| `runtime/IRenderer.h` | `src/include/Renderer.h` (SDL3 GPU)     |
| `runtime/IInputManager.h` | `src/include/InputManager.h` (SDL3) |
| `runtime/IPhysicsWorld.h` | `src/include/Physics.h` (kinematic) |
| `runtime/IAudioEngine.h` | Pendente de implementação             |
| `runtime/IUiHost.h`   | `src/include/gui/TitanUI.h` (Titan)     |
| `runtime/IScriptHost.h` | `src/include/Nucleo.h` (Nucleo VM)    |

### 1.3 Plugins — extensões dinâmicas

- Novos componentes, sistemas, IA, pathfinding.
- Ferramentas editoriais (painéis, importadores).
- Integrações externas (serviços, SDKs de terceiros).

---

## 2. Estrutura de diretórios atual → alvo

```
src/
  include/
    EngineApiVersion.h        ← Core: ABI
    EngineApiVersion.h        ← Core: ABI
    TypeRegistry.h            ← Core: reflexão
    EventBus.h                ← Core: eventos
    PluginContracts.h         ← Core: interface de plugin
    PluginManager.h           ← Core: ciclo de vida de plugins
    Engine.h                  ← Core: lifecycle
    runtime/                  ← Interfaces puras do Runtime
      IRenderer.h
      IInputManager.h
      IPhysicsWorld.h
      IAudioEngine.h
      IUiHost.h
      IScriptHost.h
      runtime.h               ← umbrella include

examples/
  plugins/
    HelloPlugin/              ← Template de plugin mínimo

docs/
  developers/                 ← Documentação para contribuidores do core
  enduser/                    ← Documentação para autores de plugins
```

---

## 3. Sistema de plugins

### 3.1 Interface e ABI

Todo plugin implementa `IEnginePlugin` (em `src/include/PluginContracts.h`)
e exporta três símbolos `extern "C"` com ABI estável:

```cpp
// Obrigatórios em qualquer plugin nativo
extern "C" {
    LIGHTNING_PLUGIN_EXPORT uint32_t LightningPluginApiVersion();
    LIGHTNING_PLUGIN_EXPORT IEnginePlugin* LightningPluginCreate();
    LIGHTNING_PLUGIN_EXPORT void LightningPluginDestroy(IEnginePlugin*);
}
```

`LIGHTNING_PLUGIN_EXPORT` é `__declspec(dllexport)` no Windows e
`__attribute__((visibility("default")))` em Linux/macOS — definido em
`src/include/EngineApiVersion.h`.

### 3.2 Ciclo de vida (cinco hooks)

```
Discovered → Loaded → Registered → Active
                           ▲            │ Deactivate
                           └────────────┘
                  Unload ←─────────────┘
```

| Hook            | Quando usar                                             |
| --------------- | ------------------------------------------------------- |
| `OnLoad()`      | Alocar recursos próprios. Não tocar no Core ainda.      |
| `OnRegister()`  | Registrar tipos, componentes, sistemas no TypeRegistry. |
| `OnActivate()`  | Assinar eventos no EventBus. Abrir conexões.            |
| `OnDeactivate()`| Cancelar assinaturas (espelho de OnActivate).           |
| `OnUnload()`    | Liberar recursos (espelho de OnLoad).                   |

### 3.3 Manifesto (`plugin.json`)

```json
{
  "id": "com.studio.myplugin",
  "name": "My Plugin",
  "version": "1.0.0",
  "engineVersionMin": "0.1.0",
  "engineVersionMax": "",
  "scope": "Project",
  "category": "Gameplay",
  "dependencies": [],
  "permissions": ["FileSystem"],
  "entryNative": "MyPlugin.dll"
}
```

### 3.4 Risco de ABI e mitigação

| Risco                                     | Mitigação adotada                                   |
| ----------------------------------------- | --------------------------------------------------- |
| Vtable C++ diferente entre compiladores   | Handshake via `LightningPluginApiVersion()` antes de qualquer vtable call |
| Mudança de layout de `PluginManifest`     | Bump de `LIGHTNING_ENGINE_API_VERSION`              |
| Plugins crashando o core                  | Roadmap: sandbox WASM para plugins user-generated   |

---

## 4. Sistema de reflexão (TypeRegistry)

Reflexão manual via registro explícito — sem geração de código, sem UHT.

```cpp
// Registra tipo e campos (em OnRegister do plugin)
LE_REGISTER_TYPE(Health)
    .Field("current", &Health::current)
    .Field("max",     &Health::max);

// Acesso por nome (editor, serialização)
const TypeInfo* info = TypeRegistry::Get().Find("Health");
```

**Usos válidos:** serialização JSON/YAML, Inspector do editor, instanciação
dinâmica, bindings Rust/JS via Tauri.

**Proibido:** chamada dentro de hot-loop (cache o `TypeInfo*` no setup).

---

## 5. ECS — recomendação

O sistema atual usa `Node` + `Component`. Para máxima extensibilidade, o
Core deve evoluir para expor:

- `Entity` — ID numérico (`uint32_t`)
- `ComponentStore<T>` — storage denso por tipo
- `ISystem` — interface de sistema com `Update(float dt)`

Plugins adicionam novos `Component` e `ISystem` registrados via
`TypeRegistry`. O Core não conhece nenhum deles.

---

## 6. Comunicação entre módulos

**Regra:** módulos não se chamam diretamente quando o efeito é não-local.
Use `EventBus<T>`:

```cpp
// Emissor (Physics)
engine.OnCollision().Emit({ entityA, entityB, impulse });

// Assinante (plugin de gameplay)
tok = engine.OnCollision().Subscribe([](const CollisionEvent& ev) {
    // ...
});

// Sempre cancelar em OnDeactivate
engine.OnCollision().Unsubscribe(tok);
```

Ver detalhes em `docs/developers/03-event-bus.md`.

---

## 7. Integração com Rust/Tauri e Next.js

### 7.1 Estratégia atual — IPC via arquivo + polling

```
[C++ Engine]  EditorBridge::SaveStatusSnapshot()
                    → editor-bridge-status.json

[Rust/Tauri]  polling 500 ms → app.emit("motor-status", payload)

[Next.js UI]  tauriService.onStatusChange → React setState
```

### 7.2 Caminhos de evolução

| Opção               | Quando usar                                         |
| ------------------- | --------------------------------------------------- |
| IPC (atual)         | MVP, hot-reload, engine como processo separado      |
| FFI direto (C ABI)  | Embedding de alta performance sem isolamento        |
| gRPC / WebSocket    | Editor remoto, múltiplos clientes simultâneos       |

**Regra:** o frontend Next.js **nunca** conhece a engine diretamente.
Toda comunicação passa pelo layer `tauriService` → comandos Tauri.

### 7.3 Engine API Layer (comandos Tauri)

| Comando               | Direção       | Descrição                       |
| --------------------- | ------------- | ------------------------------- |
| `get_motor_status`    | UI → Core     | Estado atual                    |
| `start_motor`         | UI → Core     | Liga o runtime                  |
| `stop_motor`          | UI → Core     | Desliga o runtime               |
| `motor-status` (evt)  | Core → UI     | Push de atualização             |
| `list_components`*    | UI → Core     | Reflexão para o Inspector       |

*Planejado — depende de TypeRegistry populado.

Ver `Docs/UI-Engine-Integration.md` e `docs/developers/07-rust-tauri-bridge.md`.

---

## 8. Scripting

Nucleo (VM própria, linguagem `.spark`) está implementado. Caminho de evolução:

| Opção       | Custo | Isolamento | Indicado para               |
| ----------- | ----- | ---------- | --------------------------- |
| Nucleo (.spark) | Baixo | Nenhum | Scripting de gameplay atual |
| Lua         | Médio | Parcial    | Scripting leve por usuários |
| WASM        | Alto  | Total      | Plugins sandboxed           |

Decisão estratégica: manter Nucleo para gameplay; avaliar WASM apenas
quando plugins user-generated exigirem sandboxing real.

---

## 9. Pipeline de build e versionamento

```
Engine Core (.exe / .lib)
  └── LIGHTNING_ENGINE_API_VERSION = 1

Plugins (.dll / .so)
  └── LightningPluginApiVersion() → deve retornar 1
  └── Rejeitado se versão divergir
```

Política de bump da versão: qualquer mudança que altere layout binário
(adicionar `virtual` puro, mudar structs públicas) exige bump.
Ver `docs/developers/05-engine-api-versioning.md`.

---

## 10. Problemas previstos e mitigações

| Problema                    | Mitigação implementada / planejada                    |
| --------------------------- | ----------------------------------------------------- |
| Dependência circular        | EventBus + interfaces puras em `runtime/`             |
| Plugin quebrando o core     | Handshake de versão; roadmap WASM sandbox             |
| Debug difícil               | `Logger.h` centralizado; `EditorBridge` para o editor |
| Reflexão em hot-loop        | Cache `TypeInfo*` — documentado em `04-reflection.md` |
| Complexidade prematura      | Fases incrementais — ver `06-restructure-roadmap.md`  |
| ABI quebrada entre builds   | Mesma toolchain (MSVC x64); `EngineApiVersion.h`      |

---

## 11. Roadmap de implementação

| Fase | Status       | Entrega                                                    |
| ---- | ------------ | ---------------------------------------------------------- |
| 0    | ✅ Concluída | Regras, scaffolding, `EngineApiVersion.h`, `TypeRegistry.h`, `HelloPlugin` |
| 1    | ✅ Concluída | Interfaces puras do Runtime (`src/include/runtime/`)       |
| 2    | ⬜ Pendente  | Implementações concretas herdam interfaces; migrar `#include` |
| 3    | ⬜ Pendente  | Reorganização física do repositório (`engine/core` etc.)   |
| 4    | ⬜ Pendente  | Hurricane e Equinox como plugins oficiais dinâmicos        |
| 5    | ⬜ Aberto    | Sandbox WASM para plugins user-generated                   |

Detalhamento completo: `docs/developers/06-restructure-roadmap.md`.