# 2. Sistema de plugins

A Lightning Engine carrega extensões dinâmicas seguindo um contrato C ABI
estável. Os tipos C++ que orquestram isso já existem em
[`src/include/PluginContracts.h`](../../src/include/PluginContracts.h) e
[`src/include/PluginManager.h`](../../src/include/PluginManager.h).

## 2.1 Contrato C ABI (entry point)

Todo plugin nativo expõe **duas funções `extern "C"`** com símbolos
estáveis. Esses símbolos sobrevivem a diferenças de toolchain e são o que
o `PluginManager` resolve via `LoadLibrary`/`dlopen`.

```cpp
// HelloPlugin.cpp
#include "PluginContracts.h"
#include "EngineApiVersion.h"

using namespace LightningEngine;

class HelloPlugin : public IEnginePlugin {
public:
    const PluginManifest& Manifest() const override { return manifest; }
    bool OnLoad()        override { return true; }
    void OnRegister()    override {}
    void OnActivate()    override {}
    void OnDeactivate()  override {}
    void OnUnload()      override {}

private:
    PluginManifest manifest{
        /*id*/      "com.example.hello",
        /*name*/    "Hello Plugin",
        /*version*/ "0.1.0",
        /*minEng*/  "0.1.0",
        /*maxEng*/  "",
        /*scope*/   PluginScope::Project,
    };
};

// ── Entry points obrigatórios ────────────────────────────────────────
extern "C" {
    LIGHTNING_PLUGIN_EXPORT std::uint32_t LightningPluginApiVersion()
    {
        return LIGHTNING_ENGINE_API_VERSION;
    }

    LIGHTNING_PLUGIN_EXPORT IEnginePlugin* LightningPluginCreate()
    {
        return new HelloPlugin();
    }

    LIGHTNING_PLUGIN_EXPORT void LightningPluginDestroy(IEnginePlugin* p)
    {
        delete p;
    }
}
```

`LIGHTNING_PLUGIN_EXPORT` é um macro de visibilidade definido em
[`EngineApiVersion.h`](../../src/include/EngineApiVersion.h). O `Manager`
**recusa** carregar bibliotecas cuja versão da API não bate com a do core.

## 2.2 Manifest

`PluginManifest` em `PluginContracts.h` carrega:

- `id` — id único reverse-DNS (ex.: `com.studio.combat`).
- `version` / `engineVersionMin` / `engineVersionMax` — semver.
- `scope` — `Global` (instalado por usuário) ou `Project` (vinculado ao .leproj).
- `category` / `subcategory` — usado pelo editor para agrupar.
- `dependencies` — lista de `id`s requeridos (carregados antes).
- `permissions` — bitmask `PluginPermission` (FileSystem, Network, Device,
  ScriptApi, EditorUI). O Core valida antes de autorizar.
- `entryNative` / `entryCSharp` / `entryIgnite` — caminhos para cada tipo
  de runtime (um plugin pode misturar nativo + script).

> Um plugin distribuído inclui um arquivo `plugin.json` ao lado do binário,
> consumido pelo `PluginManager` antes de chamar `LightningPluginCreate`.

## 2.3 Ciclo de vida (estados)

```
Discovered ──Load()──► Loaded ──Register()──► Registered ──Activate()──► Active
     ▲                                                  │
     │                                            Deactivate()
     │                                                  ▼
     └────────────── Unload() ◄─────────── Registered ──┘
```

Mapeado 1:1 a `PluginState` em
[`PluginManager.h`](../../src/include/PluginManager.h):

| Estado       | Significado                                                       |
| ------------ | ----------------------------------------------------------------- |
| `Discovered` | Manifest lido, biblioteca não carregada                           |
| `Loaded`     | DLL/SO mapeada, `OnLoad()` retornou `true`                        |
| `Registered` | Plugin registrou seus tipos/sistemas no Core                       |
| `Active`     | Plugin recebe `OnUpdate`/eventos                                   |
| `Disabled`   | Usuário desabilitou; permanece descoberto, não é carregado         |
| `Failed`     | Erro em alguma transição; `lastError` preenchido                   |

### Quando implementar cada hook

- **`OnLoad()`** — alocar recursos próprios. **Não** mexer no Core ainda.
- **`OnRegister()`** — chamar `TypeRegistry::Register<T>()`, registrar
  componentes ECS, sistemas, comandos do editor.
- **`OnActivate()`** — assinar eventos no `EventBus`, abrir conexões.
- **`OnDeactivate()`** — espelho de `OnActivate` (cancelar assinaturas).
- **`OnUnload()`** — espelho de `OnLoad` (liberar recursos).

## 2.4 Riscos de ABI C++ e mitigações

`IEnginePlugin` é uma classe abstrata C++. Isso é **conveniente** para o
core (mesmo binário) mas **frágil** entre toolchains. Mitigações já
adotadas:

1. **Mesma toolchain garantida.** Plugins oficiais e o engine são
   compilados com o MSVC do build da .sln (Debug/Release x64). Plugins
   third-party devem documentar a toolchain.
2. **Funções `extern "C"`** para criar/destruir/identificar versão. O
   "vtable C++" só é exercido após o handshake de versão.
3. **Roadmap WASM** (escopo futuro): para plugins user-generated, mover
   para sandbox WASM com API C plana — eliminado o problema de ABI.

## 2.5 Segurança e permissões

`PluginPermission` é avaliado **antes** de cada operação sensível:

```cpp
if (!HasPermission(plugin->Manifest().permissions, PluginPermission::FileSystem)) {
    Log::Warn("plugin %s tentou acesso a FS sem permissão", id.c_str());
    return false;
}
```

O editor exibe a lista de permissões na hora de instalar e exige
consentimento explícito. Permissões adicionadas em update obrigam a
re-confirmação.

## 2.6 Hot-reload

`PluginManager::Unload(id)` segue por `Discovered` — o caminho está pronto
para fluxo `Unload → swap binário → Load → Register → Activate`. Boas
práticas para plugins:

- Não armazenar ponteiros para objetos do plugin **fora** do plugin (use
  IDs no `TypeRegistry`).
- Cancelar todas as assinaturas em `OnDeactivate`.
- Liberar memória própria em `OnUnload`.
