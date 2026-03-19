# Regras de Projeto Lightning Engine

# Hierarquia do Projeto Lightning Engine

- / (raiz do projeto)
  - config/
    - project.leproject
    - editor.leeditor
    - gamesettings.lesettings
  - content/
  - plugins/
  - extras/
  - binaries/

## Configurações

- project.leproject (criados na pagina do projeto)
- editor.leeditor
- gamesettings.lesettings

- O arquivo project.leproject é o ponto de entrada para a configuração do projeto, ele é configurado em uma aba de cena editor->Arquivo->Configurações do Projeto, Uma Aba de configuração do projeto deve ser aberta e botao de salvar salva as mudanças nesse arquivo, ele deve conter as seguintes informações:
  - Nome do projeto
  - Versão do projeto
  - Configurações de build (Debug/Release)
  - Configurações de plataforma alvo (Windows/Linux/Mac)
  - Configurações de renderização (OpenGL/Vulkan/DirectX)
  - Configurações de input (teclado, mouse, gamepad)
  - Configurações de áudio (sample rate, canais)
  - Configurações de física (gravidade, colisões)
  - Configurações de rede (porta, protocolo)
  ----
  Configurações do Jogo tambem sao configuradas nessa aba de configurações do projeto.

- O arquivo project.leproject é o ponto de entrada para a configuração do projeto, ele é configurado em uma aba de cena editor->Edit-> Configurações do Editor, Uma Aba de configuração do editor deve ser aberta e botao de salvar salva as mudanças nesse arquivo, ele deve conter as seguintes informações:
  - Configurações de interface (tema, layout)
  - Configurações de atalhos de teclado
  - Configurações de plugins (habilitar/desabilitar)
  - Configurações de renderização do editor (qualidade, anti-aliasing)
  - Configurações de performance (limite de FPS, uso de threads)
  - Configurações de debug (log level, visualização de gizmos)

## Regras de Contexto

- A Lightning Engine uma vez definida a pasta do projeto, deve operar exclusivamente dentro dessa pasta, sem acessar arquivos fora dela.
- O editor deve ser capaz de ler e escrever arquivos de configuração, assets e outros recursos apenas

### Pasta Content

- A Pasta de Content é o local onde todos os assets do projeto são armazenados, incluindo scripts, texturas, modelos 3D, cenas e outros recursos.
- O editor deve ser capaz de acessar e manipular os arquivos dentro da pasta Content, mas não deve permitir acesso a arquivos fora dela.
- O editor deve fornecer uma interface para navegar, organizar e gerenciar os assets dentro da pasta Content, mas não deve permitir a criação ou modificação de arquivos fora dessa pasta.

### 

