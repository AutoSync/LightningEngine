# Especificação do Editor

O Editor é a interface gráfica principal para desenvolvimento de jogos com a Lightning Engine. Ele deve ser intuitivo, responsivo e fornecer acesso fácil a todas as funcionalidades do motor, incluindo gerenciamento de projetos, edição de cenas, configuração de assets e depuração em tempo real.

- O Sketch do editor esta disponível em `Plan/Design/editor.png` e pode ser atualizado conforme o desenvolvimento do editor avança.

## Composição e Regras de Design

- Editor: Janela principal contem um TabControl que gerencia as abs de Projeto, Cena, Assets, etc.
- Projeto: A Aba do projeto carreaga a Cena atual, permitindo a edição de objetos, configuração de propriedades, etc. somente uma cena pode ser editada por vez. a tela de projetos so pode ser encerrada ao fechar o editor.
  - Cena: A Aba de cena exibe a hierarquia de objetos da cena, permitindo a seleção e edição de objetos. A cena deve ser atualizada em tempo real conforme as mudanças são feitas. o content browser exibe os assets disponíveis para a cena, permitindo a adição de objetos e configuração de propriedades. cada asset aberto possui uma aba propria sensivel ao contexto, a aba de texturas possui funcionabilidades relativas a edição e configuração de texturas, assim como Shaders(Equinox), Particulas(Hurricane), e outros ativos, permitindo a edição de multiplos assets ao mesmo tempo. `[Cor da Fonte da Aba Branca, Náo pode ser fechada, Icone Paisagem 🌄]`
- Random - A aba de codigo exibe o código fonte dos scripts do projeto, permitindo a edição e configuração de scripts. a aba de código deve ser atualizada em tempo real conforme as mudanças são feitas. `[Cor da Fonte da Aba Laranja, Poder Fechar, Icone de Código 💻]`
- Textura - A aba de texturas exibe as texturas disponíveis no projeto, permitindo a edição e configuração de texturas. a aba de texturas deve ser atualizada em tempo real conforme as mudanças são feitas. `[Cor da Fonte da Aba Vermelho, Poder Fechar, Icone de Textura 🖼️]`
- Shaders - A aba de shaders exibe os shaders disponíveis no projeto, permitindo a edição e configuração de shaders. a aba de shaders deve ser atualizada em tempo real conforme as mudanças são feitas. `[Cor da Fonte da Aba Verde, Poder Fechar, Icone de Shader 🎨]`
- Materiais (Equinox) - A aba de materiais exibe os materiais disponíveis no projeto, permitindo a edição e configuração de materiais. a aba de materiais deve ser atualizada em tempo real conforme as mudanças são feitas. `[Cor da Fonte da Aba Verde Escuro, Poder Fechar, Icone de esfera 🔵]`
- Particulas (Hurricane) - A aba de partículas exibe os sistemas de partículas disponíveis no projeto, permitindo a edição e configuração de partículas. a aba de partículas deve ser atualizada em tempo real conforme as mudanças são feitas. `[Cor da Fonte da Aba Roxa, Poder Fechar, Icone de Particula 💨]`
- Prefabs - A aba de prefabs exibe os prefabs disponíveis no projeto, permitindo a edição e configuração de prefabs. a aba de prefabs deve ser atualizada em tempo real conforme as mudanças são feitas. `[Cor da Fonte da Aba Amarela, Poder Fechar, Icone de Game Object 🧩]`
- Malha Estatica - A aba de malhas exibe as malhas estáticas disponíveis no projeto, permitindo a edição e configuração de malhas. a aba de malhas deve ser atualizada em tempo real conforme as mudanças são feitas. `[Cor da Fonte da Aba Azul Claro, Poder Fechar, Icone de Cubo 🧊]`
- Malha Animada - A aba de malhas animadas exibe as malhas animadas disponíveis no projeto, permitindo a edição e configuração de malhas animadas. a aba de malhas animadas deve ser atualizada em tempo real conforme as mudanças são feitas. `[Cor da Fonte da Aba Rosa, Poder Fechar, Icone de Boneco 🧍]`
- Struct - A Aba de Struct exibe as structs disponíveis no projeto, permitindo a edição e configuração de structs. a aba de structs deve ser atualizada em tempo real conforme as mudanças são feitas. `[Cor da Fonte da Aba Cinza Claro, Poder Fechar, Icone de Struct 📦]`
- Enum - A Aba de Enum exibe os enums disponíveis no projeto, permitindo a edição e configuração de enums. a aba de enums deve ser atualizada em tempo real conforme as mudanças são feitas. `[Cor da Fonte da Aba Cinza Escuro, Poder Fechar, Icone de Lista 📋]`
- Classes, Componentes que so sao visiveis dentro de prefabs, e ajudam no funcionamento do jogo - A Aba de Classes exibe as classes disponíveis no projeto, permitindo a edição e configuração de classes. a aba de classes deve ser atualizada em tempo real conforme as mudanças são feitas. `[Cor da Fonte da Aba Roxo Escuro, Poder Fechar, Icone de Classe 🏷️]`
- GameObjects - A aba de GameObjects e uma ClassSensitive que exibe classes, componentes e prefabs, permitindo a edição e configuração de GameObjects. a aba de GameObjects deve ser atualizada em tempo real conforme as mudanças são feitas. `[Cor da Fonte da Aba Cinza, Poder Fechar, Icone de controle 🎮]`
- Configurações - A aba de configurações exibe as configurações do projeto, permitindo a edição e configuração de propriedades do projeto. a aba de configurações deve ser atualizada em tempo real conforme as mudanças são feitas. `[Cor da Fonte da Aba Cinza, Poder Fechar, Icone de Configurações ⚙️]`

