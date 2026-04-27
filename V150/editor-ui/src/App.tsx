import {
  useCallback,
  useEffect,
  useMemo,
  useRef,
  useState,
  type CSSProperties,
  type KeyboardEvent as ReactKeyboardEvent,
  type MouseEvent as ReactMouseEvent,
} from 'react';
import { tauriService, type MotorStatus } from './services/tauri-service';
import { ContextMenu, type ContextItem } from './components/ContextMenu';

// ─────────────────────────────────────────────────────────────────────────────
// Types
// ─────────────────────────────────────────────────────────────────────────────

type HNode = {
  id: string;
  label: string;
  parentId: string | null;
  expanded: boolean;
  active: boolean;
  transform: { x: number; y: number; z: number };
  components: string[];
};

type EditorTab = {
  id: string;
  label: string;
  accent: string;
  closable: boolean;
  pinned?: boolean;
};

type TrayTab = 'browser' | 'console' | 'output';

type CtxState = { x: number; y: number; items: ContextItem[] } | null;

// ─────────────────────────────────────────────────────────────────────────────
// Initial data
// ─────────────────────────────────────────────────────────────────────────────

const INITIAL_NODES: HNode[] = [
  { id: 'root',       label: 'Root',       parentId: null,     expanded: true,  active: true, transform: { x: 0,   y: 0,   z: 0 }, components: [] },
  { id: 'player',     label: 'Player',     parentId: 'root',   expanded: true,  active: true, transform: { x: 200, y: 200, z: 0 }, components: ['SpriteRenderer', 'Collider2D', 'NucleoScript'] },
  { id: 'sprite',     label: 'Sprite',     parentId: 'player', expanded: false, active: true, transform: { x: 0,   y: 0,   z: 0 }, components: ['SpriteRenderer'] },
  { id: 'collider',   label: 'Collider',   parentId: 'player', expanded: false, active: true, transform: { x: 0,   y: 0,   z: 0 }, components: ['Collider2D'] },
  { id: 'camera',     label: 'Camera',     parentId: 'root',   expanded: false, active: true, transform: { x: 0,   y: 0,   z: 0 }, components: ['Camera2D'] },
  { id: 'background', label: 'Background', parentId: 'root',   expanded: false, active: true, transform: { x: 0,   y: 0,   z: 0 }, components: ['SpriteRenderer'] },
];

const INITIAL_TABS: EditorTab[] = [
  { id: 'scene',        label: 'Scene',        accent: '#62a7ff', closable: false },
  { id: 'player_spark', label: 'player.spark', accent: '#57cf73', closable: true },
  { id: 'texture',      label: 'hero.png',     accent: '#ff6363', closable: true },
];

const MENU_MODEL: Record<string, { label: string; shortcut?: string; separator?: boolean }[]> = {
  FILE:   [
    { label: 'Nova Cena',     shortcut: 'Ctrl+N' },
    { label: 'Abrir Projeto', shortcut: 'Ctrl+O' },
    { label: 'Salvar Cena',   shortcut: 'Ctrl+S' },
    { label: 'Salvar Tudo',   shortcut: 'Ctrl+Shift+S' },
    { separator: true, label: '' },
    { label: 'Sair' },
  ],
  EDIT:   [
    { label: 'Desfazer',  shortcut: 'Ctrl+Z' },
    { label: 'Refazer',   shortcut: 'Ctrl+Y' },
    { separator: true, label: '' },
    { label: 'Duplicar',  shortcut: 'Ctrl+D' },
    { label: 'Deletar',   shortcut: 'Del' },
  ],
  TOOLS:  [
    { label: 'Build',           shortcut: 'F7' },
    { label: 'Compilar Shaders' },
    { separator: true, label: '' },
    { label: 'Profiler' },
    { label: 'Configuracoes',   shortcut: 'Ctrl+,' },
  ],
  WINDOW: [
    { label: 'Viewport' },
    { label: 'Hierarquia' },
    { label: 'Inspector' },
    { label: 'Content Browser' },
    { separator: true, label: '' },
    { label: 'Console' },
  ],
  HELP:   [
    { label: 'Documentacao' },
    { label: 'Roadmap' },
    { separator: true, label: '' },
    { label: 'Sobre o Lightning Engine' },
  ],
};

