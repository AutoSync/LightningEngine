# MVP da UI em React para a Lightning Engine

## Objetivo
Entregar a primeira versão funcional da UI do editor em React, substituindo a superficie visual principal do Titan com foco em navegacao, contexto e controle do motor.

## Definicao de MVP
O MVP deve permitir:
1. Abrir o editor com shell visual base.
2. Exibir menu principal, toolbar e status bar.
3. Navegar entre abas contextuais do editor.
4. Visualizar Content Browser, Hierarchy, Inspector e Viewport.
5. Disparar comandos basicos no motor por Tauri.
6. Refletir estado do motor e eventos de runtime em tempo real.

## Componentes Incluidos
1. Shell do editor
- Layout geral da janela.
- Top bar, menubar, toolbar, body, status bar.

2. Sistema de abas
- Aba Project fixa.
- Abas contextuais por tipo de asset.
- Estado de aba ativa e fechamento de abas derivadas.

3. Dock principal
- Content Browser.
- Hierarchy.
- Inspector.
- Viewport.

4. Comandos basicos
- iniciar_motor.
- parar_motor.
- get_estado.
- set_parametro.

5. Estado e eventos
- Status do motor.
- Erros de runtime.
- Selecionar item e refletir propriedades na UI.

## Nao incluido no MVP
1. Editor visual completo de scripts.
2. Undo/redo de interacoes complexas.
3. Sistema completo de plugins.
4. Widgets avancados do Titan que nao impactam o fluxo principal.
5. Tema customizavel completo.

## Ordem de Implementacao
1. Shell do editor.
2. Menubar, toolbar e status bar.
3. Sistema de abas e roteamento contextual.
4. Content Browser e Hierarchy.
5. Inspector com propriedades basicas.
6. Viewport e controle do motor.
7. Eventos e sincronizacao com a bridge.
8. Hardening visual e navegacao.

## Estrutura Logica da UI
- AppShell
  - TitleBar
  - MenuBar
  - QuickToolbar
  - MainDock
    - ProjectTab
    - SceneTab
    - ContentBrowserPanel
    - HierarchyPanel
    - InspectorPanel
    - ViewportPanel
  - StatusBar

## Regras do MVP
1. A aba Project nao pode ser fechada.
2. Toda aba contextual deve ser derivada do tipo de asset ou contexto ativo.
3. O motor deve poder ser controlado sem recarregar a UI.
4. A UI deve permanecer responsiva durante eventos do runtime.
5. O MVP deve aceitar extensao posterior para widgets do Titan convertidos em React.

## Critérios de Aceite
1. O editor abre e renderiza todos os blocos principais.
2. Pelo menos um fluxo real de iniciar/parar motor funciona.
3. O estado do motor aparece na UI sem bloqueio.
4. O usuario consegue navegar entre contexto de cena e contexto de asset.
5. O layout principal permanece estável ao redimensionar.

## Dependencias
1. Decisao de embed SDL registrada em ADR.
2. Bridge C++/Rust definida para comandos basicos.
3. Especificacao visual do editor em Plan/Roadmaps/design.md.
4. Regras de contexto e abas em Plan/Roadmaps/Editor.md.

## Entregaveis Associados
1. Shell React inicial.
2. Componentes basicos do editor.
3. Serviço Tauri para comandos e eventos.
4. Documento de aceite do MVP.
