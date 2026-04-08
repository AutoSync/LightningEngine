# GUI: `Panel` e Docking

## Objetivo

Documentar a estrutura atual do container `Titan::Panel` e o fluxo de docking usado pela interface do editor.

## `Panel`

`Panel` é um widget container que herda de `Widget` e adiciona:

- `title`
- `showTitle`
- `clipChildren`

### Comportamento

- Renderiza fundo, barra de título e borda.
- Quando `clipChildren` está ativo, aplica `Scissor` antes de desenhar os filhos.
- Encaminha input para os filhos da frente para trás.
- Consome clique quando o painel é atingido.

### Observações de implementação

- A iteração de `children` assume que a árvore não será mutada no meio do processamento.
- Se um filho puder se remover ou reorganizar a árvore durante `ProcessInput()` ou `Render()`, a operação precisa ser adiada ou protegida.

## Docking no editor

O fluxo de docking atual fica concentrado em `DockSpace`.

### Estrutura

- `DockNode::Type::Split` divide a área em dois filhos.
- `DockNode::Type::Leaf` guarda uma lista de abas/painéis.
- `unique_ptr` é usado para expressar propriedade dos nós filhos.

### Fluxo de entrada

1. O `DockSpace` processa drag, splitter e tabs.
2. Um painel pode ser removido com `Undock()`.
3. Se o nó ficar vazio, `CollapseIfEmpty()` reconstrói a árvore.
4. O conteúdo solto pode ser enviado para o `TitanUI` como widget flutuante.

## Risco conhecido

A árvore de docking é sensível a mutações durante input. Qualquer destruição antecipada de widgets ou uso de ponteiro cru após `Undock()` pode invalidar nós e causar `use-after-free`.

## Diretriz

- Evitar chamar destruição imediata antes da reorganização da árvore.
- Preferir collapse e atualização estrutural antes de liberar recursos que possam reentrar no fluxo de UI.
- Manter a árvore em estado consistente entre `Undock()`, `Dock()` e `CollapseIfEmpty()`.