const CB_ITEMS = [
  { id: 'player_tex',   name: 'hero.png',        type: 'Texture',  color: '#ff6363' },
  { id: 'player_spark', name: 'player.spark',    type: 'Script',   color: '#57cf73' },
  { id: 'level_scene',  name: 'main.lescene',    type: 'Scene',    color: '#62a7ff' },
  { id: 'enemy_spark',  name: 'enemy.spark',     type: 'Script',   color: '#57cf73' },
  { id: 'bg_mat',       name: 'background.lemat',type: 'Material', color: '#36b86f' },
  { id: 'particles',    name: 'fire.leparticle', type: 'Particle', color: '#a66cff' },
];

const CB_FOLDERS = [
  { id: 'all',      label: 'All',      color: '#f5a623' },
  { id: 'scripts',  label: 'Scripts',  color: '#57cf73' },
  { id: 'textures', label: 'Textures', color: '#ff6363' },
  { id: 'scenes',   label: 'Scenes',   color: '#62a7ff' },
  { id: 'prefabs',  label: 'Prefabs',  color: '#a66cff' },
];

let _nextId = 100;
const genId = () => `node_${_nextId++}`;

// ─────────────────────────────────────────────────────────────────────────────
// App
// ─────────────────────────────────────────────────────────────────────────────

