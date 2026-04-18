import { useEffect, useMemo, useState, type CSSProperties } from 'react';
import { tauriService, type MotorStatus } from './services/tauri-service';

/* ── Tab types with semantic colors (design.md §3.2) ── */
type EditorTab = {
  id: string;
  label: string;
  accent: string;
  closable: boolean;
};

const TABS: EditorTab[] = [
  { id: 'scene', label: 'Scene', accent: '#62a7ff', closable: false },
  { id: 'project', label: 'Project', accent: '#f5f5f5', closable: true },
  { id: 'random', label: 'Random', accent: '#ff9a2d', closable: true },
  { id: 'texture', label: 'Textura', accent: '#ff6363', closable: true },
  { id: 'shader', label: 'Shaders', accent: '#57cf73', closable: true },
  { id: 'material', label: 'Materiais', accent: '#36b86f', closable: true },
  { id: 'particle', label: 'Particulas', accent: '#a66cff', closable: true },
  { id: 'prefab', label: 'Prefabs', accent: '#ffd34f', closable: true },
  { id: 'mesh', label: 'Malha', accent: '#70bbff', closable: true },
  { id: 'animation', label: 'Animacao', accent: '#ef8fd0', closable: true },
  { id: 'struct', label: 'Struct', accent: '#d7dbe5', closable: true },
  { id: 'enum', label: 'Enum', accent: '#9ca3b1', closable: true },
  { id: 'class', label: 'Classes', accent: '#8355d6', closable: true },
];

const MENU_MODEL: Record<string, string[]> = {
  FILE: ['New Scene', 'Open Project', 'Save', 'Save All', 'Exit'],
  EDIT: ['Undo', 'Redo', 'Duplicate', 'Delete'],
  TOOLS: ['Build', 'Compile Shaders', 'Profiler', 'Settings'],
  WINDOW: ['Viewport', 'Hierarchy', 'Properties', 'Content Browser'],
  HELP: ['Documentation', 'Roadmap', 'About'],
};

/* ── Hierarchy mock data ── */
const hierarchyItems = [
  { id: 'root', label: 'Root', depth: 0 },
];

/* ── Content Browser mock data ── */
type CBFolder = { id: string; label: string; color: string };
const cbFolders: CBFolder[] = [
  { id: 'all', label: 'All', color: '#f5a623' },
  { id: 'player', label: 'Player', color: '#4cd964' },
  { id: 'enemies', label: 'Enemies', color: '#4cd964' },
  { id: 'levels', label: 'Levels', color: '#f5a623' },
  { id: 'textures', label: 'Textures', color: '#f5a623' },
];

type CBFilter = { id: string; label: string; color: string };
const cbFilters: CBFilter[] = [
  { id: 'static-meshes', label: 'Static Meshes', color: '#70bbff' },
  { id: 'textures', label: 'Textures', color: '#ff6363' },
  { id: 'prefabs', label: 'Prefabs', color: '#a66cff' },
];

type CBAsset = { id: string; name: string; subtitle: string; borderColor: string };
const cbAssets: CBAsset[] = [
  { id: 'playermaster', name: 'PlayerMaster', subtitle: 'CharacterMovem...', borderColor: '#62a7ff' },
  { id: 'enemies', name: 'enemies', subtitle: 'ignite', borderColor: '#ff6363' },
  { id: 'level-spawner', name: 'level_spawner', subtitle: 'csharp', borderColor: '#a66cff' },
];

/* ── Inspector / Properties mock ── */
const inspectorFields = [
  { label: 'Name', value: 'Player' },
  { label: 'Transform', values: [
    { axis: 'X', val: '200.0', color: '#ff4444' },
    { axis: 'Y', val: '200.0', color: '#44cc44' },
    { axis: 'Z', val: '0', color: '#4488ff' },
  ]},
  { label: 'IsActive', checked: true },
];

