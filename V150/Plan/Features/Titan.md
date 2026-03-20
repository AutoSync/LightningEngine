# Titan — GUI

Baseado no roadmap v0.15.0, com status unificado por contexto.

## Concluído

- Sistema de GUI
- Menus, janelas, abas
- Widgets base
	- Border, Backdrop, Button, Image, Label, TextField, Text
	- Checkbox, ProgressBar, Dropdown, List, Slider, Window, TabStrip, ContextMenu
	- Viewport2D (Canvas/Panel, ScrollView, Layouts)
- Eventos (`EventBus`)
- Framebuffer para UI (Create/Resize/Release, Begin/End RenderToTexture)
- Blur (`Renderer::BlurTexture` + `blur.frag`)
- Tooltips, Modais, Toasts
- Widgets avançados
	- TreeView, NumericUpDown, Splitter, Toolbar, MenuBar, ColorPicker
	- Table/GridView, FileDialog, StatusBar
- Atalhos customizáveis (`TitanUI::AddShortcut`)
- Redimensionamento/movimentação de janelas internas
- DockSpace (docking de painéis)
- Chart widget (Line/Bar)
- Clipboard e edição de texto avançada no `TextField`
- Eventos customizados e bindings (`EventBus` + `InputBinding`)

## Pendente

- Responsive layout
- Scriptable UI
	- Link com Script Engine
	- Realtime GUI no editor
- GUI Editor
	- Drag and drop de widgets
	- Configuração de propriedades
	- Visualização em tempo real
- DatePicker
- Drag and drop entre widgets e arquivos do sistema
- Suporte a múltiplas janelas
- Undo/redo para interações de UI
- Widgets customizados via scripts
- Preview em tempo real de alterações de UI no editor
- Sistema de estilos (CSS-like)
	- Glassmorphism
	- Presets de tema (Dark, Light, Glassmorphism, Custom)
	- Estilo por widget

## Observações

- O Titan atende o editor e runtime da engine; editor visual dedicado segue como etapa futura.