# Arquitetura de Plugins

## Estado atual implementado

- Existe contrato base de plugin em src/include/PluginContracts.h.
- O contrato inclui:
  - PluginManifest
  - PluginScope (Global/Project)
  - PluginPermission (filesystem, network, device, script, editor-ui)
  - IEnginePlugin com ciclo de vida minimo

## Ciclo de vida alvo

1. Discover
2. Validate manifest
3. Resolve dependencies
4. Permission gate
5. Load
6. Register
7. Activate
8. Deactivate/Unload

## Contrato do manifesto

Campos principais:

- id, name, version
- engineVersionMin, engineVersionMax
- scope
- category, subcategory
- dependencies
- permissions
- entrypoints (native, csharp, ignite)

## Proximos passos tecnicos

- Criar PluginManager para discovery/load/unload.
- Criar leitura de configuracao YAML de plugins ativos.
- Integrar painel de plugins no editor.
- Adicionar empacotamento leplugin.
