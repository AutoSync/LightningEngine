import { useEffect, useState } from 'react';
import { tauriService } from '../services/tauri-service';
import type { SceneSnapshot } from '../services/tauri-service';

/**
 * useEngineScene
 *
 * Tries to read the live scene from the C++ EditorBridge through Tauri.
 * Returns:
 *   { snapshot: null,  source: 'unavailable' } when the bridge cannot be reached.
 *   { snapshot: <obj>, source: 'engine'      } when the engine answered.
 *
 * The UI keeps full responsibility for falling back to its mock state when
 * source !== 'engine'. This hook never fabricates engine data.
 */
export function useEngineScene(pollIntervalMs = 1000) {
  const [snapshot, setSnapshot] = useState<SceneSnapshot | null>(null);
  const [source, setSource] = useState<'engine' | 'unavailable' | 'pending'>('pending');

  useEffect(() => {
    let cancelled = false;
    let timer: ReturnType<typeof setInterval> | null = null;

    const tick = async () => {
      const snap = await tauriService.getScene();
      if (cancelled) return;
      if (snap && Array.isArray(snap.nodes)) {
        setSnapshot(snap);
        setSource('engine');
      } else {
        setSnapshot(null);
        setSource('unavailable');
      }
    };

    void tick();
    timer = setInterval(tick, pollIntervalMs);

    let unlistenEvent: (() => void) | undefined;
    void tauriService.onSceneChange((s) => {
      if (cancelled) return;
      if (s && Array.isArray(s.nodes)) {
        setSnapshot(s);
        setSource('engine');
      }
    }).then((u) => { unlistenEvent = u; });

    return () => {
      cancelled = true;
      if (timer) clearInterval(timer);
      if (unlistenEvent) unlistenEvent();
    };
  }, [pollIntervalMs]);

  return { snapshot, source };
}
