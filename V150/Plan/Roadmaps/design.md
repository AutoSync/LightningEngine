# Sistema de Design do Editor (Fonte de Verdade)

Status: ativo
Escopo: Lightning Engine Editor (Titan UI)

Este documento define as regras visuais e comportamentais do editor.
Quando houver necessidade de mudança de design, este arquivo deve ser atualizado antes da implementação no código.

---

## 1) Fontes de referência de design

As seguintes imagens são a base visual do editor:

- Plan/Design/editor.png
- Plan/Design/tabs_mock.png
- Plan/Design/Titan Ui Styles.png
- Plan/Design/containers_rules.png
- Plan/Design/script_tab.png
- Plan/Design/splash.png
- Plan/Design/logo.png

Observação: as abas visíveis em editor.png representam linguagem visual e polimorfismo de abas, não necessariamente páginas simultaneamente ativas de conteúdo final.

---

## 2) Regras de composição e hierarquia visual

### 2.1 Estrutura macro

- Topo: barra de título + faixa de abas.
- Linha 2: menubar horizontal (FILE, EDIT, TOOLS, WINDOW, HELP).
- Linha 3: toolbar de ações rápidas.
- Corpo: DockSpace com Camera, Content Browser, Viewport, Hierarchy e Inspector.
- Rodapé: status bar com informações de projeto.

### 2.2 Prioridade de camadas

- Menus dropdown e context menus devem sempre sobrepor o restante da UI.
- Popups não podem vazar visual de elementos traseiros.
- Tooltips/toasts ficam acima de componentes comuns, sem quebrar legibilidade.

### 2.3 Regras de legibilidade

- Fundo de menu/context menu opaco.
- Hover deve ser perceptível em até 1 estado de interação (sem animações complexas).
- Borda de painéis/menus deve separar regiões sem ruído visual.

---

## 3) Sistema de cores (tokens)

### 3.1 Tokens base do tema Titan (Editor)

- winBg: 20,20,26
- panelBg: 28,28,36
- panelHeader: 22,22,30
- panelBorder: 50,50,65
- btnNormal: 48,48,62
- btnHover: 62,66,90
- btnActive: 38,84,155
- btnBorder: 70,70,90
- btnText: 215,215,220
- textNormal: 210,210,215
- textDim: 120,120,132
- textBright: 255,255,255
- textAccent: 90,160,255
- textGreen: 90,210,110
- textRed: 235,75,75
- selection: 38,88,158,200
- highlight: 50,110,195
- separator: 48,48,62
- menuBg: 36,36,46
- menuBorder: 60,60,76
- statusBg: 22,22,28

### 3.2 Paleta semântica de abas por tipo

- Project: 255,255,255 (não fechável)
- Random (script): 255,138,28
- Textura: 210,65,65
- Shaders: 72,190,90
- Material (Equinox): 60,150,80
- Hurricane (partículas): 145,80,210
- Prefab: 220,190,70
- Malha Estática: 110,180,245
- Malha Animada: 220,120,185
- Struct: 185,185,195
- Enum: 110,110,125
- Classes: 120,88,160
- GameObjects: 135,140,155
- Configurações: 140,140,150

Regra: a cor da aba representa o tipo de asset/contexto (semantic color).

---

## 4) Regras comportamentais de UI

### 4.1 Menus

- Menubar com grupos: FILE, EDIT, TOOLS, WINDOW, HELP.
- Dropdown segue ordem de desenho estrita (preservar painter order).
- Clique fora fecha dropdown/context menu.

### 4.2 Toolbar

Botões mínimos:
- Salvar
- Play/Pause
- Compilar
- Config Cena
- Ferramentas

### 4.3 Abas context-sensitive

- Clique em asset no Content Browser deve abrir/ativar aba contextual pelo tipo.
- Abas de conteúdo derivadas de asset são fecháveis.
- Aba Project é fixa e não fechável.
- Ativar uma aba não deve destruir o layout principal do editor.

### 4.4 Content Browser

- Exibe assets relevantes do projeto (assets/scripts/scenes).
- Cor de texto do item pode indicar tipo.
- Clique: ativa contexto visual (aba por tipo).
- Drag and drop continua disponível para viewport.

---

## 5) Mapeamento de extensões para contexto visual

- .lescene -> contexto Project/Scene
- .spark, .cs -> contexto Random (script)
- .png, .jpg, .jpeg, .bmp, .tga -> contexto Textura
- .vert, .frag, .spv -> contexto Shaders
- .mat, .material -> contexto Material
- .hurricane, .particle -> contexto Hurricane
- .prefab -> contexto Prefab
- .obj, .fbx, .gltf, .glb -> contexto Malha Estática
- .anim, .skel -> contexto Malha Animada
- .ini, .json, .toml, .yaml, .yml -> contexto Configurações

---

## 6) Regras de evolução (obrigatório)

1. Alteração de regra visual/comportamental deve ser descrita primeiro neste arquivo.
2. Depois da atualização do documento, implementar no código.
3. Após implementação, validar build Debug x64.
4. Se necessário, anexar screenshot comparativo no processo de revisão.

---

## 7) Critérios de aceitação para pixel-match

- Estrutura macro respeita editor.png.
- Menus/context menus sobrepõem corretamente sem bleed-through.
- Tab strip reflete semântica de cor por tipo.
- Toolbar e menu correspondem à especificação funcional.
- Contraste e legibilidade consistentes no tema escuro.
