# Desenvolvimento do Jogo

## Objetivo

Consolidar a frente de desenvolvimento do jogo sobre a base da Lightning Engine, com foco em entrega incremental, estabilidade e previsibilidade.

## Visão geral

A base técnica atual favorece um ciclo de jogo com:

- `Window`
- `GameInstance`
- `Level`
- `Node`
- `Component`

Isso permite separar runtime, cena, entidades e comportamento.

## Frentes principais

### 1. Núcleo jogável

- movimento do jogador
- câmera
- entrada e resposta imediata aos controles
- colisão e navegação

### 2. Combate

- armas / ataque
- dano e feedback visual
- vida, morte e respawn
- balanceamento inicial

### 3. IA

- inimigos básicos
- percepção
- perseguição, patrulha e ataque
- estados simples para protótipo

### 4. Interface

- HUD do jogador
- feedback de status
- menus essenciais
- telas de pausa e fim de jogo

### 5. Progressão

- objetivos de fase
- checkpoints
- persistência de progresso
- desbloqueios, se aplicável

## Pipeline de entrega

### Fase A — Protótipo

- validar loop principal
- validar câmera e controle
- validar uma arena/janela de teste
- validar feedback de combate

### Fase B — Vertical slice

- uma fase completa pequena
- inimigos, HUD e objetivos mínimos
- entrada, combate e saída de fase

### Fase C — Expansão

- mais conteúdo
- mais inimigos
- mais variação de armas/ações
- otimização e polimento

## Dependências técnicas

- renderização estável
- entrada consistente
- carregamento de assets
- organização clara de cenas e componentes
- suporte a UI sem mutação insegura da árvore

## Critérios de qualidade

- o jogo deve iniciar e encerrar sem corrupção de estado
- mudanças de cena não podem deixar referências pendentes
- callbacks não devem destruir estruturas usadas no mesmo frame
- qualquer sistema novo deve respeitar a divisão entre runtime e conteúdo

## Próximo passo recomendado

Documentar por cena ou por modo de jogo:

- o que é carregado
- quem é dono de cada recurso
- quais eventos podem alterar a árvore de cena
- quais subsistemas podem ser usados por cada fase
