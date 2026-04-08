# Progresso de Implementacao

## 2026-04-03

### Concluido

- Estrutura de expansao documentada em Plan/Docs/EstruturaElementosEngine.md.
- Registro de elementos criado em src/include/EditorElementRegistry.h.
- Contratos de plugin criados em src/include/PluginContracts.h.
- PluginManager inicial criado em src/include/PluginManager.h.
- EditorApp integrado ao registro para montar itens da Hierarchy por category/subcategory (fase 1 flat).
- Menu FILE com Plugins -> Adicionar e Plugins -> Criar Wizard implementado no EditorApp.
- Wizard de plugins implementado com scaffold para C++, C# e Ignite.
- Cache de estado do editor implementado (janela, containers, viewport, content browser e ultima alteracao).
- Titulos das abas agora removem extensao (assets e cenas).
- Renomear diretorios adicionado no Content Browser (F2 e menu de contexto).
- Pasta Docs/ criada como base para documentacao ativa.

### Em andamento

- Migracao do restante das acoes do menu de contexto para registro.
- Evolucao do ContextMenu para submenu nativo.

### Proximo marco

- Criar leitura/escrita YAML de plugins ativos e integrar ao PluginManager.
