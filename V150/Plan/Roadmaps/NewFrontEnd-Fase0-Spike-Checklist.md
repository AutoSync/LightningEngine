# Fase 0: Spike Tecnico de Embed SDL (Checklist Executavel)

## Objetivo
Validar a viabilidade tecnica de embed SDL em janela unica hosteada pelo app, com foco em estabilidade de ciclo de vida, eventos e input.

## Entradas
1. Roadmap principal atualizado em Plan/Roadmaps/NewFrontEnd.md.
2. Decisao arquitetural em Plan/Roadmaps/NewFrontEnd-ADR-001-EmbedSDL.md.
3. Referencia de loop atual em src/core/Window.cpp.
4. Analise consolidada em Plan/Roadmaps/NewFrontEnd-Fase0-AnaliseWindow.md.

## Escopo do Spike
Incluido:
1. Prototipo minimo de inicializacao de host + ciclo SDL.
2. Validacao de eventos de foco, input e resize.
3. Medicao basica de estabilidade em sessao continua.

Excluido:
1. Implementacao completa da UI React.
2. Integracao completa de paineis de editor.
3. Otimizacoes de performance de longa duracao.

## Tarefas
- [ ] Mapear pontos de acoplamento do loop em src/core/Window.cpp.
- [ ] Definir estrategia de ownership da janela host e ordem de inicializacao.
- [ ] Criar prototipo minimo de inicializacao (host + ciclo SDL).
- [ ] Validar input teclado/mouse com mudanca de foco.
- [ ] Validar resize sem corrupcao de render.
- [ ] Rodar sessao de 30 minutos e registrar erros.
- [ ] Consolidar resultados e decisao Go/No-Go.

## Criterios de Aceite
1. Aplicacao inicializa sem crash em Debug x64.
2. Input continua funcional apos alternancia de foco.
3. Resize mantem render funcional sem regressao critica.
4. Sessao de 30 minutos sem deadlock e sem leak evidente.

## Evidencias Minimas
1. Log da execucao com timestamps.
2. Lista de erros/reproducoes (quando houver).
3. Resultado final Go/No-Go com justificativa tecnica.

## Resultado Esperado
1. Go: prosseguir para Fase 1 mantendo embed como estrategia ativa.
2. No-Go: abrir ADR de fallback para arquitetura de duas janelas.
