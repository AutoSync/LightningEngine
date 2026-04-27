# 4. Distribuição e instalação

Plugins são distribuídos como diretórios autocontidos. Esta página
descreve o formato `.lepkg` (zip) e as opções de canal.

## 4.1 Formato do pacote `.lepkg`

`.lepkg` é simplesmente um **zip** com a árvore do plugin na raiz:

```
HelloPlugin.lepkg
├── plugin.json
├── HelloPlugin.dll        (Windows)
├── libHelloPlugin.so      (Linux)
├── libHelloPlugin.dylib   (macOS)
├── README.md
├── LICENSE
└── assets/
```

Inclua **todos** os binários nativos que você suporta. O loader escolhe
o correto pela plataforma.

> Não inclua `.lib`/`.exp` (Windows) nem símbolos de debug no pacote
> publicado — distribua um zip separado para debug se quiser.

## 4.2 Canais de distribuição

| Canal              | Quem usa                        | Observação                              |
| ------------------ | ------------------------------- | --------------------------------------- |
| Drag-and-drop      | Usuário final                   | Arrasta `.lepkg` no editor              |
| Pasta `Plugins/`   | Desenvolvedor do jogo           | Versão fica no repositório do jogo      |
| Marketplace oficial| Comunidade (futuro)             | Assinado, atualização automática        |
| Diretório global   | Usuário avançado                | `~/.lightningengine/plugins/`           |

## 4.3 Atualização

Substituir os arquivos do plugin **com a engine fechada** sempre funciona.
Para hot-reload via editor:

1. **Window → Plugin Manager → Deactivate** no plugin alvo.
2. **Unload** (libera o handle da DLL).
3. Substitua os arquivos.
4. **Load → Register → Activate**.

Plugins que aderem ao contrato (todas as assinaturas canceladas em
`OnDeactivate`, todos os recursos liberados em `OnUnload`) sobrevivem
a este ciclo sem reiniciar a engine.

## 4.4 Versionamento da sua release

Recomendação: semver (`MAJOR.MINOR.PATCH`).

| Mudança                                | Bump                  |
| -------------------------------------- | --------------------- |
| Bugfix sem mudar API exposta           | `PATCH`               |
| Novo componente / sistema retrocompat. | `MINOR`               |
| Renomear/remover componente            | `MAJOR`               |
| Mudar `engineVersionMin`               | `MAJOR` (na maioria)  |

## 4.5 Assinatura (futuro)

O marketplace oficial exige `.lepkg` assinado. Hoje a verificação é
**opcional**: o editor avisa quando carrega plugin não assinado, mas
permite. Quando publicar:

```bash
lightning plugin sign HelloPlugin.lepkg --key publisher.key
```

> A CLI `lightning plugin` ainda está em design. Ver
> [Plan/Roadmaps/Roadmap.md](../../Plan/Roadmaps/Roadmap.md).

## 4.6 Suporte e documentação

- README dentro do pacote: explique requisitos de permissão e uso.
- Inclua exemplos minimalistas em `examples/`.
- Sempre liste `engineVersionMin`/`engineVersionMax` testados.
- Mantenha um `CHANGELOG.md` legível.
