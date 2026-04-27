# 5. Versionamento da Engine API

`LIGHTNING_ENGINE_API_VERSION` é o **único contrato** que liga um plugin
binário ao core. Definido em
[`src/include/EngineApiVersion.h`](../../src/include/EngineApiVersion.h).

```cpp
// engine
#define LIGHTNING_ENGINE_API_VERSION 1
```

```cpp
// plugin
extern "C" std::uint32_t LightningPluginApiVersion() {
    return LIGHTNING_ENGINE_API_VERSION;
}
```

O `PluginManager` chama `LightningPluginApiVersion()` **antes** de
`LightningPluginCreate()`. Se a versão diferir, recusa carregar e marca
como `Failed` com mensagem clara.

## 5.1 Política de bump

| Mudança                                          | Bump? |
| ------------------------------------------------ | ----- |
| Adicionar campo no fim de `PluginManifest`       | Não   |
| Adicionar método **virtual puro** em `IEnginePlugin` | **Sim** |
| Adicionar método com default em `IEnginePlugin`  | Não (mas evite) |
| Mudar assinatura de função `extern "C"`          | **Sim** |
| Renomear/remover `EventBus<T>` público           | **Sim** |
| Adicionar novo `EventBus<T>` público             | Não   |
| Adicionar `PluginPermission` novo                | Não   |
| Mudar layout binário de `PluginManifest`         | **Sim** |

Bumps maiores fazem **breaking release** da engine (`v0.x` → `v0.x+1`).
Plugins precisam recompilar.

## 5.2 Range de compatibilidade no manifest

Cada plugin declara:

```json
{
  "engineVersionMin": "0.1.0",
  "engineVersionMax": "0.2.0"
}
```

O Manager rejeita plugin fora do range, mesmo que `LightningPluginApiVersion`
bata, porque a versão semântica também controla compat de comportamento
(não apenas binário).

## 5.3 Suporte a múltiplas versões no mesmo binário

Para plugins oficiais que precisam suportar múltiplas versões da engine:

```cpp
extern "C" std::uint32_t LightningPluginApiVersion() {
#if defined(LE_TARGET_API_2)
    return 2;
#else
    return 1;
#endif
}
```

Use diretivas `#if` em torno do código que usa APIs novas; nunca tente
detectar em runtime.
