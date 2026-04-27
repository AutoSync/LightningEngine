# Lightning Engine Editor UI

Frontend **Next.js 15** (Pages Router + static export) hospedado dentro do **Tauri v2**
para o editor da Lightning Engine.

## Stack

- Next.js 15 (Pages Router, `output: 'export'`)
- React 19
- Tauri 2 (host desktop + IPC com o core C++)
- TypeScript 5

## Desenvolvimento

```bash
npm install
npm run tauri:dev   # sobe `next dev` em :3000 e abre a janela Tauri
```

Para depurar somente a UI no navegador (sem Tauri), use `npm run dev` e
acesse `http://localhost:3000`. As chamadas a `@tauri-apps/api` ficam em
fallback mock (ver `src/services/tauri-service.ts`).

## Build

```bash
npm run tauri:build
```

O `next build` produz `out/` (static export) consumido pelo Tauri
(`frontendDist: "../out"` em `src-tauri/tauri.conf.json`).

## Estrutura

- `pages/_app.tsx`, `pages/_document.tsx`, `pages/index.tsx` — entrada Next.js.
- `src/App.tsx` — shell do editor (carregado via `next/dynamic` com `ssr: false`).
- `src/services/tauri-service.ts` — camada IPC (Tauri commands + eventos).
- `src-tauri/` — backend Rust que faz a ponte com o core C++ da engine.

## Integração UI ↔ Core

Veja [`Docs/UI-Engine-Integration.md`](../Docs/UI-Engine-Integration.md)
para o fluxo completo: comandos, eventos e contrato JSON com `EditorBridge`.

