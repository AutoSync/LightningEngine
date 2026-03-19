# Definição para AutoSync Equinox integrado a Lightning Engine

## Contexto

- Equinox é o sistema de materiais da Lightning Engine, responsável por gerenciar shaders, texturas e propriedades de materiais.

## Uso

Todo Novo código relacionado a materiais, shaders e texturas é gerenciado por um fluxo de trabalho específico para Equinox, que inclui:

1. Criação/edição de shaders GLSL em `src/shaders/` e compilação para SPIR-V.
2. Definição de materiais usando o sistema de Equinox, associando shaders e texturas.
3. Integração de materiais em componentes de renderização, como `MeshRenderer` ou `SpriteRenderer`.
4. Testes de renderização para validar a aparência e desempenho dos materiais.

## Regras de Contribuição

- Manter compatibilidade com o pipeline de renderização atual, evitando mudanças disruptivas sem necessidade.
- Seguir os padrões de organização de arquivos e nomenclatura já estabelecidos para shaders e materiais.
- Validar mudanças com builds regulares e testes visuais, garantindo que os materiais se comportem conforme esperado no runtime.
- Documentar qualquer nova funcionalidade ou mudança significativa no sistema de materiais para facilitar a manutenção futura

## Compatibilidade

- O Equinox deve ser compativel com Random Ignite e C# e do lado editor, e do lado runtime ser compativel com o sistema de renderização atual, sem introduzir dependências ou padrões que não sejam suportados pelo pipeline existente.