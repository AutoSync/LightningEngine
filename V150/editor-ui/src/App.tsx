import { useEffect, useMemo, useState, type CSSProperties } from 'react';
import { tauriService, type MotorStatus } from './services/tauri-service';

type TabType = 'project' | 'scene' | 'assets' | 'materials' | 'styles' | 'camera' | 'inspector' | 'animation';

type AssetTab = {
  id: string;
  label: string;
  type: TabType;
  accent: string;
  closable: boolean;
};

const tabs: AssetTab[] = [
  { id: 'project', label: '3D Boy Character', type: 'project', accent: '#111827', closable: false },
  { id: 'scene', label: 'Scene', type: 'scene', accent: '#2563eb', closable: false },
  { id: 'assets', label: 'Assets', type: 'assets', accent: '#8b5cf6', closable: false },
  { id: 'materials', label: 'Materials', type: 'materials', accent: '#b45309', closable: true },
  { id: 'styles', label: 'Styles', type: 'styles', accent: '#0f766e', closable: true },
  { id: 'camera', label: 'Camera', type: 'camera', accent: '#475569', closable: true },
  { id: 'inspector', label: 'Inspector', type: 'inspector', accent: '#6366f1', closable: true },
  { id: 'animation', label: 'Animation', type: 'animation', accent: '#dc2626', closable: true },
];

const statusSeed: MotorStatus = {
  running: true,
  fps: 60,
  scene: 'studio_scene.lescene',
  project: '3D Boy Character',
};

const sceneItems = [
  { label: 'Camera 1', icon: '◌' },
  { label: 'Dome Light', icon: '☼' },
  { label: 'Key Light', icon: '⌖' },
  { label: 'Area Light', icon: '✦' },
  { label: 'Object 2', icon: '⬚', active: true },
  { label: 'Background 2', icon: '⬡' },
  { label: 'Character', icon: '◉' },
  { label: 'Background 1', icon: '⬟' },
];

const materialItems = ['F4F4F4', 'D6A08A', '3F3F44', '1F1F22'];
const styleItems = ['Warm Studio', 'Sunset Mood'];
const inspectorFields = [
  { label: 'X', value: '12.0' },
  { label: 'Y', value: '4.0' },
  { label: 'Z', value: '2.0' },
  { label: 'Scale', value: '1.0' },
  { label: 'Opacity', value: '100%' },
];

