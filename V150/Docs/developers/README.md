# Lightning Engine — Documentação para Desenvolvedores

> Audiência: desenvolvedores que trabalham **no core** da Lightning Engine
> (commiters do repositório). Para autores de plugins externos, ver
> [`docs/enduser/`](../enduser/README.md).

Esta pasta consolida a arquitetura-alvo da engine após a reestruturação
proposta em [`Plan/Rules/EngineExapand.md`](../../Plan/Rules/EngineExapand.md)
e mapeia o estado atual do repositório para esse alvo.

## Sumário

1. [Arquitetura em 3 camadas](01-architecture.md)
2. [Sistema de plugins](02-plugin-system.md)
3. [Event bus](03-event-bus.md)
4. [Reflexão (TypeRegistry)](04-reflection.md)
5. [Versionamento da Engine API](05-engine-api-versioning.md)
6. [Roadmap de reestruturação](06-restructure-roadmap.md)
7. [Bridge Rust/Tauri e bindings JS](07-rust-tauri-bridge.md)

## Princípios não-negociáveis

- **Core estável e mínimo.** Lifecycle, scheduler, event bus, plugin loader,
  reflection registry, memória. Core **não** depende de plugins.
- **Runtime expansível.** Render, áudio, input, física, UI, scripting são
  módulos pensados como plugins internos (mesma interface dos externos).
- **Plugins externos.** Carregamento dinâmico via ABI C estável + manifest.
- **Comunicação por eventos.** Evite chamadas diretas entre módulos —
  publique no `EventBus`.
- **Reflexão por registro manual.** Sem macros pesadas; serialização e
  bindings consomem o `TypeRegistry`.
- **Versionamento explícito da API.** Plugin valida `ENGINE_API_VERSION`
  antes de registrar qualquer coisa.

## Como contribuir com este documento

- Mantenha cada arquivo focado em **um tema**.
- Cite arquivos reais via links relativos (ex.:
  [`src/include/PluginContracts.h`](../../src/include/PluginContracts.h)).
- Quando o código divergir do documento, **atualize o documento na mesma PR**.
