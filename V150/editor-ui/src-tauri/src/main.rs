use serde::{Deserialize, Serialize};
use std::sync::Mutex;
use tauri::{AppHandle, Manager, State};

#[derive(Debug, Clone, Serialize, Deserialize)]
struct MotorStatus {
    running: bool,
    fps: u32,
    scene: String,
    project: String,
}

impl Default for MotorStatus {
    fn default() -> Self {
        Self {
            running: false,
            fps: 60,
            scene: String::from("main_scene.lescene"),
            project: String::from("LightningEngine Demo"),
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
    let motor = state.motor.lock().map_err(|error| error.to_string())?;
    Ok(motor.clone())
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
