# Fluxo de script de um Character

## 1. Conceito de Prefab na Lightning Engine

Prefab e uma cena reutilizavel e serializada, composta por Node, Component e Script,
comportando-se de forma equivalente ao que o mercado conhece como:

- Prefab (Unity)
- Actor Blueprint (Unreal)
- Scene (Godot)

Na pratica, o prefab e um bloco pronto de gameplay que pode ser instanciado em Level,
duplicado, versionado e evoluido como parte da biblioteca de recursos da engine.

## 2. Estrutura do prefab Character

Exemplo de composicao esperada:

````text
[CharacterNode]
    -> [Transform]
    -> [Collider2D]
    -> [CharacterMovementComponent]
    -> [ScriptComponent -> CharacterMovement.cs]
    -> [Sprite2D]
    -> [Camera2D]
````

Regras de composicao:

- CharacterMovementComponent e nativo (C++) e contem fisica, colisao e locomocao.
- ScriptComponent referencia CharacterMovement.cs para regra de gameplay.
- Sprite2D e Camera2D sao componentes de apresentacao e suporte visual.
- O prefab deve ser instanciavel em qualquer Level sem exigir codigo C++ no projeto de jogo.

## 3. Fluxo de script do CharacterMovement

1. O usuario cria um CharacterNode pelo editor.
2. O prefab instancia os componentes base (Transform, Collider2D, CharacterMovementComponent, ScriptComponent).
3. CharacterMovement.cs envia input para o componente nativo.
4. O componente nativo processa Move, Jump, gravidade e estado grounded.
5. O script consulta IsGrounded() e aciona logica de animacao/acao.

APIs esperadas para script:

- Move(Vector2 input)
- Jump()
- IsGrounded()
- Parametros de locomocao (MaxSpeed, JumpForce, GravityScale, AirControl)

Mapeamento de facade para binding nativo:

- Move(Vector2 input) -> SetMoveInput(entityId, x, y)

## 4. Projecao de prefab e serializacao

Para atender a projecao de prefab da engine, o asset precisa salvar estrutura e estado.

Exemplo de schema minimo:

````yaml
nodeType: CharacterNode
components:
  CharacterMovementComponent:
    maxSpeed: 6.5
    jumpForce: 12.0
    gravityScale: 1.0
    airControl: 0.35
  ScriptComponent:
    script: CharacterMovement.cs
  Collider2D:
    shape: capsule
  Sprite2D:
    asset: characters/hero_idle.png
````

Esse formato permite:

- restaurar o Character completo em runtime/editor;
- reutilizar o mesmo asset em multiplos Levels;
- evoluir a biblioteca de recursos sem refazer estrutura manual.

## 5. Papel do prefab na estrutura do projeto

Prefab e um ativo central para organizacao de projetos de jogos porque:

- separa composicao de gameplay da logica de engine;
- reduz retrabalho com templates reutilizaveis;
- padroniza montagem de personagens, inimigos, props e cameras;
- acelera iteracao no editor;
- fortalece a biblioteca de recursos da engine (assets + comportamento + configuracao).

## 6. Criterios de aceite para Character prefab

- Usuario cria CharacterNode no editor sem escrever C++.
- Prefab serializa e restaura CharacterMovement corretamente.
- Script CharacterMovement.cs move, pula e consulta grounded.
- Instanciacao funciona em diferentes Levels sem quebrar referencias.
- Prefab pode ser versionado e reutilizado como recurso padrao da engine.