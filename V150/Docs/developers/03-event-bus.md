# 3. Event bus

A comunicação entre Core, Runtime e plugins acontece via
[`EventBus<T>`](../../src/include/EventBus.h) — uma fila tipada, thread-safe,
com dispatch imediato e diferido.

## 3.1 Por que eventos?

A regra é: **nenhum módulo chama método de outro diretamente quando o efeito
não é local**. Isso elimina dependências circulares e permite que plugins
reajam a fatos do core sem precisarem ser conhecidos por ele.

```
[Physics] ──Emit(CollisionEvent)──► [EventBus]
                                       │
                                       ├──► [Audio]   (toca som)
                                       ├──► [VFX]     (spawn de partícula)
                                       └──► [Plugin]  (lógica de gameplay)
```

## 3.2 Definição de evento

Eventos são `struct`s simples e POD-friendly. Ficam no header do módulo
emissor:

```cpp
// engine/runtime/physics/CollisionEvent.h
namespace LightningEngine {
    struct CollisionEvent {
        Entity a;
        Entity b;
        float  impulse;
    };
}
```

> Não coloque ponteiros fortes no payload — eventos podem ser entregues
> em frames seguintes (`Post` + `FlushQueue`).

## 3.3 Padrão de uso

```cpp
EventBus<CollisionEvent> onCollision;

// (1) Emissor
onCollision.Emit({ a, b, impulse });   // imediato, mesma thread

// (2) Diferido (típico para frame boundary)
onCollision.Post({ a, b, impulse });
// ... no fim do tick:
onCollision.FlushQueue();

// (3) Assinante
EventToken tok = onCollision.Subscribe([](const CollisionEvent& ev) {
    // tratar...
});

// (4) Cancelar (sempre faça em OnDeactivate / OnUnload do plugin)
onCollision.Unsubscribe(tok);
```

## 3.4 Convenções

- **Imediato vs diferido**: use `Post` quando o handler pode mudar estado
  que outros sistemas no mesmo frame estão lendo. Use `Emit` quando o
  efeito é puramente local/sincronizado.
- **Naming**: `XxxEvent` no nome do tipo, instâncias `onXxx`.
- **Dono do bus**: o módulo que **emite** é dono. Plugins assinam, nunca
  registram seu próprio bus para serem ouvidos pelo core (inversão errada).
- **Tokens**: tokens de assinatura **vivem com o plugin** — perdê-los é
  vazamento que sobrevive ao `Unload`.
- **Thread-safety**: `Subscribe`/`Unsubscribe`/`Emit`/`Post`/`FlushQueue`
  já são thread-safe. Handlers, não.

## 3.5 Anti-padrões

- Postar `EventBus<T>` por valor em outro evento — bus é stateful.
- Capturar `this` cru em lambdas de plugin sem cancelar antes do `Unload`.
- Usar event bus como substituto de chamada local (overhead inútil).

## 3.6 Mapa dos buses do core (alvo)

| Bus                           | Emissor             | Consumidores típicos       |
| ----------------------------- | ------------------- | -------------------------- |
| `EventBus<TickEvent>`         | Scheduler do Core   | Sistemas, plugins ativos   |
| `EventBus<CollisionEvent>`    | Runtime/Physics     | Audio, VFX, gameplay       |
| `EventBus<InputEvent>`        | Runtime/Input       | UI, gameplay, plugins      |
| `EventBus<SceneLoadedEvent>`  | Core/Lifecycle      | Editor, salvamento, render |
| `EventBus<PluginStateChanged>`| Core/PluginManager  | Editor (UI de plugins)     |

> Quando criar um novo bus, registre-o nesta tabela na mesma PR.
