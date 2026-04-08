# Plano de Implementacao: Integracao C++ (SDL3) + React + Tauri

## 1. Objetivo
Definir e executar a migracao da camada de interface/editor para React + Tauri, mantendo o motor C++/SDL3 como nucleo da composicao da engine.

Resultado esperado:
- UI do editor rodando em React.
- Motor C++ controlado por comandos Tauri.
- Integracao estavel entre loop SDL3, bridge Rust e IPC com front-end.

## 2. Escopo
Incluido:
- Setup de projeto Tauri + React + TypeScript.
- Bridge C++/Rust para comandos de controle e leitura de estado.
- Eventos de status do motor para a UI.
- Pipeline de build e validacao basica.

Excluido neste ciclo:
- Reescrita completa de todos os modulos da engine.
- Refatoracao profunda da logica de renderizacao do motor.
- Migracao total de plugins/ferramentas nao relacionadas ao editor base.

## 3. Decisoes Arquiteturais
Status atual das decisoes:

1. Decidido: motor C++/SDL3 permanece como responsavel por renderizacao e logica pesada.
2. Decidido: composicao com janela unica integrada (embed SDL no host do app).
3. Em validacao: estrategia final de sincronizacao de eventos e foco entre SDL e Tauri.
4. Pendente: estrategia de transicao da GUI Titan (fallback temporario ou descontinuacao progressiva).

## 4. Arquitetura Alvo
Camadas e responsabilidades:

1. Front-end (React)
- Interface do editor, paineis, overlays e configuracoes.
- Consumo de comandos Tauri via servico unico.
- Renderizacao de estado recebido por eventos.

2. Core Tauri (Rust)
- Exposicao de comandos para front-end.
- Orquestracao de ciclo de vida da bridge.
- Roteamento de eventos C++ para eventos da UI.

3. Motor C++ (SDL3)
- Loop principal da engine.
- Renderizacao e sistemas de runtime.
- API C para bridge (extern "C") com contrato estavel.

Contrato de integracao minimo:
- Comandos: iniciar_motor, parar_motor, get_estado, set_parametro.
- Eventos: motor_status, motor_error.
- Formato de payload: JSON versionado por schema.

## 5. Plano de Implementacao por Fases

### Fase 0 - Alinhamento e Spike Tecnico
Objetivo:
- Validar viabilidade de janela unica integrada (embed SDL).

Entregaveis:
- Documento curto de decisao tecnica do embed.
- Registro de riscos de loop/foco/input.

Artefatos da Fase 0:
- Plan/Roadmaps/NewFrontEnd-ADR-001-EmbedSDL.md
- Plan/Roadmaps/NewFrontEnd-Fase0-AnaliseWindow.md
- Plan/Roadmaps/NewFrontEnd-Fase0-Spike-Checklist.md
- Plan/Roadmaps/NewFrontEnd-MVP.md

Pre-requisitos:
- Revisao do fluxo atual de janela em src/core/Window.cpp.

Riscos:
- Conflito entre ciclo de vida de janela SDL e host Tauri.

Criterio de aceite:
- Prototipo simples abre janela host e acopla ciclo SDL sem crash.

### Fase 1 - Preparacao e Isolamento da API C++
Objetivo:
- Expor uma superficie minima estavel da engine para a bridge.

Entregaveis:
- Header de bridge C com comandos basicos.
- Wrapper inicial para estado e controle do motor.

Pre-requisitos:
- Mapa das funcoes candidatas no runtime.

Riscos:
- Acoplamento alto com classes internas e memoria nao ownership-safe.

Criterio de aceite:
- API C compila em Debug x64 e retorna estado consistente.

### Fase 2 - Scaffold Tauri + React
Objetivo:
- Criar base de app desktop para integrar front-end e bridge.

Entregaveis:
- Estrutura src-tauri e app React inicial.
- Dependencias de Tauri configuradas.

Pre-requisitos:
- Ambiente Node e Rust configurados.

Riscos:
- Divergencia de build entre MSVC e toolchain Rust.

Criterio de aceite:
- App inicial abre e executa invoke de comando mock sem erro.

### Fase 3 - Bridge C++/Rust
Objetivo:
- Conectar comandos reais C++ via Rust com FFI seguro.

Entregaveis:
- cpp_bridge.rs funcional.
- build.rs com linkage de biblioteca C++.
- Conversao segura de strings/erros entre camadas.

Pre-requisitos:
- API C++ estavel da Fase 1.

Riscos:
- Vazamento de memoria em fronteira FFI.

Criterio de aceite:
- iniciar/parar/get_estado funcionam em execucao real.

### Fase 4 - Integracao Front-end
Objetivo:
- Controlar motor e refletir estado na UI React.