## Do Codigo ao Editor

- O Editor deve ser capaz de ler o codigo dos scripts do projeto, interpretá-los e exibir as informações relevantes nas abas correspondentes. por exemplo, a aba de GameObjects deve ser capaz de ler as classes e componentes definidos nos scripts do projeto, e exibir essas informações de forma organizada e acessível para o usuário.
- O Editor deve ser capaz de converter Variaveis, Funções, Classes, etc. em elementos visuais nas abas correspondentes, permitindo a edição e configuração desses elementos de forma intuitiva e eficiente. por exemplo, a aba de GameObjects deve ser capaz de converter as classes e componentes definidos nos scripts do projeto em elementos visuais que possam ser editados e configurados pelo usuário.
  - Float, Int, devem ser convertidos em campos de texto ou sliders, permitindo a edição de seus valores. por exemplo, uma variável do tipo Float pode ser convertida em um campo de texto que permita a edição de seu valor, ou em um slider que permita a edição de seu valor de forma mais visual e intuitiva.
  - String deve ser convertido em um campo de texto, permitindo a edição de seu valor. por exemplo, uma variável do tipo String pode ser convertida em um campo de texto que permita a edição de seu valor de forma simples e direta.
  - Bool deve ser convertido em uma checkbox, permitindo a edição de seu valor. por exemplo, uma variável do tipo Bool pode ser convertida em uma checkbox que permita a edição de seu valor de forma rápida e fácil.
  - Vector2, Vector3, Vector4 devem ser convertidos em campos de texto ou sliders, permitindo a edição de seus valores. por exemplo, uma variável do tipo Vector3 pode ser convertida em três campos de texto que permitam a edição de seus valores individuais, ou em um conjunto de sliders que permitam a edição de seus valores de forma mais visual e intuitiva.
  - Color deve ser convertido em um seletor de cores, permitindo a edição de seu valor. por exemplo, uma variável do tipo Color pode ser convertida em um seletor de cores que permita a edição de seu valor de forma visual e intuitiva.
  - Enums devem ser convertidos em dropdowns, permitindo a seleção de seus valores. por exemplo, uma variável do tipo Enum pode ser convertida em um dropdown que permita a seleção de seu valor a partir de uma lista de opções pré-definidas.
  - Arrays devem ser convertidos em listas editáveis, permitindo a adição, remoção e edição de seus elementos. por exemplo, uma variável do tipo Array pode ser convertida em uma lista editável que permita a adição, remoção e edição de seus elementos de forma fácil e eficiente.
  - Classes e Componentes devem ser convertidos em elementos visuais que possam ser editados e configurados pelo usuário, permitindo a edição de suas propriedades e a adição de componentes a GameObjects. por exemplo, uma classe definida em um script do projeto pode ser convertida em um elemento visual na aba de GameObjects que permita a edição de suas propriedades e a adição de componentes a GameObjects.

## Aba de Cena

### Menu

- File
  - New Project (Ctrl + N)
  - New Scene (Ctrl + Shift + N)
  - Open Project (Ctrl + O)
  - Open Scene (Ctrl + Shift + O)
  - Save Files (Ctrl + S)
  - Save All (Ctrl + Shift + S)
  - Close Project
  - Exit
- Edit
  - Select All (Ctrl + A)
  - Undo (Ctrl + Z)
  - Redo (Ctrl + Shift + Z)
  - Cut (Ctrl + X)
  - Copy (Ctrl + C)
  - Paste (Ctrl + V)
  - Duplicate (Ctrl + W)
  - Delete (Del)
  --------------
  - Move Forward (Mouse Left Click + W)
  - Move Backward (Mouse Left Click + S)
  - Move Left (Mouse Left Click + A)
  - Move Right (Mouse Left Click + D)
  - Move Up (Mouse Left Click + E)
  - Move Down (Mouse Left Click + Q)
- Tools
  - Open Atlas
  - Sprite Tool
  - Open Equinox
  - Open Hurricane
  - Open Level Script
  - Profiler
- Window
  - Project
  - Content Browser
  - Assets
  - Hierarchy View
  - Properties
  - Console
- Help
  - Documentation
  - About

### Barra de Ferramentas

- Botao de Salvar
- Botao de Play/Pause
- Botao de Compilação
- Botao de Configurações de Cena
- Botao de Ferramentas (Atlas, Sprite Tool, Equinox, Hurricane, Level Script, Profiler)

### Hierarquia de Objetos

- Container de objetos da cena, exibindo a hierarquia de objetos e permitindo a seleção e edição de objetos. deve ser atualizado em tempo real conforme as mudanças são feitas na cena.

### Content Browser

- Container de assets disponíveis para a cena, exibindo os assets disponíveis e permitindo a adição de objetos e configuração de propriedades. deve ser atualizado em tempo real conforme as mudanças são feitas na cena.

### Aba Random Ignite

- Aba responsavel por exibir e processar codigo visual em grafos e nos, containers de variaveis, funcoes, classes, etc. deve ser atualizado em tempo real conforme as mudanças são feitas nos scripts do projeto.

- Os Tipos sao de codigo script. Componentes de Cena, Componentes de Logica, Prefabs, Shaders, Materials, Particulas, Inteligencia Artificial, Arvore de Decisão, etc. devem ser convertidos em elementos visuais que possam ser editados e configurados pelo usuário, permitindo a edição de suas propriedades e a adição de componentes a GameObjects. por exemplo, uma classe definida em um script do projeto pode ser convertida em um elemento visual na aba de Random Ignite que permita a edição de suas propriedades e a adição de componentes a GameObjects.