# Fase 0: Analise do Loop Atual de Window e Implicacoes para Embed SDL

## Resumo Executivo
A engine ja possui um ponto unico de ownership do ciclo SDL em [src/core/Window.cpp](src/core/Window.cpp) e um exemplo operacional de janela adicional em [src/include/GamePreviewWindow.h](src/include/GamePreviewWindow.h). Isso confirma que a integracao com uma janela unica embutida exige preservar a semantica de lifecycle da janela principal e o contrato de claim/release do device GPU.

## Achados Principais
1. `Window::onInit()` executa `SDL_Init`, cria `SDL_Window`, cria `SDL_GPUDevice` e chama `SDL_ClaimWindowForGPUDevice`.
2. `Window::Run()` faz o loop principal com `inputManager.Update()`, `SDL_PollEvent`, `Update(dt)` e `Render()`.
3. `Window::onShutdown()` libera renderer, device, window e finaliza SDL em ordem fixa.
4. `GamePreviewWindow` mostra que o repositorio ja trabalha com uma segunda janela SDL compartilhando o mesmo `SDL_GPUDevice`.
5. O preview window processa eventos filtrando por `windowID`, o que e um padrao util para o embed.

## Pontos de Acoplamento Relevantes
1. Ownership do `SDL_Window` e do `SDL_GPUDevice` esta centralizado em `Window`.
2. O renderer depende diretamente do window/device criado na inicializacao.
3. O loop atual assume ownership unico do ciclo de eventos e da atualizacao de input.
4. A abertura de janelas adicionais ja existe, mas ainda em modelo separado do host Tauri.

## Implicacoes para o Embed SDL
1. A integracao precisa decidir quem cria e quem destrói a janela host.
2. A ordem de inicializacao deve preservar `SDL_Init` antes do claim do device.
3. O tratamento de eventos precisa separar eventos do host Tauri e do contexto SDL.
4. O shutdown precisa manter a ordem de release do renderer, GPU device e janela para evitar vazamentos ou crash.

## Recomendacoes de Implementacao
1. Extrair uma camada minima de lifecycle para facilitar a composicao com o host.
2. Formalizar a ponte de eventos por `windowID` e por canais de mensagem entre camadas.
3. Validar o embed em spike curto antes de mexer no restante do editor.
4. Manter o preview window como referencia de comportamento para janela secundaria e roteamento de eventos.

## Resultado Esperado da Fase
1. Documento Go/No-Go para embed SDL.
2. Lista de ajustes necessarios no ciclo de vida da janela.
3. Base para iniciar a Fase 1 com riscos conhecidos e limitados.
