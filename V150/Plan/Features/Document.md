# Regras de Projeto Lightning Engine

# Hierarquia do Projeto Lightning Engine

- / (raiz do projeto)
  - config/
    - project.ini
    - editor.ini
    - gamesettings.ini
  - content/
  - plugins/
  - extras/
  - binaries/

## Configurações

- project.ini (criados na pagina do projeto)
- editor.ini
- gamesettings.ini

- O arquivo project.ini é o ponto de entrada para a configuração do projeto, ele é configurado em uma aba de cena Menu->Edit->Project Settings, Uma Aba de configuração do projeto deve ser aberta e botao de salvar salva as mudanças nesse arquivo, ele deve conter as seguintes informações:
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
  Resolução, qualidade gráfica, volume de áudio, controles, etc.

- O arquivo editor.ini é o ponto de entrada para a configuração do editor, ele é configurado em uma aba de cena editor->Edit->Editor Settings, Uma Aba de configuração do editor deve ser aberta e botao de salvar salva as mudanças nesse arquivo, ele deve conter as seguintes informações:
- Ao carregar o projeto o Editor deve ler as configurações do editor.ini e aplicar as configurações, ele deve conter as seguintes informações:
  - Configurações de layout (posição das janelas, abas abertas)
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

### Pasta Plugins

- A pasta Plugins é o local onde todos os plugins do projeto são armazenados.
- O editor deve ser capaz de carregar e gerenciar plugins dentro dessa pasta, mas não deve permitir acesso a arquivos fora dela.
- O editor deve fornecer uma interface para ativar, desativar e configurar plugins dentro da pasta Plugins, mas não deve permitir a criação ou modificação de arquivos fora dessa pasta.

### Pasta Extras

- A pasta Extras é o local onde recursos adicionais, como documentação, exemplos e outros arquivos relacionados ao projeto são armazenados.
- O editor deve ser capaz de acessar e manipular os arquivos dentro da pasta Extras, mas não deve permitir acesso a arquivos fora dela.
- O editor deve fornecer uma interface para navegar e acessar os recursos dentro da pasta Extras, mas não deve permitir a criação ou modificação de arquivos fora dessa pasta.

### Pasta Binaries

- A pasta Binaries é o local onde os arquivos executáveis e outros arquivos de saída do projeto são armazenados.
- O editor deve ser capaz de gerar e armazenar os arquivos de saída dentro dessa pasta, mas não deve permitir acesso a arquivos fora dela.
- O editor deve fornecer uma interface para acessar e executar os arquivos dentro da pasta Binaries, mas não deve permitir a criação ou modificação de arquivos fora dessa pasta.