# Estruturacao dos Elementos da Engine e Expansao por Codigo

## Objetivo

Este documento descreve como os elementos da Lightning Engine sao estruturados hoje, como sao criados no runtime e no editor, como entram no menu de contexto, e como a organizacao por categorias e subcategorias funciona no estado atual do codigo.

O foco aqui e expansao da engine por codigo. Onde o projeto ainda nao possui um sistema formal de registro, isso esta descrito explicitamente para evitar acoplamento em suposicoes incorretas.

Referencia operacional atualizada: [../../Docs/Engine-Element-Expansion.md](../../Docs/Engine-Element-Expansion.md)


---

## 1. Hierarquia estrutural da engine

O fluxo principal da engine segue esta cadeia:

```text
Window
  -> GameInstance
      -> Level
          -> Node / Node2D / Node3D
              -> Component
```

### Responsabilidades

- Window: loop principal, janela, input bruto e renderer base.
- GameInstance: ponto de entrada da aplicacao; carrega e troca Levels.
- Level: dono dos nodes raiz da cena.
- Node: unidade base da hierarquia de cena.
- Component: comportamento anexado ao Node.

### Contrato de ownership

- GameInstance possui o Level ativo.
- Level possui os nodes raiz via std::unique_ptr.
- Node possui seus filhos e seus componentes.
- Component nao e dono do Node; ele recebe owner.

Isso significa que a expansao por codigo deve respeitar a cadeia de posse existente. O caminho normal nao e instanciar elementos soltos, mas cria-los e entrega-los ao dono correto.

---

## 2. Como os elementos sao criados

## 2.1. Criacao de Levels

O ponto padrao de criacao de um Level e GameInstance::LoadLevel<T>().

```cpp
class MyGame : public LightningEngine::GameInstance {
public:
    void Initialize() override {
        LoadLevel<MyLevel>();
    }

    void Shutdown() override {}
    void Update(float dt) override { UpdateLevel(dt); }
    void Render() override { RenderLevel(); }
};
```

O metodo:

1. destroi o Level ativo anterior, se existir;
2. instancia o novo Level;
3. injeta renderer e input com SetContext();
4. chama Initialize().

## 2.2. Criacao de Nodes

Nodes sao criados normalmente com std::make_unique e adicionados ao Level com AddNode().

```cpp
auto player = std::make_unique<LightningEngine::Node2D>("Player");
player->tag = "player";
player->SetPosition(200.f, 120.f);
AddNode(std::move(player));
```

Quando um node entra no Level:

- o contexto de renderer/input/level e injetado;
- a ownership passa para o Level;
- o node passa a participar do Update() e Render().

## 2.3. Criacao de filhos na hierarquia

Filhos sao criados com AddChild().

```cpp
auto weapon = std::make_unique<LightningEngine::Node>("Weapon");
parentNode->AddChild(std::move(weapon));
```

Nesse fluxo o child herda o contexto do pai no momento da insercao.

## 2.4. Criacao de Components

Components entram em nodes via AddComponent<T>().

```cpp
auto* sprite = player->AddComponent<LightningEngine::SpriteRendererComponent>();
sprite->SetSize(64.f, 64.f);

auto* body = player->AddComponent<LightningEngine::RigidBody2DComponent>();
body->body.useGravity = true;
```

Regras importantes:

- o sistema permite uma instancia por tipo por node;
- se o component ja existir, AddComponent<T>() devolve o existente;
- OnAttach() e chamado automaticamente;
- RemoveComponent<T>() chama OnDetach().

---

## 3. Como o editor cria elementos hoje

O editor atual usa um modelo hibrido.

- Hierarchy: usa um registro parcial via `EditorElementRegistry`, alimentado em `registerHierarchyElements()` e consumido em `buildHierarchyContextMenuFromRegistry()`.
- Content Browser: ainda e montado manualmente em `rebuildContentBrowserContextMenu()` e nos helpers de criacao/importacao de `EditorApp`.
- Inspector: continua manual e dirigido por `refreshInspector()`.

Isso significa que a expansao do editor ja tem um ponto formal para factories de Hierarchy, mas os fluxos de assets e propriedades ainda dependem de integracao explicita em `EditorApp`.

## 3.1. Criacao pela Hierarchy

Hoje existem dois fluxos principais de criacao na Hierarchy:

### A. Botao Add

O botao da Hierarchy chama addNodeToScene("Node").

Esse helper:

1. cria um Node simples;
2. adiciona ao editorLevel;
3. reconstrui a arvore visual;
4. registra operacao no UndoStack.

### B. Menu de contexto da Hierarchy

