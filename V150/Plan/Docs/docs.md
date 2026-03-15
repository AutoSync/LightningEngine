# Lightning Engine — Documentação

> Versão 0.15.0 · Windows / Linux · Backend: SDL3 GPU (Vulkan / D3D12 / Metal — selecionado automaticamente)

---

## Índice

1. [Visão Geral](#visão-geral)
2. [Hierarquia de Classes](#hierarquia-de-classes)
3. [Criando um Jogo](#criando-um-jogo)
4. [Ciclo de Vida](#ciclo-de-vida)
5. [InputManager](#inputmanager)
6. [Renderer](#renderer)
7. [Texture](#texture)
8. [Level](#level)
9. [Types](#types)
10. [Estrutura de Arquivos](#estrutura-de-arquivos)

---

## Visão Geral

Lightning Engine é um motor de jogos 2D escrito em C++17.
O usuário nunca lida diretamente com SDL ou GPU — o motor expõe apenas `renderer` e `inputManager` como membros protegidos.

O backend gráfico é SDL3 GPU (abstração sobre Vulkan/D3D12/Metal).
A seleção do backend é automática conforme a máquina de destino — o desenvolvedor não precisa se preocupar com isso.

---

## Hierarquia de Classes

```text
Window
  └── GameInstance        ← ponto de entrada do usuário
        └── Level         ← cena/fase gerenciada pelo jogo
              └── (Nodes / Components — roadmap futuro)
```

`Renderer` e `InputManager` são utilitários disponíveis em qualquer nível da hierarquia via membros protegidos.

---

## Criando um Jogo

Inclua `GameInstance.h`, herde de `LightningEngine::GameInstance` e implemente os quatro métodos virtuais.

```cpp
#include "src/include/GameInstance.h"
#include "src/include/Types.h"

class MyGame : public LightningEngine::GameInstance {
public:
    void Initialize() override {
        renderer.SetClearColor(30, 30, 30);
    }

    void Shutdown() override {}

    void Update(float dt) override {
        if (inputManager.IsKeyPressed(SDL_SCANCODE_ESCAPE)) Quit();
    }

    void Render() override {
        renderer.Clear();
        renderer.SetDrawColor(255, 0, 0);
        renderer.FillRect(100.f, 100.f, 50.f, 50.f);
        renderer.Present();
    }
};

int main(int argc, char* argv[]) {
    MyGame game;
    game.Run("Meu Jogo", 1280, 720);
    return 0;
}
```

### `Run(title, width, height, fullscreen)`

| Parâmetro    | Tipo         | Padrão  | Descrição                  |
|--------------|--------------|---------|----------------------------|
| `title`      | `const char*`| —       | Título da janela           |
| `width`      | `int`        | `1280`  | Largura em pixels          |
| `height`     | `int`        | `720`   | Altura em pixels           |
| `fullscreen` | `bool`       | `false` | Modo tela cheia            |

---

## Ciclo de Vida

A cada frame o motor executa na seguinte ordem:

```text
InputManager::Update()        ← snapshot do frame anterior
SDL_PollEvent → ProcessEvent  ← coleta eventos de hardware
Initialize()  (1x na startup)
↓
loop:
  Update(deltaTime)           ← lógica do jogo
  Render()                    ← desenho
↓
Shutdown()    (1x no encerramento)
```

### Métodos da janela disponíveis no jogo

| Método          | Descrição                                      |
|-----------------|------------------------------------------------|
| `Quit()`        | Encerra o loop no fim do frame atual           |
| `GetWidth()`    | Largura atual da janela em pixels              |
| `GetHeight()`   | Altura atual da janela em pixels               |

---

## InputManager

Acessado via `inputManager` (membro protegido de `GameInstance`).

### Teclado

```cpp
// Tecla pressionada e mantida neste frame
inputManager.IsKeyDown(SDL_SCANCODE_W)

// Tecla foi pressionada neste frame (edge — não repete)
inputManager.IsKeyPressed(SDL_SCANCODE_SPACE)

// Tecla foi solta neste frame
inputManager.IsKeyReleased(SDL_SCANCODE_SHIFT)
```

Usa `SDL_Scancode` (independente de layout de teclado).
Referência completa: [SDL_Scancode](https://wiki.libsdl.org/SDL3/SDL_Scancode)

### Mouse

```cpp
// Botões: 1 = esquerdo, 2 = meio, 3 = direito
inputManager.IsMouseDown(1)
inputManager.IsMousePressed(1)
inputManager.IsMouseReleased(1)

// Posição do cursor em pixels (origin = canto superior esquerdo)
float x = inputManager.GetMouseX();
float y = inputManager.GetMouseY();
```

---

## Renderer

Acessado via `renderer` (membro protegido de `GameInstance`).

### Frame

```cpp
renderer.SetClearColor(30, 30, 30);   // cor de fundo (r, g, b) ou (r, g, b, a)
renderer.Clear();                      // início do frame — limpa a fila de draw
renderer.Present();                    // envia todos os draws para a GPU e exibe
```

`Clear()` e `Present()` **devem** ser chamados no início e fim de `Render()`, respectivamente.

### Formas coloridas

```cpp
renderer.SetDrawColor(255, 0, 0);           // cor atual (r, g, b) — alpha = 255
renderer.SetDrawColor(0, 255, 0, 128);      // com transparência

renderer.FillRect(x, y, largura, altura);   // retângulo preenchido
renderer.DrawRect(x, y, largura, altura);   // contorno de retângulo (1px)
```

- Coordenadas em pixels, origem no canto superior esquerdo.
- Alpha blending está habilitado: `a = 0` transparente, `a = 255` opaco.

### Texturas

```cpp
// Carregar (BMP nativo; PNG/JPG requer stb_image)
Texture tex = renderer.LoadTexture("assets/sprite.bmp");

// Desenhar no tamanho natural da imagem
renderer.DrawTexture(tex, x, y);

// Desenhar com tamanho explícito
renderer.DrawTexture(tex, x, y, largura, altura);

// Desenhar região (sprite sheet) — UVs de 0.0 a 1.0
renderer.DrawTextureRegion(tex, x, y, largura, altura,
                            u0, v0,   // canto superior esquerdo
                            u1, v1);  // canto inferior direito

// Liberar quando não precisar mais
tex.Release();
```

`SetDrawColor` age como **tint** nas texturas: `(255, 255, 255, 255)` = sem tint.

#### Exemplo — sprite sheet 4×1 (4 frames de 32px cada)

```cpp
float frameW = 1.f / 4.f; // cada frame ocupa 25% da largura
int   frame  = 2;         // frame 0..3

renderer.DrawTextureRegion(sheet,
    x, y, 32.f, 32.f,
    frame * frameW, 0.f,
    (frame + 1) * frameW, 1.f);
```

---

## Texture

`Texture` é um handle para uma textura na GPU.

```cpp
Texture tex = renderer.LoadTexture("assets/img.bmp");

tex.IsValid()    // false se o carregamento falhou
tex.GetWidth()   // largura em pixels
tex.GetHeight()  // altura em pixels
tex.Release()    // libera recursos da GPU
```

Texturas são carregadas **uma vez** (em `Initialize()`) e reutilizadas nos frames.

---

## Level

`Level` representa uma cena ou fase. Gerencie manualmente dentro do `GameInstance`.

```cpp
#include "src/include/Level.h"

class Level1 : public LightningEngine::Level {
public:
    void Initialize() override { /* setup */ }
    void Shutdown()   override { /* cleanup */ }
    void Update(float dt) override { /* lógica */ }
    void Render()     override { /* desenho via renderer */ }
};
```

> **Nota:** `Level` ainda não recebe `renderer` / `inputManager` automaticamente na v0.15.0.
> Passe referências via construtor ou métodos setter conforme necessário.

---

## Types

Inclua `src/include/Types.h` para os tipos matemáticos do motor.

```cpp
#include "src/include/Types.h"
using namespace Lightning;

V2  pos(400.f, 300.f);     // vetor 2D (float x, y)
V3  col(1.f, 0.f, 0.f);   // vetor 3D
V4  rect(0,0,100,100);     // vetor 4D
```

---

## Estrutura de Arquivos

```text
LightningEngine/V150/
├── Editor.cpp                    ← entry point / sandbox de desenvolvimento
├── examples/
│   └── third_person_shooter.h   ← exemplo de jogo completo
├── src/
│   ├── include/
│   │   ├── GameInstance.h       ← herdar para criar seu jogo
│   │   ├── Window.h             ← loop principal, SDL interno
│   │   ├── Renderer.h           ← API de desenho 2D
│   │   ├── InputManager.h       ← teclado e mouse
│   │   ├── Texture.h            ← handle de textura GPU
│   │   ├── Level.h              ← base para cenas/fases
│   │   └── Types.h              ← V2, V3, V4, etc.
│   ├── core/
│   │   ├── Window.cpp
│   │   ├── Engine.cpp
│   │   ├── InputManager.cpp
│   │   └── Texture.cpp
│   ├── graphics/
│   │   └── Renderer.cpp
│   └── shaders/
│       ├── rect2d.vert / rect2d.frag         ← shapes coloridas
│       ├── rect2d_tex.vert / rect2d_tex.frag ← texturas
│       └── compile_shaders.bat               ← recompilar SPIRV (requer Vulkan SDK)
├── assets/
│   └── shaders/
│       ├── rect2d_vert.spv / rect2d_frag.spv
│       └── rect2d_tex_vert.spv / rect2d_tex_frag.spv
├── third_party/
│   └── SDL3/                    ← headers e libs SDL3
└── LightningEngine.vcxproj      ← projeto Visual Studio 2022
```

### Compilar

Abra no Visual Studio 2022 (toolset v145) e compile `Debug | x64`, ou via VSCode com a task **Build**.

Para recompilar os shaders (necessário apenas ao modificar `.vert`/`.frag`):

```bat
cd src\shaders
compile_shaders.bat
```

Requer `glslc` do [Vulkan SDK](https://vulkan.lunarg.com/) no PATH.
