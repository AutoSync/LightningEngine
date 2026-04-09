# Lightning Engine Editor UI

Scaffold Vinext + Tauri para o MVP da interface do editor.

## Desenvolvimento

* Instale as dependencias do frontend:

```bash
npm install
```

Isso também instala o Vinext e o CLI do Tauri usados pelo script de desenvolvimento.

* Inicie o host desktop com Tauri:

```bash
npm run tauri:dev
```

* Rode o frontend Vinext isolado, se quiser depurar a UI sem Tauri:

```bash
npm run dev
```

## Build

```bash
npm run tauri:build
```

## Estrutura

* `src/` - UI React do editor.
* `src/services/tauri-service.ts` - camada de integracao com Tauri.
* `src-tauri/` - backend Rust do host desktop.
