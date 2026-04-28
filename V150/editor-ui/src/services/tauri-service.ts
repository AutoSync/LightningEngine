export interface MotorStatus {
  running: boolean;
  fps?: number;
  scene?: string;
  project?: string;
  lastChange?: string;
}

// ── Scene contract (mirrors EditorBridge.h GetSceneJson schema) ──────────
export interface SceneNodeTransform {
  px: number; py: number; pz: number;
  rx: number; ry: number; rz: number;
  sx: number; sy: number; sz: number;
}

export interface SceneNode {
  id: string;
  parentId: string | null;
  name: string;
  tag: string;
  active: boolean;
  components: string[];
  transform: SceneNodeTransform;
}

export interface SceneSnapshot {
  scene: string;
  selectedId: string | null;
  nodes: SceneNode[];
}

export interface CommandResult {
  ok: boolean;
  id?: string;
  error?: string;
}

class TauriService {
  private async getCoreApi() {
    try {
      return await import('@tauri-apps/api/core');
    } catch {
      return null;
    }
  }

  private async getEventApi() {
    try {
      return await import('@tauri-apps/api/event');
    } catch {
      return null;
    }
  }

  // ── true if the desktop bridge is reachable ─────────────────────────────
  async isBridgeAvailable(): Promise<boolean> {
    const core = await this.getCoreApi();
    return core !== null;
  }

  async startMotor(): Promise<void> {
    const core = await this.getCoreApi();
    if (core) {
      await core.invoke('start_motor');
    }
  }

  async stopMotor(): Promise<void> {
    const core = await this.getCoreApi();
    if (core) {
      await core.invoke('stop_motor');
    }
  }

  async getStatus(): Promise<MotorStatus> {
    const core = await this.getCoreApi();
    if (core) {
      return core.invoke<MotorStatus>('get_motor_status');
    }

    return {
      running: true,
      fps: 60,
      scene: 'main_scene.lescene',
      project: 'LightningEngine Demo',
      lastChange: 'Waiting for C++ bridge',
    };
  }

  async onStatusChange(callback: (status: MotorStatus) => void): Promise<() => void> {
    const event = await this.getEventApi();
    if (event) {
      const unlisten = await event.listen<MotorStatus>('motor-status', ({ payload }) => {
        callback(payload);
      });

      return unlisten;
    }

    callback({
      running: true,
      fps: 60,
      scene: 'main_scene.lescene',
      project: 'LightningEngine Demo',
      lastChange: 'Waiting for C++ bridge',
    });
    return () => undefined;
  }

  // ─────────────────────────────────────────────────────────────────────
  // Scene contract — these mirror EditorBridge.h. Until the Rust host
  // implements `get_scene` / `dispatch_command`, the calls fall through
  // to `null` / a refusal result, and the UI must keep its mock state.
  // ─────────────────────────────────────────────────────────────────────

  async getScene(): Promise<SceneSnapshot | null> {
    const core = await this.getCoreApi();
    if (!core) return null;
    try {
      return await core.invoke<SceneSnapshot>('get_scene');
    } catch {
      return null;
    }
  }

  async onSceneChange(callback: (snapshot: SceneSnapshot) => void): Promise<() => void> {
    const event = await this.getEventApi();
    if (!event) return () => undefined;
    try {
      const unlisten = await event.listen<SceneSnapshot>('scene-snapshot', ({ payload }) => {
        callback(payload);
      });
      return unlisten;
    } catch {
      return () => undefined;
    }
  }

  async dispatchCommand(command: object): Promise<CommandResult> {
    const core = await this.getCoreApi();
    if (!core) return { ok: false, error: 'bridge-unavailable' };
    try {
      return await core.invoke<CommandResult>('dispatch_command', { command });
    } catch (err) {
      return { ok: false, error: String(err) };
    }
  }

  selectNode(id: string)        { return this.dispatchCommand({ op: 'select', id }); }
  deselect()                     { return this.dispatchCommand({ op: 'deselect' }); }
  setNodeTransform(id: string, p: { px?: number; py?: number; pz?: number }) {
    return this.dispatchCommand({ op: 'setTransform', id, ...p });
  }
  createNode(parentId: string | null, name: string, archetype: string) {
    return this.dispatchCommand({ op: 'createNode', parentId: parentId ?? '', name, archetype });
  }
  deleteNode(id: string)         { return this.dispatchCommand({ op: 'deleteNode', id }); }
}

export const tauriService = new TauriService();
