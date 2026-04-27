# 1. Conceitos básicos

Antes de escrever código, entenda os três conceitos que sustentam todo
plugin da Lightning Engine.

## 1.1 Plugin

Um **plugin** é uma biblioteca dinâmica (`.dll` no Windows, `.so` no Linux,
`.dylib` no macOS) acompanhada de um **manifesto** (`plugin.json`).

```
MyPlugin/
├── plugin.json        ← manifesto (obrigatório)
├── MyPlugin.dll       ← binário nativo (Windows)
└── assets/            ← opcional: assets que vêm com o plugin
```

A engine carrega o manifesto primeiro, valida permissões e versão da API,
e só então abre a biblioteca.

## 1.2 Ciclo de vida

```
Discovered → Loaded → Registered → Active
                         ▲           │
                         └───────────┘  Deactivate
```

Você implementa **cinco métodos**:

| Hook            | Quando rodar                                         |
| --------------- | ---------------------------------------------------- |
| `OnLoad()`      | Bib. mapeada na memória. Aloque recursos próprios.   |
| `OnRegister()`  | Registre componentes, sistemas, tipos no core.       |
| `OnActivate()`  | Assine eventos, abra conexões.                       |
| `OnDeactivate()`| Cancele assinaturas (espelho de `OnActivate`).       |
| `OnUnload()`    | Libere recursos próprios (espelho de `OnLoad`).      |

**Regra de ouro:** o que você criou em `OnLoad/OnActivate` deve ser
desfeito em `OnUnload/OnDeactivate`. Isso é o que permite **hot-reload**.

## 1.3 Comunicação por eventos

Plugins **não** chamam o core diretamente. Você se inscreve no
`EventBus<T>` para receber notificações:

```cpp
EventToken tok = engine.OnCollision().Subscribe(
    [](const CollisionEvent& ev) {
        // sua lógica
    });
```

E publica eventos próprios:

```cpp
struct InventoryFullEvent { Entity owner; };
EventBus<InventoryFullEvent> onInventoryFull;

onInventoryFull.Emit({ entity });
```

> Sempre cancele assinaturas (`Unsubscribe`) em `OnDeactivate`. Esquecer
> isso causa crash quando a engine descarrega o plugin.

## 1.4 Permissões

Tudo que sai do sandbox do plugin (rede, disco, dispositivos) precisa
ser **declarado no manifesto**. O usuário aceita explicitamente. Sem a
permissão correspondente, a chamada falha em runtime.

| Permissão     | Permite                                       |
| ------------- | --------------------------------------------- |
| `FileSystem`  | Ler/escrever arquivos do projeto              |
| `Network`     | Sockets / HTTP                                |
| `Device`      | Câmera, microfone, dispositivos USB           |
| `ScriptApi`   | Adicionar APIs ao runtime de scripting        |
| `EditorUI`    | Adicionar painéis/menus ao editor             |

## 1.5 Próximos passos

- [Escreva seu manifesto](02-plugin-manifest.md)
- [Implemente em C++](03-c-api-plugin.md)
- [Empacote e distribua](04-distributing.md)
