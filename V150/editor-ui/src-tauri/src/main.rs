use serde::{Deserialize, Serialize};
use std::{fs, io::Write, sync::Mutex, thread, time::Duration};
use tauri::{AppHandle, Emitter, Manager, State};

#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
struct MotorStatus {
    running: bool,
    fps: u32,
    scene: String,
    project: String,
    last_change: String,
}

impl PartialEq for MotorStatus {
    fn eq(&self, other: &Self) -> bool {
        self.running == other.running
            && self.fps == other.fps
            && self.scene == other.scene
            && self.project == other.project
            && self.last_change == other.last_change
    }
}

impl Default for MotorStatus {
    fn default() -> Self {
        Self {
            running: false,
            fps: 60,
            scene: String::from("main_scene.lescene"),
            project: String::from("LightningEngine Demo"),
            last_change: String::from("Waiting for C++ bridge"),
        }
    }
}

struct AppState {
    motor: Mutex<MotorStatus>,
}

fn emit_status(app: &AppHandle, status: &MotorStatus) -> Result<(), String> {
    app.emit("motor-status", status.clone())
        .map_err(|error| error.to_string())
}

// ─── File-IPC paths shared with the C++ EditorBridge ───────────────────
//
// The engine writes editor-bridge-*.json/.jsonl into its working dir
// (the V150 repo root's x64/Debug or x64/Release). When `tauri:dev`
// runs, the Rust process CWD lives somewhere under editor-ui/src-tauri/
// (often `editor-ui/src-tauri/target/debug/`). We can't trust `cwd`
// alone, so we anchor on `CARGO_MANIFEST_DIR` (= editor-ui/src-tauri)
// and walk up to find `x64/Debug` or `x64/Release`.

const COMMAND_FILE_NAME: &str = "editor-bridge-commands.jsonl";
const STATUS_FILE_NAME: &str = "editor-bridge-status.json";
const SCENE_FILE_NAME: &str = "editor-bridge-scene.json";

fn engine_output_dirs() -> Vec<std::path::PathBuf> {
    let mut roots: Vec<std::path::PathBuf> = Vec::new();

    // 1. Manifest dir → walk up looking for x64/{Debug,Release}.
    if let Some(manifest) = option_env!("CARGO_MANIFEST_DIR") {
        let mut p = std::path::PathBuf::from(manifest);
        for _ in 0..6 {
            for cfg in ["Debug", "Release"] {
                let candidate = p.join("x64").join(cfg);
                if candidate.is_dir() {
                    roots.push(candidate);
                }
            }
            if !p.pop() { break; }
        }
    }

    // 2. Fallbacks relative to current cwd.
    for rel in [
        "../x64/Debug",
        "../x64/Release",
        "../../x64/Debug",
        "../../x64/Release",
        "../../../x64/Debug",
        "../../../x64/Release",
        "x64/Debug",
        "x64/Release",
        ".",
    ] {
        let candidate = std::path::PathBuf::from(rel);
        if candidate.is_dir() {
            roots.push(candidate);
        }
    }

    // De-dup while preserving order.
    let mut seen = std::collections::HashSet::new();
    roots.retain(|p| {
        let key = fs::canonicalize(p).unwrap_or_else(|_| p.clone());
        seen.insert(key)
    });
    roots
}

fn read_cpp_status() -> Option<MotorStatus> {
    for dir in engine_output_dirs() {
        let path = dir.join(STATUS_FILE_NAME);
        if let Ok(contents) = fs::read_to_string(&path) {
            if let Ok(status) = serde_json::from_str::<MotorStatus>(&contents) {
                return Some(status);
            }
        }
    }
    None
}

fn read_cpp_scene() -> Option<serde_json::Value> {
    for dir in engine_output_dirs() {
        let path = dir.join(SCENE_FILE_NAME);
        if let Ok(contents) = fs::read_to_string(&path) {
            if let Ok(value) = serde_json::from_str::<serde_json::Value>(&contents) {
                return Some(value);
            }
        }
    }
    None
}

