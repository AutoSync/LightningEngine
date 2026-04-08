# Equinox — Materiais e Shader Composer

Baseado no roadmap v0.15.0, com status unificado por contexto.

## Concluído

- Sistema de instanciamento de materiais (`MaterialInstance` em `Equinox.h`)
- Biblioteca de materiais prontos para uso (`MaterialLibrary` em `Equinox.h`)
- Gerenciador de arquivos do Equinox (`EquinoxFileManager` em `Equinox.h`)
- Templates base para documentos `.equinox` e `.etexgen`
- Detecção e inspeção básica de assets Equinox no editor
- MVP do Equinox
	- `MaterialInstance`: `SetFloat`, `SetColor`, `SetTexture`, `Apply`
	- Integração inicial com Spark (`Tint`, `Light`, `Fog`, `Time`)
	- `MaterialLibrary`: `Add`, `Get`, `Has`
	- `EquinoxFileManager`: detectar tipo, gerar template, ler/escrever texto, inspecionar documento

## Concluído no editor

- Aba própria de Equinox no workspace de documentos
- Integração do menu `Tools -> Open Equinox` com um workspace dedicado em `assets/equinox`
- Integração do Content Browser com criação contextual de `Equinox Shader Composer` e `Texture Generator`
- Propriedades de arquivo com resumo do documento Equinox (`stage`, `output`, contagem de nodes e parametros)
- Roteamento de extensões `.equinox`, `.etexgen`, `.lmat`, `.lmatfunc`, `.lmatinst` e `.lmatlayer` para o contexto Equinox

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
- Fluxo atual de editor: `Content Browser/Tools -> EquinoxFileManager -> Aba Equinox -> save/load textual`
- Compatibilidade alvo: uso no runtime C++ e integração com script/editor no ecossistema da engine.