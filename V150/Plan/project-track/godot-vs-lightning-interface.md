# Godot vs Lightning - Interface do Editor

Contexto: comparacao tecnica de como o Godot resolve interface de editor e como adotar padroes equivalentes no Lightning Engine.

## Objetivo

Documentar arquitetura, fluxo e decisoes tecnicas para evoluir a interface do editor do Lightning com base em praticas maduras do Godot.

## Como o Godot resolve

### Estrutura principal de UI

- Orquestrador central: EditorNode.
- Composicao por areas de dock, menus e tela principal.
- Separacao entre API publica do editor e implementacao interna.

Evidencias:
- editor/editor_node.h: composicao de menus, split containers, plugins ativos.
- editor/editor_interface.h: API publica para controle de telas, viewport 2D/3D e ciclo de play.

### Sistema de plugins de editor

- Base comum: EditorPlugin.
- Plugins podem:
  - injetar UI,
  - participar de build/run,
  - persistir layout,
  - conectar overlays e input em viewport.

Evidencias:
- editor/plugins/editor_plugin.h
- editor/plugins/editor_plugin.cpp

### Persistencia de layout

- Plugins implementam set_window_layout/get_window_layout.
- Salvamento desacoplado via queue_save_layout.

Resultado:
- estado de UI consistente entre sessoes,
- baixo acoplamento entre painel e shell principal.

## Estado atual no Lightning

- Editor funcional com DockSpace, abas e paineis.
- Elementos principais implementados (Hierarchy, Inspector, Viewport, Content Browser, Console).
- Registro de elementos e plugin manager em evolucao.

Evidencias internas:
- Docs/Implementation-Progress.md
- Plan/Roadmaps/Roadmap.md
- Docs/developers/01-architecture.md

## Gap tecnico

1. Contrato de plugin de UI ainda parcial para ciclo completo de layout e overlays.
2. Persistencia de estado de plugins e layout ainda sem consolidacao em formato estavel.
3. Faltam pontos de extensao padronizados para barra principal, run bar e paineis contextuais.
4. Falta um protocolo unico para acoplamento entre comandos globais e componentes de UI.

## Proposta para Lightning

### Principios

- Shell de editor minimo e estavel.
- Plugins donos de funcionalidade vertical.
- Layout e estado serializaveis desde o inicio.

### Contratos alvo (incremental)

- EditorUIPlugin:
  - OnLoadUI
  - OnRegisterPanels
  - OnRegisterMenus
  - OnSaveLayout
  - OnRestoreLayout
  - OnUnloadUI
- EditorCommandRegistry:
  - Registrar comandos com id, atalho, escopo.
- EditorLayoutStore:
  - persistencia de dock, tabs, visibilidade e preferencia por projeto.

## Plano de adocao

### Fase 0 - Spike de arquitetura

Entregaveis:
- Definir contratos de extensao de interface.
- Mapear paineis existentes para novo ciclo de vida.
- Prototipo de persistencia de layout de um painel.

Criterio de sucesso:
- 1 plugin de exemplo restaura layout apos reinicio.

### Fase 1 - MVP de shell extensivel

Entregaveis:
- Migrar paineis principais para contratos unificados.
- Centralizar comando/menu/atalhos em registry unico.
- Persistir layout por projeto.

Criterio de sucesso:
- Editor abre com layout identico em pelo menos 3 cenarios de projeto.

### Fase 2 - Hardening e DX

Entregaveis:
- APIs para terceiros adicionarem painel/menu com baixo boilerplate.
- Telemetria local de falha de layout e fallback seguro.
- Validacao de compatibilidade de versao de layout.

Criterio de sucesso:
- Sem perda de layout em upgrades menores do editor.

## Backlog priorizado

### P0

- Contrato unico de plugin de interface.
- Persistencia de layout por projeto.
- Registry central de comandos e atalhos.

### P1

- API para extensao de barra superior/inferior.
- Hooks para overlays e paineis contextuais.
- Fallback automatico em layout invalido.

### P2

- Ferramenta visual para editar layout default.
- Migração automatica entre versoes de layout.

## Riscos e mitigacoes

- Risco: regressao de UX durante migracao.
  - Mitigacao: migracao por feature flags e fallback para layout default.
- Risco: contratos amplos demais gerarem acoplamento.
  - Mitigacao: interfaces pequenas e orientadas a casos reais.
- Risco: conflitos de atalhos entre plugins.
  - Mitigacao: arbitration layer com prioridade e diagnostico.

## Criterios de aceite

- [ ] API de plugin de UI documentada e versionada.
- [ ] Layout de projeto persiste com integridade.
- [ ] Comandos globais e atalhos funcionam sem conflito critico.
- [ ] Painel de exemplo third-party funciona sem alterar core.

## Referencias

- editor/editor_node.h (Godot)
- editor/editor_interface.h (Godot)
- editor/plugins/editor_plugin.h (Godot)
- editor/plugins/editor_plugin.cpp (Godot)
- Docs/Implementation-Progress.md (Lightning)
- Plan/Roadmaps/Roadmap.md (Lightning)
