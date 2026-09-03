# Lightning Engine v0.14.0

Esta pasta contém a implementação da versão `0.14.0` do Lightning Engine, um protótipo de motor 2D/3D para Windows baseado em C++, GLFW, GLAD, OpenGL, GLM, Assimp e `stb_image`. A hierarquia de gameplay está disponível em `GameNode.h` (`GameObject` é o alias compatível) e a primeira camada de UI em `UI.h`.

## Documentação

- [Visão técnica da v0.14.0](docs/v0.14.0.md)

## Estado da versão

A solução já possui o núcleo do motor, a criação de contexto OpenGL, o laço principal, entrada, tempo, shaders, texturas, importação de malhas e componentes de cena. O ponto de entrada atual (`main.cpp`) ainda é um stub e não instancia uma aplicação; portanto, o executável gerado a partir deste snapshot termina imediatamente.

## Build

Abra `Lightning Engine.sln` no Visual Studio 2022, selecione `Debug | x64` e compile o projeto `Lightning Engine 0.14.0`. As dependências usadas nessa configuração estão em `third_party/`.
