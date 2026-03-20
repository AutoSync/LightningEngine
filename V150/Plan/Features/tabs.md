# Tabs — Sistema de Abas do Editor

Baseado no roadmap v0.15.0, com status unificado por contexto.

## Concluído

- `EditorTabSystem.h` — gerenciador completo de abas com kinds: Scene, Script, Texture, Shader, Material, Particle, Prefab, StaticMesh, SkeletalMesh, Config, Generic
- `EditorDocumentContent.h` — conteúdo contextual por aba (outline, leitura de arquivo, detecção binária, modo de sintaxe)
- Distinção de layout: `SceneWorkspace` vs `DocumentWorkspace`
- Indicador de dirty (`*`) e salvamento individual ou em lote (`SaveAllDirty`)
- Acento por tipo de asset (RGB codificado por kind)
- Integração com layout dockável (DockSpace)
- Troca de conteúdo por aba no fluxo do editor

## Pendente

- Expansão de editor visual por tipo de asset
- Integração mais profunda com preview em tempo real e customização de estilo
- Evolução de workflows de edição multi-documento
