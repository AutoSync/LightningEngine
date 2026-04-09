export interface MotorStatus {
  running: boolean;
  fps?: number;
  scene?: string;
  project?: string;
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

    return { running: true, fps: 60, scene: 'main_scene.lescene', project: 'LightningEngine Demo' };
  }

  async onStatusChange(callback: (status: MotorStatus) => void): Promise<() => void> {
    const event = await this.getEventApi();
    if (event) {
      const unlisten = await event.listen<MotorStatus>('motor-status', ({ payload }) => {
        callback(payload);
      });

      return unlisten;
    }

    callback({ running: true, fps: 60, scene: 'main_scene.lescene', project: 'LightningEngine Demo' });
    return () => undefined;
  }
}

export const tauriService = new TauriService();