O menu de contexto da Hierarchy nasce em `buildContextMenus()`, mas os itens de criacao sao registrados antes em `registerHierarchyElements()` e montados via `buildHierarchyContextMenuFromRegistry()`.

Itens atuais:

- Add Child Node
- Duplicate
- Delete
- Add Script

Hoje o fluxo e dividido em duas partes:

1. `EditorElementRegistry` registra descritores de `NodeFactory` e `ComponentFactory`.
2. O `ContextMenu` recebe os itens agrupados por categoria/subcategoria.

Cada acao continua chamando helpers diretos, por exemplo:

- addChildToSelected()
- duplicateSelected()
- deleteSelected()
- AddComponent<NucleoScriptComponent>()

Conclusao pratica: para inserir uma nova factory de node ou component na Hierarchy, o caminho atual e registrar um novo descritor em `registerHierarchyElements()`. Para acoes fora desse escopo, o fluxo ainda e manual em `EditorApp`.

## 3.2. Criacao de assets pelo Content Browser

O Content Browser funciona como gerenciador de arquivos do projeto.

Os fluxos principais sao:

- `importFileToCurrentDir()`: importa arquivos externos para o diretorio atual;
- `createFolderInCurrentDir()`: cria pastas no diretorio atual;
- `createAssetInCurrentDir()`: cria documentos textuais com template inicial;
- `openAssetContextTab()`: abre o asset no workspace de documentos sensivel ao tipo.

Para Equinox, o editor agora possui helpers dedicados:

- `createEquinoxAssetInCurrentDir(EquinoxAssetType)`
- `openEquinoxWorkspace()`

Esses helpers usam `EquinoxFileManager` para gerar templates padrao de `.equinox` e `.etexgen`, abrir o workspace `assets/equinox` e encaminhar o documento para a aba propria do Equinox.

## 3.3. Criacao por drag and drop

Existe um segundo fluxo de criacao em processCBDrop().

Exemplos atuais:

- .spark: adiciona ou reusa NucleoScriptComponent no node selecionado; se nao houver node, cria um novo no drop;
- .lescene: carrega a cena;
- imagem: ainda nao instancia automaticamente SpriteRendererComponent, apenas registra log.

Esse ponto e relevante para expansao porque ele ja funciona como uma fabrica de comportamento dirigida por tipo de asset.

## 3.4. Exposicao no Inspector

O inspector atual tambem e manual. Ele exibe:

- propriedades base do Node;
- transform;
- bloco do NucleoScriptComponent, se presente;
- botao para adicionar script.

Ou seja: criar um novo component no runtime nao o torna editavel automaticamente no editor. Para isso, e necessario ampliar refreshInspector().

---

## 4. Como o menu de contexto funciona

O widget `ContextMenu` evoluiu para um menu hierarquico baseado em caminhos separados por `/`.

Capacidades atuais:

- AddItem(texto, acao, enabled)
- AddSeparator()
- Open(x, y)
- arvore automatica de submenus a partir de labels como `Equinox/Texture Generator`
- submenus em janelas adicionais
- scroll por nivel quando o menu excede a altura visivel

Limitacoes atuais:

- nao existe uma API publica separada de `AddSubmenu()`; a hierarquia e inferida por path;
- o menu ainda nao nasce de metadata universal do projeto inteiro; parte dele continua sendo montada manualmente em `EditorApp`;
- categorias de components fora da Hierarchy continuam dependendo de codigo explicito para aparecer no editor.

Implicacao arquitetural:

Se voce quiser um menu com estrutura do tipo:

```text
Criar
  -> 2D
      -> Sprite
      -> Camera
  -> 3D
      -> Mesh
      -> Light


```

isso ja pode ser representado diretamente com labels hierarquicas no `ContextMenu`.

Hoje ha dois caminhos de extensao:

1. registrar descritores em `EditorElementRegistry` quando o fluxo for de Hierarchy;
2. adicionar entradas hierarquicas em `rebuildContentBrowserContextMenu()` quando o fluxo for de asset/tooling.

Para expansao limpa da engine, a segunda opcao e a mais sustentavel.

---

## 5. Como categorias e subcategorias estao organizadas hoje

## 5.1. Estado atual real

Hoje o projeto possui categorias como convencao, nao como sistema formal de dados.

Exemplos:

- SpriteRendererComponent esta documentado como Category: Scene.
- BoxRendererComponent tambem esta em Scene.
- RigidBody2DComponent esta em Logic.

Essas categorias aparecem como comentario nos headers dos componentes. Elas nao sao lidas automaticamente pelo editor, nao geram menus e nao alimentam nenhum registro global.

