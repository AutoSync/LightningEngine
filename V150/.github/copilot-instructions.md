# Lightning Engine V150 — Project Guidelines

## Code Style
- Manter compatibilidade com C++17 (MSVC e Makefile usam C++17).
- Seguir organização atual: headers públicos em `src/include/` e implementação em `src/core/`/`src/graphics/`.
- Preservar namespaces existentes:
  - `LightningEngine` para runtime/core da engine
  - `Lightning` para tipos utilitários/matemáticos
  - `Titan` para GUI
- Antes de criar novos padrões, preferir exemplos existentes em `examples/rpg2D/` e componentes em `src/include/components/`.

## Architecture
- Fluxo principal da engine: `Window -> GameInstance -> Level -> Node -> Component`.
- `GameInstance` orquestra ciclo de vida (`Initialize`, `Update`, `Render`, `Shutdown`) e níveis.
- `Node` agrega componentes e estrutura de cena; componentes encapsulam comportamento (`Update`/`Render`).
- Renderização centralizada em `Renderer` com SDL3 + shaders SPIR-V em `assets/shaders/`.
- Submódulos relevantes: Nucleo (scripting), Titan (UI), Hurricane (particles), Equinox (materials).

## Build and Test
- Priorizar tarefas do VS Code:
  - `Build Debug x64 (MSBuild)`
  - `Build Release x64 (MSBuild)`
- Alternativas de build:
  - `build-msvc.bat`
  - `do_build.bat`
  - `make debug`, `make release`, `make clean`
- Compilação de shaders GLSL:
  - `src/shaders/compile_shaders.bat` (requer `glslc` no PATH via Vulkan SDK)
- Não há suíte de testes formal no repositório; validar mudanças compilando em Debug x64.

## Conventions
- Arquivos de referência para padrões de contribuição:
  - `examples/rpg2D/RPG2DApp.h` (padrão de `GameInstance`)
  - `examples/rpg2D/GameLevel.h` (montagem de nível e componentes)
  - `src/include/components/SpriteRenderer.h` (padrão de componente)
  - `src/include/Renderer.h` (API de renderização)
- Dependências third-party locais em `third_party/` (`SDL3`, `glm`, `Assimp`, `stb`).
- Evitar introduzir novos padrões de nomenclatura/macros onde já existem convenções em `src/include/Types.h`.

## Environment Pitfalls
- Build MSVC depende de caminho local hardcoded em scripts (`build-msvc.bat`, `do_build.bat`); ajustar se o Visual Studio estiver em outro diretório.
- O projeto precisa das DLLs de runtime (SDL3/Assimp) no diretório de output; o `.vcxproj` já copia via pre-build.
- O Makefile é útil para fluxo rápido, mas não representa toda a solução MSVC; para validação completa, usar build da `.sln`.

## Key Docs
- Roadmap/visão técnica: `Plan/Roadmaps/Roadmap.md`
- Setup do VS Code e troubleshooting: `README_VSCODE.md`
