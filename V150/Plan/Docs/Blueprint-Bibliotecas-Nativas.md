# Blueprint Tecnico - Bibliotecas Nativas e Exposicao para Lightning Engine

Objetivo: definir como bibliotecas nativas C++ entram no runtime/editor da engine, como sao expostas para C# e Ignite, e como se conectam ao fluxo Node -> Component -> Prefab.

## 1. Principios de Arquitetura

- Core de simulacao e desempenho em C++.
- API de uso para criadores em C# e Ignite como camada de fachada.
- Prefab e Node como ponto de composicao de gameplay.
- PluginManager como orquestrador de ciclo de vida dos modulos.
- EditorElementRegistry como ponto de exposicao no editor (menu, painel, contexto).

Fluxo principal da engine:
Window -> GameInstance -> Level -> Node -> Component.

## 2. Onde as bibliotecas devem ficar

### 2.1 Expansao por codigo (compila junto com a engine)

Para funcionalidades base (ex.: CharacterMovement), preferir modulo interno.

- Headers publicos:
  - src/include/modules/character/CharacterMovementComponent.h
  - src/include/modules/character/CharacterMovementConfig.h
  - src/include/script/api/CharacterMovementApi.h
- Implementacao:
  - src/core/modules/character/CharacterMovementComponent.cpp
  - src/core/script/bindings/CharacterMovementBindings.cpp
- Registro de editor/contexto:
  - src/core/editor/registries/CharacterEditorRegistration.cpp

Uso indicado:

- Sistemas de fisica, colisao, locomocao e logica com alto custo.
- Funcionalidades que precisam estar em todos os projetos.

### 2.2 Expansao por plugin nativo (DLL)

Para recursos opcionais e distribuidos por pacote.

- plugins/global/[plugin-id]/
  - manifest.yaml
  - bin/win64/[plugin-id].dll
  - csharp/[plugin-id].dll (API opcional)
  - ignite/[plugin-id].sparklib (opcional)
- plugins/project/[plugin-id]/
  - mesma estrutura, escopo por projeto

Manifest deve mapear:

- id, versao, permissoes, dependencias
- entryNative (DLL), entryCSharp (assembly), entryIgnite

## 3. Contratos e Pontos de Integracao

Contratos existentes usados como base:

- src/include/PluginContracts.h
- src/include/PluginManager.h
- src/include/EditorElementRegistry.h

Regra:

- Toda biblioteca nativa exposta para scripts deve registrar API em um ScriptApiRegistry (novo modulo) durante OnRegister.

## 4. Modelo de Exposicao para C# e Ignite

### 4.1 Camadas

- Camada A: Native Runtime (C++)
  - implementa regra real de movimento/colisao/gravidade.
- Camada B: Binding Layer (C ABI + marshaling)
  - funcoes estaveis exportadas para VM/host C#.
- Camada C: Managed Facade (C#)
  - classes amigaveis para usuario final.

### 4.2 Exemplo de API alvo em C#

Classe C# para usuario final:

- CharacterMovement
  - float MaxSpeed
  - float JumpForce
  - float GravityScale
  - bool IsGrounded()
  - void Move(Vector2 input)
  - void Jump()

Essa classe nao contem fisica completa: ela chama bindings nativos.

## 5. Fluxo Completo (Build -> Editor -> Runtime)

### 5.1 Build

1. Compilar engine e modulos internos C++.
2. Compilar plugins nativos opcionais (DLL).
3. Compilar assembly C# de API (Lightning.Gameplay.dll).
4. Publicar artefatos no output do projeto.

### 5.2 Empacotamento para binario final

Copiar para output final:

- executavel da engine/jogo
- DLLs de runtime third_party
- DLLs de plugin nativo habilitadas
- assemblies C# de API e scripts do projeto
- assets e prefabs

### 5.3 Inicializacao em runtime/editor

1. ProjectManager resolve lista de plugins globais e de projeto.
2. PluginManager faz Discover -> Load -> Register -> Activate.
3. Cada plugin/modulo registra:
   - componentes e nodes suportados
   - API de script (C# / Ignite)
   - entradas de editor (menu, contexto, tabs)
4. Prefab loader instancia Node e Component.
5. Script side anexa ao componente nativo via handle/id de entidade.

## 6. Expansao da base da engine (itens estruturais)

### 6.1 Novos registries recomendados

- RuntimeComponentRegistry
  - cria componentes por id serializado.
- ScriptApiRegistry
  - registra funcoes nativas expostas para C#/Ignite.
- PrefabNodeFactoryRegistry
  - cria nodes padrao por tipo (CharacterNode, CameraNode etc.).

### 6.2 Persistencia

Prefab deve serializar componente com schema estavel:

- nodeType: CharacterNode
- components:
  - CharacterMovementComponent:
    - maxSpeed: 6.5
    - jumpForce: 12.0
    - gravityScale: 1.0
    - airControl: 0.35

## 7. Blueprint especifico: CharacterMovement

Nomenclatura canonica para este fluxo:

- Node: CharacterNode
- Componente de movimento nativo: CharacterMovementComponent
- Componente de script: ScriptComponent
- Colisao no fluxo 2D: Collider2D
- API C# de fachada: CharacterMovement (managed facade)

### 7.1 Runtime C++

CharacterMovementComponent:

- dependencias: Transform, Collider2D, opcional Rigidbody/CharacterController
- ciclo:
  - Update(dt):
    - ler input acumulado
    - aplicar aceleracao
    - aplicar gravidade
    - resolver colisao
    - atualizar estado grounded

### 7.2 Integracao com Node

CharacterNode (factory):

- cria Node com:
  - Transform
  - Collider2D
  - CharacterMovementComponent
  - ScriptComponent (opcional)

### 7.3 Exposicao para script

Bindings minimos:

- API C# de fachada: Move(Vector2 input), Jump(), IsGrounded()
- Move(Vector2 input) deve mapear para SetMoveInput(entityId, x, y)

- SetMoveInput(entityId, x, y)
- Jump(entityId)
- IsGrounded(entityId)
- Get/Set params de locomocao

### 7.4 Editor

- Menu de contexto: Add Node/Character/CharacterNode
- Inspector: secao CharacterMovement com sliders e validacao
- Documentacao contextual na aba de codigo para API CharacterMovement

## 8. Seguranca e Compatibilidade

- Permissoes por plugin (FileSystem, Network, Device, ScriptApi, EditorUI).
- Versionamento semantico no manifest.
- Gate de compatibilidade engineVersionMin/Max.
- Falha de plugin nao derruba a engine: estado Failed com isolamento.

## 9. Roadmap de implementacao (fases)

Fase 1 - Fundacao

- RuntimeComponentRegistry
- ScriptApiRegistry
- Loading de plugin por manifest

Fase 2 - CharacterMovement MVP

- Componente nativo funcional
- Wrapper C# com API minima
- Prefab CharacterNode padrao

Fase 3 - Editor e DX

- Inspector completo
- Acoes no menu de contexto
- Templates e snippets de codigo

Fase 4 - Empacotamento

- Pipeline de copy de DLL/assemblies
- Validacao automatica de dependencias em build

## 10. Criterios de aceite

- Usuario cria CharacterNode no editor sem escrever C++.
- Prefab serializa e restaura CharacterMovement corretamente.
- Script C# consegue mover, pular e consultar grounded.
- Build final inclui binarios nativos + assemblies necessarios.
- Plugin com erro entra em Failed sem quebrar o editor.
