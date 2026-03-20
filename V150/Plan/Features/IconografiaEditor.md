# Iconografia do Editor — Vínculo de Assets

Mapeamento oficial de `assets/icons` e `assets/thumbnails` para seus propósitos no editor.

## Regras Gerais

- Ícones (`assets/icons`) são usados em UI (abas, toolbar, menu, containers, ações e contexto de asset).
- Thumbnails (`assets/thumbnails`) são usados para preview visual de asset/tipo no Content Browser e painéis de documento.
- Prefixo `cb_` é **exclusivo do Content Browser**.
- Se um contexto não tiver ícone dedicado, usar `default.png` como fallback.

## Shell do Editor (Topbar / Abas / Menus / Toolbar)

- `scene` -> cena ativa / aba de cena
- `script` -> scripts (`.cs`, `.spark`)
- `texture` -> texturas
- `shader` -> shader source / shader tab
- `material` -> materiais
- `particle` -> partículas
- `prefab` -> prefabs
- `mesh`, `skeletal_mesh` -> malhas
- `config`, `controller`, `camera`, `camera_speed` -> itens de configuração e câmera
- `save`, `save_all`, `undo`, `redo`, `search`, `close`, `maximize` -> ações gerais de UI

## Viewport / Gizmos / Manipulação

- `translate`, `rotate`, `scale` -> gizmos de transformação
- `snap` -> modo snap
- `drag` -> arrastar/manipular
- `sprite` -> contexto 2D/sprite

## Módulos e Ferramentas

- `equinox`, `hurricane`, `random_ignite`, `gamesystem`, `hud`, `lightning`, `audio`, `globe`
- Uso: abertura de módulo, ações de menu e painéis dedicados.

## Graph / Material Editor

- `material_functions`, `material_instance`, `material_layer`
- `port_input`, `port_output`
- Uso: nós, propriedades e conexões de graph.

## Content Browser (exclusivo `cb_*`)

- `cb_add` -> criar asset/ação rápida
- `cb_undo`, `cb_redo` -> histórico local do browser
- `cb_saveall` -> salvar alterações pendentes no browser
- `cb_folder` -> pasta no browser
- `cb_openfolder` -> abrir pasta no SO

## Thumbnails (`assets/thumbnails`)

- `default.png` -> fallback universal para asset sem thumbnail dedicado
- `csharp.png` -> scripts C#
- `ignite.png` -> assets/graphs relacionados ao Random Ignite

## Fallbacks recomendados

- Script sem correspondência -> `icons/script` + `thumbnails/csharp.png` (quando aplicável)
- Asset desconhecido -> `icons/prefab` ou `icons/folder` + `thumbnails/default.png`
- Módulo sem ícone específico -> `icons/lightning`
