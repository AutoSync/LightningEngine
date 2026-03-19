---
applyTo: "src/shaders/**"
description: "Use when editing shader files (.vert/.frag/.spark) or shader build scripts in src/shaders/."
---

# Shader Guidelines (Lightning Engine)

## Scope
- Estas regras valem para arquivos em `src/shaders/**`.
- Para diretrizes gerais de engine, usar `.github/copilot-instructions.md`.

## Shader Format and Targets
- Manter shaders GLSL no padrão usado pelo projeto (versão 450).
- Para shaders raster padrão, preservar pares de estágios coerentes (`.vert` + `.frag`).
- Quando alterar fontes GLSL, garantir que o output SPIR-V correspondente em `assets/shaders/` seja atualizado no fluxo de build adequado.
- Em arquivos `.spark`, manter separação clara de estágios (`@vertex`, `@fragment`) e semântica já adotada pelo Spark Composer.

## Naming and Compatibility
- Evitar renomear shaders já referenciados pelo runtime sem atualizar todos os pontos de uso.
- Preservar nomes e contratos de uniforms/inputs/outputs quando o shader já estiver integrado no `Renderer`.
- Preferir mudanças incrementais e compatíveis com o pipeline existente, sem introduzir novo padrão de binding sem necessidade.

## Validation
- Após editar shaders em `src/shaders/`, executar `src/shaders/compile_shaders.bat` quando aplicável.
- Se houver mudança com impacto em runtime, validar com build Debug x64 da solução (`Build Debug x64 (MSBuild)`).
- Se a compilação de shader falhar, verificar primeiro `glslc` no PATH (Vulkan SDK).

## Editing Principles
- Não mover shaders entre diretórios sem motivo forte.
- Não adicionar novos formatos de shader além dos já utilizados pelo projeto.
- Manter arquivos objetivos: sem comentários longos e sem blocos experimentais não utilizados.
