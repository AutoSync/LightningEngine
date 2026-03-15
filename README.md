# Lightning Engine Migration 0.14.0 to -> V0.15.0 [WIP]

- [Lightning Engine V0.15.0 [WIP]](#lightning-engine-v0150-wip)
- [Presentation](#presentation)
  - [Engine Progress](#engine-progress)
  - [API](#api)
  - [Features](#features)
  - [Nucleo](#nucleo)
  - [Motor de Fisica](#motor-de-fisica)
  - [Motor de Partículas](#motor-de-partículas)

# Presentation

# MIGRATION 0.14.0 -> 0.15.0

<p>Lightning engine is a game engine that is currently under development. </p>

## Proposta do projeto

Ser uma engine rapida de acesso direto aos sistemas de compilação, renderização e execução de jogos, com foco em ser leve, rápida e fácil de usar, com uma API simples e intuitiva, e com uma comunidade ativa e colaborativa.

- Interface simples e intuitiva
- Integração com motor de renderização que pode ser inbutido ou externo
- MultiRender DX11, DX12, Vulkan, OpenGL
- Suporte a multiplataforma (Windows, Linux, MacOS) mediante a recompilação do motor
- Linguagem de script leve e fácil de usar, com suporte a C#
- Interface GUI para edição de jogos, com suporte a arrastar e soltar, interface construída com a própria engine, e suporte a plugins de terceiros
- Expansao por ferramentas externas do proprio ambiente de desenvolvimento, como editores de texto, modeladores 3D, etc
- Tanto a proposta como o desenvolvimento devem ser movidos para a pasta de desenvolvimento atual `v150/Plan/Roadmaps` para melhor organização e controle de versões, e para evitar confusões com a versão atual `v150/Plan/Roadmap` que é a versão 0.14.0, e para manter um histórico claro do desenvolvimento do projeto.

## Engine Progress

- [x] Window Handler
  - [x] SDL BASE
  - [x] Class Input
  - [ ] Game Instance
  - [ ] Renderer
  - [x] Engine Settings
    - [ ] Framebuffer
    - [ ] ClearColor
- [x] Class GameEditor
- [ ] Shader Compiler
  - [x] Load Shader
  - [x] Shader Injection
    - [ ] Vectors
    - [ ] Lights
    - [ ] Fog
    - [ ] Time
    - [ ] UV Coords
    - [ ] World Coords
  - [ ] Shader Engine
  - [ ] Shader Processor
  - [ ] Shader Functions
  - [ ] Shader Instance
- [ ] Model Object
  - [ ] Loader
- [ ] GUI
  - [ ] Widgets
  - [ ] Events
  - [ ] Process
  - [ ] Responsive
  - [ ] Scriptable

## API

- [ ] Math
- [ ] Physics
- [ ] Particles
- [ ] GameObject/Prefabs
- [ ] Inputs

## Features

- [ ] Render 2D
- [ ] Render 3D
- [ ] PostFX

## Nucleo

- [ ] Binary
  - [ ] Read Binary
  - [ ] Write Binary
  - [ ] Write Bitecode
- [ ] Run Game
  - [ ] Run Game By Nucleo VM
- [ ] Script Engine
  - [ ] Link Game With Code
  - [ ] Realtime gameplay in editor

## Motor de Fisica

O MotionPhysics é um motor de física 2D e 3D desenvolvido para ser leve, rápido e fácil de usar, com uma API simples e intuitiva, e com uma comunidade ativa e colaborativa. Ele é projetado para ser integrado com o Lightning Engine, mas também pode ser usado como um motor de física independente.

## Motor de Partículas

O Hurricane é um motor de partículas desenvolvido para ser leve, rápido e fácil de usar, com uma API simples e intuitiva, e com uma comunidade ativa e colaborativa. Ele é projetado para ser integrado com o Lightning Engine, mas também pode ser usado como um motor de partículas independente.

