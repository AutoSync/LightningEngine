# Lightning Engine Editor UI

Scaffold React + Tauri para o MVP da interface do editor.

## Desenvolvimento

* Instale as dependencias do frontend:

```bash
npm install
```

* Inicie o host desktop com Tauri:

```bash
npm run tauri:dev
```

## Build

```bash
npm run tauri:build
```

## Estrutura

* `src/` - UI React do editor.
* `src/services/tauri-service.ts` - camada de integracao com Tauri.
* `src-tauri/` - backend Rust do host desktop.
