# Cache de Estado do Editor

## Objetivo

Persistir estado de execucao do editor para restaurar sessao e registrar ultimas alteracoes.

## Arquivo de cache

- Windows: %APPDATA%\LightningEngine\editor_cache.ini
- Migração legada: o editor ainda pode ler caches antigos do projeto ou do diretório atual na primeira carga, mas grava sempre no AppData

## Dados persistidos

### Window

- Width
- Height
- Fullscreen
- Maximized

### Viewport

- CamX
- CamY
- Zoom
- ShowGrid
- SnapToGrid

### ContentBrowser

- CurrentDir
- SelectedPath
- TypeFilter
- ViewMode

### Containers

- BottomTrayActive
- Geometria registrada de Hierarchy, Inspector e Viewport (X, Y, W, H)

### Changes

- LastEngineChange

## Como funciona

- Carrega no Initialize() do EditorApp.
- Salva imediatamente a cada ajuste relevante do editor.
- Salva no Shutdown().

## Eventos que atualizam LastEngineChange

- Add/Delete/Duplicate node
- New Scene / Save Scene
- Import/Create asset
- Add plugin / Plugin wizard
- Troca da aba ativa do container inferior
