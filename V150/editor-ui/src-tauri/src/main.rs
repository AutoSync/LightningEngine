use serde::{Deserialize, Serialize};
use std::{fs, sync::Mutex, thread, time::Duration};
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

fn read_cpp_status() -> Option<MotorStatus> {
    let candidates = [
        "../x64/Debug/editor-bridge-status.json",
        "../x64/Release/editor-bridge-status.json",
        "editor-bridge-status.json",
    ];

    for candidate in candidates {
        if let Ok(contents) = fs::read_to_string(candidate) {
            if let Ok(status) = serde_json::from_str::<MotorStatus>(&contents) {
                return Some(status);
            }
        }
    }

    None
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

                loop {
                    if let Some(next_status) = read_cpp_status() {
                        if last_status.as_ref() != Some(&next_status) {
                            let _ = emit_status(&polling_handle, &next_status);
                            last_status = Some(next_status);
                        }
                    }

                    thread::sleep(Duration::from_millis(500));
                }
            });

            Ok(())
        })
        .invoke_handler(tauri::generate_handler![
            start_motor,
            stop_motor,
            get_motor_status
        ])
        .run(tauri::generate_context!())
        .expect("error while running Lightning Engine Editor");
}