const statusSeed: MotorStatus = {
  running: false,
  fps: 60,
  scene: 'main_scene.lescene',
  project: 'MyProject',
  lastChange: '',
};

export default function App() {
  const [openTabs, setOpenTabs] = useState<EditorTab[]>(TABS);
  const [activeTab, setActiveTab] = useState<EditorTab>(TABS[0]);
  const [activeMenu, setActiveMenu] = useState<string | null>(null);
  const [status, setStatus] = useState(statusSeed);
  const [loadingMotor, setLoadingMotor] = useState(false);
  const [selectedFolder, setSelectedFolder] = useState('player');
  const [collapsedPanels, setCollapsedPanels] = useState({
    hierarchy: false,
    properties: false,
    browser: false,
  });

  const effectiveTabs = useMemo(
    () => (openTabs.length ? openTabs : [TABS[0]]),
    [openTabs],
  );

  useEffect(() => {
    if (!effectiveTabs.some((tab) => tab.id === activeTab.id)) {
      setActiveTab(effectiveTabs[0]);
    }
  }, [activeTab.id, effectiveTabs]);

  useEffect(() => {
    let unsubscribe: () => void = () => {};
    tauriService.getStatus().then(setStatus).catch(() => undefined);
    tauriService.onStatusChange((s) => setStatus(s)).then((d) => { unsubscribe = d; });
    return () => { unsubscribe(); };
  }, []);

  const toggleMotor = async () => {
    setLoadingMotor(true);
    try {
      if (status.running) await tauriService.stopMotor();
      else await tauriService.startMotor();
    } finally {
      setLoadingMotor(false);
    }
  };

  const togglePanel = (panel: 'hierarchy' | 'properties' | 'browser') => {
    setCollapsedPanels((prev) => ({ ...prev, [panel]: !prev[panel] }));
  };

  const closeTab = (tabId: string) => {
    setOpenTabs((prev) => {
      const next = prev.filter((tab) => tab.id !== tabId || !tab.closable);
      return next.length ? next : [TABS[0]];
    });
  };

  return (
    <div className="editor-shell">
      {/* ═══ TOPBAR: logo + tabs + project name ═══ */}
      <header className="topbar">
        <div className="topbar-left">
          <div className="logo" aria-label="Lightning Engine">⚡</div>
          <nav className="tabs-strip" aria-label="Open tabs">
            {effectiveTabs.map((tab) => (
              <button
                key={tab.id}
                type="button"
                className={tab.id === activeTab.id ? 'editor-tab active' : 'editor-tab'}
                style={{ '--tab-accent': tab.accent } as CSSProperties}
                onClick={() => setActiveTab(tab)}
              >
                <span className="tab-label">{tab.label}</span>
                {tab.closable && (
                  <span
                    className="tab-close"
                    role="button"
                    tabIndex={0}
                    onClick={(event) => {
                      event.stopPropagation();
                      closeTab(tab.id);
                    }}
                    onKeyDown={(event) => {
                      if (event.key === 'Enter' || event.key === ' ') {
                        event.preventDefault();
                        closeTab(tab.id);
                      }
                    }}
                  >
                    ×
                  </span>
                )}
              </button>
            ))}
          </nav>
        </div>
        <span className="topbar-project">{status.project}</span>
      </header>

      {/* ═══ MENUBAR ═══ */}
      <nav className="menubar" aria-label="Editor menu">
        {Object.keys(MENU_MODEL).map((m) => (
          <div
            key={m}
            className="menu-group"
            onMouseEnter={() => setActiveMenu(m)}
            onMouseLeave={() => setActiveMenu((prev) => (prev === m ? null : prev))}
          >
            <button
              type="button"
              className={activeMenu === m ? 'menu-item active' : 'menu-item'}
              onClick={() => setActiveMenu((prev) => (prev === m ? null : m))}
            >
              {m}
            </button>
            {activeMenu === m ? (
              <div className="menu-dropdown" role="menu" aria-label={`${m} options`}>
                {MENU_MODEL[m].map((item) => (
                  <button key={item} type="button" className="menu-dropdown-item">
                    {item}
                  </button>
                ))}
              </div>
            ) : null}
          </div>
        ))}
      </nav>

      {/* ═══ TOOLBAR ═══ */}
      <div className="toolbar">
        <button type="button" className="tool-btn" title="Scene Settings">⚙</button>
        <button
          type="button"
          className={`tool-btn ${status.running ? 'running' : ''}`}
          title={status.running ? 'Stop' : 'Play'}
          onClick={toggleMotor}
          disabled={loadingMotor}
        >▶</button>
        <button type="button" className="tool-btn" title="Compile">⛭</button>
      </div>

      {/* ═══ DOCKSPACE ═══ */}
      <main className="dockspace">
        {/* ── Hierarchy (left) ── */}
        <section className="panel panel-hierarchy">
          <div className="panel-header">
            <span className="panel-drag">⋮</span>
            <span className="panel-title">Root</span>
            <div className="panel-actions">
              <button
                type="button"
                className="panel-btn"
                title="Minimize"
                onClick={() => togglePanel('hierarchy')}
              >
                —
              </button>
              <button type="button" className="panel-btn" title="Detach">●</button>
            </div>
          </div>
          <div className={collapsedPanels.hierarchy ? 'panel-body hidden' : 'panel-body'}>
            {hierarchyItems.map((item) => (
              <div key={item.id} className="hierarchy-item" style={{ paddingLeft: `${8 + item.depth * 16}px` }}>
                {item.label}
              </div>
            ))}
          </div>
        </section>

        {/* ── Viewport (center) ── */}
        <section className="panel panel-viewport">
          <div className="viewport-surface">
            {/* Gizmo cube */}
            <div className="viewport-gizmo-cube">
              <div className="gizmo-cube-face">
                <div className="gizmo-axis-x">X</div>
                <div className="gizmo-axis-y">Y</div>
                <div className="gizmo-axis-z">Z</div>
              </div>
              <div className="gizmo-label-line">
                <span className="gizmo-label cyan">ILUMINADO</span>
              </div>
              <div className="gizmo-label-line">
                <span>ORTHO - </span>
                <span className="gizmo-label cyan">PESPEC</span>
              </div>
            </div>
            {/* Viewport settings */}
            <div className="viewport-top-left">
              <span className="viewport-dot">●</span>
              <button type="button" className="viewport-settings-btn" title="Viewport settings">⚙</button>
            </div>
            {/* Grid checkered */}
            <div className="viewport-grid" />
            {/* Watermark */}
            <div className="viewport-watermark">Scene2D</div>
            {/* Gizmo toolbar */}
            <div className="viewport-gizmo-bar">
              <button type="button" className="gizmo-tool active" title="Move">✥</button>
              <button type="button" className="gizmo-tool" title="Rotate">↻</button>
              <button type="button" className="gizmo-tool" title="Scale">⤴</button>
              <button type="button" className="gizmo-tool" title="More">›</button>
            </div>
          </div>
        </section>

        {/* ── Properties (right) ── */}
        <section className="panel panel-properties">
          <div className="panel-header">
            <span className="panel-drag">⋮</span>
            <span className="panel-title">Properties</span>
            <div className="panel-actions">
              <button
                type="button"
                className="panel-btn"
                title="Minimize"
                onClick={() => togglePanel('properties')}
              >
                —
              </button>
              <button type="button" className="panel-btn" title="Detach">●</button>
            </div>
          </div>
          <div className={collapsedPanels.properties ? 'panel-body properties-body hidden' : 'panel-body properties-body'}>
            {inspectorFields.map((field) => {
              if (field.values) {
                return (
                  <div key={field.label} className="prop-row">
                    <span className="prop-label">{field.label}:</span>
                    <div className="prop-transform">
                      {field.values.map((v) => (
                        <label key={v.axis} className="transform-field">
                          <span style={{ color: v.color }}>{v.axis}</span>
                          <input type="text" defaultValue={v.val} readOnly />
                        </label>
                      ))}
                      <button type="button" className="transform-reset" title="Reset">↺</button>
                    </div>
                  </div>
                );
              }
              if (field.checked !== undefined) {
                return (
                  <div key={field.label} className="prop-row">
                    <span className="prop-label">{field.label}:</span>
                    <input type="checkbox" checked={field.checked} readOnly className="prop-checkbox" />
                  </div>
                );
              }
              return (
                <div key={field.label} className="prop-row">
                  <span className="prop-label">{field.label}:</span>
                  <input type="text" defaultValue={field.value} readOnly className="prop-input" />
                </div>
              );
            })}
          </div>
        </section>

        {/* ── Content Browser (bottom) ── */}
        <section className="panel panel-browser">
          <div className="panel-header">
            <span className="panel-drag">⋮</span>
            <span className="panel-title">Content Browser</span>
            <div className="panel-actions">
              <button
                type="button"
                className="panel-btn"
                title="Minimize"
                onClick={() => togglePanel('browser')}
              >
                —
              </button>
              <button type="button" className="panel-btn" title="Detach">●</button>
            </div>
          </div>
          <div className={collapsedPanels.browser ? 'browser-body hidden' : 'browser-body'}>
            {/* Browser toolbar */}
            <div className="browser-toolbar">
              <div className="browser-toolbar-icons">
                <button type="button" title="Back">⊙</button>
                <button type="button" title="Forward">⊙</button>
                <button type="button" title="Add">⊕</button>
                <button type="button" title="Import">⬇</button>
              </div>
              <div className="browser-path">
                <span>📁</span>
                <span>Content &gt;</span>
              </div>
            </div>
            <div className="browser-content">
              {/* Folder tree */}
              <div className="browser-tree">
                <div className="tree-section">
                  <div className="tree-header">Favorites <span>▼</span></div>
                </div>
                <div className="tree-section">
                  <div className="tree-header">My Project <span>▼</span></div>
                  {cbFolders.map((f) => (
                    <button
                      key={f.id}
                      type="button"
                      className={`tree-item ${f.id === selectedFolder ? 'selected' : ''}`}
                      onClick={() => setSelectedFolder(f.id)}
                    >
                      <span className="tree-dot" style={{ background: f.color }} />
                      {f.label}
                    </button>
                  ))}
                </div>
                <div className="tree-section">
                  <div className="tree-header">Collections <span>▼</span></div>
                </div>
              </div>
              {/* Filters */}
              <div className="browser-filters">
                <div className="filters-header">Filters <span className="filters-collapse">‹</span></div>
                {cbFilters.map((f) => (
                  <div key={f.id} className="filter-tag">
                    <span className="filter-dot" style={{ background: f.color }} />
                    <span>{f.label}</span>
                    <button type="button" className="filter-remove">×</button>
                  </div>
                ))}
              </div>
              {/* Asset grid */}
              <div className="browser-assets">
                {cbAssets.map((a) => (
                  <div key={a.id} className="asset-card" style={{ '--asset-border': a.borderColor } as CSSProperties}>
                    <div className="asset-thumb" />
                    <div className="asset-info">
                      <span className="asset-name">{a.name}</span>
                      <span className="asset-sub">{a.subtitle}</span>
                    </div>
                  </div>
                ))}
              </div>
            </div>
          </div>
        </section>
      </main>

      {/* ═══ TRAY / STATUS BAR ═══ */}
      <footer className="tray">
        <div className="tray-info">
          <span>Scene: {status.scene}</span>
          <span>FPS: {status.fps ?? 0}</span>
          <span>Motor: {status.running ? 'Running' : 'Stopped'}</span>
          <span>Tab: {activeTab.label}</span>
        </div>
        <span className="tray-icon" title="Collections">🔖</span>
      </footer>
    </div>
  );
}