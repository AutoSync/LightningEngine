# Integração UI ↔ Core da Engine

> Documento técnico que descreve como o editor (Next.js + Tauri) conversa com o
> runtime C++ da Lightning Engine. Atualizado para a migração **Vite → Next.js 15**.

## 1. Visão geral em camadas

```
┌─────────────────────────────────────────────────────────────────┐
│  UI (Next.js 15 / React 19)                                     │
│  editor-ui/pages/*  +  editor-ui/src/App.tsx                    │
│              │                                                  │
│              │  invoke()  /  listen()                           │
│              ▼                                                  │
│  Camada de serviço TS                                           │
│  editor-ui/src/services/tauri-service.ts                        │
│              │                                                  │
│              │  IPC do Tauri (JSON-RPC interno)                 │
│              ▼                                                  │
│  Host Rust (Tauri 2)                                            │
│  editor-ui/src-tauri/src/main.rs                                │
│              │                                                  │
│              │  Polling de arquivo + (futuro) FFI               │
│              ▼                                                  │
│  Core C++ da Engine                                             │
│  src/include/EditorBridge.h  +  src/core/EditorBridge.cpp       │
└─────────────────────────────────────────────────────────────────┘
```

A UI **nunca** fala diretamente com o C++. Toda comunicação atravessa o host
Rust do Tauri, que oferece dois modelos:

1. **Comandos** (`invoke`) — request/response síncronos, iniciados pela UI.
2. **Eventos** (`emit` / `listen`) — push assíncrono do core para a UI.

## 2. Pilha do frontend (após migração para Next.js)

| Item              | Antes (Vite)              | Agora (Next.js 15)                      |
| ----------------- | ------------------------- | --------------------------------------- |
| Dev server        | `vite` em `:3000`         | `next dev -p 3000`                      |
| Entry HTML        | `index.html` + `main.tsx` | `pages/_document.tsx` + `pages/_app.tsx`|
| Página principal  | `src/App.tsx` (mount JS)  | `pages/index.tsx` → `dynamic(App)`      |
| Build artifact    | `dist/`                   | `out/` (`output: 'export'`)             |
| Tauri `frontendDist` | `../dist`              | `../out`                                |

### Por que Pages Router + static export?

- O Tauri serve assets estáticos a partir do disco — não há servidor Node em
  produção. `output: 'export'` gera um bundle 100% estático.
- `App.tsx` depende de APIs do navegador (`window`) e do `@tauri-apps/api`,
  que **não existem em SSR**. Por isso `pages/index.tsx` carrega o app via
  `next/dynamic(..., { ssr: false })`.
- Pages Router evita a complexidade de Server Components (RSC) — todo o
  editor é client-side, alinhado ao modelo do Tauri.

## 3. Camada TS de serviço (`tauri-service.ts`)

Interface mínima estável consumida pelo `App.tsx`:

```ts
interface MotorStatus {
  running: boolean;
  fps?: number;
  scene?: string;
  project?: string;
  lastChange?: string;
}

tauriService.startMotor(): Promise<void>
tauriService.stopMotor(): Promise<void>
tauriService.getStatus(): Promise<MotorStatus>
tauriService.onStatusChange(cb): Promise<() => void>  // retorna unlisten
```

Pontos importantes:

- O serviço importa `@tauri-apps/api/core` e `@tauri-apps/api/event` de forma
  **dinâmica** (`await import(...)`). Quando rodando fora do Tauri (browser
  comum durante `npm run dev`), o import falha silenciosamente e o serviço
  retorna **mocks**, permitindo desenvolvimento puramente web.
- Toda nova feature de UI deve passar por este serviço — nunca chamar
  `invoke`/`listen` diretamente do componente.

## 4. Host Rust (`src-tauri/src/main.rs`)

### Comandos expostos

| Comando             | Assinatura                                  | Uso na UI                       |
| ------------------- | ------------------------------------------- | ------------------------------- |
| `start_motor`       | `() -> Result<MotorStatus, String>`         | `tauriService.startMotor()`     |
| `stop_motor`        | `() -> Result<MotorStatus, String>`         | `tauriService.stopMotor()`      |
| `get_motor_status`  | `() -> Result<MotorStatus, String>`         | `tauriService.getStatus()`      |

