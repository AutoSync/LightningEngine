# ADR-001: Composicao de Janela Unica com Embed SDL no Host Tauri

## Status
Aprovado para implementacao da Fase 0 (validacao tecnica obrigatoria).

## Contexto
A evolucao do editor para React + Tauri exige manter o motor C++/SDL3 como nucleo da composicao. A decisao principal e como integrar janela/renderizacao mantendo UX unificada e sem comprometer estabilidade do runtime.

Alternativas avaliadas:
1. Janela unica com embed SDL no host Tauri.
2. Duas janelas separadas (UI Tauri + janela SDL dedicada).

## Decisao
Adotar janela unica com embed SDL como estrategia alvo do roadmap NewFrontEnd.

## Justificativas
1. Experiencia de usuario unificada no editor.
2. Menor friccao operacional para fluxo de ferramentas e docking.
3. Alinhamento com objetivo de composicao unica do produto.

## Consequencias Positivas
1. Interface unica para usuario final.
2. Menor sobrecarga cognitiva no uso do editor.
3. Base mais coerente para evolucao de layout e paineis.

## Riscos e Consequencias Negativas
1. Maior complexidade no ciclo de vida da janela e foco de input.
2. Risco de conflito entre loop SDL e orquestracao do host Tauri.
3. Potencial aumento de custo de depuracao em eventos de resize/focus.

## Mitigacoes
1. Executar spike tecnico da Fase 0 com criterios de aceite objetivos.
2. Padronizar logs por camada (C++, Rust, Front-end) para diagnostico cruzado.
3. Definir fallback explicito para janela separada caso criterios minimos falhem.

## Criterios de Validacao da Decisao
1. Inicializacao do host + ciclo SDL sem crash.
2. Input de teclado/mouse funcional apos alternancia de foco.
3. Resize da janela sem regressao visual critica.
4. Execucao continua por 30 minutos sem deadlock/travamento grave.

## Plano de Fallback
Se dois ou mais criterios de validacao falharem de forma reprodutivel, reavaliar arquitetura para modo de duas janelas e registrar ADR complementar.

## Referencias
1. Plan/Roadmaps/NewFrontEnd.md
2. src/core/Window.cpp
3. Plan/Features/EngineCore.md
4. Plan/Features/Titan.md