Entregaveis:
- tauri-service.ts padronizado.
- Componente de controle de motor com estados de loading/erro.
- Listener de eventos de status.

Pre-requisitos:
- Comandos Tauri reais publicados.

Riscos:
- Saturacao de eventos ou payload excessivo.

Criterio de aceite:
- UI inicia/para motor e exibe status sem inconsistencias.

### Fase 5 - Embed SDL + Estabilidade de Runtime
Objetivo:
- Consolidar composicao de janela unica integrada.

Entregaveis:
- Integracao de handle de janela e ciclo de eventos.
- Tratamento de foco, resize e input.

Pre-requisitos:
- Bridge funcional e UI de controle pronta.

Riscos:
- Deadlocks, foco oscilante e perda de input.

Criterio de aceite:
- Sessao continua de uso (>= 30 min) sem travamento critico.

### Fase 6 - Build e Distribuicao
Objetivo:
- Tornar build reproduzivel para desenvolvimento e CI.

Entregaveis:
- Scripts de build local.
- Workflow CI com build em plataformas alvo.

Pre-requisitos:
- Dependencias SDL3 e toolchains documentadas.

Riscos:
- Falhas de linkage em Windows/MSVC.

Criterio de aceite:
- Build automatizado concluido no CI para pelo menos Windows.

### Fase 7 - Testes e Hardening
Objetivo:
- Validar estabilidade, performance e seguranca da integracao.

Entregaveis:
- Smoke tests de bridge.
- Checklist de performance e memoria.
- Log de erros padronizado entre C++, Rust e front-end.

Pre-requisitos:
- Pipeline de build estavel.

Riscos:
- Regressao silenciosa por sincronizacao de threads.

Criterio de aceite:
- Suite de validacao minima passa sem regressao critica.

## 6. Dependencias e Paralelismo
Dependencias criticas:
1. Fase 1 depende da Fase 0.
2. Fase 3 depende da Fase 1 e Fase 2.
3. Fase 4 depende da Fase 3.
4. Fase 5 depende da Fase 3 e Fase 4.
5. Fase 6 e Fase 7 podem iniciar parcialmente apos Fase 4.

Paralelismo recomendado:
1. UI base React pode avancar em paralelo com contratos de payload da bridge.
2. Estrutura de CI pode ser iniciada enquanto integra embed SDL.

## 7. Cronograma Estimado (Faixas)
Estimativa por fase (dias uteis):
- Fase 0: 1 a 2
- Fase 1: 2 a 3
- Fase 2: 1 a 2
- Fase 3: 2 a 4
- Fase 4: 2 a 3
- Fase 5: 3 a 5
- Fase 6: 1 a 2
- Fase 7: 2 a 3

Total estimado:
- Otimista: 14 dias
- Base: 18 dias
- Conservador: 24 dias

Gatilho de replanejamento:
- Se Fase 0 ou Fase 5 falhar na estabilidade do embed, revisar estrategia de janela imediatamente.

## 8. Riscos e Mitigacoes
1. Conflito de event loop SDL/Tauri.
- Mitigacao: spike inicial, isolamento de responsabilidades de ciclo e testes de longa duracao.

2. Vazamento de memoria na fronteira FFI.
- Mitigacao: ownership explicito, free dedicated API, revisao com sanitizers quando possivel.

3. Falha de build em MSVC.
- Mitigacao: padronizar build.rs/CMake e validar task Build Debug x64 (MSBuild) no inicio.

4. Latencia alta em payloads de estado.
- Mitigacao: reduzir frequencia de eventos, payload incremental e schema versionado.

## 9. Trilha de Validacao Continua
Checklist obrigatorio por marco:
1. Build Debug x64 (MSBuild) sem erro.
2. Smoke test de iniciar/parar/get_estado.
3. Verificacao de estabilidade de eventos por 30 minutos.
4. Medicao de latencia media de chamada FFI.
5. Verificacao de memoria sem crescimento anormal.

## 10. Backlog Priorizado
MVP (P0):
1. API C++ minima de controle do motor.
2. Comandos Tauri reais para iniciar/parar/estado.
3. UI React de controle e status em tempo real.

Hardening (P1):
1. Estabilizacao de embed SDL na janela unica.
2. Padrao unico de logs e erros entre camadas.
3. Testes automatizados de bridge.

Escala (P2):
1. Expansao da UI para paineis de editor (Hierarchy, Inspector, Content).
2. Telemetria basica de performance em dev mode.
3. Pipeline CI multi-plataforma completo.

## 11. Referencias Internas
- Plan/Roadmaps/Editor.md
- Plan/Roadmaps/design.md
- Plan/Features/EngineCore.md
- Plan/Features/Titan.md
- src/core/Window.cpp
- examples/rpg2D/RPG2DApp.h