## 5.2. Onde existe organizacao de dados de fato

Hoje a organizacao existe em tres niveis diferentes:

### A. Organizacao estrutural

- Level organiza nodes raiz.
- Node organiza children.
- Node organiza components por std::type_index.

### B. Organizacao visual manual do editor

- Hierarchy organiza a arvore de nodes.
- Inspector organiza blocos por secoes codificadas manualmente.
- ContextMenu organiza itens em lista simples.

### C. Organizacao semantica por convencao

- comentarios Category nos headers;
- nomes de tipos;
- namespaces e pastas.

## 5.3. O que ainda nao existe

No estado atual, a engine/editor ainda nao possuem:

- registro unico de elementos editaveis;
- metadados formais de categoria/subcategoria;
- fabrica de nodes/components baseada em descritores;
- geracao automatica do menu de contexto a partir de metadados;
- inspector dinamico por tipo.

Isso e o ponto central para qualquer expansao consistente do editor.

---

## 6. Fluxo recomendado para expansao por codigo

Se o objetivo for adicionar novos elementos da engine e expo-los no editor, o fluxo mais seguro hoje e este.

## 6.1. Criar o tipo base do elemento

Escolha primeiro se o novo elemento e:

- um novo Node especializado, como Node2D/Node3D;
- um novo Component para anexar a nodes existentes.

Na maioria dos casos, a extensao deve ser um Component.

Exemplo:

```cpp
class Light2DComponent : public LightningEngine::Component {
public:
    float radius = 128.f;
    Uint8 colorR = 255;
    Uint8 colorG = 220;
    Uint8 colorB = 120;
    Uint8 colorA = 180;

    void Render() override
    {
        if (!owner || !owner->renderer) return;
        Lightning::V2 p = owner->WorldPosition();
        owner->renderer->SetDrawColor(colorR, colorG, colorB, colorA);
        owner->renderer->FillRect(p.x - radius * 0.5f, p.y - radius * 0.5f, radius, radius);
    }
};
```

## 6.2. Definir a categoria semantica

Como ainda nao existe metadata formal, a categoria precisa ser mantida por convencao no proprio codigo.

Padrao recomendado:

- comentar Category e Subcategory no header;
- manter o tipo dentro da pasta adequada;
- usar nome consistente com o papel do componente.

Exemplo:

```cpp
// Category: Rendering
// Subcategory: Lighting2D
class Light2DComponent : public LightningEngine::Component {
    ...
};
```

## 6.3. Expor criacao no editor

Para aparecer no menu de contexto da Hierarchy hoje, adicione um item manual em buildContextMenus().

Exemplo conceitual:

```cpp
hierMenu->AddItem("Add Light2D", [this]{
    Node* target = selectedNode;
    if (!target) {
        auto node = std::make_unique<Node2D>("Light2D");
        target = node.get();
        editorLevel.AddNode(std::move(node));
    }
    target->AddComponent<Light2DComponent>();
    rebuildHierarchyTree();
    refreshInspector();
});
```

## 6.4. Expor edicao no inspector

Tambem e necessario criar o bloco de edicao manual dentro de refreshInspector().

Exemplo conceitual:

```cpp
if (auto* light = node->GetComponent<Light2DComponent>()) {
    stack->Add<Label>(0.f, 0.f, "Light2D");
    auto* radius = stack->Add<NumericUpDown>(0.f, 0.f, innerW, lh, light->radius, 0.f, 4096.f, 1.f, 1);
    radius->onChanged = [light](float v) { light->radius = v; };
}
```

## 6.5. Expor uso em examples e runtime

Depois de criar o tipo, use um exemplo real em um Level para consolidar o contrato de uso.

Isso e importante porque o projeto segue muito a pratica de usar examples como referencia arquitetural.

---

## 7. Modelo recomendado para categorias e subcategorias futuras

Para a engine crescer sem continuar expandindo menus e inspector na mao, o recomendado e introduzir um registro de elementos do editor.

## 7.1. Descritor sugerido

```cpp
struct EditorElementDescriptor {
    std::string id;
    std::string label;
    std::string category;
    std::string subcategory;
    bool addAsChildWhenPossible = true;
    std::function<void(EditorApp&)> createInEditor;
};
```

## 7.2. Registro sugerido

```cpp
class EditorElementRegistry {
public:
    static EditorElementRegistry& Instance();

    void Register(EditorElementDescriptor descriptor);
    const std::vector<EditorElementDescriptor>& All() const;
};
```

## 7.3. Beneficios

Com um registro assim, o editor passa a conseguir:

