# File -> Plugins -> Adicionar e Criar Wizard

## O que foi implementado

No editor, o menu FILE recebeu duas entradas:

- Plugins -> Adicionar
- Plugins -> Criar Wizard

## Comportamento

### Plugins -> Adicionar

- Exige projeto aberto.
- Abre seletor de pasta para escolher um plugin existente.
- Copia a pasta escolhida para <project>/plugins/ com nome unico.
- Atualiza o Content Browser e registra log no console.

### Plugins -> Criar Wizard

- Abre modal de criacao de plugin.
- Campos:
  - Plugin Name
  - Plugin ID (opcional)
  - Version
  - Category
  - Subcategory
  - Scope (Project/Global)
  - Plugin Type (C++, C#, Ignite)
- Ao criar, gera scaffold em disco com:
  - plugin.yaml
  - README.md
  - src/PluginMain.* (de acordo com o tipo)

## Paths de saida

- Scope Project: <project>/plugins/<plugin-id>
- Scope Global: <engine-cwd>/plugins/global/<plugin-id>

## Arquivos alterados

- src/editor/EditorApp.h

## Observacoes

- O item aparece no menu FILE como caminho textual (fase flat do MenuBar atual).
- Nao depende ainda do PluginManager para load dinamico, apenas scaffold e organizacao inicial.
