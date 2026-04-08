# CodeEditor in Text

Capacidade do Editor em Editar e analisar Codigo via texto

## ContentBrowser

- [x] Editor Proprio para codigos em cs
- [x] Identificar arquivos cs no ContentBrowser e abrir em Editor Especifico

## Linguagens Suportadas

! Cada Linguagem deve ser expandida para sua area de atuação, somente o sistema de reconhecimento e LINT deve ser implementado, codigos especificos devem
ser implementados na sua base de codigo como CS no Editor de Codigo C#, codigos de shader no Editor Spark ou Equinox

- [x] CS
- [ ] Shaders -> OpenGL, DirectX, Vulkan
- [ ] Text
- [ ] Markdown / Viewer MD

## Edicao de Codigo

- [ ] Implementar syntax highlighting para C# com temas personalizáveis.
- [ ] Adicionar code completion com IntelliSense baseado em assemblies do projeto.
- [ ] Implementar navegação de símbolos (Go to Definition, Find All References).
- [ ] Adicionar suporte a debugging com breakpoints e step execution.
- [ ] Integrar formatação de código automática (F#, C# roslyn formatter).
- [ ] Implementar folding de código para blocos (classes, métodos, namespaces).
- [ ] Adicionar minimap e scroll bar visual para arquivos grandes.
- [ ] Suporte a snippets customizáveis para padrões comuns.
- [x] Adicionar ajuste de tamanho da fonte no editor de codigo com Ctrl + Wheel.
- [x] Adicionar atalho Ctrl + '+' para aumentar fonte e Ctrl + '-' para reduzir fonte.
- [x] Persistir tamanho da fonte no cache do editor por projeto.
- [x] Melhorar experiencia de edicao: destaque de linha atual, numeracao de linhas e guia de identacao.
- [x] Melhorar lint em tempo real para scripts (Ignite/C#) com painel de diagnosticos e navegacao para erro.
- [x] Exibir severidade de lint (erro/aviso/info) com cores e contadores na aba de codigo.
- [x] Implementar syntax highlighting com suporte a múltiplas linguagens (C#, GLSL, JSON).
- [ ] Adicionar colorização de tokens com tema escuro/claro configurável.
- [x] Destacar pares de parênteses, chaves e colchetes com matching visual.
- [ ] Implementar busca e substituição com regex e case sensitivity.
- [x] Adicionar visualização de whitespace e caracteres invisíveis.
- [ ] Destacar ocorrências da palavra selecionada no documento.
- [x] Suportar diferenças visuais entre espaços, tabs e quebras de linha.
- [ ] Implementar undo/redo com histórico visual.
- [ ] Adicionar breadcrumb de estrutura (namespace > classe > método).
- [ ] Exibir informações de tipo ao passar o mouse sobre símbolos (hover tooltips).