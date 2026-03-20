# Editor — Lightning Engine Editor

Baseado no roadmap v0.15.0, com status unificado por contexto.

- [x] Splash / Welcome Screen
- [x] Histórico de projetos recentes
- [x] Browse de pasta nativo (Win32/Linux)
- [x] `ProjectManager` (Create/Open/Save/Close + INIs)
- [x] Save/Load de cena (`.lescene`)
- [x] Hierarquia real de Nodes (TreeView)
- [x] Inspector real de Node (nome/tag/ativo/transform)
- [x] Viewport 2D com Framebuffer
- [x] Content Browser com leitura de disco
- [x] Play/Pause com scripts Nucleo em tempo real
- [x] Menu FILE completo
- [x] Sistema de abas (Project, Random, Textura, Material, Hurricane)
- [x] DockSpace com painéis redimensionáveis
- [x] `NucleoScriptComponent` no inspector + Add Script
- [x] Undo/Redo com `UndoStack`
- [x] Drag & drop de assets para cena
- [x] Gizmos de transformação (move X/Y)
- [x] Play em janela separada (`GamePreviewWindow`)
- [x] Console de output (`Logger` + painel)
- [ ] Melhorias de UX/editor visual em módulos dependentes (GUI scriptable, preview e estilo)

### Assets

Todo o sistema de assets e pipeline de importação ainda está em desenvolvimento, mas o editor já tem suporte básico para leitura de arquivos do disco e drag & drop para a cena. A integração completa com o sistema de assets (incluindo `.leasset`, preview em tempo real, edição visual e gerenciamento) é uma etapa futura importante.

- [ ] Recursos de projeto e asset pipeline ainda abertos (ex.: `.leasset`)
