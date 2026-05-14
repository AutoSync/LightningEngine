# Godot vs Lightning - Rodar Jogo Dentro do Editor

Contexto: comparacao tecnica de run-in-editor, cobrindo play/stop, embedding, depuracao e convivencia com o workspace do editor.

## Objetivo

Definir como evoluir o Lightning para um fluxo robusto de executar jogo dentro do editor com bom isolamento operacional.

## Como o Godot resolve

### Camada de controle de execucao

- API publica no editor para play/stop e selecao de cena.
- Runtime de execucao encapsulado em EditorRun.

Evidencias:
- editor/editor_interface.h (play_main_scene/play_current_scene/stop_playing_scene)
- editor/run/editor_run.cpp

### Game Workspace embutido

- Plugin dedicado para game view no editor.
- Suporte a janela embutida/flutuante, controle de tempo, estado, audio debug e camera.

Evidencias:
- editor/run/game_view_plugin.h
- editor/run/game_view_plugin.cpp

### Integracao com debugger

- GameViewDebugger conecta sessao e troca mensagens para setup de game view.
- Integracao direta com plugin de debugger do editor.

Evidencias:
- editor/run/game_view_plugin.cpp (_session_started, setup, comandos)
- editor/run/game_view_plugin.h (GameViewDebugger)

### Extensibilidade por plugin

- EditorPlugin pode alterar args de execucao e integrar UI no run bar.

Evidencias:
- editor/plugins/editor_plugin.h (run_scene)
- modules/mono/editor/editor_internal_calls.cpp (EditorRunPlay/EditorRunStop/AddControlToEditorRunBar)

## Estado atual no Lightning

- Fluxo Play/Pause basico presente no editor.
- Preview de jogo existente em nivel MVP.
- Ainda sem arquitetura consolidada para:
  - embedding robusto,
  - sessao de debug integrada,
  - run bar extensivel por plugins,
  - politicas de isolamento de processo/estado.

Evidencias internas:
- Docs/Implementation-Progress.md
- Plan/Roadmaps/Roadmap.md
- Docs/developers/07-rust-tauri-bridge.md

## Gap tecnico

1. Ciclo de run sem camadas explicitas (controller, session, transport, UI).
2. Falta contrato de debug session para sincronizar estado de game preview.
3. Sem padrao para embutido vs janela dedicada com fallback automatico.
4. Run bar e comandos de execucao ainda pouco extensivos para plugins.

## Proposta para Lightning

### Arquitetura alvo

- EditorRunController:
  - play main/current/custom,
  - stop,
  - estado (idle, starting, running, stopping, failed).
- GameWorkspacePlugin:
  - container de execucao embutido,
  - fallback para janela externa,
  - controles de tempo, mute, camera debug.
- RunDebugSession:
  - canal de eventos run/debug,
  - mensagens de setup, status, break, screenshot.
- RunBarExtensibility:
  - API para plugins adicionarem acoes e indicadores.

### Politicas de isolamento

- Separar claramente estado de editor e estado de jogo.
- Garantir reset deterministico da sessao ao parar.
- Definir ownership de recursos compartilhados (input/audio/render targets).

## Plano de adocao

### Fase 0 - Spike de sessao

Entregaveis:
- State machine de run.
- Canal minimo de eventos run/debug.

Criterio de sucesso:
- Play/Stop repetidos sem lock ou vazamento de estado.

### Fase 1 - MVP de game workspace

Entregaveis:
- Embedding estavel no editor.
- Fallback para janela externa.
- Controles basicos no run bar.

Criterio de sucesso:
- Fluxo diario de iteracao com play/stop confiavel em projeto exemplo.

### Fase 2 - Debug integrado e extensibilidade

Entregaveis:
- Integracao de breakpoints/suspend/step com painel de jogo.
- API para plugins participarem do run pipeline.
- Diagnostico de falha de inicializacao e relatorio de sessao.

Criterio de sucesso:
- Sessao de debug e execucao coexistem sem degradar UX do editor.

## Backlog priorizado

### P0

- EditorRunController com state machine.
- GameWorkspacePlugin com embedding + fallback.
- Canal basico RunDebugSession.

### P1

- Run bar extensivel por plugins.
- Comandos de camera/time scale/mute.
- Telemetria local de falhas de run.

### P2

- Multipla instancia para teste rapido.
- Snapshot/replay de estado de sessao.

## Riscos e mitigacoes

- Risco: deadlock entre loop do editor e loop do jogo.
  - Mitigacao: state machine explicita e limites claros de ownership.
- Risco: input/audio presos apos stop.
  - Mitigacao: reset transacional de sessao e testes de repeticao.
- Risco: embedding instavel em plataformas especificas.
  - Mitigacao: fallback automatico para janela externa por capability.

## Criterios de aceite

- [ ] Play/Stop confiavel em ciclos longos de desenvolvimento.
- [ ] Embedding e fallback funcionando por capability.
- [ ] Sessao de debug integrada ao game workspace.
- [ ] API de extensao do run bar documentada e testada.

## Referencias

- editor/editor_interface.h (Godot)
- editor/run/editor_run.cpp (Godot)
- editor/run/game_view_plugin.h (Godot)
- editor/run/game_view_plugin.cpp (Godot)
- editor/plugins/editor_plugin.h (Godot)
- modules/mono/editor/editor_internal_calls.cpp (Godot)
- Docs/Implementation-Progress.md (Lightning)
- Plan/Roadmaps/Roadmap.md (Lightning)
- Docs/developers/07-rust-tauri-bridge.md (Lightning)
