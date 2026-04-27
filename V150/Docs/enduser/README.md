# Lightning Engine — Guia para autores de plugins

> Audiência: desenvolvedores externos que querem **estender** a Lightning
> Engine sem modificar o core. Para trabalhar **no core** da engine, vá
> para [`docs/developers/`](../developers/README.md).

## Sumário

1. [Conceitos básicos](01-getting-started.md)
2. [Manifesto do plugin](02-plugin-manifest.md)
3. [Plugin nativo (C++ / C ABI)](03-c-api-plugin.md)
4. [Distribuição e instalação](04-distributing.md)

## O que um plugin pode fazer

- Adicionar **componentes** novos (ex.: `HealthBar`, `Inventory`).
- Adicionar **sistemas** rodando no loop (ex.: `AISystem`).
- Reagir a **eventos** do core (`CollisionEvent`, `InputEvent`, …).
- Adicionar **comandos do editor** (botões, menus, painéis).
- Expor **assets** novos (formatos, importadores).

## O que **não** pode

- Substituir/remover símbolos do core.
- Acessar memória interna de outros plugins.
- Bypass das `PluginPermission` declaradas no manifest.

## Caminho mais rápido

```bash
git clone https://github.com/<você>/lightning-plugin-template
cd lightning-plugin-template
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
# copie build/MyPlugin.dll + plugin.json para
# <projeto-do-jogo>/Plugins/MyPlugin/
```

Abra o editor e o plugin aparece em **Window → Plugin Manager**, pronto
para ser ativado.

> O template `lightning-plugin-template` segue o padrão do exemplo
> [`examples/plugins/HelloPlugin`](../../examples/plugins/HelloPlugin/README.md).

## Compatibilidade

Sua biblioteca **deve** declarar a versão da API que foi compilada:

```cpp
extern "C" std::uint32_t LightningPluginApiVersion() {
    return LIGHTNING_ENGINE_API_VERSION;
}
```

Se o usuário tentar carregar seu plugin numa versão de engine
incompatível, ele será marcado como `Failed` com a mensagem
`engine API mismatch`. Atualize/recompile contra a nova versão e
publique novo build.
