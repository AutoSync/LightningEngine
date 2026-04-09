import { useEffect, useMemo, useState, type CSSProperties } from 'react';
import { tauriService, type MotorStatus } from './services/tauri-service';

type TabType = 'project' | 'scene' | 'script' | 'texture' | 'shader' | 'material' | 'particle' | 'prefab' | 'mesh' | 'animation' | 'struct' | 'enum' | 'class' | 'gameobject' | 'settings';

type AssetTab = {
  id: string;
  label: string;
  type: TabType;
  accent: string;
  closable: boolean;
};

type MotorStatus = {
  running: boolean;
  fps: number;
  scene: string;
  project: string;
};

const tabs: AssetTab[] = [
  { id: 'project', label: 'Project', type: 'project', accent: '#f5f5f5', closable: false },
  { id: 'scene', label: 'Scene', type: 'scene', accent: '#7db1ff', closable: false },
  { id: 'script', label: 'Random', type: 'script', accent: '#ff9f40', closable: true },
  { id: 'texture', label: 'Textura', type: 'texture', accent: '#e55d5d', closable: true },
  { id: 'shader', label: 'Shaders', type: 'shader', accent: '#48be6a', closable: true },
  { id: 'material', label: 'Materiais', type: 'material', accent: '#3f9b58', closable: true },
  { id: 'particle', label: 'Particulas', type: 'particle', accent: '#a66eea', closable: true },
  { id: 'prefab', label: 'Prefabs', type: 'prefab', accent: '#e7c34a', closable: true },
  { id: 'mesh', label: 'Malha', type: 'mesh', accent: '#76c0ff', closable: true },
  { id: 'animation', label: 'Animacao', type: 'animation', accent: '#d98fc0', closable: true },
  { id: 'struct', label: 'Struct', type: 'struct', accent: '#c2c2cc', closable: true },
  { id: 'enum', label: 'Enum', type: 'enum', accent: '#7f7f90', closable: true },
  { id: 'class', label: 'Classes', type: 'class', accent: '#9270c6', closable: true },
  { id: 'gameobject', label: 'GameObjects', type: 'gameobject', accent: '#aeb4c4', closable: true },
  { id: 'settings', label: 'Configuracoes', type: 'settings', accent: '#9c9ca6', closable: true },
];

const statusSeed: MotorStatus = {
  running: true,
  fps: 60,
  scene: 'main_scene.lescene',
  project: 'LightningEngine Demo',
};

const hierarchyItems = ['Root', 'Player', 'Camera', 'Level Light', 'Enemy Spawn', 'UI Canvas'];
const contentItems = ['main_scene.lescene', 'player.prefab', 'atlas.png', 'terrain.mesh', 'movement.cs', 'glass.material'];
const inspectorFields = [
  { label: 'Name', value: 'Player' },
  { label: 'Active', value: 'true' },
  { label: 'Position', value: '12.0, 4.0, 2.0' },
  { label: 'Scale', value: '1.0, 1.0, 1.0' },
  { label: 'Speed', value: '6.5' },
];

export default function App() {
  const [activeTab, setActiveTab] = useState<AssetTab>(tabs[1]);
  const [status, setStatus] = useState(statusSeed);
  const [selectedAsset, setSelectedAsset] = useState(contentItems[0]);
  const [loadingMotor, setLoadingMotor] = useState(false);

  const visibleTabs = useMemo(() => tabs.filter((tab) => tab.closable || tab.id === activeTab.id || tab.id === 'project' || tab.id === 'scene'), [activeTab]);

  useEffect(() => {
    let unsubscribe = () => undefined;

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
      <header className="titlebar">
        <div>
          <div className="eyebrow">Lightning Engine</div>
          <h1>Editor React MVP</h1>
        </div>
        <div className="titlebar-actions">
          <button type="button" className="ghost-button">Save</button>
          <button type="button" className={status.running ? 'primary-button danger' : 'primary-button'} onClick={toggleMotor} disabled={loadingMotor}>
            {loadingMotor ? 'Working...' : status.running ? 'Pause Motor' : 'Start Motor'}
          </button>
        </div>
      </header>

      <nav className="menubar" aria-label="Main menu">
        {['File', 'Edit', 'Tools', 'Window', 'Help'].map((item) => (
          <button key={item} type="button" className="menu-item">{item}</button>
        ))}
      </nav>

      <section className="toolbar" aria-label="Quick actions">
        {['Save', 'Play', 'Compile', 'Scene Settings', 'Tools'].map((item) => (
          <button key={item} type="button" className="toolbar-button">{item}</button>
        ))}
      </section>

      <section className="tabs-row" aria-label="Context tabs">
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
            {tab.closable ? <span className="tab-close">x</span> : null}
          </button>
        ))}
      </section>

      <main className="workspace">
        <aside className="panel content-browser">
          <div className="panel-header">Content Browser</div>
          <div className="asset-list">
            {contentItems.map((asset) => (
              <button
                key={asset}
                type="button"
                className={`asset-item ${selectedAsset === asset ? 'selected' : ''}`}
                onClick={() => {
                  setSelectedAsset(asset);
                  if (asset.endsWith('.cs')) setActiveTab(tabs.find((tab) => tab.id === 'script') ?? tabs[0]);
                  if (asset.endsWith('.png')) setActiveTab(tabs.find((tab) => tab.id === 'texture') ?? tabs[0]);
                  if (asset.endsWith('.material')) setActiveTab(tabs.find((tab) => tab.id === 'material') ?? tabs[0]);
                }}
              >
                {asset}
              </button>
            ))}
          </div>
        </aside>

        <section className="center-stack">
          <div className="panel viewport">
            <div className="panel-header">Viewport</div>
            <div className="viewport-canvas">
              <div className="viewport-hud">
                <span>{status.project}</span>
                <span>{status.running ? 'Running' : 'Stopped'}</span>
                <span>{status.fps} fps</span>
              </div>
              <div className="viewport-stage">
                <div className="viewport-grid" />
                <div className="viewport-card">SDL3 embed target</div>
              </div>
            </div>
          </div>

          <div className="panel scene-dock">
            <div className="panel-grid">
              <div className="dock-panel">
                <div className="panel-header">Hierarchy</div>
                <div className="list-stack">
                  {hierarchyItems.map((item) => (
                    <div key={item} className="list-row">{item}</div>
                  ))}
                </div>
              </div>

              <div className="dock-panel">
                <div className="panel-header">Inspector</div>
                <div className="inspector-grid">
                  {inspectorFields.map((field) => (
                    <label key={field.label} className="field-row">
                      <span>{field.label}</span>
                      <input type="text" defaultValue={field.value} />
                    </label>
                  ))}
                  <div className="field-row">
                    <span>Selected Asset</span>
                    <input type="text" value={selectedAsset} readOnly />
                  </div>
                </div>
              </div>
            </div>
          </div>
        </section>
      </main>

      <footer className="statusbar">
        <span>Project: {status.project}</span>
        <span>Scene: {status.scene}</span>
        <span>Active Tab: {activeTab.label}</span>
        <span>Motor: {status.running ? 'Running' : 'Stopped'}</span>
      </footer>
    </div>
  );
}