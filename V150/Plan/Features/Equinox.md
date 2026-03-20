# Equinox — Materiais e Shader Composer

Baseado no roadmap v0.15.0, com status unificado por contexto.

## Concluído

- Sistema de instanciamento de materiais (`MaterialInstance` em `Equinox.h`)
- Biblioteca de materiais prontos para uso (`MaterialLibrary` em `Equinox.h`)
- MVP do Equinox
	- `MaterialInstance`: `SetFloat`, `SetColor`, `SetTexture`, `Apply`
	- Integração inicial com Spark (`Tint`, `Light`, `Fog`, `Time`)
	- `MaterialLibrary`: `Add`, `Get`, `Has`

## Pendente

- Material Graph Editor (node-based, drag and drop, preview)
- Suporte a múltiplos tipos de materiais (Standard, Unlit, PBR, Custom)
- Integração com Spark para geração automática de shaders via graph
- Suporte a parâmetros customizáveis (cores, texturas, floats, bools)
- Suporte a texturas múltiplas (normal, especular, emissivo, etc)
- Exportação/importação de materiais (`.leasset`)
- Preview em tempo real no editor
- Integração com Hurricane e MotionPhysix
- Suporte a materiais animados (script ou nodes)
- Suporte a custom nodes (via script)

## Observações

- Fluxo macro planejado: `MaterialGraph -> MaterialInstance -> MaterialLibrary -> Spark`
- Compatibilidade alvo: uso no runtime C++ e integração com script/editor no ecossistema da engine.