Registrados em `tauri::generate_handler![...]` no final do `main()`.

### Eventos emitidos

| Evento          | Payload         | Quando                                                    |
| --------------- | --------------- | --------------------------------------------------------- |
| `motor-status`  | `MotorStatus`   | (1) setup inicial; (2) a cada `start/stop_motor`; (3) quando o polling detecta mudança no JSON do core. |

### Fontes de verdade do estado

1. **Estado interno** (`AppState.motor`): mutado pelos comandos `start_motor`/
   `stop_motor`. Usado quando o core C++ ainda não está rodando.
2. **Snapshot do core** (`editor-bridge-status.json`): lido por
   `read_cpp_status()` em até três caminhos candidatos:
   - `../x64/Debug/editor-bridge-status.json`
   - `../x64/Release/editor-bridge-status.json`
   - `editor-bridge-status.json`
   Quando presente, **sobrepõe** o estado interno (`effective_status`).

Uma thread de polling (500 ms) compara o último snapshot lido com o atual e
emite `motor-status` somente quando há diferença real (`PartialEq`).

## 5. Core C++ — `EditorBridge`

Definido em [`src/include/EditorBridge.h`](../src/include/EditorBridge.h) e
implementado em [`src/core/EditorBridge.cpp`](../src/core/EditorBridge.cpp).

```cpp
namespace LightningEngine::EditorBridge {
    struct Status {
        bool running = false;
        float fps = 0.f;
        std::string project;
        std::string scene;
        std::string lastChange;
    };

    void SetProject(const std::string&);
    void SetScene(const std::string&);
    void SetRunning(bool);
    void SetFps(float);
    void SetLastChange(const std::string&);

    Status      GetStatus();
    std::string GetStatusJson();
    void        SaveStatusSnapshot(const std::string& path = "editor-bridge-status.json");
    void        Reset();
}
```

### Contrato do snapshot JSON

Forma exata gerada por `GetStatusJson()` (consumida pelo Rust e desserializada
em `MotorStatus`):

```json
{
  "running": true,
  "fps": 60.0,
  "project": "MyProject",
  "scene": "main_scene.lescene",
  "lastChange": "Saved scene"
}
```

Notas:

- Todos os campos são obrigatórios na escrita.
- Strings passam por `jsonEscape` (escape de aspas, controle, unicode `<0x20`).
- O acesso é guardado por `std::mutex` — pode ser chamado de qualquer thread
  do runtime da engine.

### Como o core publica estado

Padrão consolidado (vide histórico em `src/editor/EditorApp.h.bak`):

```cpp
EditorBridge::SetRunning(isPlaying);
EditorBridge::SetFps(curFps);
EditorBridge::SetScene(currentScenePath);
EditorBridge::SetProject(projectName);
EditorBridge::SetLastChange(lastChange);
EditorBridge::SaveStatusSnapshot();   // chamada típica: a cada N frames
```

`SaveStatusSnapshot()` reescreve o arquivo `editor-bridge-status.json` no
diretório de trabalho do executável. O Tauri detecta a mudança via polling
e propaga para a UI.

## 6. Fluxos de exemplo

### 6.1 UI lendo status inicial

```
React (App.tsx)
  └─ useEffect: tauriService.getStatus()
       └─ invoke('get_motor_status')
            └─ Rust: effective_status()
                 ├─ lê editor-bridge-status.json (se existir) ──► retorna ao TS
                 └─ senão, retorna AppState.motor                ──► retorna ao TS
```

### 6.2 Core empurrando atualização

```
C++  EditorBridge::SetFps(...) + SaveStatusSnapshot()
  └─ arquivo editor-bridge-status.json reescrito
       └─ Rust polling thread detecta diff
            └─ app.emit("motor-status", payload)
                 └─ TS event.listen → tauriService.onStatusChange callback
                      └─ React setState
```

### 6.3 UI iniciando o motor

```
botão "Play" → tauriService.startMotor()
  └─ invoke('start_motor')
       └─ Rust: AppState.motor.running = true
            └─ emit("motor-status", ...)
                 └─ React atualiza badge "Running"
```

