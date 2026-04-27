import { useEffect, useRef, useState, type CSSProperties } from 'react';
import { createPortal } from 'react-dom';

export type ContextItem =
  | { kind?: 'item'; label: string; icon?: string; shortcut?: string; disabled?: boolean; danger?: boolean; onClick: () => void }
  | { kind: 'separator' }
  | { kind: 'header'; label: string };

interface Props {
  x: number;
  y: number;
  items: ContextItem[];
  onClose: () => void;
}

export function ContextMenu({ x, y, items, onClose }: Props) {
  const ref = useRef<HTMLDivElement>(null);
  const [pos, setPos] = useState<CSSProperties>({ left: x, top: y, opacity: 0 });

  /* Close on outside click or Escape */
  useEffect(() => {
    const onDown = (e: MouseEvent) => {
      if (ref.current && !ref.current.contains(e.target as Node)) onClose();
    };
    const onKey = (e: KeyboardEvent) => { if (e.key === 'Escape') onClose(); };
    document.addEventListener('mousedown', onDown);
    document.addEventListener('keydown', onKey);
    return () => {
      document.removeEventListener('mousedown', onDown);
      document.removeEventListener('keydown', onKey);
    };
  }, [onClose]);

  /* Clamp to viewport after first paint */
  useEffect(() => {
    if (!ref.current) return;
    const rect = ref.current.getBoundingClientRect();
    const vw = window.innerWidth;
    const vh = window.innerHeight;
    setPos({
      left: Math.min(x, vw - rect.width - 8),
      top:  Math.min(y, vh - rect.height - 8),
      opacity: 1,
    });
  }, [x, y]);

  if (typeof document === 'undefined') return null;

  return createPortal(
    <div ref={ref} className="ctx-menu" style={{ ...pos, position: 'fixed', zIndex: 9999 }} role="menu">
      {items.map((item, i) => {
        if (item.kind === 'separator') return <div key={i} className="ctx-sep" role="separator" />;
        if (item.kind === 'header')   return <div key={i} className="ctx-header">{item.label}</div>;
        return (
          <button
            key={i}
            type="button"
            role="menuitem"
            disabled={item.disabled}
            className={`ctx-item${item.danger ? ' danger' : ''}`}
            onClick={() => { item.onClick(); onClose(); }}
          >
            {item.icon && <span className="ctx-icon">{item.icon}</span>}
            <span className="ctx-label">{item.label}</span>
            {item.shortcut && <span className="ctx-shortcut">{item.shortcut}</span>}
          </button>
        );
      })}
    </div>,
    document.body,
  );
}