- montar o menu de contexto a partir dos descritores;
- ordenar itens por category/subcategory;
- aplicar filtros no Content Browser ou Inspector;
- reduzir alteracoes manuais espalhadas por EditorApp;
- permitir expansao modular por novos arquivos ou modulos.

## 7.4. Estrutura de dados recomendada para menu

Mesmo sem suporte nativo a submenu no widget atual, a organizacao dos dados pode nascer antes da UI.

Exemplo de agrupamento:

```cpp
std::map<std::string, std::map<std::string, std::vector<EditorElementDescriptor>>> grouped;

for (const auto& item : registry.All()) {
    grouped[item.category][item.subcategory].push_back(item);
}
```

Assim a UI pode ser trocada no futuro sem mudar a fonte de dados.

---

## 8. Diretriz pratica para expansao da engine

Se a intencao for expandir a engine de forma consistente, siga esta ordem:

1. crie o tipo de runtime primeiro;
2. documente Category e Subcategory no header;
3. valide o uso em um Level de exemplo;
4. exponha criacao no editor;
5. exponha edicao no inspector;
6. so depois pense em automatizar via registro.

Essa ordem evita criar um editor generico antes de existir um contrato estavel de elementos.

---

## 9. Resumo executivo

No estado atual da Lightning Engine:

- a estrutura base de elementos esta bem definida em GameInstance -> Level -> Node -> Component;
- a criacao de elementos no runtime ja segue um fluxo consistente com ownership claro;
- a integracao com o editor e manual, concentrada principalmente em EditorApp;
- categorias e subcategorias ainda sao convencoes sem metadata executavel;
- o menu de contexto atual e flat e nao possui suporte a submenu.

Portanto, para expansao por codigo, o caminho real hoje e:

- criar o tipo;
- adicionar manualmente a acao no editor;
- adicionar manualmente a edicao no inspector;
- se o projeto quiser escalar essa abordagem, introduzir um registro de elementos com category/subcategory e fabrica associada.

---

## 10. Estrutura alvo para expansao da engine (codigo + plugins)

Com base na especificacao de expansao, a estrutura de elementos deve sair do modelo manual e evoluir para um modelo dirigido por registro.

## 10.1. Eixos de expansao

O sistema passa a ter dois eixos oficiais:

1. Expansao por codigo (compilada junto da engine);
2. Expansao por plugin (carregamento dinamico em runtime/editor).

Ambos devem publicar elementos no mesmo registro para manter comportamento consistente no editor.

## 10.2. Registro unificado de elementos

Introduzir um registro unificado com metadados obrigatorios:

```cpp
enum class ElementSource {
    Core,
    CodeExpansion,
    PluginGlobal,
    PluginProject
};

enum class ElementKind {
    NodeFactory,
    ComponentFactory,
    AssetType,
    MenuAction,
    PanelFactory
};

struct ElementCategoryPath {
    std::string category;      // ex: "Rendering"
    std::string subcategory;   // ex: "Lighting2D"
};

struct EngineElementDescriptor {
    std::string id;            // id unico estavel: plugin/light2d.component
    std::string label;         // nome de exibicao
    ElementKind kind;
    ElementSource source;
    ElementCategoryPath path;
    int sortOrder = 0;
    bool enabledByDefault = true;

    std::function<void(EditorApp&)> createInEditor;
};
```

Esse registro passa a ser a fonte unica para:

- montagem do menu de contexto;
- listagens por categoria/subcategoria;
- filtros do inspector/content browser;
- habilitacao/desabilitacao por plugin.

## 10.3. Organizacao de dados por categoria/subcategoria

Estrutura recomendada para indexacao:

```cpp
using ElementBucket = std::vector<EngineElementDescriptor>;
using SubcategoryMap = std::map<std::string, ElementBucket>;
using CategoryMap = std::map<std::string, SubcategoryMap>;
```

Regra de ordenacao:

1. category;
2. subcategory;
3. sortOrder;
4. label.

Essa ordenacao precisa ser aplicada em qualquer ponto de exibicao para evitar divergencia entre paines.

## 10.4. Menu de contexto orientado a registro

O menu deve deixar de ser codificado item por item e passar a ser montado por dados.

Requisito funcional:

- suportar submenu por categoria e subcategoria.

Como o ContextMenu atual e flat, a evolucao deve ocorrer em duas fases:

1. Fase transitoria: prefixos no label para simular arvore (ex: Rendering/Lighting2D/Add Light);
2. Fase final: widget com submenu nativo e navegacao hierarquica.

## 10.5. Contrato de API para expansao por codigo

A expansao por codigo deve expor APIs especificas por dominio:

- API de UI/editor (widgets, tabs, paineis, inspectors);
- API de registro de elementos (categorias e subcategorias);
- API de tipos de arquivo proprietarios/comuns (extensoes, thumbnails, icones);
- API de servicos (HTTP, dispositivos USB/proprietarios quando habilitados).

Contrato minimo para um modulo de expansao por codigo:

```cpp
class ICodeExpansionModule {
public:
    virtual ~ICodeExpansionModule() = default;
    virtual const char* Id() const = 0;
    virtual void RegisterElements(EditorElementRegistry& registry) = 0;
    virtual void RegisterAssetTypes(AssetTypeRegistry& assets) = 0;
};
```

---

## 11. Estrutura alvo de plugins

## 11.1. Tipos de plugin

Suportar oficialmente:

1. Plugin C++ (DLL/biblioteca dinamica);
2. Plugin gerenciado/scriptado (C# e Ignite, conforme roadmap).

Todos devem expor descritores para o mesmo registro unificado.

## 11.2. Escopo de plugin

Dois escopos devem coexistir:

- Global: disponivel para todos os projetos;
- Projeto: disponivel apenas para o projeto atual.

Recomendacao de pasta:

```text
engine/plugins/global/
projects/<project>/plugins/
```

## 11.3. Manifesto de plugin

Cada plugin deve fornecer manifesto com metadados e permissoes.

Campos minimos:

- id, name, version;
- engineVersionMin/Max;
- scope (global/project);
- category/subcategory;
- entrypoints (C++, C#, Ignite);
- permissions (filesystem, network, device, scripting, editor-ui);
- dependencies;
- enabledByDefault.

## 11.4. Configuracao YAML de plugins ativos

Manter lista de plugins ativos por padrao em YAML da engine e opcionalmente sobrescrever no projeto.

Exemplo:

```yaml
plugins:
  enabled:
    - core.rendering
    - core.scripting
    - user.light2d
  disabled:
    - experimental.mesh-tools
```

## 11.5. Ciclo de vida de plugin

Ciclo recomendado:

1. Discover;
2. Validate manifest;
3. Resolve dependencies;
4. Sandbox/permissions;
5. Load runtime;
6. Register elements;
7. Activate;
8. Deactivate/Unload.

Para estabilidade, falha de um plugin nao pode interromper bootstrap do editor inteiro.

## 11.6. Painel de plugins no editor

Criar painel para:

- habilitar/desabilitar plugins;
- visualizar categoria/subcategoria;
- mostrar origem (global/projeto);
- mostrar permissoes solicitadas;
- exibir estado (loaded, failed, disabled);
- aplicar reload seguro quando possivel.

---

## 12. Pacotes de plugin e distribuicao

Formato de pacote sugerido: leplugin

Operacoes esperadas:

- package: gerar pacote instalavel;
- unpackage/install: instalar e registrar automaticamente;
- enable on startup: iniciar junto ao projeto quando habilitado.

Fluxo minimo:

1. instalar pacote;
2. validar manifesto/assinatura;
3. registrar plugin no escopo correto;
4. aplicar no proximo bootstrap (ou hot-load se suportado).

---

## 13. Seguranca e isolamento

A especificacao exige isolamento de plugins. A estrutura deve incluir:

- permissoes declarativas por manifesto;
- bloqueio de APIs sensiveis sem permissao;
- limites de falha por plugin (isolamento de erro);
- logs de auditoria por plugin;
- opcionalmente assinatura/verificacao de integridade de pacote.

Politica minima:

- deny-by-default para recursos sensiveis;
- permitir apenas o que estiver declarado e aceito;
- registrar no painel quais permissoes estao ativas.

---

## 14. Compatibilidade multiplataforma

Para garantir portabilidade:

- separar interface de plugin de detalhes de loader por plataforma;
- padronizar ABI/C-API para plugins nativos;
- resolver artefatos por plataforma no pacote (win/linux/mac);
- validar versionamento de API da engine antes de load.

Isso evita acoplamento do ecossistema a uma unica plataforma.

---

## 15. Roadmap de implementacao sugerido

Implementacao incremental recomendada:

1. Introduzir EditorElementRegistry com category/subcategory;
2. Migrar buildContextMenus para leitura de registro;
3. Adicionar suporte a submenu no ContextMenu;
4. Criar PluginManifest + PluginManager + YAML de plugins ativos;
5. Integrar painel de plugins no editor;
6. Adicionar empacotamento leplugin;
7. Expor ponte para C# e Ignite no mesmo contrato de registro.

Essa ordem reduz risco e permite validacao continua sem quebrar o fluxo atual do editor.