export default function App() {
  // ── engine bridge ──────────────────────────────────────────────────────────
  const [status, setStatus] = useState<MotorStatus>({
    running: false, fps: 60, scene: 'main.lescene', project: 'MyProject', lastChange: '',
  });
  const [loadingMotor, setLoadingMotor] = useState(false);

  useEffect(() => {
    let unsub = () => {};
    tauriService.getStatus().then(setStatus).catch(() => undefined);
    tauriService.onStatusChange((s) => setStatus(s)).then((d) => { unsub = d; });
    return () => unsub();
  }, []);

  const toggleMotor = async () => {
    setLoadingMotor(true);
    try {
      if (status.running) await tauriService.stopMotor();
      else await tauriService.startMotor();
    } finally { setLoadingMotor(false); }
  };

  // ── tabs ───────────────────────────────────────────────────────────────────
  const [tabs, setTabs]           = useState<EditorTab[]>(INITIAL_TABS);
  const [activeTabId, setActiveTabId] = useState('scene');

  const closeTab = useCallback((id: string) => {
    setTabs((prev) => {
      const next = prev.filter((t) => t.id !== id || !t.closable);
      return next.length ? next : [INITIAL_TABS[0]];
    });
    setActiveTabId((prev) => {
      if (prev !== id) return prev;
      const idx = tabs.findIndex((t) => t.id === id);
      const remaining = tabs.filter((t) => t.id !== id || !t.closable);
      return remaining[Math.max(0, idx - 1)]?.id ?? remaining[0]?.id ?? 'scene';
    });
  }, [tabs]);

  const closeOtherTabs = (keepId: string) => {
    setTabs((prev) => prev.filter((t) => t.id === keepId || !t.closable));
    setActiveTabId(keepId);
  };

  const closeTabsToRight = (id: string) => {
    const idx = tabs.findIndex((t) => t.id === id);
    setTabs((prev) => prev.filter((t, i) => i <= idx || !t.closable));
  };

  // ── hierarchy ──────────────────────────────────────────────────────────────
  const [nodes, setNodes]             = useState<HNode[]>(INITIAL_NODES);
  const [selectedId, setSelectedId]   = useState<string | null>('player');
  const [renamingId, setRenamingId]   = useState<string | null>(null);
  const renameRef = useRef<HTMLInputElement>(null);

  const selectedNode = useMemo(
    () => nodes.find((n) => n.id === selectedId) ?? null,
    [nodes, selectedId],
  );

  const toggleExpand = (id: string) =>
    setNodes((prev) => prev.map((n) => n.id === id ? { ...n, expanded: !n.expanded } : n));

  const toggleActive = (id: string) =>
    setNodes((prev) => prev.map((n) => n.id === id ? { ...n, active: !n.active } : n));

  const addNode = useCallback((parentId: string, componentPreset = '') => {
    const id = genId();
    const newNode: HNode = {
      id, label: 'New Node', parentId, expanded: false, active: true,
      transform: { x: 0, y: 0, z: 0 },
      components: componentPreset ? [componentPreset] : [],
    };
    setNodes((prev) => {
      const parentIdx = prev.findIndex((n) => n.id === parentId);
      const updated = prev.map((n) => n.id === parentId ? { ...n, expanded: true } : n);
      const next = [...updated];
      next.splice(parentIdx + 1, 0, newNode);
      return next;
    });
    setSelectedId(id);
    setTimeout(() => setRenamingId(id), 50);
  }, []);

  const deleteNode = (id: string) => {
    const toDelete = new Set<string>();
    const collect = (nid: string) => {
      toDelete.add(nid);
      nodes.filter((n) => n.parentId === nid).forEach((c) => collect(c.id));
    };
    collect(id);
    setNodes((prev) => prev.filter((n) => !toDelete.has(n.id)));
    if (selectedId && toDelete.has(selectedId)) setSelectedId(null);
  };

  const duplicateNode = (id: string) => {
    const src = nodes.find((n) => n.id === id);
    if (!src) return;
    const newId = genId();
    const copy: HNode = { ...src, id: newId, label: src.label + ' (copy)' };
    setNodes((prev) => {
      const idx = prev.findIndex((n) => n.id === id);
      const next = [...prev];
      next.splice(idx + 1, 0, copy);
      return next;
    });
    setSelectedId(newId);
  };

  const commitRename = (id: string, newLabel: string) => {
    setNodes((prev) => prev.map((n) => n.id === id ? { ...n, label: newLabel.trim() || n.label } : n));
    setRenamingId(null);
  };

  // ── inspector edits ────────────────────────────────────────────────────────
  const updateTransform = (id: string, axis: 'x' | 'y' | 'z', raw: string) => {
    const val = parseFloat(raw);
    if (Number.isNaN(val)) return;
    setNodes((prev) => prev.map((n) =>
      n.id === id ? { ...n, transform: { ...n.transform, [axis]: val } } : n,
    ));
  };

  const updateLabel = (id: string, label: string) =>
    setNodes((prev) => prev.map((n) => n.id === id ? { ...n, label } : n));

  // ── content browser ────────────────────────────────────────────────────────
  const [selectedFolder, setSelectedFolder]   = useState('all');
  const [selectedAssetId, setSelectedAssetId] = useState<string | null>(null);

  // ── tray ──────────────────────────────────────────────────────────────────
  const [trayTab, setTrayTab] = useState<TrayTab>('browser');
  const consoleLogs = [
    '[Info]  Editor initialized.',
    '[Info]  Scene "main.lescene" loaded.',
    '[Warn]  Nucleo VM: script recompile triggered.',
    '[Info]  EditorBridge connected.',
  ];

  // ── context menu ──────────────────────────────────────────────────────────
  const [ctx, setCtx] = useState<CtxState>(null);

  const openCtx = (e: ReactMouseEvent, items: ContextItem[]) => {
    e.preventDefault();
    e.stopPropagation();
    setCtx({ x: e.clientX, y: e.clientY, items });
  };

  const closeCtx = useCallback(() => setCtx(null), []);

  // ── menu bar ───────────────────────────────────────────────────────────────
  const [activeMenu, setActiveMenu] = useState<string | null>(null);

  // ── flattened visible tree ─────────────────────────────────────────────────
  const visibleNodes = useMemo(() => {
    const result: { node: HNode; depth: number }[] = [];
    const visit = (parentId: string | null, depth: number) => {
      nodes
        .filter((n) => n.parentId === parentId)
        .forEach((n) => {
          result.push({ node: n, depth });
          if (n.expanded) visit(n.id, depth + 1);
        });
    };
    visit(null, 0);
    return result;
  }, [nodes]);

  // ─────────────────────────────────────────────────────────────────────────
  // Context menu item builders
  // ─────────────────────────────────────────────────────────────────────────

  const tabCtxItems = (tab: EditorTab): ContextItem[] => [
    { label: 'Fechar',               onClick: () => closeTab(tab.id),         disabled: !tab.closable },
    { label: 'Fechar Outros',        onClick: () => closeOtherTabs(tab.id),   disabled: !tab.closable },
    { label: 'Fechar para Direita',  onClick: () => closeTabsToRight(tab.id), disabled: !tab.closable },
    { kind: 'separator' },
    { label: tab.pinned ? 'Desafixar Aba' : 'Fixar Aba',
      onClick: () => setTabs((p) => p.map((t) => t.id === tab.id ? { ...t, pinned: !t.pinned } : t)) },
  ];

  const nodeCtxItems = (node: HNode): ContextItem[] => [
    { kind: 'header', label: node.label },
    { label: 'Adicionar Filho',  icon: '+',  onClick: () => addNode(node.id) },
    { label: 'Duplicar',         icon: '⧉', shortcut: 'Ctrl+D', onClick: () => duplicateNode(node.id) },
    { label: 'Renomear',         icon: '✎', shortcut: 'F2',     onClick: () => setRenamingId(node.id) },
    { kind: 'separator' },
    { label: node.active ? 'Desativar' : 'Ativar', icon: '◉', onClick: () => toggleActive(node.id) },
    { kind: 'separator' },
    { label: 'Deletar', icon: '✕', danger: true, disabled: node.parentId === null,
      onClick: () => deleteNode(node.id) },
  ];

  const emptyHierCtxItems = (): ContextItem[] => [
    { kind: 'header', label: 'Adicionar Node' },
    { label: 'Node Vazio',       icon: '○', onClick: () => addNode('root') },
    { label: 'SpriteRenderer',   icon: '▣', onClick: () => addNode('root', 'SpriteRenderer') },
    { label: 'Camera 2D',        icon: '⊡', onClick: () => addNode('root', 'Camera2D') },
    { label: 'NucleoScript',     icon: '⬡', onClick: () => addNode('root', 'NucleoScript') },
    { label: 'Collider 2D',      icon: '⬟', onClick: () => addNode('root', 'Collider2D') },
  ];

  const cbAssetCtxItems = (assetId: string): ContextItem[] => {
    const asset = CB_ITEMS.find((a) => a.id === assetId);
    return [
      { kind: 'header', label: asset?.name ?? 'Asset' },
      { label: 'Abrir',                icon: '↗', onClick: () => {} },
      { label: 'Renomear',             icon: '✎', onClick: () => {} },
      { label: 'Duplicar',             icon: '⧉', onClick: () => {} },
      { kind: 'separator' },
      { label: 'Copiar Caminho',       icon: '📋', onClick: () => {} },
      { label: 'Mostrar no Explorer',  icon: '📂', onClick: () => {} },
      { kind: 'separator' },
      { label: 'Propriedades',         icon: 'ℹ',  onClick: () => {} },
      { kind: 'separator' },
      { label: 'Deletar', icon: '✕', danger: true, onClick: () => {} },
    ];
  };

  const cbEmptyCtxItems = (): ContextItem[] => [
    { label: 'Nova Pasta',       icon: '📁', onClick: () => {} },
    { label: 'Novo Script',      icon: '⬡', onClick: () => {} },
    { label: 'Nova Cena',        icon: '▣', onClick: () => {} },
    { kind: 'separator' },
    { label: 'Importar Arquivo', icon: '⬇', onClick: () => {} },
    { kind: 'separator' },
    { label: 'Atualizar',        icon: '↻', onClick: () => {} },
  ];

  const viewportCtxItems = (): ContextItem[] => [
    { kind: 'header', label: 'Viewport' },
    { label: 'Adicionar Node',     icon: '+',  onClick: () => addNode('root') },
    { kind: 'separator' },
    { label: 'Enquadrar Selecao',  icon: '⊡', shortcut: 'F',    onClick: () => {} },
    { label: 'Resetar Camera',     icon: '⌂', shortcut: 'Home', onClick: () => {} },
    { kind: 'separator' },
    { label: 'Colar',              icon: '📋', shortcut: 'Ctrl+V', onClick: () => {} },
  ];

  // ─────────────────────────────────────────────────────────────────────────
  // Render
  // ─────────────────────────────────────────────────────────────────────────

  return (
    <div className="editor-shell" onClick={() => { setActiveMenu(null); }}>

      {/* TOPBAR */}
      <header className="topbar">
        <div className="topbar-left">
          <div className="logo" aria-label="Lightning Engine">&#9889;</div>
          <nav className="tabs-strip" aria-label="Open tabs">
            {tabs.map((tab) => (
              <button
                key={tab.id}
                type="button"
                className={'editor-tab' + (tab.id === activeTabId ? ' active' : '') + (tab.pinned ? ' pinned' : '')}
                style={{ '--tab-accent': tab.accent } as CSSProperties}
                onClick={() => setActiveTabId(tab.id)}
                onContextMenu={(e) => openCtx(e, tabCtxItems(tab))}
              >
                {tab.pinned && <span className="tab-pin">&#128204;</span>}
                <span className="tab-label">{tab.label}</span>
                {tab.closable && !tab.pinned && (
                  <span
                    className="tab-close"
                    role="button"
                    tabIndex={0}
                    onClick={(e) => { e.stopPropagation(); closeTab(tab.id); }}
                    onKeyDown={(e: ReactKeyboardEvent) => {
                      if (e.key === 'Enter' || e.key === ' ') { e.preventDefault(); closeTab(tab.id); }
                    }}
                  >&#215;</span>
                )}
              </button>
            ))}
          </nav>
        </div>
        <span className="topbar-project">{status.project}</span>
      </header>

      {/* MENUBAR */}
      <nav className="menubar" aria-label="Editor menu" onClick={(e) => e.stopPropagation()}>
        {Object.entries(MENU_MODEL).map(([key, items]) => (
          <div key={key} className="menu-group">
            <button
              type="button"
              className={'menu-item' + (activeMenu === key ? ' active' : '')}
              onClick={() => setActiveMenu((p) => (p === key ? null : key))}
              onMouseEnter={() => { if (activeMenu) setActiveMenu(key); }}
            >{key}</button>
            {activeMenu === key && (
              <div className="menu-dropdown" role="menu">
                {items.map((item, i) =>
                  item.separator
                    ? <div key={i} className="ctx-sep" role="separator" />
                    : (
                      <button key={i} type="button" className="menu-dropdown-item" onClick={() => setActiveMenu(null)}>
                        <span>{item.label}</span>
                        {item.shortcut && <span className="ctx-shortcut">{item.shortcut}</span>}
                      </button>
                    )
                )}
              </div>
            )}
          </div>
        ))}
      </nav>

      {/* TOOLBAR */}
      <div className="toolbar">
        <button type="button" className="tool-btn" title="Settings">&#9881;</button>
        <div className="tool-sep" />
        <button
          type="button"
          className={'tool-btn' + (status.running ? ' running' : '')}
          title={status.running ? 'Stop' : 'Play'}
          onClick={toggleMotor}
          disabled={loadingMotor}
        >&#9654;</button>
        <button type="button" className="tool-btn" title="Pause">&#9208;</button>
        <button type="button" className="tool-btn" title="Step">&#9193;</button>
        <div className="tool-sep" />
        <button type="button" className="tool-btn" title="Build">&#128296;</button>
      </div>

      {/* DOCKSPACE */}
      <main className="dockspace">

        {/* Hierarchy */}
        <section className="panel panel-hierarchy">
          <div className="panel-header">
            <span className="panel-title">Hierarquia</span>
            <div className="panel-actions">
              <button type="button" className="panel-btn" title="Add Node" onClick={() => addNode('root')}>+</button>
            </div>
          </div>
          <div
            className="panel-body"
            onContextMenu={(e) => {
              if ((e.target as HTMLElement).closest('[data-nodeid]')) return;
              openCtx(e, emptyHierCtxItems());
            }}
          >
            {visibleNodes.map(({ node, depth }) => (
              <div
                key={node.id}
                data-nodeid={node.id}
                className={'hierarchy-item' + (selectedId === node.id ? ' selected' : '') + (!node.active ? ' inactive' : '')}
                style={{ paddingLeft: (8 + depth * 14) + 'px' }}
                onClick={() => setSelectedId(node.id)}
                onContextMenu={(e) => openCtx(e, nodeCtxItems(node))}
                onDoubleClick={() => setRenamingId(node.id)}
              >
                {nodes.some((c) => c.parentId === node.id) ? (
                  <button
                    type="button"
                    className="hier-arrow"
                    onClick={(e) => { e.stopPropagation(); toggleExpand(node.id); }}
                  >
                    {node.expanded ? '&#9662;' : '&#9656;'}
                  </button>
                ) : (
                  <span className="hier-arrow-spacer" />
                )}

                {renamingId === node.id ? (
                  <input
                    ref={renameRef}
                    className="hier-rename"
                    defaultValue={node.label}
                    autoFocus
                    onBlur={(e) => commitRename(node.id, e.target.value)}
                    onKeyDown={(e: ReactKeyboardEvent<HTMLInputElement>) => {
                      if (e.key === 'Enter')  commitRename(node.id, e.currentTarget.value);
                      if (e.key === 'Escape') setRenamingId(null);
                    }}
                    onClick={(e) => e.stopPropagation()}
                  />
                ) : (
                  <span className="hier-label">{node.label}</span>
                )}

                {node.components.length > 0 && (
                  <span className="hier-comp-dot" title={node.components.join(', ')} />
                )}
              </div>
            ))}
          </div>
        </section>

        {/* Viewport */}
        <section className="panel panel-viewport" onContextMenu={(e) => openCtx(e, viewportCtxItems())}>
          <div className="viewport-surface">
            <div className="viewport-gizmo-cube">
              <div className="gizmo-cube-face">
                <div className="gizmo-axis-x">X</div>
                <div className="gizmo-axis-y">Y</div>
                <div className="gizmo-axis-z">Z</div>
              </div>
            </div>
            <div className="viewport-top-left">
              <button type="button" className="viewport-settings-btn" title="Viewport Settings">&#9881;</button>
              <span className="viewport-mode-label">Scene2D</span>
            </div>
            <div className="viewport-grid" />
            <div className="viewport-watermark">Scene2D</div>
            <div className="viewport-gizmo-bar">
              <button type="button" className="gizmo-tool active" title="Mover">&#10021;</button>
              <button type="button" className="gizmo-tool" title="Rotacionar">&#8635;</button>
              <button type="button" className="gizmo-tool" title="Escalar">&#10548;</button>
            </div>
          </div>
        </section>

        {/* Inspector */}
        <section className="panel panel-properties">
          <div className="panel-header">
            <span className="panel-title">Inspector</span>
          </div>
          <div className="panel-body properties-body">
            {selectedNode ? (
              <>
                <div className="prop-section-title">Node</div>
                <div className="prop-row">
                  <span className="prop-label">Nome</span>
                  <input
                    type="text"
                    className="prop-input"
                    value={selectedNode.label}
                    onChange={(e) => updateLabel(selectedNode.id, e.target.value)}
                  />
                </div>
                <div className="prop-row">
                  <span className="prop-label">Ativo</span>
                  <input
                    type="checkbox"
                    className="prop-checkbox"
                    checked={selectedNode.active}
                    onChange={() => toggleActive(selectedNode.id)}
                  />
                </div>

                <div className="prop-section-title">Transform</div>
                {(['x', 'y', 'z'] as const).map((axis) => (
                  <div key={axis} className="prop-row">
                    <span className="prop-label axis-label" data-axis={axis}>{axis.toUpperCase()}</span>
                    <input
                      type="number"
                      className="prop-input"
                      value={selectedNode.transform[axis]}
                      onChange={(e) => updateTransform(selectedNode.id, axis, e.target.value)}
                    />
                  </div>
                ))}

                {selectedNode.components.length > 0 && (
                  <>
                    <div className="prop-section-title">Componentes</div>
                    {selectedNode.components.map((c) => (
                      <div key={c} className="prop-component">
                        <span className="comp-icon">&#11041;</span>
                        <span className="comp-name">{c}</span>
                      </div>
                    ))}
                  </>
                )}
              </>
            ) : (
              <div className="insp-empty">Nenhum node selecionado</div>
            )}
          </div>
        </section>

        {/* Bottom Tray */}
        <section className="panel panel-tray">
          <div className="panel-header tray-header">
            {(['browser', 'console', 'output'] as TrayTab[]).map((t) => (
              <button
                key={t}
                type="button"
                className={'tray-tab-btn' + (trayTab === t ? ' active' : '')}
                onClick={() => setTrayTab(t)}
              >
                {t === 'browser' ? 'Content Browser' : t === 'console' ? 'Console' : 'Output'}
              </button>
            ))}
          </div>

          {trayTab === 'browser' && (
            <div className="browser-body">
              <div className="browser-toolbar">
                <div className="browser-toolbar-icons">
                  <button type="button" title="Voltar">&#9665;</button>
                  <button type="button" title="Novo">&#8853;</button>
                  <button type="button" title="Importar">&#11015;</button>
                  <button type="button" title="Atualizar">&#8635;</button>
                </div>
                <div className="browser-path">
                  <span>&#128193;</span>
                  <span>Content &gt; {selectedFolder}</span>
                </div>
              </div>
              <div className="browser-content">
                <div className="browser-tree">
                  {CB_FOLDERS.map((f) => (
                    <button
                      key={f.id}
                      type="button"
                      className={'tree-item' + (f.id === selectedFolder ? ' selected' : '')}
                      onClick={() => setSelectedFolder(f.id)}
                    >
                      <span className="tree-dot" style={{ background: f.color }} />
                      {f.label}
                    </button>
                  ))}
                </div>
                <div
                  className="browser-assets"
                  onContextMenu={(e) => {
                    if ((e.target as HTMLElement).closest('[data-assetid]')) return;
                    openCtx(e, cbEmptyCtxItems());
                  }}
                >
                  {CB_ITEMS.map((a) => (
                    <div
                      key={a.id}
                      data-assetid={a.id}
                      className={'asset-card' + (selectedAssetId === a.id ? ' selected' : '')}
                      style={{ '--asset-border': a.color } as CSSProperties}
                      onClick={() => setSelectedAssetId(a.id)}
                      onContextMenu={(e) => { e.stopPropagation(); openCtx(e, cbAssetCtxItems(a.id)); }}
                    >
                      <div className="asset-thumb">
                        <span className="asset-type-badge" style={{ color: a.color }}>{a.type[0]}</span>
                      </div>
                      <div className="asset-info">
                        <span className="asset-name">{a.name}</span>
                        <span className="asset-sub">{a.type}</span>
                      </div>
                    </div>
                  ))}
                </div>
              </div>
            </div>
          )}

          {trayTab === 'console' && (
            <div className="console-body">
              {consoleLogs.map((line, i) => (
                <div
                  key={i}
                  className={'console-line' + (line.includes('[Warn]') ? ' warn' : line.includes('[Error]') ? ' error' : '')}
                >
                  {line}
                </div>
              ))}
            </div>
          )}

          {trayTab === 'output' && (
            <div className="console-body">
              <div className="console-line">Build output will appear here.</div>
            </div>
          )}
        </section>
      </main>

      {/* STATUS BAR */}
      <footer className="tray">
        <div className="tray-info">
          <span>Scene: {status.scene}</span>
          <span>FPS: {status.fps ?? 0}</span>
          <span className={status.running ? 'status-running' : 'status-stopped'}>
            Motor: {status.running ? 'Running' : 'Stopped'}
          </span>
          {selectedNode && <span>Sel: {selectedNode.label}</span>}
        </div>
        <span className="tray-icon">&#9889; Lightning Engine</span>
      </footer>

      {ctx && <ContextMenu x={ctx.x} y={ctx.y} items={ctx.items} onClose={closeCtx} />}
    </div>
  );
}
