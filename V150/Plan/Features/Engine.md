# Features da Engine para desenvolvimento de Jogos/Apresentações/Animações 2D e 3D

## Controles e Elementos de Cena

- [ ] Gizmo 2D e 3D Transform/Rotate/Scale: Selecionar e manipular entidades na cena. para rotate e Scale precisa ser implementado um sistema de snapping (ex: 15° para rotação, 0.1x para escala).e Relativo e Absoluto (world vs local).
- [ ] Camera Controller:
    - [ ] Spectator (orbit + pan + zoom) usada quando não há player ou editor possuindo a controlando a câmera.
    - [ ] CameraComponent: Componente que pode ser adicionado a uma entidade para controlar a câmera. Suporta diferentes modos de controle (ex: FPS, top-down, etc) e pode ser possuída por um player ou pelo editor.
    - [ ] CineCamera: Câmera avançada para animação e cinematics, com suporte a rigs, dolly tracks, etc.
- [ ] Node: Elemento básico da cena, pode conter componentes e filhos. Suporta hierarquia e transformações locais.
