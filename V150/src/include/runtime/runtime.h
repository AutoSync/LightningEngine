// runtime.h — Umbrella header for all Lightning Engine Runtime interfaces.
//
// Include this when you need access to all runtime interface contracts.
// Prefer including individual headers when only one interface is needed.
//
// These interfaces decouple Core and plugins from concrete SDL3/Titan
// implementations. See docs/developers/01-architecture.md for the rationale.
#pragma once

#include "IRenderer.h"
#include "IInputManager.h"
#include "IPhysicsWorld.h"
#include "IAudioEngine.h"
#include "IUiHost.h"
#include "IScriptHost.h"
