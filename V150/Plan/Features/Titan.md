# Especificação do Titan — Sistema de GUI da Lightning Engine

## Visão Geral

- Titan ataca duas frentes principais: GUI pra C++ (runtime) pra construir interfaces para a Lightning Engine, e um editor visual de UI (Editor de UI) pra criar e editar essas interfaces de forma visual.
- O foco inicial é a parte de runtime, com o editor visual planejado para uma fase posterior. O sistema de runtime deve ser flexível o suficiente para suportar a criação de interfaces complexas, enquanto o editor visual deve facilitar a construção dessas interfaces sem necessidade de código.

## Etapas para Expandir o Titan

- [ ] Draw2D, Line, Rect, Circle
- [x] Text
- [x] Imagens (Sprite)
- [x] Containers (StackPanel, Grid)
- [x] Interação básica (cliques, hover)

## Documentar a Criação de GUI em C++

- Criar um Guia de uso do Titan para c++ em `Plan/Docs/TitanGuide.md` com exemplos de código e melhores práticas para criar interfaces usando o Titan.