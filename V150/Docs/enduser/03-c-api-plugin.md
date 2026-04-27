# 3. Plugin nativo (C++ / C ABI)

Este guia mostra como escrever um plugin nativo do zero. Ele segue
**exatamente** o exemplo em
[`examples/plugins/HelloPlugin`](../../examples/plugins/HelloPlugin/README.md).

## 3.1 Toolchain

- Windows: MSVC 2022 (mesma toolchain do build da engine).
- Linux: GCC 11+ ou Clang 14+.
- CMake 3.20+.

> Plugins compilados com toolchain diferente da engine **podem** funcionar,
> mas não há garantia de ABI C++. As funções `extern "C"` sempre funcionam.

## 3.2 Estrutura do projeto

```
HelloPlugin/
├── CMakeLists.txt
├── plugin.json
├── include/
│   └── HelloPlugin.h
└── src/
    └── HelloPlugin.cpp
```

## 3.3 `CMakeLists.txt` mínimo

```cmake
cmake_minimum_required(VERSION 3.20)
project(HelloPlugin CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Aponte LIGHTNING_ENGINE_SDK para a pasta com headers públicos da engine.
# (em desenvolvimento: o próprio src/include do repo)
set(LIGHTNING_ENGINE_SDK "" CACHE PATH "Path to Lightning Engine public headers")
if (NOT LIGHTNING_ENGINE_SDK)
    message(FATAL_ERROR "Set -DLIGHTNING_ENGINE_SDK=<path>")
endif()

add_library(HelloPlugin SHARED src/HelloPlugin.cpp)
target_include_directories(HelloPlugin PRIVATE
    include
    ${LIGHTNING_ENGINE_SDK})
target_compile_definitions(HelloPlugin PRIVATE LIGHTNING_PLUGIN_BUILD)
```

## 3.4 Código do plugin

```cpp
// src/HelloPlugin.cpp
#include "PluginContracts.h"
#include "EngineApiVersion.h"
#include <cstdio>

using namespace LightningEngine;

namespace {
    PluginManifest gManifest{
        /*id*/      "com.example.hello",
        /*name*/    "Hello Plugin",
        /*version*/ "0.1.0",
        /*minEng*/  "0.1.0",
        /*maxEng*/  "",
        /*scope*/   PluginScope::Project,
    };

    class HelloPlugin : public IEnginePlugin {
    public:
        const PluginManifest& Manifest() const override { return gManifest; }

        bool OnLoad()       override { std::printf("[Hello] OnLoad\n");       return true; }
        void OnRegister()   override { std::printf("[Hello] OnRegister\n"); }
        void OnActivate()   override { std::printf("[Hello] OnActivate\n"); }
        void OnDeactivate() override { std::printf("[Hello] OnDeactivate\n"); }
        void OnUnload()     override { std::printf("[Hello] OnUnload\n"); }
    };
}

extern "C" {
    LIGHTNING_PLUGIN_EXPORT std::uint32_t LightningPluginApiVersion() {
        return LIGHTNING_ENGINE_API_VERSION;
    }

    LIGHTNING_PLUGIN_EXPORT IEnginePlugin* LightningPluginCreate() {
        return new HelloPlugin();
    }

    LIGHTNING_PLUGIN_EXPORT void LightningPluginDestroy(IEnginePlugin* plugin) {
        delete plugin;
    }
}
```

## 3.5 Build

```bash
cmake -B build -DLIGHTNING_ENGINE_SDK=<repo>/src/include
cmake --build build --config Release
```

Saída: `build/Release/HelloPlugin.dll` (Windows).

## 3.6 Instalação

Copie o binário e o manifesto para `<projeto>/Plugins/HelloPlugin/`:

```
MyGame/
└── Plugins/
    └── HelloPlugin/
        ├── plugin.json
        └── HelloPlugin.dll
```

Abra o editor → **Window → Plugin Manager** → o plugin aparece em
estado `Discovered`. Clique **Activate**.

## 3.7 Adicionando um componente refletido

```cpp
struct Health {
    float current = 100.f;
    float max     = 100.f;
};

void HelloPlugin::OnRegister() {
    LE_REGISTER_TYPE(Health)
        .Field("current", &Health::current)
        .Field("max",     &Health::max);
}
```

Imediatamente após `Activate`, o componente `Health` aparece no menu
**Add Component** do Inspector e na lista de tipos exposta para o
runtime de scripting.

## 3.8 Reagindo a eventos

```cpp
EventToken collisionTok = 0;

void HelloPlugin::OnActivate() {
    collisionTok = engine.OnCollision().Subscribe(
        [](const CollisionEvent& ev) {
            std::printf("[Hello] colisão %u <-> %u\n", ev.a, ev.b);
        });
}

void HelloPlugin::OnDeactivate() {
    engine.OnCollision().Unsubscribe(collisionTok);
    collisionTok = 0;
}
```

## 3.9 Erros comuns

| Sintoma                                                 | Causa                                                                |
| ------------------------------------------------------- | -------------------------------------------------------------------- |
| Plugin marcado `Failed: engine API mismatch`            | `LightningPluginApiVersion()` retorna número diferente do core       |
| Crash ao desativar plugin                               | Esqueceu `Unsubscribe` em `OnDeactivate`                             |
| Componente não aparece no Inspector                     | Registro feito em `OnLoad` em vez de `OnRegister`                    |
| Plugin não é descoberto                                 | `plugin.json` ausente ou pasta fora de `Plugins/<id>/`               |
| `dlopen`/`LoadLibrary` falha sem mensagem               | Toolchain de compilação diferente da engine; recompile               |
| Plugin chama `fopen` e nada acontece                    | `permissions` não inclui `FileSystem`                                |