fn append_command_line(line: &str) -> Result<(), String> {
    for dir in engine_output_dirs() {
        if dir.is_dir() {
            let path = dir.join(COMMAND_FILE_NAME);
            let mut file = fs::OpenOptions::new()
                .create(true)
                .append(true)
                .open(&path)
                .map_err(|e| e.to_string())?;
            file.write_all(line.as_bytes()).map_err(|e| e.to_string())?;
            file.write_all(b"\n").map_err(|e| e.to_string())?;
            return Ok(());
        }
    }
    Err(String::from("no writable engine output directory found"))
}

fn effective_status(state: &State<'_, AppState>) -> MotorStatus {
    read_cpp_status().unwrap_or_else(|| {
        state
            .motor
            .lock()
            .map(|motor| motor.clone())
            .unwrap_or_default()
    })
}

#[tauri::command]
fn start_motor(app: AppHandle, state: State<'_, AppState>) -> Result<MotorStatus, String> {
    let mut motor = state.motor.lock().map_err(|error| error.to_string())?;
    motor.running = true;
    emit_status(&app, &motor)?;
    Ok(motor.clone())
}

#[tauri::command]
fn stop_motor(app: AppHandle, state: State<'_, AppState>) -> Result<MotorStatus, String> {
    let mut motor = state.motor.lock().map_err(|error| error.to_string())?;
    motor.running = false;
    emit_status(&app, &motor)?;
    Ok(motor.clone())
}

#[tauri::command]
fn get_motor_status(state: State<'_, AppState>) -> Result<MotorStatus, String> {
    Ok(effective_status(&state))
}

#[tauri::command]
fn get_scene() -> Result<serde_json::Value, String> {
    read_cpp_scene().ok_or_else(|| String::from("scene file not found"))
}

// Ack returned to React. The actual command result is asynchronous: the
// engine drains the queue on its next frame and updates the scene snapshot.
#[derive(Debug, Serialize)]
struct CommandAck {
    ok: bool,
    queued: bool,
    #[serde(skip_serializing_if = "Option::is_none")]
    error: Option<String>,
}

#[tauri::command]
fn dispatch_command(command: serde_json::Value) -> Result<CommandAck, String> {
    let line = serde_json::to_string(&command).map_err(|e| e.to_string())?;
    match append_command_line(&line) {
        Ok(()) => Ok(CommandAck { ok: true, queued: true, error: None }),
        Err(e) => Ok(CommandAck { ok: false, queued: false, error: Some(e) }),
    }
}

fn main() {
    tauri::Builder::default()
        .manage(AppState {
            motor: Mutex::new(MotorStatus::default()),
        })
        .setup(|app| {
            let handle = app.handle().clone();
            let motor = app
                .state::<AppState>()
                .motor
                .lock()
                .expect("motor state should be available")
                .clone();
            emit_status(&handle, &motor).expect("failed to emit initial motor status");

            let polling_handle = handle.clone();
            thread::spawn(move || {
                let mut last_status: Option<MotorStatus> = None;
                let mut last_scene: Option<serde_json::Value> = None;

                loop {
                    if let Some(next_status) = read_cpp_status() {
                        if last_status.as_ref() != Some(&next_status) {
                            let _ = emit_status(&polling_handle, &next_status);
                            last_status = Some(next_status);
                        }
                    }

                    if let Some(next_scene) = read_cpp_scene() {
                        if last_scene.as_ref() != Some(&next_scene) {
                            let _ = polling_handle.emit("scene-snapshot", next_scene.clone());
                            last_scene = Some(next_scene);
                        }
                    }

                    thread::sleep(Duration::from_millis(250));
                }
            });

            Ok(())
        })
        .invoke_handler(tauri::generate_handler![
            start_motor,
            stop_motor,
            get_motor_status,
            get_scene,
            dispatch_command
        ])
        .run(tauri::generate_context!())
        .expect("error while running Lightning Engine Editor");
}
