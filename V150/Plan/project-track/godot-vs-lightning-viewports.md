# Godot vs Lightning - Gerenciamento de Viewports

Contexto: comparacao tecnica de gerenciamento de viewport no editor, com foco em input routing, overlays e isolamento entre editor e jogo.

## Objetivo

Definir estrategia de evolucao do Lightning para multiplos viewports de editor e jogo, inspirada nos mecanismos do Godot.

## Como o Godot resolve

### Modelo de viewport

- Classe base robusta: Viewport.
- Suporte a input routing, GUI hit test, drag and drop, filtros e estado local.
- Editor expoe SubViewport 2D/3D via interface publica.

Evidencias:
- scene/main/viewport.h
- scene/main/viewport.cpp
- editor/editor_interface.h (get_editor_viewport_2d/get_editor_viewport_3d)

### Input e separacao editor x runtime

- push_input controla roteamento e bloqueios.
- Guardas especificas evitam input indevido no contexto de cena editada durante editor hint.

Evidencia:
- scene/main/viewport.cpp: regras de push_input e validacoes.

### Overlays e extensao por plugin

- Plugins de editor desenham sobre viewport e interceptam input de forma controlada.
- Padrao forward_canvas_draw_over_viewport e forward_canvas_gui_input.

Evidencias:
- editor/scene/2d/camera_2d_editor_plugin.h
- editor/scene/2d/tiles/tiles_editor_plugin.cpp
- editor/scene/gui/virtual_joystick_editor_plugin.cpp

## Estado atual no Lightning

- Viewport do editor baseado em Framebuffer para renderizacao embutida.
- Ferramentas de editor e gizmos presentes no fluxo atual.
- Falta padronizacao de multiplos viewports com contratos de input/overlay por plugin.

Evidencias internas:
- Plan/Roadmaps/Roadmap.md
- Docs/Implementation-Progress.md
- Docs/developers/01-architecture.md

## Gap tecnico

1. Ausencia de camada unificada para roteamento de input por viewport.
2. Falta protocolo explicito para overlays por plugin com prioridade e composicao.
3. Ausencia de politicas de isolamento editor/runtime por contexto de viewport.
4. Sem contrato formal para viewport 2D, 3D e game preview como tipos distintos.

## Proposta para Lightning

### Arquitetura alvo

- EditorViewportManager:
  - registra viewports (Scene2D, Scene3D, GamePreview, Auxiliares),
  - roteia input por foco e contexto,
  - aplica pipeline de overlays.
- ViewportContext:
  - tipo, camera ativa, modo de manipulacao, politicas de input.
- EditorOverlayPipeline:
  - before_gizmo, gizmo, after_gizmo,
  - ordem deterministica por prioridade.

### Regras de input

- Input lock por modo (play/edit).
- Captura e bubbling definidos por contrato.
- Bloqueio de input cruzado entre viewport de jogo e viewport de edicao.

## Plano de adocao

### Fase 0 - Spike de input routing

Entregaveis:
- Prototipo com 2 viewports (Scene + GamePreview).
- Tabela de regras de foco/captura.

Criterio de sucesso:
- Sem vazamento de input entre viewports em casos basicos.

### Fase 1 - MVP multi-viewport

Entregaveis:
- Manager central de viewports.
- Contrato de overlay com prioridade.
- Integracao de gizmos no pipeline.

Criterio de sucesso:
- Plugin de exemplo desenha overlay sem quebrar selecao/manipulacao.

### Fase 2 - Hardening

Entregaveis:
- Metricas de latencia de input por viewport.
- Modo de debug visual de roteamento.
- Testes de regressao de foco e drag.

Criterio de sucesso:
- Fluxos de edicao 2D/3D e game preview estaveis em sessoes longas.

## Backlog priorizado

### P0

- EditorViewportManager.
- Contrato de input por viewport.
- Contrato de overlays com prioridade.

### P1

- Ferramenta de diagnostico de foco/captura.
- Politicas de bloqueio por modo play/edit.

### P2

- Layout dinamico de viewports auxiliares.
- Perf counters de render/input por viewport.

## Riscos e mitigacoes

- Risco: overlays conflitarem com gizmos.
  - Mitigacao: fases de composicao fixas e prioridades numericas.
- Risco: regressao de input em edge cases.
  - Mitigacao: suite de cenarios de foco/drag/shortcut.
- Risco: custo de manutencao alto.
  - Mitigacao: contratos pequenos e coverage de integracao.

## Criterios de aceite

- [ ] Dois viewports ativos com input isolado e previsivel.
- [ ] Plugin de overlay interoperando com gizmos.
- [ ] GamePreview sem interferir na viewport de edicao.
- [ ] Documentacao de contrato de input e overlay publicada.

## Referencias

- scene/main/viewport.h (Godot)
- scene/main/viewport.cpp (Godot)
- editor/editor_interface.h (Godot)
- editor/scene/2d/camera_2d_editor_plugin.h (Godot)
- editor/scene/2d/tiles/tiles_editor_plugin.cpp (Godot)
- Docs/Implementation-Progress.md (Lightning)
- Plan/Roadmaps/Roadmap.md (Lightning)
