#pragma once
#include "Types.h"
// The system header defines the application data

//Software Version
#define APP_VERSION_STRING "0.14.0.0"
using namespace Lightning;

static EngineSettings Settings(840, 480, "Lightning Engine", true);
static MouseEvents mouse = MouseEvents();
//navigation settings
static double lastX = Settings.width / 2, lastY = Settings.height / 2;
static bool firstMouse = true;
static bool isProjection = true;