# Arquitetura de Expansao de Elementos

## Estado atual implementado

- Existe um registro de elementos no header src/include/EditorElementRegistry.h.
- O registro suporta:
  - source (Core, CodeExpansion, PluginGlobal, PluginProject)
  - kind (NodeFactory, ComponentFactory, AssetType, MenuAction, PanelFactory)
  - category e subcategory
  - ordenacao por sortOrder + label
- O menu de contexto da Hierarchy no editor ja consome esse registro em modo fase 1 (flat com prefixos category/subcategory).

## Fluxo de extensao por codigo

1. Criar o comportamento (Node/Component).
2. Registrar elemento no EditorElementRegistry.
3. Expor acao via menu de contexto por category/subcategory.
4. Expor edicao no inspector.

## Exemplo de registro

```cpp
reg.Register({
    "core.hierarchy.add_script_component",
    "Add Script Component",
    "Hierarchy",
    "Components",
    EditorElementKind::ComponentFactory,
    EditorElementSource::Core,
    20,
    true,
    [this]{ /* acao */ }
});
```

## Proximos passos tecnicos

- Adicionar submenus nativos no widget ContextMenu.
- Permitir multiplos providers de registro (core + modulos + plugins).
- Integrar filtros por category/subcategory no Inspector e Content Browser.