export default function App() {
  const [activeTab, setActiveTab] = useState<AssetTab>(tabs[1]);
  const [status, setStatus] = useState(statusSeed);
  const [selectedItem, setSelectedItem] = useState(sceneItems[4].label);
  const [loadingMotor, setLoadingMotor] = useState(false);

  const visibleTabs = useMemo(() => tabs, []);

  useEffect(() => {
    let unsubscribe: () => void = () => {};

    tauriService.getStatus().then(setStatus).catch(() => undefined);
    tauriService.onStatusChange((nextStatus) => setStatus(nextStatus)).then((dispose) => {
      unsubscribe = dispose;
    });

    return () => {
      unsubscribe();
    };
  }, []);

  const toggleMotor = async () => {
    setLoadingMotor(true);
    try {
      if (status.running) {
        await tauriService.stopMotor();
      } else {
        await tauriService.startMotor();
      }
    } finally {
      setLoadingMotor(false);
    }
  };

  return (
    <div className="editor-shell">
      <div className="shell-frame">
        <header className="titlebar">
          <div className="project-meta">
            <div className="brand-mark" aria-hidden="true">◇</div>
            <div>
              <div className="project-name">{status.project}</div>
              <div className="project-subtitle">3D Design Project</div>
            </div>
          </div>

          <div className="top-controls" aria-label="Viewport controls">
            {['↖', '⟲', '▢', '◫', '▶'].map((symbol) => (
              <button key={symbol} type="button" className="icon-button" aria-label={symbol}>
                {symbol}
              </button>
            ))}
            <div className="zoom-chip">100%</div>
            <button type="button" className="arrow-button" aria-label="Undo">←</button>
            <button type="button" className="arrow-button" aria-label="Redo">→</button>
            <button type="button" className="export-button">Export</button>
          </div>

          <div className="titlebar-actions">
            <div className="avatar-stack" aria-label="Collaborators">
              <span className="avatar avatar-a" />
              <span className="avatar avatar-b" />
            </div>
            <button type="button" className="share-button">Share</button>
          </div>
        </header>

        <nav className="tabs-row" aria-label="Context tabs">
          {visibleTabs.map((tab) => (
            <button
              key={tab.id}
              type="button"
              className={`tab-pill ${activeTab.id === tab.id ? 'active' : ''}`}
              style={{ '--tab-accent': tab.accent } as CSSProperties}
              onClick={() => setActiveTab(tab)}
            >
              <span className="tab-dot" />
              {tab.label}
              {tab.closable ? <span className="tab-close">×</span> : null}
            </button>
          ))}
        </nav>

        <div className="menu-row">
          {['File', 'Edit', 'View', 'GameObject', 'Component', 'Window', 'Help'].map((item) => (
            <button key={item} type="button" className="menu-item">{item}</button>
          ))}
          <div className="menu-spacer" />
          <button type="button" className={status.running ? 'play-pill running' : 'play-pill'} onClick={toggleMotor} disabled={loadingMotor}>
            {loadingMotor ? '...' : status.running ? 'Pause' : 'Play'}
          </button>
        </div>

        <section className="toolbar" aria-label="Quick actions">
          {['□', '○', '△', '⌂', '▣'].map((item) => (
            <button key={item} type="button" className="toolbar-button">{item}</button>
          ))}
          <div className="toolbar-divider" />
          <span className="toolbar-note">Scene mode</span>
        </section>

        <main className="workspace">
          <aside className="panel sidebar-left">
            <div className="sidebar-head">
              <div>
                <div className="sidebar-title">{status.project}</div>
                <div className="sidebar-subtitle">3D Design Project</div>
              </div>
              <button type="button" className="small-icon-button">▣</button>
            </div>

            <div className="segmented-control" aria-label="Scene tabs">
              <button type="button" className="segmented-button active">Scene</button>
              <button type="button" className="segmented-button">Assets</button>
            </div>

            <div className="scene-list">
              {sceneItems.map((item) => (
                <button
                  key={item.label}
                  type="button"
                  className={`scene-row ${selectedItem === item.label ? 'active' : ''}`}
                  onClick={() => setSelectedItem(item.label)}
                >
                  <span className="scene-icon">{item.icon}</span>
                  <span className="scene-label">{item.label}</span>
                  <span className="scene-actions">⌂ ◎</span>
                </button>
              ))}
            </div>

            <div className="search-box">
              <span>⌕</span>
              <input type="text" value="Search..." readOnly />
              <kbd>⌘K</kbd>
            </div>
          </aside>

          <section className="stage-column">
            <div className="panel viewport">
              <div className="viewport-stage">
                <div className="viewport-grid" />
                <div className="viewport-canvas">
                  <div className="stage-toolbar" aria-label="Viewport toolbar">
                    {['◻', '⟲', '◌', '◩'].map((item) => (
                      <button key={item} type="button" className="icon-button small">{item}</button>
                    ))}
                    <span className="stage-toolbar-label">{status.running ? 'Live' : 'Idle'}</span>
                  </div>

                  <div className="viewport-figure">
                    <div className="figure-head" />
                    <div className="figure-glasses">
                      <span />
                      <span />
                    </div>
                    <div className="figure-body" />
                    <div className="figure-bag" />
                    <div className="figure-shadow" />
                  </div>

                  <div className="stage-caption">
                    <div>
                      <strong>Brainwave 2.5</strong>
                      <span>Scene view</span>
                    </div>
                    <button type="button" className="caption-button">Inspiration ▾</button>
                  </div>
                </div>
              </div>
            </div>

            <div className="bottom-toolbar">
              <button type="button" className="bottom-action">＋</button>
              <div className="bottom-cta">
                <span className="cta-label">Add photos or videos</span>
                <button type="button" className="cta-pill">Add 3D objects</button>
                <button type="button" className="cta-pill secondary">Add files</button>
              </div>
              <div className="bottom-prompt">Create something...</div>
              <button type="button" className="bottom-submit">↑</button>
            </div>
          </section>

          <aside className="panel sidebar-right">
            <div className="sidebar-right-top">
              <div className="avatar-stack">
                <span className="avatar avatar-a" />
                <span className="avatar avatar-b" />
              </div>
              <button type="button" className="share-button compact">Share</button>
            </div>

            <div className="segmented-control right-segmented">
              <button type="button" className="segmented-button active">Design</button>
              <button type="button" className="segmented-button">Animation</button>
            </div>

            <section className="right-section">
              <div className="section-header">
                <h2>Materials</h2>
                <button type="button" className="plus-button">+</button>
              </div>
              <div className="material-grid">
                {materialItems.map((item, index) => (
                  <button key={item} type="button" className={`material-swatch swatch-${index + 1}`} aria-label={`Material ${item}`}>
                    <span>{item}</span>
                  </button>
                ))}
              </div>
            </section>

            <section className="right-section">
              <div className="section-header">
                <h2>Styles</h2>
                <button type="button" className="plus-button">+</button>
              </div>
              <div className="style-grid">
                {styleItems.map((item) => (
                  <div key={item} className="style-card">
                    <div className="style-preview" />
                    <span>{item}</span>
                  </div>
                ))}
              </div>
            </section>

            <section className="right-section compact-grid">
              <div className="section-header">
                <h2>Backgrounds</h2>
                <button type="button" className="plus-button">+</button>
              </div>
              <div className="background-row">
                <div className="background-field">F4F4F4</div>
                <div className="background-field percent">100 %</div>
              </div>
            </section>

            <section className="right-section compact-grid">
              <div className="section-header">
                <h2>Camera</h2>
                <button type="button" className="plus-button">+</button>
              </div>
              <div className="segmented-control camera-control">
                <button type="button" className="segmented-button active">Isometric</button>
                <button type="button" className="segmented-button">Perspective</button>
              </div>
              <div className="slider-card">
                <span>Distortion</span>
                <div className="slider-track">
                  <div className="slider-fill" />
                </div>
                <div className="slider-value">0.283</div>
              </div>
            </section>

            <section className="right-section status-block">
              <div className="section-header">
                <h2>Inspector</h2>
              </div>
              <div className="inspector-grid">
                {inspectorFields.map((field) => (
                  <label key={field.label} className="field-row">
                    <span>{field.label}</span>
                    <input type="text" defaultValue={field.value} />
                  </label>
                ))}
                <label className="field-row">
                  <span>Selected Item</span>
                  <input type="text" value={selectedItem} readOnly />
                </label>
              </div>
            </section>
          </aside>
        </main>

        <footer className="statusbar">
          <span>Scene: {status.scene}</span>
          <span>Motor: {status.running ? 'Running' : 'Stopped'}</span>
          <span>FPS: {status.fps}</span>
        </footer>
      </div>
    </div>
  );
}