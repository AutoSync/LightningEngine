# 2. Manifesto do plugin (`plugin.json`)

Todo plugin é precedido por um arquivo `plugin.json` no mesmo diretório
do binário. Este arquivo é a **única coisa** que a engine lê antes de
decidir se abre sua biblioteca.

## 2.1 Schema mínimo

```json
{
  "id": "com.example.hello",
  "name": "Hello Plugin",
  "version": "0.1.0",
  "engineVersionMin": "0.1.0",
  "engineVersionMax": "",
  "scope": "Project",
  "category": "Examples",
  "subcategory": "",
  "dependencies": [],
  "permissions": [],
  "enabledByDefault": true,
  "entryNative": "HelloPlugin.dll"
}
```

## 2.2 Campos

| Campo                | Tipo       | Obrigatório | Descrição                                       |
| -------------------- | ---------- | ----------- | ----------------------------------------------- |
| `id`                 | string     | sim         | ID reverse-DNS único (`com.studio.feature`)     |
| `name`               | string     | sim         | Nome legível, exibido no editor                 |
| `version`            | semver     | sim         | Versão do plugin                                |
| `engineVersionMin`   | semver     | sim         | Versão mínima da engine suportada               |
| `engineVersionMax`   | semver     | não         | Vazio = sem teto                                |
| `scope`              | enum       | sim         | `"Project"` ou `"Global"`                       |
| `category`           | string     | não         | Para agrupamento na UI                          |
| `subcategory`        | string     | não         |                                                 |
| `dependencies`       | string[]   | não         | Lista de `id`s carregados antes                 |
| `permissions`        | string[]   | não         | Subset de: FileSystem, Network, Device, ScriptApi, EditorUI |
| `enabledByDefault`   | bool       | não         | Default: `true`                                 |
| `entryNative`        | string     | não         | Caminho relativo do `.dll`/`.so`                |
| `entryCSharp`        | string     | não         | Caminho do `.dll` C# (script)                   |
| `entryIgnite`        | string     | não         | Caminho do bundle Ignite                        |

> Pelo menos **um** dos `entryNative`/`entryCSharp`/`entryIgnite` deve ser
> declarado.

## 2.3 Escopo: Project × Global

- **Project**: o plugin vive em `<projeto>/Plugins/<id>/`. Versionado
  junto com o jogo. Recomendado para plugins específicos do título.
- **Global**: instalado pelo usuário em `~/.lightningengine/plugins/<id>/`.
  Compartilhado entre projetos. Use para ferramentas (ex.: importador
  custom de FBX).

## 2.4 Permissões — exemplos

```json
{
  "permissions": ["FileSystem", "EditorUI"]
}
```

Significa: o plugin pode ler arquivos do projeto **e** desenhar painéis
no editor. Ele **não** pode acessar rede; chamadas sockets falham.

> Nunca peça permissões que não usa. O usuário pode revogar e seu plugin
> deve degradar com elegância.

## 2.5 Dependências

```json
{
  "dependencies": ["com.studio.combat", "com.studio.inventory"]
}
```

A engine carrega `combat` e `inventory` **antes** do seu plugin. Se
qualquer dependência falhar, seu plugin entra em estado `Failed` com
`lastError = "dependency com.studio.combat failed"`.

## 2.6 Validação

Use `lightning plugin validate plugin.json` (CLI da engine, futuro) ou
abra o editor — qualquer campo inválido aparece em
**Window → Plugin Manager** com mensagem específica.
