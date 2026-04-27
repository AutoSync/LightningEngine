# HelloPlugin — exemplo mínimo de plugin nativo

Plugin "olá mundo" para a Lightning Engine. Demonstra:

- Manifesto `plugin.json`.
- Handshake de versão da API via `LightningPluginApiVersion`.
- Implementação dos cinco hooks de `IEnginePlugin`.
- Build com CMake fora do repositório principal.

Documentação completa: [`docs/enduser/03-c-api-plugin.md`](../../../docs/enduser/03-c-api-plugin.md).

## Build (dentro do repo)

```powershell
cd examples/plugins/HelloPlugin
cmake -B build -A x64
cmake --build build --config Release
```

Saída: `build/Release/HelloPlugin.dll` + `plugin.json` ao lado.

## Build (fora do repo)

Aponte o SDK para a pasta de headers públicos da engine:

```powershell
cmake -B build -A x64 -DLIGHTNING_ENGINE_SDK="C:/path/to/engine/src/include"
cmake --build build --config Release
```

## Instalação

Copie a pasta `build/Release/` (renomeada para `HelloPlugin/`) para
`<seu-projeto>/Plugins/HelloPlugin/`. Abra o editor → **Window → Plugin
Manager** → **Activate**.

## Estrutura

```
HelloPlugin/
├── CMakeLists.txt
├── plugin.json
├── README.md
├── include/
│   └── (vazio neste exemplo)
└── src/
    └── HelloPlugin.cpp
```
