# Arquitetura do Jogo

- GameInstance: Carrega elementos que devem permanecer durante toda a execução do jogo, como dados de configuração, progresso do jogador, etc.
- GameMode: Carrega conjunto de scripts e regras especificar

# Hierarquia

- GameInstance
  - Scene
    - GameMode
      - GameState
      - PlayerState
      - HUD
      - PlayerController
      - Pawn
    - LandscapeScript
    - 