> ⚠️ Hoje `start_motor`/`stop_motor` mutam **apenas** o estado do host Rust.
> Quando o core C++ for embarcado in-process (FFI), esses comandos devem
> chamar `LightningEngine::Engine::Start/Stop` diretamente.

## 7. Adicionando um novo canal de integração

Checklist para adicionar, por exemplo, `open_scene(path)`:

1. **Core C++**
   - Adicionar API em `EditorBridge.h/.cpp` (ou outro módulo) para receber
     o pedido. Ex.: fila de comandos lida pelo `Engine::Update`.
2. **Host Rust** (`src-tauri/src/main.rs`)
   - Criar `#[tauri::command] fn open_scene(path: String, ...) -> Result<...>`.
   - Registrar em `tauri::generate_handler![...]`.
3. **Camada TS** (`tauri-service.ts`)
   - Adicionar método `openScene(path: string): Promise<void>` que faz
     `invoke('open_scene', { path })`.
   - Manter fallback mock para o modo browser puro.
4. **UI** (`src/App.tsx` ou novo componente)
   - Consumir somente o método tipado do serviço — nunca `invoke` direto.
5. **Eventos** (se houver retorno assíncrono)
   - Definir nome estável (ex.: `scene-loaded`), serializar payload via
     `serde::Serialize`, chamar `app.emit(...)` no Rust.
   - Adicionar `tauriService.onSceneLoaded(cb)` espelhando o padrão de
     `onStatusChange`.

## 8. Build & execução

| Cenário                          | Comando                              | Saída                          |
| -------------------------------- | ------------------------------------ | ------------------------------ |
| UI no browser (sem Tauri/engine) | `npm run dev` em `editor-ui/`        | `http://localhost:3000`        |
| Editor desktop (dev)             | `npm run tauri:dev` em `editor-ui/`  | janela Tauri + Next dev        |
| Editor desktop (release)         | `npm run tauri:build` em `editor-ui/`| binário em `src-tauri/target/` |
| Core C++ (Debug x64)             | task `Build Debug x64 (MSBuild)`     | `x64/Debug/*.exe`              |

Para validação ponta a ponta:

1. Build do core em Debug x64 e execute para gerar
   `x64/Debug/editor-bridge-status.json`.
2. `npm run tauri:dev` em `editor-ui/` — o host Rust encontra o JSON pelo
   caminho relativo `../x64/Debug/...` e empurra atualizações para a UI.

## 9. Limitações conhecidas / próximos passos

- Comunicação atual é **unidirecional fácil** (C++ → UI via arquivo) e
  **bidirecional limitada** (UI → C++ ainda só simulada no Rust). Próximo
  passo: substituir o JSON-no-disco por:
  - **Embedding in-process** via FFI (Rust chama uma `lightningengine.dll`
    que expõe API C); ou
  - **IPC local** (named pipe / TCP loopback) com mensagens versionadas.
- Não há schema versionado para `MotorStatus`. Antes de evoluir o contrato,
  criar `protocolVersion: number` no payload e validar em ambos os lados.
- O fallback mock do `tauri-service.ts` retorna `running: true`, o que pode
  mascarar bugs no fluxo Tauri durante teste no browser puro. Considerar
  flag `import.meta.env`/`process.env.NEXT_PUBLIC_TAURI` para detectar.

## 10. Arquivos-chave

- [editor-ui/pages/index.tsx](../editor-ui/pages/index.tsx)
- [editor-ui/pages/_app.tsx](../editor-ui/pages/_app.tsx)
- [editor-ui/pages/_document.tsx](../editor-ui/pages/_document.tsx)
- [editor-ui/next.config.js](../editor-ui/next.config.js)
- [editor-ui/src/App.tsx](../editor-ui/src/App.tsx)
- [editor-ui/src/services/tauri-service.ts](../editor-ui/src/services/tauri-service.ts)
- [editor-ui/src-tauri/src/main.rs](../editor-ui/src-tauri/src/main.rs)
- [editor-ui/src-tauri/tauri.conf.json](../editor-ui/src-tauri/tauri.conf.json)
- [src/include/EditorBridge.h](../src/include/EditorBridge.h)
- [src/core/EditorBridge.cpp](../src/core/EditorBridge.cpp)
