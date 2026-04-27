# 7. Bridge Rust/Tauri e bindings JS

A integração do core C++ com a UI passa por uma camada de bindings que
**nunca** vive dentro do Core. Está em
[`src/include/EditorBridge.h`](../../src/include/EditorBridge.h) hoje e
deve migrar para `engine/bindings/rust_bridge` (Fase 3).

Para o lado do frontend (Next.js + Tauri), ver também
[`Docs/UI-Engine-Integration.md`](../../Docs/UI-Engine-Integration.md).

## 7.1 Três caminhos possíveis

### A) FFI direto (C ABI)

```cpp
extern "C" void LightningEngine_Start();
extern "C" void LightningEngine_Stop();
extern "C" const char* LightningEngine_GetStatusJson();
```

```rust
extern "C" {
    fn LightningEngine_Start();
    fn LightningEngine_Stop();
    fn LightningEngine_GetStatusJson() -> *const c_char;
}
```

- ✔ Simples para começar
- ✔ Zero overhead
- ✖ Engine roda **dentro** do processo Tauri — crash do core derruba a UI
- ✖ Difícil escalar para hot-reload

### B) IPC (recomendado durante o MVP)

Engine roda como **processo separado**. UI fala via JSON-RPC sobre stdio,
named pipe, ou TCP loopback. **É o que está implementado hoje** com o
arquivo `editor-bridge-status.json` lido pelo polling do Tauri.

- ✔ Crash isolado
- ✔ Hot-reload do core sem matar a UI
- ✔ Mesmo protocolo serve um eventual editor remoto
- ✖ Latência maior; serialização precisa ser eficiente

### C) Embedding (futuro)

Engine empacotada como `lightningengine.dll` consumida via FFI rico
(rust bindings gerados a partir do `TypeRegistry`). Caminho de longo prazo.

## 7.2 Estado atual da bridge

```
[ Core C++ ] ─ EditorBridge::SaveStatusSnapshot()
       │            │
       │            ▼
       │   editor-bridge-status.json  (no diretório do exe)
       │
       ▼
[ Host Rust Tauri ] ─ thread polling 500 ms
       │            │
       │            ▼
       │   app.emit("motor-status", payload)
       │
       ▼
[ Camada TS ] ─ tauriService.onStatusChange
       │
       ▼
[ React/Next.js ] ─ setState
```

## 7.3 Geração de bindings JS via reflexão

Quando `TypeRegistry` estiver populado, o backend Rust expõe um comando
Tauri:

```rust
#[tauri::command]
fn list_components() -> Vec<ComponentSchema> { ... }
```

A UI consome um único endpoint e gera o Inspector dinamicamente. Isso
**substitui** mocks como `inspectorFields` em `editor-ui/src/App.tsx`.

## 7.4 Endpoints sugeridos do Engine API Layer

| Verbo | Rota                          | Descrição                            |
| ----- | ----------------------------- | ------------------------------------ |
| GET   | `/engine/status`              | Estado atual (substitui o JSON file) |
| POST  | `/engine/start` / `/stop`     | Lifecycle                            |
| GET   | `/types`                      | Reflexão completa                    |
| POST  | `/scene/entity`               | Criar entidade                       |
| POST  | `/scene/entity/:id/component` | Adicionar componente                 |
| GET   | `/scene`                      | Snapshot da cena                     |
| GET   | `/plugins`                    | Lista plugins + estado               |
| POST  | `/plugins/:id/enable`         | Habilita/desabilita                  |

> Esses endpoints são **comandos Tauri** (`#[tauri::command]`), não HTTP
> de verdade. Mantemos o vocabulário REST porque serve documentação.

## 7.5 Critérios para promover do file-polling para IPC tipado

A migração do JSON em disco para IPC (mensagens versionadas via stdio ou
pipe) é desejável quando **qualquer um** destes ocorrer:

1. Necessidade de comandos UI → Core (hoje só Core → UI funciona de fato).
2. Mais de 3 mensagens distintas trafegando.
3. Latência percebida > 100 ms.
4. Editor precisa de stream contínuo (logs, profiler).

Quando migrar, definir `ProtocolVersion` no payload e validar nos dois
lados.
