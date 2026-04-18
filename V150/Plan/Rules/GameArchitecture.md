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
  - LevelScript
  - Player [script:component_engine] (inherits Pawn)
    - CharacterMovement [script:component_engine](inherits CharacterMovementComponent)
      - CharacterMovementComponent(script:component_engine)
    - CapsuleComponent(script:component_engine)
    - SkeletalMeshComponent(script:component_engine)


  - 

## Equivalentes entre Engines

| Lightning Engine              | Unreal Engine                 | Unity                 | Godot                     | Details                           |     
|------------------------------ |-------------------------------|-----------------------|---------------------------|-----------------------------------|
| GameInstance                  | GameInstance                  | GameManager           | Singleton                 | Game-wide data and logic          |
| Scene                         | Level                         | Scene                 | Scene                     | Organizes game elements and logic |
| GameMode                      | GameMode                      | GameManager           | Scene                     | Defines game rules and flow             |
| GameState                     | GameState                     | GameState             | Scene                     | Tracks game state and progress     |
| PlayerState                   | PlayerState                   | PlayerState           | Scene                     | Tracks player-specific data    |
| HUD                           | HUD                           | UI                    | CanvasLayer               | UI game information to the player |
| PlayerController              | PlayerController              | PlayerController      | Scene                     | Handles player input and control logic |
| GameObject                    | Pawn                          | GameObject            | Node                      | Base class for all game objects |
| LandscapeScript               | LandscapeScript               | Terrain               | Node                      | Manages landscape and terrain |
| LevelScript                   | LevelScript                   | SceneManager          | Node                      | Manages level-specific logic |
| Prefab                        | Blueprint                     | Prefab                | PackedScene               | Reusable game object templates |
| Camera                        | Camera                        | Camera                | Camera                    | Represents the player's view |
| Node3D or Node2D              | Actor                         | GameObject            | Node3D/Node2D             | Base class for 3D/2D objects in the scene |
| CharacterMovementComponent    | CharacterMovementComponent    | CharacterController   | CharacterBody             | Handles character movement and physics |
| CapsuleComponent              | CapsuleComponent              | CapsuleCollider       | CollisionShape            | Defines a capsule-shaped collision volume |
| AnimatedMeshComponent         | SkeletalMeshComponent         | SkinnedMeshRenderer   | AnimatedSprite3D          | Handles animated meshes and skeletal animations |
| Hurricane Engine Particles    | Niagara Particles             | ParticleSystem        | Particles2D/Particles3D   | Handles particle effects |
| AudioComponent                | AudioComponent                | AudioSource           | AudioStreamPlayer         | Handles audio playback |
| Equinox ShaderComposer        | Material Editor               | Shader Graph          | Shader Editor             | Tool for creating shaders visually |
| Titan UI                      | UMG                           | UI Toolkit            | Control                   | UI framework for creating user interfaces |
| Vision Direct - Movies Editor | Sequencer                    | Timeline               | AnimationPlayer           | Tool for creating cinematic sequences |

